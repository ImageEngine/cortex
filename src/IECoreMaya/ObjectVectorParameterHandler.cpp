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

#include "IECoreMaya/ObjectVectorParameterHandler.h"
#include "IECoreMaya/FromMayaPlugConverter.h"
#include "IECoreMaya/ObjectData.h"

#include "IECore/MessageHandler.h"
#include "IECore/TypedObjectParameter.h"
#include "IECore/ObjectVector.h"

#include "maya/MFnTypedAttribute.h"

using namespace std;
using namespace IECore;
using namespace IECoreMaya;

static ParameterHandler::Description< ObjectVectorParameterHandler > registrar( ObjectVectorParameter::staticTypeId() );

MPlug ObjectVectorParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	ConstObjectVectorParameterPtr p = IECore::runTimeCast< const ObjectVectorParameter >( parameter );
	if( !p )
	{
		return MPlug();
	}

	MFnTypedAttribute fnTAttr;
	MObject attribute = fnTAttr.create( plugName, plugName, ObjectData::id );
	fnTAttr.setArray( true );
	fnTAttr.setDisconnectBehavior( MFnAttribute::kDelete );

	MPlug result = finishCreating( parameter, attribute, node );

	if( !finishUpdating( parameter, result ) )
	{
		return MPlug();
	}

	return result;
}

MStatus ObjectVectorParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	ConstObjectVectorParameterPtr p = IECore::runTimeCast< const ObjectVectorParameter >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnTypedAttribute fnTAttr( attribute );
	if( !fnTAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	return finishUpdating( parameter, plug );
}

MStatus ObjectVectorParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	ConstObjectVectorParameterPtr p = IECore::runTimeCast<const ObjectVectorParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	/// \todo Can we implement this?

	return MS::kSuccess;
}

MStatus ObjectVectorParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	ObjectVectorParameterPtr p = IECore::runTimeCast<ObjectVectorParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MStatus s;

	ObjectVectorPtr v = new ObjectVector;

	unsigned numElements = const_cast<MPlug &>( plug ).evaluateNumElements();
	for( unsigned i=0; i<numElements; i++ )
	{
		MPlug elementPlug = plug.elementByPhysicalIndex( i, &s );
		assert( s );

		ObjectPtr obj = 0;
		FromMayaConverterPtr c = FromMayaPlugConverter::create( elementPlug, Object::staticTypeId() );
		if( c )
		{
			obj = c->convert();
		}

		v->members().resize( std::max<ObjectVector::MemberContainer::size_type>( elementPlug.logicalIndex() + 1, v->members().size() ), 0 );
		v->members()[elementPlug.logicalIndex()] = obj;
	}

	p->setValue( v );

	return MS::kSuccess;
}
