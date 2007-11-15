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
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/ObjectMFnDataParameterHandler.h"
#include "IECoreMaya/MFnDataTypeTraits.h"

#include "IECore/VectorTypedData.h"
#include "IECore/TypedParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/ObjectParameter.h"

#include "maya/MFnData.h"
#include "maya/MFnGenericAttribute.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::V3fVectorParameter,    MFnData::kVectorArray > > v3fVectorRegistrar( IECore::V3fVectorParameter::staticTypeId(), IECore::V3fVectorData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::V3dVectorParameter,    MFnData::kVectorArray > > v3dVectorRegistrar( IECore::V3dVectorParameter::staticTypeId(), IECore::V3dVectorData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::StringVectorParameter, MFnData::kStringArray > > stringVectorRegistrar( IECore::StringVectorParameter::staticTypeId(), IECore::StringVectorData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::StringParameter,       MFnData::kString > >      stringRegistrar( IECore::StringParameter::staticTypeId(), IECore::StringData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::M44fParameter,         MFnData::kMatrix > >      m44fRegistrar( IECore::M44fParameter::staticTypeId(), IECore::M44fData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::M44dParameter,         MFnData::kMatrix > >      m44dRegistrar( IECore::M44dParameter::staticTypeId(), IECore::M44dData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::FloatVectorParameter,  MFnData::kDoubleArray > > floatVectorRegistrar( IECore::FloatVectorParameter::staticTypeId(), IECore::FloatVectorData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::DoubleVectorParameter, MFnData::kDoubleArray > > doubleVectorRegistrar( IECore::DoubleVectorParameter::staticTypeId(), IECore::DoubleVectorData::staticTypeId() );
ParameterHandler::Description< ObjectMFnDataParameterHandler< IECore::IntVectorParameter,    MFnData::kIntArray > >    intVectorRegistrar( IECore::IntVectorParameter::staticTypeId(), IECore::IntVectorData::staticTypeId() );

template<typename T, MFnData::Type D>
MStatus ObjectMFnDataParameterHandler<T, D>::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	if (!dynamic_pointer_cast<const IECore::ObjectParameter>( parameter ) && !dynamic_pointer_cast<const T>( parameter ))
	{
		return MS::kFailure;
	}

	MFnGenericAttribute fnGAttr( attribute );
	if( !fnGAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	fnGAttr.addAccept( D );
	
	return MS::kSuccess;
}

template<typename T, MFnData::Type D>
MObject ObjectMFnDataParameterHandler<T, D>::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	if (!dynamic_pointer_cast<const IECore::ObjectParameter>( parameter ) && !dynamic_pointer_cast<const T>( parameter ))
	{
		return MObject::kNullObj;
	}
	
	/// Use a generic attribute, so we could eventually accept other ObjectParamter types, too.
	MStatus s;
	MFnGenericAttribute fnGAttr;
	MObject result = fnGAttr.create( attributeName, attributeName, &s );
	
	if ( !ObjectMFnDataParameterHandler<T, D>::update( parameter, result ) )
	{
		return MObject::kNullObj;
	}
	
	return result;
}
		
template<typename T, MFnData::Type D>		
MStatus ObjectMFnDataParameterHandler<T, D>::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	if (!dynamic_pointer_cast<const IECore::ObjectParameter>( parameter ) && !dynamic_pointer_cast<const T>( parameter ))
	{
		return MS::kFailure;
	}
	
	typedef IECore::TypedData<typename T::ValueType> Data;
		
	assert( parameter->getValue() );
	intrusive_ptr<const Data> v = dynamic_pointer_cast<const Data>( parameter->getValue() );
	
	/// It's OK for this cast to fail, for example if a parameter type has changed from V3fVectorData to V3dVectorData. This can
	/// happen when scenes are reopened. The ObjectParameterHandler will just try other ways of setting the value instead.
	if (!v)
	{
		return MS::kFailure;
	}
	
	ToMayaObjectConverterPtr converter = ToMayaObjectConverter::create( v, MFnDataTypeTraits<D>::dataType() ); 
	assert( converter );
	
	MObject obj;
	bool success = converter->convert( obj );
	if (!success)
	{
		return MS::kFailure;
	}
	
	assert( obj != MObject::kNullObj );
		
	return plug.setValue( obj );
}

template<typename T, MFnData::Type D>
MStatus ObjectMFnDataParameterHandler<T, D>::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{	
	if (!dynamic_pointer_cast<IECore::ObjectParameter>( parameter ) && !dynamic_pointer_cast<T>( parameter ))
	{
		return MS::kFailure;
	}
	
	MObject obj;
	MStatus result = plug.getValue( obj );
	if (!result)
	{
		return MS::kFailure;
	}
	
	FromMayaObjectConverterPtr converter = FromMayaObjectConverter::create( obj, T::ObjectType::staticTypeId() );
	/// It's OK if there's no converter - the ObjectParameterHandler will just try other ways of setting the value instead.
	if (!converter)
	{
		return MS::kFailure;
	}
	
	IECore::ObjectPtr v = converter->convert();
	assert( v );
	
	parameter->setValue( v );
	
	return MS::kSuccess;
}
