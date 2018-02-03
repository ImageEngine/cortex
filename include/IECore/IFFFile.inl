//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_IFFFILE_INL
#define IE_CORE_IFFFILE_INL

#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"

#include <cassert>
#include <vector>

namespace IECore
{

template<typename T>
void IFFFile::Chunk::read( T &data )
{
	if ( sizeof(T) != m_dataSize )
	{
		msg( Msg::Error, "IFFFile::Chunk::read()", boost::format( "Attempting to read data of size '%d' for a Chunk '%s' with dataSize '%d'." ) % sizeof(T) % m_type.name() % m_dataSize );
	}

	T dataBuffer[1];
	readData( dataBuffer, 1 );

	data = dataBuffer[0];
}

template<typename T>
size_t IFFFile::Chunk::read( std::vector<T> &data )
{
	size_t length = data.size();

	if ( sizeof(T) * length != m_dataSize )
	{
		msg( Msg::Error, "IFFFile::Chunk::read()", boost::format( "Attempting to read '%d' pieces of data of size '%d' for a Chunk '%s' with dataSize '%d'." ) % length % sizeof(T) % m_type.name() % m_dataSize );
	}

	std::vector<T> dataBuffer( length );
	readData( &dataBuffer[0], length );

	for ( size_t i = 0; i < length; i++ )
	{
		data[i] = dataBuffer[i];
	}

	return data.size();
}

template<typename T>
size_t IFFFile::Chunk::read( std::vector<Imath::Vec3<T> > &data )
{
	size_t length = data.size();

	if ( sizeof(T) * length * 3 != m_dataSize )
	{
		msg( Msg::Error, "IFFFile::Chunk::read()", boost::format( "Attempting to read %d pieces of IMath::Vec3 data of size %d for a Chunk '%s' with dataSize %d." ) % length % sizeof(T) % m_type.name() % m_dataSize );
	}

	std::vector<T> dataBuffer( length * 3 );
	readData( &dataBuffer[0], length * 3 );

	for ( size_t i = 0; i < length ; i++ )
	{
		data[ i ][0] = dataBuffer[ 3*i ];
		data[ i ][1] = dataBuffer[ 3*i + 1 ];
		data[ i ][2] = dataBuffer[ 3*i + 2 ];
	}

	return data.size();
}

template<typename T>
void IFFFile::Chunk::readData( T *dataBuffer, unsigned long n )
{
	m_file->m_iStream->seekg( m_filePosition, std::ios_base::beg );

	std::vector<char> buffer( m_dataSize );
	m_file->m_iStream->read( &buffer[0], m_dataSize );

	IFFFile::readData( &buffer[0], dataBuffer, n );
}

template<typename T>
void IFFFile::readData( const char *dataBuffer, T *attrBuffer, unsigned long n )
{
	for( unsigned long i=0; i < n; i++ )
	{
		T *data = (T*)dataBuffer;
		dataBuffer += sizeof( T );

		*attrBuffer = asBigEndian( *data );
		attrBuffer++;
	}
}

} // namespace IECore

#endif // IE_CORE_IFFFILE_INL
