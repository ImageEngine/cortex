//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_STRINGALGO_INL
#define IECORE_STRINGALGO_INL

#include "IECore/Exception.h"

#include <charconv>
#include <string.h>

namespace IECore
{

namespace Detail
{

// Performs matching of character classes within [], returning
// true for a match and false for no match. In either case, updates
// pattern to point to the first character after the closing ']', or
// to the terminating null in the case of a missing ']'.
inline bool matchCharacterClass( char c, const char *&pattern )
{
	const bool invert = *pattern == '!';
	if( invert )
	{
		pattern++;
	}

	bool matched = false;
	for( const char *start = pattern; true; pattern++ )
	{
		switch( char d = *pattern )
		{
			case '\0' :
				return false;
			case ']' :
				pattern++;
				return matched == !invert;
			case '-' :
				if( pattern > start && pattern[1] != ']' )
				{
					const char l = pattern[-1];
					const char r = *++pattern;
					if( c >= l && c <= r )
					{
						matched = true;
					}
					continue;
				}

				// The '-' was at the start or end of the
				// pattern, fall through to treat it
				// as a regular character below.
				[[fallthrough]];
			default :
				if( d == c )
				{
					matched = true;
				}
				continue;
		}
	}
}

// Returns true for a match and false for no match. Increments
// pattern to point to the last character tested.
inline bool matchInternal( const char *s, const char *&pattern, bool spaceTerminates = false )
{
	char c;
	while( true )
	{
		// Each case either returns a result if it can determine one,
		// or updates pattern and breaks if the match is ok so far.
		switch( c = *pattern )
		{
			case '\0' :

				return *s == c;

			case '*' :

				if( pattern[1] == '\0' || ( spaceTerminates && pattern[1] == ' ' ) )
				{
					// optimisation for when pattern
					// ends with '*'.
					return true;
				}

				// general case - recurse.
				for( const char *rs = s; *rs != '\0'; ++rs )
				{
					const char *rp = pattern + 1;
					if( matchInternal( rs, rp, spaceTerminates ) )
					{
						return true;
					}
				}
				return false;

			case '?' :

				if( *s++ != '\0' )
				{
					pattern++;
					break;
				}
				return false;

			case '\\' :

				if( pattern[1] && pattern[1] == *s++ )
				{
					pattern += 2;
					break;
				}
				return false;

			case '[' :

				if( matchCharacterClass( *s++, ++pattern ) )
				{
					break;
				}
				return false;

			case ' ' :

				if( spaceTerminates )
				{
					return *s == '\0';
				}
				// Fall through to default
				[[fallthrough]];
			default :

				if( c != *s++ )
				{
					return false;
				}
				pattern++;
		}
	}
}

template<class String>
constexpr auto stringData( const String& str )
{
	if constexpr( std::is_array_v< String > )
	{
		return str;
	}
	else if constexpr( std::is_pointer_v<String> )
	{
		return str;
	}
	else
	{
		return std::data( str );
	}
}

template<class String>
constexpr size_t stringSize( const String & str )
{
	if constexpr( std::is_array_v< String > )
	{
		return std::char_traits< typename std::remove_all_extents< String >::type >::length( str );
	}
	else if constexpr( std::is_pointer_v<String> )
	{
		return std::char_traits<std::remove_pointer_t<String>>::length( str );
	}
	else
	{
		return std::size( str );
	}
}

} // namespace Detail

namespace StringAlgo
{

inline bool match( const std::string &string, const std::string &pattern )
{
	return match( string.c_str(), pattern.c_str() );
}

inline bool match( const char *s, const char *pattern )
{
	return Detail::matchInternal( s, pattern );
}

inline bool matchMultiple( const std::string &s, const MatchPattern &patterns )
{
	return matchMultiple( s.c_str(), patterns.c_str() );
}

inline bool matchMultiple( const char *s, const char *patterns )
{
	do
	{
		if( Detail::matchInternal( s, patterns, /* spaceTerminates = */ true ) )
		{
			return true;
		}
		// Advance to next pattern.
		patterns += strcspn( patterns, " " ); // Increment to first space
		patterns += strspn( patterns, " " ); // Increment to next non-space
	} while( *patterns );

	return false;
}

inline bool hasWildcards( const std::string &pattern )
{
	return hasWildcards( pattern.c_str() );
}

inline bool hasWildcards( const char *pattern )
{
	return pattern[strcspn( pattern, "*?\\[" )];
}

template<typename Token, typename OutputIterator>
void tokenize( const std::string &s, const char separator, OutputIterator outputIterator )
{
	size_t index = 0, size = s.size();
	while( index < size )
	{
		const size_t prevIndex = index;
		index = s.find( separator, index );
		index = index == std::string::npos ? size : index;
		if( index > prevIndex )
		{
			*outputIterator++ = Token( s.c_str() + prevIndex, index - prevIndex );
		}
		index++;
	}
}

template<typename OutputContainer>
void tokenize( const std::string &s, const char separator, OutputContainer &outputContainer )
{
	tokenize<typename OutputContainer::value_type>( s, separator, std::back_inserter( outputContainer ) );
}

template<class String>
bool isUpperCase( const String &s )
{
	bool haveAlpha = false;
	for( typename String::const_iterator it = s.begin(), end=s.end(); it != end; ++it )
	{
		if( isalpha( *it ) )
		{
			if( !isupper( *it ) )
			{
				return false;
			}
			else
			{
				haveAlpha = true;
			}
		}
	}
	return haveAlpha;
}

template<class String>
bool isLowerCase( const String &s )
{
	bool haveAlpha = false;
	for( typename String::const_iterator it = s.begin(), end=s.end(); it != end; ++it )
	{
		if( isalpha( *it ) )
		{
			if( !islower( *it ) )
			{
				return false;
			}
			else
			{
				haveAlpha = true;
			}
		}
	}
	return haveAlpha;
}

inline int toInt( const std::string_view &s )
{
	int result = 0;

	auto elementIdResult = std::from_chars( s.data(), s.data() + s.size(), result );
	if( elementIdResult.ec == std::errc::invalid_argument || elementIdResult.ptr != s.data() + s.size() )
	{
		throw IECore::Exception( StringAlgo::concat( "Invalid integer ", s ) );
	}

	return result;
}

template<typename ... StringsFoldType >
std::string concat( StringsFoldType  const& ... strs )
{
	// Adapted from various posts on Stackoverflow linking to Godbolt links ... it's been passed
	// around a bunch, not clear who first wrote it

	std::string result;

	// C++17 fold for summation
	result.resize( ( 0 + ... + Detail::stringSize( strs ) ) );

	size_t pos = 0;

	// C++17 fold for function calls.
	(
		(
			std::copy(
				Detail::stringData( strs ),
				Detail::stringData( strs ) + Detail::stringSize( strs ),
				result.data() + pos
			),
			pos += Detail::stringSize(strs)
		), ...
	);

	return result;
}

} // namespace StringAlgo

} // namespace IECore

#endif // IECORE_STRINGALGO_INL
