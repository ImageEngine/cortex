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

#ifndef IECORE_STRINGALGO_H
#define IECORE_STRINGALGO_H

#include "IECore/Export.h"
#include "IECore/InternedString.h"

#include <string>
#include <vector>

namespace IECore
{

namespace StringAlgo
{

/// A type which can be used to store a pattern to be matched against.
/// Note that the match() function can actually operate on other string
/// types as well so the use of this type is purely optional. The main
/// reason to use a MatchPattern is documentation - by including it in a function
/// signature, the use of an argument can be made more obvious.
///
/// Patterns support the following syntax, which is
/// based on shell glob expressions :
///
/// - "*", which matches any sequence of characters
/// - "?", which matches any single character
/// - "\", which escapes a subsequent wildcard
/// - [ABC], which matches any single character from the specified set
/// - [A-Z], which matches any single character from the specified range
/// - [!ABC], which matches any character not in the specified set
/// - [!A-Z], which matches any character not in the specified range
typedef std::string MatchPattern;

/// Returns true if the string matches the pattern and false otherwise.
inline bool match( const std::string &s, const MatchPattern &pattern );
inline bool match( const char *s, const char *pattern );

/// As above, but considering multiple patterns, separated by spaces.
inline bool matchMultiple( const std::string &s, const MatchPattern &patterns );
inline bool matchMultiple( const char *s, const char *patterns );

/// Returns true if the specified pattern contains characters which
/// have special meaning to the match() function.
inline bool hasWildcards( const MatchPattern &pattern );
inline bool hasWildcards( const char *pattern );

/// A type that holds a pattern that can be matched against a path
/// of names. Matching for each path component is performed using the
/// `match()` function above. An additional "..." token allows any sequence
/// of path components to be matched. This gives the same matching behaviour
/// as the `PathMatcher` class.
typedef std::vector<InternedString> MatchPatternPath;

/// Returns true if `path` matches `patternPath`, and false otherwise.
IECORE_API bool match( const std::vector<InternedString> &path, const MatchPatternPath &patternPath );

/// Tokenizes string into a MatchPatternPath, splitting on `separator`. Equivalent to
/// `tokenize()`, but with special handling for the "..." match token when separator is '.'.
IECORE_API MatchPatternPath matchPatternPath( const std::string &patternPath, char separator = '/' );

/// Returns the numeric suffix from the end of s, if one exists, and -1 if
/// one doesn't. If stem is specified then it will be filled with the contents
/// of s preceding the suffix, or the whole of s if no suffix exists.
IECORE_API int numericSuffix( const std::string &s, std::string *stem = nullptr );
/// As above, but returns defaultSuffix in the case that no suffix exists.
IECORE_API int numericSuffix( const std::string &s, int defaultSuffix, std::string *stem = nullptr );

/// Splits the input string wherever the separator is found, outputting all non-empty tokens
/// in sequence. Note that this is significantly quicker than boost::tokenizer
/// where TokenType is IECore::InternedString.
template<typename TokenType, typename OutputIterator>
void tokenize( const std::string &s, const char separator, OutputIterator outputIterator );
template<typename OutputContainer>
void tokenize( const std::string &s, const char separator, OutputContainer &outputContainer );

template<class Iterator>
typename std::iterator_traits<Iterator>::value_type join( Iterator begin, Iterator end, const typename std::iterator_traits<Iterator>::reference separator );

/// Returns true if s has no lower case letters and at least one upper case
/// letter - non alphabetic characters are ignored.
template<class String>
bool isUpperCase( const String &s );

/// Returns true if s has no upper case letters and at least one lower case
/// letter - non alphabetic characters are ignored.
template<class String>
bool isLowerCase( const String &s );

} // namespace StringAlgo

} // namespace IECore

#include "IECore/StringAlgo.inl"

#endif // IECORE_STRINGALGO_H
