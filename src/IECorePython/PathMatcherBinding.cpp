//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, John Haddon. All rights reserved.
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
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

#include "IECorePython/PathMatcherBinding.h"

#include "IECorePython/RunTimeTypedBinding.h"

#include "IECore/PathMatcher.h"
#include "IECore/PathMatcherData.h"
#include "IECore/VectorTypedData.h"

#include "boost/format.hpp"
#include "boost/python/suite/indexing/container_utils.hpp"
#include "boost/tokenizer.hpp"

using namespace std;
using namespace boost::python;
using namespace IECore;

namespace
{

#define IECORETEST_ASSERT( x ) \
	if( !( x ) ) \
	{ \
		throw IECore::Exception( boost::str( \
			boost::format( "Failed assertion \"%s\" : %s line %d" ) % #x % __FILE__ % __LINE__ \
		) ); \
	}

void testPathMatcherRawIterator()
{
	vector<InternedString> root;
	vector<InternedString> a = { "a" };
	vector<InternedString> ab = { "a", "b" };
	vector<InternedString> abc = { "a", "b", "c" };

	PathMatcher m;
	PathMatcher::RawIterator it = m.begin();
	IECORETEST_ASSERT( it == m.end() );

	m.addPath( abc );
	it = m.begin();
	IECORETEST_ASSERT( *it == root );
	IECORETEST_ASSERT( it.exactMatch() == false );
	IECORETEST_ASSERT( it != m.end() );
	++it;
	IECORETEST_ASSERT( *it == a );
	IECORETEST_ASSERT( it.exactMatch() == false );
	IECORETEST_ASSERT( it != m.end() );
	++it;
	IECORETEST_ASSERT( *it == ab );
	IECORETEST_ASSERT( it.exactMatch() == false );
	IECORETEST_ASSERT( it != m.end() );
	++it;
	IECORETEST_ASSERT( *it == abc );
	IECORETEST_ASSERT( it.exactMatch() == true );
	IECORETEST_ASSERT( it != m.end() );
	++it;
	IECORETEST_ASSERT( it == m.end() );
}

void testPathMatcherIteratorPrune()
{
	vector<InternedString> root;
	vector<InternedString> abc = { "a", "b", "c" };

	// Prune an empty iterator range.
	PathMatcher m;
	PathMatcher::Iterator it = m.begin();
	IECORETEST_ASSERT( it == m.end() );
	it.prune();
	IECORETEST_ASSERT( it == m.end() );

	// Prune the root iterator itself.
	m.addPath( root );
	it = m.begin();
	IECORETEST_ASSERT( *it == root );
	IECORETEST_ASSERT( it != m.end() );
	it.prune();
	IECORETEST_ASSERT( *it == root );
	IECORETEST_ASSERT( it != m.end() );
	++it;
	IECORETEST_ASSERT( it == m.end() );

	// As above, but actually with some
	// descendants to be pruned.
	m.addPath( abc );
	it = m.begin();
	IECORETEST_ASSERT( *it == root );
	IECORETEST_ASSERT( it != m.end() );
	it.prune();
	IECORETEST_ASSERT( *it == root );
	IECORETEST_ASSERT( it != m.end() );
	++it;
	IECORETEST_ASSERT( it == m.end() );

}

void testPathMatcherFind()
{
	vector<InternedString> root;
	vector<InternedString> a = { "a" };
	vector<InternedString> ab = { "a", "b" };
	vector<InternedString> abc = { "a", "b", "c" };
	vector<InternedString> abcd = { "a", "b", "c", "d" };

	PathMatcher m;
	PathMatcher::RawIterator it = m.find( root );
	IECORETEST_ASSERT( it == m.end() );

	it = m.find( ab );
	IECORETEST_ASSERT( it == m.end() );

	m.addPath( abc );

	it = m.find( root );
	IECORETEST_ASSERT( it == m.begin() );
	IECORETEST_ASSERT( it != m.end() );
	IECORETEST_ASSERT( *it == root );
	++it;
	IECORETEST_ASSERT( *it == a );
	++it;
	IECORETEST_ASSERT( *it == ab );
	++it;
	IECORETEST_ASSERT( *it == abc );
	++it;
	IECORETEST_ASSERT( it == m.end() );

	it = m.find( ab );
	IECORETEST_ASSERT( it != m.end() );
	IECORETEST_ASSERT( *it == ab );
	++it;
	IECORETEST_ASSERT( *it == abc );
	++it;
	IECORETEST_ASSERT( it == m.end() );

	it = m.find( abcd );
	IECORETEST_ASSERT( it == m.end() );

}

// PathMatcher paths are just std::vector<InternedString>,
// which doesn't exist in Python. So we register a conversion from
// InternedStringVectorData which contains just such a vector.
/// \todo We could instead do this in the Cortex bindings for all
/// VectorTypedData types.
struct PathFromInternedStringVectorData
{

