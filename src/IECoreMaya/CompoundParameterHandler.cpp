//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/Parameter.h"
#include "IECoreMaya/CompoundParameterHandler.h"

#include "IECore/CompoundParameter.h"

#include "maya/MFnMessageAttribute.h"

using namespace IECoreMaya;

static ParameterHandler::Description<CompoundParameterHandler> registrar( IECore::CompoundParameter::staticTypeId() );

MStatus CompoundParameterHandler::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	IECore::ConstCompoundParameterPtr p = IECore::runTimeCast<const IECore::CompoundParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MFnMessageAttribute fnMAttr( attribute );
	if( !fnMAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}
		
	return MS::kSuccess;
}

MObject CompoundParameterHandler::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	IECore::ConstCompoundParameterPtr p = IECore::runTimeCast<const IECore::CompoundParameter>( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}
	
	MFnMessageAttribute fnMAttr;
	MObject result = fnMAttr.create( attributeName, attributeName );
	return result;
}
		
MStatus CompoundParameterHandler::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	return MS::kSuccess;
}

MStatus CompoundParameterHandler::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	return MS::kSuccess;	
}
