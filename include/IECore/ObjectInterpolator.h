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


#ifndef IE_CORE_OBJECTINTERPOLATOR_H
#define IE_CORE_OBJECTINTERPOLATOR_H

/// Extension module for Interpolator class specialized on ObjectPtr
/// Remember to update both functions in this module (for two and four data points ).

#include "IECore/Object.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/CompoundData.h"
#include "IECore/CompoundObject.h"
#include "IECore/Interpolator.h"
#include "IECore/Exception.h"

namespace IECore
{

/// Utility function that applies linear interpolation on objects. Returns a "null" pointer if the Object cannot be interpolated.
ObjectPtr linearObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, double x );

/// Utility function that applies cubic interpolation on objects. Returns a "null" pointer if the Object cannot be interpolated.
ObjectPtr cubicObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, const ObjectPtr &y2, const ObjectPtr &y3, double x );


#define INTERPOLATE2_TYPE( TYPE )												\
		case TYPE ## TypeId:													\
			{																	\
			TYPE ## Ptr x0 = boost::static_pointer_cast<TYPE>( y0 );			\
			TYPE ## Ptr x1 = boost::static_pointer_cast<TYPE>( y1 );			\
			TYPE ## Ptr xRes = boost::static_pointer_cast<TYPE>( result );		\
			Functor<TYPE>()( x0, x1, x, xRes );									\
			break;																\
			}

