//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include "DDImage/Knobs.h"

#include "IECore/CompoundObject.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/CamelCase.h"

#include "IECoreNuke/ParameterHandler.h"
#include "IECoreNuke/PresetsOnlyParameterHandler.h"

using namespace DD::Image;
using namespace IECore;
using namespace IECoreNuke;

ParameterHandler::ParameterHandler()
{
}

int ParameterHandler::minimumInputs( const IECore::Parameter *parameter )
{
	return 0;
}

int ParameterHandler::maximumInputs( const IECore::Parameter *parameter )
{
	return 0;
}

bool ParameterHandler::testInput( const IECore::Parameter *parameter, int input, const DD::Image::Op *op )
{
	return false;
}

void ParameterHandler::setParameterValue( IECore::Parameter *parameter, InputIterator first, InputIterator last )
{
}

void ParameterHandler::knobs( const IECore::Parameter *parameter, const char *knobName, DD::Image::Knob_Callback f )
{
}

void ParameterHandler::setParameterValue( IECore::Parameter *parameter, ValueSource valueSource )
{
}

void ParameterHandler::setKnobValue( const IECore::Parameter *parameter )
{
}

void ParameterHandler::setState( IECore::Parameter *parameter, const IECore::Object *state )
{
	assert( 0 ); // shouldn't get called because we don't return a state in getState()
}

IECore::ObjectPtr ParameterHandler::getState( const IECore::Parameter *parameter )
{
	return 0;
}

ParameterHandlerPtr ParameterHandler::create( const IECore::Parameter *parameter )
{
	if( parameter->presetsOnly() )
	{
		return new PresetsOnlyParameterHandler();
	}

	const CreatorFnMap &creators = creatorFns();
	TypeId typeId = parameter->typeId();
	while( typeId!=InvalidTypeId )
	{
		CreatorFnMap::const_iterator it = creators.find( typeId );
		if( it!=creators.end() )
		{
			return it->second();
		}
		typeId = RunTimeTyped::baseTypeId( typeId );
	}
	return 0;
}

ParameterHandler::CreatorFnMap &ParameterHandler::creatorFns()
{
	static CreatorFnMap creators;
	return creators;
}

std::string ParameterHandler::knobLabel( const IECore::Parameter *parameter ) const
{
	return IECore::CamelCase::toSpaced( parameter->name().c_str() );
}

void ParameterHandler::setKnobProperties( const IECore::Parameter *parameter, DD::Image::Knob_Callback f, DD::Image::Knob *knob ) const
{
	const CompoundObject *userData = parameter->userData();
	const CompoundObject *ui = userData->member<CompoundObject>( "UI" );

	int flags = 0;
	if( ui )
	{
		const BoolData *visible = ui->member<BoolData>( "visible" );
		if( visible && !visible->readable() )
		{
			flags |= Knob::INVISIBLE;
		}
	}

	SetFlags( f, flags );
	Tooltip( f, parameter->description() );

	if( f.makeKnobs() )
	{
		const CompoundObject *nuke = userData->member<CompoundObject>( "nuke" );
		if( nuke )
		{
			const StringData *defaultExpression = nuke->member<StringData>( "defaultExpression" );
			if( defaultExpression )
			{
				if( knob->from_script( defaultExpression->readable().c_str() ) )
				{
					knob->changed();
				}
			}
		}
	}
}
