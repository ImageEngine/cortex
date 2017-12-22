//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECore/BoxAlgo.h"

#include "IECorePython/BoxAlgoBinding.h"

using namespace boost::python;
using namespace Imath;
using namespace IECore;
using namespace IECorePython;

namespace
{

template<typename T>
static tuple split1( const Box<T> &box, int axis )
{
	Box<T> low, high;
	BoxAlgo::split( box, low, high, axis );
	return make_tuple( low, high );
}

template<typename T>
static tuple split2( const Box<T> &box )
{
	Box<T> low, high;
	BoxAlgo::split( box, low, high );
	return make_tuple( low, high );
}

template<typename T>
void bind()
{
	def( "contains", &BoxAlgo::contains<T> );
	def( "split", &split1<T> );
	def( "split", &split2<T> );
}

} // namespace

void IECorePython::bindBoxAlgo()
{
	object boxAlgoModule( borrowed( PyImport_AddModule( "IECore.BoxAlgo" ) ) );
	scope().attr( "BoxAlgo" ) = boxAlgoModule;

	scope boxAlgoScope( boxAlgoModule );

	def( "closestPointInBox", &BoxAlgo::closestPointInBox<short> );
	def( "closestPointInBox", &BoxAlgo::closestPointInBox<int> );
	def( "closestPointInBox", &BoxAlgo::closestPointInBox<float> );
	def( "closestPointInBox", &BoxAlgo::closestPointInBox<double> );

	bind<V2s>();
	bind<V2i>();
	bind<V2f>();
	bind<V2d>();

	bind<V3s>();
	bind<V3i>();
	bind<V3f>();
	bind<V3d>();

}
