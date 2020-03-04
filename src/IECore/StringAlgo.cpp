//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, John Haddon. All rights reserved.
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

#include "IECore/StringAlgo.h"

#include "IECore/CompoundData.h"
#include "IECore/SimpleTypedData.h"

#include "boost/algorithm/string/replace.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include "boost/version.hpp"

#if BOOST_VERSION > 105500

#include "boost/utility/string_view.hpp"

#endif

using namespace std;
using namespace IECore;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

const InternedString g_ellipsis( "..." );
const InternedString g_ellipsisSubstitute( "!!!" );

bool matchInternal(
	vector<InternedString>::const_iterator pathBegin,
	vector<InternedString>::const_iterator pathEnd,
	StringAlgo::MatchPatternPath::const_iterator matchPathBegin,
	StringAlgo::MatchPatternPath::const_iterator matchPathEnd
)
{
	while( true )
	{
		if( matchPathBegin == matchPathEnd )
		{
			return pathBegin == pathEnd;
		}
		else if( *matchPathBegin == g_ellipsis )
		{
			auto nextMatchPathBegin = std::next( matchPathBegin );
			if( nextMatchPathBegin == matchPathEnd )
			{
				return true;
			}
			for( auto pb = pathBegin; pb != pathEnd; ++pb )
			{
				if( matchInternal( pb, pathEnd, nextMatchPathBegin, matchPathEnd ) )
				{
					return true;
				}
			}
			return false;
		}
		else if( pathBegin == pathEnd )
		{
			return false;
		}
		else if( !StringAlgo::match( *pathBegin, *matchPathBegin ) )
		{
			return false;
		}
		++pathBegin;
		++matchPathBegin;
	}
}

#if BOOST_VERSION > 105500

inline void substituteInternal( const char *s, const StringAlgo::VariableProvider &variables, std::string &result, const int recursionDepth, unsigned substitutions )
{
	if( recursionDepth > 8 )
	{
		throw IECore::Exception( "StringAlgo::substitute() : maximum recursion depth reached." );
	}

	while( *s )
	{
		switch( *s )
		{
			case '\\' :
			{
				if( substitutions & StringAlgo::EscapeSubstitutions )
				{
					s++;
					if( *s )
					{
						result.push_back( *s++ );
					}
				}
				else
				{
					// variable substitutions disabled
					result.push_back( *s++ );
				}
				break;
			}
			case '$' :
			{
				if( substitutions & StringAlgo::VariableSubstitutions )
				{
					s++; // skip $
					bool bracketed = *s =='{';
					const char *variableNameStart = nullptr;
					const char *variableNameEnd = nullptr;
					if( bracketed )
					{
						s++; // skip initial bracket
						variableNameStart = s;
						while( *s && *s != '}' )
						{
							s++;
						}
						variableNameEnd = s;
						if( *s )
						{
							s++; // skip final bracket
						}
					}
					else
					{
						variableNameStart = s;
						while( isalnum( *s ) )
						{
							s++;
						}
						variableNameEnd = s;
					}

					bool recurse = false;
					const std::string &variable = variables.variable(
						boost::string_view( variableNameStart, variableNameEnd - variableNameStart ),
						recurse
					);

					if( recurse )
					{
						substituteInternal( variable.c_str(), variables, result, recursionDepth + 1, substitutions );
					}
					else
					{
						result += variable;
					}
				}
				else
				{
					// variable substitutions disabled
					result.push_back( *s++ );
				}
				break;
			}
			case '#' :
			{
				if( substitutions & StringAlgo::FrameSubstitutions )
				{
					int padding = 0;
					while( *s == '#' )
					{
						padding++;
						s++;
					}
					std::ostringstream padder;
					padder << std::setw( padding ) << std::setfill( '0' ) << variables.frame();
					result += padder.str();
				}
				else
				{
					// frame substitutions disabled
					result.push_back( *s++ );
				}
				break;
			}
			case '~' :
			{
				if( substitutions & StringAlgo::TildeSubstitutions && result.size() == 0 )
				{
					if( const char *v = getenv( "HOME" ) )
					{
						result += v;
					}
					++s;
					break;
				}
				else
				{
					// tilde substitutions disabled
					result.push_back( *s++ );
				}
				break;
			}
			default :
				result.push_back( *s++ );
				break;
		}
	}
}

struct CompoundDataVariableProvider : public StringAlgo::VariableProvider
{

