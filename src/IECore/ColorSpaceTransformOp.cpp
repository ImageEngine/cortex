//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include <cassert>
#include <algorithm>

#include "boost/tokenizer.hpp"
#include "boost/format.hpp"

#include "IECore/Object.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Exception.h"
#include "IECore/ChannelOp.h"
#include "IECore/ColorTransformOp.h"
#include "IECore/MessageHandler.h"

#include "IECore/ColorSpaceTransformOp.h"
#include "IECore/ImagePremultiplyOp.h"
#include "IECore/ImageUnpremultiplyOp.h"

using namespace IECore;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ColorSpaceTransformOp );

ColorSpaceTransformOp::ColorSpaceTransformOp()
	:	ImagePrimitiveOp( "Converts channels from one named colorspace to another. Additional colorspaces can be registered at runtime." )
{
	m_inputColorSpaceParameter = new StringParameter(
		"inputColorSpace",
		"Input color space name",
		"linear"
	);

	m_outputColorSpaceParameter = new StringParameter(
		"outputColorSpace",
		"Output color space name",
		"linear"
	);

	StringVectorData::ValueType defaultChannels;
	defaultChannels.push_back( "R" );
	defaultChannels.push_back( "G" );
	defaultChannels.push_back( "B" );
	m_channelsParameter = new StringVectorParameter(
		"channels",
		"The names of all channels to convert on the image. "
		"The order of the channels listed is important if the conversion is done by a ColorTransformOp. "
		"In that case it is expected to receive Red,Green,Blue channels respectively.",
		new StringVectorData( defaultChannels )
	);

	m_alphaPrimVarParameter = new StringParameter(
		"alphaPrimVar",
		"The name of the primitive variable which holds the alpha channel. This is only used "
		"if the premultiplied parameter is on. The type must match the type of the color channels.",
		"A"
	);

	m_premultipliedParameter = new BoolParameter(
		"premultiplied",
		"If this is on, then the colors are divided by alpha before transformation and "
		"premultiplied again afterwards.",
		true
	);

	parameters()->addParameter( m_inputColorSpaceParameter );
	parameters()->addParameter( m_outputColorSpaceParameter );
	parameters()->addParameter( m_channelsParameter );
	parameters()->addParameter( m_alphaPrimVarParameter );
	parameters()->addParameter( m_premultipliedParameter );
}

ColorSpaceTransformOp::~ColorSpaceTransformOp()
{
}

StringParameter *ColorSpaceTransformOp::inputColorSpaceParameter()
{
	return m_inputColorSpaceParameter;
}

const StringParameter *ColorSpaceTransformOp::inputColorSpaceParameter() const
{
	return m_inputColorSpaceParameter;
}

StringParameter *ColorSpaceTransformOp::outputColorSpaceParameter()
{
	return m_outputColorSpaceParameter;
}

const StringParameter *ColorSpaceTransformOp::outputColorSpaceParameter() const
{
	return m_outputColorSpaceParameter;
}

StringVectorParameter *ColorSpaceTransformOp::channelsParameter()
{
	return m_channelsParameter;
}

const StringVectorParameter *ColorSpaceTransformOp::channelsParameter() const
{
	return m_channelsParameter;
}

StringParameter *ColorSpaceTransformOp::alphaPrimVarParameter()
{
	return m_alphaPrimVarParameter;
}

const StringParameter *ColorSpaceTransformOp::alphaPrimVarParameter() const
{
	return m_alphaPrimVarParameter;
}

BoolParameter *ColorSpaceTransformOp::premultipliedParameter()
{
	return m_premultipliedParameter;
}

const BoolParameter *ColorSpaceTransformOp::premultipliedParameter() const
{
	return m_premultipliedParameter;
}

