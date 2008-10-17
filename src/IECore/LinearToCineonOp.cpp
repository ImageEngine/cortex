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

#include "IECore/LinearToCineonOp.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/LinearToCineonDataConversion.h"
#include "IECore/CineonToLinearOp.h"

using namespace IECore;
using namespace std;
using namespace boost;

LinearToCineonOp::LinearToCineonOp()
	:	ChannelOp( "LinearToCineon", 
				   "Applies linear to Cineon conversion on ImagePrimitive channels."
		)
{
	CompoundParameterPtr cineonParameters = CineonToLinearOp::createCineonSettings();
	m_filmGamma = cineonParameters->parameter< FloatParameter >( "filmGamma" );
	m_refWhiteVal = cineonParameters->parameter< IntParameter >( "refWhiteVal" );
	m_refBlackVal = cineonParameters->parameter< IntParameter >( "refBlackVal" );
	parameters()->addParameter( cineonParameters );
}

LinearToCineonOp::~LinearToCineonOp()
{
}

FloatParameterPtr LinearToCineonOp::filmGammaParameter()
{
	return m_filmGamma;
}

ConstFloatParameterPtr LinearToCineonOp::filmGammaParameter() const
{
	return m_filmGamma;
}

IntParameterPtr LinearToCineonOp::refWhiteValParameter()
{
	return m_refWhiteVal;
}

ConstIntParameterPtr LinearToCineonOp::refWhiteValParameter() const
{
	return m_refWhiteVal;
}

IntParameterPtr LinearToCineonOp::refBlackValParameter()
{
	return m_refBlackVal;
}

ConstIntParameterPtr LinearToCineonOp::refBlackValParameter() const
{
	return m_refBlackVal;
}

struct LinearToCineonOp::Converter
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
			LinearToCineonDataConversion< V, unsigned int > converter( m_filmGamma, m_refWhiteVal, m_refBlackVal );
			It end = data->writable().end();
			for ( It it = data->writable().begin(); it != end; it++ )
			{
				*it = static_cast<V>(converter( *it ) / 1023.);
			}
		}

	private:

		float m_filmGamma;
		int m_refWhiteVal;
		int m_refBlackVal;

};

void LinearToCineonOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	LinearToCineonOp::Converter converter( filmGammaParameter()->getNumericValue(), 
											refWhiteValParameter()->getNumericValue(), 
											refBlackValParameter()->getNumericValue() );
	for ( ChannelVector::iterator it = channels.begin(); it != channels.end(); it++ )
	{
		despatchTypedData<LinearToCineonOp::Converter, TypeTraits::IsFloatVectorTypedData>( *it, converter );
	}
}
