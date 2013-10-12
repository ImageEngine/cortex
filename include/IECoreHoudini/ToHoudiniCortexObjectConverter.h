//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TOHOUDINICORTEXOBJECTCONVERTER_H
#define IECOREHOUDINI_TOHOUDINICORTEXOBJECTCONVERTER_H

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"

namespace IECoreHoudini
{

/// Converter which converts from an IECore::Object to a Houdini GU_Detail. This converter
/// creates a single GU_CortexPrimitive containing the Object, and adds it to the detail.
class CortexHOUAPI ToHoudiniCortexObjectConverter : public IECoreHoudini::ToHoudiniGeometryConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniCortexObjectConverter, ToHoudiniCortexObjectConverterTypeId, IECoreHoudini::ToHoudiniGeometryConverter );

		ToHoudiniCortexObjectConverter( const IECore::Object *object );

		virtual ~ToHoudiniCortexObjectConverter();
		
		/// We don't actually transfer any attribs during conversion, but this method can be used to
		/// transfer PrimitiveVariables from the source Object, provided it is an IECore::Primitive,
		/// the GA_Range contains a GU_CortexPrimitive which also holds an IECore::Primitive, and both
		/// Primitives have the same variable sizes.
		virtual void transferAttribs( GU_Detail *geo, const GA_Range &points, const GA_Range &prims ) const;
	
	protected :
		
		/// performs conversion from the IECore::Object into the given GU_Detail
		virtual bool doConversion( const IECore::Object *object, GU_Detail *geo ) const;

	private :
		
		// Used to apply the attributeFilter to the Object, if it is a Primitive
		IECore::ConstObjectPtr filterAttribs( const IECore::Object *object ) const;
		
		static ToHoudiniGeometryConverter::Description<ToHoudiniCortexObjectConverter> m_description;
};

// register our converter
IE_CORE_DECLAREPTR( ToHoudiniCortexObjectConverter );

}

#endif // IECOREHOUDINI_TOHOUDINICORTEXOBJECTCONVERTER_H
