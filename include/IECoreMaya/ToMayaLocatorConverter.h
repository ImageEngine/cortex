//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013 Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_TOMAYALOCATORCONVERTER_H
#define IECOREMAYA_TOMAYALOCATORCONVERTER_H

#include "IECoreMaya/ToMayaObjectConverter.h"

namespace IECoreMaya
{

/// Converts an IECore::CoordinateSystem to a Maya locator.
/// Converting to an existing locator or parent of a locator will alter
/// the locator without renaming the locator. Converting to a transform
/// that doesn't contain a locator will create a new locator parented
/// under that transform and named according to coordinateSystem->getName()
/// \ingroup conversionGroup
class ToMayaLocatorConverter : public ToMayaObjectConverter
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToMayaLocatorConverter, ToMayaLocatorConverterTypeId, ToMayaObjectConverter );

		ToMayaLocatorConverter( IECore::ConstObjectPtr object );

	protected:

		virtual bool doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const;

	private:

		typedef ToMayaObjectConverterDescription<ToMayaLocatorConverter> Description;
		static Description g_description;
};

} // namespace IECoreMaya

#endif // IECOREMAYA_TOMAYALOCATORCONVERTER_H
