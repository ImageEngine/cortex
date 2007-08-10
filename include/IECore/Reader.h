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

#ifndef IE_CORE_READER_H
#define IE_CORE_READER_H

#include "IECore/Op.h"

#include <map>
#include <vector>

namespace IECore
{

IE_CORE_FORWARDDECLARE( Reader );
IE_CORE_FORWARDDECLARE( FileNameParameter );

/// The Reader class defines an interface for loading
/// files of arbitrary type in the form of Object instances.
class Reader : public Op
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( Reader, Op	 );

		/// If resultParameter is not specified then creates a resultParameter
		/// which is a simple Parameter instance. If a derived class provides
		/// more concrete constraints on the type of the result it should
		/// pass an appropriate resultParameter in its initialiser.
		Reader( const std::string name, const std::string description, ParameterPtr resultParameter = 0 );
		
		/// Returns the name of the file this Reader
		/// is set to read. Actually calls parameters()->parameter<FileNameParameter>( "fileName" )->getTypedValue();
		/// and therefore can potentially throw an Exception if the fileName is invalid.
		const std::string &fileName() const;
		
		/// Reads the file specified by fileName() and
		/// returns it in the form of an Object. This function
		/// actually just calls Op::operate() - it's provided for
		/// backwards compatibility and prettier syntax. 
		ObjectPtr read();
		
		/// Creates and returns a Reader appropriate to the specified file.
		/// Returns 0 if no suitable creator is found.
		static ReaderPtr create( const std::string &fileName );
		
		/// Fills the passed vector with all the extensions for which a Reader is
		/// available. Extensions are of the form "tif" - ie without a preceding '.'.
		static void supportedExtensions( std::vector<std::string> &extensions );
		
	protected :
	
		/// Definition of a function which can create a Reader when
		/// given a fileName.
		typedef ReaderPtr (*CreatorFn)( const std::string &fileName );
		/// Definition of a function  to answer the
		/// question can this file be read?
		typedef bool (*CanReadFn)( const std::string &fileName );
		
		/// Registers a Reader type which is capable of reading files ending with
		/// the space separated extensions specified (e.g. "tif tiff"). Before creating a reader the canRead function
		/// will be called as a final check that the Reader is appropriate - if this returns true
		/// then the creator function will then be called. If a file has no extension then all registered
		/// canRead functions are called in a last ditch attempt to find a suitable reader. Typically
		/// you will not call this function directly to register a reader type - you will instead use
		/// the registration utility class below.
		static void registerReader( const std::string &extensions, CanReadFn canRead, CreatorFn creator );
	
		/// This utility class is provided to help with Reader registration. By having a private static
		/// const instance of one of these in your class, it will call registerReader() for you when
		/// it is constructed. It assumes your Reader class has a constructor taking a fileName as 
		/// const std::string and also has a static canRead function matching the CanReadFn type.
		template<class T>
		class ReaderDescription
		{
			public :
				ReaderDescription( const std::string &extensions );
			private :
				static ReaderPtr creator( const std::string &fileName );
		};
		
		FileNameParameterPtr m_fileNameParameter;
	
	private :
		
		struct ReaderFns
		{
			CreatorFn creator;
			CanReadFn canRead;
		};
		typedef std::map<std::string, ReaderFns> ExtensionsToFnsMap;
		static ExtensionsToFnsMap *extensionsToFns();
		
};

} // namespace IECore

#include "IECore/Reader.inl"

#endif // IE_CORE_READER_H
