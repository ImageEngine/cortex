//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_LINEARTOALEXALOGCOP_H
#define IECORE_LINEARTOALEXALOGCOP_H

#include "IECore/Export.h"
#include "IECore/ChannelOp.h"
#include "IECore/ColorSpaceTransformOp.h"

namespace IECore
{

/// Applies linear to Alexa (V3) Log C conversion on ImagePrimitive channels.
/// \ingroup imageProcessingGroup
class IECORE_API LinearToAlexaLogcOp : public ChannelOp
{
	public:

		LinearToAlexaLogcOp();
		virtual ~LinearToAlexaLogcOp();

		IE_CORE_DECLARERUNTIMETYPED( LinearToAlexaLogcOp, ChannelOp );

	protected :

		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels );

		struct Converter;

	private :

		static ColorSpaceTransformOp::ColorSpaceDescription<LinearToAlexaLogcOp> g_colorSpaceDescription;
};

IE_CORE_DECLAREPTR( LinearToAlexaLogcOp );


} // namespace IECore

#endif // IECORE_LINEARTOALEXALOGCOP_H

