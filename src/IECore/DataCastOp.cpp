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

#include "boost/format.hpp"

#include "IECore/DataCastOp.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/NullObject.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( DataCastOp );

DataCastOp::DataCastOp()
	:	Op(
		"Performs cast conversion on Data types.",
		new ObjectParameter(
			"result",
			"Converted Data object.",
			new NullObject(),
			DataTypeId
		)
	)
{
	m_objectParameter = new ObjectParameter(
		"object",
		"The Data object that will be converted.",
		new NullObject(),
		DataTypeId
	);
	m_targetTypeParameter = new IntParameter(
		"targetType",
		"The target Data typeId.",
		InvalidTypeId,
		0,
		Imath::limits<int>::max()
	);
	parameters()->addParameter( m_objectParameter );
	parameters()->addParameter( m_targetTypeParameter );
}

DataCastOp::~DataCastOp()
{
}

ObjectParameter * DataCastOp::objectParameter()
{
	return m_objectParameter.get();
}

const ObjectParameter * DataCastOp::objectParameter() const
{
	return m_objectParameter.get();
}

IntParameter * DataCastOp::targetTypeParameter()
{
	return m_targetTypeParameter.get();
}

const IntParameter * DataCastOp::targetTypeParameter() const
{
	return m_targetTypeParameter.get();
}

/// A functor suitable for use with stl algorithms such as transform(), allowing
/// the copying of a container of vectors of type T into a container of vectors of type S.
template<typename T, typename S>
struct CastRawData
{
	inline S operator()( const T &v ) const
	{
		return static_cast< S >( v );
	}
};

template< typename S, typename T >
static typename T::Ptr
castToData( const Data *array )
{
	const S * dataArray = static_cast< const S * >( array );
	const typename S::BaseType *source = dataArray->baseReadable();
	unsigned sourceSize = dataArray->baseSize();

	if ( sourceSize % ( sizeof( typename T::ValueType ) / sizeof( typename T::BaseType ) ) )
	{
		throw Exception( "Size mismatch on cast operation!" );
	}

	typename T::Ptr resultT = new T;

	typename T::BaseType *target = resultT->baseWritable();

	std::transform( source, source + sourceSize, target, CastRawData< typename S::BaseType, typename T::BaseType >() );
	return resultT;
}

template< typename S, typename T >
static typename T::Ptr
castToVectorData( Data * array )
{
	const S *dataArray = static_cast< const S * >( array );
	const typename S::BaseType *source = dataArray->baseReadable();
	unsigned sourceSize = dataArray->baseSize();
	unsigned targetItemSize = ( sizeof( typename T::ValueType::value_type ) / sizeof( typename T::BaseType ) );

	if ( sourceSize % targetItemSize )
	{
		throw Exception( "Size mismatch on cast operation!" );
	}

	typename T::Ptr resultT = new T;
	resultT->writable().resize( sourceSize / targetItemSize );
	typename T::BaseType *target = resultT->baseWritable();
	std::transform( source, source + sourceSize, target, CastRawData< typename S::BaseType, typename T::BaseType >() );
	return resultT;
}

#define CASTDATA( SOURCE, TARGET )													\
			case TARGET ## DataTypeId:												\
				return castToData< SOURCE ## Data, TARGET ## Data >( data );		\

#define CASTVECTORDATA( SOURCE, TARGET )											\
			case TARGET ## DataTypeId:												\
				return castToVectorData< SOURCE ## Data, TARGET ## Data >( data );	\


