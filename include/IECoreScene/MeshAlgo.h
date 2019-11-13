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

#ifndef IECORESCENE_MESHALGO_H
#define IECORESCENE_MESHALGO_H

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PointsPrimitive.h"
#include "IECoreScene/PrimitiveVariable.h"

#include <utility>

namespace IECoreScene
{

namespace MeshAlgo
{

/// Calculate the normals of a mesh primitive.
IECORESCENE_API PrimitiveVariable calculateNormals( const MeshPrimitive *mesh, PrimitiveVariable::Interpolation interpolation = PrimitiveVariable::Vertex, const std::string &position = "P" );

/// TODO: remove this compatibility function:
IECORESCENE_API std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangents( const MeshPrimitive *mesh, const std::string &uvSet = "uv", bool orthoTangents = true, const std::string &position = "P" );
/// Calculate the surface tangent vectors of a mesh primitive based on UV information
IECORESCENE_API std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromUV( const MeshPrimitive *mesh, const std::string &uvSet = "uv", const std::string &position = "P", bool orthoTangents = true, bool leftHanded = false );
/// Calculate the surface tangent vectors of a mesh primitive based on the first neighbor edge
IECORESCENE_API std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromFirstEdge( const MeshPrimitive *mesh, const std::string &position = "P", const std::string &normal = "N", bool orthoTangents = true, bool leftHanded = false );
/// Calculate the surface tangent vectors of a mesh primitive based on the primitives centroid
IECORESCENE_API std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromPrimitiveCentroid( const MeshPrimitive *mesh, const std::string &position = "P", const std::string &normal = "N", bool orthoTangents = true, bool leftHanded = false );
/// Calculate the surface tangent vectors of a mesh primitive based on the first two adjacent edges
IECORESCENE_API std::pair<PrimitiveVariable, PrimitiveVariable> calculateTangentsFromTwoEdges( const MeshPrimitive *mesh, const std::string &position = "P", const std::string &normal = "N", bool orthoTangents = true, bool leftHanded = false );

/// Calculate the face area of a mesh primitive.
IECORESCENE_API PrimitiveVariable calculateFaceArea( const MeshPrimitive *mesh, const std::string &position = "P" );

/// Calculate the face texture area of a mesh primitive based on the specified UV set.
IECORESCENE_API PrimitiveVariable calculateFaceTextureArea( const MeshPrimitive *mesh, const std::string &uvSet = "uv", const std::string &position = "P" );

/// Calculate the distortions (expansion and contraction) on the mesh edges
/// The first return value is the float distortion between the two position variables.
/// The second return value is the V2f distortion of the UV set.
IECORESCENE_API std::pair<PrimitiveVariable, PrimitiveVariable> calculateDistortion( const MeshPrimitive *mesh, const std::string &uvSet = "uv", const std::string &referencePosition = "Pref", const std::string &position = "P" );

IECORESCENE_API void resamplePrimitiveVariable( const MeshPrimitive *mesh, PrimitiveVariable& primitiveVariable, PrimitiveVariable::Interpolation interpolation );

/// create a new MeshPrimitive deleting faces from the input MeshPrimitive based on the facesToDelete uniform (int|float|bool) PrimitiveVariable
/// When invert is set then zeros in facesToDelete indicate which faces should be deleted
IECORESCENE_API MeshPrimitivePtr deleteFaces( const MeshPrimitive *meshPrimitive, const PrimitiveVariable &facesToDelete, bool invert = false );

/// Reverses the winding order of each face by adjusting the vertex ids and updating all FaceVarying
/// primitive variables to match.
IECORESCENE_API void reverseWinding( MeshPrimitive *meshPrimitive );

/// Reorder the vertices of a mesh based on an initial choice of 3 vertices
IECORESCENE_API void reorderVertices( MeshPrimitive *mesh, int id0, int id1, int id2 );

/// Distributes points over a mesh using an IECore::PointDistribution in UV space
/// and mapping it to 3d space. It gives a fairly even distribution regardless of
/// vertex spacing, provided the UVs are well layed out.
IECORESCENE_API PointsPrimitivePtr distributePoints( const MeshPrimitive *mesh, float density = 100.0, const Imath::V2f &offset = Imath::V2f( 0 ), const std::string &densityMask = "density", const std::string &uvSet = "uv", const std::string &position = "P" );

/// Segment the input mesh in to N meshes based on the N unique values contained in the segmentValues argument.
/// If segmentValues isn't supplied then primitive is split into the unique values contained in the primitiveVariable.
/// The primitiveVariable must have 'Uniform' iterpolation and match the base type of the VectorTypedData in the segmentValues.
/// Specifying the two parameters segmentValues & primitiveVariable allows for a subset of meshes to be created, rather than
/// completely segmententing the mesh based on the unique values in a primitive variable.
IECORESCENE_API std::vector<MeshPrimitivePtr> segment( const MeshPrimitive *mesh, const PrimitiveVariable &primitiveVariable, const IECore::Data *segmentValues = nullptr );

/// Merge the input meshes into a single mesh.
/// Any PrimitiveVariables that exist will be combined or extended using a default value.
IECORESCENE_API MeshPrimitivePtr merge( const std::vector<const MeshPrimitive *> &meshes );

/// Generate a new triangulated MeshPrimitive
/// If throwExceptions is true the input mesh is validated to ensure all polygons are convex planar and only then the
/// tolerance parameter is used to define a floating point epsilon for these checks.
IECORESCENE_API MeshPrimitivePtr triangulate( const MeshPrimitive *mesh, float tolerance = 1e-6f, bool throwExceptions = false);


/// Generate a list of connected vertices per vertex
/// The first vector contains a flat list of all the indices of the connected neighbor vertices.
///	The second one holds an offset index for every vertex. Note that the offset indices vector skips the first offset index (since it's 0)
IECORESCENE_API	std::pair<IECore::IntVectorDataPtr, IECore::IntVectorDataPtr> connectedVertices( const IECoreScene::MeshPrimitive *mesh );

} // namespace MeshAlgo

} // namespace IECoreScene

#endif // IECORESCENE_MESHALGO_H
