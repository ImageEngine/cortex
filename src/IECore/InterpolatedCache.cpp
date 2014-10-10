//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "tbb/mutex.h"

#include "boost/format.hpp"
#include "boost/bind.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/OversamplesCalculator.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/InterpolatedCache.h"
#include "IECore/CompoundObject.h"
#include "IECore/FileSequence.h"
#include "IECore/EmptyFrameList.h"
#include "IECore/LRUCache.h"

using namespace IECore;
using namespace boost;

//////////////////////////////////////////////////////////////////////////
// Private implementation class
//////////////////////////////////////////////////////////////////////////

class InterpolatedCache::Implementation : public IECore::RefCounted
{

	public :
	
		Implementation( const std::string &pathTemplate, Interpolation interpolation, const OversamplesCalculator &o, size_t maxOpenFiles )
			:	m_cachesForTicks( bind( &Implementation::cachesForTicksGetter, this, _1, _2 ), maxOpenFiles )
		{
			if( pathTemplate.size() )
			{
				setPathTemplate( pathTemplate );
			}
			setInterpolation( interpolation );
			setOversamplesCalculator( o );
		}
	
		void setPathTemplate( const std::string &pathTemplate )
		{
			if ( !m_fileSequence || getPathTemplate() != pathTemplate )
			{
				m_fileSequence = new FileSequence( pathTemplate, new EmptyFrameList() );
				m_cachesForTicks.clear();
			}
		}

		const std::string &getPathTemplate() const
		{
			if( !m_fileSequence )
			{
				// i'd prefer to return "" but that would mean returning a reference
				// to a temporary or having some weird static string instance.
				throw Exception( "Path template has not been set" );
			}
			return m_fileSequence->getFileName();
		}
		
		void setMaxOpenFiles( size_t maxOpenFiles )
		{
			m_cachesForTicks.setMaxCost( maxOpenFiles );
		}
		
		size_t getMaxOpenFiles() const
		{
			return m_cachesForTicks.getMaxCost();
		}
	
		void setInterpolation( Interpolation interpolation )
		{
			m_interpolation = interpolation;
		}

		Interpolation getInterpolation() const
		{
			return m_interpolation;
		}

		void setOversamplesCalculator( const OversamplesCalculator &oc )
		{
			m_oversamplesCalculator = oc;
		}

		const OversamplesCalculator &getOversamplesCalculator() const
		{
			return m_oversamplesCalculator;
		}

		ObjectPtr read( float frame, const ObjectHandle &obj, const AttributeHandle &attr ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			
			assert( numCaches );
			
			ObjectPtr r[4];
			for( int i=0; i<numCaches; i++ )
			{
				/// \todo Can we launch each of these reads in a separate thread?
				CacheAndMutex::Mutex::scoped_lock lock( c[i]->mutex );
				r[i] = c[i]->cache->read( obj, attr );
			}
			
			ObjectPtr result = 0;
			if( numCaches > 1 )
			{
				switch( m_interpolation )
				{
					case Linear :
						assert( numCaches==2 );
						result = linearObjectInterpolation( r[0].get(), r[1].get(), x );
						break;
					case Cubic :
						assert( numCaches==4 );
						result = cubicObjectInterpolation( r[0].get(), r[1].get(), r[2].get(), r[3].get(), x );
						break;
					default :
						assert( false );
				}
			}
			
			if( !result )
			{
				// either there was only one cache, or interpolation failed.
				// in both cases we just return the first sample.
				result = r[0];
			}
			
			assert( result );
			return result;
		}
		
		CompoundObjectPtr read( float frame, const ObjectHandle &obj ) const
		{
			CompoundObjectPtr result = new CompoundObject();

			std::vector<AttributeHandle> attrs;
			attributes( frame, obj, attrs );

			for( std::vector<AttributeHandle>::const_iterator it = attrs.begin(); it != attrs.end(); ++it )
			{
				/// \todo currently each function call is recalculating a bunch of stuff about which caches
				/// to use and wotnot. this could be done once and shared.
				result->members()[ *it ] = read( frame, obj, *it );
			}

			return result;
		}

