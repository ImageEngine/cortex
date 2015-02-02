//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "boost/filesystem/convenience.hpp"

#include "renderer/api/version.h"
#include "renderer/api/entity.h"

#include "IECore/MessageHandler.h"

#include "IECoreAppleseed/private/PrimitiveConverter.h"

#include "IECoreAppleseed/ToAppleseedConverter.h"
#include "IECoreAppleseed/private/AppleseedUtil.h"

using namespace std;
using namespace boost;
using namespace IECore;

namespace asf = foundation;
namespace asr = renderer;

#include <iostream>

IECoreAppleseed::PrimitiveConverter::PrimitiveConverter( const filesystem::path &projectPath )
{
	m_projectPath = projectPath;
	m_interactive = m_projectPath.empty();
	setMeshFileFormat( BinaryMeshFormat );
}

void IECoreAppleseed::PrimitiveConverter::setMeshFileFormat( MeshFileFormat format )
{
	if( format == BinaryMeshFormat )
		m_meshGeomExtension = ".binarymesh";
	else
		m_meshGeomExtension = ".obj";
}

const asr::Assembly *IECoreAppleseed::PrimitiveConverter::convertPrimitive( PrimitivePtr primitive, const AttributeState &attrState, const string &materialName, asr::Assembly &parentAssembly, const foundation::SearchPaths &searchPaths )
{
	// Compute the hash of the primitive and save it for later use.
	MurmurHash geometryHash;
	primitive->hash( geometryHash );

	// Right now, appleseed instances share all the same material.
	// This will be lifted soon, but for now, we need to include
	// the shading / material state in the hash so that objects with
	// the same geometry but different materials are not instances.
	MurmurHash geomAndShadingHash( geometryHash );
	geomAndShadingHash.append( attrState.alphaMap() );
	geomAndShadingHash.append( attrState.materialHash() );

	// Check if we already processed this primitive.
	string assemblyKey = geomAndShadingHash.toString() + "_assembly";
	std::map<string, const asr::Assembly*>::const_iterator it = m_instanceMap.find( assemblyKey );
	if( it != m_instanceMap.end() )
	{
		return it->second;
	}

	asf::auto_release_ptr<asr::Object> obj;

	if( m_interactive )
	{
		ToAppleseedConverterPtr converter = ToAppleseedConverter::create( primitive.get() );

		if( !converter )
		{
			msg( Msg::Warning, "IECoreAppleseed::PrimitiveConverter", "Couldn't convert object" );
		}

		obj.reset( static_cast<asr::Object*>( converter->convert() ) );

		if( obj.get() )
		{
			obj->set_name( geomAndShadingHash.toString().c_str() );
		}
	}
	else
	{
		switch( primitive->typeId() )
		{
			case MeshPrimitiveTypeId :
				obj = convertAndWriteMeshPrimitive( primitive, geometryHash );
				break;

			case CurvesPrimitiveTypeId :
			case PointsPrimitiveTypeId :
			case DiskPrimitiveTypeId :
			case SpherePrimitiveTypeId :
			case NURBSPrimitiveTypeId :
			case PatchMeshPrimitiveTypeId :
			default :
				return 0;
		}
	}

	if( !obj.get() )
	{
		return 0;
	}

	string objName = obj->get_name();

#if APPLESEED_VERSION > 10100
	if( !attrState.alphaMap().empty() )
	{
		string alphaMapTextureInstanceName = createAlphaMapTextureEntity( parentAssembly.textures(), parentAssembly.texture_instances(), searchPaths, objName + "_alpha_map", attrState.alphaMap() );
		obj->get_parameters().insert( "alpha_map", alphaMapTextureInstanceName.c_str() );
	}
#endif

	string assemblyName = attrState.name();
	asf::auto_release_ptr<asr::Assembly> ass = asr::AssemblyFactory::create( assemblyName.c_str(), asr::ParamArray() );
	const asr::Object *objPtr = obj.get();
	ass->objects().insert( obj );
	createObjectInstance( *ass, objPtr, objName, materialName );
	const asr::Assembly *p = ass.get();
	parentAssembly.assemblies().insert( ass );
	m_instanceMap[assemblyKey] = p;
	return p;
}

asf::auto_release_ptr<asr::Object> IECoreAppleseed::PrimitiveConverter::convertAndWriteMeshPrimitive( PrimitivePtr primitive, const MurmurHash &meshHash )
{
	asf::auto_release_ptr<asr::Object> entity;
	string objectName = meshHash.toString();

	// Check if we already have a mesh saved for this object.
	string fileName = string( "_geometry/" ) + objectName + m_meshGeomExtension;
	filesystem::path p = m_projectPath / fileName;

	if( !filesystem::exists( p ) )
	{
		ToAppleseedConverterPtr converter = ToAppleseedConverter::create( primitive.get() );

		if( !converter )
		{
			msg( Msg::Warning, "IECoreAppleseed::PrimitiveConverter", "Couldn't convert object" );
		}

		entity.reset( static_cast<asr::Object*>( converter->convert() ) );

		if( entity.get() == 0 )
		{
			msg( Msg::Warning, "IECoreAppleseed::PrimitiveConverter", "Couldn't convert object" );
			return entity;
		}

		entity->set_name( objectName.c_str() );
		const asr::MeshObject *mesh = static_cast<const asr::MeshObject*>( entity.get() );

		// Write the mesh to a file.
		p = m_projectPath / fileName;
		if( asr::MeshObjectWriter::write( *mesh, mesh->get_name(), p.string().c_str() ) )
		{
			// we don't need the mesh data anymore.
			// Replace the object by a new empty mesh object referencing the file
			// we just saved.
			entity.reset( asr::MeshObjectFactory().create( objectName.c_str(), asr::ParamArray().insert( "filename", fileName.c_str() ) ).release() );
		}
		else
		{
			msg( Msg::Warning, "IECoreAppleseed::PrimitiveConverter", "Couldn't save mesh primitive" );
			entity.reset();
		}
	}
	else
	{
		entity.reset( asr::MeshObjectFactory().create( objectName.c_str(), asr::ParamArray().insert( "filename", fileName.c_str() ) ).release() );
	}

	return entity;
}

void IECoreAppleseed::PrimitiveConverter::createObjectInstance( asr::Assembly &assembly, const renderer::Object *obj, const string &objSourceName, const string &materialName )
{
	assert( obj );

	string sourceName = objSourceName + "." + objSourceName;
	string instanceName = string( assembly.get_name() ) + "_obj_instance";

	asf::StringDictionary materials;

	if( !materialName.empty() )
	{
		materials.insert( "default", materialName.c_str() );
	}

	asf::auto_release_ptr<asr::ObjectInstance> objInstance = asr::ObjectInstanceFactory::create( instanceName.c_str(), asr::ParamArray(), sourceName.c_str(), asf::Transformd::make_identity(), materials, materials );
	assembly.object_instances().insert( objInstance );
}
