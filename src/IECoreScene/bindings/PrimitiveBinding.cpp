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

#include "PrimitiveBinding.h"

#include "IECoreScene/Primitive.h"
#include "IECoreScene/MeshPrimitive.h" // need to construct a concrete type to test

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
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

#define IECORETEST_ASSERT_MSG( x, msg ) \
    if( !( x ) ) \
    { \
        throw IECore::Exception( boost::str( \
            boost::format( "Failed assertion \"%s\" msg: '%s': %s line %d" ) % #x % msg % __FILE__ % __LINE__ \
        ) ); \
    }

void testVariableIndexedView()
{
	IECoreScene::PrimitivePtr primitive = new IECoreScene::MeshPrimitive();
	primitive->variables["P"] = PrimitiveVariable(
		IECoreScene::PrimitiveVariable::Interpolation::Vertex, new IECore::V3fVectorData( {Imath::V3f( 0, 0, 0 ), Imath::V3f( 1, 2, 3 )} )
	);

	// check we get an view if type & interpolation are compatible
	{
		boost::optional<PrimitiveVariable::IndexedView<Imath::V3f>> optionalIndexedView = primitive->variableIndexedView<IECore::V3fVectorData>(
			"P", IECoreScene::PrimitiveVariable::Interpolation::Vertex
		);

		if( optionalIndexedView )
		{
			PrimitiveVariable::IndexedView<Imath::V3f> indexedView = *optionalIndexedView;

			IECORETEST_ASSERT( indexedView.size() == 2 );
			IECORETEST_ASSERT( indexedView[0] == Imath::V3f( 0, 0, 0 ) );
			IECORETEST_ASSERT( indexedView[1] == Imath::V3f( 1, 2, 3 ) );
		}
		else
		{
			IECORETEST_ASSERT( false );
		}

		// If requiredInterpolation = Invalid matches any interpolation
		boost::optional<PrimitiveVariable::IndexedView<Imath::V3f>> optionalIndexedView2 = primitive->variableIndexedView<IECore::V3fVectorData>(
			"P", IECoreScene::PrimitiveVariable::Interpolation::Invalid
		);

		if( optionalIndexedView2 )
		{
			PrimitiveVariable::IndexedView<Imath::V3f> indexedView = *optionalIndexedView2;

			IECORETEST_ASSERT( indexedView.size() == 2 );
			IECORETEST_ASSERT( indexedView[0] == Imath::V3f( 0, 0, 0 ) );
			IECORETEST_ASSERT( indexedView[1] == Imath::V3f( 1, 2, 3 ) );
		}
		else
		{
			IECORETEST_ASSERT( false );
		}
	}

	// missing primvar
	{
		boost::optional<PrimitiveVariable::IndexedView<Imath::V3f>> optionalIndexedView = primitive->variableIndexedView<IECore::V3fVectorData>(
			"MISSING", IECoreScene::PrimitiveVariable::Interpolation::Vertex
		);

		IECORETEST_ASSERT( !optionalIndexedView );

		bool exceptionRaised = false;
		try
		{
			boost::optional<PrimitiveVariable::IndexedView<Imath::V3f>> optionalIndexedView = primitive->variableIndexedView<IECore::V3fVectorData>(
				"MISSING", IECoreScene::PrimitiveVariable::Interpolation::Vertex, true /* throwIfInvalid */
			);
		}
		catch( IECore::Exception &e )
		{
			IECORETEST_ASSERT_MSG( strcmp( e.what(), "Primitive::variableIndexedView - No primvar named 'MISSING' found" ) == 0, e.what() );
			exceptionRaised = true;
		}

		IECORETEST_ASSERT( exceptionRaised );
	}

	// invalid interpolation
	{
		boost::optional<PrimitiveVariable::IndexedView<Imath::V3f>> optionalIndexedView = primitive->variableIndexedView<IECore::V3fVectorData>(
			"P", IECoreScene::PrimitiveVariable::Interpolation::FaceVarying
		);

		IECORETEST_ASSERT( !optionalIndexedView );

		bool exceptionRaised = false;
		try
		{
			boost::optional<PrimitiveVariable::IndexedView<Imath::V3f>> optionalIndexedView = primitive->variableIndexedView<IECore::V3fVectorData>(
				"P", IECoreScene::PrimitiveVariable::Interpolation::FaceVarying, true /* throwIfInvalid */
			);
		}
		catch( IECore::Exception &e )
		{
			IECORETEST_ASSERT_MSG( strcmp(
				e.what(),
				"Primitive::variableIndexedView - PrimVar 'P' interpolation (3) doesn't match requiredInterpolation (5)"
			) == 0, e.what() );
			exceptionRaised = true;
		}

		IECORETEST_ASSERT( exceptionRaised );
	}

	// invalid type
	{
		boost::optional<PrimitiveVariable::IndexedView<Imath::V2f>> optionalIndexedView = primitive->variableIndexedView<IECore::V2fVectorData>(
			"P", IECoreScene::PrimitiveVariable::Interpolation::Vertex
		);

		IECORETEST_ASSERT( !optionalIndexedView );

		bool exceptionRaised = false;
		try
		{
			boost::optional<PrimitiveVariable::IndexedView<Imath::V2f>> optionalIndexedView = primitive->variableIndexedView<IECore::V2fVectorData>(
				"P", IECoreScene::PrimitiveVariable::Interpolation::Vertex, true /* throwIfInvalid */
			);
		}
		catch( IECore::Exception &e )
		{
			IECORETEST_ASSERT_MSG( strcmp(
				e.what(),
				"Primitive::variableIndexedView - Unable to created indexed view for 'P' PrimVar, requested type: 'V2fVectorDataBase', actual type: 'V3fVectorData'"
			) == 0, e.what() );
			exceptionRaised = true;
		}

		IECORETEST_ASSERT( exceptionRaised );
	}
}

} // namespace

