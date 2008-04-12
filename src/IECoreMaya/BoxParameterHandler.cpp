//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
MStatus BoxParameterHandler<T>::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	intrusive_ptr<const IECore::TypedParameter<Box<T> > > p = IECore::runTimeCast<const IECore::TypedParameter<Box<T> > >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
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
	
	Box<T> defValue = p->typedDefaultValue();
	MStatus s;
	switch( T::dimensions() )
	{
		case 2 :
			s = fnMinAttr.setDefault( defValue.min[0], defValue.min[1] );
			if( !s )
			{
				return s;
			}
			s = fnMaxAttr.setDefault( defValue.max[0], defValue.max[1] );
			if( !s )
			{
				return s;
			}
			break;
		case 3 :
			s = fnMinAttr.setDefault( defValue.min[0], defValue.min[1], defValue.min[2] );
			if( !s )
			{
				return s;
			}
			s = fnMaxAttr.setDefault( defValue.max[0], defValue.max[1], defValue.max[2] );
			if( !s )
			{
				return s;
			}
			break;
		default :
			return MS::kFailure;
	}
		
	return MS::kSuccess;
}

template<typename T>
MObject BoxParameterHandler<T>::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	intrusive_ptr<const IECore::TypedParameter<Box<T > > > p = IECore::runTimeCast<const IECore::TypedParameter<Box<T > > >( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}
	
	MFnNumericAttribute fnNAttr;
	MFnCompoundAttribute fnCAttr;
	MObject oMin, oMax;
	switch( T::dimensions() )
	{
		case 2 :
			{
				MObject oMinX = fnNAttr.create( attributeName + "MinX", attributeName + "MinX", NumericTraits<T>::baseDataType() );
				MObject oMinY = fnNAttr.create( attributeName + "MinY", attributeName + "MinY", NumericTraits<T>::baseDataType() );
				oMin = fnNAttr.create( attributeName, attributeName, oMinX, oMinY );
				
				MObject oMaxX = fnNAttr.create( attributeName + "MaxX", attributeName + "MaxX", NumericTraits<T>::baseDataType() );
				MObject oMaxY = fnNAttr.create( attributeName + "MaxY", attributeName + "MaxY", NumericTraits<T>::baseDataType() );
				oMax = fnNAttr.create( attributeName, attributeName, oMaxX, oMaxY );
			}
			break;
		case 3 :
			oMin = fnNAttr.createPoint( attributeName + "Min", attributeName + "Min" );
			oMax = fnNAttr.createPoint( attributeName + "Max", attributeName + "Max" );
			break;
		default :	
			return MObject::kNullObj;
	}
	
	MObject result = fnCAttr.create( attributeName, attributeName );
	fnCAttr.addChild( oMin );
	fnCAttr.addChild( oMax );
	update( parameter, result );
	return result;
}
		
template<typename T>
MStatus BoxParameterHandler<T>::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	intrusive_ptr<const IECore::TypedParameter<Box<T> > > p = IECore::runTimeCast<const IECore::TypedParameter<Box<T> > >( parameter );
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
MStatus BoxParameterHandler<T>::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	intrusive_ptr<IECore::TypedParameter<Box<T> > > p = IECore::runTimeCast<IECore::TypedParameter<Box<T> > >( parameter );
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
