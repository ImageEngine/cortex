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


#ifndef IE_CORE_OBJECTINTERPOLATOR_H
#define IE_CORE_OBJECTINTERPOLATOR_H

/// Extension module for Interpolator class specialized on ObjectPtr
/// Remember to update both functions in this module (for two and four data points ).

#include "IECore/Object.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/Interpolator.h"
#include "IECore/Exception.h"

namespace IECore
{

/// Utility function that applies linear interpolation on objects. Returns a "null" pointer if the Object cannot be interpolated.
ObjectPtr linearObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, double x );

/// Utility function that applies cosine interpolation on objects. Returns a "null" pointer if the Object cannot be interpolated.
ObjectPtr cosineObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, double x );

/// Utility function that applies cubic interpolation on objects. Returns a "null" pointer if the Object cannot be interpolated.
ObjectPtr cubicObjectInterpolation( const ObjectPtr &y0, const ObjectPtr &y1, const ObjectPtr &y2, const ObjectPtr &y3, double x );

/// Two data points interpolation template function for ObjectPtr.
/// Gets the typeId from y0 and make the proper conversion if the type is continuous. Otherwise returns false.
/// Assumes y1 and result has the same type as y0 parameter.
template<template<typename T> class Functor>
bool ObjectInterpolator( const ObjectPtr &y0, const ObjectPtr & y1, double x, ObjectPtr &result )
{

	if ( y0->typeId() != y1->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}

	switch( y0->typeId() )
	{
		case FloatDataTypeId :
			{
			FloatDataPtr x0 = boost::static_pointer_cast<FloatData>( y0 );
			FloatDataPtr x1 = boost::static_pointer_cast<FloatData>( y1 );
			FloatDataPtr xRes = boost::static_pointer_cast<FloatData>( result );
			Functor<FloatData>()( x0, x1, x, xRes );
			break;
			}
		case DoubleDataTypeId :
			{
			DoubleDataPtr x0 = boost::static_pointer_cast<DoubleData>( y0 );
			DoubleDataPtr x1 = boost::static_pointer_cast<DoubleData>( y1 );
			DoubleDataPtr xRes = boost::static_pointer_cast<DoubleData>( result );
			Functor<DoubleData>()( x0, x1, x, xRes );
			break;
			}
		case V2fDataTypeId :
			{
			V2fDataPtr x0 = boost::static_pointer_cast<V2fData>( y0 );
			V2fDataPtr x1 = boost::static_pointer_cast<V2fData>( y1 );
			V2fDataPtr xRes = boost::static_pointer_cast<V2fData>( result );
			Functor<V2fData>()( x0, x1, x, xRes );
			break;
			}
		case V3fDataTypeId :
			{
			V3fDataPtr x0 = boost::static_pointer_cast<V3fData>( y0 );
			V3fDataPtr x1 = boost::static_pointer_cast<V3fData>( y1 );
			V3fDataPtr xRes = boost::static_pointer_cast<V3fData>( result );
			Functor<V3fData>()( x0, x1, x, xRes );
			break;
			}
		case V2dDataTypeId :
			{
			V2dDataPtr x0 = boost::static_pointer_cast<V2dData>( y0 );
			V2dDataPtr x1 = boost::static_pointer_cast<V2dData>( y1 );
			V2dDataPtr xRes = boost::static_pointer_cast<V2dData>( result );
			Functor<V2dData>()( x0, x1, x, xRes );
			break;
			}
		case V3dDataTypeId :
			{
			V3dDataPtr x0 = boost::static_pointer_cast<V3dData>( y0 );
			V3dDataPtr x1 = boost::static_pointer_cast<V3dData>( y1 );
			V3dDataPtr xRes = boost::static_pointer_cast<V3dData>( result );
			Functor<V3dData>()( x0, x1, x, xRes );
			break;
			}
		case QuatfDataTypeId :
			{
			QuatfDataPtr x0 = boost::static_pointer_cast<QuatfData>( y0 );
			QuatfDataPtr x1 = boost::static_pointer_cast<QuatfData>( y1 );
			QuatfDataPtr xRes = boost::static_pointer_cast<QuatfData>( result );
			Functor<QuatfData>()( x0, x1, x, xRes );
			break;
			}
		case QuatdDataTypeId :
			{
			QuatdDataPtr x0 = boost::static_pointer_cast<QuatdData>( y0 );
			QuatdDataPtr x1 = boost::static_pointer_cast<QuatdData>( y1 );
			QuatdDataPtr xRes = boost::static_pointer_cast<QuatdData>( result );
			Functor<QuatdData>()( x0, x1, x, xRes );
			break;
			}
		case FloatVectorDataTypeId :
			{
			FloatVectorDataPtr x0 = boost::static_pointer_cast<FloatVectorData>( y0 );
			FloatVectorDataPtr x1 = boost::static_pointer_cast<FloatVectorData>( y1 );
			FloatVectorDataPtr xRes = boost::static_pointer_cast<FloatVectorData>( result );
			Functor<FloatVectorData>()( x0, x1, x, xRes );
			break;
			}
		case DoubleVectorDataTypeId :
			{
			DoubleVectorDataPtr x0 = boost::static_pointer_cast<DoubleVectorData>( y0 );
			DoubleVectorDataPtr x1 = boost::static_pointer_cast<DoubleVectorData>( y1 );
			DoubleVectorDataPtr xRes = boost::static_pointer_cast<DoubleVectorData>( result );
			Functor<DoubleVectorData>()( x0, x1, x, xRes );
			break;
			}
		case HalfVectorDataTypeId :
			{
			HalfVectorDataPtr x0 = boost::static_pointer_cast<HalfVectorData>( y0 );
			HalfVectorDataPtr x1 = boost::static_pointer_cast<HalfVectorData>( y1 );
			HalfVectorDataPtr xRes = boost::static_pointer_cast<HalfVectorData>( result );
			Functor<HalfVectorData>()( x0, x1, x, xRes );
			break;
			}
		case V2fVectorDataTypeId :
			{
			V2fVectorDataPtr x0 = boost::static_pointer_cast<V2fVectorData>( y0 );
			V2fVectorDataPtr x1 = boost::static_pointer_cast<V2fVectorData>( y1 );
			V2fVectorDataPtr xRes = boost::static_pointer_cast<V2fVectorData>( result );
			Functor<V2fVectorData>()( x0, x1, x, xRes );
			break;
			}
		case V2dVectorDataTypeId :
			{
			V2dVectorDataPtr x0 = boost::static_pointer_cast<V2dVectorData>( y0 );
			V2dVectorDataPtr x1 = boost::static_pointer_cast<V2dVectorData>( y1 );
			V2dVectorDataPtr xRes = boost::static_pointer_cast<V2dVectorData>( result );
			Functor<V2dVectorData>()( x0, x1, x, xRes );
			break;
			}
		case V3fVectorDataTypeId :
			{
			V3fVectorDataPtr x0 = boost::static_pointer_cast<V3fVectorData>( y0 );
			V3fVectorDataPtr x1 = boost::static_pointer_cast<V3fVectorData>( y1 );
			V3fVectorDataPtr xRes = boost::static_pointer_cast<V3fVectorData>( result );
			Functor<V3fVectorData>()( x0, x1, x, xRes );
			break;
			}
		case V3dVectorDataTypeId :
			{
			V3dVectorDataPtr x0 = boost::static_pointer_cast<V3dVectorData>( y0 );
			V3dVectorDataPtr x1 = boost::static_pointer_cast<V3dVectorData>( y1 );
			V3dVectorDataPtr xRes = boost::static_pointer_cast<V3dVectorData>( result );
			Functor<V3dVectorData>()( x0, x1, x, xRes );
			break;
			}
		case QuatfVectorDataTypeId :
			{
			QuatfVectorDataPtr x0 = boost::static_pointer_cast<QuatfVectorData>( y0 );
			QuatfVectorDataPtr x1 = boost::static_pointer_cast<QuatfVectorData>( y1 );
			QuatfVectorDataPtr xRes = boost::static_pointer_cast<QuatfVectorData>( result );
			Functor<QuatfVectorData>()( x0, x1, x, xRes );
			break;
			}
		case QuatdVectorDataTypeId :
			{
			QuatdVectorDataPtr x0 = boost::static_pointer_cast<QuatdVectorData>( y0 );
			QuatdVectorDataPtr x1 = boost::static_pointer_cast<QuatdVectorData>( y1 );
			QuatdVectorDataPtr xRes = boost::static_pointer_cast<QuatdVectorData>( result );
			Functor<QuatdVectorData>()( x0, x1, x, xRes );
			break;
			}

		case Color3fDataTypeId :
		case Color4fDataTypeId :
		case Color3dDataTypeId :
		case Color4dDataTypeId :
		case Box2fDataTypeId :
		case Box3fDataTypeId :
		case Box2dDataTypeId :
		case Box3dDataTypeId :
		case M33fDataTypeId :
		case M33dDataTypeId :
		case M44fDataTypeId :
		case M44dDataTypeId :
		case Color3fVectorDataTypeId :
		case Color4fVectorDataTypeId :
		case Color3dVectorDataTypeId :
		case Color4dVectorDataTypeId :
		case Box3fVectorDataTypeId :
		case Box3dVectorDataTypeId :
		case Box2fVectorDataTypeId :
		case Box2dVectorDataTypeId :
		case M33fVectorDataTypeId :
		case M33dVectorDataTypeId :
		case M44fVectorDataTypeId :
		case M44dVectorDataTypeId :
			// complex types currently not supported...
			// \todo help your self implementing interpolation for this types.
			return false;

		case BoolDataTypeId :
		case IntDataTypeId :
		case LongDataTypeId :
		case UIntDataTypeId :
		case CharDataTypeId :
		case UCharDataTypeId :
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

/// Four data points interpolation template function for ObjectPtr.
/// Gets the typeId from y0 and make the proper conversion if the type is continuous. Otherwise returns false.
/// Assumes y1, y2, y3 and result has the same type as y0 parameter.
template<template<typename T> class Functor>
bool ObjectInterpolator( const ObjectPtr &y0, const ObjectPtr & y1, const ObjectPtr &y2, const ObjectPtr & y3, double x, ObjectPtr &result )
{

	if ( y0->typeId() != y1->typeId() || y0->typeId() != y2->typeId() || y0->typeId() != y3->typeId() || y0->typeId() != result->typeId() )
	{
		throw( Exception( "Interpolation objects type don't match!" ) );
	}


	switch( y0->typeId() )
	{
		case FloatDataTypeId :
			{
			FloatDataPtr x0 = boost::static_pointer_cast<FloatData>( y0 );
			FloatDataPtr x1 = boost::static_pointer_cast<FloatData>( y1 );
			FloatDataPtr x2 = boost::static_pointer_cast<FloatData>( y2 );
			FloatDataPtr x3 = boost::static_pointer_cast<FloatData>( y3 );
			FloatDataPtr xRes = boost::static_pointer_cast<FloatData>( result );
			Functor<FloatData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case DoubleDataTypeId :
			{
			DoubleDataPtr x0 = boost::static_pointer_cast<DoubleData>( y0 );
			DoubleDataPtr x1 = boost::static_pointer_cast<DoubleData>( y1 );
			DoubleDataPtr x2 = boost::static_pointer_cast<DoubleData>( y2 );
			DoubleDataPtr x3 = boost::static_pointer_cast<DoubleData>( y3 );
			DoubleDataPtr xRes = boost::static_pointer_cast<DoubleData>( result );
			Functor<DoubleData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V2fDataTypeId :
			{
			V2fDataPtr x0 = boost::static_pointer_cast<V2fData>( y0 );
			V2fDataPtr x1 = boost::static_pointer_cast<V2fData>( y1 );
			V2fDataPtr x2 = boost::static_pointer_cast<V2fData>( y2 );
			V2fDataPtr x3 = boost::static_pointer_cast<V2fData>( y3 );
			V2fDataPtr xRes = boost::static_pointer_cast<V2fData>( result );
			Functor<V2fData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V3fDataTypeId :
			{
			V3fDataPtr x0 = boost::static_pointer_cast<V3fData>( y0 );
			V3fDataPtr x1 = boost::static_pointer_cast<V3fData>( y1 );
			V3fDataPtr x2 = boost::static_pointer_cast<V3fData>( y2 );
			V3fDataPtr x3 = boost::static_pointer_cast<V3fData>( y3 );
			V3fDataPtr xRes = boost::static_pointer_cast<V3fData>( result );
			Functor<V3fData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V2dDataTypeId :
			{
			V2dDataPtr x0 = boost::static_pointer_cast<V2dData>( y0 );
			V2dDataPtr x1 = boost::static_pointer_cast<V2dData>( y1 );
			V2dDataPtr x2 = boost::static_pointer_cast<V2dData>( y2 );
			V2dDataPtr x3 = boost::static_pointer_cast<V2dData>( y3 );
			V2dDataPtr xRes = boost::static_pointer_cast<V2dData>( result );
			Functor<V2dData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V3dDataTypeId :
			{
			V3dDataPtr x0 = boost::static_pointer_cast<V3dData>( y0 );
			V3dDataPtr x1 = boost::static_pointer_cast<V3dData>( y1 );
			V3dDataPtr x2 = boost::static_pointer_cast<V3dData>( y2 );
			V3dDataPtr x3 = boost::static_pointer_cast<V3dData>( y3 );
			V3dDataPtr xRes = boost::static_pointer_cast<V3dData>( result );
			Functor<V3dData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case QuatfDataTypeId :
			{
			QuatfDataPtr x0 = boost::static_pointer_cast<QuatfData>( y0 );
			QuatfDataPtr x1 = boost::static_pointer_cast<QuatfData>( y1 );
			QuatfDataPtr x2 = boost::static_pointer_cast<QuatfData>( y2 );
			QuatfDataPtr x3 = boost::static_pointer_cast<QuatfData>( y3 );
			QuatfDataPtr xRes = boost::static_pointer_cast<QuatfData>( result );
			Functor<QuatfData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case QuatdDataTypeId :
			{
			QuatdDataPtr x0 = boost::static_pointer_cast<QuatdData>( y0 );
			QuatdDataPtr x1 = boost::static_pointer_cast<QuatdData>( y1 );
			QuatdDataPtr x2 = boost::static_pointer_cast<QuatdData>( y2 );
			QuatdDataPtr x3 = boost::static_pointer_cast<QuatdData>( y3 );
			QuatdDataPtr xRes = boost::static_pointer_cast<QuatdData>( result );
			Functor<QuatdData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case FloatVectorDataTypeId :
			{
			FloatVectorDataPtr x0 = boost::static_pointer_cast<FloatVectorData>( y0 );
			FloatVectorDataPtr x1 = boost::static_pointer_cast<FloatVectorData>( y1 );
			FloatVectorDataPtr x2 = boost::static_pointer_cast<FloatVectorData>( y2 );
			FloatVectorDataPtr x3 = boost::static_pointer_cast<FloatVectorData>( y3 );
			FloatVectorDataPtr xRes = boost::static_pointer_cast<FloatVectorData>( result );
			Functor<FloatVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case DoubleVectorDataTypeId :
			{
			DoubleVectorDataPtr x0 = boost::static_pointer_cast<DoubleVectorData>( y0 );
			DoubleVectorDataPtr x1 = boost::static_pointer_cast<DoubleVectorData>( y1 );
			DoubleVectorDataPtr x2 = boost::static_pointer_cast<DoubleVectorData>( y2 );
			DoubleVectorDataPtr x3 = boost::static_pointer_cast<DoubleVectorData>( y3 );
			DoubleVectorDataPtr xRes = boost::static_pointer_cast<DoubleVectorData>( result );
			Functor<DoubleVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case HalfVectorDataTypeId :
			{
			HalfVectorDataPtr x0 = boost::static_pointer_cast<HalfVectorData>( y0 );
			HalfVectorDataPtr x1 = boost::static_pointer_cast<HalfVectorData>( y1 );
			HalfVectorDataPtr x2 = boost::static_pointer_cast<HalfVectorData>( y2 );
			HalfVectorDataPtr x3 = boost::static_pointer_cast<HalfVectorData>( y3 );
			HalfVectorDataPtr xRes = boost::static_pointer_cast<HalfVectorData>( result );
			Functor<HalfVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V2fVectorDataTypeId :
			{
			V2fVectorDataPtr x0 = boost::static_pointer_cast<V2fVectorData>( y0 );
			V2fVectorDataPtr x1 = boost::static_pointer_cast<V2fVectorData>( y1 );
			V2fVectorDataPtr x2 = boost::static_pointer_cast<V2fVectorData>( y2 );
			V2fVectorDataPtr x3 = boost::static_pointer_cast<V2fVectorData>( y3 );
			V2fVectorDataPtr xRes = boost::static_pointer_cast<V2fVectorData>( result );
			Functor<V2fVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V2dVectorDataTypeId :
			{
			V2dVectorDataPtr x0 = boost::static_pointer_cast<V2dVectorData>( y0 );
			V2dVectorDataPtr x1 = boost::static_pointer_cast<V2dVectorData>( y1 );
			V2dVectorDataPtr x2 = boost::static_pointer_cast<V2dVectorData>( y2 );
			V2dVectorDataPtr x3 = boost::static_pointer_cast<V2dVectorData>( y3 );
			V2dVectorDataPtr xRes = boost::static_pointer_cast<V2dVectorData>( result );
			Functor<V2dVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V3fVectorDataTypeId :
			{
			V3fVectorDataPtr x0 = boost::static_pointer_cast<V3fVectorData>( y0 );
			V3fVectorDataPtr x1 = boost::static_pointer_cast<V3fVectorData>( y1 );
			V3fVectorDataPtr x2 = boost::static_pointer_cast<V3fVectorData>( y2 );
			V3fVectorDataPtr x3 = boost::static_pointer_cast<V3fVectorData>( y3 );
			V3fVectorDataPtr xRes = boost::static_pointer_cast<V3fVectorData>( result );
			Functor<V3fVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case V3dVectorDataTypeId :
			{
			V3dVectorDataPtr x0 = boost::static_pointer_cast<V3dVectorData>( y0 );
			V3dVectorDataPtr x1 = boost::static_pointer_cast<V3dVectorData>( y1 );
			V3dVectorDataPtr x2 = boost::static_pointer_cast<V3dVectorData>( y2 );
			V3dVectorDataPtr x3 = boost::static_pointer_cast<V3dVectorData>( y3 );
			V3dVectorDataPtr xRes = boost::static_pointer_cast<V3dVectorData>( result );
			Functor<V3dVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case QuatfVectorDataTypeId :
			{
			QuatfVectorDataPtr x0 = boost::static_pointer_cast<QuatfVectorData>( y0 );
			QuatfVectorDataPtr x1 = boost::static_pointer_cast<QuatfVectorData>( y1 );
			QuatfVectorDataPtr x2 = boost::static_pointer_cast<QuatfVectorData>( y2 );
			QuatfVectorDataPtr x3 = boost::static_pointer_cast<QuatfVectorData>( y3 );
			QuatfVectorDataPtr xRes = boost::static_pointer_cast<QuatfVectorData>( result );
			Functor<QuatfVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}
		case QuatdVectorDataTypeId :
			{
			QuatdVectorDataPtr x0 = boost::static_pointer_cast<QuatdVectorData>( y0 );
			QuatdVectorDataPtr x1 = boost::static_pointer_cast<QuatdVectorData>( y1 );
			QuatdVectorDataPtr x2 = boost::static_pointer_cast<QuatdVectorData>( y2 );
			QuatdVectorDataPtr x3 = boost::static_pointer_cast<QuatdVectorData>( y3 );
			QuatdVectorDataPtr xRes = boost::static_pointer_cast<QuatdVectorData>( result );
			Functor<QuatdVectorData>()( x0, x1, x2, x3, x, xRes );
			break;
			}

		case Color3fDataTypeId :
		case Color4fDataTypeId :
		case Color3dDataTypeId :
		case Color4dDataTypeId :
		case Box2fDataTypeId :
		case Box3fDataTypeId :
		case Box2dDataTypeId :
		case Box3dDataTypeId :
		case M33fDataTypeId :
		case M33dDataTypeId :
		case M44fDataTypeId :
		case M44dDataTypeId :
		case Color3fVectorDataTypeId :
		case Color4fVectorDataTypeId :
		case Color3dVectorDataTypeId :
		case Color4dVectorDataTypeId :
		case Box3fVectorDataTypeId :
		case Box3dVectorDataTypeId :
		case Box2fVectorDataTypeId :
		case Box2dVectorDataTypeId :
		case M33fVectorDataTypeId :
		case M33dVectorDataTypeId :
		case M44fVectorDataTypeId :
		case M44dVectorDataTypeId :
			// complex types currently not supported...
			// \todo help your self implementing interpolation for this types.
			return false;

		case BoolDataTypeId :
		case IntDataTypeId :
		case LongDataTypeId :
		case UIntDataTypeId :
		case CharDataTypeId :
		case UCharDataTypeId :
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
