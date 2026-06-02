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

#include "plugin.h"

#include <cstdio>
typedef unsigned char byte;
#include <cstdlib>
#include <algorithm>
#include <list>

#include "iscenegraph.h"
#include "irender.h"
#include "iselection.h"
#include "iimage.h"
#include "imodel.h"
#include "igl.h"
#include "ifilesystem.h"
#include "iundo.h"
#include "ifiletypes.h"

#include "modulesystem/singletonmodule.h"
#include "stream/textstream.h"
#include "string/string.h"
#include "stream/stringstream.h"
#include "typesystem.h"

#include "model.h"
#include "glbarchive.h"

#include "assimp/Importer.hpp"
#include "assimp/importerdesc.h"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/IOSystem.hpp"
#include "assimp/MemoryIOWrapper.h"
#include "assimp/mesh.h"
#include "iarchive.h"
#include "idatastream.h"
#include "mdlimage.h"


class AssLogger : public Assimp::Logger
{
public:
	void OnDebug( const char* message ) override {
#ifdef _DEBUG
		globalOutputStream() << message << '\n';
#endif
	}
	void OnVerboseDebug( const char *message ) override {
#ifdef _DEBUG
		globalOutputStream() << message << '\n';
#endif
	}
	void OnInfo( const char* message ) override {
#ifdef _DEBUG
		globalOutputStream() << message << '\n';
#endif
	}
	void OnWarn( const char* message ) override {
		globalWarningStream() << message << '\n';
	}
	void OnError( const char* message ) override {
		globalErrorStream() << message << '\n';
	}

	bool attachStream( Assimp::LogStream *pStream, unsigned int severity ) override {
		return false;
	}
	bool detachStream( Assimp::LogStream *pStream, unsigned int severity ) override {
		return false;
	}
};


class AssIOSystem : public Assimp::IOSystem
{
public:
	bool Exists( const char* pFile ) const override {
		if( strchr( pFile, '\\' ) != nullptr ){
			globalWarningStream() << "AssIOSystem::Exists " << pFile << '\n';
			return false;
		}

		ArchiveFile *file = GlobalFileSystem().openFile( pFile );
		if ( file != nullptr ) {
			file->release();
			return true;
		}
		return false;
	}

	char getOsSeparator() const override {
		return '/';
	}

	Assimp::IOStream* Open( const char* pFile, const char* pMode = "rb" ) override {
		if( strchr( pFile, '\\' ) != nullptr ){
			globalWarningStream() << "AssIOSystem::Open " << pFile << '\n';
			return nullptr;
		}

		ArchiveFile *file = GlobalFileSystem().openFile( pFile );
		if ( file != nullptr ) {
			const size_t size = file->size();
			byte *buffer = new byte[ size ];
			file->getInputStream().read( buffer, size );
			file->release();
			return new Assimp::MemoryIOStream( buffer, size, true );
		}
		return nullptr;
	}

	void Close( Assimp::IOStream* pFile ) override {
		delete pFile;
	}

	bool CreateDirectory( const std::string &path ) override {
		ASSERT_MESSAGE( false, "AssIOSystem::CreateDirectory" );
		return false;
	}

	bool ChangeDirectory( const std::string &path ) override {
		ASSERT_MESSAGE( false, "AssIOSystem::ChangeDirectory" );
		return false;
	}

	bool DeleteFile( const std::string &file ) override {
		ASSERT_MESSAGE( false, "AssIOSystem::DeleteFile" );
		return false;
	}
};

static Assimp::Importer *s_assImporter = nullptr;

void pico_initialise(){
	s_assImporter = new Assimp::Importer();

	s_assImporter->SetPropertyBool( AI_CONFIG_PP_PTV_ADD_ROOT_TRANSFORMATION, true );
	s_assImporter->SetPropertyInteger( AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE );
	s_assImporter->SetPropertyString( AI_CONFIG_IMPORT_MDL_COLORMAP, "gfx/palette.lmp" );
	s_assImporter->SetPropertyBool( AI_CONFIG_IMPORT_MD3_LOAD_SHADERS, false );
	s_assImporter->SetPropertyString( AI_CONFIG_IMPORT_MD3_SHADER_SRC, "scripts/" );
	s_assImporter->SetPropertyBool( AI_CONFIG_IMPORT_MD3_HANDLE_MULTIPART, false );

	Assimp::DefaultLogger::set( new AssLogger );

	s_assImporter->SetIOHandler( new AssIOSystem );
}


class PicoModelLoader : public ModelLoader
{
public:
	PicoModelLoader(){
	}
	scene::Node& loadModel( ArchiveFile& file ){
		return loadPicoModel( *s_assImporter, file );
	}
};

