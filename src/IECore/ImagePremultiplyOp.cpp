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

#include "IECore/ImagePremultiplyOp.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypedParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ImagePremultiplyOp );

ImagePremultiplyOp::ImagePremultiplyOp(): ChannelOp( "ImagePremultiplyOp", "Premultiplies image channels by the alpha channel." )
{
	m_alphaChannelNameParameter = new StringParameter(
		"alphaChannelName",
		"The name of the alpha channel to premultiply by",
		"A"
	);

	parameters()->addParameter( m_alphaChannelNameParameter );
}

ImagePremultiplyOp::~ImagePremultiplyOp()
{
}

StringParameterPtr ImagePremultiplyOp::alphaChannelNameParameter()
{
	return m_alphaChannelNameParameter;
}

ConstStringParameterPtr ImagePremultiplyOp::alphaChannelNameParameter() const
{
	return m_alphaChannelNameParameter;
}

struct ImagePremultiplyOp::ToFloatVectorData
{
	typedef FloatVectorDataPtr ReturnType;

	template<typename T>
	ReturnType operator()( typename T::ConstPtr dataPtr )
	{
		return DataConvert< T, FloatVectorData, ScaledDataConversion< typename T::ValueType::value_type, float > >()( dataPtr );
	}
};

struct ImagePremultiplyOp::PremultFn
{
	typedef void ReturnType;

	ConstFloatVectorDataPtr m_alphaChannel;

	PremultFn( ConstFloatVectorDataPtr alphaChannel ) : m_alphaChannel( alphaChannel )
	{
		assert( m_alphaChannel );
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr dataPtr )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::value_type ValueType;
		typedef typename Container::iterator Iterator;

		ScaledDataConversion< ValueType, float > toFloat;
		ScaledDataConversion< float, ValueType > fromFloat;

		FloatVectorData::ValueType::const_iterator alphaIt = m_alphaChannel->readable().begin();

		Container &data = dataPtr->writable();
		for ( typename Container::iterator it = data.begin(); it != data.end(); ++it, ++alphaIt )
		{
			float channelValue = toFloat( *it );

			channelValue *= *alphaIt;

			*it = fromFloat( channelValue );
		}
	}
};

void ImagePremultiplyOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	const std::string &alphaChannelName = m_alphaChannelNameParameter->getTypedValue();

	const StringVectorParameter::ValueType &channelNames = channelNamesParameter()->getTypedValue();

	if ( std::find( channelNames.begin(), channelNames.end(), alphaChannelName ) != channelNames.end() )
	{
		throw InvalidArgumentException( "ImagePremultiplyOp: Specified channel names list contains alpha channel" );
	}

	ImagePrimitivePtr image = assertedStaticCast< ImagePrimitive >( inputParameter()->getValue() );

	const PrimitiveVariableMap::const_iterator it = image->variables.find( alphaChannelName );
	if ( it == image->variables.end() )
	{
		throw InvalidArgumentException( "ImagePremultiplyOp: Cannot find specified alpha channel" );
	}

	FloatVectorDataPtr alphaData = despatchTypedData< ToFloatVectorData, TypeTraits::IsNumericVectorTypedData >( it->second.data );

	ImagePremultiplyOp::PremultFn fn( alphaData );
	for ( ChannelVector::iterator it = channels.begin(); it != channels.end(); it++ )
	{
		despatchTypedData<ImagePremultiplyOp::PremultFn, TypeTraits::IsNumericVectorTypedData>( *it, fn );
	}
}
