//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "IECore/AlexaLogcToLinearOp.h"
#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/AlexaLogcToLinearDataConversion.h"

using namespace IECore;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( AlexaLogcToLinearOp );

ColorSpaceTransformOp::ColorSpaceDescription<AlexaLogcToLinearOp> AlexaLogcToLinearOp::g_colorSpaceDescription( "alexaLogC", "linear" );

AlexaLogcToLinearOp::AlexaLogcToLinearOp()
	:	ChannelOp( "Applies Alexa Log C to linear conversion on ImagePrimitive channels." )
{
}

AlexaLogcToLinearOp::~AlexaLogcToLinearOp()
{
}

struct AlexaLogcToLinearOp::Converter
{
	typedef void ReturnType;

	template<typename T>
	ReturnType operator()( T * data )
	{
		typedef typename T::ValueType Container;
		typedef typename Container::value_type V;
		typedef typename Container::iterator It;
		AlexaLogcToLinearDataConversion< V, V > converter;
		It end = data->writable().end();
		for ( It it = data->writable().begin(); it != end; it++ )
		{
			*it = converter( *it );
		}
	}
};

void AlexaLogcToLinearOp::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	AlexaLogcToLinearOp::Converter converter;
	for ( ChannelVector::iterator it = channels.begin(); it != channels.end(); it++ )
	{
		despatchTypedData<AlexaLogcToLinearOp::Converter, TypeTraits::IsFloatVectorTypedData>( it->get(), converter );
	}
}
