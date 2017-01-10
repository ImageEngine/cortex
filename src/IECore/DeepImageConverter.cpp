//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#include "boost/algorithm/string/join.hpp"

#include "IECore/CompoundParameter.h"
#include "IECore/DeepImageConverter.h"
#include "IECore/DeepImageReader.h"
#include "IECore/DeepImageWriter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/SimpleTypedParameter.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( DeepImageConverter );

DeepImageConverter::DeepImageConverter()
	: Op( "Converts from one deep image format to another", new StringParameter( "result", "The new file", "" ) )
{
	std::vector<std::string> extensions;
	Reader::supportedExtensions( DeepImageReaderTypeId, extensions );

	m_inputFileParameter = new FileNameParameter(
		"inputFile",
		"The deep image file to read.",
		boost::algorithm::join( extensions, " " ),
		"",
		false,
		PathParameter::MustExist
	);

	extensions.clear();
	DeepImageWriter::supportedExtensions( extensions );

	m_outputFileParameter = new FileNameParameter(
		"outputFile",
		"The deep image file to write.",
		boost::algorithm::join( extensions, " " ),
		"",
		false,
		PathParameter::DontCare
	);

	parameters()->addParameter( m_inputFileParameter );
	parameters()->addParameter( m_outputFileParameter );
}

DeepImageConverter::~DeepImageConverter()
{
}

ObjectPtr DeepImageConverter::doOperation( const CompoundObject *operands )
{
	if ( m_inputFileParameter->getTypedValue() == m_outputFileParameter->getTypedValue() )
	{
		throw InvalidArgumentException( "Different input and output files must be specified." );
	}

	DeepImageReaderPtr reader = IECore::runTimeCast<DeepImageReader>( Reader::create( m_inputFileParameter->getTypedValue() ) );
	if ( !reader )
	{
		throw InvalidArgumentException( "The input file does not have an associated DeepImageReader: " + m_inputFileParameter->getTypedValue() );
	}

	DeepImageWriterPtr writer = DeepImageWriter::create( m_outputFileParameter->getTypedValue() );

	CompoundObjectPtr header = reader->readHeader();
	writer->channelNamesParameter()->setValue( header->member<StringVectorData>( "channelNames" ) );

	const Imath::Box2i dataWindow = header->member<Box2iData>( "dataWindow" )->readable();
	writer->resolutionParameter()->setTypedValue( dataWindow.size() + Imath::V2i( 1 ) );

	M44fData *worldToCamera = header->member<M44fData>( "worldToCameraMatrix" );
	if ( worldToCamera )
	{
		writer->worldToCameraParameter()->setValue( worldToCamera );
	}

	M44fData *worldToNDC = header->member<M44fData>( "worldToNDCMatrix" );
	if ( worldToNDC )
	{
		writer->worldToNDCParameter()->setValue( worldToNDC );
	}

	for ( int y=dataWindow.min.y; y <= dataWindow.max.y; ++y )
	{
		for ( int x=dataWindow.min.x; x <= dataWindow.max.x; ++x )
		{
			DeepPixelPtr pixel = reader->readPixel( x, y );
			writer->writePixel( x, y, pixel.get() );
		}
	}

	return new StringData( writer->fileName() );
}
