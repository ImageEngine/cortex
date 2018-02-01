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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "IECorePython/LookupBinding.h"

#include "IECore/Lookup.h"
#include "IECore/Spline.h"
#include "IECore/TypedData.h"
#include "IECore/VectorTypedData.h"

#include <cassert>
#include <iterator>
#include <string>

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

template<typename T, typename U>
void bindLookup(const char *bindName);

void bindLookup()
{
	bindLookup< float, float >("Lookupff");
	bindLookup< double, double >("Lookupdd");
	bindLookup< float, Imath::Color3f >("LookupfColor3f");
	bindLookup< float, Imath::Color4f >("LookupfColor4f");
}

/// \todo Implement Lookup for generic functors.
template<typename T, typename U>
void initLookup( Lookup<T,U> &lookup, Spline< T,U > &spline, T xMin, T xMax, unsigned numSamples )
{
	lookup.init( spline, xMin, xMax, numSamples );
}

template<typename T, typename U>
typename TypedData< std::vector< U > >::Ptr vectorLookup( Lookup<T,U> &lookup, typename TypedData< std::vector< T > >::Ptr x )
{
	typename TypedData< std::vector< U > >::Ptr res = new TypedData< std::vector< U > >;
	res->writable().resize( x->readable().size() );
	typename std::vector< T >::const_iterator xIt = x->readable().begin();
	typename std::vector< U >::iterator yIt = res->writable().begin();

	for ( ; xIt != x->readable().end(); xIt++, yIt++ )
	{
		*yIt = lookup( *xIt );
	}
	return res;
}

template<typename T, typename U>
void bindLookup(const char *bindName)
{
	class_< Lookup<T,U>, boost::noncopyable>(bindName, no_init)
		.def(init<>())
		.def( "init", &initLookup<T,U>, ( arg_( "self" ), arg_( "func" ), arg_( "xMin" ) = 0, arg_( "xMax" )= 1, arg_( "numSamples" )=100 ) )
		.def("__call__", &Lookup<T,U>::operator() )
		.def("__call__", &vectorLookup<T,U> )
		;
}

}
