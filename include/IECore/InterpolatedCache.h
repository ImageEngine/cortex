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

#ifndef IECORE_INTERPOLATEDCACHE_H
#define IECORE_INTERPOLATEDCACHE_H

#include "IECore/AttributeCache.h"
#include "IECore/OversamplesCalculator.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( FileSequence );

///Provides higher level access to cache files by automatically interpolating data from multiple files.
///Or returns the data from the nearest frame if the data cannot be interpolated.
///The interface looks like AttributeCache reader functions.
class InterpolatedCache : public RefCounted
{
	public:

		typedef std::vector< AttributeCachePtr > CacheVector;
		typedef IECore::AttributeCache::ObjectHandle ObjectHandle;
		typedef IECore::AttributeCache::HeaderHandle HeaderHandle;
		typedef IECore::AttributeCache::AttributeHandle AttributeHandle;

		IE_CORE_DECLAREMEMBERPTR( InterpolatedCache );

		enum Interpolation
		{
			None = 0,
			Linear,
			Cubic
		};

		///Constructor
		///pathTemplate must be a valid FileSequence filename specifier, e.g. "myCacheFile.####.cob"
		InterpolatedCache(
		        const std::string &pathTemplate = "",
		        float frame = 0.0,
		        Interpolation interpolation = None,
		        const OversamplesCalculator & = OversamplesCalculator()
		);

		~InterpolatedCache();

		///Changes the path template for cache files.
		// It will not try to load the new files until the read methods are called.
		void setPathTemplate( const std::string &pathTemplate );

		///Returns the current path template used to open cache files.
		const std::string &getPathTemplate() const;

		///Advances in time.
		// It will not try to load the new files until the read methods are called.
		void setFrame( float frame );

		///Returns the current frame.
		float getFrame() const;

		///Sets the interpolation method.
		// It will not try to load the new files until the read methods are called.
		void setInterpolation( Interpolation interpolation );

		///Returns the current interpolation method.
		Interpolation getInterpolation() const;

		///Read a piece of data associated with the specified object and attribute from the cache.
		///Throws an exception if the requested data is not present in the cache or if the cache file is not found.
		ObjectPtr read( const ObjectHandle &obj, const AttributeHandle &attr ) const;

		///Read a piece of data associated with the specified object from the cache.
		///Returns a CompoundObject with attribute as keys.
		///Throws an exception if the requested data is not present in the cache or if the cache file is not found.
		CompoundObjectPtr read( const ObjectHandle &obj ) const;

		///Read data associated with the specified header from the open cache files.
		///The result will be interpolated whenever possible. Objects not existent in
		///every opened file will not be interpolated and will be returned if they come from the nearest frame.
		///Throws an exception if the requested header is not present in the cache or if the cache file is not found.
		ObjectPtr readHeader( const HeaderHandle &hdr ) const;

		///Creates a CompoundObject with the header names as keys.
		///Read all header data present in the open cache files. The result will be
		///interpolated whenever possible. Objects not existent in every opened file will not be interpolated and
		///will be returned if they come from the nearest frame.
		///Throws an exception if the cache file is not found.
		CompoundObjectPtr readHeader( ) const;

		///Retrieve the list of object handles from the cache
		///Throws an exception if the cache file is not found.
		void objects( std::vector<ObjectHandle> &objs ) const;

		///Retrieve the list of header handles from the cache (from the nearest frame).
		///Throws an exception if the cache file is not found.
		void headers( std::vector<HeaderHandle> &hds ) const;

		///Retrieve the list of attribute handles from the specified objects.
		// Throws an exception if the object is not within the cache or the cache file is not found.
		void attributes( const ObjectHandle &obj, std::vector<AttributeHandle> &attrs ) const;

		///Retrieve the list of attribute handles that match the given regex from the specified objects.
		// Throws an exception if the object is not within the cache or the cache file is not found.
		void attributes( const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs ) const;

		///Determines whether or not the cache contains the specified object
		// Throws an exception if the cache file is not found.
		bool contains( const ObjectHandle &obj ) const;

		///Determines whether or not the cache contains the specified object and attribute
		// Throws an exception if the cache file is not found.
		bool contains( const ObjectHandle &obj, const AttributeHandle &attr ) const;

	protected:

		///Close old cache files, reuse or open other cache files acoording to the current parameters.
		///If they are opened already, do nothing. This function is used all around for lazy file opening.
		void updateCacheFiles() const;

		///Close all cache files
		void closeCacheFiles() const;

	protected:

		FileSequencePtr m_fileSequence;
		Interpolation m_interpolation;
		float m_frame;
		OversamplesCalculator m_oversamplesCalculator;

		mutable bool m_parametersChanged;

		unsigned m_curFrameIndex;
		mutable bool m_useInterpolation;
		mutable float m_x;

		mutable CacheVector m_caches;
		mutable std::vector< std::string> m_cacheFiles;



};

IE_CORE_DECLAREPTR( InterpolatedCache );

} // namespace IECore

#endif //  IECORE_INTERPOLATEDCACHE_H
