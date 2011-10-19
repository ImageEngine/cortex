//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"

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

void ToArnoldConverter::setParameter( AtNode *node, const char *name, int parameterType, const IECore::Data *value )
{
	switch( parameterType )
	{
		case AI_TYPE_INT :
		{
			if( const IntData *data = runTimeCast<const IntData>( value ) )
			{
				AiNodeSetInt( node, name, data->readable() );
			}
			break;
		}
		case AI_TYPE_FLOAT :
		{
			if( const FloatData *data = runTimeCast<const FloatData>( value ) )
			{
				AiNodeSetFlt( node, name, data->readable() );
			}
			break;
		}
		case AI_TYPE_STRING :
		{
			if( const StringData *data = runTimeCast<const StringData>( value ) )
			{
				AiNodeSetStr( node, name, data->readable().c_str() );
			}
			break;
		}
		case AI_TYPE_RGB :
		{
			if( const Color3fData *data = runTimeCast<const Color3fData>( value ) )
			{
				const Imath::Color3f &c = data->readable();
				AiNodeSetRGB( node, name, c[0], c[1], c[2] );
			}
			break;
		}		
	}
}

void ToArnoldConverter::setParameter( AtNode *node, const AtParamEntry *parameter, const IECore::Data *value )
{
	setParameter( node, AiParamGetName( parameter ), AiParamGetType( parameter ), value );
}

void ToArnoldConverter::setParameter( AtNode *node, const char *name, const IECore::Data *value )
{
	const AtParamEntry *parameter = AiNodeEntryLookUpParameter( node->base_node, name );
	if( parameter )
	{
		setParameter( node, parameter, value );
	}
	else
	{
		int parameterType = AI_TYPE_NONE;
		
		switch( value->typeId() )
		{
			case IntDataTypeId :
				AiNodeDeclare( node, name, "constant INT" );
				parameterType = AI_TYPE_INT;
				break;
			case FloatDataTypeId :
				AiNodeDeclare( node, name, "constant FLOAT" );
				parameterType = AI_TYPE_FLOAT;
				break;
			case StringDataTypeId :
				AiNodeDeclare( node, name, "constant STRING" );
				parameterType = AI_TYPE_STRING;
				break;
			case Color3fDataTypeId :
				AiNodeDeclare( node, name, "constant RGB" );
				parameterType = AI_TYPE_RGB;
				break;	
			default :
				break;
		}
		
		if( parameterType != AI_TYPE_NONE )
		{
			setParameter( node, name, parameterType, value );
		}
	}
}

void ToArnoldConverter::setParameters( AtNode *node, const IECore::CompoundDataMap &values )
{
	for( CompoundDataMap::const_iterator it=values.begin(); it!=values.end(); it++ )
	{
		setParameter( node, it->first.value().c_str(), it->second );
	}
}

IECore::DataPtr ToArnoldConverter::getParameter( AtNode *node, const char *name, int parameterType )
{
	switch( parameterType )
	{
		case AI_TYPE_INT :
			return new IntData( AiNodeGetInt( node, name ) );
		case AI_TYPE_FLOAT :
			return new FloatData( AiNodeGetFlt( node, name ) );
		case AI_TYPE_STRING :
			return new StringData( AiNodeGetStr( node, name ) );
	}
	return 0;
}

IECore::DataPtr ToArnoldConverter::getParameter( AtNode *node, const AtParamEntry *parameter )
{
	return getParameter( node, AiParamGetName( parameter ), AiParamGetType( parameter ) );
}

IECore::DataPtr ToArnoldConverter::getParameter( AtNode *node, const AtUserParamEntry *parameter )
{
	return getParameter( node, AiUserParamGetName( parameter ), AiUserParamGetType( parameter ) );
}
		
IECore::DataPtr ToArnoldConverter::getParameter( AtNode *node, const char *name )
{
	const AtParamEntry *parameter = AiNodeEntryLookUpParameter( node->base_node, name );
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
	/// \todo Error handling and non-user parameters

	AtUserParamIterator *it = AiNodeGetUserParamIterator( node );  	
	while( const AtUserParamEntry *param = AiUserParamIteratorGetNext( it ) )
	{
		values[AiUserParamGetName( param )] = getParameter( node, param );
	}
	AiUserParamIteratorDestroy( it );
}