void ColorSpaceTransformOp::registerConversion( const InputColorSpace &inputColorSpace, const OutputColorSpace &outputColorSpace, const CreatorFn &creator )
{
	if ( inputColorSpace != outputColorSpace )
	{
		Conversion conversion( inputColorSpace, outputColorSpace );

		if ( conversionsSet().find( conversion ) != conversionsSet().end() )
		{
			msg( Msg::Warning, "ColorSpaceTransformOp", boost::format( "Converter for '%s' to '%s 'registered twice" ) % inputColorSpace % outputColorSpace );
		}
		conversionsSet().insert( conversion );

		CreatorFn *pCreator = new CreatorFn( creator );

		converterTypes()[ pCreator ] = conversion;

		ConversionInfo info( pCreator, inputColorSpace, outputColorSpace );

		converters().insert(
			ConvertersMap::value_type(
				inputColorSpace,
				info
			)
		);
	}
}

void ColorSpaceTransformOp::inputColorSpaces( std::vector< InputColorSpace > &colorSpaces )
{
	colorSpaces.clear();

	std::set< InputColorSpace > uniqueColorSpaces;

	for ( ConversionsSet::const_iterator it = conversionsSet().begin(); it != conversionsSet().end(); ++it )
	{
		uniqueColorSpaces.insert( it->first );
	}

	colorSpaces.resize( uniqueColorSpaces.size() );
	std::copy( uniqueColorSpaces.begin(), uniqueColorSpaces.end(), colorSpaces.begin() ) ;
}

void ColorSpaceTransformOp::outputColorSpaces( std::vector< OutputColorSpace > &colorSpaces )
{
	colorSpaces.clear();

	std::set< OutputColorSpace > uniqueColorSpaces;

	for ( ConversionsSet::const_iterator it = conversionsSet().begin(); it != conversionsSet().end(); ++it )
	{
		uniqueColorSpaces.insert( it->second );
	}

	colorSpaces.resize( uniqueColorSpaces.size() );
	std::copy( uniqueColorSpaces.begin(), uniqueColorSpaces.end(), colorSpaces.begin() ) ;
}

void ColorSpaceTransformOp::colorSpaces( std::vector< std::string > &colorSpaces )
{
	colorSpaces.clear();

	std::set< std::string > uniqueColorSpaces;

	for ( ConversionsSet::const_iterator it = conversionsSet().begin(); it != conversionsSet().end(); ++it )
	{
		uniqueColorSpaces.insert( it->first );
		uniqueColorSpaces.insert( it->second );
	}

	colorSpaces.resize( uniqueColorSpaces.size() );
	std::copy( uniqueColorSpaces.begin(), uniqueColorSpaces.end(), colorSpaces.begin() );
}

void ColorSpaceTransformOp::findConversion( const InputColorSpace &inputColorSpace, const OutputColorSpace &outputColorSpace, std::vector< ConversionInfo > &conversions  )
{
	conversions.clear();

	std::set< Conversion > visitedConversions;
	std::vector< ConversionInfo > currentConversion;

	findConversion(
		inputColorSpace,
		outputColorSpace,
		visitedConversions,
		currentConversion,
		conversions
	);
}

void ColorSpaceTransformOp::findConversion(
	const InputColorSpace &inputColorSpace,
	const OutputColorSpace &outputColorSpace,
	std::set< Conversion > &visitedConversions,
	std::vector< ConversionInfo > &currentConversion,
	std::vector< ConversionInfo > &bestConversion
	)
{
	Conversion conversion( inputColorSpace, outputColorSpace );

	/// Only proceed if we've not found a conversion yet, or if the conversion we're working on is likely to yield a better one than the best conversion found so far
	if ( ( bestConversion.size() > 0 ) && currentConversion.size() >= bestConversion.size() )
	{
		return;
	}

	/// Prevent cycles and back-tracking
	if ( visitedConversions.find( conversion ) != visitedConversions.end()
		|| visitedConversions.find( Conversion( outputColorSpace, inputColorSpace ) ) != visitedConversions.end())
	{
		return;
	}

	/// Mark conversion as visited so we don't try it again
	visitedConversions.insert( conversion );

	/// Find all converters which take our input color space
	ConvertersMap::const_iterator it = converters().find( inputColorSpace );
	if ( it == converters().end() )
	{
		return;
	}
	ConvertersMap::const_iterator end = converters().upper_bound( inputColorSpace );

	/// For each of these converters, either see if we can directly convert to the output color space, or recurse to find a sub-chain which can convert to it,
	/// keeping track of the best conversion found so far (shorter chains are better).
	for (; it != end; ++it )
	{
		ConversionInfo info = it->second;

		currentConversion.push_back( info );

		if ( it->second.get<2>() == outputColorSpace )
		{
			/// Termination condition
			bestConversion = currentConversion;
			currentConversion.pop_back();
			return;
		}

		/// Recurse
		ConverterTypesMap::const_iterator cIt = converterTypes().find( it->second.get<0>() );
		assert( cIt != converterTypes().end() );

		findConversion(
			cIt->second.second,
			outputColorSpace,

			visitedConversions,
			currentConversion,
			bestConversion
		);

		currentConversion.pop_back();
	}
}

