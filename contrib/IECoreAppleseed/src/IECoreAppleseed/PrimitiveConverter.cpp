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

#include <algorithm>

#include "foundation/math/scalar.h"

#include "renderer/api/entity.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/ObjectInterpolator.h"

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
	m_shutterOpenTime = 1.0f;
	m_shutterCloseTime = 0.0f;
}

IECoreAppleseed::PrimitiveConverter::~PrimitiveConverter()
{
}

void IECoreAppleseed::PrimitiveConverter::setOption( const string &name, ConstDataPtr value )
{
	if( name == "as:automatic_instancing" )
	{
		if( const BoolData *f = runTimeCast<const BoolData>( value.get() ) )
		{
			m_autoInstancing = f->readable();
		}
		else
		{
			msg( Msg::Error, "IECoreAppleseed::RendererImplementation::setOption", "as:automatic_instancing option expects a BoolData value." );
		}
	}
}

void IECoreAppleseed::PrimitiveConverter::setShutterInterval( float openTime, float closeTime )
{
	m_shutterOpenTime = openTime;
	m_shutterCloseTime = closeTime;
}

const asr::Assembly *IECoreAppleseed::PrimitiveConverter::convertPrimitive( PrimitivePtr primitive, const AttributeState &attrState,
	const string &materialName, asr::Assembly &parentAssembly )
{
	MurmurHash primitiveHash;
	primitive->hash( primitiveHash );
	attrState.attributesHash( primitiveHash );

	// Right now, appleseed instances share all the same material.
	// This will be lifted soon, but for now, we need to include
	// the shading / material state in the hash so that objects with
	// the same geometry but different materials are not instances.
	attrState.materialHash( primitiveHash );

	// Check if we already processed this primitive.
	InstanceMapType::const_iterator it = m_instanceMap.find( primitiveHash );
	if( it != m_instanceMap.end() )
	{
		return it->second;
	}

	asf::auto_release_ptr<asr::Object> obj = doConvertPrimitive( primitive, attrState.name() );

	if( !obj.get() )
	{
		return 0;
	}

	return addObjectToScene( obj, primitiveHash, attrState, materialName, parentAssembly );
}

const asr::Assembly *IECoreAppleseed::PrimitiveConverter::convertPrimitive( const set<float> &times,
			const vector<PrimitivePtr> &primitives, const AttributeState &attrState,
			const string &materialName, renderer::Assembly &parentAssembly )
{
	assert( times.size() >= 2 );

	if( m_shutterCloseTime <= m_shutterOpenTime )
	{
		msg( Msg::Error, "IECoreAppleseed::RendererImplementation::motionEnd", "Camera shutter times not specified." );

		// shutter interval is not set or empty. ignore motion blur.
		return convertPrimitive( primitives[0], attrState, materialName, parentAssembly );
	}

	// appleseed requires a power of 2 number of deformation samples,
	// equally spaced between shutter open / close times.
	// check if the time samples satisfy the conditions.
	const vector<PrimitivePtr> *primitivesPtr = &primitives;
	vector<PrimitivePtr> resampledPrimitives;

	if( !checkTimeSamples( times ) )
	{
		// we need to resample the deformation samples.
		msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::motionEnd", "Resampling primitive samples." );

		int samples = asf::is_pow2( times.size() ) ? times.size() : asf::next_pow2( times.size() );
		vector<float> sortedTimes( times.begin(), times.end() );

		for( int i = 0; i < samples; ++i)
		{
			const float time = static_cast<float>( i ) / (samples - 1) * (m_shutterCloseTime - m_shutterOpenTime) + m_shutterOpenTime;

			if( time <= sortedTimes.front())
			{
				resampledPrimitives.push_back( primitives.front() );
				continue;
			}

			if( time >= sortedTimes.back())
			{
				resampledPrimitives.push_back( primitives.back() );
				continue;
			}

			vector<float>::const_iterator it = lower_bound(sortedTimes.begin(), sortedTimes.end(), time);
			int index = it - sortedTimes.begin() - 1;

			float t = (time - sortedTimes[index]) / (sortedTimes[index + 1] - sortedTimes[index]);

			ObjectPtr obj = linearObjectInterpolation( primitives[index].get(), primitives[index + 1].get(), t);

			if( !obj )
			{
				msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::motionEnd", "Error converting primitive. Disabling motion blur." );
				return convertPrimitive( primitives[0], attrState, materialName, parentAssembly );
			}

			PrimitivePtr p = runTimeCast<Primitive>( obj );

			if( !p )
			{
				msg( Msg::Warning, "IECoreAppleseed::RendererImplementation::motionEnd", "Error converting primitive. Disabling motion blur." );
				return convertPrimitive( primitives[0], attrState, materialName, parentAssembly );
			}

			resampledPrimitives.push_back( p );
		}

		primitivesPtr = &resampledPrimitives;
	}

	// compute the hash of all the deformation samples.
	MurmurHash primitiveHash;
	primitiveHash.append( (uint64_t)primitivesPtr->size() );
	primitiveHash.append( m_shutterOpenTime );
	primitiveHash.append( m_shutterCloseTime );

	for( size_t i = 0, e = primitivesPtr->size(); i < e; ++i )
	{
		(*primitivesPtr)[i]->hash( primitiveHash );
	}

	attrState.attributesHash( primitiveHash );

	// Right now, appleseed instances share all the same material.
	// This will be lifted soon, but for now, we need to include
	// the shading / material state in the hash so that objects with
	// the same geometry but different materials are not instances.
	attrState.materialHash( primitiveHash );

	// Check if we already processed this primitive.
	InstanceMapType::const_iterator it = m_instanceMap.find( primitiveHash );
	if( it != m_instanceMap.end() )
	{
		return it->second;
	}

	asf::auto_release_ptr<asr::Object> obj = doConvertPrimitive( *primitivesPtr, attrState.name() );

	if( !obj.get() )
	{
		return 0;
	}

	return addObjectToScene( obj, primitiveHash, attrState, materialName, parentAssembly );
}

