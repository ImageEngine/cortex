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

#ifndef IECORE_COLORTRANSFORMOP_H
#define IECORE_COLORTRANSFORMOP_H

#include "IECore/PrimitiveOp.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

/// The ColorTransformOp defines a base class for Ops which
/// transform the colors of a Primitive. By default the "Cs" or "R",
/// "G", and "B" channels are transformed but this can be changed
/// using the appropriate parameters.
class ColorTransformOp : public PrimitiveOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( ColorTransformOp, PrimitiveOp );

		ColorTransformOp( const std::string &name, const std::string &description );
		virtual ~ColorTransformOp();

		StringParameterPtr colorPrimVarParameter();
		ConstStringParameterPtr colorPrimVarParameter() const;

		StringParameterPtr redPrimVarParameter();
		ConstStringParameterPtr redPrimVarParameter() const;

		StringParameterPtr greenPrimVarParameter();
		ConstStringParameterPtr greenPrimVarParameter() const;

		StringParameterPtr bluePrimVarParameter();
		ConstStringParameterPtr bluePrimVarParameter() const;

		StringParameterPtr alphaPrimVarParameter();
		ConstStringParameterPtr alphaPrimVarParameter() const;

		BoolParameterPtr premultipliedParameter();
		ConstBoolParameterPtr premultipliedParameter() const;

	protected :

		/// Called once per operation. This is an opportunity to perform any preprocessing
		/// necessary before many calls to transform() are made.
		virtual void begin( ConstCompoundObjectPtr operands );
		/// Called once per color element (pixel for ImagePrimitives).
		/// Must be implemented by subclasses to transform color in place.
		virtual void transform( Imath::Color3f &color ) const = 0;
		/// Called once per operation, after all calls to transform() have been made - even if
		// /transform() throws an exception. This is an opportunity to perform any cleanup necessary.
		virtual void end();

	private :

		/// Implemented in terms of begin(), transform() and end(), which should be implemented
		/// appropriately by subclasses.
		virtual void modifyPrimitive( PrimitivePtr primitive, ConstCompoundObjectPtr operands );

		template<typename T>
		const typename T::BaseType *alphaData( PrimitivePtr primitive, size_t requiredElements );
		template <typename T>
		void transformSeparate( PrimitivePtr primitive, ConstCompoundObjectPtr operands, typename T::Ptr r, typename T::Ptr g, typename T::Ptr b );
		template <typename T>
		void transformInterleaved( PrimitivePtr primitive, ConstCompoundObjectPtr operands, typename T::Ptr colors );

		StringParameterPtr m_colorPrimVarParameter;
		StringParameterPtr m_redPrimVarParameter;
		StringParameterPtr m_greenPrimVarParameter;
		StringParameterPtr m_bluePrimVarParameter;
		StringParameterPtr m_alphaPrimVarParameter;
		BoolParameterPtr m_premultipliedParameter;

};

IE_CORE_DECLAREPTR( ColorTransformOp );

} // namespace IECore

#endif // IECORE_COLORTRANSFORMOP_H
