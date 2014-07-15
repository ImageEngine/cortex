//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/ParameterList.h"

#include "IECore/MatrixAlgo.h"
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
		appendParameter( it->first, it->second.get(), typeHints );
	}
}

ParameterList::ParameterList( const IECore::CompoundDataMap &parameters, const std::string &prefix,
	const std::map<std::string, std::string> *typeHints )
{
	reserve( parameters );
	CompoundDataMap::const_iterator it;
	for( it=parameters.begin(); it!=parameters.end(); it++ )
	{
		if( 0==it->first.value().compare( 0, prefix.size(), prefix ) )
		{
			appendParameter( string( it->first, prefix.size() ), it->second.get(), typeHints );
		}
	}
}

ParameterList::ParameterList( const std::string &name, const IECore::Data *parameter, const std::map<std::string, std::string> *typeHints )
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

const char *ParameterList::type( const std::string &name, const IECore::Data *d, bool &isArray, size_t &arraySize, const std::map<std::string, std::string> *typeHints )
{
	arraySize = 0;
	isArray = false;
	switch( d->typeId() )
	{
		case V3fVectorDataTypeId :
			isArray = true;
			arraySize = static_cast<const V3fVectorData *>( d )->readable().size();
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
			isArray = true;
			arraySize = static_cast<const Color3fVectorData *>( d )->readable().size();
		case Color3fDataTypeId :
			return "color";
		case FloatVectorDataTypeId :
			isArray = true;
			arraySize = static_cast<const FloatVectorData *>( d )->readable().size();
		case FloatDataTypeId :
			return "float";
		case V2iDataTypeId :
			isArray = true;
			arraySize = 2;
			return "integer";
		case IntVectorDataTypeId :
			isArray = true;
			arraySize = static_cast<const IntVectorData *>( d )->readable().size();
		case IntDataTypeId :
		case BoolDataTypeId :
			return "int";
		case StringVectorDataTypeId :
			isArray = true;
			arraySize = static_cast<const StringVectorData *>( d )->readable().size();
		case StringDataTypeId :
			return "string";
		case M44fDataTypeId :
		case M44dDataTypeId :
			return "matrix";
		default :
			msg( Msg::Warning, "ParameterList::type", format( "Variable \"%s\" has unsupported datatype." ) % name );
			return 0;
	}
}

const void *ParameterList::value( const IECore::Data *d )
{
	
	switch( d->typeId() )
	{
		case StringDataTypeId :
		{
			const char *v = static_cast<const StringData *>( d )->readable().c_str();
			m_charPtrs.push_back( v );
			return &*(m_charPtrs.rbegin());
		}
		case StringVectorDataTypeId :
		{
			const vector<string> &v = static_cast<const StringVectorData *>( d )->readable();
			for( vector<string>::const_iterator it=v.begin(); it!=v.end(); it++ )
			{
				m_charPtrs.push_back( it->c_str() );
			}
			if( v.size() )
			{
				return (&*(m_charPtrs.rbegin())) - ( v.size() - 1 );			
			}
			else
			{
				// you'd think it'd be ok to pass 3delight a NULL pointer when we're trying to
				// send an array of size 0, but that doesn't work. we have to pass a valid pointer
				// of some sort even though it appears to be unused. here we're just choosing totally
				// arbitrarily to use "this" for the purpose.
				return this;
			}
		}
		case BoolDataTypeId :
		
			m_ints.push_back( static_cast<const BoolData *>( d )->readable() );
			return &*(m_ints.rbegin());
		case M44dDataTypeId :
			{
				const Imath::M44d& t = static_cast<const M44dData *>( d )->readable();
				m_floats.push_back( (float)t[0][0] );
				m_floats.push_back( (float)t[0][1] );
				m_floats.push_back( (float)t[0][2] );
				m_floats.push_back( (float)t[0][3] );
				m_floats.push_back( (float)t[1][0] );
				m_floats.push_back( (float)t[1][1] );
				m_floats.push_back( (float)t[1][2] );
				m_floats.push_back( (float)t[1][3] );
				m_floats.push_back( (float)t[2][0] );
				m_floats.push_back( (float)t[2][1] );
				m_floats.push_back( (float)t[2][2] );
				m_floats.push_back( (float)t[2][3] );
				m_floats.push_back( (float)t[3][0] );
				m_floats.push_back( (float)t[3][1] );
				m_floats.push_back( (float)t[3][2] );
				m_floats.push_back( (float)t[3][3] );

				return &*(m_floats.rbegin() + 15);
			}
		default :
		
			return despatchTypedData< TypedDataAddress, TypeTraits::IsTypedData, DespatchTypedDataIgnoreError >( const_cast<Data *>( d ) );

	
	}
}

