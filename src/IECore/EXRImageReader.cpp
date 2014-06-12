//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
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

#include "math.h"

#include "IECore/EXRImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"
#include "IECore/MessageHandler.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/TimeCodeData.h"

#include "boost/format.hpp"

#include "OpenEXR/Iex.h"
#include "OpenEXR/ImfTestFile.h"
#include "OpenEXR/ImfFloatAttribute.h"
#include "OpenEXR/ImfDoubleAttribute.h"
#include "OpenEXR/ImfIntAttribute.h"
#include "OpenEXR/ImfBoxAttribute.h"
#include "OpenEXR/ImfVecAttribute.h"
#include "OpenEXR/ImfMatrixAttribute.h"
#include "OpenEXR/ImfStringAttribute.h"
#include "OpenEXR/ImfTimeCodeAttribute.h"

#ifdef IECORE_WITH_DEEPEXR

#include "OpenEXR/ImfPartType.h"

#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace Imf;
using namespace std;

using Imath::Box2i;

IE_CORE_DEFINERUNTIMETYPED( EXRImageReader );

const Reader::ReaderDescription<EXRImageReader> EXRImageReader::g_readerDescription("exr");

EXRImageReader::EXRImageReader() :
		ImageReader( "Reads ILM OpenEXR file format." ),
		m_inputFile( 0 )
{
}

EXRImageReader::EXRImageReader(const string &fileName) :
		ImageReader( "Reads ILM OpenEXR file format." ),
		m_inputFile( 0 )
{
	m_fileNameParameter->setTypedValue( fileName );
}

EXRImageReader::~EXRImageReader()
{
	delete m_inputFile;
}

bool EXRImageReader::canRead( const string &fileName )
{
	if( isOpenExrFile( fileName.c_str() ) )
	{

		Imf::InputFile *inputFile( NULL );
		
		try
		{
			
			// This will throw an exception if the image is deep and the EXR version is < 2.0.
			inputFile = new Imf::InputFile( fileName.c_str() );
			Imf::Header header( inputFile->header() );

#ifdef IECORE_WITH_DEEPEXR

			if ( header.hasType() )
			{
				/// \todo: determine why we are checking for the empty string
				if ( header.type() != TILEDIMAGE && header.type() != SCANLINEIMAGE && header.type() != "" )
				{
					throw IOException( "EXR is not a flat image." );
				}
			}

#endif

		}
		catch( ... )
		{
			delete inputFile;
			return false;
		}
		delete inputFile;

		return true;
	}

	return false;
}

void EXRImageReader::channelNames( vector<string> &names )
{
	open( true );

	const ChannelList &channels = m_inputFile->header().channels();
	names.clear();
	for( ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i )
	{
		names.push_back( i.name() );
	}
}

bool EXRImageReader::isComplete()
{
	if( !open() )
	{
		return false;
	}
	return m_inputFile->isComplete();
}

Imath::Box2i EXRImageReader::dataWindow()
{
	open( true );
	return m_inputFile->header().dataWindow();
}

Imath::Box2i EXRImageReader::displayWindow()
{
	open( true );
	return m_inputFile->header().displayWindow();
}

std::string EXRImageReader::sourceColorSpace() const
{
	return "linear";
}

