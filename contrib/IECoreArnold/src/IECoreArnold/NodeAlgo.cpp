//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

#include "IECoreArnold/NodeAlgo.h"

#include "boost/unordered_map.hpp"

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

using namespace IECoreArnold;

struct Converters
{

	Converters( NodeAlgo::Converter converter, NodeAlgo::MotionConverter motionConverter )
		:	converter( converter ), motionConverter( motionConverter )
	{
	}

	NodeAlgo::Converter converter;
	NodeAlgo::MotionConverter motionConverter;

};

typedef boost::unordered_map<IECore::TypeId, Converters> Registry;

Registry &registry()
{
	static Registry g_registry;
	return g_registry;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Public implementation
//////////////////////////////////////////////////////////////////////////

namespace IECoreArnold
{

namespace NodeAlgo
{

AtNode *convert( const IECore::Object *object, const std::string &nodeName, const AtNode *parentNode )
{
	const Registry &r = registry();
	Registry::const_iterator it = r.find( object->typeId() );
	if( it == r.end() )
	{
		return nullptr;
	}
	return it->second.converter( object, nodeName, parentNode );
}

AtNode *convert( const std::vector<const IECore::Object *> &samples, float motionStart, float motionEnd, const std::string &nodeName, const AtNode *parentNode )
{
	if( samples.empty() )
	{
		return nullptr;
	}

	const IECore::Object *firstSample = samples.front();
	const IECore::TypeId firstSampleTypeId = firstSample->typeId();
	for( std::vector<const IECore::Object *>::const_iterator it = samples.begin()+1, eIt = samples.end(); it != eIt; ++it )
	{
		if( (*it)->typeId() != firstSampleTypeId )
		{
			throw IECore::Exception( "Inconsistent object types." );
		}
	}

	const Registry &r = registry();
	Registry::const_iterator it = r.find( firstSampleTypeId );
	if( it == r.end() )
	{
		return nullptr;
	}

	if( it->second.motionConverter )
	{
		return it->second.motionConverter( samples, motionStart, motionEnd, nodeName, parentNode );
	}
	else
	{
		return it->second.converter( firstSample, nodeName, parentNode );
	}
}

void registerConverter( IECore::TypeId fromType, Converter converter, MotionConverter motionConverter )
{
	registry().insert( Registry::value_type( fromType, Converters( converter, motionConverter ) ) );
}

void ensureUniformTimeSamples( const std::vector<float> &times )
{
	if( times.size() == 0 )
	{
		throw IECore::Exception( "Motion block times must not be empty" );
	}

	float motionStart = times[0];
	float motionEnd = times[ times.size() - 1 ];

	for( unsigned int i = 0; i < times.size(); i++ )
	{
		// Use a really coarse epsilon to check if the values are uniform - if someone is sloppy with
		// floating point precision when computing their sample times, we don't want to stop them from rendering.
		// But we should warn someone if they are actually trying to use a feature Arnold doesn't support.
		const float uniformity_epsilon = 0.01;
		float expectedTime = motionStart + ( motionEnd - motionStart ) / ( times.size() - 1 ) * i;
		if( times[i] < expectedTime - uniformity_epsilon || times[i] > expectedTime + uniformity_epsilon )
		{
			std::stringstream text;
			text << "Arnold does not support non-uniform motion blocks.\n";
			text << "Invalid motion block: [ " << times[0];
			for( unsigned int j = 1; j < times.size(); j++ )
			{
				text << ", " << times[j];
			}
			text << " ]\n";
			text << "( sample " << i << ", with value " << times[i] << " does not match " << expectedTime << ")\n";
			throw IECore::Exception( text.str() );


		}
	}

}

} // namespace NodeAlgo

} // namespace IECoreArnold
