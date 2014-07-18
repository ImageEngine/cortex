//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/WrapperGarbageCollector.h"

#include <vector>

using namespace IECorePython;

size_t WrapperGarbageCollector::g_allocCount = 0;
size_t WrapperGarbageCollector::g_allocThreshold = 50;
WrapperGarbageCollector::InstanceMap WrapperGarbageCollector::g_refCountedToPyObject;

WrapperGarbageCollector::WrapperGarbageCollector( PyObject *pyObject, IECore::RefCounted *object )
			:	m_pyObject( pyObject )
{
	g_allocCount++;

	if (g_allocCount >= g_allocThreshold)
	{
		collect();
	}

	g_refCountedToPyObject[object] = pyObject;
}

WrapperGarbageCollector::WrapperGarbageCollector( PyObject *self, IECore::RefCounted *wrapped, PyTypeObject *wrappedType )
	:	m_pyObject( NULL )
{
	assert( self );
	assert( wrapped );
	assert( wrappedType );
	
	if( self->ob_type != wrappedType )
	{
		// we're dealing with a python subclass.
		
		m_pyObject = self;

		g_allocCount++;

		if( g_allocCount >= g_allocThreshold )
		{
			collect();
		}

		g_refCountedToPyObject[wrapped] = m_pyObject;
		Py_INCREF( m_pyObject );
	}
}

WrapperGarbageCollector::~WrapperGarbageCollector()
{
}

void WrapperGarbageCollector::collect()
{
	std::vector<PyObject*> toCollect;

	do
	{
		toCollect.clear();
		for( InstanceMap::iterator it = g_refCountedToPyObject.begin(); it!=g_refCountedToPyObject.end(); )
		{
			InstanceMap::iterator nextIt = it; nextIt++;
			if( it->first->refCount()==1 )
			{
				if( it->second->ob_refcnt==1 )
				{
					// add to the list of objects to destroy
					toCollect.push_back( it->second );
					
					// Make sure the object is removed from the list before the next loop, which
					// destroys it. This is because Py_DECREF() can run arbitrary, multithreaded python
					// code, (especially if the python object has a __del__ method) which may create
					// WrapperGarbageCollector objects, effectively making this method recurse. It's also
					// possible that this python code can call collect() directly. Removing the object
					// from the list here avoids double deallocation.
					g_refCountedToPyObject.erase( it );
				}
			}
			it = nextIt;
		}

		for (std::vector<PyObject*>::const_iterator jt = toCollect.begin(); jt != toCollect.end(); ++jt)
		{
			// decrement the reference count for the python object, which will trigger the destruction
			// of the WrapperGarbageCollector object.
			
			// NOTE: Py_DECREF() also used to be called in Wrapper::~Wrapper(), which inherits from WrapperGarbageCollector,
			// and is defined in IECorePython/Wrapper.h. This was conditional on the reference count being greater than
			// zero, and so didn't usually happen.
			// Occasionally, however, the python code that was run during Py_DECREF() would create a python object with
			// exactly the same address as (*jt), before invoking the c++ destructors. This meant that Wrapper::~Wrapper()
			// would think it was still holding on to a python object with a non zero reference count, when really it
			// was holding onto someone else's object. This means the new object would get destroyed, leading to dangling
			// pointers and crashes, and has since been removed.
			Py_DECREF( *jt );
		}
	} while( toCollect.size() );

	g_allocCount = 0;
	/// Scale the collection threshold with the number of objects to be collected, otherwise we get
	/// awful (quadratic?) behaviour when creating large numbers of objects.
	/// \todo Revisit this with a better thought out strategy, perhaps like python's own garbage collector.
	g_allocThreshold = std::max( size_t( 50 ), g_refCountedToPyObject.size() );
}

PyObject *WrapperGarbageCollector::pyObject( IECore::RefCounted *refCountedObject )
{
	InstanceMap::const_iterator it = g_refCountedToPyObject.find( refCountedObject );
	if( it==g_refCountedToPyObject.end() )
	{
		return 0;
	}
	return it->second;
}

size_t WrapperGarbageCollector::numWrappedInstances()
{
	return g_refCountedToPyObject.size();
}

void WrapperGarbageCollector::setCollectThreshold( size_t t )
{
	g_allocThreshold = t;
}

size_t WrapperGarbageCollector::getCollectThreshold()
{
	return g_allocThreshold;
}

boost::python::object WrapperGarbageCollector::methodOverride( const char *name, PyTypeObject *wrappedType ) const
{
	if( !m_pyObject )
	{
		return boost::python::object();
	}
	
	// lookup the method on the python instance. this may
	// or may not be an override. a new reference is returned
	// so we must use a handle to manage it.
	boost::python::handle<> methodFromInstance(
		boost::python::allow_null(
			PyObject_GetAttrString(
				m_pyObject,
				const_cast<char*>( name )
			)
		)
	);
		
	if( !methodFromInstance || !PyMethod_Check( methodFromInstance.get() ) )
	{
		// if the attribute lookup failed, an error will be set, and we
		// need to clear it before returning.
		PyErr_Clear();
		return boost::python::object();
	}
	
	// lookup the method defined by our type. a borrowed reference
	// is returned so we don't need to use a handle to manage the
	// reference count.
	PyObject *methodFromType = PyDict_GetItemString(
		wrappedType->tp_dict, const_cast<char*>( name )
	);
	
	// if they're not the same then we have an override
	if( methodFromType != ((PyMethodObject *)methodFromInstance.get())->im_func )
	{
		return boost::python::object( methodFromInstance );
	}

	return boost::python::object();
}
