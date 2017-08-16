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

#ifndef IECOREIMAGE_LUMINANCEOP_H
#define IECOREIMAGE_LUMINANCEOP_H

#include "IECore/ModifyOp.h"

#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

/// The LuminanceOp calculates a channel representing luminance.
/// \ingroup imageProcessingGroup
class IECOREIMAGE_API LuminanceOp : public IECore::ModifyOp
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( LuminanceOp, LuminanceOpTypeId, IECore::ModifyOp );

		LuminanceOp();
		virtual ~LuminanceOp();

		IECore::StringParameter *colorChannelParameter();
		const IECore::StringParameter *colorChannelParameter() const;

		IECore::StringParameter *redChannelParameter();
		const IECore::StringParameter *redChannelParameter() const;

		IECore::StringParameter *greenChannelParameter();
		const IECore::StringParameter *greenChannelParameter() const;

		IECore::StringParameter *blueChannelParameter();
		const IECore::StringParameter *blueChannelParameter() const;

		IECore::Color3fParameter *weightsParameter();
		const IECore::Color3fParameter *weightsParameter() const;

		IECore::StringParameter *luminanceChannelParameter();
		const IECore::StringParameter *luminanceChannelParameter() const;

		IECore::BoolParameter *removeColorChannelsParameter();
		const IECore::BoolParameter *removeColorChannelsParameter() const;

	protected :

		virtual void modify( IECore::Object *object, const IECore::CompoundObject *operands );

	private :

		template <typename T>
		void calculate( const T *r, const T *g, const T *b, int steps[3], int size, T *y );

		IECore::StringParameterPtr m_colorChannelParameter;
		IECore::StringParameterPtr m_redChannelParameter;
		IECore::StringParameterPtr m_greenChannelParameter;
		IECore::StringParameterPtr m_blueChannelParameter;
		IECore::Color3fParameterPtr m_weightsParameter;
		IECore::BoolParameterPtr m_removeColorChannelsParameter;

		IECore::StringParameterPtr m_luminanceChannelParameter;

};

IE_CORE_DECLAREPTR( LuminanceOp );

} // namespace IECoreImage

#endif // IECOREIMAGE_LUMINANCEOP_H
