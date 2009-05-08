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

#include "IECoreTruelight/TruelightColorTransformOp.h"
#include "IECoreTruelight/IECoreTruelight.h"

#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

#include "truelight.h"

using namespace IECoreTruelight;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( IECoreTruelight::TruelightColorTransformOp );

TruelightColorTransformOp::TruelightColorTransformOp()
	:	ColorTransformOp( "TruelightColorTransformOp", "Applies truelight transforms." ), m_instance( 0 )
{

	m_profileParameter = new StringParameter(
		"profile",
		"The name of a truelight profile to define the transformation.",
		"Kodak"
	);
	m_displayParameter = new StringParameter(
		"display",
		"The name of a display calibration to define the transformation.",
		"monitor"
	);

	IntParameter::PresetsContainer inputSpacePresets;
	inputSpacePresets.push_back( IntParameter::Preset( "log", TL_INPUT_LOG ) );
	inputSpacePresets.push_back( IntParameter::Preset( "linear", TL_INPUT_LIN ) );
	inputSpacePresets.push_back( IntParameter::Preset( "video", TL_INPUT_VID ) );
	m_inputSpaceParameter = new IntParameter(
		"inputSpace",
		"The colorspace of the input to the transform.",
		TL_INPUT_LIN,
		inputSpacePresets
	);

	m_rawTruelightOutputParameter = new BoolParameter(
		"rawTruelightOutput",
		"If disabled, applies an sRGB->Linear conversion after the Truelight LUT.",
		true
	);

	parameters()->addParameter( m_profileParameter );
	parameters()->addParameter( m_displayParameter );
	parameters()->addParameter( m_inputSpaceParameter );
	parameters()->addParameter( m_rawTruelightOutputParameter );

	init( "" );

	m_instance = TruelightCreateInstance();
	if( !m_instance )
	{
		throw( Exception( TruelightGetErrorString() ) );
	}
	assert( m_instance );
}

TruelightColorTransformOp::~TruelightColorTransformOp()
{
	TruelightDestroyInstance( m_instance );
}

IECore::StringParameterPtr TruelightColorTransformOp::profileParameter()
{
	return m_profileParameter;
}

IECore::ConstStringParameterPtr TruelightColorTransformOp::profileParameter() const
{
	return m_profileParameter;
}

IECore::StringParameterPtr TruelightColorTransformOp::displayParameter()
{
	return m_displayParameter;
}

IECore::ConstStringParameterPtr TruelightColorTransformOp::displayParameter() const
{
	return m_displayParameter;
}

IECore::IntParameterPtr TruelightColorTransformOp::inputSpaceParameter()
{
	return m_inputSpaceParameter;
}

IECore::ConstIntParameterPtr TruelightColorTransformOp::inputSpaceParameter() const
{
	return m_inputSpaceParameter;
}

IECore::BoolParameterPtr TruelightColorTransformOp::rawTruelightOutputParameter()
{
	return m_rawTruelightOutputParameter;
}

IECore::ConstBoolParameterPtr TruelightColorTransformOp::rawTruelightOutputParameter() const
{
	return m_rawTruelightOutputParameter;
}

void TruelightColorTransformOp::begin( IECore::ConstCompoundObjectPtr operands )
{
	assert( operands );
	assert( m_instance );

	setInstanceFromParameters();
	if( !TruelightInstanceSetUp( m_instance ) )
	{
		throw Exception( TruelightGetErrorString() );
	}
}

void TruelightColorTransformOp::setInstanceFromParameters() const
{
	assert( m_instance );
	if( !TruelightInstanceSetProfile( m_instance, m_profileParameter->getTypedValue().c_str() ) )
	{
		throw Exception( TruelightGetErrorString() );
	}
	maybeWarn();
	if( !TruelightInstanceSetDisplay( m_instance, m_displayParameter->getTypedValue().c_str() ) )
	{
		throw Exception( TruelightGetErrorString() );
	}
	maybeWarn();
	if( !TruelightInstanceSetCubeInput( m_instance, m_inputSpaceParameter->getNumericValue() ) )
	{
		throw Exception( TruelightGetErrorString() );
	}
	maybeWarn();
}

std::string TruelightColorTransformOp::commands() const
{
	setInstanceFromParameters();
	return TruelightInstanceGetCommands( m_instance, "\n" );
}

void TruelightColorTransformOp::transform( Imath::Color3f &color ) const
{
	assert( m_instance );
	TruelightInstanceTransformF( m_instance, color.getValue() );
	maybeWarn();

	if ( !m_rawTruelightOutputParameter->getTypedValue() )
	{
		/// \todo This would be easier if we had some sort of DataConversionToColorTransformAdapter template
		color.x = m_srgbToLinearConversion( color.x );
		color.y = m_srgbToLinearConversion( color.y );
		color.z = m_srgbToLinearConversion( color.z );
	}
}

void TruelightColorTransformOp::maybeWarn() const
{
	assert( m_instance );
	const char *warning = TruelightInstanceGetWarning( m_instance );
	if ( warning )
	{
		msg( Msg::Warning, "TruelightColorTransformOp", warning );
	}
}