	CompoundDataVariableProvider( const CompoundData *variables )
		:	m_variables( variables )
	{
	}

	int frame() const override
	{
		const Data *d = m_variables->member( "frame" );
		if( !d )
		{
			return 1;
		}
		switch( d->typeId() )
		{
			case IntDataTypeId :
				return static_cast<const IntData *>( d )->readable();
			case FloatDataTypeId :
				return (int)round( static_cast<const FloatData *>( d )->readable() );
			default :
				throw IECore::Exception(
					string( "Unexpected data type \"" ) + d->typeName() +
					"\" for frame : expected IntData or FloatData"
				);
		}
	}

	const std::string &variable( const boost::string_view &name, bool &recurse ) const override
	{
		const IECore::Data *d = m_variables->member<IECore::Data>( name );
		if( d )
		{
			switch( d->typeId() )
			{
				case IECore::StringDataTypeId :
					recurse = true;
					return static_cast<const IECore::StringData *>( d )->readable();
				case IECore::FloatDataTypeId :
					m_formattedString = boost::lexical_cast<std::string>(
						static_cast<const IECore::FloatData *>( d )->readable()
					);
					return m_formattedString;
				case IECore::IntDataTypeId :
					m_formattedString = boost::lexical_cast<std::string>(
						static_cast<const IECore::IntData *>( d )->readable()
					);
					return m_formattedString;
				default :
					break;
			}
		}
		m_formattedString.clear();
		return m_formattedString;
	}

	private :

		const CompoundData *m_variables;
		mutable std::string m_formattedString;

};

#endif

} // namespace

//////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////

namespace IECore
{

namespace StringAlgo
{

bool match( const std::vector<InternedString> &path, const MatchPatternPath &patternPath )
{
	return matchInternal( path.begin(), path.end(), patternPath.begin(), patternPath.end() );
}

MatchPatternPath matchPatternPath( const std::string &patternPath, char separator )
{
	string path = patternPath;
	if( separator == '.' )
	{
		boost::replace_all( path, "...", "." + g_ellipsisSubstitute.string() + "." );
	}

	vector<InternedString> result;
	StringAlgo::tokenize( path, separator, result );

	if( separator == '.' )
	{
		std::replace( result.begin(), result.end(), g_ellipsisSubstitute, g_ellipsis );
	}
	return result;
}

#if BOOST_VERSION > 105500

std::string substitute( const std::string &s, const CompoundData *variables, unsigned substitutions )
{
	return substitute(
		s, CompoundDataVariableProvider( variables ), substitutions
	);
}

std::string substitute( const std::string &input, const VariableProvider &variableProvider, unsigned substitutions )
{
	std::string result;
	result.reserve( input.size() ); // Might need more or less, but this is a reasonable ballbark
	substituteInternal( input.c_str(), variableProvider, result, 0, substitutions );
	return result;
}

#endif

unsigned substitutions( const std::string &input )
{
	unsigned result = NoSubstitutions;
	for( const char *c = input.c_str(); *c; )
	{
		switch( *c )
		{
			case '$' :
				result |= VariableSubstitutions;
				c++;
				break;
			case '#' :
				result |= FrameSubstitutions;
				c++;
				break;
			case '~' :
				result |= TildeSubstitutions;
				c++;
				break;
			case '\\' :
				result |= EscapeSubstitutions;
				c++;
				if( *c )
				{
					c++;
				}
				break;
			default :
				c++;
		}
		if( result == AllSubstitutions )
		{
			return result;
		}
	}
	return result;
}

bool hasSubstitutions( const std::string &input )
{
	for( const char *c = input.c_str(); *c; c++ )
	{
		switch( *c )
		{
			case '$' :
			case '#' :
			case '~' :
			case '\\' :
				return true;
			default :
				; // do nothing
		}
	}
	return false;
}

int numericSuffix( const std::string &s, std::string *stem )
{
	static boost::regex g_regex( "^(.*[^0-9]+)([0-9]+)$" );
	boost::cmatch match;
	if( regex_match( s.c_str(), match, g_regex ) )
	{
		if( stem )
		{
			*stem = match[1];
		}
		return boost::lexical_cast<int>( match[2] );
	}
	if( stem )
	{
		*stem = s;
	}
	return -1;
}

int numericSuffix( const std::string &s, int defaultSuffix, std::string *stem )
{
	int n = numericSuffix( s, stem );
	return n < 0 ? defaultSuffix : n;
}

} // namespace StringAlgo

} // namespace IECore
