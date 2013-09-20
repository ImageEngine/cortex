//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREHOUDINI_FROMHOUDINIPOLYGONSCONVERTER_H
#define IE_COREHOUDINI_FROMHOUDINIPOLYGONSCONVERTER_H

#include "IECore/MeshPrimitive.h"

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

namespace IECoreHoudini
{

/// Converter which converts from a Houdini GU_Detail to an IECore::MeshPrimitive
class FromHoudiniPolygonsConverter : public IECoreHoudini::FromHoudiniGeometryConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromHoudiniPolygonsConverter, FromHoudiniPolygonsConverterTypeId, IECore::ToCoreConverter );

		FromHoudiniPolygonsConverter( const GU_DetailHandle &handle );
		FromHoudiniPolygonsConverter( const SOP_Node *sop );
		
		virtual ~FromHoudiniPolygonsConverter();

		/// Determines if the given GU_Detail can be converted
		static FromHoudiniGeometryConverter::Convertability canConvert( const GU_Detail *geo );
	
	protected :
		
		/// performs conversion to a IECore::MeshPrimitive
		virtual IECore::ObjectPtr doDetailConversion( const GU_Detail *geo, const IECore::CompoundObject *operands ) const;

	private :

		static FromHoudiniGeometryConverter::Description<FromHoudiniPolygonsConverter> m_description;
};

// register our converter
IE_CORE_DECLAREPTR( FromHoudiniPolygonsConverter );

}

#endif // IE_COREHOUDINI_FROMHOUDINIPOLYGONSCONVERTER_H
