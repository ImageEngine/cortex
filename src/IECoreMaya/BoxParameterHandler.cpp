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
#include "IECoreMaya/BoxParameterHandler.h"

#include "IECore/NumericParameter.h"
#include "IECore/TypedParameter.h"

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnCompoundAttribute.h"


using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< BoxParameterHandler<V2f> > box2fRegistrar( IECore::Box2fParameter::staticTypeId() );
static ParameterHandler::Description< BoxParameterHandler<V3f> > box3fRegistrar( IECore::Box3fParameter::staticTypeId() );
static ParameterHandler::Description< BoxParameterHandler<V2d> > box2dRegistrar( IECore::Box2dParameter::staticTypeId() );
static ParameterHandler::Description< BoxParameterHandler<V3d> > box3dRegistrar( IECore::Box3dParameter::staticTypeId() );

template<typename T>
MStatus BoxParameterHandler<T>::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename IECore::TypedParameter<Box<T> >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<Box<T> > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnCompoundAttribute fnCAttr( attribute );
	if( !fnCAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	if( fnCAttr.numChildren()!=2 )
	{
		return MS::kFailure;
	}

	MFnNumericAttribute fnMinAttr( fnCAttr.child( 0 ) );
	if( fnMinAttr.unitType()!=NumericTraits<T>::dataType() )
	{
		return MS::kFailure;
	}

	MFnNumericAttribute fnMaxAttr( fnCAttr.child( 1 ) );
	if( fnMaxAttr.unitType()!=NumericTraits<T>::dataType() )
	{
		return MS::kFailure;
	}

	// Set the default value for the leaf attributes individually. Calling
	// the variants of setDefault that set several components at a time
	// seems to exercise a maya bug. See similar comment in CompoundNumericParameterHandler.
	Box<T> defValue = p->typedDefaultValue();
	MStatus s;
	for( unsigned i=0; i<T::dimensions(); i++ )
	{
		MObject minChildAttr = fnMinAttr.child( i, &s );
		if( !s )
		{
			return s;
		}

		MObject maxChildAttr = fnMaxAttr.child( i, &s );
		if( !s )
		{
			return s;
		}

		MFnNumericAttribute fnMinChildAttr( minChildAttr, &s );
		if( !s )
		{
			return s;
		}

		MFnNumericAttribute fnMaxChildAttr( maxChildAttr, &s );
		if( !s )
		{
			return s;
		}

		s = fnMinChildAttr.setDefault( defValue.min[i] );
		if( !s )
		{
			return s;
		}

		s = fnMaxChildAttr.setDefault( defValue.max[i] );
		if( !s )
		{
			return s;
		}

	}

	return finishUpdating( parameter, plug );
}

template<typename T>
MPlug BoxParameterHandler<T>::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	typename IECore::TypedParameter<Box<T> >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<Box<T > > >( parameter );
	if( !p )
	{
		return MPlug();
	}

	MFnNumericAttribute fnNAttr;
	MFnCompoundAttribute fnCAttr;
	MObject oMin, oMax;
	switch( T::dimensions() )
	{
		case 2 :
			{
				MObject oMinX = fnNAttr.create( plugName + "MinX", plugName + "MinX", NumericTraits<T>::baseDataType() );
				MObject oMinY = fnNAttr.create( plugName + "MinY", plugName + "MinY", NumericTraits<T>::baseDataType() );
				oMin = fnNAttr.create( plugName + "Min", plugName + "Min", oMinX, oMinY );

				MObject oMaxX = fnNAttr.create( plugName + "MaxX", plugName + "MaxX", NumericTraits<T>::baseDataType() );
				MObject oMaxY = fnNAttr.create( plugName + "MaxY", plugName + "MaxY", NumericTraits<T>::baseDataType() );
				oMax = fnNAttr.create( plugName + "Max", plugName + "Max", oMaxX, oMaxY );
			}
			break;
		case 3 :
			oMin = fnNAttr.createPoint( plugName + "Min", plugName + "Min" );
			oMax = fnNAttr.createPoint( plugName + "Max", plugName + "Max" );
			break;
		default :
			return MPlug();
	}

	MObject attribute = fnCAttr.create( plugName, plugName );
	fnCAttr.addChild( oMin );
	fnCAttr.addChild( oMax );

	MPlug result = finishCreating( parameter, attribute, node );
	doUpdate( parameter, result );

	return result;
}

template<typename T>
MStatus BoxParameterHandler<T>::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename IECore::TypedParameter<Box<T> >::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<Box<T> > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	if( plug.numChildren() != 2 )
	{
		return MS::kFailure;
	}

	MPlug minPlug = plug.child( 0 );
	MPlug maxPlug = plug.child( 1 );

	if( minPlug.numChildren()!=T::dimensions() || maxPlug.numChildren()!=T::dimensions() )
	{
		return MS::kFailure;
	}

	Box<T> v = p->getTypedValue();
	for( unsigned i=0; i<minPlug.numChildren(); i++ )
	{
		MStatus s = minPlug.child( i ).setValue( v.min[i] );
		if( !s )
		{
			return s;
		}
		s = maxPlug.child( i ).setValue( v.max[i] );
		if( !s )
		{
			return s;
		}
	}

	return MS::kSuccess;
}

template<typename T>
MStatus BoxParameterHandler<T>::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	typename IECore::TypedParameter<Box<T> >::Ptr p = IECore::runTimeCast<IECore::TypedParameter<Box<T> > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	if( plug.numChildren() != 2 )
	{
		return MS::kFailure;
	}

	MPlug minPlug = plug.child( 0 );
	MPlug maxPlug = plug.child( 1 );

	if( minPlug.numChildren()!=T::dimensions() || maxPlug.numChildren()!=T::dimensions() )
	{
		return MS::kFailure;
	}

	Box<T> v;
	for( unsigned i=0; i<minPlug.numChildren(); i++ )
	{
		MStatus s = minPlug.child( i ).getValue( v.min[i] );
		if( !s )
		{
			return s;
		}
		s = maxPlug.child( i ).getValue( v.max[i] );
		if( !s )
		{
			return s;
		}
	}

	p->setTypedValue( v );
	return MS::kSuccess;
}
