//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/WarpOp.h"
#include "IECore/Interpolator.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace Imath;
using namespace boost;

WarpOp::WarpOp( const std::string &name, const std::string &description )
	:	ImagePrimitiveOp( name, description )
{
	IntParameter::PresetsMap filterPresets;
	filterPresets["None"] = WarpOp::None;
	filterPresets["Bilinear"] = WarpOp::Bilinear;
	m_filterParameter = new IntParameter(
		"filter", 
		"Defines the filter to be used on the warped coordinates.",
		(int)WarpOp::Bilinear,
		(int)WarpOp::None, 
		(int)WarpOp::TypeCount - 1,
		filterPresets, 
		true
	);
	
	parameters()->addParameter( m_filterParameter );
	
}

WarpOp::~WarpOp()
{
}

IntParameterPtr WarpOp::filterParameter()
{
	return m_filterParameter;
}

ConstIntParameterPtr WarpOp::filterParameter() const
{
	return m_filterParameter;
}

struct WarpOp::Warp
{
	typedef void ReturnType;

	Warp( WarpOpPtr warp, WarpOp::FilterType filter, const Imath::Box2i &warpedDataWindow, const Imath::Box2i &originalDataWindow )
		:	m_warpOp( warp ), m_filter( filter ), m_outputDataWindow( warpedDataWindow ), m_inputDataWindow( originalDataWindow )
	{
	}

	inline void computePixelCoordinates( float x, float y, int &x1, int &y1, int &x2, int &y2, float &ratioX, float &ratioY )
	{
		Imath::V2f inPos = m_warpOp->warp( Imath::V2f( x, y ) );
		x1 = int(inPos.x);
		y1 = int(inPos.y);
		if ( x1 > inPos.x )
		{
			ratioX = x1 - inPos.x;
			x2 = x1;
			x1--;
		}
		else
		{
			x2 = x1 + 1;
			ratioX = inPos.x - x1;
		}
		if ( y1 > inPos.y )
		{
			ratioY = y1 - inPos.y;
			y2 = y1;
			y1--;
		}
		else
		{
			y2 = y1 + 1;
			ratioY = inPos.y - y1;
		}
		x1 -= m_inputDataWindow.min.x;
		y1 -= m_inputDataWindow.min.y;
		x2 -= m_inputDataWindow.min.x;
		y2 -= m_inputDataWindow.min.y;
	}

	template<typename V>
	inline V clampXY( const std::vector<V> &buffer, int x, int y, int width, int height ) const
	{
		x = ( x < 0 ? 0 : ( x >= width ? width - 1 : x ));
		y = ( y < 0 ? 0 : ( y >= height ? height - 1 : y ));
		return buffer[ x + y * width ];
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::value_type V;
		typedef typename Container::iterator It;
		
		const Container &inBuffer = data->readable();
		unsigned int outputWidth = m_outputDataWindow.size().x + 1;
		unsigned int inputWidth = m_inputDataWindow.size().x + 1;
		unsigned int inputHeight = m_inputDataWindow.size().y + 1;
		Container &outBuffer = data->writable();
		outBuffer.resize( outputWidth * (m_outputDataWindow.size().y + 1) );
		int x1, x2, y1, y2;
		float ratioX, ratioY;
		unsigned pixelIndex=0;
		double r1, r2, r;

		switch( m_filter )
		{
		case WarpOp::None:
			for( int y=m_outputDataWindow.min.y; y<=m_outputDataWindow.max.y; y++ )
			{
				for( int x=m_outputDataWindow.min.x; x<=m_outputDataWindow.max.x; x++, pixelIndex++ )
				{
					Imath::V2f inPos = m_warpOp->warp( Imath::V2f( x, y ) );
					x1 = int(inPos.x) - m_inputDataWindow.min.x;
					y1 = int(inPos.y) - m_inputDataWindow.min.y;
					outBuffer[pixelIndex] = clampXY<V>( inBuffer, x1, y1, inputWidth, inputHeight);
				}
			}
			break;

		case WarpOp::Bilinear:
			for( int y=m_outputDataWindow.min.y; y<=m_outputDataWindow.max.y; y++ )
			{
				for( int x=m_outputDataWindow.min.x; x<=m_outputDataWindow.max.x; x++, pixelIndex++ )
				{
					computePixelCoordinates( x, y, x1, y1, x2, y2, ratioX, ratioY );
					LinearInterpolator<double>()( (double)clampXY<V>( inBuffer, x1, y1, inputWidth, inputHeight ), 
												  (double)clampXY<V>( inBuffer, x2, y1, inputWidth, inputHeight ), ratioX, r1 );
					LinearInterpolator<double>()( (double)clampXY<V>( inBuffer, x1, y2, inputWidth, inputHeight ), 
												  (double)clampXY<V>( inBuffer, x2, y2, inputWidth, inputHeight ), ratioX, r2 );
					LinearInterpolator<double>()( r1, r2, ratioY, r );
					outBuffer[pixelIndex] = (V)r;
				}
			}
			break;

		default:
			throw Exception("Invalid filter type!");
		}
	}
	
	private :
		WarpOpPtr m_warpOp;
		WarpOp::FilterType m_filter;
		Imath::Box2i m_outputDataWindow;
		Imath::Box2i m_inputDataWindow;
};

void WarpOp::modifyTypedPrimitive( ImagePrimitivePtr image, ConstCompoundObjectPtr operands )
{
	Imath::Box2i originalDataWindow = image->getDataWindow();

	begin( operands );
	Imath::Box2i newDataWindow = warpedDataWindow( originalDataWindow );
	std::string error;
	Warp w( this, (FilterType)m_filterParameter->getNumericValue(), newDataWindow, originalDataWindow );
	for( PrimitiveVariableMap::iterator it = image->variables.begin(); it != image->variables.end(); it++ )
	{
		if( it->second.interpolation!=PrimitiveVariable::Vertex &&
			it->second.interpolation!=PrimitiveVariable::Varying &&
			it->second.interpolation!=PrimitiveVariable::FaceVarying )
		{
			continue;
		}

		if ( !image->channelValid( it->second, &error ) )
		{
			throw Exception( error );
		}
		despatchTypedData<Warp, TypeTraits::IsNumericVectorTypedData>( it->second.data, w );
	}
	end();
	image->setDataWindow( newDataWindow );
}

Imath::Box2i WarpOp::warpedDataWindow( const Imath::Box2i &dataWindow ) const
{
	return dataWindow;
}

void WarpOp::begin( ConstCompoundObjectPtr operands )
{
}

void WarpOp::end()
{
}
