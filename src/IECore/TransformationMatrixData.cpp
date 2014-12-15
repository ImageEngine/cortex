//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Export.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/TypedData.inl"

namespace IECore {

static IndexedIO::EntryID g_valueEntry("value");

#define TRANSFORMATIONMATRIX_SIZE	(8 * 3) + 4 + 1		// 8 3D vectors, 1 quaternion and one rotation order.

// define save/load methods backward compatible with the old structure that represented rotate as Quat<T> instead of Euler<T>
#define IE_CORE_DEFINETRANSFORMATIONMATRIXIOSPECIALIZATION( TNAME )									\
	template<>																						\
	void TNAME::save( SaveContext *context ) const													\
	{																								\
		const TNAME::ValueType &base = TNAME::readable();											\
		TNAME::BaseType values[ TRANSFORMATIONMATRIX_SIZE ] = {										\
			base.scalePivot.x, base.scalePivot.y, base.scalePivot.z,								\
			base.scale.x, base.scale.y, base.scale.z,												\
			base.shear.x, base.shear.y, base.shear.z,																				\
			base.scalePivotTranslation.x, base.scalePivotTranslation.y, base.scalePivotTranslation.z,								\
			base.rotatePivot.x, base.rotatePivot.y, base.rotatePivot.z,																\
			base.rotationOrientation.r, base.rotationOrientation.v.x, base.rotationOrientation.v.y, base.rotationOrientation.v.z, 	\
			(TNAME::BaseType)(base.rotate.order())+0.2f, base.rotate.x, base.rotate.y, base.rotate.z, 								\
			base.rotatePivotTranslation.x, base.rotatePivotTranslation.y, base.rotatePivotTranslation.z, 							\
			base.translate.x, base.translate.y, base.translate.z 																	\
		};																							\
		Data::save( context );																		\
		IndexedIO *container = context->rawContainer();												\
		container->write( g_valueEntry, values, TRANSFORMATIONMATRIX_SIZE );						\
	}																								\
																									\
	template<>																						\
	void TNAME::load( LoadContextPtr context )														\
	{																								\
		Data::load( context );																		\
		TNAME::BaseType values[ TRANSFORMATIONMATRIX_SIZE ];										\
		TNAME::BaseType *p = &values[0];															\
		try																							\
		{																							\
			const IndexedIO *container = context->rawContainer();									\
			container->read( g_valueEntry, p, TRANSFORMATIONMATRIX_SIZE );							\
		}																							\
		catch( ... )																				\
		{																							\
			unsigned int v = 0;																		\
			ConstIndexedIOPtr container = context->container( staticTypeName(), v );				\
			container->read( g_valueEntry, p, TRANSFORMATIONMATRIX_SIZE );							\
		}																							\
		TNAME::ValueType &base = TNAME::writable();													\
		base.scalePivot.x = *p++;																	\
		base.scalePivot.y = *p++;																	\
		base.scalePivot.z = *p++;																	\
		base.scale.x = *p++;																		\
		base.scale.y = *p++;																		\
		base.scale.z = *p++;																		\
		base.shear.x = *p++;																		\
		base.shear.y = *p++;																		\
		base.shear.z = *p++;																		\
		base.scalePivotTranslation.x = *p++;														\
		base.scalePivotTranslation.y = *p++;														\
		base.scalePivotTranslation.z = *p++;														\
		base.rotatePivot.x = *p++;																	\
		base.rotatePivot.y = *p++;																	\
		base.rotatePivot.z = *p++;																	\
		base.rotationOrientation.r = *p++;															\
		base.rotationOrientation.v.x = *p++;														\
		base.rotationOrientation.v.y = *p++;														\
		base.rotationOrientation.v.z = *p++;														\
		TNAME::BaseType order = *p++;																\
		base.rotate.x = *p++;																		\
		base.rotate.y = *p++;																		\
		base.rotate.z = *p++;																		\
		if ( order <= 1.0 )																			\
		{																							\
			/* backward compatibility: rotate used to be a quaternion... */							\
			base.rotate.setOrder( Imath::Euler<TNAME::BaseType>::XYZ );								\
			base.rotate.extract( Imath::Quat<TNAME::BaseType>( order, base.rotate.x, base.rotate.y, base.rotate.z ));	\
		} else																						\
		{																							\
			base.rotate.setOrder( (Imath::Euler<TNAME::BaseType>::Order)(order) );		\
		}																							\
		base.rotatePivotTranslation.x = *p++;														\
		base.rotatePivotTranslation.y = *p++;														\
		base.rotatePivotTranslation.z = *p++;														\
		base.translate.x = *p++;																	\
		base.translate.y = *p++;																	\
		base.translate.z = *p++;																	\
	}\
\
	template<>\
	void SimpleDataHolder<TNAME::ValueType>::hash( MurmurHash &h ) const\
	{\
		const TNAME::ValueType &v = readable();\
		h.append( v.scalePivot );\
		h.append( v.scale );\
		h.append( v.shear );\
		h.append( v.scalePivotTranslation );\
		h.append( v.rotatePivot );\
		h.append( v.rotationOrientation );\
		h.append( v.rotate );\
		h.append( v.rotate.order() );\
		h.append( v.rotatePivotTranslation );\
		h.append( v.translate );\
	}\

#define IE_CORE_DEFINEDATASPECIALISATION( TNAME, TID )							\
	IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( TNAME, TID )					\
	IE_CORE_DEFINETRANSFORMATIONMATRIXIOSPECIALIZATION( TNAME )

IE_CORE_DEFINEDATASPECIALISATION( TransformationMatrixfData, TransformationMatrixfDataTypeId )
IE_CORE_DEFINEDATASPECIALISATION( TransformationMatrixdData, TransformationMatrixdDataTypeId )

template class IECORE_API TypedData< TransformationMatrix<float> >;
template class IECORE_API TypedData< TransformationMatrix<double> >;


} // namespace IECore
