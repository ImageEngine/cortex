//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ChannelOp.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ChannelOp );

ChannelOp::ChannelOp( const std::string &name, const std::string &description )
	:	ImagePrimitiveOp( name, description )
{

	StringVectorDataPtr defaultChannels = new StringVectorData;
	defaultChannels->writable().push_back( "R" );
	defaultChannels->writable().push_back( "G" );
	defaultChannels->writable().push_back( "B" );

	m_channelNamesParameter = new StringVectorParameter(
		"channels",
		"The names of the channels to modify.",
		defaultChannels
	);

	parameters()->addParameter( m_channelNamesParameter );
}

ChannelOp::~ChannelOp()
{
}

StringVectorParameterPtr ChannelOp::channelNamesParameter()
{
	return m_channelNamesParameter;
}

ConstStringVectorParameterPtr ChannelOp::channelNamesParameter() const
{
	return m_channelNamesParameter;
}

void ChannelOp::modifyTypedPrimitive( ImagePrimitivePtr image, ConstCompoundObjectPtr operands )
{
	if( image->getDataWindow().isEmpty() )
	{
		return;
	}

	ChannelVector channels;

	/// \todo Just use ImagePrimitive::channelValid. We don't want to do that right now
	/// as it imposes loose restrictions on the channel datatype - in the future it should perhaps
	/// impose the restrictions we have here (float, int or half vector data).
	/// Would be useful to add a TypeTraits class which can test compatiblity againt ImagePrimitive-supported channels, too.
	size_t numPixels = image->variableSize( PrimitiveVariable::Vertex );
	const vector<string> channelNames = channelNamesParameter()->getTypedValue();
	for( unsigned i=0; i<channelNames.size(); i++ )
	{
		PrimitiveVariableMap::iterator it = image->variables.find( channelNames[i] );
		if( it==image->variables.end() )
		{
			throw Exception( str( format( "Channel \"%s\" does not exist." ) % channelNames[i] ) );
		}

		if( it->second.interpolation!=PrimitiveVariable::Vertex &&
			it->second.interpolation!=PrimitiveVariable::Varying &&
			it->second.interpolation!=PrimitiveVariable::FaceVarying )
		{
			throw Exception( str( format( "Primitive variable \"%s\" has inappropriate interpolation." ) % channelNames[i] ) );
		}

		if( !it->second.data )
		{
			throw Exception( str( format( "Primitive variable \"%s\" has no data." ) % channelNames[i] ) );
		}

		if( !it->second.data->isInstanceOf( FloatVectorData::staticTypeId() ) &&
			!it->second.data->isInstanceOf( HalfVectorData::staticTypeId() ) &&
			!it->second.data->isInstanceOf( IntVectorData::staticTypeId() ) )
		{
			throw Exception( str( format( "Primitive variable \"%s\" has inappropriate type." ) % channelNames[i] ) );
		}

		size_t size = despatchTypedData<TypedDataSize>( it->second.data );
		if( size!=numPixels )
		{
			throw Exception( str( format( "Primitive variable \"%s\" has wrong size (%d but should be %d)." ) % channelNames[i] % size % numPixels ) );
		}

		channels.push_back( it->second.data );
	}

	modifyChannels( image->getDisplayWindow(), image->getDataWindow(), channels );
	/// \todo Consider cases where the derived class invalidates the channel data (by changing its length)
}
