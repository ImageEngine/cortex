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
	:	ChannelOp( name, description )
{
	IntParameter::PresetsMap filterPresets;
	filterPresets["None"] = WarpOp::None;
	filterPresets["Bilinear"] = WarpOp::Bilinear;
	m_filterParameter = new IntParameter(
		"filter", 
		"Defines the filter to be used on the warped coordinates.",
		(int)WarpOp::None,
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

	Warp( WarpOpPtr warp, WarpOp::FilterType filter, const Imath::Box2i &dataWindow )
		:	m_warpOp( warp ), m_filter( filter ), m_dataWindow( dataWindow )
	{
	}

	inline void computePixelCoordinates( float x, float y, int &x1, int &y1, int &x2, int &y2, float &ratioX, float &ratioY )
	{
		Imath::V2f inPos = m_warpOp->warp( Imath::V2f( x, y ) );
		x1 = int(inPos.x);
		y1 = int(inPos.y);
		x2 = ( inPos.x < 0 ? x1 - 1 : x1 + 1 );
		y2 = ( inPos.y < 0 ? y1 - 1 : y1 + 1 );
		ratioX = inPos.x - x1;
		ratioY = inPos.y - y1;
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::value_type V;
		typedef typename Container::iterator It;
		
		const Container &inBuffer = data->readable();
		Container &outBuffer = data->writable();
		int x1, x2, y1, y2;
		float ratioX, ratioY;
		unsigned int width = m_dataWindow.size().y + 1;
		unsigned pixelIndex=0;
		double r1, r2, r;

		switch( m_filter )
		{
		case WarpOp::None:
			for( int y=m_dataWindow.min.y + 1; y<=m_dataWindow.max.y; y++ )
			{
				for( int x=m_dataWindow.min.x; x<=m_dataWindow.max.x; x++, pixelIndex++ )
				{
					Imath::V2f inPos = m_warpOp->warp( Imath::V2f( x, y ) );
					x1 = int(inPos.x);
					y1 = int(inPos.y);
					outBuffer[pixelIndex] = inBuffer[ x1 + y1 * width ];
				}
			}
			break;

		case WarpOp::Bilinear:
			for( int y=m_dataWindow.min.y + 1; y<=m_dataWindow.max.y; y++ )
			{
				for( int x=m_dataWindow.min.x; x<=m_dataWindow.max.x; x++, pixelIndex++ )
				{
					computePixelCoordinates( x, y, x1, y1, x2, y2, ratioX, ratioY );
					LinearInterpolator<double>()( (double)inBuffer[ x1 + y1 * width ], (double)inBuffer[ x2 + y1 * width ], ratioX, r1 );
					LinearInterpolator<double>()( (double)inBuffer[ x1 + y2 * width ], (double)inBuffer[ x2 + y2 * width ], ratioX, r2 );
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
		Box2i m_dataWindow;

};

void WarpOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	Warp w( this, (FilterType)m_filterParameter->getNumericValue(), dataWindow );
	begin( static_pointer_cast<const CompoundObject>( parameters()->getValue() ) );
	for( unsigned i=0; i<channels.size(); i++ )
	{
		despatchTypedData<Warp, TypeTraits::IsNumericVectorTypedData>( channels[i], w );
	}
	end();
}

void WarpOp::begin( ConstCompoundObjectPtr operands )
{
}

void WarpOp::end()
{
}
