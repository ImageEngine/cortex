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

#include "IECore/BoxOps.h"
#include "IECore/YUVImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

#include <fstream>

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( YUVImageWriter )

const Writer::WriterDescription<YUVImageWriter> YUVImageWriter::m_writerDescription("yuv");

YUVImageWriter::YUVImageWriter() :
		ImageWriter("YUVImageWriter", "Serializes images to the Joint Photographic Experts Group (JPEG) format")
{
	constructParameters();
}

YUVImageWriter::YUVImageWriter(ObjectPtr image, const string &fileName) :
		ImageWriter("YUVImageWriter", "Serializes images to the YUV (YCbCr)")
{
	constructParameters();
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

void YUVImageWriter::constructParameters()
{
	IntParameter::PresetsContainer formatPresets;
	formatPresets.push_back( IntParameter::Preset( "yuv420p", YUV420P ) );
	formatPresets.push_back( IntParameter::Preset( "yuv422p", YUV422P ) );
	formatPresets.push_back( IntParameter::Preset( "yuv444p", YUV444P ) );

	m_formatParameter = new IntParameter(
	        "format",
	        "Specifies the desired layout of the output file",
	        YUV420P,
	        formatPresets
	);
	
	V2fParameter::PresetsContainer kBkRPresets;
	kBkRPresets.push_back( V2fParameter::Preset( "rec601", V2f( 0.114,  0.299  ) ) );
	kBkRPresets.push_back( V2fParameter::Preset( "rec709", V2f( 0.0722, 0.2126 ) ) );
	m_kBkRParameter = new V2fParameter(
		"kBkR",
		"Defines the constants kB/kR for calculating YUV (Y'CbCr) components",
		V2f( 0.0722, 0.2126 ),
		kBkRPresets,
		false
	);
	
	Box3f defaultRange = Box3f( V3f( 0.0f, 0.0f, 0.0f ), V3f( 1.0f, 1.0f, 1.0f ) );
	
	Box3fParameter::PresetsContainer rangePresets;
	rangePresets.push_back( Box3fParameter::Preset( "zeroToOne", defaultRange ) );
	rangePresets.push_back( Box3fParameter::Preset( "standardScaling", Box3f( 
		V3f( 
			16.0f / 255.0f, 
			16.0f / 255.0f,
			16.0f / 255.0f
		), 
		V3f( 
			235.0f / 255.0f, 
			240.0f / 255.0f, 
			240.0f / 255.0f		
		)
	) ) );
	
	m_rangeParameter = new Box3fParameter(
		"range",
		"Specifies the floating-point min/max of the YUV components, to allow rescaling due to addition of head-room and/or toe-room. The standardScaling preset "
		"gives, in 8-bit representation, a standard range of 16-235 for Y, and 16-240 for U and V",
		
		defaultRange,
		rangePresets
	);
				
	parameters()->addParameter( m_formatParameter );
	parameters()->addParameter( m_kBkRParameter );
	parameters()->addParameter( m_rangeParameter );
}

YUVImageWriter::~YUVImageWriter()
{
}

std::string YUVImageWriter::destinationColorSpace() const
{
	return "srgb";
}

IntParameterPtr YUVImageWriter::formatParameter()
{
	return m_formatParameter;
}

ConstIntParameterPtr YUVImageWriter::formatParameter() const
{
	return m_formatParameter;
}
	
V2fParameterPtr YUVImageWriter::kBkRParameter()
{
	return m_kBkRParameter;
}

ConstV2fParameterPtr YUVImageWriter::kBkRParameter() const
{
	return m_kBkRParameter;
}
	
Box3fParameterPtr YUVImageWriter::rangeParameter()
{
	return m_rangeParameter;
}

ConstBox3fParameterPtr YUVImageWriter::rangeParameter() const
{
	return m_rangeParameter;
}

struct YUVImageWriter::ChannelConverter
{
	typedef void ReturnType;

	std::string m_channelName;
	ConstImagePrimitivePtr m_image;	
	Box2i m_dataWindow;
	int m_channelOffset;
	Color3fVectorDataPtr m_rgbData;

	ChannelConverter( const std::string &channelName, ConstImagePrimitivePtr image, const Box2i &dataWindow, int channelOffset, Color3fVectorDataPtr rgbData  ) 
	: m_channelName( channelName ), m_image( image ), m_dataWindow( dataWindow ), m_channelOffset( channelOffset ), m_rgbData( rgbData )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr dataContainer )
	{
		assert( dataContainer );

		const typename T::ValueType &data = dataContainer->readable();
		ScaledDataConversion<typename T::ValueType::value_type, float> converter;
		
		typedef boost::multi_array_ref< const typename T::ValueType::value_type, 2 > SourceArray2D;
		typedef boost::multi_array_ref< Color3f, 2 > TargetArray2D;
		
		const SourceArray2D sourceData( &data[0], extents[ m_image->getDataWindow().size().y + 1 ][ m_image->getDataWindow().size().x + 1 ] );
		TargetArray2D targetData( &m_rgbData->writable()[0], extents[ m_image->getDisplayWindow().size().y + 1 ][ m_image->getDisplayWindow().size().x + 1 ] );
		
		const Box2i copyRegion = boxIntersection( m_dataWindow, boxIntersection( m_image->getDisplayWindow(), m_image->getDataWindow() ) );
		
		for ( int y = copyRegion.min.y; y <= copyRegion.max.y ; y++ )
		{
			for ( int x = copyRegion.min.x; x <= copyRegion.max.x ; x++ )
			{				
				targetData[ y - m_image->getDisplayWindow().min.y + copyRegion.min.y ][ x - m_image->getDisplayWindow().min.x + copyRegion.min.x ][ m_channelOffset ]
					= converter( sourceData[ y - m_image->getDataWindow().min.y ][ x - m_image->getDataWindow().min.x ] );			
			}
		}
	};
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			throw InvalidArgumentException( ( boost::format( "YUVImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_channelName ).str() );		
		}
	};
};

void YUVImageWriter::writeImage( const vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow ) const
{
	const V2f &kBkR = m_kBkRParameter->getTypedValue();
	const Box3f &range = m_rangeParameter->getTypedValue();	
	
	if ( range.isEmpty() )
	{
		throw InvalidArgumentException("YUVImageWriter: Empty YUV range specified ");	
	}
	
	const float kB = kBkR.x;
	const float kR = kBkR.y;	

	int displayWidth  = 1 + image->getDisplayWindow().size().x;
	int displayHeight = 1 + image->getDisplayWindow().size().y;
	
	if ( displayWidth % 2 != 0 || displayHeight % 2 != 0 )
	{
		throw IOException("YUVImageWriter: Unsupported resolution");
	}

	vector<string>::const_iterator rIt = std::find( names.begin(), names.end(), "R" );
	vector<string>::const_iterator gIt = std::find( names.begin(), names.end(), "G" );	
	vector<string>::const_iterator bIt = std::find( names.begin(), names.end(), "B" );
	
	if ( rIt == names.end() || gIt == names.end() || bIt == names.end() )
	{
		throw IOException("YUVImageWriter: Unsupported channel names specified");		
	} 

	std::ofstream outFile( fileName().c_str(), std::ios::trunc | std::ios::binary | std::ios::out );
	if (! outFile.is_open() )
	{
		throw IOException("Could not open '" + fileName() + "' for writing.");
	}
		
	Color3fVectorDataPtr rgbData = new Color3fVectorData();
	rgbData->writable().resize( displayWidth * displayHeight, Color3f(0,0,0) );
	
	try
	{	
		for ( vector<string>::const_iterator it = names.begin(); it != names.end(); it++ )
		{
			const string &name = *it;
			
			if (!( name == "R" || name == "G" || name == "B" ) )
			{
				msg( Msg::Warning, "YUVImageWriter", format( "Channel \"%s\" was not encoded." ) % name );
				continue;
			}

			int channelOffset = 0;
			if ( name == "R" )
			{
				channelOffset = 0;
			}
			else if ( name == "G" )
			{
				channelOffset = 1;
			}
			else 
			{
				assert( name == "B" );
				channelOffset = 2;
			}
			
			// get the image channel
			assert( image->variables.find( name ) != image->variables.end() );			
			DataPtr dataContainer = image->variables.find( name )->second.data;
			assert( dataContainer );
			
			ChannelConverter converter( *it, image, dataWindow, channelOffset, rgbData );
		
			despatchTypedData<			
				ChannelConverter, 
				TypeTraits::IsNumericVectorTypedData,
				ChannelConverter::ErrorHandler
			>( dataContainer, converter );	

		}
		
		V3fVectorDataPtr yuvData = new V3fVectorData();
		yuvData->writable().resize( displayWidth * displayHeight, V3f(0,0,0) );
		
		assert( yuvData->readable().size() == rgbData->readable().size() );
		
		for ( int i = 0; i < displayWidth * displayHeight; i ++ )
		{
			Color3f rgb = rgbData->readable()[i]; 
			
			if ( rgb.x < 0.0 ) rgb.x = 0;
			if ( rgb.x > 1.0 ) rgb.x = 1.0;			
			
			if ( rgb.y < 0.0 ) rgb.y = 0;
			if ( rgb.y > 1.0 ) rgb.y = 1.0;			
			
			if ( rgb.z < 0.0 ) rgb.z = 0;
			if ( rgb.z > 1.0 ) rgb.z = 1.0;									
			
			V3f yPbPr;
			
			float &Y = yPbPr.x;
			float &Pb = yPbPr.y;
			float &Pr = yPbPr.z;	
			
			Y = kR * rgb.x + ( 1.0 - kR - kB ) * rgb.y + kB * rgb.z;
			Pb = 0.5 * ( rgb.z - Y ) / ( 1.0 - kB );
			Pr = 0.5 * ( rgb.x - Y ) / ( 1.0 - kR );
			
			V3f yCbCr = yPbPr;
			
			/// Map from 0-1
			yCbCr.y += 0.5;
			yCbCr.z += 0.5;	

			/// Apply any scaling for "head-room" and "toe-room"
			yCbCr.x	= ( yCbCr.x * ( range.max.x - range.min.x ) ) + range.min.x;
			yCbCr.y	= ( yCbCr.y * ( range.max.y - range.min.y ) ) + range.min.y;
			yCbCr.z	= ( yCbCr.z * ( range.max.z - range.min.z ) ) + range.min.z;
			
			yuvData->writable()[i] = yCbCr;
		}
		
		/// \todo Chroma-filtering. Ideally we should do a proper sampled downsize of the chroma, rather than skipping data
		/// elements. This would avoid any aliasing.
		
		int lumaStepX = 1;
		int lumaStepY = 1;
		
		int chromaUStepX = 1;
		int chromaUStepY = 1;		
		
		int chromaVStepX = 1;		
		int chromaVStepY = 1;
		
		switch ( m_formatParameter->getNumericValue() )
		{
			case YUV420P :
				/// Half res in U and V
				chromaUStepX = 2;
				chromaUStepY = 2;
				chromaVStepX = 2;
				chromaVStepY = 2;				
				break;
			case YUV422P :
				/// Half horizonal res in U and V
				chromaUStepX = 2;
				chromaUStepY = 1;
				chromaVStepX = 2;
				chromaVStepY = 1;				
				break;	
			case YUV444P :
				/// Full res in U and V
				break;		
			default :
				assert( false );
		}				
					
		ScaledDataConversion<float, unsigned char> converter;
		/// Y-plane				
		for ( int y = 0; y < displayHeight; y += lumaStepX )
		{
			for ( int x = 0; x < displayWidth; x += lumaStepY )
			{								
				const V3f yCbCr = yuvData->readable()[y*displayWidth + x];
				
				const unsigned char val = converter( yCbCr.x );
				
				outFile.write( (const char*)&val, 1 );
			}
		}
		
		/// U-plane			
		for ( int y = 0; y < displayHeight; y+=chromaUStepX )
		{
			for ( int x = 0; x < displayWidth; x+=chromaUStepY )
			{								
				const V3f yCbCr = yuvData->readable()[y*displayWidth + x];
				
				const unsigned char val = converter( yCbCr.y );
				
				outFile.write( (const char*)&val, 1 );
			}
		}
		
		/// V-plane			
		for ( int y = 0; y < displayHeight; y+=chromaVStepX )
		{
			for ( int x = 0; x < displayWidth; x+=chromaVStepY )
			{								
				const V3f yCbCr = yuvData->readable()[y*displayWidth + x];
				
				const unsigned char val = converter( yCbCr.z );
				
				outFile.write( (const char*)&val, 1 );
			}
		}		
	}
	catch ( std::exception &e )
	{
		throw IOException( ( boost::format( "YUVImageWriter : %s" ) % e.what() ).str() );
	}
	catch ( ... )
	{
		throw IOException( "YUVImageWriter: Unexpected error" );
	}
}
