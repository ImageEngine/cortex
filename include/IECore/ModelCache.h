//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MODELCACHE_H
#define IECORE_MODELCACHE_H

#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathMatrix.h"

#include "IECore/Object.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ModelCache );

/// A simple means of saving and loading hierarchical descriptions of static models, with
/// the ability to traverse the model and perform partial loading on demand. Intended to
/// be used with an AttributeCache when animation is needed.
/// \threading It is not safe to use instances of this class accessing the same file from
/// multiple concurrent threads. It is fine to load different files in different threads.
/// \ingroup ioGroup
class ModelCache : public RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( ModelCache )

		/// Opens the cache, using the specified open mode, and setting the
		/// current object path to "/". Depending on what mode is chosen,
		/// different subsets of the methods below are available. When the
		/// open mode is Read, only the const methods may be used and
		/// when the open mode is Write, the non-const methods
		/// may be used in addition. Append mode is currently not supported.
		ModelCache( const std::string &fileName, IndexedIO::OpenMode mode );
		/// Constructor which uses an already-opened IndexedIO, this
		/// can be used if you wish to use an alternative IndexedIO
		/// implementation for the backend.
		ModelCache( IECore::IndexedIOPtr indexedIO );
		
		virtual ~ModelCache();

		/// Returns the object path within the model that this class is
		/// referring to. For instances created using the constructor
		/// this will be "/". Use the childNames() and child() methods
		/// to traverse to other parts of the model.
		const std::string &path() const;
		/// Returns the name of the current directory in the path
		const std::string &name() const;
		
		/// Returns the bounding box for the entire scene contents from
		/// path() down, inclusive of the object at this path, but
		/// exclusive of the transform at this path. Note that during
		/// writing this method will raise an Exception, as bounds are
		/// only written when the object is closed - this allows the
		/// bounds to be calculated automatically and propagated up through
		/// the parent hierarchy automatically.
		Imath::Box3d readBound() const;
		/// Writes the bound for this path, overriding the default bound
		/// that would be written automatically. Note that generally there
		/// is no need to call this function - but perhaps it might be useful
		/// when writing objects which conceptually have a bound but don't
		/// derive from VisibleRenderable.
		void writeBound( const Imath::Box3d &bound );
	
		/// Returns the transform applied to this path within the model.
		Imath::M44d readTransform() const;
		/// Writes the transform applied to this path within the model.
		void writeTransform( const Imath::M44d &transform );
		
		/// Reads the object stored at this path in the model - may
		/// return 0 when no object has been stored.
		ObjectPtr readObject() const;
		/// Writes an object to this path in the model.	
		void writeObject( const IECore::Object *object );
		/// Convenience method to determine if an object exists without reading it
		bool hasObject() const;
		
		/// Queries the names of any existing children of path() within
		/// the model.
		void childNames( IndexedIO::EntryIDList &childNames ) const;
		/// Returns an object for writing to the specified child, throwing
		/// an Exception if the child already exists. Bounding boxes will
		/// automatically be propagated up from the children to the parent
		/// as it is written.
		ModelCachePtr writableChild( const IndexedIO::EntryID &childName );
		/// Returns an object for querying the existing child, throwing an
		/// Exception if no such child exists.
		ConstModelCachePtr readableChild( const IndexedIO::EntryID &childName ) const;

	private :

		IE_CORE_FORWARDDECLARE( Implementation );

		ModelCache( ImplementationPtr implementation );

		ImplementationPtr m_implementation;


};

} // namespace IECore

#endif // IECORE_MODELCACHE_H
