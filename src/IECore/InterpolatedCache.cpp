//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/OversamplesCalculator.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/InterpolatedCache.h"
#include "IECore/CompoundObject.h"
#include "IECore/FileSequence.h"
#include "IECore/EmptyFrameList.h"

using namespace IECore;
using namespace boost;

InterpolatedCache::InterpolatedCache( const std::string &pathTemplate, float frame, Interpolation interpolation, const OversamplesCalculator &o )
		: m_fileSequence( 0 ), m_oversamplesCalculator( o )
{
	setPathTemplate( pathTemplate );
	setFrame( frame );
	setInterpolation( interpolation );

	m_parametersChanged = true;
}

InterpolatedCache::~InterpolatedCache()
{
	closeCacheFiles();
}

void InterpolatedCache::setPathTemplate( const std::string &pathTemplate )
{
	if ( !m_fileSequence || getPathTemplate() != pathTemplate )
	{
		m_fileSequence = new FileSequence( pathTemplate, new EmptyFrameList() );
		m_parametersChanged = true;
		closeCacheFiles();
	}
}

const std::string &InterpolatedCache::getPathTemplate() const
{
	assert( m_fileSequence );
	return m_fileSequence->getFileName();
}

void InterpolatedCache::setFrame( float frame )
{
	if ( frame != m_frame )
	{
		m_frame = frame;
		m_parametersChanged = true;
	}
}

float InterpolatedCache::getFrame() const
{
	return m_frame;
}

void InterpolatedCache::setInterpolation( InterpolatedCache::Interpolation interpolation )
{
	if ( interpolation != m_interpolation )
	{
		m_interpolation = interpolation;
		m_parametersChanged = true;
	}
}

InterpolatedCache::Interpolation InterpolatedCache::getInterpolation() const
{
	return m_interpolation;
}

void InterpolatedCache::setOversamplesCalculator( const OversamplesCalculator &oc )
{
	m_oversamplesCalculator = oc;
}

OversamplesCalculator InterpolatedCache::getOversamplesCalculator() const
{
	return m_oversamplesCalculator;
}


ObjectPtr InterpolatedCache::read( const ObjectHandle &obj, const AttributeHandle &attr ) const
{
	updateCacheFiles();

	ObjectPtr data;

	if ( m_useInterpolation )
	{
		std::vector< ObjectPtr > pts;
		for ( unsigned int c = 0; c < m_caches.size(); c++ )
		{
			data = m_caches[c]->read( obj, attr );
			assert( data );
			pts.push_back( data );
		}
		assert( pts.size() == m_caches.size() );

		switch ( m_interpolation )
		{
		case Linear:
		{
			assert( pts.size() == 2 );
			data = linearObjectInterpolation( pts[0], pts[1], m_x );

			break;
		}
		case Cubic:
		{
			assert( pts.size() == 4 );
			data = cubicObjectInterpolation( pts[0], pts[1], pts[2], pts[3], m_x );
			break;
		}
		default:
			assert( false );
		}

		if ( !data )
		{
			data = pts[ m_curFrameIndex ];
		}
	}
	else
	{
		data = m_caches[ m_curFrameIndex ]->read( obj, attr );
	}

	assert( data );
	return data;
}

CompoundObjectPtr InterpolatedCache::read( const ObjectHandle &obj ) const
{
	CompoundObjectPtr dict = new CompoundObject();
	std::vector<AttributeHandle> attrs;
	attributes( obj, attrs );

	for ( std::vector<AttributeHandle>::const_iterator it = attrs.begin(); it != attrs.end(); ++it )
	{
		ObjectPtr data = read( obj, *it );

		dict->members()[ *it ] = data;
	}

	return dict;
}

ObjectPtr InterpolatedCache::readHeader( const HeaderHandle &hdr ) const
{
	updateCacheFiles();

	ObjectPtr data;

	if ( m_useInterpolation )
	{
		std::vector< ObjectPtr > pts;
		for ( unsigned int c = 0; c < m_caches.size(); c++ )
		{
			data = m_caches[c]->readHeader( hdr );
			assert( data );
			pts.push_back( data );
		}
		assert( pts.size() == m_caches.size() );

		switch ( m_interpolation )
		{
		case Linear:
		{
			assert( pts.size() == 2 );
			data = linearObjectInterpolation( pts[0], pts[1], m_x );
			break;
		}
		case Cubic:
		{
			assert( pts.size() == 4 );
			data = cubicObjectInterpolation( pts[0], pts[1], pts[2], pts[3], m_x );
			break;
		}
		default:
			assert( false );
		}

		if ( !data )
		{
			data = pts[ m_curFrameIndex ];
		}
	}
	else
	{
		data = m_caches[ m_curFrameIndex ]->readHeader( hdr );
	}

	assert( data );
	return data;
}