namespace IECoreSceneModule
{

static unsigned int len( Primitive &p )
{
	return p.variables.size();
}

static PrimitiveVariable getItem( Primitive &p, const std::string &n )
{
	PrimitiveVariableMap::const_iterator it = p.variables.find( n );
	if( it==p.variables.end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	return it->second;
}

static void setItem( Primitive &p, const std::string &n, const PrimitiveVariable &v )
{
	p.variables[n] = v;
}

static bool contains( Primitive &p, const std::string &n )
{
	return p.variables.find( n )!=p.variables.end();
}

static boost::python::list keys( Primitive &p )
{
	boost::python::list result;
	PrimitiveVariableMap::const_iterator it;
	for( it = p.variables.begin(); it!=p.variables.end(); it++ )
	{
		result.append( it->first );
	}
	return result;
}

static boost::python::list values( Primitive &p )
{
	boost::python::list result;
	PrimitiveVariableMap::const_iterator it;
	for( it = p.variables.begin(); it!=p.variables.end(); it++ )
	{
		result.append( it->second );
	}
	return result;
}

static void delItem( Primitive &p, const std::string &n )
{
	PrimitiveVariableMap::iterator it = p.variables.find( n );
	if( it==p.variables.end() )
	{
		throw std::out_of_range( "Bad index" );
	}
	p.variables.erase( it );
}

static MurmurHash topologyHash( Primitive &p )
{
	MurmurHash h;
	p.topologyHash( h );
	return h;
}

void bindPrimitive()
{
	def( "testVariableIndexedView", &testVariableIndexedView );

	RunTimeTypedClass<Primitive>()
		.def( "variableSize", &Primitive::variableSize )
		.def( "__len__", &len )
		.def( "__getitem__", &getItem, "Returns a shallow copy of the requested PrimitiveVariable object." )
		.def( "__setitem__", &setItem )
		.def( "__delitem__", &delItem )
		.def( "__contains__", &contains )
		.def( "keys", &keys )
		.def( "values", &values, "Returns a list containing shallow copies of the PrimitiveVariable objects held." )
		.def( "isPrimitiveVariableValid", &Primitive::isPrimitiveVariableValid)
		.def( "arePrimitiveVariablesValid", &Primitive::arePrimitiveVariablesValid)
		.def( "inferInterpolation", (PrimitiveVariable::Interpolation (Primitive::*)( const Data * ) const)&Primitive::inferInterpolation )
		.def( "inferInterpolation", (PrimitiveVariable::Interpolation (Primitive::*)( size_t ) const)&Primitive::inferInterpolation )
		.def( "topologyHash", &topologyHash )
		.def( "topologyHash", (void (Primitive::*)( MurmurHash & ) const)&Primitive::topologyHash )
	;
}

}
