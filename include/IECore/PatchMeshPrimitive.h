//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_PATCHMESHPRIMITIVE_H
#define IECORE_PATCHMESHPRIMITIVE_H

#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"
#include "IECore/CubicBasis.h"

namespace IECore
{

/// A Primitive representation of an RiPatchMesh
class PatchMeshPrimitive : public Primitive
{
	public :

		PatchMeshPrimitive();
		
		/// Copy of p is taken.
		PatchMeshPrimitive(		        
		        unsigned int uPoints,
		        unsigned int vPoints,
		        const CubicBasisf &uBasis=CubicBasisf::linear(),
		        const CubicBasisf &vBasis=CubicBasisf::linear(),
		        bool uPeriodic = false,
		        bool vPeriodic = false,
		        ConstV3fVectorDataPtr p = 0
		);
		
		virtual ~PatchMeshPrimitive();

		IE_CORE_DECLAREOBJECT( PatchMeshPrimitive, Primitive );
		
		/// Returns the number of control points in U or V
		unsigned int uPoints() const;
		unsigned int vPoints() const;		

		/// Returns the number of sub-patches in U or V
		unsigned int uPatches() const;
		unsigned int vPatches() const;

		const CubicBasisf &uBasis() const;
		const CubicBasisf &vBasis() const;

		/// Returns whether the U or V wrap mode specifies "periodic"
		bool uPeriodic() const;
		bool vPeriodic() const;

		virtual void render( RendererPtr renderer ) const;
		
		/// Follows the RenderMan specification for variable sizes.
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const;

	protected :

		bool m_linear;
		unsigned int m_uPoints, m_vPoints;
		CubicBasisf m_uBasis, m_vBasis;
		bool m_uPeriodic, m_vPeriodic;

	private :

		static const unsigned int m_ioVersion;

};

IE_CORE_DECLAREPTR( PatchMeshPrimitive );

} // namespace IECore

#endif // IECORE_PATCHMESHPRIMITIVE_H
