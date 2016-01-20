//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

// This must come before the Cortex includes, because on OSX headers included
// by TBB define macros which conflict with the inline functions in ai_types.h.
#include "ai.h"

#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"

#include "IECoreArnold/ToArnoldConverter.h"

using namespace IECore;
using namespace IECoreArnold;

IE_CORE_DEFINERUNTIMETYPED( ToArnoldConverter );

ToArnoldConverter::ToArnoldConverter( const std::string &description, IECore::TypeId supportedType )
	:	IECore::FromCoreConverter( description, supportedType )
{
}

ToArnoldConverter::~ToArnoldConverter()
{
}

AtNode *ToArnoldConverter::convert() const
{
	IECore::ConstCompoundObjectPtr operands = parameters()->getTypedValidatedValue<IECore::CompoundObject>();
	return doConversion( srcParameter()->getValidatedValue(), operands );
}

void ToArnoldConverter::setParameter( AtNode *node, const AtParamEntry *parameter, const IECore::Data *value )
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

void ToArnoldConverter::setParameter( AtNode *node, const char *name, const IECore::Data *value )
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
				"ToArnoldConverter::setParameter",
				boost::format( "Unsupported data type \"%s\" for name \"%s\"" ) % value->typeName() % name
			);
		}
	}
}

void ToArnoldConverter::setParameters( AtNode *node, const IECore::CompoundDataMap &values )
{
	for( CompoundDataMap::const_iterator it=values.begin(); it!=values.end(); it++ )
	{
		setParameter( node, it->first.value().c_str(), it->second.get() );
	}
}

IECore::DataPtr ToArnoldConverter::getParameter( AtNode *node, const AtParamEntry *parameter )
{
	return getParameterInternal( node, AiParamGetName( parameter ), AiParamGetType( parameter ) );
}

IECore::DataPtr ToArnoldConverter::getParameter( AtNode *node, const AtUserParamEntry *parameter )
{
	return getParameterInternal( node, AiUserParamGetName( parameter ), AiUserParamGetType( parameter ) );
}

IECore::DataPtr ToArnoldConverter::getParameter( AtNode *node, const char *name )
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

	return 0;
}

void ToArnoldConverter::getParameters( AtNode *node, IECore::CompoundDataMap &values )
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
				"ToArnoldConverter::getParameters",
				boost::format( "Unable to convert user parameter \"%s\"" ) % AiUserParamGetName( param )
			);
		}
	}
	AiUserParamIteratorDestroy( it );
}

int ToArnoldConverter::parameterType( IECore::TypeId dataType, bool &array )
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
		default :
			return AI_TYPE_NONE;
	}
}

AtArray *ToArnoldConverter::dataToArray( const IECore::Data *data )
{
	bool isArray = false;
	int type = parameterType( data->typeId(), isArray );
	if( type == AI_TYPE_NONE || !isArray )
	{
		return 0;
	}

	const void *dataAddress = despatchTypedData<TypedDataAddress, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( data ) );
	size_t dataSize = despatchTypedData<TypedDataSize, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError>( const_cast<Data *>( data ) );
	return AiArrayConvert( dataSize, 1, type, dataAddress );
}

template<typename T>
static inline const T *dataCast( const char *name, const IECore::Data *data )
{
	const T *result = runTimeCast<const T>( data );
	if( result )
	{
		return result;
	}
	msg( Msg::Warning, "ToArnoldConverter::setParameter", boost::format( "Unsupported value type \"%s\" for parameter \"%s\" (expected %s)." ) % data->typeName() % name % T::staticTypeName() );
	return 0;
}

void ToArnoldConverter::setParameterInternal( AtNode *node, const char *name, int parameterType, bool array, const IECore::Data *value )
{
	if( array )
	{
		AtArray *a = dataToArray( value );
		if( !a )
		{
			msg( Msg::Warning, "ToArnoldConverter::setParameter", boost::format( "Unable to create array from data of type \"%s\" for parameter \"%s\"" ) % value->typeName() % name );
			return;
		}
		if( a->type != parameterType )
		{
			msg( Msg::Warning, "ToArnoldConverter::setParameter", boost::format( "Unable to create array of type %s from data of type \"%s\" for parameter \"%s\"" ) % AiParamGetTypeName( parameterType ) % value->typeName() % name );
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
			case AI_TYPE_ENUM :
				if( const StringData *data = dataCast<StringData>( name, value ) )
				{
					AiNodeSetStr( node, name, data->readable().c_str() );
				}
				break;
			case AI_TYPE_BOOLEAN :
			{
				if( const BoolData *data = dataCast<BoolData>( name, value ) )
				{
					AiNodeSetBool( node, name, data->readable() );
				}
				break;
			}
			default :
			{
				msg( Msg::Warning, "ToArnoldConverter::setParameter", boost::format( "Arnold parameter \"%s\" has unsupported type \"%s\"." ) % name % AiParamGetTypeName( parameterType ) );
			}
		}
	}
}

IECore::DataPtr ToArnoldConverter::getParameterInternal( AtNode *node, const char *name, int parameterType )
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
	}
	return 0;
}

ToArnoldConverterPtr ToArnoldConverter::create( IECore::ObjectPtr object )
{
	const CreatorMap &m = creators();
	CreatorMap::const_iterator it = m.find( object->typeId() );
	if( it != m.end() )
	{
		return it->second( object );
	}
	return 0;
}

ToArnoldConverter::CreatorMap &ToArnoldConverter::creators()
{
	static CreatorMap m;
	return m;
}

