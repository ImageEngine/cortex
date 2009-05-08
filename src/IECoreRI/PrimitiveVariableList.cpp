//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/PrimitiveVariableList.h"

#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreRI;
using namespace std;
using namespace boost;

PrimitiveVariableList::PrimitiveVariableList( const IECore::PrimitiveVariableMap &primVars, const std::map<std::string, std::string> *typeHints )
{
	// figure out how many strings we need to deal with so we can reserve
	// enough space for them
	int numStrings = 0;
	PrimitiveVariableMap::const_iterator it;
	for( it=primVars.begin(); it!=primVars.end(); it++ )
	{
		TypeId type = it->second.data->typeId();
		switch( type )
		{
			case StringVectorDataTypeId :
				numStrings += static_pointer_cast<StringVectorData>( it->second.data )->readable().size();
				break;

			case StringDataTypeId :
				numStrings++;
				break;

			default :

				break;

		}
	}

	// reserve the space
	m_strings.reserve( numStrings );
	m_charPtrs.reserve( numStrings );

	// build the tokens and values arrays
	for( it=primVars.begin(); it!=primVars.end(); it++ )
	{
		size_t arraySize = 0;
		const char *t = type( it->first, it->second.data, arraySize, typeHints );
		const char *i = interpolation( it->second.interpolation );
		if( t && i )
		{
			// push the interpolation/type/name string
			if( it->second.interpolation==PrimitiveVariable::Constant && arraySize )
			{
				// when interpolation is constant, we should treat anything with
				// multiple elements to be an array. ideally we'd have a mechanism
				// for specifying arrays with other interpolations, but this requires
				// much more information than we have - we'd need the Primitive::variableSize().
				m_strings.push_back( boost::str( boost::format( "%s %s %s[%d]" ) % i % t % it->first % arraySize ) );
			}
			else
			{
				m_strings.push_back( string( i ) + " " + t + " " + it->first );
			}
			m_tokens.push_back( m_strings.rbegin()->c_str() );
			m_values.push_back( value( it->second.data ) );
		}
	}
}

int PrimitiveVariableList::n()
{
	return m_tokens.size();
}

char **PrimitiveVariableList::tokens()
{
	return (char **)&*(m_tokens.begin());
}

void **PrimitiveVariableList::values()
{
	return (void **)&*(m_values.begin());
}

const char *PrimitiveVariableList::type( const std::string &name, ConstDataPtr d, size_t &arraySize, const std::map<std::string, std::string> *typeHints )
{
	arraySize = 0;
	IECore::TypeId t = d->typeId();
	switch( t )
	{
		case V3fVectorDataTypeId :
			arraySize = static_pointer_cast<const V3fVectorData>( d )->readable().size();
			if( name=="P" || name=="Pref"  )
			{
				return "point";
			}
			else if( name=="N" )
			{
				return "normal";
			}
			else
			{
				if( typeHints )
				{
					map<string, string>::const_iterator it = typeHints->find( name );
					if( it!=typeHints->end() )
					{
						return it->second.c_str();
					}
				}
				return "vector";
			}
		case V3fDataTypeId :
			if( typeHints )
			{
				map<string, string>::const_iterator it = typeHints->find( name );
				if( it!=typeHints->end() )
				{
					return it->second.c_str();
				}
			}
			return "vector";
		case Color3fVectorDataTypeId :
			arraySize = static_pointer_cast<const Color3fVectorData>( d )->readable().size();
		case Color3fDataTypeId :
			return "color";
		case FloatVectorDataTypeId :
			arraySize = static_pointer_cast<const FloatVectorData>( d )->readable().size();
		case FloatDataTypeId :
			return "float";
		case IntVectorDataTypeId :
			arraySize = static_pointer_cast<const IntVectorData>( d )->readable().size();
		case IntDataTypeId :
			return "int";
		case StringVectorDataTypeId :
			arraySize = static_pointer_cast<const StringVectorData>( d )->readable().size();
		case StringDataTypeId :
			return "string";
		default :
			msg( Msg::Warning, "PrimitiveVariableList::type", format( "Variable \"%s\" has unsupported datatype \"%s\"." ) % name % d->typeName() );
			return 0;
	}
}

const char *PrimitiveVariableList::interpolation( IECore::PrimitiveVariable::Interpolation i )
{
	switch( i )
	{
		case PrimitiveVariable::Constant :
			return "constant";
		case PrimitiveVariable::Uniform :
			return "uniform";
		case PrimitiveVariable::Vertex :
			return "vertex";
		case PrimitiveVariable::Varying :
			return "varying";
		case PrimitiveVariable::FaceVarying :
			return "facevarying";
		default :
			return 0;
	}
}

const void *PrimitiveVariableList::value( IECore::DataPtr d )
{
	if( d->typeId()==StringData::staticTypeId() )
	{
		const char *v = static_pointer_cast<StringData>( d )->readable().c_str();
		m_charPtrs.push_back( v );
		return &*(m_charPtrs.rbegin());
	}
	else if( d->typeId()==StringVectorData::staticTypeId() )
	{
		const StringVectorData &sd = static_cast<StringVectorData &>( *d );
		for( unsigned i=0; i<sd.readable().size(); i++ )
		{
			m_charPtrs.push_back( sd.readable()[i].c_str() );
		}
		return (&*(m_charPtrs.rbegin())) - ( sd.readable().size() - 1 );
	}

	return despatchTypedData< TypedDataAddress, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError >( boost::const_pointer_cast<Data>( d ) );
}
