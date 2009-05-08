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

#ifndef IECORE_WRAPPERGARBAGECOLLECTORBASE_H
#define IECORE_WRAPPERGARBAGECOLLECTORBASE_H

#include <list>
#include <map>

#include "IECore/RefCounted.h"

// forward declaration of the Python object type.
extern "C" {
	struct _object;
	typedef _object PyObject;
}

namespace IECore
{

class WrapperGarbageCollector;

/// This base class is used to store a static map of pointers
/// to all the Wrapper instances in existence. This is then
/// used to resolve not only the circular reference problem with
/// wrapped objects (RefCounted->PyObject->RefCounted) but also
/// the identity problem when pushing RefCountedPtr to python (we
/// need to find the corresponding PyObject). This class is compiled
/// into the IECore library rather than the IECore python module so
/// that python bindings for other libraries can link to it - it doesn't
/// introduce python dependencies into IECore as it uses only a forward
/// declaration to PyObject - all the python related work happens in
/// WrapperGarbageCollector and Wrapper. We could make an IECorePython
/// library to contain this class, in which case it could be merged
/// with WrapperGarbageCollector.
class WrapperGarbageCollectorBase
{
	public :

		/// Returns the python object holding the specified
		/// object, or 0 if no python object is associated
		/// with that object.
		static PyObject *pyObject( RefCounted *refCountedObject );
		/// Returns the number of wrapped instances currently
		/// in existence.
		static size_t numWrappedInstances();

		/// Sets the number of object instance allocations
		/// after which a garbage collection pass will be
		/// performed.
		static void setCollectThreshold( size_t t );
		/// Returns the collection threshold.
		static size_t getCollectThreshold();

	protected:

		static size_t g_allocCount;
		static size_t g_allocThreshold;

		typedef std::map<RefCounted *, PyObject *> InstanceMap;
		static InstanceMap g_refCountedToPyObject;

};

} // namespace IECore

#endif // IECORE_WRAPPERGARBAGECOLLECTORBASE_H
