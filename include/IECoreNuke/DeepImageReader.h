//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORENUKE_DEEPIMAGEREADER_H
#define IECORENUKE_DEEPIMAGEREADER_H

#include "DDImage/Knobs.h"
#include "DDImage/Thread.h"
#include "DDImage/DeepReader.h"
#include "DDImage/DeepPlane.h"

namespace IECoreNuke
{

/// This class can be extended to expose reader options that will be
/// displayed on Nuke's DeepRead node. 
class DeepImageReaderFormats : public DD::Image::DeepReaderFormat
{
	void append( DD::Image::Hash &h)
	{
	}

	void knobs( DD::Image::Knob_Callback f )
	{
	}
};

/// Reads DeepImage files that are supported by cortex.
/// The DeepImageReader enables support for the deep image types
/// registered to cortex from Nuke's DeepReader node.
class DeepImageReader : public DD::Image::DeepReader
{

	public:

		DeepImageReader( DD::Image::DeepReaderOwner *op, const std::string &fileName );

		virtual bool doDeepEngine( DD::Image::Box box, const DD::Image::ChannelSet &channels, DD::Image::DeepOutputPlane &plane );
		virtual const DD::Image::MetaData::Bundle &fetchMetaData( const char *key );

		static const char *supportedExtensions();
		static DD::Image::DeepReader *build( DD::Image::DeepReaderOwner *op, const std::string &fn );
		static DD::Image::DeepReaderFormat *buildformat( DD::Image::DeepReaderOwner *op );
		static const DD::Image::DeepReader::Description g_description;

	private:

		/// Loads an image and sets m_reader to the reader for the file. If an Exception is
		/// raised then the reason is saved and returned in errorMsg.
		/// Returns true if the file was successfully loaded.
		bool loadFileFromPath( const std::string &filePath, std::string &errorMsg );
		
		/// Holds the path of the file that is currently being read. 
		std::string m_currentPath;
		
		/// A mutex which ensures that only one thread reads from the file at once.	
		DD::Image::Lock m_lock;

		/// The data window of the file. This is set within loadFileFromPath().
		DD::Image::Box m_dataWindow;

		/// The channels within the file. This is set within loadFileFromPath().
		DD::Image::ChannelSet m_channels;

		/// The cortex reader that we use to read the file.
		IECore::DeepImageReaderPtr m_reader;
		
		/// A map of Channels to indexes within the IECore::DeepPixel class's channelData().
		std::map< DD::Image::Channel, int > m_channelMap;

		/// The metadata of the deep image.
		DD::Image::MetaData::Bundle m_meta;
};

}; // namespace IECoreNuke

#endif
