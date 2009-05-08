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

#ifndef IECORE_LINEARTOCINEON_H
#define IECORE_LINEARTOCINEON_H

#include "IECore/ChannelOp.h"
#include "IECore/NumericParameter.h"
#include "IECore/ColorSpaceTransformOp.h"

namespace IECore
{

/// Applies linear to SRGB conversion on ImagePrimitive channels.
class LinearToCineonOp : public ChannelOp
{
	public:

		LinearToCineonOp();
		virtual ~LinearToCineonOp();

		IE_CORE_DECLARERUNTIMETYPED( LinearToCineonOp, ChannelOp );

		FloatParameterPtr filmGammaParameter();
		ConstFloatParameterPtr filmGammaParameter() const;

		IntParameterPtr refWhiteValParameter();
		ConstIntParameterPtr refWhiteValParameter() const;

		IntParameterPtr refBlackValParameter();
		ConstIntParameterPtr refBlackValParameter() const;

	protected :

		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels );

		FloatParameterPtr m_filmGamma;
		IntParameterPtr m_refWhiteVal;
		IntParameterPtr m_refBlackVal;

		struct Converter;

	private :

		static ColorSpaceTransformOp::ColorSpaceDescription<LinearToCineonOp> g_colorSpaceDescription;

};

IE_CORE_DECLAREPTR( LinearToCineonOp );


} // namespace IECore

#endif // IECORE_LINEARTOCINEON_H

