//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TOHOUDINIQUATATTRIBCONVERTER_H
#define IECOREHOUDINI_TOHOUDINIQUATATTRIBCONVERTER_H

#include "IECoreHoudini/ToHoudiniAttribConverter.h"
#include "IECoreHoudini/TypeIds.h"

#include "IECore/VectorTypedParameter.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( ToHoudiniQuatVectorAttribConverter );

/// A ToHoudiniQuatVectorAttribConverter can convert from IECore::QuatfVectorData
/// to a Houdini GA_Attribute on the provided GU_Detail.
class ToHoudiniQuatVectorAttribConverter : public ToHoudiniAttribConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniQuatVectorAttribConverter, ToHoudiniQuatVectorAttribConverterTypeId, ToHoudiniAttribConverter );

		ToHoudiniQuatVectorAttribConverter( const IECore::Data *data );

		virtual ~ToHoudiniQuatVectorAttribConverter();

	protected :

		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const;
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const;

	private :

		static ToHoudiniAttribConverter::Description<ToHoudiniQuatVectorAttribConverter> m_description;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_TOHOUDINIQUATATTRIBCONVERTER_H
