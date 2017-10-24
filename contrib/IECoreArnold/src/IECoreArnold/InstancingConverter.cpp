//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

// This must come before the Cortex includes, because on OSX headers included
// by TBB define macros which conflict with the inline functions in ai_types.h.
#include "ai.h"

#include "tbb/concurrent_hash_map.h"

#include "IECoreArnold/InstancingConverter.h"
#include "IECoreArnold/NodeAlgo.h"

using namespace IECoreArnold;

struct InstancingConverter::MemberData
{
	typedef tbb::concurrent_hash_map<IECore::MurmurHash, AtNode *> Cache;
	Cache cache;
};

InstancingConverter::InstancingConverter()
{
	m_data = new MemberData();
}

InstancingConverter::~InstancingConverter()
{
	delete m_data;
}

AtNode *InstancingConverter::convert( const IECore::Primitive *primitive )
{
	return convert( primitive, IECore::MurmurHash() );
}

AtNode *InstancingConverter::convert( const IECore::Primitive *primitive, const IECore::MurmurHash &additionalHash )
{
	IECore::MurmurHash h = primitive->::IECore::Object::hash();
	h.append( additionalHash );

	MemberData::Cache::accessor a;
	if( m_data->cache.insert( a, h ) )
	{
		a->second = NodeAlgo::convert( primitive );
		return a->second;
	}
	else
	{
		if( a->second )
		{
			AtNode *instance = AiNode( "ginstance" );
			AiNodeSetPtr( instance, "node", a->second );
			return instance;
		}
	}

	return nullptr;
}

AtNode *InstancingConverter::convert( const std::vector<const IECore::Primitive *> &samples, const std::vector<float> &sampleTimes )
{
	return convert( samples, sampleTimes, IECore::MurmurHash() );
}

AtNode *InstancingConverter::convert( const std::vector<const IECore::Primitive *> &samples, const std::vector<float> &sampleTimes, const IECore::MurmurHash &additionalHash )
{
	IECore::MurmurHash h;
	for( std::vector<const IECore::Primitive *>::const_iterator it = samples.begin(), eIt = samples.end(); it != eIt; ++it )
	{
		(*it)->hash( h );
	}
	h.append( additionalHash );

	MemberData::Cache::accessor a;
	if( m_data->cache.insert( a, h ) )
	{
		std::vector<const IECore::Object *> objectSamples( samples.begin(), samples.end() );
		a->second = NodeAlgo::convert( objectSamples, sampleTimes );
		return a->second;
	}
	else
	{
		if( a->second )
		{
			AtNode *instance = AiNode( "ginstance" );
			AiNodeSetPtr( instance, "node", a->second );
			return instance;
		}
	}

	return nullptr;
}
