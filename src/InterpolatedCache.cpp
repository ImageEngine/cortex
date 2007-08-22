//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MessageHandler.h"
#include "IECore/OversamplesCalculator.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/InterpolatedCache.h"
#include "IECore/CompoundObject.h"


using namespace IECore;
using namespace boost;

InterpolatedCache::InterpolatedCache( const std::string &pathTemplate, double frame, Interpolation interpolation, 
							int oversamples, double frameRate ) :
					m_pathTemplate( pathTemplate ), m_frameRate( frameRate ), m_oversamples( oversamples ), m_interpolation( interpolation ), m_frame( frame ), m_parametersChanged( true )
{
}

InterpolatedCache::~InterpolatedCache()
{
	closeCacheFiles();
}

void InterpolatedCache::setPathTemplate( const std::string &pathTemplate )
{
	if ( pathTemplate == m_pathTemplate )
	{
		return;
	}
	m_pathTemplate = pathTemplate;
	closeCacheFiles();
}

std::string InterpolatedCache::getPathTemplate()
{
	return m_pathTemplate;
}

void InterpolatedCache::setFrame( double frame )
{
	m_frame = frame;
	m_parametersChanged = true;
}

double InterpolatedCache::getFrame()
{
	return m_frame;
}

void InterpolatedCache::setInterpolation( InterpolatedCache::Interpolation interpolation )
{
	m_interpolation = interpolation;
	m_parametersChanged = true;
}

InterpolatedCache::Interpolation InterpolatedCache::getInterpolation()
{
	return m_interpolation;
}

void InterpolatedCache::setOversamples( int oversamples )
{
	m_oversamples = oversamples;
	m_parametersChanged = true;
}

int InterpolatedCache::getOversamples()
{
	return m_oversamples;
}

void InterpolatedCache::setFrameRate( double frameRate )
{
	m_frameRate = frameRate;
	m_parametersChanged = true;
}

double InterpolatedCache::getFrameRate()
{
	return m_frameRate;
}

ObjectPtr InterpolatedCache::read( const ObjectHandle &obj, const AttributeHandle &attr )
{
	updateCacheFiles();
	
	ObjectPtr data;
	bool interpolationFailed = false;

	if ( m_useInterpolation )
	{
		std::vector< ObjectPtr > pts;
		for (unsigned int c = 0; c < m_caches.size(); c++)
		{
			try
			{
				data = m_caches[c]->read( obj, attr );
			}
			catch (IECore::Exception &e)
			{
				break;
			}
			if ( !data )
			{
				break;
			}
			pts.push_back( data );
		}

		if ( pts.size() != m_caches.size() )
		{
			interpolationFailed = true;
		}
		else
		{
			switch (m_interpolation)
			{
				case Linear:
				{
					data = linearObjectInterpolation( pts[0], pts[1], m_x);
					break;
				}
				case Cosine:
				{
					data = cosineObjectInterpolation( pts[0], pts[1], m_x);
					break;
				}
				case Cubic:
				{
					data = cubicObjectInterpolation(pts[0], pts[1], pts[2], pts[3], m_x );
					break;
				}
				default:
					data = 0;
					break;
			}
			if (! data )
			{
				data = pts[ m_curFrameIndex ];
			}
		}
	}
	if ( !m_useInterpolation || interpolationFailed )
	{
		try
		{
			data = m_caches[ m_curFrameIndex ]->read( obj, attr );
		}
		catch (IECore::Exception &e)
		{
			std::string err = (format( "Could not load attribute %s from object %s: %s" ) % obj % attr % e.what() ).str();
			throw IOException( err );
		}
		if ( !data )
		{
			throw IOException( (format( "Could not load attribute %s from object %s." ) % obj % attr).str() );
		}
	}
	return data;
}

CompoundObjectPtr InterpolatedCache::read( const ObjectHandle &obj )
{
	CompoundObjectPtr dict = new CompoundObject();
	std::vector<AttributeHandle> attrs;
	attributes( obj, attrs );

	for (std::vector<AttributeHandle>::const_iterator it = attrs.begin(); it != attrs.end(); ++it)
	{
		ObjectPtr data = read( obj, *it );

		dict->members()[ *it ] = data;

	}
	return dict;
}

