//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012 Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_TOMAYACAMERACONVERTER_H
#define IECOREMAYA_TOMAYACAMERACONVERTER_H

#include "IECoreMaya/ToMayaObjectConverter.h"

namespace IECoreMaya
{

/// Converts an IECore::Camera to a Maya camera.
/// Converting to an existing cameraShape or parent of a cameraShape will alter
/// the camera settings without renaming the shape. Converting to a transform
/// that doesn't contain a cameraShape will create a new cameraShape parented
/// under that transform and named according to camera->getName()
/// \ingroup conversionGroup
class IECOREMAYA_API ToMayaCameraConverter : public ToMayaObjectConverter
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToMayaCameraConverter, ToMayaCameraConverterTypeId, ToMayaObjectConverter );

		ToMayaCameraConverter( IECore::ConstObjectPtr object );

	protected:

		virtual bool doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const;

	private:

		typedef ToMayaObjectConverterDescription<ToMayaCameraConverter> Description;
		static Description g_description;
};

} // namespace IECoreMaya

#endif // IECOREMAYA_TOMAYACAMERACONVERTER_H
