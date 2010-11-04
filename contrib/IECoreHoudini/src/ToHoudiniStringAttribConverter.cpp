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

#include "ToHoudiniStringAttribConverter.h"

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

GB_AttributeRef ToHoudiniStringVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	UT_PtrArray<GU_Detail*> geoList( 1 );
	geoList[0] = geo;
	
	return doVectorConversion<UT_PtrArray<GU_Detail*> >( data, name, geo, &geoList, GEO_DETAIL_DICT );
}

GB_AttributeRef ToHoudiniStringVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PointList *points ) const
{
	return doVectorConversion<GEO_PointList>( data, name, geo, points, GEO_POINT_DICT );
}

GB_AttributeRef ToHoudiniStringVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PrimList *primitives ) const
{
	return doVectorConversion<GEO_PrimList>( data, name, geo, primitives, GEO_PRIMITIVE_DICT );
}

GB_AttributeRef ToHoudiniStringVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, VertexList *vertices ) const
{
	return doVectorConversion<VertexList>( data, name, geo, vertices, GEO_VERTEX_DICT );
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

GB_AttributeRef ToHoudiniStringDetailAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	assert( data );

	const StringData *stringData = IECore::runTimeCast<const StringData>( data );
	if ( !stringData )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringDetailAttribConverter::doConversion: PrimitiveVariable \"%s\" does not contain IECore::StringData." ) % name ).str() );
	}

	GB_AttributeRef attrRef = geo->addAttribute( name.c_str(), sizeof( int ), GB_ATTRIB_INDEX, "", GEO_DETAIL_DICT );
	if ( GBisAttributeRefInvalid( attrRef ) )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniStringDetailAttribConverter::doConversion: Invalid GB_AttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}
	
	GEO_AttributeHandle attribHandle = geo->getAttribute( GEO_DETAIL_DICT, name.c_str() );
	attribHandle.addDefinedString( stringData->readable().c_str() );
	
	UT_StringArray definedStrings;
	if ( attribHandle.getDefinedStrings( definedStrings ) && definedStrings.entries() )
	{
		attribHandle.setElement( geo );
		attribHandle.setString( definedStrings( 0 ) );
	}
	
	return attrRef;
}

GB_AttributeRef ToHoudiniStringDetailAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PointList *points ) const
{
	throw IECore::Exception( "ToHoudiniStringDetailAttribConverter does not support Point attributes." );
}

GB_AttributeRef ToHoudiniStringDetailAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, GEO_PrimList *primitives ) const
{
	throw IECore::Exception( "ToHoudiniStringDetailAttribConverter does not support Primitive attributes." );
}

GB_AttributeRef ToHoudiniStringDetailAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, VertexList *vertices ) const
{
	throw IECore::Exception( "ToHoudiniStringDetailAttribConverter does not support Vertex attributes." );
}

} // namespace IECoreHoudini
