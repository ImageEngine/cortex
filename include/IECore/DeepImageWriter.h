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

#ifndef IECORE_DEEPIMAGEWRITER_H
#define IECORE_DEEPIMAGEWRITER_H

#include "IECore/Export.h"
#include "IECore/DeepPixel.h"
#include "IECore/Parameterised.h"
#include "IECore/SimpleTypedParameter.h"
#include "IECore/VectorTypedParameter.h"
#include "IECore/FileNameParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( DeepImageWriter );
IE_CORE_FORWARDDECLARE( FileNameParameter );

/// The DeepImageWriter class defines an abstract base class for writing deep images.
/// As with DeepPixel, DeepImageWriter only supports float channels. DeepImageWriter
/// derives directly from Parameterised rather than Writer, as it's only concerned
/// with writing DeepPixels, which are not Objects. As such, the write() method does
/// not exist, and pixels are instead writen one at a time using writePixel().
/// \ingroup deepCompositingGroup
/// \ingroup ioGroup
class IECORE_API DeepImageWriter : public Parameterised
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( DeepImageWriter, Parameterised )
		
		/// Creates and returns a DeepImageWriter appropriate for writing to the
		/// specified file (the file extension is used to determine format).
		/// Throws an Exception if no suitable writer can be found.
		static DeepImageWriterPtr create( const std::string &fileName );

		/// Returns the name of the file this DeepImageWriter is set to create.
		/// This is just a convenience returning the equivalent of
		/// parameters()->parameter<FileNameParameter>( "fileName" )->getTypedValue().
		const std::string &fileName() const;

		/// The parameter specifying the channels to write, in order. All pixels to
		/// be written must have these, and only these channels. Defaults to RGBA.
		StringVectorParameter *channelNamesParameter();
		const StringVectorParameter *channelNamesParameter() const;
		
		/// The parameter specifying the resolution of the image to write.
		/// This is required to be set before any pixels can be written.
		/// \todo: should this be dataWindow or resolution?
		IECore::V2iParameter *resolutionParameter();
		const IECore::V2iParameter *resolutionParameter() const;
		
		/// The parameter specifying the world to camera space transformation matrix
		IECore::M44fParameter *worldToCameraParameter();
		const IECore::M44fParameter *worldToCameraParameter() const;
		
		/// The parameter specifying the world to screen space projection matrix
		IECore::M44fParameter *worldToNDCParameter();
		const IECore::M44fParameter *worldToNDCParameter() const;
		
		/// Writes an individual DeepPixel to the file. Should throw if the data could not
		/// be written. Note that regardless of image format, x and y should be specified
		/// as if the origin is in the upper left corner of the displayWindow. It is up to
		/// the derived classes to account for that fact if necessary.
		void writePixel( int x, int y, const DeepPixel *pixel );

		/// Fills the passed vector with all the extensions for which a DeepImageWriter is
		/// available. Extensions are of the form "exr" - ie without a preceding '.'.
		static void supportedExtensions( std::vector<std::string> &extensions );

		/// Fills the passed vector with all the extensions for which a DeepImageWriter of,
		/// or inherited from, the given type is available.
		static void supportedExtensions( TypeId typeId, std::vector<std::string> &extensions );

	protected :

		DeepImageWriter( const std::string &description );

		/// Writes an individual DeepPixel. This is called by the public writePixel() method
		/// and must be implemented in all derived classes. It is guaranteed that the given
		/// DeepPixel is a valid pointer, has at least one depth sample, and has the correct
		/// number of channels. For speed, channel name verification is avoided. Note that
		/// regardless of image format, x and y should be specified as if the origin is in
		/// the upper left corner of the displayWindow. It is up to the derived classes to
		/// account for that fact if necessary.
		virtual void doWritePixel( int x, int y, const DeepPixel *pixel ) = 0;
		
		/// Definition of a function which can create a DeepImageWriter when given a fileName.
		typedef DeepImageWriterPtr (*CreatorFn)( const std::string &fileName );
		/// Definition of a function to answer the question can this file be opened for writing?
		typedef bool (*CanWriteFn)( const std::string &fileName );
		
		/// Registers a DeepImageWriter type which is capable of writing files ending with
		/// the space separated extensions specified (e.g. "tif tiff"). Typically you will
		/// not call this function directly to register a DeepImageWriter type - you will
		/// instead use the registration utility class below.
		static void registerDeepImageWriter( const std::string &extensions, CanWriteFn canWrite, CreatorFn creator, TypeId typeId );

		/// This utility class is provided to help with DeepImageWriter registration. By
		/// having a private static const instance of one of these in your class, it will
		/// call registerDeepImageWriter() for you when it is constructed. It assumes your
		/// DeepImageWriter class has a constructor taking a const std::string fileName.
		template<class T>
		class DeepImageWriterDescription
		{
			public :
				DeepImageWriterDescription( const std::string &extensions );
			private :
				static DeepImageWriterPtr creator( const std::string &fileName );
		};

		FileNameParameterPtr m_fileNameParameter;
		StringVectorParameterPtr m_channelsParameter;
		V2iParameterPtr m_resolutionParameter;

	private :
		
		struct DeepImageWriterFns
		{
			CreatorFn creator;
			CanWriteFn canWrite;
			TypeId typeId;
		};
		
		typedef std::multimap<std::string, DeepImageWriterFns> ExtensionsToFnsMap;
		static ExtensionsToFnsMap *extensionsToFns();

};

IE_CORE_DECLAREPTR( DeepImageWriter );

} // namespace IECore

#include "IECore/DeepImageWriter.inl"

#endif // IECORE_DEEPIMAGEWRITER_H
