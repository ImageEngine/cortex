//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2014, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/DeepPixel.h"
#include "IECore/Exception.h"
#include "IECore/Interpolator.h"

using namespace IECore;

DeepPixel::DeepPixel( const std::string channelNames, unsigned numSamples )
	: m_sorted( false ),
	m_channels( channelNames.size() )
{
	m_depthIndices.reserve( numSamples );
	m_samples.reserve( numSamples * ( 1 + channelNames.size() ) );
	
	unsigned i = 0;
	for ( std::string::const_iterator it = channelNames.begin(); it != channelNames.end(); ++it, ++i )
	{
		m_channels[i] = *it;
	}
}

DeepPixel::DeepPixel( const std::vector<std::string> &channelNames, unsigned numSamples )
	: m_sorted( false ) 
{
	m_depthIndices.reserve( numSamples );
	m_samples.reserve( numSamples * ( 1 + channelNames.size() ) );
	
	m_channels.reserve( channelNames.size() );
	for ( std::vector<std::string>::const_iterator it = channelNames.begin(); it != channelNames.end(); ++it )
	{
		m_channels.push_back( *it );
	}
}

DeepPixel::DeepPixel( const DeepPixel &pixel )
{
	m_channels.reserve( pixel.numChannels() );
	const std::vector<std::string> *names = pixel.channelNames();
	for ( std::vector<std::string>::const_iterator it = names->begin(); it != names->end(); ++it )
	{
		m_channels.push_back( *it );
	}
	
	merge( &pixel );
}

DeepPixel::~DeepPixel()
{
}

unsigned DeepPixel::numSamples() const
{
	return m_depthIndices.size();
}

float DeepPixel::min() const
{
	if ( !numSamples() )
	{
		return 0.0;
	}
	
	sort();
	
	return m_samples[ *m_depthIndices.begin() ];
}

float DeepPixel::max() const
{
	if ( !numSamples() )
	{
		return 0.0;
	}
	
	sort();
	
	return m_samples[ *m_depthIndices.rbegin() ];;
}

float DeepPixel::getDepth( unsigned index ) const
{
	sort();
	
	if( index >= numSamples() )
	{
		throw IECore::InvalidArgumentException( ( boost::format( "DeepPixel::getDepth: Depth index %d does not exist" ) % index ).str() );
	}
	
	return m_samples[ m_depthIndices[index] ];
}

class DeepPixel::DepthComparison
{	
	public :
		
		DepthComparison( const DeepPixel *pixel ) : m_pixel( pixel )
		{
		}
		
		bool operator ()( unsigned a, unsigned b )
		{
			return ( m_pixel->m_samples[ a ] < m_pixel->m_samples[ b ] );
		}
	
	private :
		
		const DeepPixel *m_pixel;

};

void DeepPixel::setDepth( unsigned index, float depth )
{
	if( index >= numSamples() )
	{
		throw IECore::InvalidArgumentException( ( boost::format( "DeepPixel::setDepth: Depth index %d does not exist" ) % index ).str() );
	}
	
	sort();
	
	m_samples[ m_depthIndices[index] ] = depth;
	
	std::make_heap( m_depthIndices.begin(), m_depthIndices.end(), DepthComparison( this ) );
	
	m_sorted = false;
}

void DeepPixel::addSample( float depth, const float *channelData )
{
	unsigned sampleIndex = m_samples.size();
	unsigned numChannels = this->numChannels();
	m_samples.reserve( sampleIndex + numChannels + 1 );
	
	m_samples.push_back( depth );
	for ( unsigned i=0; i < numChannels; ++i )
	{
		m_samples.push_back( channelData[i] );
	}
	
	m_depthIndices.push_back( sampleIndex );
	
	if ( m_sorted )
	{
		std::make_heap( m_depthIndices.begin(), m_depthIndices.end(), DepthComparison( this ) );
	}
	else
	{
		std::push_heap( m_depthIndices.begin(), m_depthIndices.end(), DepthComparison( this ) );
	}
	
	m_sorted = false;
}

void DeepPixel::removeSample( unsigned index )
{
	sort();
	
	/// \todo: should this also remove the data from m_samples? do we have to worry about the heap?
	m_depthIndices.erase( m_depthIndices.begin() + index );
}

float *DeepPixel::channelData( unsigned index )
{
	sort();
	
	if( index >= numSamples() )
	{
		throw IECore::InvalidArgumentException( ( boost::format( "DeepPixel::channelData: Depth index %d does not exist" ) % index ).str() );
	}
	
	return &m_samples[ m_depthIndices[index]+1 ];
}

const float *DeepPixel::channelData( unsigned index ) const
{
	return const_cast<DeepPixel*>( this )->channelData( index );
}

