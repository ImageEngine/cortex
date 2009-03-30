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

#ifndef IE_CORE_MEMORYSTREAM_H
#define IE_CORE_MEMORYSTREAM_H

#include "boost/iostreams/categories.hpp"
#include "boost/iostreams/detail/ios.hpp"
#include "boost/iostreams/detail/fstream.hpp"
#include "boost/iostreams/operations.hpp"

#include "IECore/RefCounted.h"

#include "IECore/LRUCache.h"

namespace IECore
{

/// A boost.iostreams "device" for reading to/writing from memory.
class MemoryStream
{
	public:
		struct category
		: public boost::iostreams::device_tag
	        { };
		
		MemoryStream();
	
		/// Construct a new stream pointing to the existing buffer. Optionally, ownership of this
		/// buffer can be passed to the stream. In this case it is assumed that it was allocated
		/// with new[] and can safely be deleted with delete[].
		MemoryStream( char *buf, std::streamsize sz, bool ownsBuf = false );
				
		/// Read from the stream		
		std::streamsize read(char* s, std::streamsize n);
		
		/// Write to the stream
		std::streamsize write(const char* s, std::streamsize n);
		
		/// Retrieve a reference to the contents of the stream.
		void get( char *& data, std::streamsize &sz ) const;
		
	private:
	
		struct Impl : public IECore::RefCounted
		{
			Impl();

			Impl( char *buf, std::streamsize sz, bool ownsBuf = false );

			~Impl();

			std::streamsize read(char* s, std::streamsize n);

			std::streamsize write(const char* s, std::streamsize n);
			
			char *m_head;
			char *m_next;
		
			std::streamsize m_size;
			std::streamsize m_bufSize;
		
			bool m_ownsBuf;
		};
		
		IE_CORE_DECLAREPTR( Impl );
		ImplPtr m_impl;	
};

struct MemoryStreamSource : private MemoryStream
{
	typedef char                          char_type;
	typedef boost::iostreams::source_tag  category;
	
	using MemoryStream::read;
	using MemoryStream::get;
	
	MemoryStreamSource(char *buf, std::streamsize sz, bool ownsBuf = false);
};

struct MemoryStreamSink : private MemoryStream
{
	typedef char                        char_type;
	typedef boost::iostreams::sink_tag  category;
	
	using MemoryStream::write;
	using MemoryStream::get;
	
	MemoryStreamSink();
};

} // namespace IECore

#endif // IE_CORE_MEMORYSTREAM_H
