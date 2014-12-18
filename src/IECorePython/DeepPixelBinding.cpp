//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp" // this include /must/ come first!

#include "boost/format.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"

#include "IECore/DeepPixel.h"
#include "IECorePython/RefCountedBinding.h"

#include "IECorePython/DeepPixelBinding.h"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

struct DeepPixelHelper
{
	static DeepPixelPtr Constructor( unsigned numSamples )
	{
		return new DeepPixel( "RGBA", numSamples );
	}
	
	static DeepPixelPtr Constructor2( object names, unsigned numSamples )
	{
		extract<std::string> extractor( names );
		if ( extractor.check() )
		{
			return new DeepPixel( extractor(), numSamples );
		}
		
		std::vector<std::string> channelNames;
		container_utils::extend_container( channelNames, names );
		
		return new DeepPixel( channelNames, numSamples );
	}
	
	static tuple range( ConstDeepPixelPtr pixel )
	{
		return make_tuple( pixel->min(), pixel->max() );
	}
	
	static unsigned adjustIndex( ConstDeepPixelPtr pixel, long index )
	{
		unsigned numSamples = pixel->numSamples();
		
		if ( index < 0 )
		{
			index += numSamples;
		}
		
		if ( index < 0 || index >= numSamples )
		{
			PyErr_SetString( PyExc_IndexError, "Index out of range" );
			throw_error_already_set();
		}
		
		return index;
	}
	
	static float getDepth( ConstDeepPixelPtr pixel, long index )
	{
		return pixel->getDepth( adjustIndex( pixel, index ) );
	}
	
	static void setDepth( DeepPixelPtr pixel, long index, float depth )
	{
		pixel->setDepth( adjustIndex( pixel, index ), depth );
	}
	
	static void addSample( DeepPixelPtr pixel, float depth, object data )
	{
		std::vector<float> d;
		container_utils::extend_container( d, data );
		
		unsigned numChannels = pixel->numChannels();
		if ( numChannels != d.size() )
		{
			PyErr_SetString( PyExc_TypeError, ( boost::format( "Channel data must contain %d floats" ) % numChannels ).str().c_str() );
			throw_error_already_set();
		}
		
		std::vector<float> channelData( pixel->numChannels() );
		
		for ( unsigned i=0; i < numChannels; i++ )
		{
			channelData[i] = d[i];
		}
		
		pixel->addSample( depth, &channelData[0] );
	}
	
	static void removeSample( DeepPixelPtr pixel, long index )
	{
		pixel->removeSample( adjustIndex( pixel, index ) );
	}
	
	static tuple channelData( ConstDeepPixelPtr pixel, long index )
	{
		list result;
		
		const float *data = pixel->channelData( adjustIndex( pixel, index ) );
		unsigned numChannels = pixel->numChannels();
		for ( unsigned c=0; c < numChannels; ++c )
		{
			result.append( data[c] );
		}

		return tuple( result );
	}
	
	static void setItem( DeepPixelPtr pixel, long index, object data )
	{
		std::vector<float> d;
		container_utils::extend_container( d, data );
		
		unsigned numChannels = pixel->numChannels();
		if ( numChannels != d.size() )
		{
			PyErr_SetString( PyExc_TypeError, ( boost::format( "Channel data must contain %d floats" ) % numChannels ).str().c_str() );
			throw_error_already_set();
		}
		
		float *channelData = pixel->channelData( adjustIndex( pixel, index ) );
		
		for ( unsigned i=0; i < numChannels; i++ )
		{
			channelData[i] = d[i];
		}
	}
	
	static tuple interpolatedChannelData( ConstDeepPixelPtr pixel, float depth )
	{
		list result;
		
		unsigned numChannels = pixel->numChannels();

#ifdef _MSC_VER
		float* data = new float[numChannels];
#else
		float data[numChannels];
#endif
		pixel->interpolatedChannelData( depth, data );
		for ( unsigned c=0; c < numChannels; ++c )
		{
			result.append( data[c] );
		}
#ifdef _MSC_VER
		delete[] data;
#endif
		return tuple( result );
	}
	
	static tuple channelNames( ConstDeepPixelPtr pixel )
	{
		list result;
		
		const std::vector<std::string> *names = pixel->channelNames();
		for ( std::vector<std::string>::const_iterator it=names->begin(); it != names->end(); ++it )
		{
			result.append( *it );
		}

		return tuple( result );
	}
	
	static list composite( ConstDeepPixelPtr pixel )
	{
		unsigned numChannels = pixel->numChannels();
		
#ifdef _MSC_VER
		float* data = new float[numChannels];
#else
		float data[numChannels];
#endif
		
		pixel->composite( data );
		
		list result;
		for ( unsigned c=0; c < numChannels; ++c )
		{
			result.append( data[c] );
		}
#ifdef _MSC_VER
		delete[] data;
#endif
		return result;
	}
	
	static DeepPixelPtr average( object pixels, object weights )
	{
		std::vector<const DeepPixel *> pixelVector;
		container_utils::extend_container( pixelVector, pixels );
		
		std::vector<float> weightVector;
		container_utils::extend_container( weightVector, weights );
		
		return DeepPixel::average( pixelVector, weightVector );
	}
};

void bindDeepPixel()
{
	RefCountedClass<DeepPixel, RefCounted>( "DeepPixel" )
		.def( "__init__", make_constructor( &DeepPixelHelper::Constructor2, default_call_policies(), ( boost::python::arg_( "channelNames" ), boost::python::arg_( "numSamples" ) = 0 ) ) )
		.def( "__init__", make_constructor( &DeepPixelHelper::Constructor, default_call_policies(), ( boost::python::arg_( "numSamples" ) = 0 ) ) )
		.def( init<const DeepPixel &>() )
		.def( "numSamples", &DeepPixel::numSamples )
		.def( "__len__", &DeepPixel::numSamples )
		.def( "range", &DeepPixelHelper::range )
		.def( "min", &DeepPixel::min )
		.def( "max", &DeepPixel::max )
		.def( "getDepth", &DeepPixelHelper::getDepth )
		.def( "setDepth", &DeepPixelHelper::setDepth )
		.def( "addSample", &DeepPixelHelper::addSample )
		.def( "removeSample", &DeepPixelHelper::removeSample )
		.def( "__delitem__", &DeepPixelHelper::removeSample )
		.def( "channelData", &DeepPixelHelper::channelData )
		.def( "__getitem__", &DeepPixelHelper::channelData )
		.def( "__setitem__", &DeepPixelHelper::setItem )
		.def( "interpolatedChannelData", &DeepPixelHelper::interpolatedChannelData )
		.def( "numChannels", &DeepPixel::numChannels )
		.def( "channelIndex", &DeepPixel::channelIndex )
		.def( "channelNames", &DeepPixelHelper::channelNames )
		.def( "merge", &DeepPixel::merge )
		.def( "composite", &DeepPixelHelper::composite )
		.def( "average", &DeepPixelHelper::average ).staticmethod( "average" )
	;
}

} // namespace IECorePython
