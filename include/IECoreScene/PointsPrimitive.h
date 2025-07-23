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

#ifndef IECORESCENE_POINTSPRIMITIVE_H
#define IECORESCENE_POINTSPRIMITIVE_H

#include "IECoreScene/Export.h"
#include "IECoreScene/Primitive.h"

#include "IECore/VectorTypedData.h"

namespace IECoreScene
{

/// The PointsPrimitive class defines a simple pointcloud for rendering.
/// A valid PointsPrimitive should have a primitive variable named "P"
/// containing V3fVectorData of interpolation type Vertex. It optionally
/// may also contain a primitive variable "r", containing FloatVectorData
/// defining the radii of the points - this is also of interpolation type Vertex.
/// \ingroup geometryGroup
class IECORESCENE_API PointsPrimitive : public Primitive
{
	public:

		PointsPrimitive( size_t numPoints = 0 );
		PointsPrimitive( IECore::V3fVectorDataPtr points, IECore::FloatVectorDataPtr radii = nullptr );
		~PointsPrimitive() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( PointsPrimitive, PointsPrimitiveTypeId, Primitive );

		/// Returns the number of points the primitive is expected
		/// to have. This defines the expected dimensions of Primitive::Variable
		/// data.
		size_t getNumPoints() const;
		/// Sets the number of points the primitive is expected to have.
		/// This defines the expected dimensions of Primitive::Variable
		/// data.
		void setNumPoints( size_t n );

		Imath::Box3f bound() const override;

		/// Returns numPoints() for Vertex, Varying, and FaceVarying interpolations,
		/// and 1 for all other types.
		size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const override;

		void topologyHash( IECore::MurmurHash &h ) const override;

	private:

		static const unsigned int m_ioVersion;

		size_t m_numPoints;

};

IE_CORE_DECLAREPTR( PointsPrimitive );

}

#endif // IECORESCENE_POINTSPRIMITIVE_H
