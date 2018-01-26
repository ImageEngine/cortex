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

#include "IECoreImage/LuminanceOp.h"

#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/ImagePrimitiveParameter.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"

#include "boost/format.hpp"

using namespace boost;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( LuminanceOp );

LuminanceOp::LuminanceOp()
	:	ModifyOp( "Calculates luminance and adds it as a channel.", new ImagePrimitiveParameter( "result", "The result", new ImagePrimitive() ), new ImagePrimitiveParameter( "input", "The image to modify", new ImagePrimitive() ) )
{

	m_colorChannelParameter = new StringParameter(
		"colorChannel",
		"The name of the channel which holds colour data. This "
		"can have data of type Color3fData or Color3fVectorData.",
		"Cs"
	);

	m_redChannelParameter = new StringParameter(
		"redChannel",
		"The name of the channel which holds the red channel of the colour data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this channel must match the type of the other colour component Channels.",
		"R"
	);

	m_greenChannelParameter = new StringParameter(
		"greenChannel",
		"The name of the channel which holds the green channel of the colour data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this channel must match the type of the other colour component Channels.",
		"G"
	);

	m_blueChannelParameter = new StringParameter(
		"blueChannel",
		"The name of the channel which holds the blue channel of the colour data. This "
		"can have data of type HalfData, HalfVectorData, FloatData or FloatVectorData. "
		"However, The type of this channel must match the type of the other colour component Channels.",
		"B"
	);

	m_weightsParameter = new Color3fParameter(
		"weights",
		"The weights used in averaging the rgb values to produce luminance.",
		Color3f( 0.2125, 0.7154, 0.0721 )
	);

	m_luminanceChannelParameter = new StringParameter(
		"luminanceChannel",
		"The name of the channel to hold the resulting luminance data.",
		"Y"
	);

	m_removeColorChannelsParameter = new BoolParameter(
		"removeColorChannels",
		"When this is true, the input channels are removed after luminance is calculated.",
		true
	);

	parameters()->addParameter( m_colorChannelParameter );
	parameters()->addParameter( m_redChannelParameter );
	parameters()->addParameter( m_greenChannelParameter );
	parameters()->addParameter( m_blueChannelParameter );
	parameters()->addParameter( m_weightsParameter );
	parameters()->addParameter( m_luminanceChannelParameter );
	parameters()->addParameter( m_removeColorChannelsParameter );

}

LuminanceOp::~LuminanceOp()
{
}

StringParameter * LuminanceOp::colorChannelParameter()
{
	return m_colorChannelParameter.get();
}

const StringParameter * LuminanceOp::colorChannelParameter() const
{
	return m_colorChannelParameter.get();
}

StringParameter * LuminanceOp::redChannelParameter()
{
	return m_redChannelParameter.get();
}

const StringParameter * LuminanceOp::redChannelParameter() const
{
	return m_redChannelParameter.get();
}

StringParameter * LuminanceOp::greenChannelParameter()
{
	return m_greenChannelParameter.get();
}

const StringParameter * LuminanceOp::greenChannelParameter() const
{
	return m_greenChannelParameter.get();
}

StringParameter * LuminanceOp::blueChannelParameter()
{
	return m_blueChannelParameter.get();
}

const StringParameter * LuminanceOp::blueChannelParameter() const
{
	return m_blueChannelParameter.get();
}

Color3fParameter * LuminanceOp::weightsParameter()
{
	return m_weightsParameter.get();
}

const Color3fParameter * LuminanceOp::weightsParameter() const
{
	return m_weightsParameter.get();
}


StringParameter * LuminanceOp::luminanceChannelParameter()
{
	return m_luminanceChannelParameter.get();
}

const StringParameter * LuminanceOp::luminanceChannelParameter() const
{
	return m_luminanceChannelParameter.get();
}

BoolParameter * LuminanceOp::removeColorChannelsParameter()
{
	return m_removeColorChannelsParameter.get();
}

const BoolParameter * LuminanceOp::removeColorChannelsParameter() const
{
	return m_removeColorChannelsParameter.get();
}

template<typename T>
void LuminanceOp::calculate( const T *r, const T *g, const T *b, int steps[3], int size, T *y )
{
	Color3f weights = m_weightsParameter->getTypedValue();
	for( int i=0; i<size; i++ )
	{
		*y++ = weights[0] * *r + weights[1] * *g + weights[2] * *b;
		r += steps[0];
		g += steps[1];
		b += steps[2];
	}
}

