//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include <vector>

#include "IECore/DisplayDriver.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreRI/Dspy.h"

using namespace std;
using namespace IECoreRI;
using namespace IECore;
using namespace Imath;

// deal with UserParameter naming differences between prman and 3delight
#ifdef PRMANEXPORT

#define UP_VALUETYPE vtype
#define UP_VALUECOUNT vcount

#else

#define UP_VALUETYPE valueType
#define UP_VALUECOUNT valueCount

#endif

//////////////////////////////////////////////////////////////////////////
// Registration
//////////////////////////////////////////////////////////////////////////

class Dspy::Registration
{
	
	public :
		
		Registration()
		{
			PtDspyDriverFunctionTable table;
			memset( &table, 0, sizeof( table ) );
			table.Version = k_PtDriverCurrentVersion;
			table.pOpen = &imageOpen;
			table.pQuery = &imageQuery;
			table.pWrite = &imageData;
			table.pClose = &imageClose;

			DspyRegisterDriverTable( "ie", &table );
		}

};

Dspy::Registration Dspy::g_registration;

//////////////////////////////////////////////////////////////////////////
// Functions
//////////////////////////////////////////////////////////////////////////

PtDspyError Dspy::imageOpen( PtDspyImageHandle *image, const char *driverName, const char *fileName, int width, int height, int paramcount, const UserParameter *parameters, int formatCount, PtDspyDevFormat *format, PtFlagStuff *flags )
{

	*image = 0;
	
	// get channel names
	
	vector<string> channels;

	if( formatCount == 1 )
	{
		channels.push_back( "R" );
	}
	else if( formatCount == 3 )
	{
		channels.push_back( "R" );
		channels.push_back( "G" );
		channels.push_back( "B" );
	}
	else if( formatCount == 4 )
	{
		channels.push_back( "R" );
		channels.push_back( "G" );
		channels.push_back( "B" );
		channels.push_back( "A" );
	}
	else
	{
		msg( Msg::Error, "Dspy::imageOpen", "Invalid number of channels!" );
		return PkDspyErrorBadParams;
	}
	for( int i = 0; i < formatCount; i++ )
	{
		format[i].type = PkDspyFloat32 | PkDspyByteOrderNative;
	}

	// process the parameter list. we use some of the parameters to help determine
	// the display and data windows, and the others we convert ready to passed to
	// DisplayDriver::create().

	V2i originalSize( width, height );
	V2i origin( 0 );

	CompoundDataPtr convertedParameters = new CompoundData;
	
	for( int p = 0; p < paramcount; p++ )
	{		
		if ( !strcmp( parameters[p].name, "OriginalSize" ) && parameters[p].UP_VALUETYPE == (char)'i' && parameters[p].UP_VALUECOUNT == (char)2 && parameters[p].nbytes == (int) (parameters[p].UP_VALUECOUNT * sizeof(int)) )
		{
			originalSize.x = static_cast<const int *>(parameters[p].value)[0];
			originalSize.y = static_cast<const int *>(parameters[p].value)[1];
		}
		else if ( !strcmp( parameters[p].name, "origin" ) && parameters[p].UP_VALUETYPE == (char)'i' && parameters[p].UP_VALUECOUNT == (char)2 && parameters[p].nbytes == (int)(parameters[p].UP_VALUECOUNT * sizeof(int)) )
		{
			origin.x = static_cast<const int *>(parameters[p].value)[0];
			origin.y = static_cast<const int *>(parameters[p].value)[1];
		}
		else
		{
			DataPtr newParam;

			if ( !parameters[p].nbytes )
			{
				continue;
			}

			const int *pInt;
			const float *pFloat;
			char const **pChar;

			// generic converter
			switch( parameters[p].UP_VALUETYPE )
			{
			case 'i':
				// sanity check
				if ( parameters[p].nbytes / parameters[p].UP_VALUECOUNT != sizeof(int) )
				{
					msg( Msg::Error, "Dspy::imageOpen", "Invalid int data size" );
					continue;
				}
				pInt = static_cast<const int *>(parameters[p].value);
				if ( parameters[p].UP_VALUECOUNT == 1 )
				{
					newParam = new IntData( pInt[0] );
				}
				else
				{
					std::vector< int > newVec( pInt, pInt + parameters[p].UP_VALUECOUNT );
					newParam = new IntVectorData( newVec );
				}
				break;
			case 'f':
				if ( parameters[p].nbytes / parameters[p].UP_VALUECOUNT != sizeof(float) )
				{
					msg( Msg::Error, "Dspy::imageOpen", "Invalid float data size" );
					continue;
				}
				pFloat = static_cast<const float *>(parameters[p].value);
				if ( parameters[p].UP_VALUECOUNT == 1 )
				{
					newParam = new FloatData( pFloat[0] );
				}
				else
				{
					std::vector< float > newVec( pFloat, pFloat + parameters[p].UP_VALUECOUNT );
					newParam = new FloatVectorData( newVec );
				}
				break;
			case 's':
				if ( parameters[p].nbytes / parameters[p].UP_VALUECOUNT != sizeof(char*) )
				{
					msg( Msg::Error, "Dspy::imageOpen", "Invalid string data size" );
					continue;
				}
				pChar = (const char **)(parameters[p].value);
				if ( parameters[p].UP_VALUECOUNT == 1 )
				{
					newParam = new StringData( pChar[0] );
				}
				else
				{
					StringVectorDataPtr newStringVec = new StringVectorData();
					for ( int s = 0; s < parameters[p].UP_VALUECOUNT; s++ )
					{
						newStringVec->writable().push_back( pChar[s] );
					}
					newParam = newStringVec;
				}
				break;
			default :
				// we shouldn't ever get here...
				break;
			}
			if( newParam )
			{
				convertedParameters->writable()[ parameters[p].name ] = newParam;
			}
		}
	}

	convertedParameters->writable()[ "fileName" ] = new StringData( fileName );

	// calculate display and data windows

	Box2i displayWindow(
		V2i( 0 ),
		originalSize - V2i( 1 )
	);
	
	Box2i dataWindow(
		origin,
		origin + V2i( width - 1, height - 1)
	);
	
	// create the display driver

	IECore::DisplayDriverPtr dd = 0;
	try
	{
		const StringData *driverType = convertedParameters->member<StringData>( "driverType", true /* throw if missing */ );
		dd = DisplayDriver::create( driverType->readable(), displayWindow, dataWindow, channels, convertedParameters );
	}
	catch( std::exception &e )
	{
		msg( Msg::Error, "Dspy::imageOpen", e.what() );
		return PkDspyErrorUnsupported;
	}

	if( !dd )
	{
		msg( Msg::Error, "Dspy::imageOpen", "DisplayDriver::create returned 0." );
		return PkDspyErrorUnsupported;
	}

	// update flags and return

	if( dd->scanLineOrderOnly() )
	{
		flags->flags |= PkDspyFlagsWantsScanLineOrder;
	}
	
	dd->addRef(); // this will be removed in imageClose()
	*image = (PtDspyImageHandle)dd.get();
	return PkDspyErrorNone;

}


