//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MarschnerParameter.h"
#include "IECore/Math.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( MarschnerParameter );

MarschnerParameter::MarschnerParameter( const std::string &name, const std::string &description, bool absorptionAsColor, ConstCompoundObjectPtr userData ) 
	: CompoundParameter( name, description, userData ), m_absorptionAsColor( absorptionAsColor )
{
	
	m_refraction = new FloatParameter(
		"refraction",
		"",
		1.55f, 1.0f, 3.0f
	);
	
	/// \todo color -> colour, otherwise it's a reserved word in rsl. Not to be done before v6.0.0
	m_absorption = new Color3fParameter(
		absorptionAsColor ? "color" : "absorption",
		"",
		absorptionAsColor ? Imath::Color3f( 1.0, 1.0, 1.0 ) : Imath::Color3f( 0.0, 0.0, 0.0 )
	);
	
	m_eccentricity = new FloatParameter(
		"eccentricity",
		"",
		1.0f, 0.5f, 1.0f
	);
		
	m_shiftR = new FloatParameter(
		"shiftR",
		"",
		-8.0f, -20.0f, 5.0f
	);
		
	m_shiftTT = new FloatParameter(
		"shiftTT",
		"",
		4.0f, -5.0f, 20.0f
	);
	
	m_shiftTRT = new FloatParameter(
		"shiftTRT",
		"",
		12.0f, -5.0f, 20.0f
	);
	
	m_widthR = new FloatParameter(
		"widthR",
		"",
		10.0f, 0.0f, 45.0f
	);
	
	m_widthTT = new FloatParameter(
		"widthTT",
		"",
		5.0f, 0.0f, 45.0f
	);
	
	m_widthTRT = new FloatParameter(
		"widthTRT",
		"",
		20.0f, 0.0f, 45.0f
	);
	
	m_glint = new FloatParameter(
		"glint",
		"",
		1.0f,
		0.0f, 10.0f
	);
	
	m_causticWidth = new FloatParameter(
		"causticWidth",
		"",
		20.0f, 0.0f, 45.0f
	);
	
	m_causticFade = new FloatParameter(
		"causticFade",
		"",
		0.2f, 0.01f, 0.5f
	);
	
	m_causticLimit = new FloatParameter(
		"causticLimit",
		"",
		0.5, 0.0, 10.0f
	);
	
	addParameter( m_absorption );
	addParameter( m_refraction );
	addParameter( m_eccentricity );
	addParameter( m_shiftR );
	addParameter( m_widthR );
	addParameter( m_shiftTT );
	addParameter( m_widthTT );
	addParameter( m_shiftTRT );
	addParameter( m_widthTRT );
	addParameter( m_glint );
	addParameter( m_causticWidth );
	addParameter( m_causticFade );
	addParameter( m_causticLimit );
}

MarschnerParameter::~MarschnerParameter()
{
}

MarschnerBCSDFC3f MarschnerParameter::createBCSDF()
{
	Imath::Color3f absorption = m_absorption->getTypedValue();
	if( m_absorptionAsColor )
	{
		absorption[0] = -log(absorption[0]) * 0.25f;
		absorption[1] = -log(absorption[1]) * 0.25f;
		absorption[2] = -log(absorption[2]) * 0.25f;
	}

	return MarschnerBCSDFC3f(
		m_refraction->getNumericValue(),
		absorption,
		m_eccentricity->getNumericValue(),
		(m_shiftR->getNumericValue()/180.0f) * M_PI,
		(m_shiftTT->getNumericValue()/180.0f) * M_PI,
		(m_shiftTRT->getNumericValue()/180.0f) * M_PI,
		(m_widthR->getNumericValue()/180.0f) * M_PI,
		(m_widthTT->getNumericValue()/180.0f) * M_PI,
		(m_widthTRT->getNumericValue()/180.0f) * M_PI,
		m_glint->getNumericValue(),
		(m_causticWidth->getNumericValue()/180.0f) * M_PI,
		m_causticFade->getNumericValue(),
		m_causticLimit->getNumericValue()
	);
}

