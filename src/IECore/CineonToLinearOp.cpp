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

#include "IECore/CineonToLinearOp.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CineonToLinearDataConversion.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace std;
using namespace boost;

CineonToLinearOp::CineonToLinearOp()
	:	ChannelOp( "CineonToLinearOp", 
				   "Applies Cineon to linear conversion on ImagePrimitive channels."
		)
{
	m_filmGamma = new FloatParameter(
		"filmGamma",
		"Gamma value",
		0.6f
	);
	m_refWhiteVal = new IntParameter(
		"refWhiteVal",
		"White reference value",
		685
	);
	m_refBlackVal = new IntParameter(
		"refBlackVal",
		"Black reference value",
		95
	);
	CompoundParameterPtr cineonParameters = new CompoundParameter( "cineonSettings", "Define parameters of the Cineon colorspace" );

	cineonParameters->addParameter( m_filmGamma );
	cineonParameters->addParameter( m_refWhiteVal );
	cineonParameters->addParameter( m_refBlackVal );
	parameters()->addParameter( cineonParameters );
}

CineonToLinearOp::~CineonToLinearOp()
{
}

FloatParameterPtr CineonToLinearOp::filmGammaParameter()
{
	return m_filmGamma;
}

ConstFloatParameterPtr CineonToLinearOp::filmGammaParameter() const
{
	return m_filmGamma;
}

IntParameterPtr CineonToLinearOp::refWhiteValParameter()
{
	return m_refWhiteVal;
}

ConstIntParameterPtr CineonToLinearOp::refWhiteValParameter() const
{
	return m_refWhiteVal;
}

IntParameterPtr CineonToLinearOp::refBlackValParameter()
{
	return m_refBlackVal;
}

ConstIntParameterPtr CineonToLinearOp::refBlackValParameter() const
{
	return m_refBlackVal;
}

struct CineonToLinearOp::Converter
{

		typedef void ReturnType;

		Converter( float filmGamma, int refWhiteVal, int refBlackVal ) :
			m_filmGamma(filmGamma), m_refWhiteVal(refWhiteVal), m_refBlackVal(refBlackVal)
		{
		}
	
		template<typename T>
		ReturnType operator()( typename T::Ptr data )
		{
			typedef typename T::ValueType Container;
			typedef typename Container::value_type V;
			typedef typename Container::iterator It;
			CineonToLinearDataConversion< unsigned short, V > converter( m_filmGamma, m_refWhiteVal, m_refBlackVal );
			It end = data->writable().end();
			for ( It it = data->writable().begin(); it != end; it++ )
			{
				unsigned short v = static_cast<unsigned short>( (*it < 0 ? (V)0. : ( *it > 1. ? (V)1. : *it )) * 1023.);
				*it = converter( v );
			}
		}

	private:

		float m_filmGamma;
		int m_refWhiteVal;
		int m_refBlackVal;
	
};

void CineonToLinearOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	CineonToLinearOp::Converter converter( filmGammaParameter()->getNumericValue(), 
											refWhiteValParameter()->getNumericValue(), 
											refBlackValParameter()->getNumericValue() );

	for ( ChannelVector::iterator it = channels.begin(); it != channels.end(); it++ )
	{
		despatchTypedData<CineonToLinearOp::Converter, TypeTraits::IsFloatVectorTypedData>( *it, converter );
	}
}
