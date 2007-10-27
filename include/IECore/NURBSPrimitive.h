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

#ifndef IECORE_NURBSPRIMITIVE_H
#define IECORE_NURBSPRIMITIVE_H

#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

/// The NURBSPrimitive class represents a single NURBS surface
/// \todo createPlane, createSphere static member functions
class NURBSPrimitive : public Primitive
{	  
	
	public:

		IE_CORE_DECLAREOBJECT( NURBSPrimitive, Primitive );

		NURBSPrimitive();
		/// Copies of all data are taken.
		NURBSPrimitive( int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax,
			int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax, ConstV3fVectorDataPtr p = 0 );

		//! @name Topology access
		/// These functions allow access to get and set topology after construction.
		/////////////////////////////////////////////////////////////////////////////
		//@{
		int uOrder() const;
		ConstFloatVectorDataPtr uKnot() const;
		float uMin() const;
		float uMax() const;
		int uVertices() const;
		int uSegments() const;
		int vOrder() const;
		ConstFloatVectorDataPtr vKnot() const;
		float vMin() const;
		float vMax() const;
		int vVertices() const;
		int vSegments() const;
		virtual void setTopology(  int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax,
			int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax );
		//@}
	
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation );

		virtual void render( RendererPtr renderer );

	private:

		static const unsigned int m_ioVersion;

		int m_uOrder;
		FloatVectorDataPtr m_uKnot;
		float m_uMin, m_uMax;
		int m_vOrder;
		FloatVectorDataPtr m_vKnot;
		float m_vMin, m_vMax;
					
};  
	
IE_CORE_DECLAREPTR( NURBSPrimitive );

}

#endif // IECORE_NURBSPRIMITIVE_H