void LuminanceOp::modify( Object *object, const CompoundObject *operands )
{
	ImagePrimitive *image = runTimeCast<ImagePrimitive>( object );
	ImagePrimitive::ChannelMap &channels = image->channels;

	DataPtr luminanceData = nullptr;
	int steps[3] = { 1, 1, 1 };

	const auto colorIt = channels.find( m_colorChannelParameter->getTypedValue() );
	if( colorIt != channels.end() && colorIt->second )
	{
		// RGB in a single channel
		switch( colorIt->second->typeId() )
		{
			case Color3fDataTypeId :
				{
					FloatDataPtr l = new FloatData;
					const float *d = boost::static_pointer_cast<Color3fData>( colorIt->second )->baseReadable();
					calculate( d, d + 1, d + 2, steps, 1, l->baseWritable() );
					luminanceData = l;
				}
				break;
			case Color3fVectorDataTypeId :
				{
					FloatVectorDataPtr l = new FloatVectorData;
					Color3fVectorDataPtr d = boost::static_pointer_cast<Color3fVectorData>( colorIt->second );
					l->writable().resize( d->readable().size() );
					const float *dd = d->baseReadable();
					steps[0] = steps[1] = steps[2] = 3;
					calculate( dd, dd + 1, dd + 2, steps, d->readable().size(), l->baseWritable() );
					luminanceData = l;
				}
				break;
			default :
				throw Exception( "Channel has unsupported type." );
				break;
		}
	}
	else
	{
		// separate RGB channels?
		const auto rIt = channels.find( m_redChannelParameter->getTypedValue() );
		const auto gIt = channels.find( m_greenChannelParameter->getTypedValue() );
		const auto bIt = channels.find( m_blueChannelParameter->getTypedValue() );

		std::string reason;
		if( !image->channelValid( rIt->first, &reason ) )
		{
			throw Exception( str( format( "Channel \"%s\" is invalid: " ) % rIt->first ) + reason );
		}
		if( !image->channelValid( gIt->first, &reason ) )
		{
			throw Exception( str( format( "Channel \"%s\" is invalid: " ) % gIt->first ) + reason );
		}
		if( !image->channelValid( bIt->first, &reason ) )
		{
			throw Exception( str( format( "Channel \"%s\" is invalid: " ) % bIt->first ) + reason );
		}

		size_t channelSize = image->channelSize();

		switch( rIt->second->typeId() )
		{
			case HalfDataTypeId :
				{
					HalfDataPtr l = new HalfData;
					calculate(
						boost::static_pointer_cast<HalfData>( rIt->second )->baseReadable(),
						boost::static_pointer_cast<HalfData>( gIt->second )->baseReadable(),
						boost::static_pointer_cast<HalfData>( bIt->second )->baseReadable(),
						steps,
						channelSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;
			case HalfVectorDataTypeId :
				{
					HalfVectorDataPtr l = new HalfVectorData;
					l->writable().resize( channelSize );
					calculate(
						boost::static_pointer_cast<HalfVectorData>( rIt->second )->baseReadable(),
						boost::static_pointer_cast<HalfVectorData>( gIt->second )->baseReadable(),
						boost::static_pointer_cast<HalfVectorData>( bIt->second )->baseReadable(),
						steps,
						channelSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;
			case FloatDataTypeId :
				{
					FloatDataPtr l = new FloatData;
					calculate(
						boost::static_pointer_cast<FloatData>( rIt->second )->baseReadable(),
						boost::static_pointer_cast<FloatData>( gIt->second )->baseReadable(),
						boost::static_pointer_cast<FloatData>( bIt->second )->baseReadable(),
						steps,
						channelSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;
			case FloatVectorDataTypeId :
				{
					FloatVectorDataPtr l = new FloatVectorData;
					l->writable().resize( channelSize );
					calculate(
						boost::static_pointer_cast<FloatVectorData>( rIt->second )->baseReadable(),
						boost::static_pointer_cast<FloatVectorData>( gIt->second )->baseReadable(),
						boost::static_pointer_cast<FloatVectorData>( bIt->second )->baseReadable(),
						steps,
						channelSize,
						l->baseWritable()
					);
					luminanceData = l;
				}
				break;
			default :
				throw Exception( "Channels have unsupported type." );
				break;
		}
	}

	assert( luminanceData );

	channels[luminanceChannelParameter()->getTypedValue()] = luminanceData;

	if( removeColorChannelsParameter()->getTypedValue() )
	{
		channels.erase( colorChannelParameter()->getTypedValue() );
		channels.erase( redChannelParameter()->getTypedValue() );
		channels.erase( greenChannelParameter()->getTypedValue() );
		channels.erase( blueChannelParameter()->getTypedValue() );
	}
}
