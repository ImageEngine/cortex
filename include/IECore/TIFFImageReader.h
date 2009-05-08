//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
#ifndef IE_CORE_TIFFIMAGEREADER_H
#define IE_CORE_TIFFIMAGEREADER_H

#include "IECore/ImageReader.h"
#include "IECore/VectorTypedData.h"

// forward declaration
struct tiff;

namespace IECore
{

/// The TIFFImageReader reads Tagged Image File Format (TIFF) files.
///
/// Currently handled tags ( and their supported values ):
///
/// TIFFTAG_IMAGEWIDTH ( any != 0 )<br>
/// TIFFTAG_IMAGELENGTH ( any != 0 )<br>
/// TIFFTAG_TILEWIDTH ( any != 0 )<br>
/// TIFFTAG_TILELENGTH ( any != 0 )<br>
/// TIFFTAG_SAMPLESPERPIXEL ( any != 0 )<br>
/// TIFFTAG_BITSPERSAMPLE ( 8, 16, 32 )<br>
/// TIFFTAG_PHOTOMETRIC ( PHOTOMETRIC_RGB, PHOTOMETRIC_MIN_IS_BLACK )<br>
/// TIFFTAG_FILLORDER ( any )<br>
/// TIFFTAG_SAMPLEFORMAT ( SAMPLEFORMAT_INT, SAMPLEFORMAT_UINT, SAMPLEFORMAT_IEEEFP)<br>
/// TIFFTAG_ORIENTATION ( ORIENTATION_TOPLEFT )<br>
/// TIFFTAG_PLANARCONFIG ( PLANARCONFIG_CONTIG )<br>
/// TIFFTAG_EXTRASAMPLES<br>
/// TIFFTAG_XPOSITION<br>
/// TIFFTAG_YPOSITION<br>
/// TIFFTAG_PIXAR_IMAGEFULLWIDTH<br>
/// TIFFTAG_PIXAR_IMAGEFULLLENGTH<br>
/// TIFFTAG_XPOSITION<br>
/// TIFFTAG_YPOSITION<br>
///
class TIFFImageReader : public ImageReader
{

	public:

		IE_CORE_DECLARERUNTIMETYPED( TIFFImageReader, ImageReader );

		TIFFImageReader();
		TIFFImageReader(const std::string & filename);
		virtual ~TIFFImageReader();

		static bool canRead( const std::string &filename );

		/// A TIFF image can contain multiple directories (images), each capable of having its own resolution.
		/// This method returns the number of directories present in the file so that we can then call setDirectory()
		/// in order to read a specific image from the file.
		unsigned int numDirectories();

		/// Sets the index of the directory we want to read from the TIFF file. By default we read out the first directory
		/// present.
		void setDirectory( unsigned int directoryIndex );

		//! @name ImageReader interface
		/// Please note that these methods operate on the current TIFF directory.
                //@{
		virtual void channelNames( std::vector<std::string> &names );
		virtual bool isComplete();
		virtual Imath::Box2i dataWindow();
		virtual Imath::Box2i displayWindow();
		virtual std::string sourceColorSpace() const ;
		//@}

	private:

		virtual DataPtr readChannel( const std::string &name, const Imath::Box2i &dataWindow );

		// filename associator
		static const ReaderDescription<TIFFImageReader> m_readerDescription;

		/// Opens the file, if necessary, and determines the number of directories present, returning true on success
		/// and false on failure. On success, the m_tiffImage and m_numDirectories data members will be valid.
		/// If throwOnFailure is true then a descriptive Exception is thrown rather than false being returned.
		bool open( bool throwOnFailure = false );

		/// Opens the file, if necessary, and reads the image information from the current directory, returning true on
		/// success and false on failure. On success, the member data derived from the TIFF's "header" will be valid (e.g.
		/// the channel names. bits per sample, data window, etc).If throwOnFailure is true then a descriptive Exception is
		/// thrown rather than false being returned.
		bool readCurrentDirectory( bool throwOnFailure = false );

		// tiff image pointer
		tiff *m_tiffImage;
		std::string m_tiffImageFileName;

		unsigned int m_currentDirectoryIndex;
		unsigned int m_numDirectories;
		bool m_haveDirectory;

		std::vector<unsigned char> m_buffer;

		// Reads the interlaced data from the current directory into the buffer
		void readBuffer();

		Imath::Box2i m_displayWindow;
		Imath::Box2i m_dataWindow;
		int m_samplesPerPixel;
		int m_bitsPerSample;
		int m_fillOrder;
		int m_photometricInterpretation;
		int m_sampleFormat;
		int m_orientation;
		int m_planarConfig;
		std::vector<int> m_extraSamples;

		template<typename T>
		T tiffField( unsigned int t, T def = T(0) );

		template<typename T>
		T tiffFieldDefaulted( unsigned int t );

		template<typename T>
		DataPtr readTypedChannel( const std::string &name, const Imath::Box2i &dataWindow );

};

IE_CORE_DECLAREPTR(TIFFImageReader);

} // namespace IECore

#endif // IE_CORE_TIFFIMAGEREADER_H
