//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SplineData.h"
#include "IECore/bindings/RunTimeTypedBinding.h"
#include "IECore/bindings/IECoreBinding.h"

#include <sstream>

using namespace std;
using std::string;
using namespace boost;
using namespace boost::python;
using namespace Imath;

namespace IECore
{

template<class T>
static std::string repr( T &x )
{
	std::stringstream s;

	s << "IECore." << x.typeName() << "( ";

	object item( x.readable() );

	assert( item.attr( "__repr__" ) != object() );

	s << call_method< std::string >( item.ptr(), "__repr__" );

	s << " )";

	return s.str();
}

template<class T>
static void setValue( T &that, const typename T::ValueType &v )
{
	that.writable() = v;
}

template<class T>
static typename T::ValueType &getValue( T &that )
{
	return that.writable();
}

template< typename T >
void bindSplineData()
{
	RunTimeTypedClass<T>()
		.def( init<>() )
		.def( init<const typename T::ValueType &>() )
		.add_property( "value", make_function( &getValue<T>, return_internal_reference<>() ), &setValue<T> )
		.def( "__repr__", &repr<T> )
	;
}

void bindSplineData()
{
	bindSplineData<SplineffData>();
	bindSplineData<SplineddData>();
	bindSplineData<SplinefColor3fData>();
	bindSplineData<SplinefColor4fData>();
}

} // namespace IECore
