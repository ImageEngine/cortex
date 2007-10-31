#include <boost/format.hpp>

#include "IECore/DataCastOp.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/Object.h"
#include "IECore/NullObject.h"
//#include "IECore/DataTraits.h"

#include <cassert>

using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

DataCastOp::DataCastOp()
	:	Op(
		staticTypeName(),
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
static intrusive_ptr< T > 
castToData( DataPtr array )
{
	intrusive_ptr< S > dataArray = static_pointer_cast< S >( array );
	const typename S::BaseType *source = dataArray->baseReadable();
	unsigned sourceSize = dataArray->baseSize();

	if ( sourceSize % ( sizeof( typename T::ValueType ) / sizeof( typename T::BaseType ) ) )
	{
		throw Exception( "Size mismatch on cast operation!" );
	}

	intrusive_ptr< T > resultT = new T;

	typename T::BaseType *target = resultT->baseWritable();

	std::transform( source, source + sourceSize, target, CastRawData< typename S::BaseType, typename T::BaseType >() );
	return resultT;
}

template< typename S, typename T >
static intrusive_ptr< T > 
castToVectorData( DataPtr array )
{
	intrusive_ptr< S > dataArray = static_pointer_cast< S >( array );
	const typename S::BaseType *source = dataArray->baseReadable();
	unsigned sourceSize = dataArray->baseSize();
	unsigned targetItemSize = ( sizeof( typename T::ValueType::value_type ) / sizeof( typename T::BaseType ) );

	if ( sourceSize % targetItemSize )
	{
		throw Exception( "Size mismatch on cast operation!" );
	}

	intrusive_ptr< T > resultT = new T;
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


ObjectPtr DataCastOp::doOperation( ConstCompoundObjectPtr operands )
{
	const TypeId targetType = (TypeId) m_targetTypeParameter->getNumericValue();

	DataPtr data = static_pointer_cast< Data >( m_objectParameter->getValue() );

	if ( data->typeId() == targetType )
	{
		return data;
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
				CASTVECTORDATA( Float, FloatVector )
				default: 	break;
			}
			break;
		case DoubleDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Double, Float )
				CASTVECTORDATA( Double, DoubleVector )
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
		case IntDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Int, UInt )
				CASTDATA( Int, Long )
				default:	break;
			}
			break;
		case LongDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Long, Int )
				default:	break;
			}
			break;
		case UIntDataTypeId:
			switch ( targetType )
			{
				CASTDATA( UInt, Int )
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
				default:	break;
			}
			break;
		case QuatfDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Quatf, Quatd )
				CASTVECTORDATA( Quatf, FloatVector )
				default:	break;
			}
			break;
		case QuatdDataTypeId:
			switch ( targetType )
			{
				CASTDATA( Quatd, Quatf )
				CASTVECTORDATA( Quatd, DoubleVector )
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
		case IntVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( IntVector, Int )
				CASTDATA( IntVector, V2i )
				CASTDATA( IntVector, V3i )
				CASTDATA( IntVector, Box2i )
				CASTDATA( IntVector, Box3i )
				CASTVECTORDATA( IntVector, UIntVector )
				default:	break;
			}
			break;
		case UIntVectorDataTypeId:
			switch ( targetType )
			{
				CASTDATA( UIntVector, UInt )
				CASTVECTORDATA( UIntVector, IntVector )
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
