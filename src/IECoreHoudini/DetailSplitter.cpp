//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "GU/GU_Detail.h"

#include "IECoreHoudini/DetailSplitter.h"

using namespace IECoreHoudini;

DetailSplitter::DetailSplitter( const GU_DetailHandle &handle, const std::string &key )
	: m_lastMetaCount( -1 ), m_key( key ), m_handle( handle )
{
}

DetailSplitter::~DetailSplitter()
{
}

const GU_DetailHandle &DetailSplitter::handle() const
{
	return m_handle;
}

const GU_DetailHandle DetailSplitter::split( const std::string &value )
{
	if ( !validate() )
	{
		return GU_DetailHandle();
	}
	
	Cache::const_iterator it = m_cache.find( value );
	if ( it != m_cache.end() )
	{
		return it->second;
	}
	
	return GU_DetailHandle();
}

bool DetailSplitter::validate()
{
	GU_DetailHandleAutoReadLock readHandle( m_handle );
	const GU_Detail *geo = readHandle.getGdp();
	if ( !geo )
	{
		return false;
	}
	
	if ( geo->getMetaCacheCount() == m_lastMetaCount )
	{
		return true;
	}
	
	m_cache.clear();
	m_lastMetaCount = geo->getMetaCacheCount();
	
	GA_ROAttributeRef attrRef = geo->findStringTuple( GA_ATTRIB_PRIMITIVE, m_key.c_str() );
	if ( !attrRef.isValid() )
	{
		m_cache[""] = m_handle;
		return true;
	}
	
	const GA_Attribute *attr = attrRef.getAttribute();
	const GA_AIFSharedStringTuple *tuple = attr->getAIFSharedStringTuple();
	
	std::map<GA_StringIndexType, GA_OffsetList> offsets;
	GA_Range primRange = geo->getPrimitiveRange();
	for ( GA_Iterator it = primRange.begin(); !it.atEnd(); ++it )
	{
		GA_StringIndexType currentHandle = tuple->getHandle( attr, it.getOffset() );
		
		std::map<GA_StringIndexType, GA_OffsetList>::iterator oIt = offsets.find( currentHandle );
		if ( oIt == offsets.end() )
		{
			oIt = offsets.insert( std::pair<GA_StringIndexType, GA_OffsetList>( currentHandle, GA_OffsetList() ) ).first;
		}
		
		oIt->second.append( it.getOffset() );
	}
	
	for ( std::map<GA_StringIndexType, GA_OffsetList>::iterator oIt = offsets.begin(); oIt != offsets.end(); ++oIt )
	{
		GU_Detail *newGeo = new GU_Detail();
		GA_Range matchPrims( geo->getPrimitiveMap(), oIt->second );
		newGeo->mergePrimitives( *geo, matchPrims );
		newGeo->incrementMetaCacheCount();
		
		GU_DetailHandle handle;
		handle.allocateAndSet( newGeo, true );
		
		std::string current = "";
		if ( const char *value = tuple->getTableString( attr, oIt->first ) )
		{
			current = value;
		}
		
		m_cache[current] = handle;
	}
	
	return !m_cache.empty();
}

void DetailSplitter::values( std::vector<std::string> &result )
{
	/// \todo: do we really want this method to create the cache? should it just look at the names instead?
	if ( !validate() )
	{
		return;
	}
	
	result.clear();
	result.reserve( m_cache.size() );
	
	for ( Cache::iterator it = m_cache.begin(); it != m_cache.end(); ++it )
	{
		result.push_back( it->first );
	}
}
