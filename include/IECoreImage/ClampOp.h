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

#ifndef IECOREIMAGE_CLAMPOP_H
#define IECOREIMAGE_CLAMPOP_H

#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"

#include "IECoreImage/ChannelOp.h"
#include "IECoreImage/Export.h"
#include "IECoreImage/TypeIds.h"

namespace IECoreImage
{

class IECOREIMAGE_API ClampOp : public ChannelOp
{

	public:

		ClampOp();
		virtual ~ClampOp();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ClampOp, ClampOpTypeId, ChannelOp );

		IECore::FloatParameter *minParameter();
		const IECore::FloatParameter *minParameter() const;

		IECore::FloatParameter *maxParameter();
		const IECore::FloatParameter *maxParameter() const;

		IECore::BoolParameter *enableMinToParameter();
		const IECore::BoolParameter *enableMinToParameter() const;

		IECore::FloatParameter *minToParameter();
		const IECore::FloatParameter *minToParameter() const;

		IECore::BoolParameter *enableMaxToParameter();
		const IECore::BoolParameter *enableMaxToParameter() const;

		IECore::FloatParameter *maxToParameter();
		const IECore::FloatParameter *maxToParameter() const;

	protected :

		virtual void modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels );

};

IE_CORE_DECLAREPTR( ClampOp );

} // namespace IECoreImage

#endif // IECOREIMAGE_CLAMPOP_H

