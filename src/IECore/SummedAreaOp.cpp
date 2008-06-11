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

#include "IECore/SummedAreaOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

using namespace IECore;
using namespace std;
using namespace Imath;

SummedAreaOp::SummedAreaOp()
	:	ChannelOp( staticTypeName(), "Calculates summed area table for image channels." )
{
}

SummedAreaOp::~SummedAreaOp()
{
}

struct SummedAreaOp::SumArea
{
	typedef void ReturnType;

	SumArea( const Imath::Box2i &dataWindow )
		:	m_dataWindow( dataWindow )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::value_type V;
		typedef typename Container::iterator It;
		
		Container &buffer = data->writable();
		
		// deal with first row alone, as it doesn't have values above it
		unsigned pixelIndex=0;
		V rowSum = 0;
		for( int x=m_dataWindow.min.x; x<=m_dataWindow.max.x; x++, pixelIndex++ )
		{
			rowSum += buffer[pixelIndex];
			buffer[pixelIndex] = rowSum;
		}
		// now do the other rows
		unsigned upperPixelIndex = 0;
		pixelIndex = m_dataWindow.size().x + 1;
		for( int y=m_dataWindow.min.y + 1; y<=m_dataWindow.max.y; y++ )
		{
			rowSum = 0;
			for( int x=m_dataWindow.min.x; x<=m_dataWindow.max.x; x++ )
			{
				rowSum += buffer[pixelIndex];
				buffer[pixelIndex++] = rowSum + buffer[upperPixelIndex++];
			}
		}
	}
	
	private :
	
		Box2i m_dataWindow;

};
	
void SummedAreaOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{	
	SumArea summer( dataWindow );
	for( unsigned i=0; i<channels.size(); i++ )
	{
		despatchTypedData<SumArea, TypeTraits::IsNumericVectorTypedData>( channels[i], summer );
	}
}
		
