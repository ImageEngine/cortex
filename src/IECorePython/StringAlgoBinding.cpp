//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/StringAlgoBinding.h"

#include "IECore/CompoundData.h"
#include "IECore/StringAlgo.h"

#include "boost/python/suite/indexing/container_utils.hpp"

using namespace boost::python;
using namespace IECore;

namespace
{

bool matchPath( object path, object patternPath )
{
	std::vector<InternedString> p, pp;
	boost::python::container_utils::extend_container( p, path );
	boost::python::container_utils::extend_container( pp, patternPath );
	return StringAlgo::match( p, pp );
}

list matchPatternPath( const std::string &path, char separator )
{
	StringAlgo::MatchPatternPath p = StringAlgo::matchPatternPath( path, separator );
	list result;
	for( const auto &x : p )
	{
		result.append( x.c_str() );
	}
	return result;
}

struct VariableProviderWrapper : StringAlgo::VariableProvider, wrapper<StringAlgo::VariableProvider>
{

	virtual int frame() const
	{
		return this->get_override( "frame" )();
	}

	virtual const std::string &variable( const boost::string_view &name, bool &recurse ) const
	{
		object result = this->get_override( "variable" )( std::string( name ) );
		extract<tuple> tupleExtractor( result );
		if( tupleExtractor.check() )
		{
			tuple t = tupleExtractor();
			m_convertedString = extract<std::string>( t[0] );
			recurse = extract<bool>( t[1] );
		}
		else
		{
			m_convertedString = extract<std::string>( result );
		}
		return m_convertedString;
	}

	private :

		mutable std::string m_convertedString;

};

std::string substituteWrapper1( const std::string &input, ConstCompoundDataPtr variables, unsigned substitutions )
{
	return StringAlgo::substitute( input, variables.get(), substitutions );
}

std::string substituteWrapper2( const std::string &input, const StringAlgo::VariableProvider &variables, unsigned substitutions )
{
	return StringAlgo::substitute( input, variables, substitutions );
}

} // namespace

void IECorePython::bindStringAlgo()
{
	object module( borrowed( PyImport_AddModule( "IECore.StringAlgo" ) ) );
	scope().attr( "StringAlgo" ) = module;
	scope moduleScope( module );

	def( "match", &matchPath );
	def( "match", (bool (*)( const char *, const char * ))&IECore::StringAlgo::match );
	def( "matchMultiple", (bool (*)( const char *, const char * ))&IECore::StringAlgo::matchMultiple );
	def( "hasWildcards", (bool (*)( const char * ))&IECore::StringAlgo::hasWildcards );
	def( "matchPatternPath", &matchPatternPath, ( arg( "patternPath" ), arg( "separator" ) = '/' ) );

	enum_<StringAlgo::Substitutions>( "Substitutions" )
		.value( "NoSubstitutions", StringAlgo::NoSubstitutions )
		.value( "FrameSubstitutions", StringAlgo::FrameSubstitutions )
		.value( "VariableSubstitutions", StringAlgo::VariableSubstitutions )
		.value( "EscapeSubstitutions", StringAlgo::EscapeSubstitutions )
		.value( "TildeSubstitutions", StringAlgo::TildeSubstitutions )
		.value( "AllSubstitutions", StringAlgo::AllSubstitutions )
	;

	class_<VariableProviderWrapper, boost::noncopyable>( "VariableProvider" );

	def( "substitute", &substituteWrapper1, ( arg( "input" ), arg( "variables" ), arg( "substitutions" ) = StringAlgo::AllSubstitutions ) );
	def( "substitute", &substituteWrapper2, ( arg( "input" ), arg( "variables" ), arg( "substitutions" ) = StringAlgo::AllSubstitutions ) );
	def( "substitutions", &StringAlgo::substitutions );
	def( "hasSubstitutions", &StringAlgo::hasSubstitutions );

}