ObjectPtr InterpolatedCache::readHeader( const HeaderHandle &hdr )
{
	updateCacheFiles();	
	return m_caches[ m_curFrameIndex ]->readHeader( hdr );
}

CompoundObjectPtr InterpolatedCache::readHeader( )
{
	updateCacheFiles();	
	return m_caches[ m_curFrameIndex ]->readHeader();
}
	
void InterpolatedCache::objects(std::vector<ObjectHandle> &objs)
{
	updateCacheFiles();	
	m_caches[ m_curFrameIndex ]->objects( objs );
}

void InterpolatedCache::headers(std::vector<HeaderHandle> &hds)
{
	updateCacheFiles();	
	m_caches[ m_curFrameIndex ]->headers( hds );
}

void InterpolatedCache::attributes(const ObjectHandle &obj, std::vector<AttributeHandle> &attrs)
{
	updateCacheFiles();	
	m_caches[ m_curFrameIndex ]->attributes( obj, attrs );
}
		
void InterpolatedCache::attributes(const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs)
{
	updateCacheFiles();	
	m_caches[ m_curFrameIndex ]->attributes( obj, regex, attrs );
}

bool InterpolatedCache::contains( const ObjectHandle &obj )
{
	updateCacheFiles();	
	return m_caches[ m_curFrameIndex ]->contains( obj );
}
		
bool InterpolatedCache::contains( const ObjectHandle &obj, const AttributeHandle &attr )
{
	updateCacheFiles();	
	return m_caches[ m_curFrameIndex ]->contains( obj, attr );
}

void InterpolatedCache::updateCacheFiles()
{
	if ( !m_parametersChanged )
	{
		return;
	}

	OversamplesCalculator6kFPS oversamplesCalc( m_frameRate, m_oversamples );
	int curTime = oversamplesCalc.frameToTime(m_frame);
	int step = oversamplesCalc.stepSize();
	double x = oversamplesCalc.relativeStepOffset( curTime );
	int f = oversamplesCalc.stepRound( curTime );

	m_useInterpolation = true;

	int start = 0;
	int end = 0;
	int curFrameIndex = 0;

	if (x == 0.)
	{
		m_useInterpolation = false;
		start = 0;
		end = 0;
		curFrameIndex = 0;
	}
	else
	{	
		switch (m_interpolation)
		{
			case None:
				// No additional frames needed
				start = 0;
				end = 0;
				curFrameIndex = 0;
				m_useInterpolation = false;
				break;
			case Linear:
			case Cosine:
				// Need one frame ahead
				start = 0;
				end = 1;
				curFrameIndex = 0;
				break;
			case Cubic:
				// Need one frame behind, 2 frames ahead
				start = -1;
				end = 2;
				curFrameIndex = 1;
				break;
			default:
				throw Exception("Invalid interpolation method!");
		}
	}

	CacheVector caches;
	std::vector< std::string > cacheFiles;
	AttributeCachePtr cache;
	// Open all the cache files required to perform interpolation
	for (int fileNum = start; fileNum <= end; fileNum ++)
	{
		std::string fullpath = (format( m_pathTemplate ) % (f + fileNum * step)).str();

		cache = 0;
		for ( unsigned i = 0; i < m_caches.size(); i++)
		{
			if ( fullpath == m_cacheFiles[i] )
			{
				cache = m_caches[i];
				break;
			}
		}
		if ( !cache )
		{
			cache = new AttributeCache( fullpath, IndexedIO::Read );
		}
		cacheFiles.push_back( fullpath );
		caches.push_back( cache );
	}

	// save info in the object.
	m_x = x;
	m_curFrameIndex = curFrameIndex;
	m_caches = caches;
	m_cacheFiles = cacheFiles;
}

void InterpolatedCache::closeCacheFiles()
{
	m_parametersChanged = true;
	m_caches.clear();
	m_cacheFiles.clear();
}
