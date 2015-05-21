//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundParameter.h"
#include "IECore/Reader.h"
#include "IECore/TypeIds.h"
#include "IECore/Writer.h"

#include "IECoreHoudini/GEO_CobIOTranslator.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

GEO_CobIOTranslator::GEO_CobIOTranslator()
{
}

GEO_CobIOTranslator::~GEO_CobIOTranslator()
{
}

GEO_IOTranslator *GEO_CobIOTranslator::duplicate() const
{
	return new GEO_CobIOTranslator();
}

const char *GEO_CobIOTranslator::formatName() const
{
	return "Cortex Object Format";
}

int GEO_CobIOTranslator::checkExtension( const char *fileName ) 
{
	UT_String sname( fileName );
	
	/// \todo: support all extensions that can read/write any object supported by the To/FromHoudiniGeometryConverters
	if ( sname.fileExtension() && ( !strcmp( sname.fileExtension(), ".cob" ) || !strcmp( sname.fileExtension(), ".pdc" ) || !strcmp( sname.fileExtension(), ".ptc" ) ) )
	{
		return true;
	}
	
	return false;
}

int GEO_CobIOTranslator::checkMagicNumber( unsigned magic )
{
	return 0;
}

GA_Detail::IOStatus GEO_CobIOTranslator::fileLoad( GEO_Detail *geo, UT_IStream &is, bool ate_magic )
{
	((UT_IFStream&)is).close();
	
	ConstObjectPtr object = 0;
	try
	{
		ReaderPtr reader = Reader::create( is.getLabel() );
		if ( !reader )
		{
			return false;
		}
		
		object = reader->read();
	}
	catch ( IECore::Exception e )
	{
		return false;
	}
	
	if ( !object )
	{
		return false;
	}
	
	ToHoudiniGeometryConverterPtr converter = ToHoudiniGeometryConverter::create( object.get() );
	if ( !converter )
	{
		return false;
	}
	
	GU_DetailHandle handle;
	handle.allocateAndSet( (GU_Detail*)geo, false );
	
	return converter->convert( handle );
}

GA_Detail::IOStatus GEO_CobIOTranslator::fileLoad( GEO_Detail *geo, UT_IStream &is, int ate_magic )
{
	return fileLoad( geo, is, (bool)ate_magic );
}

GA_Detail::IOStatus GEO_CobIOTranslator::fileSave( const GEO_Detail *geo, std::ostream &os )
{
	return false;
}

GA_Detail::IOStatus GEO_CobIOTranslator::fileSaveToFile( const GEO_Detail *geo, const char *fileName )
{
	GU_DetailHandle handle;
	handle.allocateAndSet( (GU_Detail*)geo, false );
	
	FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( handle );
	if ( !converter )
	{
		return false;
	}
	
	ObjectPtr object = converter->convert();
	if ( !object )
	{
		return false;
	}
	
	try
	{
		WriterPtr writer = Writer::create( object, fileName );
		writer->write();
	}
	catch ( IECore::Exception e )
	{
		return false;
	}
	
	return true;
}

GA_Detail::IOStatus GEO_CobIOTranslator::fileSaveToFile( const GEO_Detail *geo, std::ostream &os, const char *fileName )
{
	((std::ofstream&)os).close();
	
	return fileSaveToFile( geo, fileName );
}
