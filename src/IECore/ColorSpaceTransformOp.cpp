//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/Object.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Exception.h"

#include "IECore/ColorSpaceTransformOp.h"

using namespace IECore;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ColorSpaceTransformOp );

ColorSpaceTransformOp::ColorSpaceTransformOp()
	:	ImagePrimitiveOp( "ColorSpaceTransformOp", 
				   "Converts channels from one named colorspace to another. Additional colorspaces can be registered at runtime."
		)
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
	
	parameters()->addParameter( m_inputColorSpaceParameter );
	parameters()->addParameter( m_outputColorSpaceParameter );	
}

ColorSpaceTransformOp::~ColorSpaceTransformOp()
{
}

StringParameterPtr ColorSpaceTransformOp::inputColorSpaceParameter()
{
	return m_inputColorSpaceParameter;
}

ConstStringParameterPtr ColorSpaceTransformOp::inputColorSpaceParameter() const
{
	return m_inputColorSpaceParameter;
}
		
StringParameterPtr ColorSpaceTransformOp::outputColorSpaceParameter()
{
	return m_outputColorSpaceParameter;
}

ConstStringParameterPtr ColorSpaceTransformOp::outputColorSpaceParameter() const
{
	return m_outputColorSpaceParameter;
}

void ColorSpaceTransformOp::registerConversion( const InputColorSpace &inputColorSpace, const OutputColorSpace &outputColorSpace, CreatorFn creator, void *data )
{
	if ( inputColorSpace != outputColorSpace )
	{		
		Conversion conversion( inputColorSpace, outputColorSpace );	
	
		if ( conversionsSet().find( conversion ) != conversionsSet().end() )
		{
			throw InvalidArgumentException( ( boost::format( "ColorSpaceTransformOp: Converter for '%s' to '%s 'registered twice" ) % inputColorSpace % outputColorSpace ).str() );
		}
		conversionsSet().insert( conversion );

		converterTypes()[ creator ] = conversion;
		
		ConversionInfo info( creator, inputColorSpace, outputColorSpace, data );

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

void ColorSpaceTransformOp::modifyTypedPrimitive( ImagePrimitivePtr image, ConstCompoundObjectPtr operands )
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
	
	bool first = true;
	ConversionInfo previous;	
	std::vector< ConversionInfo >::const_iterator it = conversions.begin();
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
		ImagePrimitiveOpPtr currentConversion = (current.get<0>())( current.get<1>(), current.get<2>(), current.get<3>() );
		assert( currentConversion );
		if ( !currentConversion->isInstanceOf( ChannelOpTypeId ) && !currentConversion->isInstanceOf( ColorTransformOpTypeId ) )		
		{
			throw InvalidArgumentException( ( boost::format( "ColorSpaceTransformOp: '%s' to '%s' conversion registered unsupported Op type '%s'" ) % inputColorSpace % outputColorSpace % currentConversion->typeName()).str() );
		}
				
		currentConversion->inputParameter()->setValue( image );
		currentConversion->copyParameter()->setTypedValue( false );		
		/// \todo Set any other parameters, e.g channelNames, etc, as appropriate
		
		ImagePrimitivePtr result = runTimeCast< ImagePrimitive >( currentConversion->operate() );
		assert( result.get() == image.get() );
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
