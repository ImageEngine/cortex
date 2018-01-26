//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECorePython/FileSequenceFunctionsBinding.h"

#include "IECorePython/IECoreBinding.h"

#include "IECore/Exception.h"
#include "IECore/FileSequence.h"
#include "IECore/FileSequenceFunctions.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/format.hpp"

using namespace boost::python;
using namespace IECore;

namespace IECorePython
{

struct FileSequenceFunctionsHelper
{
	static list findSequences( list namesList, size_t minSequenceSize = 2 )
	{
		list result;

		std::vector< std::string > names;
		for ( long i = 0; i < IECorePython::len( namesList ); i++ )
		{
			extract< std::string > ex( namesList[i] );
			if ( !ex.check() )
			{
				throw InvalidArgumentException( "findSequences: List element is not a string" );
			}

			names.push_back( ex() );
		}

		std::vector< FileSequencePtr > sequences;
		IECore::findSequences( names, sequences, minSequenceSize );
		for ( std::vector< FileSequencePtr >::const_iterator it = sequences.begin(); it != sequences.end(); ++it )
		{
			result.append( *it );
		}

		return result;
	}

	static object ls( const std::string &path, size_t minSequenceSize = 2 )
	{
		if ( boost::regex_match( path, FileSequence::fileNameValidator() ) )
		{
			FileSequencePtr sequence = nullptr;
			IECore::ls( path, sequence, minSequenceSize );

			if ( sequence )
			{
				return object( sequence );
			}
		}
		else if ( boost::filesystem::is_directory( path ) )
		{
			list result;
			std::vector< FileSequencePtr > sequences;
			IECore::ls( path, sequences, minSequenceSize );
			for ( std::vector< FileSequencePtr >::const_iterator it = sequences.begin(); it != sequences.end(); ++it )
			{
				result.append( *it );
			}

			return result;
		}

		return object();
	}

	static FrameListPtr frameListFromList( list l )
	{
		std::vector< FrameList::Frame > frameList;

		for ( long i = 0; i < IECorePython::len( l ); i++ )
		{
			extract< FrameList::Frame > ex( l[i] );
			if ( !ex.check() )
			{
				throw InvalidArgumentException( "frameListFromList: List element is not an integer" );
			}

			frameList.push_back( ex() );
		}

		return IECore::frameListFromList( frameList );
	}
};

void bindFileSequenceFunctions()
{
	def( "findSequences", &FileSequenceFunctionsHelper::findSequences, ( arg_("namesList"), arg_( "minSequenceSize" ) = 2 ) );
	def( "ls", &FileSequenceFunctionsHelper::ls, ( arg_("path"), arg_( "minSequenceSize" ) = 2 ) );
	def( "frameListFromList", &FileSequenceFunctionsHelper::frameListFromList );
}

}
