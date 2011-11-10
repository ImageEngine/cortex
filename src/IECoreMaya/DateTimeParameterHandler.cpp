//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/DateTimeParameterHandler.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECore/DateTimeParameter.h"

#include "maya/MFnTypedAttribute.h"
#include "maya/MPlug.h"

#include "boost/date_time/posix_time/posix_time.hpp"

using namespace IECoreMaya;

static ParameterHandler::Description< DateTimeParameterHandler > registrar( IECore::DateTimeParameter::staticTypeId() );

MStatus DateTimeParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstDateTimeParameterPtr p = IECore::runTimeCast<const IECore::DateTimeParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();

	// we'd like to be setting the default value here, but as maya doesn't save the default value
	// for dynamic string attributes in scene files, it'll be lost when the scene is reloaded. it's
	// best therefore that we don't set the default at all, so that the default is "", which is what
	// it'll be when we reload the scene - this ensures that any values set in the attribute later
	// will be saved correctly (if we set the default to "X" and the value was "X", maya won't save the
	// default or the value at all, and we end up with a value of "" on scene reload).

	MFnTypedAttribute fnTAttr( attribute );
	if( !fnTAttr.hasObj( attribute ) || fnTAttr.attrType() != MFnData::kString )
	{
		return MS::kFailure;
	}

	// Should this fail if getPlugValue fails?
	MString v = "";
	if( plug.getValue( v ) )
	{
		try
		{
			boost::posix_time::from_iso_string( v.asChar() );
		}
		catch ( boost::bad_lexical_cast )
		{
			return MS::kFailure;
		}
	}

	return finishUpdating( parameter, plug );
}

MPlug DateTimeParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	IECore::ConstDateTimeParameterPtr p = IECore::runTimeCast<const IECore::DateTimeParameter>( parameter );
	if( !p )
	{
		return MPlug();
	}

	MObject attribute;
	MFnTypedAttribute fnTAttr;
	attribute = fnTAttr.create( plugName, plugName, MFnData::kString /* see comments in stringUpdate for why we don't specify a default here */ );

	MPlug result = finishCreating( parameter, attribute, node );
	if( finishUpdating( parameter, result ) )
	{
		return result;
	}

	return MPlug();
}

MStatus DateTimeParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstDateTimeParameterPtr p = IECore::runTimeCast<const IECore::DateTimeParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	return plug.setValue( boost::posix_time::to_iso_string( p->getTypedValue() ).c_str() );
}

MStatus DateTimeParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	IECore::DateTimeParameterPtr p = IECore::runTimeCast<IECore::DateTimeParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MString v = "";
	MStatus result =  plug.getValue( v );
	if( result )
	{
		p->setTypedValue( boost::posix_time::from_iso_string( v.asChar() ) );
	}
	return result;
}
