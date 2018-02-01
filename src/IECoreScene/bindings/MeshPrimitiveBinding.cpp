//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "MeshPrimitiveBinding.h"

#include "IECoreScene/MeshPrimitive.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace IECoreSceneModule
{

	static IntVectorDataPtr verticesPerFace( const MeshPrimitive &p )
	{
		return p.verticesPerFace()->copy();
	}

	static IntVectorDataPtr vertexIds( const MeshPrimitive &p )
	{
		return p.vertexIds()->copy();
	}

	void bindMeshPrimitive()
	{
		RunTimeTypedClass<MeshPrimitive>()
			.def( init<>() )
			.def( init<IntVectorDataPtr, IntVectorDataPtr, optional<const std::string &, V3fVectorDataPtr> >() )
			.def( "numFaces", &MeshPrimitive::numFaces )
			.def("minVerticesPerFace", &MeshPrimitive::minVerticesPerFace )
			.def("maxVerticesPerFace", &MeshPrimitive::maxVerticesPerFace )
			/// \todo I'd rather see these bound as functions rather than properties so they match the C++ interface.
			/// I think this is particularly important for verticesPerFace and vertexIds as it's pretty unintuitive that a property
			/// should return a copy. This is something we need to be more consistent about throughout cortex.
			.add_property( "verticesPerFace", &verticesPerFace, "A copy of the mesh's list of vertices per face." )
			.add_property( "vertexIds", &vertexIds, "A copy of the mesh's list of vertex ids." )
			.add_property( "interpolation", make_function( &MeshPrimitive::interpolation, return_value_policy<copy_const_reference>() ), &MeshPrimitive::setInterpolation )
			.def( "setTopology", &MeshPrimitive::setTopology )
			.def( "createBox", &MeshPrimitive::createBox, ( arg_( "bounds" ) ) ).staticmethod( "createBox" )
			.def( "createPlane", &MeshPrimitive::createPlane, ( arg_( "bounds" ), arg_( "divisions" ) = Imath::V2i( 1 ) ) ).staticmethod( "createPlane" )
			.def( "createSphere", &MeshPrimitive::createSphere, ( arg_( "radius" ), arg_( "zMin" ) = -1.0f, arg_( "zMax" ) = 1.0f, arg_( "thetaMax" ) = 360.0f, arg_( "divisions" ) = Imath::V2i( 20, 40 ) ) ).staticmethod( "createSphere" )
		;
	}

}