void ParameterList::reserve( const IECore::CompoundDataMap &parameters )
{
	size_t numStrings = 0;
	size_t numCharPtrs = 0;
	size_t numInts = 0;
	size_t numFloats = 0;
	for( IECore::CompoundDataMap::const_iterator it=parameters.begin(); it!=parameters.end(); it++ )
	{
		accumulateReservations( it->second.get(), numStrings, numCharPtrs, numInts, numFloats );
	}
	m_strings.reserve( numStrings );
	m_charPtrs.reserve( numCharPtrs );
	m_ints.reserve( numInts );
	m_floats.reserve( numFloats );
}

void ParameterList::reserve( const IECore::Data *parameter )
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

void ParameterList::accumulateReservations( const IECore::Data *d, size_t &numStrings, size_t &numCharPtrs, size_t &numInts, size_t &numFloats )
{
	if( !d )
	{
		return;
	}
	
	numStrings++; // for the formatted parameter name
	IECore::TypeId t = d->typeId();
	switch( t )
	{
		case StringDataTypeId :
			numCharPtrs++;
			break;
		case StringVectorDataTypeId :
			numCharPtrs += static_cast<const StringVectorData *>( d )->readable().size();
		case BoolDataTypeId :
			numInts++;
			break;
		case M44dDataTypeId :
			numFloats += 16;
			break;
		case SplineffDataTypeId :
			{
				size_t s = static_cast<const SplineffData *>( d )->readable().points.size();
				numFloats += s * 2; // one for each position and one for each value
				numStrings++; // for the second formatted parameter name (splines become two array parameters)
			}
			break;
		case SplinefColor3fDataTypeId :
			{
				size_t s = static_cast<const SplinefColor3fData *>( d )->readable().points.size();
				numFloats += s * 4; // one for each position and three for each value
				numStrings++; // for the second formatted parameter name (splines become two array parameters)
			}
			break;
		default :
			break;
			// no special handling required
	}
}

void ParameterList::appendParameter( const std::string &name, const IECore::Data *d, const std::map<std::string, std::string> *typeHints )
{
	if( !d )
	{
		return;
	}

	// we have to deal with the spline types separately, as they map to two shader parameters rather than one.
	IECore::TypeId typeId = d->typeId();
	if( typeId==SplineffDataTypeId )
	{
		const IECore::Splineff &spline = static_cast<const SplineffData *>( d )->readable();
		size_t size = spline.points.size();
		if ( size )
		{
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
		else
		{
			msg( Msg::Warning, "ParameterList::appendParameter", format( "Splineff \"%s\" has no points and will be ignored." ) % name );
			return;
		}
	}
	else if( typeId==SplinefColor3fDataTypeId )
	{
		const IECore::SplinefColor3f &spline = static_cast<const SplinefColor3fData *>( d )->readable();
		size_t size = spline.points.size();
		if ( size )
		{
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
			msg( Msg::Warning, "ParameterList::appendParameter", format( "SplinefColor3f \"%s\" has no points and will be ignored." ) % name );
			return;
		}
	}
	else
	{
		// other types are easier - they map to just a single parameter
		bool isArray = false;
		size_t arraySize = 0;
		const char *t = type( name, d, isArray, arraySize, typeHints );
		if( t )
		{
			if( isArray )
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
