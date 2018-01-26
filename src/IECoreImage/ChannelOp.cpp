//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreImage/ChannelOp.h"

#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/ImagePrimitiveParameter.h"

#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreImage;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ChannelOp );

ChannelOp::ChannelOp( const std::string &description )
	:	ModifyOp( description, new ImagePrimitiveParameter( "result", "The result", new ImagePrimitive() ), new ImagePrimitiveParameter( "input", "The image to modify", new ImagePrimitive() ) )
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

StringVectorParameter * ChannelOp::channelNamesParameter()
{
	return m_channelNamesParameter.get();
}

const StringVectorParameter * ChannelOp::channelNamesParameter() const
{
	return m_channelNamesParameter.get();
}

void ChannelOp::modify( Object * primitive, const CompoundObject * operands )
{
	ImagePrimitive *image = runTimeCast<ImagePrimitive>( primitive );

	if( image->getDataWindow().isEmpty() )
	{
		return;
	}

	ChannelVector channels;

	const vector<string> &channelNames = channelNamesParameter()->getTypedValue();
	for( const auto &name : channelNames )
	{
		std::string reason;
		if( !image->channelValid( name, &reason ) )
		{
			throw Exception( str( format( "Channel \"%s\" is invalid: " ) % name ) + reason );
		}

		if( !image->channels[name]->isInstanceOf( FloatVectorData::staticTypeId() ) )
		{
			throw Exception( str( format( "Channel \"%s\" is invalid: not a float vector." ) % name ) );
		}

		channels.push_back( boost::static_pointer_cast< FloatVectorData >( image->channels[name] ) );
	}

	modifyChannels( image->getDisplayWindow(), image->getDataWindow(), channels );
	/// \todo Consider cases where the derived class invalidates the channel data (by changing its length)
}
