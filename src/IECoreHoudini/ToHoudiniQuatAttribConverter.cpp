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

#include "IECoreHoudini/ToHoudiniQuatAttribConverter.h"

#include "IECore/MessageHandler.h"

using namespace IECore;

namespace IECoreHoudini
{

IE_CORE_DEFINERUNTIMETYPED( ToHoudiniQuatVectorAttribConverter );

ToHoudiniAttribConverter::Description<ToHoudiniQuatVectorAttribConverter> ToHoudiniQuatVectorAttribConverter::m_description( QuatfVectorData::staticTypeId() );

ToHoudiniQuatVectorAttribConverter::ToHoudiniQuatVectorAttribConverter( const IECore::Data *data ) :
	ToHoudiniAttribConverter( data, "Converts IECore::QuatfVectorData to a GB_Attribute on the provided GU_Detail." )
{
}

ToHoudiniQuatVectorAttribConverter::~ToHoudiniQuatVectorAttribConverter()
{
}

GA_RWAttributeRef ToHoudiniQuatVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const
{
	IECore::msg( IECore::MessageHandler::Warning, "ToHoudiniQuatVectorAttribConverter", "Does not support Detail attributes. Ignoring \"" + name + "\"" );
	return GA_RWAttributeRef();
}

GA_RWAttributeRef ToHoudiniQuatVectorAttribConverter::doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const
{
	assert( data );

	GA_RWAttributeRef attrRef = geo->addFloatTuple( range.getOwner(), name.c_str(), 4 );
	if ( attrRef.isInvalid() )
	{
		throw IECore::Exception( ( boost::format( "ToHoudiniQuatVectorAttribConverter::doConversion: Invalid GA_RWAttributeRef returned for PrimitiveVariable \"%s\"." ) % name ).str() );
	}
	attrRef.setTypeInfo( GA_TYPE_QUATERNION );

	const QuatfVectorData* dataPtr = IECore::runTimeCast<const QuatfVectorData>( data );

	// reorder quaternion components for houdini:
	std::vector<float> floatData;
	floatData.reserve( 4 * dataPtr->readable().size() );
	for( std::vector<Imath::Quatf>::const_iterator it = dataPtr->readable().begin(); it != dataPtr->readable().end(); ++it )
	{
		floatData.push_back( (*it)[1] );
		floatData.push_back( (*it)[2] );
		floatData.push_back( (*it)[3] );
		floatData.push_back( (*it)[0] );
	}

	GA_Attribute *attr = attrRef.getAttribute();
	attr->getAIFTuple()->setRange( attr, range, floatData.data() );

	return attrRef;
}

} // namespace IECoreHoudini