template<class T>
DataPtr EXRImageReader::readTypedChannel( const std::string &name, const Imath::Box2i &dataWindow, const Imf::Channel *channel )
{
	assert( channel );
	Imath::V2i pixelDimensions = dataWindow.size() + Imath::V2i( 1 );
	unsigned numPixels = pixelDimensions.x * pixelDimensions.y;

	typedef TypedData<vector<T> > DataType;
	typename DataType::Ptr data = new DataType;
	data->writable().resize( numPixels );

	Imath::Box2i fullDataWindow = this->dataWindow();
	if( fullDataWindow.min.x==dataWindow.min.x && fullDataWindow.max.x==dataWindow.max.x )
	{
		// the width we want to read matches the width in the file, so we can read straight
		// into the result buffer
		FrameBuffer frameBuffer;
		T *buffer00 = data->baseWritable() - dataWindow.min.y * pixelDimensions.x - fullDataWindow.min.x;
		Slice slice( channel->type, (char *)buffer00, sizeof(T), sizeof(T) * pixelDimensions.x );
		frameBuffer.insert( name.c_str(), slice );
		m_inputFile->setFrameBuffer( frameBuffer );
		// exr library will choose the best order to read scanlines automatically (increasing or decreasing)
		try
		{
			m_inputFile->readPixels( dataWindow.min.y, dataWindow.max.y );
		}
		catch( Iex::InputExc &e )
		{
			// so we can read incomplete files
			msg( Msg::Warning, "EXRImageReader::readChannel", e.what() );
			return data;
		}
	}
	else
	{
		// widths don't match, we need to read into a temporary buffer and then transfer just
		// the bits we need into the result buffer.
		int numTmpPixels = fullDataWindow.size().x + 1;
		vector<T> tmpBuffer( numTmpPixels );
		T *tmpBufferTransferStart = &(tmpBuffer[0]) + dataWindow.min.x - fullDataWindow.min.x;
		size_t tmpBufferTransferLength = pixelDimensions.x * sizeof( T );
		T *transferDestination = &(data->writable()[0]);

		// slice has yStride of 0 so each successive scanline just overwrites the previous one
		Slice slice( channel->type, (char *)(&(tmpBuffer[0]) - fullDataWindow.min.x), sizeof(T), 0 );
		FrameBuffer frameBuffer;
		frameBuffer.insert( name.c_str(), slice );
		m_inputFile->setFrameBuffer( frameBuffer );

		int yStart = dataWindow.min.y;
		int yEnd = dataWindow.max.y;
		int yStep = 1;
		try
		{
			for( int y=yStart; y!=(yEnd+yStep); y+=yStep )
			{
				m_inputFile->readPixels( y );
				memcpy( (char *)transferDestination, (const char *)tmpBufferTransferStart, tmpBufferTransferLength );
				transferDestination += pixelDimensions.x;
			}
		}
		catch( Iex::InputExc &e )
		{
			// so we can read incomplete files
			msg( Msg::Warning, "EXRImageReader::readChannel", e.what() );
			return data;
		}
	}

#ifndef NDEBUG
	for ( typename DataType::ValueType::const_iterator it = data->readable().begin(); it != data->readable().end(); ++it )
	{
		assert( *it == *it ); // Will fail iff NaN
	}
#endif

	return data;
}

DataPtr EXRImageReader::readChannel( const string &name, const Imath::Box2i &dataWindow, bool raw )
{
	open( true );

	try
	{

		const Channel *channel = m_inputFile->header().channels().findChannel( name.c_str() );
		assert( channel );
		assert( channel->xSampling==1 ); /// \todo Support subsampling when we have a need for it
		assert( channel->ySampling==1 );

		DataPtr res;
		switch( channel->type )
		{
			case UINT :
				BOOST_STATIC_ASSERT( sizeof( unsigned int ) == 4 );
				res = readTypedChannel<unsigned int>( name, dataWindow, channel );
				if ( raw )
				{
					return res;
				}
				else
				{
					DataConvert< UIntVectorData, FloatVectorData, ScaledDataConversion< unsigned int, float > > converter;
					ConstUIntVectorDataPtr vec = staticPointerCast< UIntVectorData >(res);
					return converter( vec );
				}

			case HALF :
				res = readTypedChannel<half>( name, dataWindow, channel );
				if ( raw )
				{
					return res;
				}
				else
				{
					DataConvert< HalfVectorData, FloatVectorData, ScaledDataConversion< half, float > > converter;
					ConstHalfVectorDataPtr vec = staticPointerCast< HalfVectorData >(res);
					return converter( vec );
				}

			case FLOAT :
				BOOST_STATIC_ASSERT( sizeof( float ) == 4 );
				return readTypedChannel<float>( name, dataWindow, channel );

			default:
				throw IOException( ( boost::format( "EXRImageReader : Unsupported data type for channel \"%s\"" ) % name ).str() );

		}
	}
	catch ( Exception &e )
	{
		throw;
	}
	catch( Iex::BaseExc &e )
	{
		std::string s = ( boost::format( "EXRImageReader : %s" ) % e.what() ).str();
		e.assign( s.c_str() );
		throw e;
	}
	catch ( std::exception &e )
	{
		throw IOException( ( boost::format( "EXRImageReader : %s" ) % e.what() ).str() );
	}
	catch ( ... )
	{
		throw IOException( "EXRImageReader : Unexpected error" );
	}
}

bool EXRImageReader::open( bool throwOnFailure )
{
	if( m_inputFile && fileName()==m_inputFile->fileName() )
	{
		// we already opened the right file successfully
		return true;
	}

	delete m_inputFile;
	m_inputFile = 0;

	try
	{
		m_inputFile = new Imf::InputFile( fileName().c_str() );
	}
	catch( ... )
	{
		delete m_inputFile;
		m_inputFile = 0;
		if( !throwOnFailure )
		{
			return false;
		}
		else
		{
			throw IOException( string( "Failed to open file \"" ) + fileName() + "\"" );
		}
	}

	return true;
}

