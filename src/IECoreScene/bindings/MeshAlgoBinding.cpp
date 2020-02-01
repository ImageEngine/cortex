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

#include "MeshAlgoBinding.h"

#include "IECoreScene/MeshAlgo.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "boost/python/suite/indexing/container_utils.hpp"

using namespace boost::python;
using namespace IECorePython;
using namespace IECoreScene;

// Avoid cluttering the global namespace.
namespace
{

// Converts a std::pair instance to a Python tuple.
template<typename T1, typename T2>
struct StdPairToTuple
{
	static PyObject *convert( std::pair<T1, T2> const &p )
	{
		return boost::python::incref(
			boost::python::make_tuple( p.first, p.second ).ptr()
		);
	}

	static PyTypeObject const *get_pytype()
	{
		return &PyTuple_Type;
	}
};

// Helper for convenience.
template<typename T1, typename T2>
struct StdPairToTupleConverter
{
	StdPairToTupleConverter()
	{
		boost::python::to_python_converter<std::pair<T1, T2>, StdPairToTuple<T1, T2>, true //StdPairToTuple has get_pytype
										  >();
	}
};

typedef boost::python::list (*Fn)(const MeshPrimitive *mesh, const PrimitiveVariable &primitiveVariable);


boost::python::list segment(const MeshPrimitive *mesh, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues = nullptr)
{
	boost::python::list returnList;
	std::vector<MeshPrimitivePtr> segmented = MeshAlgo::segment(mesh, primitiveVariable, segmentValues);
	for (auto p : segmented)
	{
		returnList.append( p );
	}
	return returnList;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(segmentOverLoads, segment, 2, 3);

MeshPrimitivePtr merge( boost::python::list &l )
{
	std::vector<const MeshPrimitive *> meshes;
	boost::python::container_utils::extend_container( meshes, l );
	return MeshAlgo::merge( meshes );
}

} // namespace anonymous

namespace IECoreSceneModule
{

void bindMeshAlgo()
{
	object meshAlgoModule( borrowed( PyImport_AddModule( "IECore.MeshAlgo" ) ) );
	scope().attr( "MeshAlgo" ) = meshAlgoModule;

	scope meshAlgoScope( meshAlgoModule );

	StdPairToTupleConverter<PrimitiveVariable, PrimitiveVariable>();
	StdPairToTupleConverter<IECore::IntVectorDataPtr, IECore::IntVectorDataPtr>();

	def( "calculateNormals", &MeshAlgo::calculateNormals, ( arg_( "mesh" ), arg_( "interpolation" ) = PrimitiveVariable::Vertex, arg_( "position" ) = "P" ) );
	def( "calculateTangents", &MeshAlgo::calculateTangents, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv", arg_( "orthoTangents" ) = true, arg_( "position" ) = "P" ) );
	def( "calculateTangentsFromUV", &MeshAlgo::calculateTangentsFromUV, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv",  arg_( "position" ) = "P", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false ) );
	def( "calculateTangentsFromFirstEdge", &MeshAlgo::calculateTangentsFromFirstEdge, ( arg_( "mesh" ), arg_( "position" ) = "P", arg_( "normal" ) = "N", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false ) );
	def( "calculateTangentsFromTwoEdges", &MeshAlgo::calculateTangentsFromTwoEdges, ( arg_( "mesh" ), arg_( "position" ) = "P", arg_( "normal" ) = "N", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false ) );
	def( "calculateTangentsFromPrimitiveCentroid", &MeshAlgo::calculateTangentsFromPrimitiveCentroid, ( arg_( "mesh" ), arg_( "position" ) = "P", arg_( "normal" ) = "N", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false ) );
	def( "calculateFaceArea", &MeshAlgo::calculateFaceArea, ( arg_( "mesh" ), arg_( "position" ) = "P" ) );
	def( "calculateFaceTextureArea", &MeshAlgo::calculateFaceTextureArea, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv", arg_( "position" ) = "P" ) );
	def( "calculateDistortion", &MeshAlgo::calculateDistortion, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv", arg_( "referencePosition" ) = "Pref", arg_( "position" ) = "P" ) );
	def( "resamplePrimitiveVariable", &MeshAlgo::resamplePrimitiveVariable );
	def( "deleteFaces", &MeshAlgo::deleteFaces, arg_( "invert" ) = false );
	def( "reverseWinding", &MeshAlgo::reverseWinding );
	def( "reorderVertices", &MeshAlgo::reorderVertices, ( arg_( "mesh" ), arg_( "id0" ), arg_( "id1" ), arg_( "id2" ) ) );
	def( "distributePoints", &MeshAlgo::distributePoints, ( arg_( "mesh" ), arg_( "density" ) = 100.0, arg_( "offset" ) = Imath::V2f( 0 ), arg_( "densityMask" ) = "density", arg_( "uvSet" ) = "uv", arg_( "position" ) = "P" ) );
	def( "segment", &::segment, segmentOverLoads() );
	def( "merge", &::merge );
	def( "triangulate", &MeshAlgo::triangulate, (arg_("mesh"), arg_("tolerance") =1e-6f, arg_("throwExceptions") = false) );
	def( "connectedVertices", &MeshAlgo::connectedVertices );
}

} // namespace IECoreSceneModule

