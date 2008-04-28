//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_WRITER_H
#define IE_CORE_WRITER_H

#include "IECore/Op.h"
#include "IECore/ObjectParameter.h"

#include <map>
#include <vector>

namespace IECore
{

IE_CORE_FORWARDDECLARE( Writer );
IE_CORE_FORWARDDECLARE( FileNameParameter );

/// The Writer class defines an interface for creating
/// files of arbitrary type from Object instances.
class Writer : public Op
{

	public :

		IE_CORE_DECLARERUNTIMETYPED( Writer, Op )

		/// Creates and returns a Writer appropriate for saving the
		/// specified object to the specified file (the file extension is
		/// used to determine format). 
		/// Throws an Exception if no suitable writer can be found.
		static WriterPtr create( ObjectPtr object, const std::string &fileName );
		
		/// Returns the name of the file this Writer
		/// is set to create. This is just a convenience returning the equivalent of
		/// parameters()->parameter<FileNameParameter>( "fileName" )->getTypedValue().
		const std::string &fileName() const;
		/// Returns the Object this Writer will use
		/// to write the file. This is just a convenience returning the equivalent of
		/// parameters()->parameter<Parameter>( "object" )->getValue().
		ConstObjectPtr object() const;
		
		/// Writes object() to fileName(). This just calls operate() and is provided
		/// for backwards compatibility and pretty syntax.
		void write();
						
		/// Fills the passed vector with all the extensions for which a Writer is
		/// available. Extensions are of the form "tif" - ie without a preceding '.'.
		static void supportedExtensions( std::vector<std::string> &extensions );
		
	protected :
	
		Writer( const std::string &name, const std::string &description, TypeId writableType );
		Writer( const std::string &name, const std::string &description, const ObjectParameter::TypeIdSet &writableTypes );
	
		/// Implemented to call doWrite(), so derived classes need only implement that.
		ObjectPtr doOperation( ConstCompoundObjectPtr operands );
		
		/// Must be implemented by subclasses to write object() to fileName(). Implementation
		/// should throw an Exception on failure.
		/// \todo Surely doWrite should be passed the operands that are passed to doOperation?
		virtual void doWrite() = 0;
	
		/// Definition of a function which can create a Writer when
		/// given an object and fileName.
		typedef WriterPtr (*CreatorFn)( ObjectPtr object, const std::string &fileName );
		/// Definition of a function  to answer the
		/// question can this object be written to this file?
		typedef bool (*CanWriteFn)( ConstObjectPtr object, const std::string &fileName );
		
		/// Registers a Writer type which is capable of writing files ending with
		/// the space separated extensions specified (e.g. "tif tiff"). Before creating a Writer the canWrite function
		/// will be called as a final check that the Writer is appropriate for the given Object type - if this returns true
		/// then the creator function will then be called. Typically
		/// you will not call this function directly to register a writer type - you will instead use
		/// the registration utility class below.
		static void registerWriter( const std::string &extensions, CanWriteFn canWrite, CreatorFn creator );
	
		/// This utility class is provided to help with Writer registration. By having a private static
		/// const instance of one of these in your class, it will call registerWriter() for you when
		/// it is constructed. It assumes your Writer class has a constructor taking a ConstObjectPtr and a fileName as 
		/// const std::string and also has a static canWrite function matching the CanWriteFn type.
		template<class T>
		class WriterDescription
		{
			public :
				WriterDescription( const std::string &extensions );
			private :
				static WriterPtr creator( ObjectPtr object, const std::string &fileName );
		};
		
		ParameterPtr m_objectParameter;
		FileNameParameterPtr m_fileNameParameter;
	
	private :
		
		struct WriterFns
		{
			CreatorFn creator;
			CanWriteFn canWrite;
		};
		typedef std::multimap<std::string, WriterFns> ExtensionsToFnsMap;
		static ExtensionsToFnsMap *extensionsToFns();
		
		void constructParameters( void );
};

} // namespace IECore

#include "IECore/Writer.inl"

#endif // IE_CORE_WRITER_H
