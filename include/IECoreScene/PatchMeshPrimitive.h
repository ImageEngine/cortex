//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_PATCHMESHPRIMITIVE_H
#define IECORESCENE_PATCHMESHPRIMITIVE_H

#include "IECore/VectorTypedData.h"
#include "IECore/CubicBasis.h"
#include "IECoreScene/Export.h"
#include "IECoreScene/Primitive.h"

namespace IECoreScene
{

/// A Primitive representation of an RiPatchMesh
/// \ingroup geometryGroup
class IECORESCENE_API PatchMeshPrimitive : public Primitive
{
	public :

		PatchMeshPrimitive();

		/// Copy of p is taken.
		PatchMeshPrimitive(
		        unsigned int uPoints,
		        unsigned int vPoints,
		        const IECore::CubicBasisf &uBasis=IECore::CubicBasisf::linear(),
		        const IECore::CubicBasisf &vBasis=IECore::CubicBasisf::linear(),
		        bool uPeriodic = false,
		        bool vPeriodic = false,
		        IECore::ConstV3fVectorDataPtr p = nullptr
		);

		~PatchMeshPrimitive() override;

		IE_CORE_DECLAREEXTENSIONOBJECT( PatchMeshPrimitive, PatchMeshPrimitiveTypeId, Primitive );

		/// Returns the number of control points in U or V
		unsigned int uPoints() const;
		unsigned int vPoints() const;

		/// Returns the number of sub-patches in U or V
		unsigned int uPatches() const;
		unsigned int vPatches() const;

		const IECore::CubicBasisf &uBasis() const;
		const IECore::CubicBasisf &vBasis() const;

		/// Returns whether the U or V wrap mode specifies "periodic"
		bool uPeriodic() const;
		bool vPeriodic() const;

		void render( Renderer *renderer ) const override;

		/// Follows the RenderMan specification for variable sizes.
		size_t variableSize( PrimitiveVariable::Interpolation interpolation ) const override;

		void topologyHash( IECore::MurmurHash &h ) const override;

	protected :

		bool m_uLinear, m_vLinear;
		unsigned int m_uPoints, m_vPoints;
		IECore::CubicBasisf m_uBasis, m_vBasis;
		bool m_uPeriodic, m_vPeriodic;

	private :

		static const unsigned int m_ioVersion;

};

IE_CORE_DECLAREPTR( PatchMeshPrimitive );

} // namespace IECoreScene

#endif // IECORESCENE_PATCHMESHPRIMITIVE_H
