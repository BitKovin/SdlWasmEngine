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

#include "image.h"

#include "modulesystem.h"
#include "iimage.h"
#include "ifilesystem.h"
#include "iarchive.h"

#include "generic/reference.h"
#include "os/path.h"
#include "stream/stringstream.h"
#include "string/string.h"

#include <cstring>


typedef Modules<_QERPlugImageTable> ImageModules;
ImageModules& Textures_getImageModules();

/// \brief Returns a new image for the first file matching \p name in one of the available texture formats, or 0 if no file is found.
Image* QERApp_LoadImage( void* environment, const char* name ){
	Image* image = 0;
	class LoadImageVisitor : public ImageModules::Visitor
	{
		const char* m_name;
		Image*& m_image;
	public:
		LoadImageVisitor( const char* name, Image*& image )
			: m_name( name ), m_image( image ){
		}
		void visit( const char* pluginExt, const _QERPlugImageTable& table ) const {
			if ( m_image != 0 )
				return;

			// If the shader name already ends with ".{pluginExt}", try opening it
			// directly first — avoids turning "texture0.jpg" into "texture0.jpg.jpg".
			// This handles embedded GLB texture paths like:
			//   "models/weapons/glock.glb/texture0.jpg"
			const std::size_t nameLen = strlen( m_name );
			const std::size_t extLen  = strlen( pluginExt );
			if ( nameLen > extLen + 1
			  && m_name[nameLen - extLen - 1] == '.'
			  && string_equal_nocase( m_name + nameLen - extLen, pluginExt ) ) {
				globalOutputStream() << "image: trying direct open (ext match): \"" << m_name << "\"\n";
				ArchiveFile* file = GlobalFileSystem().openFile( m_name );
				if ( file != 0 ) {
					globalOutputStream() << "image: direct open succeeded: \"" << m_name << "\"\n";
					m_image = table.loadImage( *file );
					file->release();
					return;
				}
				globalOutputStream() << "image: direct open failed: \"" << m_name << "\"\n";
			}

			// Normal path: append this plugin's extension.
			globalOutputStream() << "image: trying with appended ext: \"" << m_name << '.' << pluginExt << "\"\n";
			ArchiveFile* file = GlobalFileSystem().openFile( StringStream( m_name, '.', pluginExt ) );
			if ( file != 0 ) {
				globalOutputStream() << "image: openFile succeeded: \"" << m_name << '.' << pluginExt << "\"\n";
				m_image = table.loadImage( *file );
				file->release();
			}
		}
	};

	Textures_getImageModules().foreachModule( LoadImageVisitor( name, image ) );

	return image;
}
