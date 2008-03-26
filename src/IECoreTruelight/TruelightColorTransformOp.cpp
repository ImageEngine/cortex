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

#include "IECoreTruelight/TruelightColorTransformOp.h"
#include "IECoreTruelight/IECoreTruelight.h"

#include "IECore/CompoundParameter.h"

#include "truelight.h"

using namespace IECoreTruelight;
using namespace IECore;

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
	
	IntParameter::PresetsMap inputSpacePresets;
	inputSpacePresets["log"] = 0;
	inputSpacePresets["linear"] = 1;
	inputSpacePresets["video"] = 2;
	m_inputSpaceParameter = new IntParameter(
		"inputSpace",
		"The colorspace of the input to the transform.",
		1,
		0,
		2,
		inputSpacePresets,
		true
	);
	
	parameters()->addParameter( m_profileParameter );
	parameters()->addParameter( m_displayParameter );
	parameters()->addParameter( m_inputSpaceParameter );
	
	init( "" );
	
	m_instance = TruelightCreateInstance();
	if( !m_instance )
	{
		throw( Exception( TruelightGetErrorString() ) );
	}
	
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

void TruelightColorTransformOp::begin( IECore::ConstCompoundObjectPtr operands )
{
	setInstanceFromParameters();
	if( !TruelightInstanceSetUp( m_instance ) )
	{
		throw Exception( TruelightGetErrorString() );		
	}
}

void TruelightColorTransformOp::setInstanceFromParameters() const
{
	if( !TruelightInstanceSetProfile( m_instance, m_profileParameter->getTypedValue().c_str() ) )
	{
		throw Exception( TruelightGetErrorString() );
	}
	if( !TruelightInstanceSetDisplay( m_instance, m_displayParameter->getTypedValue().c_str() ) )
	{
		throw Exception( TruelightGetErrorString() );
	}
	if( !TruelightInstanceSetCubeInput( m_instance, m_inputSpaceParameter->getNumericValue() ) )
	{
		throw Exception( TruelightGetErrorString() );
	}
}

std::string TruelightColorTransformOp::commands() const
{
	setInstanceFromParameters();
	return TruelightInstanceGetCommands( m_instance, "\n" );
}

void TruelightColorTransformOp::transform( Imath::Color3f &color ) const
{
	TruelightInstanceTransformF( m_instance, color.getValue() );
}
