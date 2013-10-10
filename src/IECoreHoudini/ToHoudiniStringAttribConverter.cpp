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

#include "IECore/MessageHandler.h"

#include "IECoreHoudini/ToHoudiniStringAttribConverter.h"

using namespace IECore;

namespace IECoreHoudini
{

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniStringVectorAttribConverter );

ToHoudiniAttribConverter::Description<ToHoudiniStringVectorAttribConverter> ToHoudiniStringVectorAttribConverter::m_description( StringVectorData::staticTypeId() );

ToHoudiniStringVectorAttribConverter::ToHoudiniStringVectorAttribConverter( const IECore::Data *data ) :
	ToHoudiniAttribConverter( data, "Converts IECore::StringVectorData to a GB_Attribute on the provided GU_Detail." )
{
	m_indicesParameter = new IntVectorParameter( "indices", "the indices into the source StringVectorData", new IntVectorData() );
	parameters()->addParameter( m_indicesParameter );
}

ToHoudiniStringVectorAttribConverter::~ToHoudiniStringVectorAttribConverter()
{
}

IntVectorParameterPtr ToHoudiniStringVectorAttribConverter::indicesParameter()
{
	return m_indicesParameter;
}

ConstIntVectorParameterPtr ToHoudiniStringVectorAttribConverter::indicesParameter() const
{
	return m_indicesParameter;
}

GA_RWAttributeRef ToHoudiniStringVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	IECore::msg( IECore::MessageHandler::Warning, "ToHoudiniStringVectorAttribConverter", "Does not support Detail attributes. Ignoring \"" + name + "\"" );
	return GA_RWAttributeRef();
}

GA_RWAttributeRef ToHoudiniStringVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const
{
	assert( data );

	const IECore::StringVectorData *stringVectorData = IECore::runTimeCast<const IECore::StringVectorData>( data );
	if ( !stringVectorData )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringVectorAttribConverter::doConversion: PrimitiveVariable \"%s\" does not contain IECore::StringVectorData." ) % name ).str() );
	}
	
	GA_RWAttributeRef attrRef = geo->addStringTuple( range.getOwner(), name.c_str(), 1 );
	if ( attrRef.isInvalid() )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringVectorAttribConverter::doConversion: Invalid GA_RWAttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}
	
	if ( !range.isValid() || range.empty() )
	{
		return attrRef;
	}
	
	GA_Attribute *attr = attrRef.getAttribute();
	
	const GA_AIFSharedStringTuple *tuple = attr->getAIFSharedStringTuple();
	
	UT_StringArray strings;
	strings.fromStdVectorOfStrings( stringVectorData->readable() );
	
	const std::vector<int> &indices = ((const IECore::IntVectorData *)m_indicesParameter->getValidatedValue())->readable();
	if ( indices.empty() || !strings.entries() )
	{
		return attrRef;
	}
	
	GA_AIFSharedStringTuple::StringBuffer handles = tuple->addStrings( attr, strings );
	
	size_t i = 0;
	size_t numIndices = indices.size();
	for ( GA_Iterator it=range.begin(); !it.atEnd(), i < numIndices; ++it, ++i )
	{
		tuple->setHandle( attr, it.getOffset(), handles.getStringIndex( indices[i] ), 0 );
	}
	
	return attrRef;
}

GA_RWAttributeRef ToHoudiniStringVectorAttribConverter::convertString( std::string name, std::string value, GU_Detail *geo, GA_Range range )
{
	StringVectorDataPtr valueData = new StringVectorData();
	valueData->writable().push_back( value );
	std::vector<int> indexValues( range.getEntries(), 0 );
	IntVectorDataPtr indexData = new IntVectorData( indexValues );
	ToHoudiniStringVectorAttribConverterPtr converter = new ToHoudiniStringVectorAttribConverter( valueData );
	converter->indicesParameter()->setValidatedValue( indexData );
	return converter->convert( name, geo, range );
}

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniStringDetailAttribConverter );

ToHoudiniAttribConverter::Description<ToHoudiniStringDetailAttribConverter> ToHoudiniStringDetailAttribConverter::m_description( StringData::staticTypeId() );

ToHoudiniStringDetailAttribConverter::ToHoudiniStringDetailAttribConverter( const IECore::Data *data ) :
	ToHoudiniAttribConverter( data, "Converts IECore::StringData to a GB_Attribute on the provided GU_Detail." )
{
}

ToHoudiniStringDetailAttribConverter::~ToHoudiniStringDetailAttribConverter()
{
}

GA_RWAttributeRef ToHoudiniStringDetailAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	assert( data );

	const StringData *stringData = IECore::runTimeCast<const StringData>( data );
	if ( !stringData )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringDetailAttribConverter::doConversion: PrimitiveVariable \"%s\" does not contain IECore::StringData." ) % name ).str() );
	}

	GA_RWAttributeRef attrRef = geo->addStringTuple( GA_ATTRIB_DETAIL, name.c_str(), 1 );
	if ( attrRef.isInvalid() )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringDetailAttribConverter::doConversion: Invalid GA_RWAttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}
	
	GA_Attribute *attr = attrRef.getAttribute();
	attr->getAIFSharedStringTuple()->setString( attr, 0, stringData->readable().c_str(), 0 );
	
	return attrRef;
}

GA_RWAttributeRef ToHoudiniStringDetailAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const
{
	throw IECore::Exception( "ToHoudiniStringDetailAttribConverter does not support Element attributes." );
}

} // namespace IECoreHoudini
