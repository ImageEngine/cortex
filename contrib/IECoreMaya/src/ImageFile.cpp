//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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


/// \todo Rework using despatchTypedData and ScaledDataConversion
template<typename T>
static float getFloatData( ConstDataPtr d, int idx )
{
	assert( d );
	assert( idx >= 0 );
	
	if ( idx >= (int)(runTimeCast< const T >(d)->readable().size()) )
	{
		return 0.0f;
	}
	
	return runTimeCast< const T >(d)->readable()[ idx ];	
}

template<typename T>
static float getNormalizedFloatData( ConstDataPtr d, int idx )
{
	assert( d );

	return getFloatData<T>( d, idx ) / Imath::limits<typename T::ValueType::value_type>::max();	
}

static float getFloatDataDespatch( ConstDataPtr d, int idx )
{
	assert( d );
	
	switch ( d->typeId() )
	{
		case FloatVectorDataTypeId:
			return getFloatData<FloatVectorData>( d, idx );
		case DoubleVectorDataTypeId:
			return getFloatData<DoubleVectorData>( d, idx );
		case HalfVectorDataTypeId:
			return getFloatData<HalfVectorData>( d, idx );
		case IntVectorDataTypeId:
			return getNormalizedFloatData<IntVectorData>( d, idx );
		case UIntVectorDataTypeId:
			return getNormalizedFloatData<UIntVectorData>( d, idx );
		case CharVectorDataTypeId:
			return getNormalizedFloatData<CharVectorData>( d, idx );
		case UCharVectorDataTypeId:
			return getNormalizedFloatData<UCharVectorData>( d, idx );
		case LongVectorDataTypeId:
			return getNormalizedFloatData<LongVectorData>( d, idx );
		default:
			throw Exception("Invalid channel data type");
	}
}

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
				
	m_width = image->width();
	m_height = image->height();
	
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
	
	m_rData = image->variables["R"].data;
	if (! m_rData )
	{
		return MS::kFailure;
	}
		
	m_gData = image->variables["G"].data;
	if (! m_gData )
	{
		return MS::kFailure;
	}
		
	m_bData = image->variables["B"].data;		
	if (! m_bData )
	{
		return MS::kFailure;
	}
	
	if ( m_numChannels == 4 )
	{
		m_aData = image->variables["A"].data;			
		if (! m_aData )
		{
			return MS::kFailure;
		}	
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
		
			*pixels++ = getFloatDataDespatch( m_rData, pixelIndex );
			*pixels++ = getFloatDataDespatch( m_gData, pixelIndex );			
			*pixels++ = getFloatDataDespatch( m_bData, pixelIndex );			

			if ( m_aData )
			{
				*pixels++ = getFloatDataDespatch( m_aData, pixelIndex );					
			}
		}
	}	
}

/// \todo Establish why this works perfectly for the viewport, software renderer, and hardware renderer
/// but NOT the swatch in the attribute editor!
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
	
	float *pixels = new float[ m_width * m_height * m_numChannels ];
	
	populateImage( pixels );
	
	switch (m_numChannels)
	{
		case 3:
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, pixels );
		case 4:
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, pixels );
		default:
			assert(false);
	}
	
	delete [] pixels;	
		
	return MS::kSuccess;
}
	
