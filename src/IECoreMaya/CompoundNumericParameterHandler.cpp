//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
#include "IECoreMaya/CompoundNumericParameterHandler.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/NumericParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/TypedParameter.h"

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnCompoundAttribute.h"


using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< CompoundNumericParameterHandler<V2i> > v2iRegistrar( IECore::V2iParameter::staticTypeId() );
static ParameterHandler::Description< CompoundNumericParameterHandler<V3i> > v3iRegistrar( IECore::V3iParameter::staticTypeId() );

static ParameterHandler::Description< CompoundNumericParameterHandler<V2f> > v2fRegistrar( IECore::V2fParameter::staticTypeId() );
static ParameterHandler::Description< CompoundNumericParameterHandler<V3f> > v3fRegistrar( IECore::V3fParameter::staticTypeId() );

static ParameterHandler::Description< CompoundNumericParameterHandler<V2d> > v2dRegistrar( IECore::V2dParameter::staticTypeId() );
static ParameterHandler::Description< CompoundNumericParameterHandler<V3d> > v3dRegistrar( IECore::V3dParameter::staticTypeId() );

static ParameterHandler::Description< CompoundNumericParameterHandler<Color3f> > color3fRegistrar( IECore::Color3fParameter::staticTypeId() );

template<typename T>
MStatus CompoundNumericParameterHandler<T>::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	typename IECore::TypedParameter<T>::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<T> >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MFnNumericAttribute fnNAttr( attribute );
	if( !fnNAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}
	
	if( fnNAttr.unitType()!=NumericTraits<T>::dataType() )
	{
		return MS::kFailure;
	}
	
	/// Set the default value one child attribute at a time. It would appear that using the variants of setDefault
	/// whicn take 2 or 3 arguments can exercise a Maya bug.
	T defValue = p->typedDefaultValue();
	for( unsigned i=0; i<T::dimensions(); i++ )
	{
		MStatus s;
		MObject childAttr = fnNAttr.child( i, &s );
		if ( !s )
		{
			return s;
		}
		MFnNumericAttribute fnChildNAttr( childAttr, &s );
		if ( !s )
		{
			return s;
		}
		s = fnChildNAttr.setDefault( defValue[i] );
		if ( !s )
		{
			return s;
		}
	}
	
#ifndef NDEBUG
	/// Verify that the defaults have been set correctly. Only do this in asserted builds.
	switch( T::dimensions() )
	{
		case 2 :
			{
				typename T::BaseType c0, c1 ;
				MStatus s = fnNAttr.getDefault( c0, c1 );
				assert( s );
				assert( c0 == defValue[0] );
				assert( c1 == defValue[1] );
			}
			break;
		case 3 :
			{
				typename T::BaseType c0, c1, c2;
				MStatus s = fnNAttr.getDefault( c0, c1, c2 );
				assert( s );
				assert( c0 == defValue[0] );
				assert( c1 == defValue[1] );
				assert( c2 == defValue[2] );								
			}
			break;
		default :
			assert( false );
	}
#endif	
			
	fnNAttr.setUsedAsColor( NumericTraits<T>::isColor() );
	
	bool keyable = true;
	bool channelBox = true;
	
	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	assert( userData );
	
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");
	if (maya)
	{
		const IECore::ConstBoolDataPtr keyableData = maya->member<const IECore::BoolData>("keyable");		
		if (keyableData)
		{
			keyable = keyableData->readable();
		}
		
		const IECore::ConstBoolDataPtr channelBoxData = maya->member<const IECore::BoolData>("channelBox");		
		if (channelBoxData)
		{
			channelBox = channelBoxData->readable();
		}
	}	
	
	fnNAttr.setKeyable( keyable );
	
	// Calling setChannelBox(true) disables keying
	if (!keyable)
	{
		fnNAttr.setChannelBox( channelBox );
	}
	
	return MS::kSuccess;
}

template<typename T>
MObject CompoundNumericParameterHandler<T>::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	typename IECore::TypedParameter<T>::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<T> >( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}
	
	MFnNumericAttribute fnNAttr;
	MObject result;
	switch( T::dimensions() )
	{
		case 2 :
			{
				assert( !NumericTraits<T>::isColor() );
				MObject e0 = fnNAttr.create( attributeName + "X", attributeName + "X", NumericTraits<T>::baseDataType() );
				MObject e1 = fnNAttr.create( attributeName + "Y", attributeName + "Y", NumericTraits<T>::baseDataType() );
				result = fnNAttr.create( attributeName, attributeName, e0, e1 );
			}
			break;
		case 3 :
			if( NumericTraits<T>::isColor() )
			{			
				result = fnNAttr.createColor( attributeName, attributeName );
			}
			else
			{
				MObject e0 = fnNAttr.create( attributeName + "X", attributeName + "X", NumericTraits<T>::baseDataType() );
				MObject e1 = fnNAttr.create( attributeName + "Y", attributeName + "Y", NumericTraits<T>::baseDataType() );
				MObject e2 = fnNAttr.create( attributeName + "Z", attributeName + "Z", NumericTraits<T>::baseDataType() );
				result = fnNAttr.create( attributeName, attributeName, e0, e1, e2 );
			}
			break;
		default :
			assert( false );	
			result = MObject::kNullObj;
	}
	
	update( parameter, result );
	return result;
}
		
template<typename T>
MStatus CompoundNumericParameterHandler<T>::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename IECore::TypedParameter<T>::ConstPtr p = IECore::runTimeCast<const IECore::TypedParameter<T> >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	if( plug.numChildren() != T::dimensions() )
	{
		return MS::kFailure;
	}
	
	T v = p->getTypedValue();
	for( unsigned i=0; i<plug.numChildren(); i++ )
	{
		MStatus s = plug.child( i ).setValue( v[i] );
		if( !s )
		{
			return s;
		}
	}

	return MS::kSuccess;
}

template<typename T>
MStatus CompoundNumericParameterHandler<T>::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	typename IECore::TypedParameter<T>::Ptr p = IECore::runTimeCast<IECore::TypedParameter<T> >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	if( plug.numChildren() != T::dimensions() )
	{
		return MS::kFailure;
	}
	
	T v;
	for( unsigned i=0; i<plug.numChildren(); i++ )
	{
		MStatus s = plug.child( i ).getValue( v[i] );
		if( !s )
		{
			return s;
		}
	}
	
	p->setTypedValue( v );
	return MS::kSuccess;
}
