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

#ifndef IE_CORE_EXCEPTION_H
#define IE_CORE_EXCEPTION_H

#include <string>
#include <exception>

#include "RefCounted.h"

namespace IECore
{
	/// The base class from which all IE core library exceptions should derive.
	class Exception : public std::exception
	{				
		public:
			/// Construct with cause of exception
			Exception(const char *what);
			Exception(const std::string &what);
		
			virtual ~Exception() throw ();
			
			// Return type of exception
			virtual const char *type() const throw();
		
			/// Return cause of exception
			virtual const char* what() const throw();
		
		protected:
		
			class RefCountedString : public RefCounted, public std::string
			{
				public:
					RefCountedString(const char * s) : std::string(s) {}
					RefCountedString(const std::string &s) : std::string(s) {}
			};
			
			IE_CORE_DECLAREPTR( RefCountedString )
	
			RefCountedStringPtr m_what;
	};
	
	/// Base class for Input/Output exceptions
	class IOException : public Exception
	{
		public:
			IOException(const char *what) : Exception(what) {};
			IOException(const std::string &what) : Exception(what) {};
			
			virtual const char *type() const throw() { return "I/O Exception"; }
	};
	
	/// A class to represent "file not found" exceptions
	class FileNotFoundIOException : public IOException
	{
		public:
			FileNotFoundIOException(const char *what) : IOException(what) {};
			FileNotFoundIOException(const std::string &what) : IOException(what) {};
			
			virtual const char *type() const throw() { return "File Not Found"; }			
	};
	
	/// Base class for Invalid Argument exceptions
	class InvalidArgumentException : public Exception
	{
		public:
			InvalidArgumentException(const char *what) : Exception(what) {};
			InvalidArgumentException(const std::string &what) : Exception(what) {};
			
			virtual const char *type() const throw() { return "Invalid Argument"; }
	};
	
	/// A class to represent "permission denied" exceptions
	class PermissionDeniedIOException : public IOException
	{
		public:
			PermissionDeniedIOException(const char *what) : IOException(what) {};
			PermissionDeniedIOException(const std::string &what) : IOException(what) {};
			
			virtual const char *type() const throw() { return "Permission Denied"; }			
	};
}

#endif // IE_CORE_EXCEPTION_H
