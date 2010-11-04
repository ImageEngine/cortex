//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TOHOUDINISTRINGATTRIBCONVERTER_INL
#define IECOREHOUDINI_TOHOUDINISTRINGATTRIBCONVERTER_INL

#include "boost/format.hpp"

#include "GEO/GEO_Vertex.h"
#include "GEO/GEO_AttributeHandle.h"

#include "IECore/VectorTraits.h"

#include "ToHoudiniStringAttribConverter.h"
#include "TypeTraits.h"

namespace IECoreHoudini
{

template<typename Container>
GB_AttributeRef ToHoudiniStringVectorAttribConverter::doVectorConversion( const IECore::Data *data, std::string name, GU_Detail *geo, Container *container, GEO_AttributeOwner owner ) const
{
	assert( data );

	const IECore::StringVectorData *stringVectorData = IECore::runTimeCast<const IECore::StringVectorData>( data );
	if ( !stringVectorData )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringVectorAttribConverter::doConversion: PrimitiveVariable \"%s\" does not contain IECore::StringVectorData." ) % name ).str() );
	}

	GB_AttributeRef attrRef = geo->addAttribute( name.c_str(), sizeof( int ), GB_ATTRIB_INDEX, "", owner );
	if ( GBisAttributeRefInvalid( attrRef ) )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringVectorAttribConverter::doConversion: Invalid GB_AttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}
	
	GEO_AttributeHandle attribHandle = geo->getAttribute( owner, name.c_str() );
	
	const std::vector<std::string> &stringVector = stringVectorData->readable();
	for ( size_t i=0; i < stringVector.size(); i++ )
	{
		attribHandle.addDefinedString( stringVector[i].c_str() );
	}
	
	UT_StringArray definedStrings;
	const std::vector<int> &indices = ((const IECore::IntVectorData *)m_indicesParameter->getValidatedValue())->readable();
	if ( !container || indices.empty() || !attribHandle.getDefinedStrings( definedStrings ) || !definedStrings.entries() )
	{
		return attrRef;
	}
	
	size_t numStrings = definedStrings.entries();
	size_t entries = std::min( (size_t)container->entries(), indices.size() );
	for ( size_t i=0; i < entries; i++ )
	{
		attribHandle.setElement( (*container)[i] );
		int index = indices[i] < numStrings ? indices[i] : numStrings - 1;
		attribHandle.setString( definedStrings( index ) );
	}
	
	return attrRef;
}

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_TOHOUDINISTRINGATTRIBCONVERTER_INL
