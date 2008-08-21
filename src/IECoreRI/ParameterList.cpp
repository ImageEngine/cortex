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

#include "IECoreRI/ParameterList.h"

#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreRI;
using namespace std;
using namespace boost;

ParameterList::ParameterList( const IECore::CompoundDataMap &parameters, const std::map<std::string, std::string> *typeHints )
{
	reserve( parameters );
	CompoundDataMap::const_iterator it;
	for( it=parameters.begin(); it!=parameters.end(); it++ )
	{
		appendParameter( it->first, it->second, typeHints );
	}
}

ParameterList::ParameterList( const IECore::CompoundDataMap &parameters, const std::string &prefix,
	const std::map<std::string, std::string> *typeHints )
{
	reserve( parameters );
	CompoundDataMap::const_iterator it;
	for( it=parameters.begin(); it!=parameters.end(); it++ )
	{
		if( 0==it->first.compare( 0, prefix.size(), prefix ) )
		{
			appendParameter( string( it->first, prefix.size() ), it->second, typeHints );
		}
	}
}
			
ParameterList::ParameterList( const std::string &name, IECore::ConstDataPtr parameter, const std::map<std::string, std::string> *typeHints )
{
	reserve( parameter );
	appendParameter( name, parameter, typeHints );
}
			
int ParameterList::n()
{
	return m_tokens.size();
}

char **ParameterList::tokens()
{
	return (char **)&*(m_tokens.begin());
}

void **ParameterList::values()
{
	return (void **)&*(m_values.begin());
}

const char *ParameterList::type( const std::string &name, IECore::ConstDataPtr d, size_t &arraySize, const std::map<std::string, std::string> *typeHints )
{
	arraySize = 0;
	switch( d->typeId() )
	{
		case V3fVectorDataTypeId :
			arraySize = static_pointer_cast<const V3fVectorData>( d )->readable().size();
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
		case BoolDataTypeId :
			return "int";
		case StringVectorDataTypeId :
			arraySize = static_pointer_cast<const StringVectorData>( d )->readable().size();
		case StringDataTypeId :
			return "string";
		case M44fDataTypeId :
			return "matrix";
		default :
			msg( Msg::Warning, "ParameterList::type", format( "Variable \"%s\" has unsupported datatype." ) % name );
			return 0;
	}
}

const void *ParameterList::value( IECore::ConstDataPtr d )
{
	if( d->typeId()==StringData::staticTypeId() )
	{
		const char *v = static_pointer_cast<const StringData>( d )->readable().c_str();
		m_charPtrs.push_back( v );
		return &*(m_charPtrs.rbegin());
	}
	if( d->typeId()==BoolData::staticTypeId() )
	{
		m_ints.push_back( static_pointer_cast<const BoolData>( d )->readable() );
		return &*(m_ints.rbegin());
	}
	
	return despatchTypedData< TypedDataAddress, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError >( boost::const_pointer_cast<Data>( d ) );	
}

void ParameterList::reserve( const IECore::CompoundDataMap &parameters )
{
	size_t numStrings = 0;
	size_t numCharPtrs = 0;
	size_t numInts = 0;
	size_t numFloats = 0;
	for( IECore::CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		accumulateReservations( it->second, numStrings, numCharPtrs, numInts, numFloats );
	}
	m_strings.reserve( numStrings );
	m_charPtrs.reserve( numCharPtrs );
	m_ints.reserve( numInts );
	m_floats.reserve( numFloats );
}

void ParameterList::reserve( IECore::ConstDataPtr &parameter )
{
	size_t numStrings = 0;
	size_t numCharPtrs = 0;
	size_t numInts = 0;
	size_t numFloats = 0;
	accumulateReservations( parameter, numStrings, numCharPtrs, numInts, numFloats );
	m_strings.reserve( numStrings );
	m_charPtrs.reserve( numCharPtrs );
	m_ints.reserve( numInts );
	m_floats.reserve( numFloats );
}

