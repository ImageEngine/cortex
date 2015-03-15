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

#include "IECorePython/Export.h"

namespace IECorePython
{

/// This base class is used to store a static map of pointers
/// to all the Wrapper instances in existence. This is then
/// used to resolve not only the circular reference problem with
/// wrapped objects (RefCounted->PyObject->RefCounted) but also
/// the identity problem when pushing RefCountedPtr to python (we
/// need to find the corresponding PyObject).
//\todo Optimize the collect() function. The function is taking too much tests on reference counting when many objects are allocated.
class IECOREPYTHON_API WrapperGarbageCollector
{

	public :

		/// \deprecated
		/// \todo Remove for major version 9.
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

		/// Constructor for use by RefCountedWrapper derived class. The self argument is
		/// the python object which holds the wrapped RefCounted argument, and wrappedType
		/// is the Python type corresponding to the C++ wrapper. This constructor
		/// initialises m_pyObject as follows :
		///
		/// If the python type of self is not wrappedType, this indicates that self
		/// is an instance of a python subclass of the wrapped C++ class. In this case
		/// we want to support access to virtual overrides via methodOverride(). Therefore
		/// m_pyObject is initialised to point to self, and self's reference count is
		/// incremented to keep it alive as long as the C++ object is alive. This reference
		/// cycle will be broken by the collect() method. After this, isSubclassed() will
		/// return true and methodOverride() may be used from virtual overrides to call into
		/// python.
		///
		/// If the python type of self is the same as wrappedType, this indicates that self
		/// is not subclassed in python. In this case we don't want to support virtual overrides
		/// nor do we want to pay the overhead of garbage collection. Therefore m_pyObject
		/// is initialised to NULL, isSubclassed() will return false, methodOverride() will always
		/// fail, and we will have no further overhead.
		WrapperGarbageCollector( PyObject *self, IECore::RefCounted *wrapped, PyTypeObject *wrappedType );

		/// Returns true if this instance is of a Python subclass, and methodOverride() is therefore
		/// useable. This method may be called without holding the GIL, so it should be tested first
		/// and methodOverride() only called if it returns true - this avoids the huge overhead associated
		/// with acquiring the GIL and entering Python only to discover that there is no override.
		bool isSubclassed() const
		{
			return m_pyObject;
		}
		
		/// Returns an overridden method for this instance if one exists. The GIL must
		/// be held before calling this method. See isSubclassed() for an important means of
		/// avoiding this overhead where it isn't necessary. Also see RefCountedWrapper::methodOverride()
		/// which provides an overload which automatically supplies wrappedType.
		boost::python::object methodOverride( const char *name, PyTypeObject *wrappedType ) const;

		/// \todo Make private for the next major version.
		PyObject *m_pyObject;

	private :

		static size_t g_allocCount;
		static size_t g_allocThreshold;

		typedef std::map<IECore::RefCounted *, PyObject *> InstanceMap;
		static InstanceMap g_refCountedToPyObject;
		
};

}

#endif // IECOREPYTHON_WRAPPERGARBAGECOLLECTOR_H
