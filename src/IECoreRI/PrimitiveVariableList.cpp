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

using namespace IECore;
using namespace IECoreRI;
using namespace std;
using namespace boost;

PrimitiveVariableList::PrimitiveVariableList( const IECore::PrimitiveVariableMap &primVars, const std::map<std::string, std::string> *typeHints )
{
	m_strings.reserve( primVars.size() );
	m_charPtrs.reserve( primVars.size() );
	PrimitiveVariableMap::const_iterator it;
	for( it=primVars.begin(); it!=primVars.end(); it++ )
	{
		const char *t = type( it->first, it->second.data, typeHints );
		const char *i = interpolation( it->second.interpolation );
		if( t && i )
		{
			m_strings.push_back( string( i ) + " " + t + " " + it->first );
			m_values.push_back( value( it->second.data ) );
			m_tokens.push_back( m_strings.rbegin()->c_str() );
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

const char *PrimitiveVariableList::type( const std::string &name, ConstDataPtr d, const std::map<std::string, std::string> *typeHints )
{
	IECore::TypeId t = d->typeId();
	switch( t )
	{
		case V3fVectorDataTypeId :
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
		case Color3fDataTypeId :
			return "color";
		case FloatVectorDataTypeId :
		case FloatDataTypeId :
			return "float";
		case IntVectorDataTypeId :
		case IntDataTypeId :
			return "int";
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
	/// \todo Support StringVectorData too
	if( d->typeId()==StringData::staticTypeId() )
	{
		const char *v = static_pointer_cast<StringData>( d )->readable().c_str();
		m_charPtrs.push_back( v );
		return &*(m_charPtrs.rbegin());
	}
	
	return despatchTypedData< TypedDataAddress, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError >( boost::const_pointer_cast<Data>( d ) );	
}
