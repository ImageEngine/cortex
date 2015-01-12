//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "tbb/parallel_for.h"

#include "IECore/ImageConvolveOp.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SphericalHarmonicsAlgo.h"
#include "IECore/Math.h"

using namespace tbb;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ImageConvolveOp );

ImageConvolveOp::ImageConvolveOp(): ChannelOp( "Convolves image channels using Spherical Harmonics. Based on \"An efficient representation for irradiance environment maps.\" by Ramamoorthi, Ravi and Hanrahan, Pat - 2001." )
{
	m_bands = new IntParameter(
		"bands",
		"Number of Spherical Harmonics bands used. Use 3 for diffuse convolution and higher values for specular convolution.",
		3,
		1,
		25
	);

	m_samples = new IntParameter( 
		"samples",
		"Number of samples taken randomly from the image to build the convolved image.",
		40000,
		0
	);

	parameters()->addParameter( m_bands );
	parameters()->addParameter( m_samples );
}

ImageConvolveOp::~ImageConvolveOp()
{
}

struct ImageConvolveOp::ParallelTask
{
	public :
		ParallelTask( const SphericalHarmonics<Imath::V3f> &sh, int imgWidth, int imgHeight, std::vector<float> &channel0, std::vector<float> &channel1, std::vector<float> &channel2 ) : m_sh(sh), m_imgWidth(imgWidth), m_imgHeight(imgHeight), m_channel0(channel0), m_channel1(channel1), m_channel2(channel2)
		{
		}

		void operator()( const blocked_range<int>& rangeY ) const
		{
			int Yoffset = rangeY.begin() * m_imgWidth;
			for( int iy=rangeY.begin(); iy!=rangeY.end(); ++iy, Yoffset += m_imgWidth )
			{
				for ( int ix = 0; ix < m_imgWidth; ix++ )
				{
					float phi = ((float)ix * ( M_PI * 2 )) / (float)m_imgWidth;
					float theta = ((float)iy * M_PI ) / (float)m_imgHeight;
					Imath::V3f value = m_sh( Imath::V2f(phi,theta) );
					int offset = Yoffset + ix;
					m_channel0[ offset ] = value[0];
					m_channel1[ offset ] = value[1];
					m_channel2[ offset ] = value[2];
				}
			}
		}

	private :

		const SphericalHarmonics<Imath::V3f> &m_sh;
		int m_imgWidth;
		int m_imgHeight;
		std::vector<float> &m_channel0;
		std::vector<float> &m_channel1;
		std::vector<float> &m_channel2;
};

void ImageConvolveOp::processChannels( SHProjectorf &projector, const SHf &kernel, int imgWidth, int imgHeight, unsigned bands,  std::vector<float> &channel0, std::vector<float> &channel1, std::vector<float> &channel2 ) const
{
	const std::vector< Imath::V2f > &directions = projector.sphericalCoordinates();
	std::vector< Imath::V2f >::const_iterator cit;

	SphericalHarmonics<Imath::V3f> sh( bands );

	unsigned int i;
	// image to SH
	for ( i = 0, cit = directions.begin(); i < directions.size(); i++, cit++ )
	{
		const Imath::V2f &phiTheta = *cit;
		int ix = (int)(phiTheta.x * (float)imgWidth / ( M_PI * 2 ));
		int iy = (int)(phiTheta.y * (float)imgHeight /  M_PI );
		if ( ix > imgWidth )
			ix = imgWidth;
		if ( iy > imgHeight )
			iy = imgHeight;
		int offset = iy * imgWidth + ix;
		projector( i, Imath::V3f( channel0[ offset ], channel1[ offset ], channel2[ offset ] ), sh );
	}

	// convolve with lambertian kernel
	sh.convolve( kernel );

	// SH to image
	ParallelTask task( sh, imgWidth, imgHeight, channel0, channel1, channel2 );
	parallel_for( blocked_range<int>( 0, imgHeight ), task );

}



void ImageConvolveOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	if ( displayWindow != dataWindow )
	{
		throw Exception( "ImageConvolveOp only works with images that display and data windows match." );
	}

	unsigned bands = m_bands->getNumericValue();
	unsigned samples = m_samples->getNumericValue();

	int imgWidth = dataWindow.size().x + 1;
	int imgHeight = dataWindow.size().y + 1;

	// create SH projector
	IECore::SHProjectorf projector( samples );
	projector.computeSamples( bands );

	SHf kernel = lambertianKernel<float>( bands, true );

	ChannelVector::iterator it = channels.begin();
	while( it != channels.end() )
	{
		std::vector<float> *ch0 = & assertedStaticCast< FloatVectorData >( *it )->writable();
		std::vector<float> *ch1 = ch0;
		std::vector<float> *ch2 = ch0;

		it++;
		if ( it != channels.end() )
		{
			ch1 = & assertedStaticCast< FloatVectorData >( *it )->writable();
			it++;
			if ( it != channels.end() )
			{
				ch2 = & assertedStaticCast< FloatVectorData >( *it )->writable();
				it++;
			}
			else
			{
				ch2 = ch1;
			}
		}
		processChannels( projector, kernel, imgWidth, imgHeight, bands, *ch0, *ch1, *ch2 );
	}
}