void ColorSpaceTransformOp::modifyTypedPrimitive( ImagePrimitive * image, const CompoundObject * operands )
{
	const InputColorSpace &inputColorSpace = m_inputColorSpaceParameter->getTypedValue();
	const OutputColorSpace &outputColorSpace = m_outputColorSpaceParameter->getTypedValue();

	if ( inputColorSpace == outputColorSpace )
	{
		return;
	}

	std::vector< ConversionInfo > conversions;
	findConversion( inputColorSpace, outputColorSpace, conversions );

	if ( !conversions.size() )
	{
		throw InvalidArgumentException( ( boost::format( "ColorSpaceTransformOp: Cannot find appropriate conversion from '%s' to '%s'" ) % inputColorSpace % outputColorSpace ).str() );
	}

	std::vector< std::string > channelNames;
	typedef std::vector< std::vector< std::string > > ChannelSets;
	ChannelSets channelSets;
	std::vector< std::string > channels;

	for ( std::vector< std::string >::const_iterator it = channelsParameter()->getTypedValue().begin(); it != channelsParameter()->getTypedValue().end(); ++it )
	{
		const std::string &channelName = *it;

		channelNames.push_back( channelName );

		PrimitiveVariableMap::iterator channelIt = image->variables.find( channelName );
		if( channelIt==image->variables.end() || !(channelIt->second.data) )
		{
			throw Exception( str( format( "Channel \"%s\" does not exist." ) % channelName ) );
		}

		TypeId type = channelIt->second.data->typeId();

		if ( type == Color3fVectorDataTypeId || type == Color3dVectorDataTypeId )
		{
			// color data types define the three channels.

			if ( channels.size() )
			{
				/// silently ignores malformed sets.
				channels.clear();
			}
			channels.push_back( channelName );
			channelSets.push_back( channels );
			channels.clear();
		}
		else
		{
			// simple data types are assigned to one image channel.

			channels.push_back( channelName );

			if ( channels.size() == 3 )
			{
				channelSets.push_back( channels );
				channels.clear();
			}
		}
	}

	bool first = true;
	ConversionInfo previous;
	ConversionInfo current;
	for( std::vector< ConversionInfo >::const_iterator it = conversions.begin() ; it != conversions.end(); ++it )
	{
		current = *it;


		if ( first )
		{
			assert( current.get<1>() == inputColorSpace );
			first = false;
		}
		else
		{
			assert( previous.get<2>() == current.get<1>() );
		}
		ModifyOpPtr currentConversion = (*current.get<0>())( current.get<1>(), current.get<2>() );
		assert( currentConversion );

		if ( !currentConversion->isInstanceOf( ChannelOpTypeId ) && !currentConversion->isInstanceOf( ColorTransformOpTypeId ) )
		{
			throw InvalidArgumentException( ( boost::format( "ColorSpaceTransformOp: '%s' to '%s' conversion registered unsupported Op type '%s'" ) % inputColorSpace % outputColorSpace % currentConversion->typeName()).str() );
		}

		currentConversion->inputParameter()->setValue( image );
		currentConversion->copyParameter()->setTypedValue( false );

		ImagePrimitivePtr result = 0;
		if ( currentConversion->isInstanceOf( ChannelOpTypeId )	)
		{
			// The channel Op doesn't handle any unpremultiplication of the colour channels.
			// So we apply an unpremult before and a premult after, if premultiplied is on and alpha exists
			if( premultipliedParameter()->getTypedValue() )
			{
				std::string alphaPrimVar = alphaPrimVarParameter()->getTypedValue();
				if( image->variables.find( alphaPrimVar ) != image->variables.end() )
				{
					ImageUnpremultiplyOpPtr unpremultOp = new ImageUnpremultiplyOp();
					unpremultOp->alphaChannelNameParameter()->setTypedValue( alphaPrimVar );
					unpremultOp->channelNamesParameter()->setTypedValue( channelNames );
					unpremultOp->copyParameter()->setTypedValue( false );
					unpremultOp->inputParameter()->setValue( image );
					unpremultOp->operate();
				}
			}

			
			ChannelOpPtr op = assertedStaticCast< ChannelOp >( currentConversion );
			op->channelNamesParameter()->setTypedValue( channelNames );
			result = runTimeCast< ImagePrimitive >( op->operate() );
			
			if( premultipliedParameter()->getTypedValue() )
			{
				std::string alphaPrimVar = alphaPrimVarParameter()->getTypedValue();
				if( result->variables.find( alphaPrimVar ) != result->variables.end() )
				{
					ImagePremultiplyOpPtr premultOp = new ImagePremultiplyOp();
					premultOp->alphaChannelNameParameter()->setTypedValue( alphaPrimVar );
					premultOp->channelNamesParameter()->setTypedValue( channelNames );
					premultOp->copyParameter()->setTypedValue( false );
					premultOp->inputParameter()->setValue( result );
					premultOp->operate();
				}
			}
		}
		else
		{
			assert( currentConversion->isInstanceOf( ColorTransformOpTypeId ) );

			ColorTransformOpPtr op = dynamicPointerCast< ColorTransformOp >( currentConversion );

			for ( ChannelSets::const_iterator it = channelSets.begin(); it != channelSets.end(); ++it )
			{
				op->inputParameter()->setValue( image );
				op->copyParameter()->setTypedValue( false );

				op->alphaPrimVarParameter()->setValue( alphaPrimVarParameter()->getValue() );
				op->premultipliedParameter()->setValue( premultipliedParameter()->getValue() );

				if ( it->size() == 1 )
				{
					op->colorPrimVarParameter()->setTypedValue( (*it)[0] );

					op->redPrimVarParameter()->setValue( op->redPrimVarParameter()->defaultValue()->copy() );
					op->greenPrimVarParameter()->setValue( op->greenPrimVarParameter()->defaultValue()->copy() );
					op->bluePrimVarParameter()->setValue( op->bluePrimVarParameter()->defaultValue()->copy() );
				}
				else
				{
					assert( it->size() == 3 );

					op->redPrimVarParameter()->setTypedValue( (*it)[0] );
					op->greenPrimVarParameter()->setTypedValue( (*it)[1] );
					op->bluePrimVarParameter()->setTypedValue( (*it)[2] );

					op->colorPrimVarParameter()->setValue( op->colorPrimVarParameter()->defaultValue()->copy() );

				}
				result = runTimeCast< ImagePrimitive >( op->operate() );
				assert( result.get() == image );
			}
		}

		assert( result.get() == image );
		( void ) result;

		previous = current;

	}
	assert( current.get<2>() == outputColorSpace );
}

ColorSpaceTransformOp::ConvertersMap &ColorSpaceTransformOp::converters()
{
	static ConvertersMap *m = new ConvertersMap();
	return *m;
}

ColorSpaceTransformOp::ConverterTypesMap &ColorSpaceTransformOp::converterTypes()
{
	static ConverterTypesMap *m = new ConverterTypesMap();
	return *m;
}

ColorSpaceTransformOp::ConversionsSet &ColorSpaceTransformOp::conversionsSet()
{
	static ConversionsSet *s = new ConversionsSet();
	return *s;
}
