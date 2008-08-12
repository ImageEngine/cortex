//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
#include "IECoreMaya/NumericTraits.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/ObjectParameterHandler.h"
#include "IECoreMaya/ObjectData.h"

#include "IECore/ObjectParameter.h"
#include "IECore/MeshPrimitive.h"

#include "maya/MFnGenericAttribute.h"
#include "maya/MPlug.h"
#include "maya/MFnPluginData.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< ObjectParameterHandler > registrar( IECore::ObjectParameter::staticTypeId() );

MStatus ObjectParameterHandler::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MFnGenericAttribute fnGAttr;
	if( !fnGAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}
	
	fnGAttr.addAccept( ObjectData::id );		
	
	for (IECore::ObjectParameter::TypeIdSet::const_iterator it = p->validTypes().begin(); it != p->validTypes().end(); ++it)
	{
		ConstParameterHandlerPtr h = ParameterHandler::get( *it );
		if (h)
		{
			if ( !h->update( parameter, attribute) )
			{
				return MS::kFailure;
			}
		}
	}
		
	return MS::kSuccess;
}

MObject ObjectParameterHandler::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}	
	
	MFnGenericAttribute fnGAttr;
	
	MObject result = fnGAttr.create( attributeName, attributeName );
			
	update( parameter, result );
	return result;
}
	
MStatus ObjectParameterHandler::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	// Keep trying all the available handlers until we find one that works.
	/// \todo Investigate whether we can do a parameter->getValue() here and just get the handler which represents it
	for (IECore::ObjectParameter::TypeIdSet::const_iterator it = p->validTypes().begin(); it != p->validTypes().end(); ++it)
	{
		ConstParameterHandlerPtr h = ParameterHandler::get( *it );
		if (h)
		{	
			if ( h->setValue( parameter, plug) )
			{
				return MS::kSuccess;
			}
		}
	}

	MStatus s;	
	
	MFnPluginData fnData;
	MObject plugData = fnData.create( ObjectData::id );
	assert( plugData != MObject::kNullObj );
	
	s = fnData.setObject( plugData );
	assert(s);
	
	ObjectData* data = dynamic_cast<ObjectData *>( fnData.data(&s) );
	assert(s);
	assert(data);
	
	data->setObject( p->getValue() );
	return plug.setValue( plugData );
}

MStatus ObjectParameterHandler::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	IECore::ObjectParameterPtr p = IECore::runTimeCast<IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	/// Keep trying all the available handlers until we find one that works
	for (IECore::ObjectParameter::TypeIdSet::const_iterator it = p->validTypes().begin(); it != p->validTypes().end(); ++it)
	{
		ConstParameterHandlerPtr h = ParameterHandler::get( *it );
		if (h)
		{
			if ( h->setValue( plug, parameter ) )
			{
				return MS::kSuccess;
			}
		}	
	}
	
	MStatus s;
	MObject plugData;
	s = plug.getValue( plugData );
	if (!s)
	{
		return MS::kFailure;
	}
	
	MFnPluginData fnData( plugData, &s );
	if (!s)
	{
		return MS::kFailure;
	}
	
	ObjectData *data = dynamic_cast<ObjectData *>( fnData.data( &s ) );
	if (!data || !s)
	{
		return MS::kFailure;
		
	}

	/// \todo Should probably be calling setValidatedValue here, and handle any thrown exceptions?	
	parameter->setValue( data->getObject() );
		
	return MS::kSuccess;			
}
