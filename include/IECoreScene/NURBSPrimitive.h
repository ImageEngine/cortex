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

#ifndef IECORESCENE_NURBSPRIMITIVE_H
#define IECORESCENE_NURBSPRIMITIVE_H

#include "IECoreScene/Export.h"
#include "IECoreScene/Primitive.h"

#include "IECore/VectorTypedData.h"

namespace IECoreScene
{

/// The NURBSPrimitive class represents a single NURBS surface
/// \todo createPlane, createSphere static member functions
/// \ingroup geometryGroup
class IECORESCENE_API NURBSPrimitive : public Primitive
{

	public:

		IE_CORE_DECLAREEXTENSIONOBJECT( NURBSPrimitive, NURBSPrimitiveTypeId, Primitive );

		NURBSPrimitive();
		/// Copies of all data are taken.
		NURBSPrimitive( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax,
			int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, IECore::ConstV3fVectorDataPtr p = nullptr );

		//! @name Topology access
		/// These functions allow access to get and set topology after construction.
		/////////////////////////////////////////////////////////////////////////////
		//@{
		int uOrder() const;
		const IECore::FloatVectorData *uKnot() const;
		float uMin() const;
		float uMax() const;
		int uVertices() const;
		int uSegments() const;
		int vOrder() const;
		const IECore::FloatVectorData *vKnot() const;
		float vMin() const;
		float vMax() const;
		int vVertices() const;
		int vSegments() const;
		/// \todo Remove virtual-ness for Cortex 9.
		virtual void setTopology(  int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax,
			int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax );
		//@}

		size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const override;

		void topologyHash( IECore::MurmurHash &h ) const override;

	private:

		static const unsigned int m_ioVersion;

		int m_uOrder;
		IECore::FloatVectorDataPtr m_uKnot;
		float m_uMin, m_uMax;
		int m_vOrder;
		IECore::FloatVectorDataPtr m_vKnot;
		float m_vMin, m_vMax;

};

IE_CORE_DECLAREPTR( NURBSPrimitive );

}

#endif // IECORESCENE_NURBSPRIMITIVE_H
