//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
#include "IECore/BlobbyImplicitSurfaceFunction.h"
#include "IECore/bindings/RefCountedBinding.h"

using namespace boost::python;

namespace IECore
{

template<typename T>
void bindBlobbyImplicitSurfaceFunction( const char *name )
{
	typedef ImplicitSurfaceFunction<typename T::Point, typename T::Value> Base;

	RefCountedClass<T, Base>( name )
		.def( init< typename T::PointVectorData::ConstPtr, ConstDoubleVectorDataPtr, ConstDoubleVectorDataPtr > () )
	;

}

void bindBlobbyImplicitSurfaceFunction()
{
	bindBlobbyImplicitSurfaceFunction<BlobbyImplicitSurfaceFunctionV3ff>( "BlobbyImplicitSurfaceFunctionV3ff" );
	bindBlobbyImplicitSurfaceFunction<BlobbyImplicitSurfaceFunctionV3fd>( "BlobbyImplicitSurfaceFunctionV3fd" );
	bindBlobbyImplicitSurfaceFunction<BlobbyImplicitSurfaceFunctionV3df>( "BlobbyImplicitSurfaceFunctionV3df" );
	bindBlobbyImplicitSurfaceFunction<BlobbyImplicitSurfaceFunctionV3dd>( "BlobbyImplicitSurfaceFunctionV3dd" );
}

} // namespace IECore
