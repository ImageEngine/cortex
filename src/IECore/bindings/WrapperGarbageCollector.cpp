//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/bindings/WrapperGarbageCollector.h"

#include <iostream>
#include <cassert>
#include <vector>

using namespace IECore;

WrapperGarbageCollector::WrapperGarbageCollector( PyObject *pyObject, RefCounted *object )
	:	m_pyObject( pyObject ), m_object( object )
{
	g_allocCount++;

	if (g_allocCount >= g_allocThreshold)
	{
		collect();
	}

	g_refCountedToPyObject[object] = pyObject;
}

WrapperGarbageCollector::~WrapperGarbageCollector()
{
	g_refCountedToPyObject.erase( m_object );
}		
		
void WrapperGarbageCollector::collect()
{
	std::vector<PyObject*> toCollect;

	do
	{
		toCollect.clear();
		for( InstanceMap::const_iterator it = g_refCountedToPyObject.begin(); it!=g_refCountedToPyObject.end(); it++ )
		{
			if( it->first->refCount()==1 )
			{
				if( it->second->ob_refcnt==1 )
				{
					toCollect.push_back( it->second );
				}
			}
		}

		for (std::vector<PyObject*>::const_iterator jt = toCollect.begin(); jt != toCollect.end(); ++jt)
		{
			Py_DECREF( *jt );
		}
	} while( toCollect.size() );

	g_allocCount = 0;
	/// Scale the collection threshold with the number of objects to be collected, otherwise we get
	/// awful (quadratic?) behaviour when creating large numbers of objects.
	/// \todo Revisit this with a better thought out strategy, perhaps like python's own garbage collector.
	g_allocThreshold = std::max( size_t( 50 ), g_refCountedToPyObject.size() );
}
