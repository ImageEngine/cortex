//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREPYTHON_WRAPPERGARBAGECOLLECTOR_H
#define IECOREPYTHON_WRAPPERGARBAGECOLLECTOR_H

#include <boost/python.hpp>

#include <map>

#include "IECore/RefCounted.h"

namespace IECorePython
{

/// This base class is used to store a static map of pointers
/// to all the Wrapper instances in existence. This is then
/// used to resolve not only the circular reference problem with
/// wrapped objects (RefCounted->PyObject->RefCounted) but also
/// the identity problem when pushing RefCountedPtr to python (we
/// need to find the corresponding PyObject).
//\todo Optimize the collect() function. The function is taking too much tests on reference counting when many objects are allocated.
class WrapperGarbageCollector
{

	public :

		WrapperGarbageCollector( PyObject *pyObject, IECore::RefCounted *object );
		virtual ~WrapperGarbageCollector();

		/// Returns the number of wrapped instances currently
		/// in existence.
		static size_t numWrappedInstances();

		/// Sets the number of object instance allocations
		/// after which a garbage collection pass will be
		/// performed automatically.
		static void setCollectThreshold( size_t t );
		/// Returns the collection threshold.
		static size_t getCollectThreshold();
		
		/// Collects any wrapped objects which exist only because
		/// of a circular reference between the python wrapper
		/// and the C++ object.
		static void collect();
		
		/// Returns the python object holding the specified
		/// object, or 0 if no python object is associated
		/// with that object.
		static PyObject *pyObject( IECore::RefCounted *refCountedObject );

	protected :

		PyObject *m_pyObject;
		IECore::RefCounted *m_object;

	private :

		static size_t g_allocCount;
		static size_t g_allocThreshold;

		typedef std::map<IECore::RefCounted *, PyObject *> InstanceMap;
		static InstanceMap g_refCountedToPyObject;
		
};

}

#endif // IECOREPYTHON_WRAPPERGARBAGECOLLECTOR_H