/// Two data points interpolation template function for ObjectPtr.
/// Gets the typeId from y0 and make the proper conversion if the type is continuous. Otherwise returns false.
/// Assumes y1 and result has the same type as y0 parameter.
/// Also interpolates data within CompoundObject and CompoundData. Takes data from y0 if interpolation is not possible.
template<template<typename T> class Functor>
bool ObjectInterpolator( const ObjectPtr &y0, const ObjectPtr & y1, double x, ObjectPtr &result )
{

	if ( y0->typeId() != y1->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}

	/// \todo Use TypeTraits::IsInterpolable, or similar (need to account for integral types not being interpolated here)
	switch( y0->typeId() )
	{
		INTERPOLATE2_TYPE( FloatData )
		INTERPOLATE2_TYPE( DoubleData )
		INTERPOLATE2_TYPE( V2fData )
		INTERPOLATE2_TYPE( V3fData )
		INTERPOLATE2_TYPE( V2dData )
		INTERPOLATE2_TYPE( V3dData )
		INTERPOLATE2_TYPE( QuatfData )
		INTERPOLATE2_TYPE( QuatdData )
		INTERPOLATE2_TYPE( FloatVectorData )
		INTERPOLATE2_TYPE( DoubleVectorData )
		INTERPOLATE2_TYPE( HalfVectorData )
		INTERPOLATE2_TYPE( V2fVectorData )
		INTERPOLATE2_TYPE( V2dVectorData )
		INTERPOLATE2_TYPE( V3fVectorData )
		INTERPOLATE2_TYPE( V3dVectorData )
		INTERPOLATE2_TYPE( QuatfVectorData )
		INTERPOLATE2_TYPE( QuatdVectorData )
		INTERPOLATE2_TYPE( TransformationMatrixfData )
		INTERPOLATE2_TYPE( TransformationMatrixdData )
		INTERPOLATE2_TYPE( Color3fData )
		INTERPOLATE2_TYPE( Color4fData )
		INTERPOLATE2_TYPE( Color3dData )
		INTERPOLATE2_TYPE( Color4dData )
		INTERPOLATE2_TYPE( Box2fData )
		INTERPOLATE2_TYPE( Box3fData )
		INTERPOLATE2_TYPE( Box2dData )
		INTERPOLATE2_TYPE( Box3dData )
		INTERPOLATE2_TYPE( M33fData )
		INTERPOLATE2_TYPE( M33dData )
		INTERPOLATE2_TYPE( M44fData )
		INTERPOLATE2_TYPE( M44dData )
		INTERPOLATE2_TYPE( Color3fVectorData )
		INTERPOLATE2_TYPE( Color4fVectorData )
		INTERPOLATE2_TYPE( Color3dVectorData )
		INTERPOLATE2_TYPE( Color4dVectorData )
		INTERPOLATE2_TYPE( Box3fVectorData )
		INTERPOLATE2_TYPE( Box3dVectorData )
		INTERPOLATE2_TYPE( Box2fVectorData )
		INTERPOLATE2_TYPE( Box2dVectorData )
		INTERPOLATE2_TYPE( M33fVectorData )
		INTERPOLATE2_TYPE( M33dVectorData )
		INTERPOLATE2_TYPE( M44fVectorData )
		INTERPOLATE2_TYPE( M44dVectorData )

		case CompoundDataTypeId :
			{
			CompoundDataPtr x0 = boost::static_pointer_cast<CompoundData>( y0 );
			CompoundDataPtr x1 = boost::static_pointer_cast<CompoundData>( y1 );
			CompoundDataPtr xRes = boost::static_pointer_cast<CompoundData>( result );
			for ( CompoundDataMap::const_iterator it0 = x0->readable().begin(); it0 != x0->readable().end(); it0++)
			{
				CompoundDataMap::const_iterator it1 = x1->readable().find( it0->first );
				if ( it1 != x1->readable().end() && it0->second->typeId() == it1->second->typeId() )
				{
					ObjectPtr resultObj = Object::create( it0->second->typeId() );
					if ( ObjectInterpolator< Functor >( it0->second, it1->second, x, resultObj ) )
					{
						xRes->writable()[ it0->first ] = boost::static_pointer_cast<Data>( resultObj );
					}
					else
					{
						xRes->writable()[ it0->first ] = it0->second;
					}
				}
			}
			break;
			}
		case CompoundObjectTypeId :
			{
			CompoundObjectPtr x0 = boost::static_pointer_cast<CompoundObject>( y0 );
			CompoundObjectPtr x1 = boost::static_pointer_cast<CompoundObject>( y1 );
			CompoundObjectPtr xRes = boost::static_pointer_cast<CompoundObject>( result );
			for ( CompoundObject::ObjectMap::const_iterator it0 = x0->members().begin(); it0 != x0->members().end(); it0++)
			{
				CompoundObject::ObjectMap::const_iterator it1 = x1->members().find( it0->first );
				if ( it1 != x1->members().end() && it0->second->typeId() == it1->second->typeId() )
				{
					ObjectPtr resultObj = Object::create( it0->second->typeId() );
					if ( ObjectInterpolator< Functor >( it0->second, it1->second, x, resultObj ) )
					{
						xRes->members()[ it0->first ] = resultObj;
					}
					else
					{
						xRes->members()[ it0->first ] = it0->second;
					}
				}
			}
			break;
			}

		case BoolDataTypeId :
		case IntDataTypeId :
		case LongDataTypeId :
		case UIntDataTypeId :
		case CharDataTypeId :
		case UCharDataTypeId :
		case ShortDataTypeId :
		case UShortDataTypeId :
		case V2iDataTypeId :
		case V3iDataTypeId :
		case Box2iDataTypeId :
		case Box3iDataTypeId :
		case IntVectorDataTypeId :
		case UIntVectorDataTypeId :
		case CharVectorDataTypeId :
		case UCharVectorDataTypeId :
		case LongVectorDataTypeId :
			/// discrete numeric values are not interpolated.
			return false;

		case StringDataTypeId :
		case StringVectorDataTypeId :
			/// non-numeric values are not interpolated
			return false;

		default :

			// unknown types.			
			return false;
	}
	return true;
}

#define INTERPOLATE4_TYPE( TYPE )												\
		case TYPE ## TypeId:													\
			{																	\
			TYPE ## Ptr x0 = boost::static_pointer_cast<TYPE>( y0 );			\
			TYPE ## Ptr x1 = boost::static_pointer_cast<TYPE>( y1 );			\
			TYPE ## Ptr x2 = boost::static_pointer_cast<TYPE>( y2 );			\
			TYPE ## Ptr x3 = boost::static_pointer_cast<TYPE>( y3 );			\
			TYPE ## Ptr xRes = boost::static_pointer_cast<TYPE>( result );		\
			Functor<TYPE>()( x0, x1, x2, x3, x, xRes );							\
			break;																\
			}


