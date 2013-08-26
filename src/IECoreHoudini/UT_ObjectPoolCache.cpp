//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/UT_ObjectPoolCache.h"

using namespace IECoreHoudini;

UT_ObjectPoolCache::UT_ObjectPoolCache( const std::string &name, IECore::ObjectPoolPtr objectPool )
	: m_name( name ), m_pool( objectPool )
{
}

UT_ObjectPoolCache::~UT_ObjectPoolCache()
{
}

const char *UT_ObjectPoolCache::utGetCacheName() const
{
	return m_name.c_str();
}

int64 UT_ObjectPoolCache::utGetCurrentSize() const
{
	return m_pool->memoryUsage();
}

int64 UT_ObjectPoolCache::utReduceCacheSizeBy( int64 amount )
{
	size_t begin = m_pool->memoryUsage();
	size_t max = m_pool->getMaxMemoryUsage();
	m_pool->setMaxMemoryUsage( begin - amount );
	m_pool->setMaxMemoryUsage( max );
	
	return begin - m_pool->memoryUsage();
}

bool UT_ObjectPoolCache::utHasMaxSize() const
{
	return true;
}

int64 UT_ObjectPoolCache::utGetMaxSize() const
{
	return m_pool->getMaxMemoryUsage();
}

void UT_ObjectPoolCache::utSetMaxSize( int64 amount )
{
	m_pool->setMaxMemoryUsage( amount );
}

bool UT_ObjectPoolCache::utHasMinSize() const
{
	return false;
}

UT_ObjectPoolCache *UT_ObjectPoolCache::defaultObjectPoolCache()
{
	static UT_ObjectPoolCache *defaultCache = 0;
	
	if ( !defaultCache )
	{
		defaultCache = new UT_ObjectPoolCache( "Cortex Object Pool", IECore::ObjectPool::defaultObjectPool() );
	}
	
	return defaultCache;
}
