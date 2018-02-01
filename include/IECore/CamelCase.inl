//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_CAMELCASE_INL
#define IECORE_CAMELCASE_INL

#include "IECore/StringAlgo.h"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"

namespace IECore
{

template<typename OutputIterator>
void CamelCase::split( const std::string &camelCase, OutputIterator output )
{
	int size = camelCase.size();
	if( !size )
	{
		return;
	}

	// split into words based on adjacent cases being the same

	std::vector<std::string> words;

	std::string currentWord;
	bool prevUpper = isupper( camelCase[0] );
	for( int i=0; i<size; i++ )
	{
		bool upper = isupper( camelCase[i] );
		if( upper==prevUpper )
		{
			currentWord.push_back( camelCase[i] );
		}
		else
		{
			words.push_back( currentWord );
			currentWord.clear();
			currentWord.push_back( camelCase[i] );
		}
		prevUpper = upper;
	}

	words.push_back( currentWord );

	// output words, moving last capital of previous word onto any lowercase words

	bool prefix = false;
	for( int i=0, size=words.size(); i<size; i++ )
	{
		if( prefix )
		{
			// output this word with the last letter of the previous word
			std::string prefixedWord;
			prefixedWord.push_back( words[i-1][words[i-1].size()-1] );
			prefixedWord += words[i];
			*output++ = prefixedWord;
			prefix = false;
		}
		else if( i + 1 < size )
		{
			// check to see if this word has a last letter better suited
			// to being on the next word.
			if( isupper( words[i][words[i].size()-1] ) && StringAlgo::isLowerCase( words[i+1] ) )
			{
				if( words[i].size() > 1 )
				{
					*output++ = words[i].substr( 0, words[i].size()-1 );
				}
				prefix = true;
			}
			else
			{
				*output++ = words[i];
				prefix = false;
			}
		}
		else
		{
			*output++ = words[i];
			prefix = false;
		}
	}

}

template<typename Iterator>
std::string CamelCase::join( Iterator begin, Iterator end, Caps caps, const std::string &separator )
{
	std::string result;
	for( Iterator it=begin; it!=end; ++it )
	{
		std::string word = *it;
		if( !word.size() )
		{
			continue;
		}
		if( caps!=Unchanged )
		{
			if( (caps==First && it==begin) || caps==All || (caps==AllExceptFirst && it!=begin) )
			{
				if( !StringAlgo::isUpperCase( word ) )
				{
					boost::algorithm::to_lower( word );
				}
				word[0] = std::toupper( word[0] );
			}
			else if( (caps==AllExceptFirst && it==begin) || (caps==First && it!=begin ) )
			{
				boost::algorithm::to_lower( word );
			}
		}
		if( it!=begin )
		{
			result += separator;
		}
		result += word;
	}

	return result;
}

} // namespace IECore

#endif // IECORE_CAMELCASE_INL
