//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
#include "IECore/ObjectWriter.h"

#include "CoreHoudini.h"
#include "GEO_CobIOTranslator.h"
#include "FromHoudiniGeometryConverter.h"
#include "ToHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

GEO_CobIOTranslator::GEO_CobIOTranslator()
{
}

GEO_CobIOTranslator::~GEO_CobIOTranslator()
{
}

const char *GEO_CobIOTranslator::formatName() const
{
	return "Cortex Object Format";
}

int GEO_CobIOTranslator::checkExtension( const char *fileName ) 
{
	UT_String sname( fileName );

	if ( sname.fileExtension() && !strcmp( sname.fileExtension(), ".cob" ) )
	{
		return true;
	}
	
	return false;
}

int GEO_CobIOTranslator::checkMagicNumber( unsigned magic )
{
	return 0;
}

bool GEO_CobIOTranslator::fileLoad( GEO_Detail *geo, UT_IStream &is, int ate_magic )
{
	((UT_IFStream&)is).close();
	
	ObjectReaderPtr reader = new ObjectReader( is.getLabel() );
	ConstPrimitivePtr primitive = 0;
	
	try
	{
		primitive = runTimeCast<Primitive>( reader->read() );
	}
	catch ( IECore::IOException e )
	{
		return false;
	}
	
	if ( !primitive )
	{
		return false;
	}
	
	GU_DetailHandle handle;
	handle.allocateAndSet( (GU_Detail*)geo, false );
	
	ToHoudiniGeometryConverterPtr converter = ToHoudiniGeometryConverter::create( primitive );
	
	return converter->convert( handle );
}

int GEO_CobIOTranslator::fileSave( const GEO_Detail *geo, ostream &os )
{
}

int GEO_CobIOTranslator::fileSaveToFile( const GEO_Detail *geo, ostream &os, const char *fileName )
{
	((ofstream&)os).close();
	
	GU_DetailHandle handle;
	handle.allocateAndSet( (GU_Detail*)geo, false );
	
	ObjectPtr object = CoreHoudini::convertFromHoudini( handle );
	if ( !object )
	{
		return false;
	}
	
	ObjectWriterPtr writer = new ObjectWriter( object, fileName );
	writer->write();
	
	return true;
}
