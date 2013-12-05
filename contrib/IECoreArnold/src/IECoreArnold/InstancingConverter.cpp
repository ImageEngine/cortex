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
#include "IECoreArnold/ToArnoldConverter.h"

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
	IECore::MurmurHash h = primitive->::IECore::Object::hash();
	return convert( primitive, h );
}

AtNode *InstancingConverter::convert( const IECore::Primitive *primitive, const IECore::MurmurHash &hash )
{
	MemberData::Cache::accessor a;
	if( m_data->cache.insert( a, hash ) )
	{
		ToArnoldConverterPtr converter = ToArnoldConverter::create( const_cast<IECore::Primitive *>( primitive ) );
		if( !converter )
		{
			return 0;
		}
		a->second = converter->convert();
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
		
	return 0;


}
