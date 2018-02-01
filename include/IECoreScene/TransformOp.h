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

#ifndef IECORESCENE_TRANSFORMOP_H
#define IECORESCENE_TRANSFORMOP_H

#include "IECoreScene/Export.h"
#include "IECoreScene/PrimitiveOp.h"

#include "IECore/MatrixMultiplyOp.h"
#include "IECore/ObjectParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECoreScene
{

/// The TransformOp class applies a matrix transformation to a Primitive,
/// using the GeometricData::Interpretation of the PrimitiveVariable data
/// to determine the approriate transformation method. Only the variables
/// specified by the PrimVars parameter will be modified.
/// \ingroup geometryProcessingGroup
class IECORESCENE_API TransformOp : public PrimitiveOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( TransformOp, TransformOpTypeId, PrimitiveOp );

		TransformOp();

		IECore::ObjectParameter * matrixParameter();
		const IECore::ObjectParameter * matrixParameter() const;

		IECore::StringVectorParameter *primVarsParameter();
		const IECore::StringVectorParameter *primVarsParameter() const;

	protected :

		void modifyPrimitive( Primitive * primitive, const IECore::CompoundObject * operands ) override;

	private :

		IECore::MatrixMultiplyOpPtr m_multiplyOp;
		IECore::StringVectorParameterPtr m_primVarsParameter;

};

IE_CORE_DECLAREPTR( TransformOp );

} // namespace IECoreScene

#endif // IECORESCENE_TRANSFORMOP_H