PtDspyError Dspy::imageQuery( PtDspyImageHandle image, PtDspyQueryType type, int size, void *data )
{
	DisplayDriver *dd = static_cast<DisplayDriver *>( image );
#ifdef PRMANEXPORT			
	return PkDspyErrorUnsupported;
#else				
	if( type == PkProgressiveQuery )
	{
		if( (!dd->scanLineOrderOnly()) && dd->acceptsRepeatedData() )
		{
			((PtDspyProgressiveInfo *)data)->acceptProgressive = 1;
		}
		else
		{
			((PtDspyProgressiveInfo *)data)->acceptProgressive = 0;
		}
		return PkDspyErrorNone;
	}
	return PkDspyErrorUnsupported;
#endif				
}

PtDspyError Dspy::imageData( PtDspyImageHandle image, int xMin, int xMaxPlusOne, int yMin, int yMaxPlusOne, int entrySize, const unsigned char *data )
{
	DisplayDriver *dd = static_cast<DisplayDriver *>( image );
	Box2i dataWindow = dd->dataWindow();

	// convert coordinates from cropped image to original image coordinates.
	Box2i box( V2i( xMin + dataWindow.min.x, yMin + dataWindow.min.y ), V2i( xMaxPlusOne - 1 + dataWindow.min.x, yMaxPlusOne - 1 + dataWindow.min.y ) );
	int channels = dd->channelNames().size();
	int blockSize = (xMaxPlusOne - xMin) * (yMaxPlusOne - yMin);
	int bufferSize = channels * blockSize;

	if( entrySize % sizeof(float) )
	{
		msg( Msg::Error, "Dspy::imageData", "The entry size is not multiple of sizeof(float)!" );
		return PkDspyErrorUnsupported;
	}

	if( entrySize == (int)(channels*sizeof(float)) )
	{
		try
		{
			dd->imageData( box, (const float *)data, bufferSize );
		}
		catch( std::exception &e )
		{
			if( strcmp( e.what(), "stop" ) == 0 )
			{
				/// \todo I would prefer DisplayDriver::imageData to have a return
				/// value which could be used to request stop/continue behaviour. 
				/// prman doesn't seem to support PkDspyErrorStop, which should
				/// also be resolved at some point.
#ifdef PRMANEXPORT			
				return PkDspyErrorUndefined;
#else				
				return PkDspyErrorStop;
#endif				
			}
			else
			{
				msg( Msg::Error, "Dspy::imageData", e.what() );
				return PkDspyErrorUndefined;
			}
		}
	}
	else
	{
		msg( Msg::Error, "Dspy::imageData", "Unexpected entry size value!" );
		return PkDspyErrorBadParams;
	}
	return PkDspyErrorNone;
}

PtDspyError Dspy::imageClose( PtDspyImageHandle image )
{
	if ( !image )
	{
		return PkDspyErrorNone;
	}

	DisplayDriver *dd = static_cast<DisplayDriver*>( image );
	try
	{
		dd->imageClose();
	}
	catch( std::exception &e )
	{
		msg( Msg::Error, "Dspy::imageClose", e.what() );
	}
	
	try
	{
		dd->removeRef();
	}
	catch( std::exception &e )
	{
		msg( Msg::Error, "DspyImageData", e.what() );
		return PkDspyErrorBadParams;
	}
	
	return PkDspyErrorNone;
}