	PathFromInternedStringVectorData()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			nullptr,
			boost::python::type_id<std::vector<InternedString>>()
		);
	}

	static void *convertible( PyObject *obj )
	{
		extract<IECore::InternedStringVectorData *> dataExtractor( obj );
		if( dataExtractor.check() )
		{
			if( IECore::InternedStringVectorData *data = dataExtractor() )
			{
				return &(data->writable());
			}
		}

		return nullptr;
	}

};

// As a convenience we also accept strings in place of paths when
// calling from python. We deliberately don't do the same in c++ to force
// people to use the faster form.
struct PathFromString
{

	PathFromString()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			&construct,
			boost::python::type_id<std::vector<InternedString>>()
		);
	}

	static void *convertible( PyObject *obj )
	{
		if( PyString_Check( obj ) )
		{
			return obj;
		}
		return nullptr;
	}

	static void construct( PyObject *obj, boost::python::converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = (( converter::rvalue_from_python_storage<std::vector<InternedString>>* ) data )->storage.bytes;
		std::vector<InternedString> *path = new( storage ) std::vector<InternedString>();
		data->convertible = storage;

		std::string s = extract<std::string>( obj );
		typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
		Tokenizer t( s, boost::char_separator<char>( "/" ) );
		for( Tokenizer::const_iterator it = t.begin(), eIt = t.end(); it != eIt; it++ )
		{
			path->push_back( *it );
		}
	}

};

// we don't actually wrap the existing init, but rather reimplement it
// here using clear() and addPath(), so that we can support a mixture
// of strings and InternedStringVectorData.
void initWrapper( PathMatcher &m, boost::python::object paths )
{
	m.clear();
	for( size_t i = 0, e = len( paths ); i < e; ++i )
	{
		object path = paths[i];
		extract<const IECore::InternedStringVectorData *> pathDataExtractor( path );
		const IECore::InternedStringVectorData *pathData = pathDataExtractor.check() ? pathDataExtractor() : nullptr;
		if( pathData )
		{
			m.addPath( pathData->readable() );
		}
		else
		{
			m.addPath( extract<std::string>( path ) );
		}
	}
}

PathMatcher *constructFromObject( boost::python::object oPaths )
{
	PathMatcher *result = new PathMatcher;
	try
	{
		initWrapper( *result, oPaths );
	}
	catch( ... )
	{
		delete result;
		throw;
	}
	return result;
}

PathMatcher *constructFromVectorData( IECore::ConstStringVectorDataPtr paths )
{
	return new PathMatcher( paths->readable().begin(), paths->readable().end() );
}

boost::python::list paths( const PathMatcher &p )
{
	std::vector<std::string> paths;
	p.paths( paths );
	boost::python::list result;
	for( std::vector<std::string>::const_iterator it = paths.begin(), eIt = paths.end(); it != eIt; it++ )
	{
		result.append( *it );
	}
	return result;
}

