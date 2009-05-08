//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_TRANSFORMOP_H
#define IECORE_TRANSFORMOP_H

#include "IECore/PrimitiveOp.h"
#include "IECore/ObjectParameter.h"
#include "IECore/VectorTypedParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( MatrixMultiplyOp )

/// The TransformOp class applies a matrix transformation to
/// selected PrimitiveVariables on a Primitive. By default it
/// transforms "P" and "N" data.
class TransformOp : public PrimitiveOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( TransformOp, PrimitiveOp );

		TransformOp();

		ObjectParameterPtr matrixParameter();
		ConstObjectParameterPtr matrixParameter() const;

		StringVectorParameterPtr pointPrimVarsParameter();
		ConstStringVectorParameterPtr pointPrimVarsParameter() const;

		StringVectorParameterPtr vectorPrimVarsParameter();
		ConstStringVectorParameterPtr vectorPrimVarsParameter() const;

		StringVectorParameterPtr normalPrimVarsParameter();
		ConstStringVectorParameterPtr normalPrimVarsParameter() const;

	protected :

		virtual void modifyPrimitive( PrimitivePtr primitive, ConstCompoundObjectPtr operands );

	private :

		MatrixMultiplyOpPtr m_multiplyOp;
		StringVectorParameterPtr m_pointPrimVarsParameter;
		StringVectorParameterPtr m_vectorPrimVarsParameter;
		StringVectorParameterPtr m_normalPrimVarsParameter;

};

IE_CORE_DECLAREPTR( TransformOp );

} // namespace IECore

#endif // IECORE_TRANSFORMOP_H
