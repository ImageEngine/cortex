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

#include "IECore/ObjectWriter.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/FileNameParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/ObjectParameter.h"
#include "IECore/CompoundData.h"
#include "IECore/Object.h"
#include "IECore/IECore.h"

using namespace IECore;
using namespace std;
using namespace boost;

const Writer::WriterDescription<ObjectWriter> ObjectWriter::g_writerDescription( "cob" );

ObjectWriter::ObjectWriter()
	:	Writer( "ObjectWriter", "Writes instances of a single Object to a file with a .cob extension", ObjectTypeId )
{
	constructParameters();
}

ObjectWriter::ObjectWriter( ObjectPtr object, const std::string &fileName )
	:	Writer( "ObjectWriter", "Writes instances of a single Object to a file with a .cob extension", ObjectTypeId )
{
	constructParameters();
	m_objectParameter->setValue( object );
	m_fileNameParameter->setTypedValue( fileName );
}

bool ObjectWriter::canWrite( ConstObjectPtr object, const std::string &fileName )
{
	return true;
}

void ObjectWriter::doWrite()
{	
	IndexedIOInterfacePtr io = new FileIndexedIO( fileName(), "/", IndexedIO::Exclusive | IndexedIO::Write);
	
	// write the header
	CompoundDataPtr header = static_pointer_cast<CompoundData>( m_headerParameter->getValue()->copy() );
	
	const int maxHNameSize = 2048;
	char hName[maxHNameSize];
	gethostname( hName, maxHNameSize );
	header->writable()["host"] = new StringData( hName );
	header->writable()["ieCoreVersion"] = new StringData( versionString() );
	header->writable()["typeName"] = new StringData( object()->typeName() );
	
	io->mkdir( "header" );
	io->chdir( "header" );
		((ObjectPtr)header)->save( io );
	io->chdir( ".." );
	
	// write the object
	object()->save( io );
}

void ObjectWriter::constructParameters()
{
	m_headerParameter = new ObjectParameter(
		"header",
		"A CompoundData object containing elements to be added to the file header.",
		new CompoundData,
		CompoundData::staticTypeId()
	);
		
	parameters()->addParameter( m_headerParameter );
}
