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

#ifndef IECOREHOUDINI_TOHOUDINISTRINGATTRIBCONVERTER_H
#define IECOREHOUDINI_TOHOUDINISTRINGATTRIBCONVERTER_H

#include "IECore/VectorTypedParameter.h"

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/ToHoudiniAttribConverter.h"

namespace IECoreHoudini
{

IE_CORE_FORWARDDECLARE( ToHoudiniStringVectorAttribConverter );

/// A ToHoudiniStringVectorAttribConverter can convert from IECore::StringVectorData
/// to a Houdini GA_Attribute on the provided GU_Detail.
class CortexHOUAPI ToHoudiniStringVectorAttribConverter : public ToHoudiniAttribConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniStringVectorAttribConverter, ToHoudiniStringVectorAttribConverterTypeId, ToHoudiniAttribConverter );
		
		ToHoudiniStringVectorAttribConverter( const IECore::Data *data );
		
		virtual ~ToHoudiniStringVectorAttribConverter();
		
		IECore::IntVectorParameterPtr indicesParameter();
		IECore::ConstIntVectorParameterPtr indicesParameter() const;
		
		/// Utility method to set a single string for all elements in the given range
		static GA_RWAttributeRef convertString( std::string name, std::string value, GU_Detail *geo, GA_Range range );
		
	protected :
		
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const;
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const;
		
	private :
			
		static ToHoudiniAttribConverter::Description<ToHoudiniStringVectorAttribConverter> m_description;
		
		/// \todo: replace this parameter with IECore::IndexedData once it exists...
		IECore::IntVectorParameterPtr m_indicesParameter;

};

IE_CORE_FORWARDDECLARE( ToHoudiniStringDetailAttribConverter );

/// A ToHoudiniStringDetailAttribConverter can convert from IECore::StringData
/// to a Houdini GA_Attribute on the provided GU_Detail.
class ToHoudiniStringDetailAttribConverter : public ToHoudiniAttribConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToHoudiniStringDetailAttribConverter, ToHoudiniStringDetailAttribConverterTypeId, ToHoudiniAttribConverter );
		
		ToHoudiniStringDetailAttribConverter( const IECore::Data *data );
		
		virtual ~ToHoudiniStringDetailAttribConverter();
		
	protected :
		
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const;
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const;

	private :
				
		static ToHoudiniAttribConverter::Description<ToHoudiniStringDetailAttribConverter> m_description;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_TOHOUDINISTRINGATTRIBCONVERTER_H
