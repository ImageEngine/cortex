//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MemoryIndexedIO.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/VectorTypedData.h"

using namespace IECore;

MemoryIndexedIO::MemoryIndexedIO( ConstCharVectorDataPtr buf, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode)
: FileIndexedIO()
{
	if (mode & IndexedIO::Write)
	{
		std::stringstream *f = new std::stringstream( std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out );

		open( f, root, mode, true );
	}
	else if (mode & IndexedIO::Append)
	{
		if ( !buf || ! buf->readable().size() )
		{
			/// Create new file
			std::stringstream *f = new std::stringstream(  std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out );

			open( f, root, mode, true );
		}
		else
		{
			assert( buf );

			/// Read existing file
			std::stringstream *f = new std::stringstream( std::string( &buf->readable()[0], buf->readable().size() ), std::ios::binary | std::ios::in | std::ios::out );

			open( f, root, mode );
		}
	}
	else
	{
		assert( buf );
		assert( mode & IndexedIO::Read );
		std::stringstream *f = new std::stringstream( std::string( &buf->readable()[0], buf->readable().size() ), std::ios::binary | std::ios::in | std::ios::out );

		open( f, root, mode );
	}
}

MemoryIndexedIO::~MemoryIndexedIO()
{
}

ConstCharVectorDataPtr MemoryIndexedIO::buffer()
{
	boost::optional<Imf::Int64> indexEnd = flush();

	std::stringstream *s = dynamic_cast< std::stringstream *>( device() ) ;
	assert( s );

	CharVectorData::ValueType d;
	const std::string &str = s->str();

	if ( indexEnd )
	{
		d.assign( str.begin(), str.end() );
		d.resize( *indexEnd );
		assert( d.size() ==  *indexEnd );
	}
	else
	{
		d.assign( str.begin(), str.end() );
	}

	return new CharVectorData( d );
}
