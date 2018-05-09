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

#include "boost/python.hpp"

#include "PrimitiveVariableBinding.h"

#include "IECoreScene/PrimitiveVariable.h"

#include "boost/format.hpp"

using namespace std;
using namespace boost::python;
using namespace IECore;
using namespace IECoreScene;

namespace
{

#define IECORETEST_ASSERT( x ) \
	if( !( x ) ) \
	{ \
		throw IECore::Exception( boost::str( \
			boost::format( "Failed assertion \"%s\" : %s line %d" ) % #x % __FILE__ % __LINE__ \
		) ); \
	}

void testIndexedRange()
{

	// Indexed primitive variable
	// -------------------------------

	IntVectorDataPtr indices = new IntVectorData( { 0, 1, 2, 0, 1, 2 } );
	IntVectorDataPtr data = new IntVectorData( { 3, 4, 5 } );
	PrimitiveVariable pi( PrimitiveVariable::FaceVarying, data, indices );

	PrimitiveVariable::IndexedRange<int> ri( pi );

	// Range-for iteration

	vector<int> expanded;
	for( auto &x : ri )
	{
		expanded.push_back( x );
	}

	IECORETEST_ASSERT( expanded == vector<int>( { 3, 4, 5, 3, 4, 5 } ) );

	// Size and subscripting

	IECORETEST_ASSERT( ri.size() == 6 );
	IECORETEST_ASSERT( ri[0] == 3 );
	IECORETEST_ASSERT( ri[1] == 4 );
	IECORETEST_ASSERT( ri[2] == 5 );
	IECORETEST_ASSERT( ri[3] == 3 );
	IECORETEST_ASSERT( ri[4] == 4 );
	IECORETEST_ASSERT( ri[5] == 5 );

	// Advance and distance

	auto it = ri.begin();
	IECORETEST_ASSERT( *it == 3 );
	it += 2;
	IECORETEST_ASSERT( *it == 5 );
	IECORETEST_ASSERT( it - ri.begin() == 2 );

	// Non-indexed primitive variable
	// -------------------------------

	PrimitiveVariable p( PrimitiveVariable::FaceVarying, data );
	PrimitiveVariable::IndexedRange<int> r( p );

	// Range-for iteration

	expanded.clear();
	for( auto &x : r )
	{
		expanded.push_back( x );
	}

	IECORETEST_ASSERT( expanded == vector<int>( { 3, 4, 5 } ) );

	// Size and subscripting

	IECORETEST_ASSERT( r.size() == 3 );
	IECORETEST_ASSERT( r[0] == 3 );
	IECORETEST_ASSERT( r[1] == 4 );
	IECORETEST_ASSERT( r[2] == 5 );

	// Advance and distance

	it = r.begin();
	IECORETEST_ASSERT( *it == 3 );
	it += 2;
	IECORETEST_ASSERT( *it == 5 );
	IECORETEST_ASSERT( it - r.begin() == 2 );

}

void testBoolIndexedRange()
{
	// Test BoolVectorData separately, because `vector<bool>` is specialised to use
	// a proxy for its `reference` typedef, and a value rather than a reference
	// for its `const_reference` typedef.

	BoolVectorDataPtr data = new BoolVectorData( { true, false } );
	IntVectorDataPtr indices = new IntVectorData( { 0, 0, 1, 0 } );

	PrimitiveVariable pi( PrimitiveVariable::FaceVarying, data, indices );

	PrimitiveVariable::IndexedRange<bool> ri( pi );
	vector<bool> expanded;
	for( bool x : ri )
	{
		expanded.push_back( x );
	}

	IECORETEST_ASSERT( expanded == vector<bool>( { true, true, false, true } ) );

	IECORETEST_ASSERT( ri[0] == true );
	IECORETEST_ASSERT( ri[1] == true );
	IECORETEST_ASSERT( ri[2] == false );
	IECORETEST_ASSERT( ri[3] == true );
}

DataPtr dataGetter( PrimitiveVariable &p )
{
	return p.data;
}

void dataSetter( PrimitiveVariable &p, DataPtr d )
{
	p.data = d;
}

IntVectorDataPtr indicesGetter( PrimitiveVariable &p )
{
	return p.indices;
}

void indicesSetter( PrimitiveVariable &p, IntVectorDataPtr i )
{
	p.indices = i;
}

string interpolationRepr( PrimitiveVariable::Interpolation i )
{
	switch( i )
	{
		case PrimitiveVariable::Invalid :
			return "IECoreScene.PrimitiveVariable.Interpolation.Invalid";
		case PrimitiveVariable::Constant :
			return "IECoreScene.PrimitiveVariable.Interpolation.Constant";
		case PrimitiveVariable::Uniform :
			return "IECoreScene.PrimitiveVariable.Interpolation.Uniform";
		case PrimitiveVariable::Vertex :
			return "IECoreScene.PrimitiveVariable.Interpolation.Vertex";
		case PrimitiveVariable::Varying :
			return "IECoreScene.PrimitiveVariable.Interpolation.Varying";
		case PrimitiveVariable::FaceVarying :
			return "IECoreScene.PrimitiveVariable.Interpolation.FaceVarying";
	}
	return "";
}

string primitiveVariableRepr( const PrimitiveVariable &p )
{
	string result = "IECoreScene.PrimitiveVariable( " + interpolationRepr( p.interpolation );

	if( p.data )
	{
		const string data = extract<string>( object( p.data ).attr( "__repr__" )() );
		result += ", " + data;
	}

	if( p.indices )
	{
		const string indices = extract<string>( object( p.indices ).attr( "__repr__" )() );
		result += ", " + indices;
	}

	result += " )";
	return result;
}

} // namespace

namespace IECoreSceneModule
{

void bindPrimitiveVariable()
{

	def( "testPrimitiveVariableIndexedRange", &testIndexedRange );
	def( "testPrimitiveVariableBoolIndexedRange", &testBoolIndexedRange );

	scope varScope = class_<PrimitiveVariable>( "PrimitiveVariable", no_init )
		.def( init<PrimitiveVariable::Interpolation, DataPtr>() )
		.def( init<PrimitiveVariable::Interpolation, DataPtr, IntVectorDataPtr>() )
		.def( init<const PrimitiveVariable &>() )
		.def( init<const PrimitiveVariable &, bool>() )
		.def_readwrite( "interpolation", &PrimitiveVariable::interpolation )
		.add_property( "data", &dataGetter, &dataSetter )
		.add_property( "indices", &indicesGetter, &indicesSetter )
		.def( "expandedData", &PrimitiveVariable::expandedData )
		.def( self == self )
		.def( self != self )
		.def( "__repr__", &primitiveVariableRepr )
	;
	enum_<PrimitiveVariable::Interpolation>( "Interpolation" )
		.value( "Invalid", PrimitiveVariable::Invalid )
		.value( "Constant", PrimitiveVariable::Constant )
		.value( "Uniform", PrimitiveVariable::Uniform )
		.value( "Vertex", PrimitiveVariable::Vertex )
		.value( "Varying", PrimitiveVariable::Varying )
		.value( "FaceVarying", PrimitiveVariable::FaceVarying )
	;

}

}
