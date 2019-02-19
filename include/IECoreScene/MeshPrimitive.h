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

#ifndef IECORESCENE_MESHPRIMITIVE_H
#define IECORESCENE_MESHPRIMITIVE_H

#include "IECoreScene/Export.h"
#include "IECoreScene/Primitive.h"

#include "IECore/VectorTypedData.h"

namespace IECoreScene
{

IE_CORE_FORWARDDECLARE( MeshPrimitive )

class PolygonIterator;

/// The MeshPrimitive represents a collection of polygonal faces. The faces should
/// be planar and may not have holes, but can have concavities. By default a polygon
/// is considered frontfacing if it's vertices wind in an anticlockwise direction from the
/// point of view of the camera - this can be changed at the point of rendering by
/// setting attributes in the Renderer. This definition of winding order is the same as
/// the OpenGL and Maya conventions but the opposite of RenderMan's.
/// \ingroup geometryGroup
class IECORESCENE_API MeshPrimitive : public Primitive
{

	public:

		IE_CORE_DECLAREEXTENSIONOBJECT( MeshPrimitive, MeshPrimitiveTypeId, Primitive );

		/// Construct a MeshPrimitive with no faces.
		MeshPrimitive();
		/// Construct a MeshPrimitive. The number of faces specified by verticesPerFace->readable()->size().
		/// Copies of the IntVectorData objects are taken rather than references to the initial data.
		MeshPrimitive( IECore::ConstIntVectorDataPtr verticesPerFace, IECore::ConstIntVectorDataPtr vertexIds,
			const std::string &interpolation = "linear", IECore::V3fVectorDataPtr p = nullptr );

		//! @name Topology access
		/// These functions allow access to get and set topology after construction.
		/////////////////////////////////////////////////////////////////////////////
		//@{
		size_t numFaces() const;
		const IECore::IntVectorData *verticesPerFace() const;
		int minVerticesPerFace() const;
		int maxVerticesPerFace() const;
		const IECore::IntVectorData *vertexIds() const;
		const std::string &interpolation() const;
		void setTopology( IECore::ConstIntVectorDataPtr verticesPerFace, IECore::ConstIntVectorDataPtr vertexIds, const std::string &interpolation = "linear" );
		void setTopologyUnchecked( IECore::ConstIntVectorDataPtr verticesPerFace, IECore::ConstIntVectorDataPtr vertexIds, size_t numVertices, const std::string &interpolation = "linear" );
		void setInterpolation( const std::string &interpolation );
		PolygonIterator faceBegin();
		PolygonIterator faceEnd();
		//@}

		//! @name Corners and creases
		/// These functions allow corners and creases to be added to subdivision
		/// surfaces.
		/////////////////////////////////////////////////////////////////////////////
		//@{
		void setCorners( const IECore::IntVectorData *ids, const IECore::FloatVectorData *sharpnesses );
		const IECore::IntVectorData *cornerIds() const;
		const IECore::FloatVectorData *cornerSharpnesses() const;
		void removeCorners();

		void setCreases( const IECore::IntVectorData *lengths, const IECore::IntVectorData *ids, const IECore::FloatVectorData *sharpnesses );
		const IECore::IntVectorData *creaseLengths() const;
		const IECore::IntVectorData *creaseIds() const;
		const IECore::FloatVectorData *creaseSharpnesses() const;
		void removeCreases();
		//@}

		size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const override;

		/// Render the mesh
		void render( Renderer *renderer ) const override;

		/// Creates a box mesh.
		/// \todo Add subdivisions and texture coordinates.
		static MeshPrimitivePtr createBox( const Imath::Box3f &b );
		/// Creates a plane at z=0, with the geometric normal facing down positive z.
		static MeshPrimitivePtr createPlane( const Imath::Box2f &b, const Imath::V2i &divisions = Imath::V2i( 1 ) );
		/// Creates a sphere
		static MeshPrimitivePtr createSphere( float radius, float zMin = -1.0f, float zMax = 1.0f, float thetaMax = 360.0f, const Imath::V2i &divisions = Imath::V2i( 20, 40 ) );

		void topologyHash( IECore::MurmurHash &h ) const override;

	private:

		static const unsigned int m_ioVersion;

		IECore::IntVectorDataPtr m_verticesPerFace;
		IECore::IntVectorDataPtr m_vertexIds;
		size_t m_numVertices;
		std::string m_interpolation;

		void computeMinMaxVertsPerFace() const;
		mutable int m_minVerticesPerFace;
		mutable int m_maxVerticesPerFace;

		IECore::ConstIntVectorDataPtr m_cornerIds;
		IECore::ConstFloatVectorDataPtr m_cornerSharpnesses;

		IECore::ConstIntVectorDataPtr m_creaseLengths;
		IECore::ConstIntVectorDataPtr m_creaseIds;
		IECore::ConstFloatVectorDataPtr m_creaseSharpnesses;

};

}

#endif // IECORE_MESHPRIMITIVE
