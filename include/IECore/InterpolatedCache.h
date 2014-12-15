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

#ifndef IECORE_INTERPOLATEDCACHE_H
#define IECORE_INTERPOLATEDCACHE_H

#include "IECore/Export.h"
#include "IECore/AttributeCache.h"
#include "IECore/OversamplesCalculator.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( FileSequence );

/// Provides higher level access to cache files by automatically interpolating data from multiple files.
/// Or returns the data from the nearest frame if the data cannot be interpolated.
/// The interface looks like AttributeCache reader functions.
/// \threading This class provides limited thread safety. The methods which specify the caches
/// to be read are not safe to call while other threads are operating on the object. However, once
/// the caches have been specified it is safe to call the read methods from multiple concurrent threads and
/// with multiple different frame arguments. See the documentation of the individual methods for more details.
/// \todo It might be great to pass interpolation and oversamples calculator to each read method rather
/// than have them store as state. This would allow different interpolation and oversampling per call and per thread.
/// If we did this I think we should look at replacing the OversamplesCalculator class with some more sensible
/// Time or TimeSampler class, and passing everything in one argument.
/// \ingroup ioGroup
class IECORE_API InterpolatedCache : public RefCounted
{
	public :
		
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

		/// Constructor
		/// pathTemplate must be a valid FileSequence filename specifier, e.g. "myCacheFile.####.cob"
		InterpolatedCache(
			const std::string &pathTemplate = "",
			Interpolation interpolation = None,
			const OversamplesCalculator &o = OversamplesCalculator(),
			size_t maxOpenFiles = 10
		);
		
		~InterpolatedCache();

		/// Changes the path template for cache files.
		/// \threading It is not safe to call this method while other threads are accessing
		/// this object.
		void setPathTemplate( const std::string &pathTemplate );
		/// Returns the current path template used to open cache files.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		const std::string &getPathTemplate() const;

		/// Sets the maximum number of caches this class will keep open at one time.
		/// \threading It is not safe to call this method while other threads are accessing
		/// this object.
		void setMaxOpenFiles( size_t maxOpenFiles );
		/// Returns the maximum number of caches this class will keep open at one time.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		size_t getMaxOpenFiles() const;

		/// Sets the interpolation method.
		/// \threading It is not safe to call this method while other threads are accessing
		/// this object.
		void setInterpolation( Interpolation interpolation );
		/// Returns the current interpolation method.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		Interpolation getInterpolation() const;
		
		/// Sets the OversamplesCalculator.
		/// \threading It is not safe to call this method while other threads are accessing
		/// this object.
		void setOversamplesCalculator( const OversamplesCalculator & );

		/// Returns the current OversamplesCalculator.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		const OversamplesCalculator &getOversamplesCalculator() const;

		/// Read a piece of data associated with the specified object and attribute from the cache.
		/// Throws an exception if the requested data is not present in the cache or if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		ObjectPtr read( float frame, const ObjectHandle &obj, const AttributeHandle &attr ) const;
		
		/// Read a piece of data associated with the specified object from the cache.
		/// Returns a CompoundObject with attribute as keys.
		/// Throws an exception if the requested data is not present in the cache or if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		CompoundObjectPtr read( float frame, const ObjectHandle &obj ) const;

		/// Read data associated with the specified header from the open cache files.
		/// The result will be interpolated whenever possible. Objects not existent in
		/// every opened file will not be interpolated and will be returned if they come from the nearest frame.
		/// Throws an exception if the requested header is not present in the cache or if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		ObjectPtr readHeader( float frame, const HeaderHandle &hdr ) const;

		/// Creates a CompoundObject with the header names as keys.
		/// Read all header data present in the open cache files. The result will be
		/// interpolated whenever possible. Objects not existent in every opened file will not be interpolated and
		/// will be returned if they come from the nearest frame.
		/// Throws an exception if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		CompoundObjectPtr readHeader( float frame ) const;
		
		/// Retrieve the list of object handles from the cache
		/// Throws an exception if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		void objects( float frame, std::vector<ObjectHandle> &objs ) const;
		
		/// Retrieve the list of header handles from the cache (from the nearest frame).
		/// Throws an exception if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		void headers( float frame, std::vector<HeaderHandle> &hds ) const;

		/// Retrieve the list of attribute handles from the specified objects.
		/// Throws an exception if the object is not within the cache or the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		void attributes( float frame, const ObjectHandle &obj, std::vector<AttributeHandle> &attrs ) const;
		
		/// Retrieve the list of attribute handles that match the given regex from the specified objects.
		/// Throws an exception if the object is not within the cache or the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		void attributes( float frame, const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs ) const;

		/// Determines whether or not the cache contains the specified object
		/// Throws an exception if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		bool contains( float frame, const ObjectHandle &obj ) const;

		/// Determines whether or not the cache contains the specified object and attribute
		/// Throws an exception if the cache file is not found.
		/// \threading It is safe to call this method while other threads are calling const
		/// methods of this class.
		bool contains( float frame, const ObjectHandle &obj, const AttributeHandle &attr ) const;

	private :

		IE_CORE_FORWARDDECLARE( Implementation );
		ImplementationPtr m_implementation;
		
};

IE_CORE_DECLAREPTR( InterpolatedCache );

} // namespace IECore

#endif //  IECORE_INTERPOLATEDCACHE_H
