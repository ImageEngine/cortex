//////////////////////////////////////////////////////////////////////////
//
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

#include "IECore/PathMatcherData.h"

#include "IECore/MessageHandler.h"
#include "IECore/TypedData.inl"

using namespace IECore;

namespace
{

static const unsigned int g_ioVersion = 0;

} // namespace

namespace IECore
{

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( PathMatcherData, PathMatcherDataTypeId )

template<>
void PathMatcherData::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), g_ioVersion );

	std::vector<InternedString> strings;
	std::vector<unsigned int> pathLengths;
	std::vector<unsigned char> exactMatches;

	for( PathMatcher::RawIterator it = readable().begin(), eIt = readable().end(); it != eIt; ++it )
	{
		pathLengths.push_back( it->size() );
		if( it->size() )
		{
			strings.push_back( it->back() );
		}
		exactMatches.push_back( it.exactMatch() );
	}

	container->write( "strings", strings.data(), strings.size() );
	container->write( "pathLengths", pathLengths.data(), pathLengths.size() );
	container->write( "exactMatches", exactMatches.data(), exactMatches.size() );
}

template<>
void PathMatcherData::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = g_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	const IndexedIO::Entry stringsEntry = container->entry( "strings" );
	std::vector<InternedString> strings;
	strings.resize( stringsEntry.arrayLength() );
	InternedString *stringsPtr = strings.data();
	container->read( "strings", stringsPtr, stringsEntry.arrayLength() );

	const IndexedIO::Entry pathLengthsEntry = container->entry( "pathLengths" );
	std::vector<unsigned int> pathLengths;
	pathLengths.resize( pathLengthsEntry.arrayLength() );
	unsigned int *pathLengthsPtr = pathLengths.data();
	container->read( "pathLengths", pathLengthsPtr, pathLengthsEntry.arrayLength() );

	const IndexedIO::Entry exactMatchesEntry = container->entry( "exactMatches" );
	std::vector<unsigned char> exactMatches;
	exactMatches.resize( exactMatchesEntry.arrayLength() );
	unsigned char *exactMatchesPtr = exactMatches.data();
	container->read( "exactMatches", exactMatchesPtr, exactMatchesEntry.arrayLength() );

	std::vector<InternedString> path;
	for( size_t i = 0, e = pathLengths.size(); i < e; ++i )
	{
		path.resize( pathLengths[i] );
		if( pathLengths[i] )
		{
			path.back() = *stringsPtr++;
		}
		if( exactMatches[i] )
		{
			writable().addPath( path );
		}
	}
}

template class TypedData<PathMatcher>;

} // namespace IECore
