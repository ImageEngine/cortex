//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TOHOUDINIGROUPCONVERTER_H
#define IECOREHOUDINI_TOHOUDINIGROUPCONVERTER_H

#include "IECore/SimpleTypedParameter.h"
#include "IECoreScene/Group.h"

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"

namespace IECoreHoudini
{

/// Converter which converts from an IECore::Group to a Houdini GU_Detail
class ToHoudiniGroupConverter : public IECoreHoudini::ToHoudiniGeometryConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniGroupConverter, ToHoudiniGroupConverterTypeId, IECoreHoudini::ToHoudiniGeometryConverter );

		ToHoudiniGroupConverter( const IECore::Object *object );

		virtual ~ToHoudiniGroupConverter();

		IECore::M44fParameter *transformParameter();
		const IECore::M44fParameter *transformParameter() const;

	protected :

		/// performs conversion from the IECore::Group into the given GU_Detail
		virtual bool doConversion( const IECore::Object *object, GU_Detail *geo ) const;

	private :

		static ToHoudiniGeometryConverter::Description<ToHoudiniGroupConverter> m_description;

		IECore::M44fParameterPtr m_transformParameter;

};

// register our converter
IE_CORE_DECLAREPTR( ToHoudiniGroupConverter );

}

#endif // IECOREHOUDINI_TOHOUDINIGROUPCONVERTER_H