CompoundObjectPtr InterpolatedCache::readHeader( ) const
{
	updateCacheFiles();

	CompoundObjectPtr dict = new CompoundObject();
	std::vector<HeaderHandle> hds;
	headers( hds );

	for ( std::vector<HeaderHandle>::const_iterator it = hds.begin(); it != hds.end(); ++it )
	{
		ObjectPtr data = readHeader( *it );

		dict->members()[ *it ] = data;

	}
	return dict;
}

void InterpolatedCache::objects( std::vector<ObjectHandle> &objs ) const
{
	updateCacheFiles();
	assert( m_caches.size() > m_curFrameIndex );
	m_caches[ m_curFrameIndex ]->objects( objs );
}

void InterpolatedCache::headers( std::vector<HeaderHandle> &hds ) const
{
	updateCacheFiles();
	assert( m_caches.size() > m_curFrameIndex );
	m_caches[ m_curFrameIndex ]->headers( hds );
}

void InterpolatedCache::attributes( const ObjectHandle &obj, std::vector<AttributeHandle> &attrs ) const
{
	updateCacheFiles();
	assert( m_caches.size() > m_curFrameIndex );
	m_caches[ m_curFrameIndex ]->attributes( obj, attrs );
}

void InterpolatedCache::attributes( const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs ) const
{
	updateCacheFiles();
	assert( m_caches.size() > m_curFrameIndex );
	m_caches[ m_curFrameIndex ]->attributes( obj, regex, attrs );
}

bool InterpolatedCache::contains( const ObjectHandle &obj ) const
{
	updateCacheFiles();
	assert( m_caches.size() > m_curFrameIndex );
	return m_caches[ m_curFrameIndex ]->contains( obj );
}

bool InterpolatedCache::contains( const ObjectHandle &obj, const AttributeHandle &attr ) const
{
	updateCacheFiles();
	assert( m_caches.size() > m_curFrameIndex );
	return m_caches[ m_curFrameIndex ]->contains( obj, attr );
}

void InterpolatedCache::updateCacheFiles() const
{
	if ( !m_parametersChanged )
	{
		return;
	}

	int lowTick, highTick;
	float x = m_oversamplesCalculator.tickInterval( m_frame, lowTick, highTick );

	float step = ( float )m_oversamplesCalculator.getTicksPerSecond() / m_oversamplesCalculator.getSamplesPerFrame() / m_oversamplesCalculator.getFrameRate() ;

	int start = 0;
	int end = 0;

	if ( x == 0. || m_interpolation == None )
	{
		m_useInterpolation = false;
		start = 0;
		end = 0;
		m_curFrameIndex = 0;
	}
	else
	{
		m_useInterpolation = true;
		switch ( m_interpolation )
		{
		case Linear:
			// Need one file ahead
			m_curFrameIndex = 0;
			start = 0;
			end = 1;
			break;
		case Cubic:
			// Need one file behind, 2 files ahead
			m_curFrameIndex = 1;
			start = -1;
			end = 2;
			break;
		default:
			assert( false );
		}
	}

	CacheVector caches;
	std::vector< std::string > cacheFiles;
	AttributeCachePtr cache;
	// Open all the cache files required to perform interpolation
	for ( int fileNum = start; fileNum <= end; fileNum ++ )
	{
		int tick = m_oversamplesCalculator.nearestTick(( int )( lowTick + fileNum * step ) );
		std::string fileName = m_fileSequence->fileNameForFrame( tick );

		cache = 0;
		for ( unsigned i = 0; i < m_caches.size(); i++ )
		{
			if ( fileName == m_cacheFiles[i] )
			{
				cache = m_caches[i];
				break;
			}
		}
		if ( !cache )
		{
			cache = new AttributeCache( fileName, IndexedIO::Read );
		}

		assert( cache );
		cacheFiles.push_back( fileName );
		caches.push_back( cache );
	}

	// save info in the object.
	m_x = x;
	m_caches = caches;
	m_cacheFiles = cacheFiles;
	assert( m_caches.size() == m_cacheFiles.size() );
	assert( m_caches.size() > m_curFrameIndex );
	m_parametersChanged = false;
}

void InterpolatedCache::closeCacheFiles() const
{
	m_parametersChanged = true;
	m_caches.clear();
	m_cacheFiles.clear();
}
