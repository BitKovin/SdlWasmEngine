/*
   Copyright (C) 2001-2006, William Joseph.
   All Rights Reserved.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

// GLB virtual archive — mounts a .glb file's embedded textures into the VFS.
//
// When the VFS scans game directories it calls OpenGLBArchive for every .glb
// it finds (because we register a _QERArchiveTable for the "glb" extension in
// plugin.cpp).  The resulting GlbArchive is added to g_archives alongside the
// normal directory and pk3 archives.  Subsequent VFS openFile calls of the form
//
//   "models/weapons/glock.glb/texture0.jpg"
//
// are routed through every registered archive's openFile() in turn; GlbArchive
// recognises the path, strips the "models/weapons/glock.glb/" prefix, looks up
// the texture by index, and returns an ArchiveFile backed by the in-memory blob.
// The existing jpg/png image plugins then decode it normally.
//
// Raw (uncompressed) pixel textures are encoded as TGA in memory so the
// existing TGA image plugin can decode them.

#include "glbarchive.h"

#include "iarchive.h"
#include "idatastream.h"
#include "ifilesystem.h"
#include "string/string.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include <cstring>
#include <string>
#include <vector>
#include <map>


// ---------------------------------------------------------------------------
// MemoryInputStream — InputStream backed by an externally-owned byte vector
// ---------------------------------------------------------------------------

class MemoryInputStream : public InputStream
{
	const byte_type* m_begin;
	const byte_type* m_end;
	const byte_type* m_cur;
public:
	MemoryInputStream( const byte_type* data, std::size_t size )
		: m_begin( data ), m_end( data + size ), m_cur( data )
	{}
	size_type read( byte_type* buffer, size_type length ) override {
		std::size_t avail = static_cast<std::size_t>( m_end - m_cur );
		if ( length > avail ) length = avail;
		std::memcpy( buffer, m_cur, length );
		m_cur += length;
		return length;
	}
};


// ---------------------------------------------------------------------------
// MemoryArchiveFile — ArchiveFile that owns its data as a byte vector
// ---------------------------------------------------------------------------

class MemoryArchiveFile final : public ArchiveFile
{
	std::string m_name;
	std::vector<unsigned char> m_data;
	MemoryInputStream m_stream;
public:
	MemoryArchiveFile( const char* name, std::vector<unsigned char> data )
		: m_name( name )
		, m_data( std::move( data ) )
		, m_stream( m_data.data(), m_data.size() )
	{}

	void release() override {
		delete this;
	}
	std::size_t size() const override {
		return m_data.size();
	}
	const char* getName() const override {
		return m_name.c_str();
	}
	InputStream& getInputStream() override {
		return m_stream;
	}
};


// ---------------------------------------------------------------------------
// TGA in-memory encoder for raw aiTexel (BGRA) pixel data
// aiTexel layout is { b, g, r, a } which is identical to TGA 32-bpp layout.
// ---------------------------------------------------------------------------

static std::vector<unsigned char> encodeRawAsTGA( const aiTexel* pixels,
                                                   unsigned int w,
                                                   unsigned int h )
{
	std::vector<unsigned char> tga;
	tga.reserve( 18 + w * h * 4 );

	unsigned char hdr[18] = {};
	hdr[2]  = 2;                             // uncompressed true-colour
	hdr[12] = (unsigned char)( w        & 0xFF );
	hdr[13] = (unsigned char)( (w >> 8) & 0xFF );
	hdr[14] = (unsigned char)( h        & 0xFF );
	hdr[15] = (unsigned char)( (h >> 8) & 0xFF );
	hdr[16] = 32;                            // 32 bpp (BGRA)
	hdr[17] = 8;                             // 8-bit alpha, lower-left origin
	tga.insert( tga.end(), hdr, hdr + 18 );

	const unsigned char* src = reinterpret_cast<const unsigned char*>( pixels );
	tga.insert( tga.end(), src, src + static_cast<std::size_t>( w ) * h * 4 );
	return tga;
}


// ---------------------------------------------------------------------------
// GlbArchive
// ---------------------------------------------------------------------------

class GlbArchive final : public Archive
{
	// Absolute path this archive was opened from.
	// e.g. "F:/game/GameData/models/weapons/glock.glb"
	std::string m_absPath;

	// VFS-relative prefix including trailing slash, computed lazily from
	// GlobalFileSystem().findRoot() on the first openFile() call.
	// e.g. "models/weapons/glock.glb/"
	std::string m_vfsPrefix;

	// Embedded texture file table.
	// key:   relative texture filename,  e.g. "texture0.jpg"
	// value: raw file bytes (PNG/JPEG blob, or TGA-encoded for raw pixels)
	std::map<std::string, std::vector<unsigned char>> m_files;


	// Build the VFS prefix from the absolute archive path.
	// Called once, lazily, on first openFile().
	void buildVfsPrefix() {
		const char* root = GlobalFileSystem().findRoot( m_absPath.c_str() );
		if ( root && root[0] != '\0' ) {
			// Strip the VFS root from the front of m_absPath.
			// findRoot returns the longest matching root, e.g. "F:/game/GameData/"
			std::size_t rootLen = std::strlen( root );
			if ( m_absPath.size() > rootLen ) {
				m_vfsPrefix = m_absPath.substr( rootLen );
			}
			else {
				m_vfsPrefix = m_absPath;
			}
		}
		else {
			// Fallback: use the path as-is (shouldn't normally happen).
			m_vfsPrefix = m_absPath;
		}
		// Normalise separators and append the trailing slash.
		for ( char& c : m_vfsPrefix )
			if ( c == '\\' ) c = '/';
		m_vfsPrefix += '/';
	}


public:
	explicit GlbArchive( const char* absPath ) : m_absPath( absPath ) {
		// Use a local importer with the default (disk) IO handler.
		// The archive is opened with the absolute path during VFS directory
		// scanning, so assimp's default fopen-based reader works fine here.
		Assimp::Importer importer;
		// We only need the embedded texture blobs; skip all mesh processing.
		const aiScene* scene = importer.ReadFile( absPath, 0 );

		if ( !scene || scene->mNumTextures == 0 )
			return;

		for ( unsigned int i = 0; i < scene->mNumTextures; ++i ) {
			const aiTexture* tex = scene->mTextures[i];

			const bool isRaw = ( tex->mHeight > 0 );
			const char* ext  = isRaw ? "tga"
			                         : ( tex->achFormatHint[0] != '\0'
			                             ? tex->achFormatHint : "png" );

			// e.g. "texture0.jpg"
			const std::string filename = "texture" + std::to_string( i ) + "." + ext;

			if ( isRaw ) {
				m_files[filename] = encodeRawAsTGA(
				    reinterpret_cast<const aiTexel*>( tex->pcData ),
				    tex->mWidth, tex->mHeight );
			}
			else {
				const unsigned char* blob = reinterpret_cast<const unsigned char*>( tex->pcData );
				m_files[filename] = std::vector<unsigned char>( blob, blob + tex->mWidth );
			}
		}
		globalOutputStream() << "GlbArchive(\"" << absPath << "\"): loaded " << (int)m_files.size() << " embedded textures\n";
		for ( auto& kv : m_files )
			globalOutputStream() << "  texture key: \"" << kv.first.c_str() << "\" (" << (int)kv.second.size() << " bytes)\n";
	}

	void release() override {
		delete this;
	}

	// Called by the VFS with the full VFS-relative path,
	// e.g. "models/weapons/glock.glb/texture0.jpg".
	ArchiveFile* openFile( const char* name ) override {
		globalOutputStream() << "GlbArchive::openFile(\"" << name << "\") files=" << (int)m_files.size() << "\n";
		if ( m_files.empty() ) return nullptr;

		if ( m_vfsPrefix.empty() )
			buildVfsPrefix();

		globalOutputStream() << "GlbArchive: prefix=\"" << m_vfsPrefix.c_str() << "\"\n";

		// Check that name starts with our archive prefix (case-insensitive).
		if ( !string_equal_prefix_nocase( name, m_vfsPrefix.c_str() ) ) {
			globalOutputStream() << "GlbArchive: prefix mismatch, skipping\n";
			return nullptr;
		}

		const char* texName = name + m_vfsPrefix.size();
		globalOutputStream() << "GlbArchive: looking up texName=\"" << texName << "\"\n";

		// Exact-match (texture names are always lower-case from our side).
		auto it = m_files.find( texName );
		if ( it != m_files.end() )
			return new MemoryArchiveFile( name, it->second );

		// Case-insensitive fallback (VFS requests may differ in case).
		for ( auto& kv : m_files ) {
			if ( string_equal_nocase( kv.first.c_str(), texName ) )
				return new MemoryArchiveFile( name, kv.second );
		}

		return nullptr;
	}

	ArchiveTextFile* openTextFile( const char* name ) override {
		return nullptr; // embedded textures are binary only
	}

	bool containsFile( const char* name ) override {
		if ( m_files.empty() ) return false;

		if ( m_vfsPrefix.empty() )
			buildVfsPrefix();

		if ( !string_equal_prefix_nocase( name, m_vfsPrefix.c_str() ) )
			return false;

		const char* texName = name + m_vfsPrefix.size();
		if ( m_files.count( texName ) ) return true;
		for ( auto& kv : m_files )
			if ( string_equal_nocase( kv.first.c_str(), texName ) ) return true;
		return false;
	}

	void forEachFile( VisitorFunc visitor, const char* root ) override {
		// Serve file listing so the VFS can enumerate embedded textures.
		if ( m_files.empty() ) return;

		if ( m_vfsPrefix.empty() )
			buildVfsPrefix();

		for ( auto& kv : m_files ) {
			const std::string fullPath = m_vfsPrefix + kv.first;
			visitor.file( fullPath.c_str() );
		}
	}
};


// ---------------------------------------------------------------------------
// Factory function registered in _QERArchiveTable
// ---------------------------------------------------------------------------

Archive* OpenGLBArchive( const char* name ) {
	return new GlbArchive( name );
}
