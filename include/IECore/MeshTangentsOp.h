//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MESHTANGENTSOP_H
#define IECORE_MESHTANGENTSOP_H

#include "IECore/Export.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedPrimitiveOp.h"

namespace IECore
{

/// A MeshPrimitiveOp to calculate vertex tangents.
/// \ingroup geometryProcessingGroup
class IECORE_API MeshTangentsOp : public MeshPrimitiveOp
{
	public:

		MeshTangentsOp();
		virtual ~MeshTangentsOp();

		BoolParameter * orthogonalizeTangentsParameter();
		const BoolParameter * orthogonalizeTangentsParameter() const;

		StringParameter * pPrimVarNameParameter();
		const StringParameter * pPrimVarNameParameter() const;

		StringParameter * uPrimVarNameParameter();
		const StringParameter * uPrimVarNameParameter() const;

		StringParameter * vPrimVarNameParameter();
		const StringParameter * vPrimVarNameParameter() const;
		
		StringParameter * uvIndicesPrimVarNameParameter();
		const StringParameter * uvIndicesPrimVarNameParameter() const;

		StringParameter * uTangentPrimVarNameParameter();
		const StringParameter * uTangentPrimVarNameParameter() const;

		StringParameter * vTangentPrimVarNameParameter();
		const StringParameter * vTangentPrimVarNameParameter() const;

		IE_CORE_DECLARERUNTIMETYPED( MeshTangentsOp, MeshPrimitiveOp );

	protected:

		virtual void modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands );

	private :

		StringParameterPtr m_uPrimVarNameParameter;
		StringParameterPtr m_vPrimVarNameParameter;
		StringParameterPtr m_uTangentPrimVarNameParameter;
		StringParameterPtr m_vTangentPrimVarNameParameter;

		struct CalculateTangents;
		struct HandleErrors;

};

IE_CORE_DECLAREPTR( MeshTangentsOp );


} // namespace IECore

#endif // IECORE_MESHTANGENTSOP_H



