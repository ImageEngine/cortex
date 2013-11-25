//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include <stdio.h>

#include "ai_drivers.h"
#include "ai_version.h"

#include "IECore/DisplayDriver.h"
#include "IECore/BoxAlgo.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"

#include "IECoreArnold/ToArnoldConverter.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreArnold;

#if ( ( AI_VERSION_ARCH_NUM * 100 ) + AI_VERSION_MAJOR_NUM ) >= 401
#define ARNOLD_4_1
#endif

static void driverParameters( AtList *params, AtMetaDataStore *metaData )
{
	AiParameterSTR( "driverType", "" );

	// we need to specify this metadata to keep MtoA happy.
	AiMetaDataSetStr( metaData, 0, "maya.attr_prefix", "" );
	AiMetaDataSetStr( metaData, 0, "maya.translator", "ie" );
}

static void driverInitialize( AtNode *node, AtParamValue *parameters )
{
	AiDriverInitialize( node, true, new DisplayDriverPtr );
}

static void driverUpdate( AtNode *node, AtParamValue *parameters )
{
}

static bool driverSupportsPixelType( const AtNode *node, AtByte pixelType )
{
	switch( pixelType )
	{
		case AI_TYPE_RGB :
		case AI_TYPE_RGBA :
		case AI_TYPE_FLOAT :
		case AI_TYPE_VECTOR :
		case AI_TYPE_POINT :
			return true;
		default:
			return false;
	}
}

static const char **driverExtension()
{
   return 0;
}

static void driverOpen( AtNode *node, struct AtOutputIterator *iterator, AtBBox2 displayWindow, AtBBox2 dataWindow, int bucketSize )
{	
	std::vector<std::string> channelNames;
	
	const char *name = 0;
	int pixelType = 0;
	while( AiOutputIteratorGetNext( iterator, &name, &pixelType, 0 ) )
	{
		std::string namePrefix;
		if( strcmp( name, "RGB" ) && strcmp( name, "RGBA" ) )
		{
			namePrefix = std::string( name ) + ".";
		}
		
		switch( pixelType )
		{
			case AI_TYPE_RGB :
			case AI_TYPE_VECTOR :
			case AI_TYPE_POINT :
				channelNames.push_back( namePrefix + "R" );
				channelNames.push_back( namePrefix + "G" );
				channelNames.push_back( namePrefix + "B" );
				break;
			case AI_TYPE_RGBA :
				channelNames.push_back( namePrefix + "R" );
				channelNames.push_back( namePrefix + "G" );
				channelNames.push_back( namePrefix + "B" );
				channelNames.push_back( namePrefix + "A" );
				break;
			case AI_TYPE_FLOAT :
				// no need for prefix because it's not a compound type
				channelNames.push_back( name );
				break;
		}
	}

	/// \todo Make Convert.h
	Box2i cortexDisplayWindow(
		V2i( displayWindow.minx, displayWindow.miny ),
		V2i( displayWindow.maxx, displayWindow.maxy )
	);

	Box2i cortexDataWindow(
		V2i( dataWindow.minx, dataWindow.miny ),
		V2i( dataWindow.maxx, dataWindow.maxy )
	);
		
	CompoundDataPtr parameters = new CompoundData();
	ToArnoldConverter::getParameters( node, parameters->writable() );	

	const char *driverType = AiNodeGetStr( node, "driverType" );
	
	DisplayDriverPtr *driver = (DisplayDriverPtr *)AiDriverGetLocalData( node );
	try
	{
		*driver = IECore::DisplayDriver::create( driverType, cortexDisplayWindow, cortexDataWindow, channelNames, parameters );
	}
	catch( const std::exception &e )
	{
		// we have to catch and report exceptions because letting them out into pure c land
		// just causes aborts.
		msg( Msg::Error, "ieOutputDriver:driverOpen", e.what() );
	}
}

#ifdef ARNOLD_4_1

static bool driverNeedsBucket( AtNode *node, int x, int y, int sx, int sy, int tId )
{
	return true;
}

#endif // ARNOLD_4_1

