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

#include "IECore/SplineToImage.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NullObject.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/VectorTraits.h"
#include "IECore/Exception.h"

using namespace IECore;
using namespace Imath;

SplineToImage::SplineToImage()
	:	Op( staticTypeName(), "Creates ImagePrimitives from SplineData", new IECore::ObjectParameter( "result", "An image of the spline.", new IECore::NullObject(), ImagePrimitiveTypeId ) )
{
	
	static TypeId splineTypes[] = { SplineffDataTypeId, SplinefColor3fDataTypeId, InvalidTypeId };
	m_splineParameter = new ObjectParameter(
		"spline",
		"The spline to be converted to an ImagePrimitive.",
		new IECore::NullObject(),
		splineTypes
	);
	
	m_resolutionParameter = new V2iParameter(
		"resolution",
		"The resolution of the created ImagePrimitive",
		V2i( 128 )
	);
	
	parameters()->addParameter( m_splineParameter );
	parameters()->addParameter( m_resolutionParameter );
	
}

SplineToImage::~SplineToImage()
{
}

ObjectParameterPtr SplineToImage::splineParameter()
{
	return m_splineParameter;
}

ConstObjectParameterPtr SplineToImage::splineParameter() const
{
	return m_splineParameter;
}
		
V2iParameterPtr SplineToImage::resolutionParameter()
{
	return m_resolutionParameter;
}

ConstV2iParameterPtr SplineToImage::resolutionParameter() const
{
	return m_resolutionParameter;
}

struct SplineToImage::CreateImage
{
	typedef ImagePrimitivePtr ReturnType;

	CreateImage( SplineToImage *parent )
		:	m_parent( parent )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		typedef typename T::ValueType SplineType;
		typedef typename SplineType::YType YType;
		typedef typename SplineType::XType XType;
		typedef VectorTraits<YType> YTraits;
	
		const SplineType &spline = data->readable();
		const typename SplineType::XInterval splineInterval = spline.interval();
		if( boost::numeric::empty( splineInterval ) || boost::numeric::width( splineInterval ) <= 0 )
		{
			throw InvalidArgumentException( "Spline interval is empty or has zero width." );
		}

		Box2i dataWindow( V2i( 0 ), m_parent->resolutionParameter()->getTypedValue() - V2i( 1 ) );
		ImagePrimitivePtr result = new ImagePrimitive( dataWindow, dataWindow );
		
		std::vector<float *> channels;
		if( YTraits::dimensions()==3 )
		{
			FloatVectorDataPtr channel = result->createChannel<float>( "R" );
			channels.push_back( &(channel->writable()[0]) );
			channel = result->createChannel<float>( "G" );
			channels.push_back( &(channel->writable()[0]) );
			channel = result->createChannel<float>( "B" );
			channels.push_back( &(channel->writable()[0]) );
		}
		else if( YTraits::dimensions()==1 )
		{
			FloatVectorDataPtr channel = result->createChannel<float>( "Y" );
			channels.push_back( &(channel->writable()[0]) );		
		}
	
		XType splineWidth = boost::numeric::width( splineInterval );
		for( int y=dataWindow.min.y; y<=dataWindow.max.y; y++ )
		{
			XType splineX = splineInterval.lower() + splineWidth * (XType)(y-dataWindow.min.y) / (XType)(dataWindow.size().y);
			YType splineResult = spline( splineX );
			for( unsigned c=0; c<channels.size(); c++ )
			{
				typename YTraits::BaseType channelValue = YTraits::get( splineResult, c );
				for( int x=dataWindow.min.x; x<=dataWindow.max.x; x++ )
				{
					*(channels[c])++ = channelValue;
				}
			}
		}
	
		return result;
	}
	
	SplineToImage *m_parent;
};

ObjectPtr SplineToImage::doOperation( ConstCompoundObjectPtr operands )
{

	CreateImage f( this );
	return despatchTypedData<CreateImage, TypeTraits::IsSplineTypedData>( boost::static_pointer_cast<Data>( m_splineParameter->getValue() ), f );
	
}