		ObjectPtr readHeader( float frame, const HeaderHandle &hdr ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			
			ObjectPtr r[4];
			for( int i=0; i<numCaches; i++ )
			{
				/// \todo Can we launch each of these reads in a separate thread?
				CacheAndMutex::Mutex::scoped_lock lock( c[i]->mutex );
				r[i] = c[i]->cache->readHeader( hdr );
			}
			
			ObjectPtr result = 0;
			if( numCaches > 1 )
			{
				switch( m_interpolation )
				{
					case Linear :
						assert( numCaches==2 );
						result = linearObjectInterpolation( r[0].get(), r[1].get(), x );
						break;
					case Cubic :
						assert( numCaches==4 );
						result = cubicObjectInterpolation( r[0].get(), r[1].get(), r[2].get(), r[3].get(), x );
						break;
					default :
						assert( false );
				}
			}
			
			if( !result )
			{
				// either there was only one cache, or interpolation failed.
				// in both cases we just return the first sample.
				result = r[0];
			}
			
			assert( result );
			return result;
		}

		CompoundObjectPtr readHeader( float frame ) const
		{
			CompoundObjectPtr result = new CompoundObject();
			
			std::vector<HeaderHandle> hds;
			headers( frame, hds );

			for( std::vector<HeaderHandle>::const_iterator it = hds.begin(); it != hds.end(); ++it )
			{
				/// \todo currently each function call is recalculating a bunch of stuff about which caches
				/// to use and wotnot. this could be done once and shared.
				result->members()[ *it ] = readHeader( frame, *it );

			}
			return result;
		}

		void objects( float frame, std::vector<ObjectHandle> &objs ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			assert( numCaches ); (void)numCaches;
			CacheAndMutex::Mutex::scoped_lock lock( c[0]->mutex );
			c[0]->cache->objects( objs );
		}

		void attributes( float frame, const ObjectHandle &obj, std::vector<AttributeHandle> &attrs ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			assert( numCaches ); (void)numCaches;
			CacheAndMutex::Mutex::scoped_lock lock( c[0]->mutex );
			c[0]->cache->attributes( obj, attrs );
		}

		void attributes( float frame, const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			assert( numCaches ); (void)numCaches;
			CacheAndMutex::Mutex::scoped_lock lock( c[0]->mutex );
			c[0]->cache->attributes( obj, regex, attrs );
		}

		void headers( float frame, std::vector<HeaderHandle> &hds ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			assert( numCaches ); (void)numCaches;
			CacheAndMutex::Mutex::scoped_lock lock( c[0]->mutex );
			c[0]->cache->headers( hds );
		}
		
		bool contains( float frame, const ObjectHandle &obj ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			assert( numCaches ); (void)numCaches;
			CacheAndMutex::Mutex::scoped_lock lock( c[0]->mutex );
			return c[0]->cache->contains( obj );
		}
		
		bool contains( float frame, const ObjectHandle &obj, const AttributeHandle &attr ) const
		{
			CacheAndMutexPtr c[4]; float x = 0;
			int numCaches = caches( frame, c, x );
			assert( numCaches ); (void)numCaches;
			CacheAndMutex::Mutex::scoped_lock lock( c[0]->mutex );
			return c[0]->cache->contains( obj, attr );
		}

	private :
	
		FileSequencePtr m_fileSequence;
		Interpolation m_interpolation;
		OversamplesCalculator m_oversamplesCalculator;
			
		// a class to store an AttributeCache and a mutex
		// that must be used to access it.
		class CacheAndMutex : public IECore::RefCounted
		{
			public :

				typedef tbb::mutex Mutex;
				Mutex mutex;	
				AttributeCachePtr cache;

		};
		
		IE_CORE_DECLAREPTR( CacheAndMutex );

		// an lru cache mapping from ticks to attribute caches

		CacheAndMutexPtr cachesForTicksGetter( const int &tick, size_t &cost )
		{			
			if( !m_fileSequence )
			{
				throw Exception( "Path template has not been set" );
			}
			
			std::string fileName = m_fileSequence->fileNameForFrame( tick );
			CacheAndMutexPtr result = new CacheAndMutex;
			result->cache = new AttributeCache( fileName, IndexedIO::Read );
			return result;
		}
		
		typedef LRUCache<int, CacheAndMutexPtr> CachesForTicks;
		mutable CachesForTicks m_cachesForTicks;
		
		// function to find the relevant caches for a given frame and return
		// them along with an interpolation type and factor. returns the number
		// of caches found. note that this may be 1 even if interpolation has
		// been requested, as the frame may coincide directly with a file cache
		// and not need interpolating.
		