/// Four data points interpolation template function for ObjectPtr.
/// Gets the typeId from y0 and make the proper conversion if the type is continuous. Otherwise returns false.
/// Assumes y1, y2, y3 and result has the same type as y0 parameter.
/// Also interpolates data within CompoundObject and CompoundData. Takes data from y1 if interpolation is not possible.
template<template<typename T> class Functor>
bool ObjectInterpolator( const ObjectPtr &y0, const ObjectPtr & y1, const ObjectPtr &y2, const ObjectPtr & y3, double x, ObjectPtr &result )
{

	if ( y0->typeId() != y1->typeId() || y0->typeId() != y2->typeId() || y0->typeId() != y3->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}

	/// \todo Use TypeTraits::IsInterpolable, or similar (need to account for integral types not being interpolated here)
	switch( y0->typeId() )
	{
		INTERPOLATE4_TYPE( FloatData )
		INTERPOLATE4_TYPE( DoubleData )
		INTERPOLATE4_TYPE( V2fData )
		INTERPOLATE4_TYPE( V3fData )
		INTERPOLATE4_TYPE( V2dData )
		INTERPOLATE4_TYPE( V3dData )
		INTERPOLATE4_TYPE( QuatfData )
		INTERPOLATE4_TYPE( QuatdData )
		INTERPOLATE4_TYPE( FloatVectorData )
		INTERPOLATE4_TYPE( DoubleVectorData )
		INTERPOLATE4_TYPE( HalfVectorData )
		INTERPOLATE4_TYPE( V2fVectorData )
		INTERPOLATE4_TYPE( V2dVectorData )
		INTERPOLATE4_TYPE( V3fVectorData )
		INTERPOLATE4_TYPE( V3dVectorData )
		INTERPOLATE4_TYPE( QuatfVectorData )
		INTERPOLATE4_TYPE( QuatdVectorData )
		INTERPOLATE4_TYPE( TransformationMatrixfData )
		INTERPOLATE4_TYPE( TransformationMatrixdData )
		INTERPOLATE4_TYPE( Color3fData )
		INTERPOLATE4_TYPE( Color4fData )
		INTERPOLATE4_TYPE( Color3dData )
		INTERPOLATE4_TYPE( Color4dData )
		INTERPOLATE4_TYPE( Box2fData )
		INTERPOLATE4_TYPE( Box3fData )
		INTERPOLATE4_TYPE( Box2dData )
		INTERPOLATE4_TYPE( Box3dData )
		INTERPOLATE4_TYPE( M33fData )
		INTERPOLATE4_TYPE( M33dData )
		INTERPOLATE4_TYPE( M44fData )
		INTERPOLATE4_TYPE( M44dData )
		INTERPOLATE4_TYPE( Color3fVectorData )
		INTERPOLATE4_TYPE( Color4fVectorData )
		INTERPOLATE4_TYPE( Color3dVectorData )
		INTERPOLATE4_TYPE( Color4dVectorData )
		INTERPOLATE4_TYPE( Box3fVectorData )
		INTERPOLATE4_TYPE( Box3dVectorData )
		INTERPOLATE4_TYPE( Box2fVectorData )
		INTERPOLATE4_TYPE( Box2dVectorData )
		INTERPOLATE4_TYPE( M33fVectorData )
		INTERPOLATE4_TYPE( M33dVectorData )
		INTERPOLATE4_TYPE( M44fVectorData )
		INTERPOLATE4_TYPE( M44dVectorData )

		case CompoundDataTypeId :
			{
			CompoundDataPtr x0 = boost::static_pointer_cast<CompoundData>( y0 );
			CompoundDataPtr x1 = boost::static_pointer_cast<CompoundData>( y1 );
			CompoundDataPtr x2 = boost::static_pointer_cast<CompoundData>( y2 );
			CompoundDataPtr x3 = boost::static_pointer_cast<CompoundData>( y3 );
			CompoundDataPtr xRes = boost::static_pointer_cast<CompoundData>( result );
			for ( CompoundDataMap::const_iterator it1 = x1->readable().begin(); it1 != x1->readable().end(); it1++)
			{
				CompoundDataMap::const_iterator it0 = x0->readable().find( it1->first );
				if ( it0 != x0->readable().end() && it0->second->typeId() == it1->second->typeId() )
				{
					CompoundDataMap::const_iterator it2 = x2->readable().find( it1->first );
					if ( it2 != x2->readable().end() && it0->second->typeId() == it2->second->typeId() )
					{
						CompoundDataMap::const_iterator it3 = x3->readable().find( it1->first );
						if ( it3 != x3->readable().end() && it0->second->typeId() == it3->second->typeId() )
						{
							ObjectPtr resultObj = Object::create( it1->second->typeId() );
							if ( ObjectInterpolator< Functor >( it0->second, it1->second, it2->second, it3->second, x, resultObj ) )
							{
								xRes->writable()[ it1->first ] = boost::static_pointer_cast<Data>( resultObj );
							}
							else
							{
								xRes->writable()[ it1->first ] = it1->second;
							}
						}
						else
						{
							xRes->writable()[ it1->first ] = it1->second;
						}
					}
					else
					{
						xRes->writable()[ it1->first ] = it1->second;
					}
				}
				else
				{
					xRes->writable()[ it1->first ] = it1->second;
				}
			}
			break;
			}
		case CompoundObjectTypeId :
			{
			CompoundObjectPtr x0 = boost::static_pointer_cast<CompoundObject>( y0 );
			CompoundObjectPtr x1 = boost::static_pointer_cast<CompoundObject>( y1 );
			CompoundObjectPtr x2 = boost::static_pointer_cast<CompoundObject>( y2 );
			CompoundObjectPtr x3 = boost::static_pointer_cast<CompoundObject>( y3 );
			CompoundObjectPtr xRes = boost::static_pointer_cast<CompoundObject>( result );
			for ( CompoundObject::ObjectMap::const_iterator it1 = x1->members().begin(); it1 != x1->members().end(); it1++)
			{
				CompoundObject::ObjectMap::const_iterator it0 = x0->members().find( it1->first );
				if ( it0 != x0->members().end() && it0->second->typeId() == it1->second->typeId() )
				{
					CompoundObject::ObjectMap::const_iterator it2 = x2->members().find( it1->first );
					if ( it2 != x2->members().end() && it0->second->typeId() == it2->second->typeId() )
					{
						CompoundObject::ObjectMap::const_iterator it3 = x3->members().find( it1->first );
						if ( it3 != x3->members().end() && it0->second->typeId() == it3->second->typeId() )
						{
							ObjectPtr resultObj = Object::create( it1->second->typeId() );
							if ( ObjectInterpolator< Functor >( it0->second, it1->second, it2->second, it3->second, x, resultObj ) )
							{
								xRes->members()[ it1->first ] = resultObj;
							}
							else
							{
								xRes->members()[ it1->first ] = it1->second;
							}
						}
						else
						{
							xRes->members()[ it1->first ] = it1->second;
						}
					}
					else
					{
						xRes->members()[ it1->first ] = it1->second;
					}
				}
				else
				{
					xRes->members()[ it1->first ] = it1->second;
				}
			}
			break;
			}

		case BoolDataTypeId :
		case IntDataTypeId :
		case LongDataTypeId :
		case UIntDataTypeId :
		case CharDataTypeId :
		case UCharDataTypeId :
		case ShortDataTypeId :
		case UShortDataTypeId :
		case V2iDataTypeId :
		case V3iDataTypeId :
		case Box2iDataTypeId :
		case Box3iDataTypeId :
		case IntVectorDataTypeId :
		case UIntVectorDataTypeId :
		case CharVectorDataTypeId :
		case UCharVectorDataTypeId :
		case LongVectorDataTypeId :
			/// discrete numeric values are not interpolated.
			return false;

		case StringDataTypeId :
		case StringVectorDataTypeId :
			/// non-numeric values are not interpolated
			return false;

		default :

			// unknown types.			
			return false;
	}
	return true;
}

} // namespace IECore

#endif // IE_CORE_OBJECTINTERPOLATOR_H
