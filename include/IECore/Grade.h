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

#ifndef IECORE_GRADE_H
#define IECORE_GRADE_H

#include "OpenEXR/ImathColor.h"

#include "IECore/ColorTransformOp.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

/// The Grade implements the same operation as Nuke's grade node over the colors of a Primitive object.
/// The computation performed is:
/// A = multiply * (gain - lift) / (whitePoint - blackPoint)
/// B = offset + lift - A * blackPoint
/// output = pow( A * input + B, 1/gamma )
class Grade : public ColorTransformOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( Grade, ColorTransformOp );

		Grade();
		virtual ~Grade();

		Color3fParameterPtr blackPointParameter();
		ConstColor3fParameterPtr blackPointParameter() const;

		Color3fParameterPtr whitePointParameter();
		ConstColor3fParameterPtr whitePointParameter() const;

		Color3fParameterPtr liftParameter();
		ConstColor3fParameterPtr liftParameter() const;

		Color3fParameterPtr gainParameter();
		ConstColor3fParameterPtr gainParameter() const;

		Color3fParameterPtr multiplyParameter();
		ConstColor3fParameterPtr multiplyParameter() const;

		Color3fParameterPtr offsetParameter();
		ConstColor3fParameterPtr offsetParameter() const;

		Color3fParameterPtr gammaParameter();
		ConstColor3fParameterPtr gammaParameter() const;

		BoolParameterPtr blackClampParameter();
		ConstBoolParameterPtr blackClampParameter() const;

		BoolParameterPtr whiteClampParameter();
		ConstBoolParameterPtr whiteClampParameter() const;

	protected :

		/// initializes temporary values A, B and 1/gamma.
		virtual void begin( ConstCompoundObjectPtr operands );
		virtual void transform( Imath::Color3f &color ) const;

	private :

		Color3fParameterPtr m_blackPointParameter;
		Color3fParameterPtr m_whitePointParameter;
		Color3fParameterPtr m_liftParameter;
		Color3fParameterPtr m_gainParameter;
		Color3fParameterPtr m_multiplyParameter;
		Color3fParameterPtr m_offsetParameter;
		Color3fParameterPtr m_gammaParameter;

		BoolParameterPtr m_blackClampParameter;
		BoolParameterPtr m_whiteClampParameter;

		Imath::V3d m_A;
		Imath::V3d m_B;
		Imath::V3d m_invGamma;
};

IE_CORE_DECLAREPTR( Grade );

} // namespace IECore

#endif // IECORE_GRADE_H