static void driverPrepareBucket( AtNode *node, int x, int y, int sx, int sy, int tId )
{
}

#ifdef ARNOLD_4_1

static void driverProcessBucket( AtNode *node, struct AtOutputIterator *iterator, struct AtAOVSampleIterator *sample_iterator, int x, int y, int sx, int sy, int tId )
{
}

#endif // ARNOLD_4_1

static void driverWriteBucket( AtNode *node, struct AtOutputIterator *iterator, struct AtAOVSampleIterator *sampleIterator, int x, int y, int sx, int sy )
{
	DisplayDriverPtr *driver = (DisplayDriverPtr *)AiDriverGetLocalData( node );
	if( !*driver )
	{
		return;
	}
	
	const int numOutputChannels = (*driver)->channelNames().size();

	std::vector<float> interleavedData;
	interleavedData.resize( sx * sy * numOutputChannels );

	int pixelType = 0;
	const void *bucketData;
	int outChannelOffset = 0;
	while( AiOutputIteratorGetNext( iterator, 0, &pixelType, &bucketData ) )
	{
		int numChannels = 0;
		switch( pixelType )
		{
			case AI_TYPE_RGB :
			case AI_TYPE_VECTOR :
			case AI_TYPE_POINT :
				numChannels = 3;
				break;
			case AI_TYPE_RGBA :
				numChannels = 4;
				break;
			case AI_TYPE_FLOAT :
				numChannels = 1;
				break;
		}
		
		for( int c = 0; c < numChannels; c++ )
		{
			float *in = (float *)(bucketData) + c;
			float *out = &(interleavedData[0]) + outChannelOffset;
			for( int j = 0; j < sy; j++ )
			{
				for( int i = 0; i < sx; i++ )
				{
					*out = *in;
					out += numOutputChannels;
					in += numChannels;
				}
			}
			outChannelOffset += 1;
		}
		
	}
	
	Box2i bucketBox(
		V2i( x, y ),
		V2i( x + sx - 1, y + sy - 1 )
	);

	try
	{
		(*driver)->imageData( bucketBox, &(interleavedData[0]), interleavedData.size() );
	}
	catch( const std::exception &e )
	{
		// we have to catch and report exceptions because letting them out into pure c land
		// just causes aborts.
		msg( Msg::Error, "ieOutputDriver:driverWriteBucket", e.what() );
	}
}

static void driverClose( AtNode *node, struct AtOutputIterator *iterator )
{
	DisplayDriverPtr *driver = (DisplayDriverPtr *)AiDriverGetLocalData( node );
	if( *driver )
	{
		try
		{
			(*driver)->imageClose(); 
		}
		catch( const std::exception &e )
		{
			// we have to catch and report exceptions because letting them out into pure c land
			// just causes aborts.
			msg( Msg::Error, "ieOutputDriver:driverClose", e.what() );
		}
	}
}

static void driverFinish( AtNode *node )
{
	DisplayDriverPtr *driver = (DisplayDriverPtr *)AiDriverGetLocalData( node );
	delete driver;
	AiDriverDestroy( node );
}

AI_EXPORT_LIB bool NodeLoader( int i, AtNodeLib *node )
{
	if( i==0 )
	{		
		static AtCommonMethods commonMethods = { 
			driverParameters,
			driverInitialize,
			driverUpdate,
			driverFinish
		};
		static AtDriverNodeMethods driverMethods = {
			driverSupportsPixelType,
			driverExtension,
			driverOpen,
#ifdef ARNOLD_4_1
			driverNeedsBucket,
#endif
			driverPrepareBucket,
#ifdef ARNOLD_4_1
			driverProcessBucket,
#endif
			driverWriteBucket,
			driverClose
		};
		static AtNodeMethods nodeMethods = {
			&commonMethods,
			&driverMethods
		};
		
		node->node_type = AI_NODE_DRIVER;
		node->output_type = AI_TYPE_NONE;
		node->name = "ieDisplay";
		node->methods = &nodeMethods;
		sprintf( node->version, AI_VERSION );
		
		return true;
	}

	return false;
}