class ModelPicoDependencies :
	public GlobalFileSystemModuleRef,
	public GlobalOpenGLModuleRef,
	public GlobalUndoModuleRef,
	public GlobalSceneGraphModuleRef,
	public GlobalShaderCacheModuleRef,
	public GlobalSelectionModuleRef,
	public GlobalFiletypesModuleRef
{
};

class ModelPicoAPI : public TypeSystemRef
{
	PicoModelLoader m_modelLoader;
public:
	typedef ModelLoader Type;

	ModelPicoAPI( const char* extension ){
		GlobalFiletypesModule::getTable().addType( Type::Name, extension, filetype_t( StringStream<32>( extension, " model" ), StringStream<16>( "*.", extension ) ) );
	}
	ModelLoader* getTable(){
		return &m_modelLoader;
	}
};

class PicoModelAPIConstructor
{
	CopiedString m_extension;
public:
	PicoModelAPIConstructor( const char* extension ) :
		m_extension( extension ) {
	}
	const char* getName(){
		return m_extension.c_str();
	}
	ModelPicoAPI* constructAPI( ModelPicoDependencies& dependencies ){
		return new ModelPicoAPI( m_extension.c_str() );
	}
	void destroyAPI( ModelPicoAPI* api ){
		delete api;
	}
};


typedef SingletonModule<ModelPicoAPI, ModelPicoDependencies, PicoModelAPIConstructor> PicoModelModule;
typedef std::list<PicoModelModule> PicoModelModules;
PicoModelModules g_PicoModelModules;


// ---------------------------------------------------------------------------
// GLB archive module — registers OpenGLBArchive for the "glb" extension so
// the VFS mounts every .glb file as a virtual archive.  Embedded textures
// become addressable as  "models/weapons/glock.glb/texture0.jpg"  etc.
// ---------------------------------------------------------------------------

class ArchiveGLBAPI
{
	_QERArchiveTable m_table;
public:
	typedef _QERArchiveTable Type;
	STRING_CONSTANT( Name, "glb" );

	ArchiveGLBAPI(){
		m_table.m_pfnOpenArchive = &OpenGLBArchive;
	}
	_QERArchiveTable* getTable(){
		return &m_table;
	}
};

typedef SingletonModule<ArchiveGLBAPI> ArchiveGLBModule;
ArchiveGLBModule g_ArchiveGLBModule;


// ---------------------------------------------------------------------------
// MDL image module (unchanged)
// ---------------------------------------------------------------------------

class ImageMDLAPI
{
	_QERPlugImageTable m_imagemdl;
public:
	typedef _QERPlugImageTable Type;
	STRING_CONSTANT( Name, "mdl" );

	ImageMDLAPI(){
		m_imagemdl.loadImage = &LoadMDLImage_;
	}
	_QERPlugImageTable* getTable(){
		return &m_imagemdl;
	}
	static Image* LoadMDLImage_( ArchiveFile& file ){
		return LoadMDLImage( *s_assImporter, file );
	}
};

typedef SingletonModule<ImageMDLAPI, GlobalFileSystemModuleRef> ImageMDLModule;

ImageMDLModule g_ImageMDLModule;



extern "C" void RADIANT_DLLEXPORT Radiant_RegisterModules( ModuleServer& server ){
	initialiseModule( server );

	pico_initialise();

	for( size_t i = 0; i < s_assImporter->GetImporterCount(); ++i ){
		globalOutputStream() << s_assImporter->GetImporterInfo( i )->mName << " (" << s_assImporter->GetImporterInfo( i )->mFileExtensions << ")\n";
	}

	aiString extensions;
	s_assImporter->GetExtensionList( extensions ); // "*.3ds;*.obj;*.dae"
	const char *c = extensions.C_Str();
	while( !string_empty( c ) ){
		StringOutputStream ext( 16 );
		do{
			if( *c == '*' && *( c + 1 ) == '.' ){
				c += 2;
				continue;
			}
			else if( *c == ';' ){
				++c;
				break;
			}
			else{
				ext << *c;
				++c;
			}
		} while( !string_empty( c ) );

		g_PicoModelModules.push_back( PicoModelModule( PicoModelAPIConstructor( ext ) ) );
		g_PicoModelModules.back().selfRegister();
	}

	// Register GLB as a VFS archive type so embedded textures are served
	// through the normal file system as  "model.glb/texture0.jpg"  etc.
	g_ArchiveGLBModule.selfRegister();

	g_ImageMDLModule.selfRegister();
}
