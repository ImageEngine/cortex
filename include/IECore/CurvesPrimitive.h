//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_CURVESPRIMITIVE_H
#define IECORE_CURVESPRIMITIVE_H

#include "IECore/Export.h"
#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CubicBasis.h"

namespace IECore
{

/// The CurvesPrimitive class represents a number of curves with either cubic
/// or linear interpolation. It is equivalent to the RenderMan RiCurves primitive.
/// \ingroup geometryGroup
class IECORE_API CurvesPrimitive : public Primitive
{
	public :

		CurvesPrimitive();
		/// Copies of vertsPerCurve and p are taken.
		CurvesPrimitive( ConstIntVectorDataPtr vertsPerCurve, const CubicBasisf &basis=CubicBasisf::linear(), bool periodic = false, ConstV3fVectorDataPtr p = 0 );
		virtual ~CurvesPrimitive();

		IE_CORE_DECLAREOBJECT( CurvesPrimitive, Primitive );

		size_t numCurves() const;
		const IntVectorData *verticesPerCurve() const;
		const CubicBasisf &basis() const;
		bool periodic() const;
		void setTopology( ConstIntVectorDataPtr verticesPerCurve, const CubicBasisf &basis, bool periodic );

		virtual void render( Renderer *renderer ) const;
		/// Follows the RenderMan specification for variable sizes.
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const;
		/// Returns the variable size for a single curve.
		size_t variableSize( PrimitiveVariable::Interpolation interpolation, unsigned curveIndex ) const;

		/// Returns the number of segments in a given curve.
		unsigned numSegments( unsigned curveIndex ) const;
		/// Returns the number of segments of a curve with the given topology.
		static unsigned numSegments( const CubicBasisf &basis, bool periodic, unsigned numVerts );

		/// Creates a wireframe box of the specified size.
		static Ptr createBox( const Imath::Box3f &b );
		
		virtual void topologyHash( MurmurHash &h ) const;

	protected :

		/// Throws an exception if numVerts is an inappropriate number for the current basis.
		static unsigned int numSegments( bool linear, int step, bool periodic, int numVerts );

		CubicBasisf m_basis;
		bool m_linear;
		bool m_periodic;
		IntVectorDataPtr m_vertsPerCurve;
		unsigned m_numVerts;
		unsigned m_numFaceVarying;

	private :

		static const unsigned int m_ioVersion;

};

IE_CORE_DECLAREPTR( CurvesPrimitive );

} // namespace IECore

#endif // IECORE_CURVESPRIMITIVE_H
