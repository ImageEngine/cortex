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

const InternedString ParameterList::g_handleParameterName( "__handle" );

ParameterList::ParameterList( const IECore::CompoundDataMap &parameters, const std::map<std::string, std::string> *typeHints )
{
	reserve( parameters );
	CompoundDataMap::const_iterator it;
	for( it=parameters.begin(); it!=parameters.end(); it++ )
	{
		if( it->first == g_handleParameterName )
		{
			// skip parameters called __handle
			continue;
		}
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

ParameterList::~ParameterList()
{
	for( size_t i=0; i < m_tokens.size(); ++i )
	{
		delete[] m_tokens[i];
	}
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
		case V3dDataTypeId :
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
		case Color3dDataTypeId :
			return "color";
		case FloatVectorDataTypeId :
			isArray = true;
			arraySize = static_cast<const FloatVectorData *>( d )->readable().size();
		case FloatDataTypeId :
		case DoubleDataTypeId :
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
		
		// convert double precision types to single precision:
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
		case V3dDataTypeId :
			{
				const Imath::V3d& t = static_cast<const V3dData *>( d )->readable();
				m_floats.push_back( (float)t.x );
				m_floats.push_back( (float)t.y );
				m_floats.push_back( (float)t.z );
				return &*(m_floats.rbegin() + 2);
			}
		case Color3dDataTypeId :
			{
				const Imath::Color3<double>& t = static_cast<const Color3dData *>( d )->readable();
				m_floats.push_back( (float)t[0] );
				m_floats.push_back( (float)t[1] );
				m_floats.push_back( (float)t[2] );
				return &*(m_floats.rbegin() + 2);
			}
		case DoubleDataTypeId :

			m_floats.push_back( (float)static_cast<const DoubleData *>( d )->readable() );
			return &*(m_floats.rbegin());

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
	m_tokens.reserve( numStrings );
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
	m_tokens.reserve( numStrings );
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
		case V3dDataTypeId :
			numFloats += 3;
			break;
		case Color3dDataTypeId :
			numFloats += 3;
			break;
		case DoubleDataTypeId :
			numFloats++;
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

int ParameterList::numPlaces( size_t n )
{
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	if (n < 10000000000) return 10;
	if (n < 100000000000) return 11;
	if (n < 1000000000000) return 12;
	if (n < 10000000000000) return 13;
	if (n < 100000000000000) return 14;
	if (n < 1000000000000000) return 15;
	if (n < 10000000000000000) return 16;
	if (n < 100000000000000000) return 17;
	if (n < 1000000000000000000) return 18;
	// maximum size_t ~= 1.8e19:
	return 19;
}

void ParameterList::appendInt( char *&str, size_t n )
{
	if( n == 0 )
	{
		*str++ = '0';
		return;
	}
	
	str += numPlaces( n );
	char* cc = str;
	--cc;
	while(n != 0)
	{
	    *cc-- = n%10+'0';
	    n=n/10;
	}
}

void ParameterList::appendString( char *&str, const char* toAppend, size_t len )
{
	memcpy( str, toAppend, len );
	str += len;
}

void ParameterList::appendString( char *&str, const std::string &toAppend )
{
	appendString( str, toAppend.c_str(), toAppend.size() );
}


void ParameterList::buildPositionsString( char*& str, const std::string& name, size_t arraySize )
{
	str = new char[ 6 + name.size() + 10 + numPlaces( arraySize ) + 2 ];
	
	char* c = str;
	appendString( c, "float ", 6 );
	appendString( c, name );
	appendString( c, "Positions[", 10 );
	appendInt( c, arraySize );
	*c++ = ']';
	*c++ = 0;
}

void ParameterList::buildColorValuesString( char*& str, const std::string& name, size_t arraySize )
{
	str = new char[ 6 + name.size() + 7 + numPlaces( arraySize ) + 2 ];
	
	char* c = str;
	appendString( c, "color ", 6 );
	appendString( c, name );
	appendString( c, "Values[", 7 );
	appendInt( c, arraySize );
	*c++ = ']';
	*c++ = 0;
}

void ParameterList::buildFloatValuesString( char*& str, const std::string& name, size_t arraySize )
{
	str = new char[ 6 + name.size() + 7 + numPlaces( arraySize ) + 2 ];
	
	char* c = str;
	appendString( c, "float ", 6 );
	appendString( c, name );
	appendString( c, "Values[", 7 );
	appendInt( c, arraySize );
	*c++ = ']';
	*c++ = 0;
}


void ParameterList::appendParameter( const std::string &name, const IECore::Data *d, const std::map<std::string, std::string> *typeHints )
{
	if( !d )
	{
		return;
	}
	
	// \todo: Investigate caching these parameter name strings so we don't have to allocate memory all the time.

	// we have to deal with the spline types separately, as they map to two shader parameters rather than one.
	IECore::TypeId typeId = d->typeId();
	if( typeId==SplineffDataTypeId )
	{
		const IECore::Splineff &spline = static_cast<const SplineffData *>( d )->readable();
		size_t size = spline.points.size();
		if ( size )
		{
			// put all the positions in one array parameter
			m_tokens.resize( m_tokens.size() + 1 );
			buildPositionsString( m_tokens.back(), name, size );
			m_values.push_back( &(m_floats[0]) + m_floats.size() );
			for( IECore::Splineff::PointContainer::const_iterator it=spline.points.begin(); it!=spline.points.end(); it++ )
			{
				m_floats.push_back( it->first );
			}
			// and put all the values in another
			m_tokens.resize( m_tokens.size() + 1 );
			buildFloatValuesString( m_tokens.back(), name, size );
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
			m_tokens.resize( m_tokens.size() + 1 );
			buildPositionsString( m_tokens.back(), name, size );
			m_values.push_back( &(m_floats[0]) + m_floats.size() );
			for( IECore::SplinefColor3f::PointContainer::const_iterator it=spline.points.begin(); it!=spline.points.end(); it++ )
			{
				m_floats.push_back( it->first );
			}
			// and put all the values in another
			m_tokens.resize( m_tokens.size() + 1 );
			buildColorValuesString( m_tokens.back(), name, size );
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
			m_tokens.resize( m_tokens.size() + 1 );
			int typeNameLen = strlen(t);
			if( isArray )
			{
				// manually build boost::str( boost::format( "%s %s[%d]" ) % t % name % arraySize ):
				
				m_tokens.back() = new char[ typeNameLen + 1 + name.size() + 1 + numPlaces( arraySize ) + 2 ];
				char* c = m_tokens.back();
				
				appendString( c, t, typeNameLen );
				*c++ = ' ';
				appendString( c, name );
				*c++ = '[';
				appendInt( c, arraySize );
				*c++ = ']';
				*c = 0;
			}
			else
			{
				// maually build string( t ) + " " + name ):
				
				m_tokens.back() = new char[ typeNameLen + 1 + name.size() + 1 ];
				char *c = m_tokens.back();
				
				appendString( c, t, typeNameLen );
				*c++ = ' ';
				appendString( c, name );
				*c++ = 0;
			}
			m_values.push_back( value( d ) );
		}
	}
}
