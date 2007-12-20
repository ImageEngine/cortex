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

#include <boost/python.hpp>

#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/MarchingCubes.h"

using namespace boost::python;

namespace IECore
{

template<typename T>
struct MarchingCubesHelper
{
	static void march1( T& marchingCubes, const typename T::BoxType &bound, const Imath::V3i &res, typename T::ValueBaseType iso )
	{
		marchingCubes.march( bound, res, iso );
	}
	
	static void march2( T& marchingCubes, const typename T::BoxType &bound, const Imath::V3i &res )
	{
		marchingCubes.march( bound, res );
	}
};

template<typename T>
void bindMarchingCubes( const char *name )
{
	typedef class_< T, typename T::Ptr, bases<RefCounted>, boost::noncopyable > MarchingCubesPyClass;
	
	MarchingCubesPyClass( name, no_init )
		.def( init< typename T::ImplicitFnType::Ptr, typename T::MeshBuilderType::Ptr > () )
		.def( "march", &MarchingCubesHelper<T>::march1 )
		.def( "march", &MarchingCubesHelper<T>::march2 )				
	;
	
	INTRUSIVE_PTR_PATCH_TEMPLATE( T, MarchingCubesPyClass );
	implicitly_convertible< typename T::Ptr, RefCountedPtr>();	
}

void bindMarchingCubes()
{
	bindMarchingCubes<MarchingCubes< ImplicitSurfaceFunctionV3ff, MeshPrimitiveBuilder<float> > >( "MarchingCubesf" );
	bindMarchingCubes<MarchingCubes< ImplicitSurfaceFunctionV3dd, MeshPrimitiveBuilder<float> > >( "MarchingCubesd" );
}

} // namespace IECore
