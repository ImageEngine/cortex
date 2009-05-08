//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaTransformationMatrixConverter.h"
#include "IECoreMaya/Convert.h"

#include "IECore/TransformationMatrixData.h"

#include "maya/MFn.h"
#include "maya/MFnMatrixData.h"
#include "maya/MFnTransform.h"

using namespace IECore;
using namespace std;
using namespace Imath;

namespace IECoreMaya
{

static const MFn::Type fromTypes[] = { MFn::kMatrixData, MFn::kTransform, MFn::kInvalid };

static const IECore::TypeId toTypesf[] = { TransformationMatrixfData::staticTypeId(), InvalidTypeId };
static const IECore::TypeId toTypesd[] = { TransformationMatrixdData::staticTypeId(), InvalidTypeId };

template<>
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaTransformationMatrixConverter<TransformationMatrixdData> > FromMayaTransformationMatrixConverter<TransformationMatrixdData>::m_description( fromTypes, toTypesd );

template<>
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaTransformationMatrixConverter<TransformationMatrixfData> > FromMayaTransformationMatrixConverter<TransformationMatrixfData>::m_description( fromTypes, toTypesf );

template<typename T>
FromMayaTransformationMatrixConverter<T>::FromMayaTransformationMatrixConverter( const MObject &object )
	:	FromMayaObjectConverter( "FromMayaTransformationMatrixConverter", "Converts maya matrix data to IECore::TransformationMatrixData.", object )
{
}

template<typename T>
IECore::ObjectPtr FromMayaTransformationMatrixConverter<T>::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;
	MFnMatrixData fnMatrixData( object );
	if( fnMatrixData.hasObj( object ) )
	{
		MTransformationMatrix t = fnMatrixData.transformation();
		typename T::ValueType tt = IECore::convert<typename T::ValueType>( t );
		return new T( tt );
	}
	MFnTransform fnTranf( object, &s );
	if (s)
	{
		MTransformationMatrix t = fnTranf.transformation();
		typename T::ValueType tt = IECore::convert<typename T::ValueType>( t );
		return new T( tt );
	}
	return 0;
}

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaTransformationMatrixfConverter, FromMayaTransformationMatrixfConverterTypeId, FromMayaObjectConverter )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaTransformationMatrixdConverter, FromMayaTransformationMatrixdConverterTypeId, FromMayaObjectConverter )

template class FromMayaTransformationMatrixConverter<TransformationMatrixfData>;
template class FromMayaTransformationMatrixConverter<TransformationMatrixdData>;

} // namespace IECoreMaya
