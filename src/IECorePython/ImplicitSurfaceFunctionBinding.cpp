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

#include "boost/python.hpp"

#include "IECore/Exception.h"

#include "IECore/ImplicitSurfaceFunction.h"
#include "IECorePython/Wrapper.h"
#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/ScopedGILLock.h"

using namespace boost;
using namespace boost::python;
using namespace IECore;

namespace IECorePython
{


template<typename T>
class ImplicitWrap :
	public ImplicitSurfaceFunction<typename T::Point, typename T::Value>,
	public Wrapper<ImplicitSurfaceFunction<typename T::Point, typename T::Value> >
{
	public :

		IE_CORE_DECLAREMEMBERPTR( ImplicitWrap<T> );

		ImplicitWrap( PyObject *self ) : ImplicitSurfaceFunction<typename T::Point, typename T::Value >(), Wrapper<ImplicitSurfaceFunction< typename T::Point, typename T::Value> >( self, this )
		{
		}

		virtual ~ImplicitWrap()
		{
		}

		virtual typename T::Value getValue( const typename T::Point &p )
		{
			ScopedGILLock gilLock;
			override o = this->get_override( "getValue" );
			if( o )
			{
				return o( p );
			}
			else
			{
				throw Exception( "getValue() python method not defined" );
			}
		};

};

template<typename T>
void bindImplicit( const char *name )
{
	RefCountedClass<T, RefCounted, typename ImplicitWrap<T>::Ptr >( name )
		.def( init<> () )
		.def( "getValue", pure_virtual( &T::getValue ) )
	;
}

void bindImplicitSurfaceFunction()
{
	bindImplicit<ImplicitSurfaceFunctionV3ff>( "ImplicitSurfaceFunctionV3ff" );
	bindImplicit<ImplicitSurfaceFunctionV3fd>( "ImplicitSurfaceFunctionV3fd" );
	bindImplicit<ImplicitSurfaceFunctionV3df>( "ImplicitSurfaceFunctionV3df" );
	bindImplicit<ImplicitSurfaceFunctionV3dd>( "ImplicitSurfaceFunctionV3dd" );
}

} // namespace IECorePython
