//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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
#include "boost/python/make_constructor.hpp"

#include "IECore/TransformationMatrixData.h"
#include "IECorePython/TransformationMatrixDataBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/IECoreBinding.h"

#include <sstream>

using namespace std;
using std::string;
using namespace boost;
using namespace boost::python;
using namespace Imath;
using namespace IECore;

namespace IECorePython
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// functions used by the bindings
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
static typename T::Ptr construct()
{
	return new T;
}

template<class T>
static typename T::Ptr constructWithValue( const typename T::ValueType &v )
{
	return new T( v );
}

template<class T>
static void setValue( T &that, const typename T::ValueType &v )
{
	that.writable() = v;
}

template<class T>
static typename T::ValueType getValue( T &that )
{
	return that.readable();
}

template<class T>
static int cmp( T &x, T &y )
{
	return ( x.readable() == y.readable() );
}

template< typename T >
void bindTypedTransformationMatrixData()
{
	RunTimeTypedClass<TypedData< TransformationMatrix< T > > >()
		.def( "__init__", make_constructor( &construct< TypedData< TransformationMatrix< T > > > ), "Construct with no specified value." )
		.def( "__init__", make_constructor( &constructWithValue< TypedData< TransformationMatrix< T > > > ), "Construct with the specified value." )
		.add_property( "value",   &getValue< TypedData< TransformationMatrix< T > > >,
	    						  &setValue< TypedData< TransformationMatrix< T > > >, "The value contained by the object.")
		.def("__cmp__", &cmp< TypedData< TransformationMatrix< T > > > )
		.def( "hasBase", &TypedData< TransformationMatrix< T > >::hasBase ).staticmethod( "hasBase" )
	;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the one function exposed to the outside world
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bindTransformationMatrixData()
{
	bindTypedTransformationMatrixData< float >();
	bindTypedTransformationMatrixData< double >();
}

} // namespace IECorePython
