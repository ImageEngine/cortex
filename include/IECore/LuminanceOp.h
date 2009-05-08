//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_LUMINANCEOP_H
#define IECORE_LUMINANCEOP_H

#include "IECore/PrimitiveOp.h"

namespace IECore
{

/// The LuminanceOp calculates a primvar representing luminance.
class LuminanceOp : public PrimitiveOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( LuminanceOp, PrimitiveOp );

		LuminanceOp();
		virtual ~LuminanceOp();

		StringParameterPtr colorPrimVarParameter();
		ConstStringParameterPtr colorPrimVarParameter() const;

		StringParameterPtr redPrimVarParameter();
		ConstStringParameterPtr redPrimVarParameter() const;

		StringParameterPtr greenPrimVarParameter();
		ConstStringParameterPtr greenPrimVarParameter() const;

		StringParameterPtr bluePrimVarParameter();
		ConstStringParameterPtr bluePrimVarParameter() const;

		Color3fParameterPtr weightsParameter();
		ConstColor3fParameterPtr weightsParameter() const;

		StringParameterPtr luminancePrimVarParameter();
		ConstStringParameterPtr luminancePrimVarParameter() const;

		BoolParameterPtr removeColorPrimVarsParameter();
		ConstBoolParameterPtr removeColorPrimVarsParameter() const;

	protected :

		virtual void modifyPrimitive( PrimitivePtr primitive, ConstCompoundObjectPtr operands );

	private :

		template <typename T>
		void calculate( const T *r, const T *g, const T *b, int steps[3], int size, T *y );

		StringParameterPtr m_colorPrimVarParameter;
		StringParameterPtr m_redPrimVarParameter;
		StringParameterPtr m_greenPrimVarParameter;
		StringParameterPtr m_bluePrimVarParameter;
		Color3fParameterPtr m_weightsParameter;
		BoolParameterPtr m_removeColorPrimVarsParameter;

		StringParameterPtr m_luminancePrimVarParameter;

};

IE_CORE_DECLAREPTR( LuminanceOp );

} // namespace IECore

#endif // IECORE_LUMINANCEOP_H