void ParameterList::accumulateReservations( const IECore::ConstDataPtr d, size_t &numStrings, size_t &numCharPtrs, size_t &numInts, size_t &numFloats )
{
	numStrings++; // for the formatted parameter name
	IECore::TypeId t = d->typeId();
	switch( t )
	{
		case StringDataTypeId :
			numCharPtrs++;
			break;
		case BoolDataTypeId :
			numInts++;
			break;
		case SplineffDataTypeId :
			{
				size_t s = static_cast<const SplineffData *>( d.get() )->readable().points.size();
				numFloats += s * 2; // one for each position and one for each value
				numStrings++; // for the second formatted parameter name (splines become two array parameters)
			}
			break;
		case SplinefColor3fDataTypeId :
			{
				size_t s = static_cast<const SplinefColor3fData *>( d.get() )->readable().points.size();
				numFloats += s * 4; // one for each position and three for each value
				numStrings++; // for the second formatted parameter name (splines become two array parameters)
			}
			break;
		default :
			break;
			// no special handling required
	}
}

void ParameterList::appendParameter( const std::string &name, IECore::ConstDataPtr d, const std::map<std::string, std::string> *typeHints )
{
	// we have to deal with the spline types separately, as they map to two shader parameters rather than one.
	IECore::TypeId typeId = d->typeId();
	if( typeId==SplineffDataTypeId )
	{
		const IECore::Splineff &spline = static_cast<const SplineffData *>( d.get() )->readable();
		size_t size = spline.points.size();
		// put all the positions in one array parameter
		m_strings.push_back( boost::str( boost::format( "float %sPositions[%d]" ) % name % size ) );
		m_tokens.push_back( m_strings.rbegin()->c_str() );
		m_values.push_back( &(m_floats[0]) + m_floats.size() );
		for( IECore::Splineff::PointContainer::const_iterator it=spline.points.begin(); it!=spline.points.end(); it++ )
		{
			m_floats.push_back( it->first );
		}
		// and put all the values in another
		m_strings.push_back( boost::str( boost::format( "float %sValues[%d]" ) % name % size ) );
		m_tokens.push_back( m_strings.rbegin()->c_str() );
		m_values.push_back( &(m_floats[0]) + m_floats.size() );
		for( IECore::Splineff::PointContainer::const_iterator it=spline.points.begin(); it!=spline.points.end(); it++ )
		{
			m_floats.push_back( it->second );
		}
	}
	else if( typeId==SplinefColor3fDataTypeId )
	{
		const IECore::SplinefColor3f &spline = static_cast<const SplinefColor3fData *>( d.get() )->readable();
		size_t size = spline.points.size();
		// put all the positions in one array parameter
		m_strings.push_back( boost::str( boost::format( "float %sPositions[%d]" ) % name % size ) );
		m_tokens.push_back( m_strings.rbegin()->c_str() );
		m_values.push_back( &(m_floats[0]) + m_floats.size() );
		for( IECore::SplinefColor3f::PointContainer::const_iterator it=spline.points.begin(); it!=spline.points.end(); it++ )
		{
			m_floats.push_back( it->first );
		}
		// and put all the values in another
		m_strings.push_back( boost::str( boost::format( "color %sValues[%d]" ) % name % size ) );
		m_tokens.push_back( m_strings.rbegin()->c_str() );
		m_values.push_back( &(m_floats[0]) + m_floats.size() );
		for( IECore::SplinefColor3f::PointContainer::const_iterator it=spline.points.begin(); it!=spline.points.end(); it++ )
		{
			m_floats.push_back( it->second[0] );
			m_floats.push_back( it->second[1] );
			m_floats.push_back( it->second[2] );
		}
	}
	else
	{
		// other types are easier - they map to just a single parameter
		size_t arraySize = 0;
		const char *t = type( name, d, arraySize, typeHints );
		if( t )
		{
			if( arraySize )
			{
				m_strings.push_back( boost::str( boost::format( "%s %s[%d]" ) % t % name % arraySize ) );
			}
			else
			{
				m_strings.push_back( string( t ) + " " + name );
			}
			m_tokens.push_back( m_strings.rbegin()->c_str() );
			m_values.push_back( value( d ) );
		}
	}
}
