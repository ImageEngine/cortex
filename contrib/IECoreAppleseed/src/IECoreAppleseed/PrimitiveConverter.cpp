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

#include "renderer/api/entity.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/private/AppleseedUtil.h"
#include "IECoreAppleseed/private/PrimitiveConverter.h"

#include "IECoreAppleseed/ToAppleseedConverter.h"
#include "IECoreAppleseed/private/AppleseedUtil.h"

using namespace std;
using namespace boost;
using namespace IECore;

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::PrimitiveConverter::PrimitiveConverter( const asf::SearchPaths &searchPaths ) : m_searchPaths( searchPaths )
{
	m_autoInstancing = true;
}

IECoreAppleseed::PrimitiveConverter::~PrimitiveConverter()
{
}

void IECoreAppleseed::PrimitiveConverter::setOption( const string &name, ConstDataPtr value )
{
}

const asr::Assembly *IECoreAppleseed::PrimitiveConverter::convertPrimitive( PrimitivePtr primitive, const AttributeState &attrState, const string &materialName, asr::Assembly &parentAssembly )
{
	// Compute the hash of the primitive and save it for later use.
	MurmurHash geometryHash;
	primitive->hash( geometryHash );

	MurmurHash geomAndAttributesHash( geometryHash );
	geomAndAttributesHash.append( attrState.alphaMap() );

	// Right now, appleseed instances share all the same material.
	// This will be lifted soon, but for now, we need to include
	// the shading / material state in the hash so that objects with
	// the same geometry but different materials are not instances.
	MurmurHash geomAndShadingHash( geomAndAttributesHash );
	geomAndShadingHash.append( attrState.materialHash() );

	// Check if we already processed this primitive.
	InstanceMapType::const_iterator it = m_instanceMap.find( geomAndShadingHash );
	if( it != m_instanceMap.end() )
	{
		return it->second;
	}

	asf::auto_release_ptr<asr::Object> obj = doConvertPrimitive( primitive, geometryHash );

	if( !obj.get() )
	{
		return 0;
	}

	string objName = obj->get_name();

	if( !attrState.alphaMap().empty() )
	{
		string alphaMapTextureInstanceName = createAlphaMapTextureEntity( parentAssembly.textures(), parentAssembly.texture_instances(), m_searchPaths, objName + "_alpha_map", attrState.alphaMap() );
		obj->get_parameters().insert( "alpha_map", alphaMapTextureInstanceName.c_str() );
	}

	asf::auto_release_ptr<asr::Assembly> ass = createAssembly( attrState.name() );
	const asr::Object *objPtr = obj.get();
	ass->objects().insert( obj );
	createObjectInstance( *ass, objPtr, objName, materialName );
	const asr::Assembly *p = ass.get();
	parentAssembly.assemblies().insert( ass );

	if( m_autoInstancing )
	{
		m_instanceMap[geomAndShadingHash] = p;
	}

	return p;
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
