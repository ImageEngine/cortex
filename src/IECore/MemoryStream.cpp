//////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//   * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//   * Neither the name of Image Engine Design nor the names of any
//    other contributors to this software may be used to endorse or
//    promote products derived from this software without specific prior
//    written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <math.h>

#include "IECore/Exception.h"
#include "IECore/MemoryStream.h"

using namespace IECore;

MemoryStream::MemoryStream()
{
	m_impl = new Impl();
}

MemoryStream::MemoryStream( char *buf, std::streamsize sz, bool ownsBuf )
{
	m_impl = new Impl( buf, sz, ownsBuf );
}

std::streamsize MemoryStream::read(char* s, std::streamsize n)
{
	return m_impl->read( s, n );
}

std::streamsize MemoryStream::write(const char* s, std::streamsize n)
{
	return m_impl->write( s, n );
}

void MemoryStream::get( char *& data, std::streamsize &sz ) const
{
	data = m_impl->m_head;
	sz = m_impl->m_size;
}

MemoryStream::Impl::Impl()
{
	m_bufSize = 16384; // Modest initial size, arbitrarily chosen
	m_head = new char[m_bufSize];
	m_size = 0;
	m_next = m_head;
	m_ownsBuf = true;
}

MemoryStream::Impl::Impl( char *buf, std::streamsize sz, bool ownsBuf ) : m_head(buf), m_next(buf), m_size(sz), m_bufSize(sz), m_ownsBuf(ownsBuf)
{
}

MemoryStream::Impl::~Impl()
{
	if (m_ownsBuf)
	{
		delete[] m_head;
	}
}

std::streamsize MemoryStream::Impl::read(char* s, std::streamsize n)
{
	if (m_next - m_head + n > m_size)
	{
		// Underflow, so read as many as we can, return num read
		n = m_size - (m_next - m_head);

		if (n == 0)
		{
			return EOF;
		}
	}

	assert( s );
	assert( m_next );
	memcpy( s, m_next, n );

	m_next += n;

	return n;
}

std::streamsize MemoryStream::Impl::write(const char* s, std::streamsize n)
{
	if (m_size + n > m_bufSize)
	{
		// Overflow, so realloc the buffer, using an exponential resizing strategy
		char *newbuf = new char[ m_bufSize*2 ];
		assert( m_head );
		memcpy( newbuf, m_head, m_size );

		if (m_ownsBuf)
		{
			delete[] m_head;
		}
		m_bufSize *= 2;


		m_next = newbuf + ( m_next - m_head );
		m_head = newbuf;
		m_ownsBuf = true;
	}
	// copy in
	memcpy( m_next, s, n );
	m_next += n;
	m_size += n;
	return n;
}

MemoryStreamSource::MemoryStreamSource(char *buf,  std::streamsize sz, bool ownsBuf)
: MemoryStream( buf, sz, ownsBuf )
{
}

MemoryStreamSink::MemoryStreamSink()
: MemoryStream()
{
}
