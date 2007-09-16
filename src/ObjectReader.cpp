//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ObjectReader.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/FileNameParameter.h"

#ifdef IECORE_WITH_SQLITE
    #include "IECore/SQLiteIndexedIO.h"
#endif // IECORE_WITH_SQLITE

#include <cassert>

using namespace IECore;
using namespace boost;

const Reader::ReaderDescription<ObjectReader> ObjectReader::g_readerDescription( "cob" );

ObjectReader::ObjectReader() : 
		Reader( "ObjectReader", "Reads instances of a single Object from a file with a .cob extension" )
{
}

ObjectReader::ObjectReader( const std::string &fileName ) : 
		Reader( "ObjectReader", "Reads instances of a single Object from a file with a .cob extension" )
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
	IndexedIOInterfacePtr io = 0;
	
	try
	{
		io = open(fileName);
		return true;
	} 
	catch (Exception &e)
	{
		return false;
	}
	
	return io != 0;
}

ObjectPtr ObjectReader::doOperation( ConstCompoundObjectPtr operands )
{
	IndexedIOInterfacePtr io = open(fileName());
	return Object::load( io );
}

CompoundDataPtr ObjectReader::readHeader() const
{
	IndexedIOInterfacePtr io = open(fileName());
	io->chdir( "header" );
	return runTimeCast<CompoundData>( Object::load( io ) );
}

IndexedIOInterfacePtr ObjectReader::open( const std::string &fileName )
{
	IndexedIOInterfacePtr iface;
	
#ifdef IECORE_WITH_SQLITE
	try 
	{
		iface = new FileIndexedIO( fileName, "/", IndexedIO::Shared | IndexedIO::Read );
	} 
	catch (...)
	{
		iface = new SQLiteIndexedIO( fileName, "/", IndexedIO::Shared | IndexedIO::Read );
	}
#else
	iface = new FileIndexedIO( fileName, "/", IndexedIO::Shared | IndexedIO::Read );
#endif
	
	return iface;
}