void DeepPixel::interpolatedChannelData( float depth, float *result ) const
{
	sort();
	
	unsigned numSamples = this->numSamples();
	unsigned numChannels = this->numChannels();
	
	unsigned i = 0;
	for ( ; i < numSamples; ++i )
	{
		if ( m_samples[ m_depthIndices[i] ] > depth )
		{
			break;
		}
	}
	
	if ( i == 0 )
	{
		const float *data = channelData( i );
		for ( unsigned c=0; c < numChannels; ++c )
		{
			result[c] = data[c];
		}
	}
	else if ( i == numSamples )
	{
		const float *data = channelData( numSamples - 1 );
		for ( unsigned c=0; c < numChannels; ++c )
		{
			result[c] = data[c];
		}
	}
	else
	{
		float previousDepth = getDepth( i - 1 );
		float nextDepth = getDepth( i );
		float weight = ( depth - previousDepth ) / ( nextDepth - previousDepth );
		
		const float *previousData = channelData( i - 1 );
		const float *nextData = channelData( i );
		
		LinearInterpolator<float> lerp;
		for ( unsigned c=0; c < numChannels; ++c )
		{
			lerp( previousData[c], nextData[c], weight, result[c] );
		}
	}
}

unsigned DeepPixel::numChannels() const
{
	return m_channels.size();
}

int DeepPixel::channelIndex( const std::string &name ) const
{
	std::vector<std::string>::const_iterator it = std::find( m_channels.begin(), m_channels.end(), name );
	if ( it == m_channels.end() )
	{
		return -1;
	}
	
	return it - m_channels.begin();
}

const std::vector<std::string> *DeepPixel::channelNames() const
{
	return &m_channels;
}

void DeepPixel::merge( const DeepPixel *pixel )
{
	unsigned numSamples = pixel->numSamples();
	m_depthIndices.reserve( this->numSamples() + numSamples );
	m_samples.reserve( m_samples.size() + numSamples * ( pixel->numChannels() + 1 ) );
	
	for ( unsigned i=0; i < numSamples; ++i )
	{
		addSample( pixel->getDepth( i ), pixel->channelData( i ) );
	}
}

void DeepPixel::composite( float *result ) const
{
	unsigned numChannels = this->numChannels();
	int alphaChannel = channelIndex( "A" );
	if ( alphaChannel < 0 )
	{
		const float *data = channelData( 0 );
		for ( unsigned c=0; c < numChannels; ++c )
		{
			result[c] = data[c];
		}
		
		return;
	}
		
	for ( unsigned c=0; c < numChannels; ++c )
	{
		result[c] = 0.0;
	}
	
	float alpha = 1.0;
	unsigned numSamples = this->numSamples();
	for ( unsigned i=0; i < numSamples && result[alphaChannel] < 1.0; ++i )
	{
		const float *data = channelData( i );
		for ( unsigned c=0; c < numChannels; ++c )
		{
			result[c] += data[c] * alpha;
		}
		
		alpha = std::max( 1 - result[alphaChannel], 0.0f );
	}
}

DeepPixelPtr DeepPixel::average( std::vector<const DeepPixel *> &pixels, std::vector<float> &weights )
{
	unsigned numPixels = pixels.size();
	if ( !numPixels || numPixels != weights.size() )
	{
		throw IECore::InvalidArgumentException( "DeepPixel::average: There must be one weight per pixel" );
	}
	
	unsigned numChannels = pixels[0]->numChannels();
	for ( unsigned p=0; p < numPixels; ++p )
	{
		if ( numChannels != pixels[p]->numChannels() )
		{
			throw IECore::InvalidArgumentException( "DeepPixel::average: All pixels must have the same number of channels" );
		}
	}
	
	unsigned numSamples = pixels[0]->numSamples();
	
	DeepPixelPtr result = new DeepPixel( *pixels[0]->channelNames(), numSamples );
	
	#ifdef _MSC_VER
	float *averageData = new float[numChannels];
	float *currentData = new float[numChannels];
	#else
	float averageData[numChannels];
	float currentData[numChannels];
	#endif
	
	for ( unsigned i=0; i < numSamples; ++i )
	{
		for ( unsigned c=0; c < numChannels; ++c )
		{
			averageData[c] = 0.0;
		}
		
		float depth = pixels[0]->getDepth( i );
		
		for ( unsigned p=0; p < numPixels; ++p )
		{
			pixels[p]->interpolatedChannelData( depth, currentData );
			for ( unsigned c=0; c < numChannels; ++c )
			{
				averageData[c] += weights[p] * currentData[c];
			}
		}
		
		result->addSample( depth, averageData );
	}
	#ifdef _MSC_VER
	delete[] currentData;
	delete[] averageData;
	#endif
	
	return result;
}

void DeepPixel::sort()
{
	if ( m_sorted )
	{
		return;
	}
	
	std::sort_heap( m_depthIndices.begin(), m_depthIndices.end(), DepthComparison( this ) );
	
	m_sorted = true;
}

void DeepPixel::sort() const
{
	const_cast<DeepPixel*>( this )->sort();
}
