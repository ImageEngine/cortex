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

#include "IECore/Grade.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"

using namespace IECore;
using namespace Imath;

Grade::Grade()
	:	ColorTransformOp( "Grade", 
				"The grade implements the same operation as Nuke's grade node over the colors of a Primitive object.\n"
				"The computation performed is:\n"
				"A = multiply * (gain - lift) / (whitePoint - blackPoint)\n"
				"B = offset + lift - A * blackPoint\n"
				"output = pow( A * input + B, 1/gamma )"
		), m_A(0), m_B(0), m_invGamma(1)
{

	m_blackPointParameter = new Color3fParameter(
		"blackPoint", 
		"This color is considered the black.",
		Imath::Color3f(0,0,0)
	);
	
	m_whitePointParameter = new Color3fParameter(
		"whitePoint", 
		"This color is considered the white.",
		Imath::Color3f(1,1,1)
	);

	m_liftParameter = new Color3fParameter(
		"lift", 
		"This color is turned into black.",
		Imath::Color3f(0,0,0)
	);

	m_gainParameter = new Color3fParameter(
		"gain", 
		"This color is turned into white.",
		Imath::Color3f(1,1,1)
	);

	m_multiplyParameter = new Color3fParameter(
		"multiply", 
		"Constant to multiply result by.",
		Imath::Color3f(1,1,1)
	);

	m_offsetParameter = new Color3fParameter(
		"offset", 
		"Constant to add to result.",
		Imath::Color3f(0,0,0)
	);

	m_gammaParameter = new Color3fParameter(
		"gamma", 
		"Gamma correction applied to final result",
		Imath::Color3f(1,1,1)
	);

	m_blackClampParameter = new BoolParameter(
		"blackClamp",
		"Output less then zero is clamped to zero.",
		true
	);

	m_whiteClampParameter = new BoolParameter(
		"whiteClamp",
		"Output greater then one is clamped to one.",
		false
	);
	
	parameters()->addParameter( m_blackPointParameter );
	parameters()->addParameter( m_whitePointParameter );
	parameters()->addParameter( m_liftParameter );
	parameters()->addParameter( m_gainParameter );
	parameters()->addParameter( m_multiplyParameter );
	parameters()->addParameter( m_offsetParameter );
	parameters()->addParameter( m_gammaParameter );
	parameters()->addParameter( m_blackClampParameter );
	parameters()->addParameter( m_whiteClampParameter );
	
}

Grade::~Grade()
{
}

Color3fParameterPtr Grade::blackPointParameter()
{
	return m_blackPointParameter;
}

ConstColor3fParameterPtr Grade::blackPointParameter() const
{
	return m_blackPointParameter;
}

Color3fParameterPtr Grade::whitePointParameter()
{
	return m_whitePointParameter;
}

ConstColor3fParameterPtr Grade::whitePointParameter() const
{
	return m_whitePointParameter;
}

Color3fParameterPtr Grade::liftParameter()
{
	return m_liftParameter;
}

ConstColor3fParameterPtr Grade::liftParameter() const
{
	return m_liftParameter;
}

Color3fParameterPtr Grade::gainParameter()
{
	return m_gainParameter;
}

ConstColor3fParameterPtr Grade::gainParameter() const
{
	return m_gainParameter;
}

Color3fParameterPtr Grade::multiplyParameter()
{
	return m_multiplyParameter;
}

ConstColor3fParameterPtr Grade::multiplyParameter() const
{
	return m_gainParameter;
}

Color3fParameterPtr Grade::offsetParameter()
{
	return m_offsetParameter;
}

ConstColor3fParameterPtr Grade::offsetParameter() const
{
	return m_offsetParameter;
}

Color3fParameterPtr Grade::gammaParameter()
{
	return m_gammaParameter;
}

ConstColor3fParameterPtr Grade::gammaParameter() const
{
	return m_gammaParameter;
}

BoolParameterPtr Grade::blackClampParameter()
{
	return m_blackClampParameter;
}

ConstBoolParameterPtr Grade::blackClampParameter() const
{
	return m_blackClampParameter;
}

BoolParameterPtr Grade::whiteClampParameter()
{
	return m_whiteClampParameter;
}

ConstBoolParameterPtr Grade::whiteClampParameter() const
{
	return m_whiteClampParameter;
}

void Grade::begin( ConstCompoundObjectPtr operands )
{
	m_invGamma = m_gammaParameter->getTypedValue();
	if ( m_invGamma.x == 0.0 || m_invGamma.y == 0.0 || m_invGamma.z == 0 )
	{
		throw Exception( "Gamma values cannot be zero!" );
	}
	m_invGamma = Imath::V3d(1.0, 1.0, 1.0) / m_invGamma;
	Imath::V3d multiply = m_multiplyParameter->getTypedValue();
	Imath::V3d gain = m_gainParameter->getTypedValue();
	Imath::V3d lift = m_liftParameter->getTypedValue();
	Imath::V3d whitePoint = m_whitePointParameter->getTypedValue();
	Imath::V3d blackPoint = m_blackPointParameter->getTypedValue();
	Imath::V3d offset = m_offsetParameter->getTypedValue();

	m_A = multiply * ( gain - lift ) / ( whitePoint - blackPoint );
	m_B = offset + lift - m_A * blackPoint;
}

void Grade::transform( Imath::Color3f &color ) const
{
	Imath::V3d c = m_A * Imath::V3d(color) + m_B;
	color.x = ( c.x >= 0.0 ? (float)pow( c.x, m_invGamma.x ) : c.x );
	color.y = ( c.y >= 0.0 ? (float)pow( c.y, m_invGamma.y ) : c.y );
	color.z = ( c.z >= 0.0 ? (float)pow( c.z, m_invGamma.z ) : c.z );

	if ( m_blackClampParameter->getTypedValue() )
	{
		if ( color.x < 0.0 ) color.x = 0.0;
		if ( color.y < 0.0 ) color.y = 0.0;
		if ( color.z < 0.0 ) color.z = 0.0;
	}

	if ( m_whiteClampParameter->getTypedValue() )
	{
		if ( color.x > 1.0 ) color.x = 1.0;
		if ( color.y > 1.0 ) color.y = 1.0;
		if ( color.z > 1.0 ) color.z = 1.0;
	}
}