std::string pathMatcherRepr( object p )
{
	std::string paths = extract<std::string>( p.attr( "paths" )().attr( "__repr__" )() );
	return "IECore.PathMatcher( " + paths + " )";
}

std::string pathMatcherDataRepr( object d )
{
	std::string p = extract<std::string>( d.attr( "value" ).attr( "__repr__" )() );
	return "IECore.PathMatcherData( " + p + " )";
}

} // namespace

void IECorePython::bindPathMatcher()
{

	PathFromInternedStringVectorData();
	PathFromString();

	/// \todo Create an IECoreTest module, and bind these in it
	def( "testPathMatcherRawIterator", &testPathMatcherRawIterator );
	def( "testPathMatcherIteratorPrune", &testPathMatcherIteratorPrune );
	def( "testPathMatcherFind", &testPathMatcherFind );

	IECorePython::RunTimeTypedClass<PathMatcherData>()
		.def( init<>() )
		.def( init<const PathMatcher &>() )
		.add_property( "value", make_function( &PathMatcherData::writable, return_internal_reference<1>() ) )
		.def( "hasBase", &PathMatcherData::hasBase ).staticmethod( "hasBase" )
		.def( "__repr__", &pathMatcherDataRepr )
	;

	scope s = class_<PathMatcher>( "PathMatcher" )
		.def( "__init__", make_constructor( constructFromObject ) )
		.def( "__init__", make_constructor( constructFromVectorData ) )
		.def( init<const PathMatcher &>() )
		.def( "init", &initWrapper )
		.def( "addPath", (bool (PathMatcher::*)( const std::vector<IECore::InternedString> & ))&PathMatcher::addPath )
		.def( "addPath", (bool (PathMatcher::*)( const std::string & ))&PathMatcher::addPath )
		.def( "removePath", (bool (PathMatcher::*)( const std::vector<IECore::InternedString> & ))&PathMatcher::removePath )
		.def( "removePath", (bool (PathMatcher::*)( const std::string & ))&PathMatcher::removePath )
		.def( "addPaths", (bool (PathMatcher::*)( const PathMatcher & ))&PathMatcher::addPaths )
		.def( "addPaths", (bool (PathMatcher::*)( const PathMatcher &, const std::vector<IECore::InternedString> & ))&PathMatcher::addPaths )
		.def( "removePaths", &PathMatcher::removePaths )
		.def( "intersection", (PathMatcher ( PathMatcher::*)( const PathMatcher & ) const)&PathMatcher::intersection )
		.def( "prune", (bool (PathMatcher::*)( const std::vector<IECore::InternedString> & ))&PathMatcher::prune )
		.def( "prune", (bool (PathMatcher::*)( const std::string & ))&PathMatcher::prune )
		.def( "subTree", (PathMatcher ( PathMatcher::*)( const std::vector<IECore::InternedString> & ) const)&PathMatcher::subTree )
		.def( "subTree", (PathMatcher ( PathMatcher::*)( const std::string & ) const)&PathMatcher::subTree )
		.def( "clear", &PathMatcher::clear )
		.def( "isEmpty", &PathMatcher::isEmpty )
		.def( "size", &PathMatcher::size )
		.def( "paths", &paths )
		.def( "match", (unsigned (PathMatcher ::*)( const std::vector<IECore::InternedString> & ) const)&PathMatcher::match )
		.def( "match", (unsigned (PathMatcher ::*)( const std::string & ) const)&PathMatcher::match )
		.def( "__repr__", &pathMatcherRepr )
		.def( self == self )
		.def( self != self )
	;

	enum_<PathMatcher::Result>( "Result" )
		.value( "NoMatch", PathMatcher::NoMatch )
		.value( "DescendantMatch", PathMatcher::DescendantMatch )
		.value( "ExactMatch", PathMatcher::ExactMatch )
		.value( "AncestorMatch", PathMatcher::AncestorMatch )
		.value( "EveryMatch", PathMatcher::EveryMatch )
	;

}
