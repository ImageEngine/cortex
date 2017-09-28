//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/NumericTraits.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/ObjectParameterHandler.h"
#include "IECoreMaya/ObjectData.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECore/ObjectParameter.h"
#include "IECore/MeshPrimitive.h"

#include "maya/MFnGenericAttribute.h"
#include "maya/MPlug.h"
#include "maya/MFnPluginData.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< ObjectParameterHandler > registrar( IECore::ObjectParameter::staticTypeId() );

MStatus ObjectParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnGenericAttribute fnGAttr( attribute );
	if( !fnGAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	fnGAttr.addAccept( ObjectData::id );
	// maya has an odd behaviour whereby a generic attribute with only one accepted datatype will
	// transform itself into a typed attribute after file save and load. here we add an accept
	// for a second dummy datatype to ensure that the attribute will still be a generic attribute
	// when saved and loaded.
	fnGAttr.addAccept( DummyDataId );

	for (IECore::ObjectParameter::TypeIdSet::const_iterator it = p->validTypes().begin(); it != p->validTypes().end(); ++it)
	{
		ConstParameterHandlerPtr h = ParameterHandler::create( *it );
		if (h)
		{
			if ( !h->doUpdate( parameter, plug ) )
			{
				return MS::kFailure;
			}
		}
	}

	return finishUpdating( parameter, plug );
}

MPlug ObjectParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	IECore::ConstObjectParameterPtr p = IECore::runTimeCast<const IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MPlug();
	}

	MFnGenericAttribute fnGAttr;
	MObject attribute = fnGAttr.create( plugName, plugName );

	MPlug result = finishCreating( parameter, attribute, node );
	doUpdate( parameter, result );

	return result;
}

MStatus ObjectParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
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
		ConstParameterHandlerPtr h = ParameterHandler::create( *it );
		if (h)
		{
			if ( h->doSetValue( parameter, plug) )
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

	data->setObject( p->getValue()->copy() );
	return plug.setValue( plugData );
}

MStatus ObjectParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	IECore::ObjectParameterPtr p = IECore::runTimeCast<IECore::ObjectParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	/// Keep trying all the available handlers until we find one that works
	for (IECore::ObjectParameter::TypeIdSet::const_iterator it = p->validTypes().begin(); it != p->validTypes().end(); ++it)
	{
		ConstParameterHandlerPtr h = ParameterHandler::create( *it );
		if (h)
		{
			if ( h->doSetValue( plug, parameter ) )
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
		// We might be here as the attribute isn't storable,
		// in that case we set the parameter to its default value.
		// If it is storable, then something has gone awry.
		MFnAttribute fnA( plug.attribute() );
		bool isStorable = fnA.isStorable( &s );
		if( s && !isStorable )
		{
			parameter->setValue( parameter->defaultValue()->copy() );
			return MS::kSuccess;
		}
		else
		{
			return MS::kFailure;
		}
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

	parameter->setValue( data->getObject() );

	return MS::kSuccess;
}