ObjectPtr DataCastOp::doOperation( const CompoundObject * operands )
{
	const TypeId targetType = (TypeId) m_targetTypeParameter->getNumericValue();

	Data *data = static_cast<Data *>( m_objectParameter->getValue() );

	if ( data->typeId() == targetType )
	{
		return data->copy();
	}

	ObjectPtr result = 0;
	switch( data->typeId() )
	{
		case BoolDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Bool, Char )
				CASTDATA( Bool, Int )
				default:	break;
			}
			break;
		case FloatDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Float, Double )
				CASTDATA( Float, Half )
				CASTVECTORDATA( Float, FloatVector )
				default: 	break;
			}
			break;
		case DoubleDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Double, Float )
				CASTDATA( Double, Half )
				CASTVECTORDATA( Double, DoubleVector )
				default:	break;
			}
			break;
		case HalfDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Half, Float )
				CASTDATA( Half, Double )
				CASTVECTORDATA( Half, HalfVector )
				default:	break;
			}
			break;
		case CharDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Char, UChar )
				default:	break;
			}
			break;
		case UCharDataTypeId:
			switch ( targetType )
			{
				CASTDATA( UChar, Char )
				default:	break;
			}
			break;
		case ShortDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Char, UChar )
				default:	break;
			}
			break;
		case UShortDataTypeId:
			switch ( targetType )
			{
				CASTDATA( UShort, Short )
				default:	break;
			}
			break;
		case Int64DataTypeId:
			switch ( targetType )
			{
				CASTDATA( Int64, UInt64 )
				default:	break;
			}
			break;
		case UInt64DataTypeId:
			switch ( targetType )
			{
				CASTDATA( UInt64, Int64 )
				default:	break;
			}
			break;
		case IntDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Int, UInt )
				CASTDATA( Int, Int64 )
				CASTDATA( Int, UInt64 )
				default:	break;
			}
			break;
		case UIntDataTypeId:
			switch ( targetType )
			{
				CASTDATA( UInt, Int )
				CASTDATA( UInt, Int64 )
				CASTDATA( UInt, UInt64 )
				default:	break;
			}
			break;
		case V2fDataTypeId:
			switch ( targetType )
			{
				CASTVECTORDATA( V2f, FloatVector )
				CASTDATA( V2f, V2d )
				default:	break;
			}
			break;
		case V3fDataTypeId:
			switch ( targetType )
			{
				CASTVECTORDATA( V3f, FloatVector )
				CASTDATA( V3f, V3d )
				CASTDATA( V3f, Color3f )
				default:	break;
			}
			break;
		case V2dDataTypeId:
			switch ( targetType )
			{
				CASTVECTORDATA( V2d, DoubleVector )
				CASTDATA( V2d, V2f )
				default:	break;
			}
			break;
		case V3dDataTypeId:
			switch ( targetType )
			{
				CASTVECTORDATA( V3d, DoubleVector )
				CASTDATA( V3d, V3f )
				CASTDATA( V3d, Color3d )
				default:	break;
			}
			break;
		case Color3fDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Color3f, Color3d )
				CASTDATA( Color3f, V3f )
				CASTDATA( Color3f, V3d )
				CASTVECTORDATA( Color3f, FloatVector )
				default:	break;
			}
			break;
		case Color3dDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Color3d, Color3f )
				CASTDATA( Color3d, V3d )
				CASTVECTORDATA( Color3d, DoubleVector )
				default:	break;
			}
			break;
		case Color4fDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Color4f, Color4d )
				CASTVECTORDATA( Color4f, FloatVector )
				default:	break;
			}
			break;
		case Color4dDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Color4d, Color4f )
				CASTVECTORDATA( Color4d, DoubleVector )
				default:	break;
			}
			break;
		case Box2iDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Box2i, Box2f )
				CASTDATA( Box2i, Box2d )
				CASTVECTORDATA( Box2i, IntVector )
				default:	break;
			}
			break;
		case Box3iDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Box3i, Box3f )
				CASTDATA( Box3i, Box3d )
				CASTVECTORDATA( Box3i, IntVector )
				default:	break;
			}
			break;
		case Box2fDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Box2f, Box2d )
				CASTVECTORDATA( Box2f, FloatVector )
				CASTVECTORDATA( Box2f, V2fVector )
				default:	break;
			}
			break;
		case Box3fDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Box3f, Box3d )
				CASTVECTORDATA( Box3f, FloatVector )
				CASTVECTORDATA( Box3f, V3fVector )
				default:	break;
			}
			break;
		case Box2dDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Box2d, Box2f )
				CASTVECTORDATA( Box2d, DoubleVector )
				CASTVECTORDATA( Box2d, V2dVector )
				default:	break;
			}
			break;
		case Box3dDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Box3d, Box3f )
				CASTVECTORDATA( Box3d, DoubleVector )
				CASTVECTORDATA( Box3d, V3dVector )
				default:	break;
			}
			break;
		case M33fDataTypeId:
			switch ( targetType )
			{
				CASTDATA( M33f, M33d )
				CASTVECTORDATA( M33f, FloatVector )
				default:	break;
			}
			break;
		case M33dDataTypeId:
			switch ( targetType )
			{
				CASTDATA( M33d, M33f )
				CASTVECTORDATA( M33d, DoubleVector )
				default:	break;
			}
			break;

		case M44fDataTypeId:
			switch ( targetType )
			{
				CASTDATA( M44f, M44d )
				CASTVECTORDATA( M44f, FloatVector )
				default:	break;
			}
			break;
		case M44dDataTypeId:
			switch ( targetType )
			{
				CASTDATA( M44d, M44f )
				CASTVECTORDATA( M44d, DoubleVector )
				CASTVECTORDATA( M44d, FloatVector )
				CASTVECTORDATA( M44d, HalfVector )
				default:	break;
			}
			break;
		case QuatfDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Quatf, Quatd )
				CASTVECTORDATA( Quatf, DoubleVector )
				CASTVECTORDATA( Quatf, FloatVector )
				CASTVECTORDATA( Quatf, HalfVector )
				default:	break;
			}
			break;
		case QuatdDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Quatd, Quatf )
				CASTVECTORDATA( Quatd, DoubleVector )
				CASTVECTORDATA( Quatd, FloatVector )
				CASTVECTORDATA( Quatd, HalfVector )
				default:	break;
			}
			break;
		case TransformationMatrixfDataTypeId:
			switch ( targetType )
			{
				CASTDATA( TransformationMatrixf, TransformationMatrixd )
				CASTVECTORDATA( TransformationMatrixf, FloatVector )
				default:	break;
			}
			break;
		case TransformationMatrixdDataTypeId:
			switch ( targetType )
			{
				CASTDATA( TransformationMatrixd, TransformationMatrixf )
				CASTVECTORDATA( TransformationMatrixd, DoubleVector )
				default:	break;
			}
			break;


		// Vectors

		case CharVectorDataTypeId:
			switch ( targetType )
			{
				CASTVECTORDATA( CharVector, UCharVector )
				CASTVECTORDATA( CharVector, IntVector )
				CASTVECTORDATA( CharVector, FloatVector )
				default:	break;
			}
			break;
		case UCharVectorDataTypeId:
			switch ( targetType )
			{
				CASTVECTORDATA( UCharVector, CharVector )
				CASTVECTORDATA( UCharVector, IntVector )
				CASTVECTORDATA( UCharVector, FloatVector )
				default:	break;
			}
			break;
		case FloatVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( FloatVector, Float )
				CASTDATA( FloatVector, V2f )
				CASTDATA( FloatVector, V3f )
				CASTDATA( FloatVector, Quatf )
				CASTDATA( FloatVector, M33f )
				CASTDATA( FloatVector, M44f )
				CASTDATA( FloatVector, Box2f )
				CASTDATA( FloatVector, Box3f )
				CASTDATA( FloatVector, TransformationMatrixf )
				CASTVECTORDATA( FloatVector, IntVector )
				CASTVECTORDATA( FloatVector, HalfVector )
				CASTVECTORDATA( FloatVector, DoubleVector )
				CASTVECTORDATA( FloatVector, V2fVector )
				CASTVECTORDATA( FloatVector, V3fVector )
				CASTVECTORDATA( FloatVector, V2dVector )
				CASTVECTORDATA( FloatVector, V3dVector )
				CASTVECTORDATA( FloatVector, QuatfVector )
				CASTVECTORDATA( FloatVector, M33fVector )
				CASTVECTORDATA( FloatVector, M44fVector )
				CASTVECTORDATA( FloatVector, Box2fVector )
				CASTVECTORDATA( FloatVector, Box3fVector )
				default:	break;
			}
			break;
		case DoubleVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( DoubleVector, Double )
				CASTDATA( DoubleVector, V2d )
				CASTDATA( DoubleVector, V3d )
				CASTDATA( DoubleVector, Quatd )
				CASTDATA( DoubleVector, M33d )
				CASTDATA( DoubleVector, M44d )
				CASTDATA( DoubleVector, Box2d )
				CASTDATA( DoubleVector, Box3d )
				CASTDATA( DoubleVector, TransformationMatrixd )
				CASTVECTORDATA( DoubleVector, IntVector )
				CASTVECTORDATA( DoubleVector, HalfVector )
				CASTVECTORDATA( DoubleVector, FloatVector )
				CASTVECTORDATA( DoubleVector, V2fVector )
				CASTVECTORDATA( DoubleVector, V3fVector )
				CASTVECTORDATA( DoubleVector, V2dVector )
				CASTVECTORDATA( DoubleVector, V3dVector )
				CASTVECTORDATA( DoubleVector, QuatdVector )
				CASTVECTORDATA( DoubleVector, M33dVector )
				CASTVECTORDATA( DoubleVector, M44dVector )
				CASTVECTORDATA( DoubleVector, Box2dVector )
				CASTVECTORDATA( DoubleVector, Box3dVector )
				default:	break;
			}
		case HalfVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( HalfVector, Half )
				CASTVECTORDATA( HalfVector, IntVector )
				CASTVECTORDATA( HalfVector, HalfVector )
				CASTVECTORDATA( HalfVector, FloatVector )
				CASTVECTORDATA( HalfVector, V2fVector )
				CASTVECTORDATA( HalfVector, V3fVector )
				CASTVECTORDATA( HalfVector, V2dVector )
				CASTVECTORDATA( HalfVector, V3dVector )
				CASTVECTORDATA( HalfVector, QuatdVector )
				CASTVECTORDATA( HalfVector, M33dVector )
				CASTVECTORDATA( HalfVector, M44dVector )
				CASTVECTORDATA( HalfVector, Box2dVector )
				CASTVECTORDATA( HalfVector, Box3dVector )
				default:	break;
			}
			break;
		case IntVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( IntVector, Int )
				CASTDATA( IntVector, V2i )
				CASTDATA( IntVector, V3i )
				CASTDATA( IntVector, Box2i )
				CASTDATA( IntVector, Box3i )
				CASTVECTORDATA( IntVector, UIntVector )
				CASTVECTORDATA( IntVector, Int64Vector )
				CASTVECTORDATA( IntVector, UInt64Vector )
				default:	break;
			}
			break;
		case UIntVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( UIntVector, UInt )
				CASTVECTORDATA( UIntVector, IntVector )
				CASTVECTORDATA( UIntVector, Int64Vector )
				CASTVECTORDATA( UIntVector, UInt64Vector )
				default:	break;
			}
			break;
		case Int64VectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Int64Vector, Int64 )
				CASTVECTORDATA( Int64Vector, UInt64Vector )
				default:	break;
			}
			break;
		case UInt64VectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( UInt64Vector, UInt64 )
				CASTVECTORDATA( UInt64Vector, Int64Vector )
				default:	break;
			}
			break;
		case V2fVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( V2fVector, V2f )
				CASTDATA( V2fVector, Box2f )
				CASTVECTORDATA( V2fVector, FloatVector )
				CASTVECTORDATA( V2fVector, V2dVector )
				CASTVECTORDATA( V2fVector, Box2fVector )
				default:	break;
			}
			break;
		case V2dVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( V2dVector, V2d )
				CASTDATA( V2dVector, Box2d )
				CASTVECTORDATA( V2dVector, DoubleVector )
				CASTVECTORDATA( V2dVector, V2fVector )
				CASTVECTORDATA( V2dVector, Box2dVector )
				default:	break;
			}
			break;
		case V3fVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( V3fVector, V3f )
				CASTDATA( V3fVector, Box3f )
				CASTVECTORDATA( V3fVector, FloatVector )
				CASTVECTORDATA( V3fVector, V3dVector )
				CASTVECTORDATA( V3fVector, Color3fVector )
				CASTVECTORDATA( V3fVector, Color3dVector )
				CASTVECTORDATA( V3fVector, Box3fVector )
				default:	break;
			}
			break;
		case V3dVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( V3dVector, V3d )
				CASTDATA( V3dVector, Box3d )
				CASTVECTORDATA( V3dVector, DoubleVector )
				CASTVECTORDATA( V3dVector, V3fVector )
				CASTVECTORDATA( V3dVector, Color3fVector )
				CASTVECTORDATA( V3dVector, Color3dVector )
				CASTVECTORDATA( V3dVector, Box3dVector )
				default:	break;
			}
			break;
		case Color3fVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( Color3fVector, V3f )
				CASTDATA( Color3fVector, Box3f )
				CASTVECTORDATA( Color3fVector, FloatVector )
				CASTVECTORDATA( Color3fVector, V3fVector )
				CASTVECTORDATA( Color3fVector, V3dVector )
				CASTVECTORDATA( Color3fVector, Color3dVector )
				CASTVECTORDATA( Color3fVector, Box3fVector )
				default:	break;
			}
			break;	
		case Box3fVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( Box3fVector, Box3f )
				CASTVECTORDATA( Box3fVector, FloatVector )
				CASTVECTORDATA( Box3fVector, V3fVector )
				CASTVECTORDATA( Box3fVector, Box3dVector )
				default:	break;
			}
			break;
		case Box3dVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( Box3dVector, Box3d )
				CASTVECTORDATA( Box3dVector, DoubleVector )
				CASTVECTORDATA( Box3dVector, V3dVector )
				CASTVECTORDATA( Box3dVector, Box3fVector )
				default:	break;
			}
			break;
		case M33fVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( M33fVector, M33f )
				CASTVECTORDATA( M33fVector, FloatVector )
				CASTVECTORDATA( M33fVector, M33dVector )
				default:	break;
			}
			break;
		case M33dVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( M33dVector, M33d )
				CASTVECTORDATA( M33dVector, DoubleVector )
				CASTVECTORDATA( M33dVector, M33fVector )
				default:	break;
			}
			break;
		case M44fVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( M44fVector, M44f )
				CASTVECTORDATA( M44fVector, FloatVector )
				CASTVECTORDATA( M44fVector, M44dVector )
				default:	break;
			}
			break;
		case M44dVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( M44dVector, M44d )
				CASTVECTORDATA( M44dVector, DoubleVector )
				CASTVECTORDATA( M44dVector, M44fVector )
				default:	break;
			}
			break;
		case QuatfVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( QuatfVector, Quatf )
				CASTVECTORDATA( QuatfVector, FloatVector )
				CASTVECTORDATA( QuatfVector, QuatdVector )
				default:	break;
			}
			break;
		case QuatdVectorDataTypeId :
			switch ( targetType )
			{
				CASTDATA( QuatdVector, Quatd )
				CASTVECTORDATA( QuatdVector, DoubleVector )
				CASTVECTORDATA( QuatdVector, QuatfVector )
				default:	break;
			}
			break;
		default:
			break;
	}

	string targetTypeName = Object::typeNameFromTypeId( targetType );
	if ( targetTypeName == "" )
		targetTypeName = ( format( "%d" ) % targetType ).str();

	throw Exception( string("Don't know how to convert from type ") + data->typeName() + " to " + targetTypeName );
}