static DataPtr attributeToData( const Imf::Attribute &attr )
{
	if ( !strcmp( "float", attr.typeName() ) )
	{
		return new FloatData( static_cast< const Imf::FloatAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "double", attr.typeName() ) )
	{
		return new DoubleData( static_cast< const Imf::DoubleAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "int", attr.typeName() ) )
	{
		return new IntData( static_cast< const Imf::IntAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "box2i", attr.typeName() ) )
	{
		return new Box2iData( static_cast< const Imf::Box2iAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "box2f", attr.typeName() ) )
	{
		return new Box2fData( static_cast< const Imf::Box2fAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "m33f", attr.typeName() ) )
	{
		return new M33fData( static_cast< const Imf::M33fAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "m44f", attr.typeName() ) )
	{
		return new M44fData( static_cast< const Imf::M44fAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "string", attr.typeName() ) )
	{
		return new StringData( static_cast< const Imf::StringAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "v2i", attr.typeName() ) )
	{
		return new V2iData( static_cast< const Imf::V2iAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "v2f", attr.typeName() ) )
	{
		return new V2fData( static_cast< const Imf::V2fAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "v3i", attr.typeName() ) )
	{
		return new V3iData( static_cast< const Imf::V3iAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "v3f", attr.typeName() ) )
	{
		return new V3fData( static_cast< const Imf::V3fAttribute & >( attr ).value() );
	}
	else if ( !strcmp( "timecode", attr.typeName() ) )
	{
		return new TimeCodeData( static_cast< const Imf::TimeCodeAttribute & >( attr ).value() );
	}
	return 0;
}

static void addHeaderAttribute( const std::string &name, Data *data, CompoundData *blindData )
{
	// search for '.'
	string::size_type pos = name.find_first_of( "." );

	if ( string::npos != pos )
	{
		std::string thisName = name.substr(0, pos);
		std::string newName = name.substr( pos+1, name.size() );
		CompoundDataMap::iterator cIt = blindData->writable().find( thisName );

		// create compound data
		CompoundDataPtr newBlindData = 0;
		if ( cIt != blindData->writable().end() )
		{
			if ( cIt->second->typeId() == CompoundDataTypeId )
			{
				newBlindData = staticPointerCast< CompoundData >( cIt->second );
			}
		}
		if ( !newBlindData )
		{
			// create compound data
			newBlindData = new CompoundData();
			// add to current blind data
			blindData->writable()[ thisName ] = newBlindData;
		}
		// call recursivelly
		addHeaderAttribute( newName, data, newBlindData );
		return;
	}
	// add blind data key
	blindData->writable()[ name ] = data;
}

static void headerToCompoundData( const Imf::Header &header, CompoundData *blindData )
{
	for ( Imf::Header::ConstIterator attrIt = header.begin(); attrIt != header.end(); attrIt++ )
	{
		std::string name = attrIt.name();
		DataPtr data = attributeToData( attrIt.attribute() );
		if ( data )
		{
			addHeaderAttribute( name, data, blindData );
		}
	}
}

// \todo This function may be useful on other situations. Add as Converter?
static void compoundDataToCompoundObject( const CompoundData *data, CompoundObject *object )
{
	for ( CompoundDataMap::const_iterator it = data->readable().begin(); it != data->readable().end(); it++ )
	{
		if ( it->second->typeId() == CompoundDataTypeId )
		{
			CompoundObjectPtr newObject = new CompoundObject();
			object->members()[ it->first ] = newObject;
			compoundDataToCompoundObject( staticPointerCast< CompoundData >( it->second ), newObject );
		}
		else
		{
			object->members()[ it->first ] = staticPointerCast< Data >( it->second );
		}
	}
}

CompoundObjectPtr EXRImageReader::readHeader()
{
	CompoundObjectPtr header = ImageReader::readHeader();
	CompoundDataPtr tmp = new CompoundData();
	headerToCompoundData( m_inputFile->header(), tmp );
	compoundDataToCompoundObject( tmp, header );
	return header;
}

ObjectPtr EXRImageReader::doOperation( const CompoundObject *operands )
{
	ImagePrimitivePtr image = staticPointerCast< ImagePrimitive >( ImageReader::doOperation( operands ) );
	if ( image )
	{
		headerToCompoundData( m_inputFile->header(), image->blindData() );
	}
	return image;
}
