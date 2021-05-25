//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2021, Image Engine Design Inc. All rights reserved.
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

#include "IECoreUSD/SceneCacheDataAlgo.h"

#include "boost/algorithm/string/replace.hpp"
#include <boost/iostreams/filter/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/regex.hpp>

using namespace boost::iostreams;
using namespace IECoreScene;
using namespace IECoreUSD;
using namespace pxr;


namespace
{
static const IECore::InternedString g_internalRootName( "__IECOREUSD_ROOT" );
static const pxr::TfToken g_internalRootNameToken( "__IECOREUSD_ROOT" );
static const IECore::InternedString g_hyphen( "-" );
static const IECore::InternedString g_hyphenSubstitute( std::string( 5, '_' ) );
static const IECore::InternedString g_closingParenthesis( ")" );
static const IECore::InternedString g_closingParenthesisSub( std::string( 3, '_' ) );
static const IECore::InternedString g_openingParenthesis( "(" );
static const IECore::InternedString g_openingParenthesisSub( std::string( 4, '_' ) );
static const regex_filter g_filterHyphenFromInternal( boost::regex( g_hyphenSubstitute.value() ), g_hyphen.value() );
static const regex_filter g_filterOpeningParenthesisFromInternal( boost::regex( g_openingParenthesisSub.value() ), g_openingParenthesis.value() );
static const regex_filter g_filterClosingParenthesisFromInternal( boost::regex( g_closingParenthesisSub.value() ), g_closingParenthesis.value() );
static const regex_filter g_filterHyphenToInternal( boost::regex( g_hyphen.value() ), g_hyphenSubstitute.value() );
static const regex_filter g_filterOpeningParenthesisToInternal( boost::regex( "\\(" ), g_openingParenthesisSub.value() );
static const regex_filter g_filterClosingParenthesisToInternal( boost::regex( "\\)" ), g_closingParenthesisSub.value() );
}

IECore::InternedString IECoreUSD::SceneCacheDataAlgo::internalRootName()
{
	return g_internalRootName;
}

pxr::TfToken IECoreUSD::SceneCacheDataAlgo::internalRootNameToken()
{
	return g_internalRootNameToken;
}

SceneInterface::Path IECoreUSD::SceneCacheDataAlgo::toInternalPath( const IECoreScene::SceneInterface::Path& scenePath )
{
	SceneInterface::Path result;
	result.reserve( scenePath.size() + 1 );

	if ( scenePath != SceneInterface::rootPath )
	{
		result.emplace_back( g_internalRootName );
	}
	for( auto& element : scenePath )
	{
		result.emplace_back( toInternalName( element ) );
	}

	return result;
}

SceneInterface::Path IECoreUSD::SceneCacheDataAlgo::fromInternalPath( const SceneInterface::Path& scenePath )
{
	SceneInterface::Path result;
	result.reserve( scenePath.size() );
	for( size_t i=0, end=scenePath.size(); i<end; i++ )
	{
		if ( i == 0 && scenePath[i] == g_internalRootName )
		{
			continue;
		}
		result.emplace_back( fromInternalName( scenePath[i] ) );
	}

	return result;
}

std::string IECoreUSD::SceneCacheDataAlgo::fromInternalName( const IECore::InternedString& name )
{
	std::string result;
	filtering_ostream out;

	out.push( g_filterHyphenFromInternal );
	out.push( g_filterOpeningParenthesisFromInternal );
	out.push( g_filterClosingParenthesisFromInternal );
	out.push( back_inserter( result ) );

	out<<name;
	out.flush();

	return result;
}

std::string IECoreUSD::SceneCacheDataAlgo::toInternalName( const IECore::InternedString& name )
{
	std::string result;
	filtering_ostream out;

	out.push( g_filterHyphenToInternal );
	out.push( g_filterOpeningParenthesisToInternal );
	out.push( g_filterClosingParenthesisToInternal );
	out.push( back_inserter( result ) );

	out<<name;
	out.flush();

	return result;
}
