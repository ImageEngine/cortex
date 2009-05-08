//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_MATRIXMULTIPLYOP_H
#define IE_CORE_MATRIXMULTIPLYOP_H

#include "IECore/ModifyOp.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// The MatrixMultiplyOp applies a matrix transformation to an array of vectors.
/// It can transform the vectors as if they were points, vectors or normals.
class MatrixMultiplyOp : public ModifyOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( MatrixMultiplyOp, ModifyOp );

		MatrixMultiplyOp();
		virtual ~MatrixMultiplyOp();

		ObjectParameterPtr matrixParameter();
		ConstObjectParameterPtr matrixParameter() const;

		/// An enumeration for the different ways of interpreting
		/// the vectors to be transformed. These are the acceptable
		/// values for modeParameter()->setTypedValue().
		enum Mode
		{
			Point = 0,
			Vector = 1,
			Normal = 2
		};

		IntParameterPtr modeParameter();
		ConstIntParameterPtr modeParameter() const;

	protected :

		virtual void modify( ObjectPtr toModify, ConstCompoundObjectPtr operands );

	private :

		ObjectParameterPtr m_matrixParameter;
		IntParameterPtr m_modeParameter;

};

IE_CORE_DECLAREPTR( MatrixMultiplyOp );

} // namespace IECore

#endif // IE_CORE_MATRIXMULTIPLYOP_H


