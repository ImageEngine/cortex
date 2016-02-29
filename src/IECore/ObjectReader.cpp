//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ObjectWriter.h"
#include "IECore/ObjectReader.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/MemoryIndexedIO.h"
#include "IECore/FileNameParameter.h"
#include "IECore/CompoundData.h"

#include "boost/filesystem/convenience.hpp"
#include "boost/iostreams/filter/gzip.hpp"

#include <cassert>

#ifdef IECORE_WITH_BLOSC
#include "blosc.h"
#endif // IECORE_WITH_BLOSC

using namespace IECore;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ObjectReader );

const Reader::ReaderDescription<ObjectReader> ObjectReader::g_readerDescription( "cob" );

ObjectReader::ObjectReader() :
		Reader( "Reads instances of a single Object from a file with a .cob extension" )
{
}

ObjectReader::ObjectReader( const std::string &fileName ) :
		Reader( "Reads instances of a single Object from a file with a .cob extension" )
{
	m_fileNameParameter->setTypedValue( fileName );
}

ObjectReader::~ObjectReader()
{
}

bool ObjectReader::canRead( const std::string &fileName )
{
	// Ideally we'd like to look inside the file and see if it contains one object only
	// but for efficiency purposes we'll just try and open the file as a database and
	// see if that succeeds. We could possibly query the structure of the database and
	// check that it matches the signature of a one-object cache without needing to
	// actually read the data.
	IndexedIOPtr io = 0;

	try
	{
		if ( FileIndexedIO::canRead( fileName ) )
		{
			return true;
		}

		io = open(fileName);
		return true;
	}
	catch (Exception &e)
	{
		return false;
	}

	return io != 0;
}

ObjectPtr ObjectReader::doOperation( const CompoundObject * operands )
{
	CompoundObjectPtr header = readHeader();
	std::string compressionType = "none";
	StringData* compressionTypeData = header->member<StringData>( "compressionType" );
	if( compressionTypeData )
	{
		compressionType = compressionTypeData->readable();
	}
	IndexedIOPtr io = open(fileName());

#ifdef IECORE_WITH_BLOSC
	// is this file compressed using blosc?
	if( compressionType == "blosc" )
	{
		IndexedIOPtr objectCompressed = io->subdirectory( "objectCompressed" );

		// we split the file up into 1gb blocks to avoid integer overflow in the
		// blosc library. Read the number of compressed blocks:
		size_t numBlocks;
		objectCompressed->read( "numBlocks", numBlocks );

		// calculate total decompressed size:
		size_t totalDecompressedSize = 0;
		for( size_t i=0; i < numBlocks; ++i )
		{
			size_t s;
			objectCompressed->subdirectory( InternedString(i) )->read( "decompressedSize", s );
			totalDecompressedSize += s;
		}

		// create buffer for decompression:
		CharVectorDataPtr memBufferData = new CharVectorData;
		memBufferData->writable().resize( totalDecompressedSize );
		char* buffer = memBufferData->writable().data();

		// loop through all the blocks (max size = 1gb), and decompress into memBufferData:
		blosc_init();
		for( size_t i=0; i < numBlocks; ++i )
		{
			IndexedIOPtr compressedBlock = objectCompressed->subdirectory( InternedString(i) );
			size_t compressedSize, decompressedSize;
			compressedBlock->read( "compressedSize", compressedSize );
			compressedBlock->read( "decompressedSize", decompressedSize );

			// read the actual data
			std::vector<char> compressedData( compressedSize );
			char* cd = compressedData.data();
			compressedBlock->read( InternedString( "data" ), cd, compressedSize * sizeof(char) );

			// decompress into a buffer for a MemoryIndexedIO:
			blosc_decompress( compressedData.data(), buffer, decompressedSize );

			buffer += decompressedSize;
		}
		blosc_destroy();

		// create a MemoryIndexedIO from the buffer we've just read:
		io = new MemoryIndexedIO( memBufferData, IndexedIO::rootPath, IndexedIO::Read );
		return Object::load( io, "object" );
	}
#endif // IECORE_WITH_BLOSC
	if( compressionType != "none" )
	{
		throw Exception( "ObjectReader::doOperation(): unsupported compression type '" + compressionType + "'" );
	}
	
	return Object::load( io, "object" );
	
}

CompoundObjectPtr ObjectReader::readHeader()
{
	CompoundObjectPtr header = Reader::readHeader();

	IndexedIOPtr io = open(fileName());
	IndexedIOPtr headerIO = io->subdirectory( "header", IndexedIO::NullIfMissing );
	if( !headerIO )
	{
		throw Exception( "ObjectReader::readHeader(): couldn't find header io entry" );
	}

	// older .cob files store the header in "header/object" rather than "/header",
	// so we need to check for that:
	CompoundDataPtr objectHeader;
	IndexedIO::EntryIDList names;
	headerIO->entryIds( names );
	if( names.size() == 1 && names[0] == "object" )
	{
		// load from "/header/object"
		objectHeader = runTimeCast<CompoundData>( Object::load( headerIO, "object" ) );
	}
	else
	{
		// load from "/header"
		objectHeader = runTimeCast<CompoundData>( Object::load( io, "header" ) );
	}

	if( !objectHeader )
	{
		throw Exception( "ObjectReader::readHeader(): header was not a CompoundData" );
	}

	for ( CompoundData::ValueType::const_iterator it = objectHeader->readable().begin(); it != objectHeader->readable().end(); ++it )
	{
		header->members()[ it->first ] = it->second ;
	}

	return header;
}

IndexedIOPtr ObjectReader::open( const std::string &fileName )
{
	return new FileIndexedIO( fileName, IndexedIO::rootPath, IndexedIO::Shared | IndexedIO::Read );
}
