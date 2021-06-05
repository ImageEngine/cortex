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

PrimitiveVariable calculateNormalsWrapper( const MeshPrimitive *mesh, PrimitiveVariable::Interpolation interpolation, const std::string &position, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateNormals( mesh, interpolation, position, canceller );
}

std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromUVWrapper( const MeshPrimitive *mesh, const std::string &uvSet, const std::string &position, bool orthoTangents, bool leftHanded, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateTangentsFromUV( mesh, uvSet, position, orthoTangents, leftHanded, canceller );
}

std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromFirstEdgeWrapper( const MeshPrimitive *mesh, const std::string &position, const std::string &normal, bool orthoTangents, bool leftHanded, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateTangentsFromFirstEdge( mesh, position, normal, orthoTangents, leftHanded, canceller );
}

std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromPrimitiveCentroidWrapper( const MeshPrimitive *mesh, const std::string &position, const std::string &normal, bool orthoTangents, bool leftHanded, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateTangentsFromPrimitiveCentroid( mesh, position, normal, orthoTangents, leftHanded, canceller );
}

std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromTwoEdgesWrapper( const MeshPrimitive *mesh, const std::string &position, const std::string &normal, bool orthoTangents, bool leftHanded, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateTangentsFromTwoEdges( mesh, position, normal, orthoTangents, leftHanded, canceller );
}

PrimitiveVariable calculateFaceAreaWrapper( const MeshPrimitive *mesh, const std::string &position, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateFaceArea( mesh, position, canceller );
}

PrimitiveVariable calculateFaceTextureAreaWrapper( const MeshPrimitive *mesh, const std::string &uvSet, const std::string &position, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateFaceTextureArea( mesh, uvSet, position, canceller );
}

std::pair<PrimitiveVariable, PrimitiveVariable> calculateDistortionWrapper( const MeshPrimitive *mesh, const std::string &uvSet, const std::string &referencePosition, const std::string &position, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::calculateDistortion( mesh, uvSet, referencePosition, position, canceller );
}

void resamplePrimitiveVariableWrapper( const MeshPrimitive *mesh, PrimitiveVariable& primitiveVariable, PrimitiveVariable::Interpolation interpolation, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::resamplePrimitiveVariable( mesh, primitiveVariable, interpolation, canceller );
}

MeshPrimitivePtr deleteFacesWrapper( const MeshPrimitive *meshPrimitive, const PrimitiveVariable &facesToDelete, bool invert, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::deleteFaces( meshPrimitive, facesToDelete, invert, canceller );
}

void reverseWindingWrapper( MeshPrimitive *meshPrimitive, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::reverseWinding( meshPrimitive, canceller );
}

void reorderVerticesWrapper( MeshPrimitive *mesh, int id0, int id1, int id2, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::reorderVertices( mesh, id0, id1, id2, canceller );
}

PointsPrimitivePtr distributePointsWrapper( const MeshPrimitive *mesh, float density, const Imath::V2f &offset, const std::string &densityMask, const std::string &uvSet, const std::string &position, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::distributePoints( mesh, density, offset, densityMask, uvSet, position, canceller );
}

boost::python::list segmentWrapper(const MeshPrimitive *mesh, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues = nullptr, const IECore::Canceller *canceller = nullptr )
{
	std::vector<MeshPrimitivePtr> segmented;
	{
		ScopedGILRelease gilRelease;
		segmented = MeshAlgo::segment(mesh, primitiveVariable, segmentValues);
	}

	boost::python::list returnList;
	for (auto p : segmented)
	{
		returnList.append( p );
	}
	return returnList;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(segmentOverLoads, segmentWrapper, 2, 3);

MeshPrimitivePtr mergeWrapper( boost::python::list &l, const IECore::Canceller *canceller )
{
	std::vector<const MeshPrimitive *> meshes;
	boost::python::container_utils::extend_container( meshes, l );
	ScopedGILRelease gilRelease;
	return MeshAlgo::merge( meshes, canceller );
}

MeshPrimitivePtr triangulateWrapper( const MeshPrimitive *mesh, const IECore::Canceller *canceller )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::triangulate( mesh, canceller );
}

