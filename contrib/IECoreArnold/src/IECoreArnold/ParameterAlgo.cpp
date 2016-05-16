//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

#include "boost/interprocess/smart_ptr/unique_ptr.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/DespatchTypedData.h"

#include "IECoreArnold/ParameterAlgo.h"

using namespace std;
using namespace IECore;
using namespace IECoreArnold;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

typedef boost::interprocess::unique_ptr<AtArray, void (*)( AtArray *)> ArrayPtr;

template<typename T>
inline const T *dataCast( const char *name, const IECore::Data *data )
{
	const T *result = runTimeCast<const T>( data );
	if( result )
	{
		return result;
	}
	msg( Msg::Warning, "setParameter", boost::format( "Unsupported value type \"%s\" for parameter \"%s\" (expected %s)." ) % data->typeName() % name % T::staticTypeName() );
	return NULL;
}

void setParameterInternal( AtNode *node, const char *name, int parameterType, bool array, const IECore::Data *value )
{
	if( array )
	{
		AtArray *a = ParameterAlgo::dataToArray( value );
		if( !a )
		{
			msg( Msg::Warning, "setParameter", boost::format( "Unable to create array from data of type \"%s\" for parameter \"%s\"" ) % value->typeName() % name );
			return;
		}
		if( a->type != parameterType )
		{
			msg( Msg::Warning, "setParameter", boost::format( "Unable to create array of type %s from data of type \"%s\" for parameter \"%s\"" ) % AiParamGetTypeName( parameterType ) % value->typeName() % name );
			return;
		}
		AiNodeSetArray( node, name, a );
	}
	else
	{
		switch( parameterType )
		{
			case AI_TYPE_INT :
				if( const IntData *data = dataCast<IntData>( name, value ) )
				{
					AiNodeSetInt( node, name, data->readable() );
				}
				break;
			case AI_TYPE_BYTE :
				if( const IntData *data = dataCast<IntData>( name, value ) )
				{
					AiNodeSetByte( node, name, data->readable() );
				}
				break;
			case AI_TYPE_FLOAT :
				if( const FloatData *data = dataCast<FloatData>( name, value ) )
				{
					AiNodeSetFlt( node, name, data->readable() );
				}
				break;
			case AI_TYPE_STRING :
				if( const StringData *data = dataCast<StringData>( name, value ) )
				{
					AiNodeSetStr( node, name, data->readable().c_str() );
				}
				break;
			case AI_TYPE_RGB :
				if( const Color3fData *data = dataCast<Color3fData>( name, value ) )
				{
					const Imath::Color3f &c = data->readable();
					AiNodeSetRGB( node, name, c[0], c[1], c[2] );
				}
				break;
			case AI_TYPE_RGBA :
				if( const Color4fData *data = dataCast<Color4fData>( name, value ) )
				{
					const Imath::Color4f &c = data->readable();
					AiNodeSetRGBA( node, name, c[0], c[1], c[2], c[3] );
				}
				break;
			case AI_TYPE_ENUM :
				if( const StringData *data = dataCast<StringData>( name, value ) )
				{
					AiNodeSetStr( node, name, data->readable().c_str() );
				}
				break;
			case AI_TYPE_BOOLEAN :
				if( const BoolData *data = dataCast<BoolData>( name, value ) )
				{
					AiNodeSetBool( node, name, data->readable() );
				}
				break;
			case AI_TYPE_POINT2 :
				if( const V2fData *data = dataCast<V2fData>( name, value ) )
				{
					const Imath::V2f &v = data->readable();
					AiNodeSetPnt2( node, name, v.x, v.y );
				}
				break;
			default :
			{
				msg( Msg::Warning, "setParameter", boost::format( "Arnold parameter \"%s\" has unsupported type \"%s\"." ) % name % AiParamGetTypeName( parameterType ) );
			}
		}
	}
}

template<typename T, typename F>
IECore::DataPtr arrayToDataInternal( AtArray *array, F f )
{
	typedef vector<T> VectorType;
	typedef IECore::TypedData<vector<T> > DataType;
	typename DataType::Ptr data = new DataType;
	VectorType &v = data->writable();

	v.reserve( array->nelements );
	for( size_t i = 0; i < array->nelements; ++i )
	{
		v.push_back( f( array, i, __AI_FILE__, __AI_LINE__ ) );
	}

	return data;
}

