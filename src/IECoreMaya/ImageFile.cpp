//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathLimits.h"

#include <GL/gl.h>

#include "maya/MGlobal.h"
#include "maya/MImage.h"
#include "maya/MImageFileInfo.h"

#include "IECore/ImageReader.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/VectorTypedData.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/DataConvert.h"

#include "IECoreMaya/ImageFile.h"

using namespace IECore;
using namespace IECoreMaya;

ImageFile::ImageFile() : m_rData( 0 ), m_gData( 0 ), m_bData( 0 ), m_aData( 0 )
{
}

ImageFile::~ImageFile()
{
}

void *ImageFile::creator()
{
	return new ImageFile;
}

struct ImageFile::ChannelConverter
{
	typedef FloatVectorDataPtr ReturnType;

	std::string m_pathName;
	std::string m_channelName;	

	template<typename T>
	ReturnType operator()( typename T::ConstPtr data )
	{
		assert( data );
			
		return DataConvert < T,  FloatVectorData, ScaledDataConversion< typename T::ValueType::value_type, float> >()( data );
	};
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			assert( data );
		
			throw InvalidArgumentException( ( boost::format( "ImageFile: Invalid data type \"%s\" for channel %s while reading %s" ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_channelName % functor.m_pathName ).str() );		
		}
	};
};

MStatus ImageFile::open( MString pathName, MImageFileInfo* info )
{
	ImagePrimitivePtr image;
	ImageReaderPtr reader;
	
	try
	{
		reader = runTimeCast<ImageReader>( Reader::create( pathName.asChar() ) );
		if (!reader)
		{		
			return MS::kFailure;
		}
		
		image = runTimeCast< ImagePrimitive >( reader->read() );
		if (!image)
		{
			return MS::kFailure;
		}
		
		if (!reader->isComplete())
		{
			return MS::kFailure;
		}				
	}
	catch ( std::exception &e )
	{		
		return MS::kFailure;
	}
				
	m_width = image->getDataWindow().size().x + 1;
	m_height = image->getDataWindow().size().y + 1;
	
	std::vector<std::string> channelNames;
	image->channelNames( channelNames );

	if ( std::find( channelNames.begin(), channelNames.end(), "R" ) == channelNames.end() 
		|| std::find( channelNames.begin(), channelNames.end(), "G" ) == channelNames.end() 
		|| std::find( channelNames.begin(), channelNames.end(), "B" ) == channelNames.end() )
	{	
		return MS::kFailure;
	}

	m_numChannels = 3;		
	if ( std::find( channelNames.begin(), channelNames.end(), "A" ) != channelNames.end() )
	{
		m_numChannels = 4;
	}		
	
	assert( m_numChannels == 3 || m_numChannels == 4 );
	
	try
	{	
	
		ChannelConverter converter;
		converter.m_pathName = pathName.asChar();

		DataPtr rData = image->variables["R"].data;
		if (! rData )
		{
			return MS::kFailure;
		}

		converter.m_channelName = "R";
		m_rData = despatchTypedData<			
				ChannelConverter, 
				TypeTraits::IsNumericVectorTypedData,
				ChannelConverter::ErrorHandler
			>( rData, converter );	

		DataPtr gData = image->variables["G"].data;
		if (! gData )
		{
			return MS::kFailure;
		}

		converter.m_channelName = "G";	
		m_gData = despatchTypedData<			
				ChannelConverter, 
				TypeTraits::IsNumericVectorTypedData,
				ChannelConverter::ErrorHandler
			>( gData, converter );		

		DataPtr bData = image->variables["B"].data;
		if (! bData )
		{
			return MS::kFailure;
		}

		converter.m_channelName = "B";	
		m_bData = despatchTypedData<			
				ChannelConverter, 
				TypeTraits::IsNumericVectorTypedData,
				ChannelConverter::ErrorHandler
			>( bData, converter );	

		if ( m_numChannels == 4 )
		{
			DataPtr aData = image->variables["A"].data;
			if (! aData )
			{
				return MS::kFailure;
			}

			converter.m_channelName = "A";	
			m_aData = despatchTypedData<			
				ChannelConverter, 
				TypeTraits::IsNumericVectorTypedData,
				ChannelConverter::ErrorHandler
			>( aData, converter );	
		}
	
	} 
	catch ( std::exception &e )
	{
		MGlobal::displayError( e.what() );	
		return MS::kFailure;
	}
	
	if (info)
	{
		info->width( m_width );
		info->height( m_height );
		
		info->channels( m_numChannels );	
		info->numberOfImages( 1 );

		info->imageType( MImageFileInfo::kImageTypeColor );
		info->pixelType( MImage::kFloat  );		
		info->hardwareType( MImageFileInfo::kHwTexture2D  );						
	}

	return MS::kSuccess;
}

void ImageFile::populateImage( float* pixels ) const
{
	assert( pixels );
			
	for (unsigned y = 0; y < m_height; y++)
	{		
		for (unsigned x = 0; x < m_width; x++)
		{					
			unsigned pixelIndex = ((m_height - y - 1) * m_width) + x;
				
			assert( pixelIndex < m_width * m_height );
		
			*pixels++ = m_rData->readable()[ pixelIndex ];
			*pixels++ = m_gData->readable()[ pixelIndex ];
			*pixels++ = m_bData->readable()[ pixelIndex ];

			if ( m_aData )
			{
				*pixels++ = m_aData->readable()[ pixelIndex ];
			}
		}
	}	
}

MStatus ImageFile::load( MImage& image, unsigned int idx )
{
	MStatus s;
	
	assert( idx == 0 );
	assert( m_rData	);
	assert( m_gData	);
	assert( m_bData	);		
	
	s = image.create( m_width, m_height, m_numChannels, MImage::kFloat );
	assert(s);
	
	image.setRGBA( true );
	
	populateImage( image.floatPixels() );	
	
	return MS::kSuccess;
}

MStatus ImageFile::glLoad( const MImageFileInfo& info, unsigned int idx )
{
	assert( idx == 0 );
	assert( m_rData	);
	assert( m_gData	);
	assert( m_bData	);
	
	std::vector<float> pixels;
	pixels.resize( m_width * m_height * m_numChannels );
	
	populateImage( &pixels[0] );
	
	switch (m_numChannels)
	{
		case 3:
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, &pixels[0] );
		case 4:
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, &pixels[0] );
		default:
			assert(false);
	}
		
	return MS::kSuccess;
}
	