std::pair<IECore::IntVectorDataPtr, IECore::IntVectorDataPtr> connectedVerticesWrapper( const IECoreScene::MeshPrimitive *mesh, const IECore::Canceller *canceller = nullptr )
{
	ScopedGILRelease gilRelease;
	return MeshAlgo::connectedVertices( mesh, canceller );
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

	def( "calculateNormals", &calculateNormalsWrapper, ( arg_( "mesh" ), arg_( "interpolation" ) = PrimitiveVariable::Vertex, arg_( "position" ) = "P", arg_( "canceller" ) = object() ) );
	def( "calculateTangents", &MeshAlgo::calculateTangents, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv", arg_( "orthoTangents" ) = true, arg_( "position" ) = "P" ) );
	def( "calculateTangentsFromUV", &calculateTangentsFromUVWrapper, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv",  arg_( "position" ) = "P", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false, arg_( "canceller" ) = object() ) );
	def( "calculateTangentsFromFirstEdge", &calculateTangentsFromFirstEdgeWrapper, ( arg_( "mesh" ), arg_( "position" ) = "P", arg_( "normal" ) = "N", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false, arg_( "canceller" ) = object() ) );
	def( "calculateTangentsFromTwoEdges", &calculateTangentsFromTwoEdgesWrapper, ( arg_( "mesh" ), arg_( "position" ) = "P", arg_( "normal" ) = "N", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false, arg_( "canceller" ) = object() ) );
	def( "calculateTangentsFromPrimitiveCentroid", &calculateTangentsFromPrimitiveCentroidWrapper, ( arg_( "mesh" ), arg_( "position" ) = "P", arg_( "normal" ) = "N", arg_( "orthoTangents" ) = true, arg_( "leftHanded" ) = false, arg_( "canceller" ) = object() ) );
	def( "calculateFaceArea", &calculateFaceAreaWrapper, ( arg_( "mesh" ), arg_( "position" ) = "P", arg_( "canceller" ) = object() ) );
	def( "calculateFaceTextureArea", &calculateFaceTextureAreaWrapper, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv", arg_( "position" ) = "P", arg_( "canceller" ) = object() ) );
	def( "calculateDistortion", &calculateDistortionWrapper, ( arg_( "mesh" ), arg_( "uvSet" ) = "uv", arg_( "referencePosition" ) = "Pref", arg_( "position" ) = "P", arg_( "canceller" ) = object() ) );
	def( "resamplePrimitiveVariable", &resamplePrimitiveVariableWrapper, ( arg_( "mesh" ), arg_( "primitiveVariable" ), arg_( "interpolation" ), arg( "canceller" ) = object() ) );
	def( "deleteFaces", &deleteFacesWrapper, ( arg_( "meshPrimitive" ), arg_( "facesToDelete" ), arg_( "invert" ) = false, arg_( "canceller" ) = object() ) );
	def( "reverseWinding", &reverseWindingWrapper, ( arg_( "meshPrimitive" ), arg_( "canceller" ) = object() ) );
	def( "reorderVertices", &reorderVerticesWrapper, ( arg_( "mesh" ), arg_( "id0" ), arg_( "id1" ), arg_( "id2" ), arg_( "canceller" ) = object() ) );
	def( "distributePoints", &distributePointsWrapper, ( arg_( "mesh" ), arg_( "density" ) = 100.0, arg_( "offset" ) = Imath::V2f( 0 ), arg_( "densityMask" ) = "density", arg_( "uvSet" ) = "uv", arg_( "position" ) = "P", arg_( "canceller" ) = object() ) );
	def( "segment", &::segmentWrapper, segmentOverLoads() );
	def( "merge", &::mergeWrapper, ( arg_( "meshes" ), arg_( "canceller" ) = object() ) );
	def( "triangulate", &triangulateWrapper, (arg_("mesh"), arg_( "canceller" ) = object() ) );
	def( "connectedVertices", &connectedVerticesWrapper, ( arg_("mesh"), arg_( "canceller" ) = object() ) );
}

} // namespace IECoreSceneModule