		int caches( float frame, CacheAndMutexPtr c[4], float &interpolationFactor ) const
		{
		
			int lowTick, highTick;
			interpolationFactor = m_oversamplesCalculator.tickInterval( frame, lowTick, highTick );

			float step = (float)m_oversamplesCalculator.getTicksPerSecond() / m_oversamplesCalculator.getSamplesPerFrame() / m_oversamplesCalculator.getFrameRate();

			int start = 0;
			int end = 0;
			if( interpolationFactor == 0.0f || m_interpolation == None )
			{
				start = 0;
				end = 0;
			}
			else
			{
				switch( m_interpolation )
				{
					case Linear:
						// Need one file ahead
						start = 0;
						end = 1;
						break;
					case Cubic:
						// Need one file behind, 2 files ahead
						start = -1;
						end = 2;
						break;
					default:
						assert( false );
				}
			}

			int cacheIndex = 0;
			for( int fileNum = start; fileNum <= end; fileNum++, cacheIndex++ )
			{				
				int tick = m_oversamplesCalculator.nearestTick(( int )( lowTick + fileNum * step ) );				
				c[cacheIndex] = m_cachesForTicks.get( tick );
			}

			assert( cacheIndex );
			return cacheIndex;
		}
	

};

//////////////////////////////////////////////////////////////////////////
// InterpolatedCache class
//////////////////////////////////////////////////////////////////////////

InterpolatedCache::InterpolatedCache( const std::string &pathTemplate, Interpolation interpolation,  const OversamplesCalculator &o, size_t maxOpenFiles )
	:	m_implementation( new Implementation( pathTemplate, interpolation, o, maxOpenFiles ) )
{
}

InterpolatedCache::~InterpolatedCache()
{
}

void InterpolatedCache::setPathTemplate( const std::string &pathTemplate )
{
	m_implementation->setPathTemplate( pathTemplate );
}

const std::string &InterpolatedCache::getPathTemplate() const
{
	return m_implementation->getPathTemplate();
}

void InterpolatedCache::setMaxOpenFiles( size_t maxOpenFiles )
{
	m_implementation->setMaxOpenFiles( maxOpenFiles );
}

size_t InterpolatedCache::getMaxOpenFiles() const
{
	return m_implementation->getMaxOpenFiles();
}

void InterpolatedCache::setInterpolation( InterpolatedCache::Interpolation interpolation )
{
	m_implementation->setInterpolation( interpolation );
}

InterpolatedCache::Interpolation InterpolatedCache::getInterpolation() const
{
	return m_implementation->getInterpolation();
}

void InterpolatedCache::setOversamplesCalculator( const OversamplesCalculator &oc )
{
	m_implementation->setOversamplesCalculator( oc );
}

const OversamplesCalculator &InterpolatedCache::getOversamplesCalculator() const
{
	return m_implementation->getOversamplesCalculator();
}

ObjectPtr InterpolatedCache::read( float frame, const ObjectHandle &obj, const AttributeHandle &attr ) const
{
	return m_implementation->read( frame, obj, attr );
}

CompoundObjectPtr InterpolatedCache::read( float frame, const ObjectHandle &obj ) const
{
	return m_implementation->read( frame, obj );
}

ObjectPtr InterpolatedCache::readHeader( float frame, const HeaderHandle &hdr ) const
{
	return m_implementation->readHeader( frame, hdr );
}

CompoundObjectPtr InterpolatedCache::readHeader( float frame ) const
{
	return m_implementation->readHeader( frame );
}

void InterpolatedCache::objects( float frame, std::vector<ObjectHandle> &objs ) const
{
	return m_implementation->objects( frame, objs );
}

void InterpolatedCache::headers( float frame, std::vector<HeaderHandle> &hds ) const
{
	return m_implementation->headers( frame, hds );
}

void InterpolatedCache::attributes( float frame, const ObjectHandle &obj, std::vector<AttributeHandle> &attrs ) const
{
	return m_implementation->attributes( frame, obj, attrs );
}

void InterpolatedCache::attributes( float frame, const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs ) const
{
	return m_implementation->attributes( frame, obj, regex, attrs );
}

bool InterpolatedCache::contains( float frame, const ObjectHandle &obj ) const
{
	return m_implementation->contains( frame, obj );
}

bool InterpolatedCache::contains( float frame, const ObjectHandle &obj, const AttributeHandle &attr ) const
{
	return m_implementation->contains( frame, obj, attr );
}