/// \todo Flesh this out to support more types and then
/// consider exposing it in the public API.
IECore::DataPtr arrayToData( AtArray *array )
{
	if( array->nkeys > 1 )
	{
		/// \todo Decide how to deal with more
		/// than one key - is it more useful to return multiple Data
		/// objects or to put it all in one?
		return NULL;
	}

	switch( array->type )
	{
		case AI_TYPE_BOOLEAN :
			return arrayToDataInternal<bool>( array, AiArrayGetBoolFunc );
		case AI_TYPE_INT :
			return arrayToDataInternal<int>( array, AiArrayGetIntFunc );
		case AI_TYPE_FLOAT :
			return arrayToDataInternal<float>( array, AiArrayGetFltFunc );
		case AI_TYPE_STRING :
			return arrayToDataInternal<string>( array, AiArrayGetStrFunc );
		default :
			return NULL;
	}
}

IECore::DataPtr getParameterInternal( AtNode *node, const char *name, int parameterType )
{
	switch( parameterType )
	{
		case AI_TYPE_BOOLEAN :
			return new BoolData( AiNodeGetBool( node, name ) );
		case AI_TYPE_INT :
			return new IntData( AiNodeGetInt( node, name ) );
		case AI_TYPE_FLOAT :
			return new FloatData( AiNodeGetFlt( node, name ) );
		case AI_TYPE_STRING :
			return new StringData( AiNodeGetStr( node, name ) );
		case AI_TYPE_ARRAY :
			return arrayToData( AiNodeGetArray( node, name ) );
	}
	return NULL;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementation of public API.
//////////////////////////////////////////////////////////////////////////

namespace IECoreArnold
{

namespace ParameterAlgo
{

void setParameter( AtNode *node, const AtParamEntry *parameter, const IECore::Data *value )
{
	bool isArray = false;
	int type = AiParamGetType( parameter );
	if( type == AI_TYPE_ARRAY )
	{
		type = AiParamGetDefault( parameter )->ARRAY->type;
		isArray = true;
	}

	setParameterInternal( node, AiParamGetName( parameter ), type, isArray, value );
}

void setParameter( AtNode *node, const char *name, const IECore::Data *value )
{
	const AtParamEntry *parameter = AiNodeEntryLookUpParameter( AiNodeGetNodeEntry( node ), name );
	if( parameter )
	{
		setParameter( node, parameter, value );
	}
	else
	{
		bool array = false;
		int type = parameterType( value->typeId(), array );
		if( type != AI_TYPE_NONE )
		{
			std::string typeString = "constant ";
			if( array )
			{
				typeString += "ARRAY ";
			}
			typeString += AiParamGetTypeName( type );
			AiNodeDeclare( node, name, typeString.c_str() );
			setParameterInternal( node, name, type, array, value );
		}
		else
		{
			msg(
				Msg::Warning,
				"setParameter",
				boost::format( "Unsupported data type \"%s\" for name \"%s\"" ) % value->typeName() % name
			);
		}
	}
}

void setParameters( AtNode *node, const IECore::CompoundDataMap &values )
{
	for( CompoundDataMap::const_iterator it=values.begin(); it!=values.end(); it++ )
	{
		setParameter( node, it->first.value().c_str(), it->second.get() );
	}
}

IECore::DataPtr getParameter( AtNode *node, const AtParamEntry *parameter )
{
	return getParameterInternal( node, AiParamGetName( parameter ), AiParamGetType( parameter ) );
}

IECore::DataPtr getParameter( AtNode *node, const AtUserParamEntry *parameter )
{
	return getParameterInternal( node, AiUserParamGetName( parameter ), AiUserParamGetType( parameter ) );
}

IECore::DataPtr getParameter( AtNode *node, const char *name )
{
	const AtParamEntry *parameter = AiNodeEntryLookUpParameter( AiNodeGetNodeEntry( node ), name );
	if( parameter )
	{
		return getParameter( node, parameter );
	}
	else
	{
		const AtUserParamEntry *userParameter = AiNodeLookUpUserParameter( node, name );
		if( userParameter )
		{
			return getParameter( node, userParameter );
		}
	}

	return NULL;
}

void getParameters( AtNode *node, IECore::CompoundDataMap &values )
{
	/// \todo Non-user parameters

	AtUserParamIterator *it = AiNodeGetUserParamIterator( node );
	while( const AtUserParamEntry *param = AiUserParamIteratorGetNext( it ) )
	{
		DataPtr d = getParameter( node, param );
		if( d )
		{
			values[AiUserParamGetName( param )] = d;
		}
		else
		{
			msg(
				Msg::Warning,
				"getParameters",
				boost::format( "Unable to convert user parameter \"%s\"" ) % AiUserParamGetName( param )
			);
		}
	}
	AiUserParamIteratorDestroy( it );
}

int parameterType( IECore::TypeId dataType, bool &array )
{
	switch( dataType )
	{
		// non-array types

		case IntDataTypeId :
			array = false;
			return AI_TYPE_INT;
		case FloatDataTypeId :
			array = false;
			return AI_TYPE_FLOAT;
		case StringDataTypeId :
			array = false;
			return AI_TYPE_STRING;
		case Color3fDataTypeId :
			array = false;
			return AI_TYPE_RGB;
		case BoolDataTypeId :
			array = false;
			return AI_TYPE_BOOLEAN;

		// array types

		case IntVectorDataTypeId :
			array = true;
			return AI_TYPE_INT;
		case FloatVectorDataTypeId :
			array = true;
			return AI_TYPE_FLOAT;
		case StringVectorDataTypeId :
			array = true;
			return AI_TYPE_STRING;
		case Color3fVectorDataTypeId :
			array = true;
			return AI_TYPE_RGB;
		case BoolVectorDataTypeId :
			array = true;
			return AI_TYPE_BOOLEAN;
		case V3fVectorDataTypeId :
			array = true;
			return AI_TYPE_VECTOR;
		default :
			return AI_TYPE_NONE;
	}
}

AtArray *dataToArray( const IECore::Data *data, int aiType )
{
	if( aiType == AI_TYPE_NONE )
	{
		bool isArray = false;
		aiType = parameterType( data->typeId(), isArray );
		if( aiType == AI_TYPE_NONE || !isArray )
		{
			return NULL;
		}
	}

	const void *dataAddress = despatchTypedData<TypedDataAddress, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( data ) );
	size_t dataSize = despatchTypedData<TypedDataSize, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( data ) );
	return AiArrayConvert( dataSize, 1, aiType, dataAddress );
}

IECOREARNOLD_API AtArray *dataToArray( const std::vector<const IECore::Data *> &samples, int aiType )
{
	if( aiType == AI_TYPE_NONE )
	{
		bool isArray = false;
		aiType = parameterType( samples.front()->typeId(), isArray );
		if( aiType == AI_TYPE_NONE || !isArray )
		{
			return NULL;
		}
	}

	size_t arraySize = despatchTypedData<TypedDataSize, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( samples.front() ) );
	ArrayPtr array(
		AiArrayAllocate( arraySize, samples.size(), aiType ),
		AiArrayDestroy
	);

	for( vector<const IECore::Data *>::const_iterator it = samples.begin(), eIt = samples.end(); it != eIt; ++it )
	{
		if( (*it)->typeId() != samples.front()->typeId() )
		{
			throw IECore::Exception( "ParameterAlgo::dataToArray() : Mismatched sample types." );
		}
		const size_t dataSize = despatchTypedData<TypedDataSize, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( *it ) );
		if( dataSize != arraySize )
		{
			throw IECore::Exception( "ParameterAlgo::dataToArray() : Mismatched sample lengths." );
		}
		const void *dataAddress = despatchTypedData<TypedDataAddress, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( *it ) );
		AiArraySetKey( array.get(), /* key = */ it - samples.begin(), dataAddress );
	}

	return array.release();
}

} // namespace ParameterAlgo

} // namespace IECoreArnold
