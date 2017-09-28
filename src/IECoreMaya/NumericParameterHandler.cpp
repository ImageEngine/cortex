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
#include "IECoreMaya/NumericParameterHandler.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/NumericParameter.h"
#include "IECore/CompoundObject.h"

#include "maya/MFnNumericAttribute.h"


using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< NumericParameterHandler<int> > intRegistrar( IECore::IntParameter::staticTypeId() );
static ParameterHandler::Description< NumericParameterHandler<float> > floatRegistrar( IECore::FloatParameter::staticTypeId() );
static ParameterHandler::Description< NumericParameterHandler<double> > doubleRegistrar( IECore::DoubleParameter::staticTypeId() );

template<typename T>
MStatus NumericParameterHandler<T>::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename IECore::NumericParameter<T>::ConstPtr p = IECore::runTimeCast<const IECore::NumericParameter<T> >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnNumericAttribute fnNAttr( attribute );
	if( !fnNAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}
	if( fnNAttr.unitType() != NumericTraits<T>::dataType() )
	{
		return MS::kFailure;
	}

	fnNAttr.setDefault( p->numericDefaultValue() );

	if( p->minValue()!=Imath::limits<T>::min() )
	{
		fnNAttr.setMin( p->minValue() );
	}
	else
	{
		// parameter has no min value
		if( fnNAttr.hasMin() )
		{
			// there is no way of unsetting a minimum value
			// in maya.
			return MS::kFailure;
		}
	}

	if( p->maxValue()!=Imath::limits<T>::max() )
	{
		fnNAttr.setMax( p->maxValue() );
	}
	else
	{
		// parameter has no max value
		if( fnNAttr.hasMax() )
		{
			// there is no way of unsetting a maximum value
			// in maya.
			return MS::kFailure;
		}
	}

	T v;
	MStatus result = plug.getValue( v );
	if( result )
	{
		IECore::ObjectPtr d = new IECore::TypedData<T>( v );
		if( !parameter->valueValid( d.get() ) )
		{
			return MS::kFailure;
		}
	}

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

	return finishUpdating( parameter, plug );
}

template<typename T>
MPlug NumericParameterHandler<T>::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	typename IECore::NumericParameter<T>::ConstPtr p = IECore::runTimeCast<const IECore::NumericParameter<T> >( parameter );
	if( !p )
	{
		return MPlug();
	}

	MFnNumericAttribute fnNAttr;
	MObject attribute = fnNAttr.create( plugName, plugName, NumericTraits<T>::dataType(), p->numericDefaultValue() );

	MPlug result = finishCreating( parameter, attribute, node );
	doUpdate( parameter, result );

	return result;
}

template<typename T>
MStatus NumericParameterHandler<T>::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	typename IECore::NumericParameter<T>::ConstPtr p = IECore::runTimeCast<const IECore::NumericParameter<T> >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	return plug.setValue( p->getNumericValue() );
}

template<typename T>
MStatus NumericParameterHandler<T>::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	typename IECore::NumericParameter<T>::Ptr p = IECore::runTimeCast<IECore::NumericParameter<T> >( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	T v;
	MStatus result;
	result = plug.getValue( v );
	if( result )
	{
		p->setNumericValue( v );
	}
	return result;
}
