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

#include "IECore/TransformationMatrixData.h"
#include "IECore/TypedData.inl"

namespace IECore {

#define TRANSFORMATIONMATRIX_SIZE	(7 * 3) + (2 * 4)		// 7 3D vectors and 2 quaternions.

#define COPYVALUES( TARGET, SOURCE, TINDEX, SINDEX, N )				\
		for ( int index = 0; index < N; index++ )					\
		{															\
			TARGET[ TINDEX + index ] = SOURCE[ SINDEX + index ];	\
		}

#define IE_CORE_DEFINEDATASPECIALISATION( T, TID, TNAME, BT )				\
																			\
	template<>																\
	TypeId TypedData<T>::typeId() const										\
	{																		\
		return TID;															\
	}																		\
	template<>																\
	TypeId TypedData<T>::staticTypeId()										\
	{																		\
		return TID;															\
	}																		\
	template<>																\
	std::string TypedData<T>::typeName() const								\
	{																		\
		return #TNAME;														\
	}																		\
	template<>																\
	std::string TypedData<T>::staticTypeName()								\
	{																		\
		return #TNAME;														\
	}																		\
	template<> 																\
	void TypedData<T>::save( SaveContext *context ) const								\
	{																					\
		Data::save( context );															\
		const T &t = readable();														\
		BT values[ TRANSFORMATIONMATRIX_SIZE ];											\
		int i = 0;																		\
		COPYVALUES( values, t.scalePivot, i, 0, 3 )										\
		i += 3;																			\
		COPYVALUES( values, t.scale, i, 0, 3 )											\
		i += 3;																			\
		COPYVALUES( values, t.shear, i, 0, 3 )											\
		i += 3;																			\
		COPYVALUES( values, t.scalePivotTranslation, i, 0, 3 )							\
		i += 3;																			\
		COPYVALUES( values, t.rotatePivot, i, 0, 3 )									\
		i += 3;																			\
		COPYVALUES( values, t.rotationOrientation, i, 0, 4 )							\
		i += 4;																			\
		COPYVALUES( values, t.rotate, i, 0, 4 )											\
		i += 4;																			\
		COPYVALUES( values, t.rotatePivotTranslation, i, 0, 3 )							\
		i += 3;																			\
		COPYVALUES( values, t.translate, i, 0, 3 )										\
		i += 3;																			\
		assert( i == TRANSFORMATIONMATRIX_SIZE );										\
																						\
		IndexedIOInterfacePtr container = context->rawContainer();						\
		container->write( "value", (const BT *)&values[0], TRANSFORMATIONMATRIX_SIZE );	\
	}																					\
	template<> 																			\
	void TypedData<T>::load( LoadContextPtr context )									\
	{																					\
		Data::load( context );															\
		BT values[ TRANSFORMATIONMATRIX_SIZE ];											\
		BT *p = (BT *)&(values[0]);														\
																						\
		try																				\
		{																				\
			IndexedIOInterfacePtr container = context->rawContainer();					\
			container->read( "value", p, TRANSFORMATIONMATRIX_SIZE );					\
		}																				\
		catch( ... )																	\
		{																				\
			unsigned int v = 0;															\
			IndexedIOInterfacePtr container = context->container( staticTypeName(), v );\
			container->read( "value", p, TRANSFORMATIONMATRIX_SIZE );					\
		}																				\
																						\
		T &t = writable();																\
		int i = 0;																		\
		COPYVALUES( t.scalePivot, values, 0, i, 3 )										\
		i += 3;																			\
		COPYVALUES( t.scale, values, 0, i, 3 )											\
		i += 3;																			\
		COPYVALUES( t.shear, values, 0, i, 3 )											\
		i += 3;																			\
		COPYVALUES( t.scalePivotTranslation, values, 0, i, 3 )							\
		i += 3;																			\
		COPYVALUES( t.rotatePivot, values, 0, i, 3 )									\
		i += 3;																			\
		COPYVALUES( t.rotationOrientation, values, 0, i, 4 )							\
		i += 4;																			\
		COPYVALUES( t.rotate, values, 0, i, 4 )											\
		i += 4;																			\
		COPYVALUES( t.rotatePivotTranslation, values, 0, i, 3 )							\
		i += 3;																			\
		COPYVALUES( t.translate, values, 0, i, 3 )										\
		i += 3;																			\
		assert( i == TRANSFORMATIONMATRIX_SIZE );										\
	}

IE_CORE_DEFINEDATASPECIALISATION( TransformationMatrixf, TransformationMatrixfDataTypeId, TransformationMatrixfData, float )
IE_CORE_DEFINEDATASPECIALISATION( TransformationMatrixd, TransformationMatrixdDataTypeId, TransformationMatrixdData, double )

template class TypedData< TransformationMatrix<float> >;
template class TypedData< TransformationMatrix<double> >;


} // namespace IECore