const asr::Assembly *IECoreAppleseed::PrimitiveConverter::addObjectToScene( asf::auto_release_ptr<asr::Object> &obj,
	const MurmurHash &primitiveHash, const AttributeState &attrState,
	const string &materialName, asr::Assembly &parentAssembly )
{
	string objName = obj->get_name();

	if( !attrState.alphaMap().empty() )
	{
		string alphaMapTextureInstanceName = createAlphaMapTextureEntity( parentAssembly.textures(), parentAssembly.texture_instances(), m_searchPaths, objName + "_alpha_map", attrState.alphaMap() );
		obj->get_parameters().insert( "alpha_map", alphaMapTextureInstanceName.c_str() );
	}

	string assemblyName = attrState.name() + "_assembly";
	asf::auto_release_ptr<asr::Assembly> ass = asr::AssemblyFactory().create( assemblyName.c_str(), asr::ParamArray() );
	const asr::Object *objPtr = obj.get();
	ass->objects().insert( obj );
	createObjectInstance( *ass, objPtr, objName, attrState, materialName );
	const asr::Assembly *p = ass.get();
	parentAssembly.assemblies().insert( ass );

	if( m_autoInstancing )
	{
		m_instanceMap[primitiveHash] = p;
	}

	return p;
}

void IECoreAppleseed::PrimitiveConverter::createObjectInstance( asr::Assembly &assembly, const renderer::Object *obj,
	const string &objSourceName, const AttributeState &attrState, const string &materialName )
{
	assert( obj );

	string instanceName = string( assembly.get_name() ) + "_obj_instance";

	asf::StringDictionary materials;

	if( !materialName.empty() )
	{
		materials.insert( "default", materialName.c_str() );
	}

	asr::ParamArray params;

	if( attrState.photonTarget() )
	{
		params.insert( "photon_target", "true" );
	}

	if( attrState.mediumPriority() != 0 )
	{
		params.insert( "medium_priority", attrState.mediumPriority() );
	}

	asf::auto_release_ptr<asr::ObjectInstance> objInstance = asr::ObjectInstanceFactory::create( instanceName.c_str(),
		params, objectEntityName( objSourceName ).c_str(), asf::Transformd::make_identity(), materials, materials );
	assembly.object_instances().insert( objInstance );
}

bool IECoreAppleseed::PrimitiveConverter::checkTimeSamples( const set<float> &times ) const
{
	// check that the number of samples is a power of 2.
	if( !asf::is_pow2( times.size() ) )
	{
		return false;
	}

	const float eps = 0.01f;

	// check that the first and last sample matches the shutter times.
	if( !asf::feq( m_shutterOpenTime, *times.begin(), eps ) )
	{
		return false;
	}

	if( !asf::feq( m_shutterCloseTime, *times.rbegin(), eps ) )
	{
		return false;
	}

	// check that the samples are equally spaced.
	set<float>::const_iterator next( times.begin() );
	set<float>::const_iterator it( next++ );

	float sampleInterval = *next - *it;

	for( set<float>::const_iterator e( times.end() ) ; next != e; ++next )
	{
		if( !asf::feq( sampleInterval, *next - *it, eps ) )
		{
			return false;
		}

		it = next;
	}

	return true;
}
