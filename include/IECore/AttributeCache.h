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

#ifndef IE_CORE_ATTRIBUTECACHE_H
#define IE_CORE_ATTRIBUTECACHE_H

#include <string>
#include <vector>
#include <set>

#include "IECore/Export.h"
#include "IECore/Object.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( CompoundObject );

/// A simple means of creating and reading caches of data values which are associated with
/// notional "Objects" and "Attributes". Will throw an exception derived from IECore::Exception if
/// any errors are encountered.
/// \threading It is not safe to use an instance of this class from multiple concurrent threads. See
/// the InterpolatedCache class for a threadsafe means of reading the files with automatic
/// interpolation.
/// \ingroup ioGroup
class IECORE_API AttributeCache : public RefCounted
{
	public:

		typedef IndexedIO::EntryID ObjectHandle;
		typedef IndexedIO::EntryID HeaderHandle;
		typedef IndexedIO::EntryID AttributeHandle;

		IE_CORE_DECLAREMEMBERPTR( AttributeCache )

		///Open the cache, using the specified open mode
		AttributeCache( const std::string &filename, IndexedIO::OpenMode mode );

		///Write a piece of data associated with the specified object and attribute to the cache.
		void write( const ObjectHandle &obj, const AttributeHandle &attr, const Object *data );

		///Write data associated with the specified header to the cache.
		void writeHeader( const HeaderHandle &hdr, const Object *data );

		///Read a piece of data associated with the specified object and attribute from the cache.
		///Throws an exception if the requested data is not present in the cache.
		ObjectPtr read( const ObjectHandle &obj, const AttributeHandle &attr );

		///Read a piece of data associated with the specified object from the cache.
		///Returns a CompoundObject with attribute as keys.
		///Throws an exception if the requested data is not present in the cache.
		CompoundObjectPtr read( const ObjectHandle &obj );

		///Read data associated with the specified header from the cache.
		///Throws an exception if the requested header is not present in the cache.
		ObjectPtr readHeader( const HeaderHandle &hdr );

		///Read all header data present in the cache.
		///Creates a CompoundObject with the header names as keys.
		CompoundObjectPtr readHeader( );

		///Retrieve the list of object handles from the cache
		void objects(std::vector<ObjectHandle> &objs);

		///Retrieve the list of header handles from the cache
		void headers(std::vector<HeaderHandle> &hds);

		///Retrieve the list of attribute handles from the specified objects. Throws
		///an exception if the object is not within the cache.
		void attributes(const ObjectHandle &obj, std::vector<AttributeHandle> &attrs);

		///Retrieve the list of attribute handles that match the given regex from the specified objects.
		/// Throws an exception if the object is not within the cache.
		void attributes(const ObjectHandle &obj, const std::string regex, std::vector<AttributeHandle> &attrs);

		///Determines whether or not the cache contains the specified object
		bool contains( const ObjectHandle &obj );

		///Determines whether or not the cache contains the specified object and attribute
		bool contains( const ObjectHandle &obj, const AttributeHandle &attr );

		///Removes an object from the cache file.
		void remove( const ObjectHandle &obj );

		///Removes an object's attribute from the cache file.
		void remove( const ObjectHandle &obj, const AttributeHandle &attr );

		///Removes a header from the cache file.
		void removeHeader( const HeaderHandle &hdr );

	protected:

		IndexedIOPtr m_objectsIO;
		IndexedIOPtr m_headersIO;

		IndexedIOPtr writableHeadersIO();
		IndexedIOPtr writableObjectsIO();
		ConstIndexedIOPtr readableHeadersIO();
		ConstIndexedIOPtr readableObjectsIO();
};

IE_CORE_DECLAREPTR( AttributeCache );

} // namespace IECore

#endif //IE_CORE_ATTRIBUTECACHE_H
