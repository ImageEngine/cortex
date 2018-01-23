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
//	     other contributors to this software may be used to endorse or
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

#include "OpenEXR/ImfInt64.h"
#include "OpenEXR/ImfXdr.h"

#include "boost/static_assert.hpp"

#include <stdint.h>
#include <string.h>

namespace IECore
{


template<>
struct IndexedIO::DataTypeTraits<float>
{
	static IndexedIO::DataType type() { return IndexedIO::Float; }
};

template<>
struct IndexedIO::DataTypeTraits<float*>
{
	static IndexedIO::DataType type() { return IndexedIO::FloatArray; }
};

template<>
struct IndexedIO::DataTypeTraits<double>
{
	static IndexedIO::DataType type() { return IndexedIO::Double; }
};

template<>
struct IndexedIO::DataTypeTraits<double*>
{
	static IndexedIO::DataType type() { return IndexedIO::DoubleArray; }
};

template<>
struct IndexedIO::DataTypeTraits<half>
{
	static IndexedIO::DataType type() { return IndexedIO::Half; }
};

template<>
struct IndexedIO::DataTypeTraits<half*>
{
	static IndexedIO::DataType type() { return IndexedIO::HalfArray; }
};

template<>
struct IndexedIO::DataTypeTraits<int>
{
	static IndexedIO::DataType type() { return IndexedIO::Int; }
};

template<>
struct IndexedIO::DataTypeTraits<int*>
{
	static IndexedIO::DataType type() { return IndexedIO::IntArray; }
};

template<>
struct IndexedIO::DataTypeTraits<int64_t>
{
	static IndexedIO::DataType type() { return IndexedIO::Int64; }
};

template<>
struct IndexedIO::DataTypeTraits<int64_t*>
{
	static IndexedIO::DataType type() { return IndexedIO::Int64Array; }
};

template<>
struct IndexedIO::DataTypeTraits<std::string>
{
	static IndexedIO::DataType type() { return IndexedIO::String; }
};

template<>
struct IndexedIO::DataTypeTraits<std::string*>
{
	static IndexedIO::DataType type() { return IndexedIO::StringArray; }
};

template<>
struct IndexedIO::DataTypeTraits<unsigned int>
{
	static IndexedIO::DataType type() { return IndexedIO::UInt; }
};

template<>
struct IndexedIO::DataTypeTraits<unsigned int*>
{
	static IndexedIO::DataType type() { return IndexedIO::UIntArray; }
};

template<>
struct IndexedIO::DataTypeTraits<uint64_t>
{
	static IndexedIO::DataType type() { return IndexedIO::UInt64; }
};

template<>
struct IndexedIO::DataTypeTraits<uint64_t*>
{
	static IndexedIO::DataType type() { return IndexedIO::UInt64Array; }
};

template<>
struct IndexedIO::DataTypeTraits<char>
{
	static IndexedIO::DataType type() { return IndexedIO::Char; }
};

template<>
struct IndexedIO::DataTypeTraits<char*>
{
	static IndexedIO::DataType type() { return IndexedIO::CharArray; }
};

template<>
struct IndexedIO::DataTypeTraits<unsigned char>
{
	static IndexedIO::DataType type() { return IndexedIO::UChar; }
};

template<>
struct IndexedIO::DataTypeTraits<unsigned char*>
{
	static IndexedIO::DataType type() { return IndexedIO::UCharArray; }
};

template<>
struct IndexedIO::DataTypeTraits<short>
{
	static IndexedIO::DataType type() { return IndexedIO::Short; }
};

template<>
struct IndexedIO::DataTypeTraits<short*>
{
	static IndexedIO::DataType type() { return IndexedIO::ShortArray; }
};

template<>
struct IndexedIO::DataTypeTraits<unsigned short>
{
	static IndexedIO::DataType type() { return IndexedIO::UShort; }
};

template<>
struct IndexedIO::DataTypeTraits<unsigned short*>
{
	static IndexedIO::DataType type() { return IndexedIO::UShortArray; }
};


template<typename T>
struct IndexedIO::DataTypeTraits
{
	static IndexedIO::DataType type()
	{
		assert(0);
		return IndexedIO::Invalid;
	}
};

namespace IndexedIODetail
{

// Classes which crudely mimic a regular iostream, but read/write from/to a memory buffer.

class InputMemoryStream
{
	public:
		InputMemoryStream(const char *p)
		{
			m_next = p;
			m_head = p;
		}

		void read(char c[], int n)
		{
			memcpy(c, m_next, n);

			m_next += n;
		}

		const char *head() { return m_head; }
		const char *next() { return m_next; }
		void skip(unsigned long n) { m_next += n; }

	protected:

		InputMemoryStream(const InputMemoryStream &other) {}

		const char *m_next;
		const char *m_head;
};

class OutputMemoryStream
{
	public:
		OutputMemoryStream(char *p)
		{
			m_next = p;
			m_head = p;
		}

		void write(const char c[], int n)
		{
			memcpy(m_next, c, n );

			m_next += n;
		}

		const char *head() { return m_head; }
		char *next() { return m_next; }
		void skip(unsigned long n) { m_next += n; }

	protected:

		OutputMemoryStream(const OutputMemoryStream &other) {}

		char *m_next;
		const char *m_head;
};


// An implementation of an Imf::Xdr reader/writer
struct MemoryStreamIO
{
	static void
	writeChars (OutputMemoryStream &o, const char c[], int n)
	{
		o.write(c, n);
	}

	static void
	readChars (InputMemoryStream &i, char c[], int n)
	{
		i.read(c, n);
	}
};

template<typename T>
inline int size()
{
	return Imf::Xdr::size<T>();
}

template<>
inline int size<int64_t>()
{
	return 8;
}

template<>
inline int size<uint64_t>()
{
	return 8;
}

template<class S, class T, class V>
struct Writer
{
	static void write( T &out, V v )
	{
		Imf::Xdr::write<S, T>( out, v );
	}
};

template<class S, class T>
struct Writer<S, T, int64_t>
{
	static void write( T &out, int64_t v )
	{
		signed char b[8];

		b[0] =  (signed char) (v);
		b[1] =  (signed char) (v >> 8);
		b[2] =  (signed char) (v >> 16);
		b[3] =  (signed char) (v >> 24);
		b[4] =  (signed char) (v >> 32);
		b[5] =  (signed char) (v >> 40);
		b[6] =  (signed char) (v >> 48);
		b[7] =  (signed char) (v >> 56);

		Imf::Xdr::writeSignedChars<S> (out, b, 8);
	}
};

template<class S, class T>
struct Writer<S, T, uint64_t>
{
	static void write( T &out, uint64_t v )
	{
		unsigned char b[8];

		b[0] =  (unsigned char) (v);
		b[1] =  (unsigned char) (v >> 8);
		b[2] =  (unsigned char) (v >> 16);
		b[3] =  (unsigned char) (v >> 24);
		b[4] =  (unsigned char) (v >> 32);
		b[5] =  (unsigned char) (v >> 40);
		b[6] =  (unsigned char) (v >> 48);
		b[7] =  (unsigned char) (v >> 56);

		Imf::Xdr::writeUnsignedChars<S> (out, b, 8);
	}
};

template<class S, class T, class V>
struct Reader
{
	static void read( T &in, V &v )
	{
		Imf::Xdr::read<S, T>( in, v );
	}
};

template<class S, class T>
struct Reader<S, T, int64_t>
{
	static void read( T &in, int64_t &v )
	{
		signed char b[8];

		Imf::Xdr::readSignedChars<S> (in, b, 8);

		v = (int64_t(b[0])     & 0x00000000000000ffLL) |
		((int64_t(b[1]) << 8)  & 0x000000000000ff00LL) |
		((int64_t(b[2]) << 16) & 0x0000000000ff0000LL) |
		((int64_t(b[3]) << 24) & 0x00000000ff000000LL) |
		((int64_t(b[4]) << 32) & 0x000000ff00000000LL) |
		((int64_t(b[5]) << 40) & 0x0000ff0000000000LL) |
		((int64_t(b[6]) << 48) & 0x00ff000000000000LL) |
		((int64_t(b[7]) << 56) & 0xff00000000000000LL);
	}
};


template<class S, class T>
struct Reader<S, T, uint64_t>
{
	static void read( T &in, uint64_t &v )
	{
		unsigned char b[8];

		Imf::Xdr::readUnsignedChars<S> (in, b, 8);

		v = (uint64_t(b[0])     & 0x00000000000000ffLL) |
		((uint64_t(b[1]) << 8)  & 0x000000000000ff00LL) |
		((uint64_t(b[2]) << 16) & 0x0000000000ff0000LL) |
		((uint64_t(b[3]) << 24) & 0x00000000ff000000LL) |
		((uint64_t(b[4]) << 32) & 0x000000ff00000000LL) |
		((uint64_t(b[5]) << 40) & 0x0000ff0000000000LL) |
		((uint64_t(b[6]) << 48) & 0x00ff000000000000LL) |
		((uint64_t(b[7]) << 56) & 0xff00000000000000LL);
	}
};

} // namespace IndexedIODetail

/// Size

template<typename T>
struct IndexedIO::DataSizeTraits<T*>
{
	static unsigned long size(const T *&x, unsigned long arrayLength)
	{
		return arrayLength * IndexedIODetail::size<T>();
	}
};

template<>
struct IndexedIO::DataSizeTraits<std::string>
{
	static unsigned long size(const std::string& x)
	{
		return x.length() + 1;
	}
};

template<>
struct IndexedIO::DataSizeTraits<std::string*>
{
	static unsigned long size(const std::string *&x, unsigned long arrayLength)
	{
		// String lengths are stored as unsigned longs
		unsigned long s = arrayLength * IndexedIODetail::size<unsigned long>();

		for (unsigned i = 0; i < arrayLength; i++)
		{
			s += x[i].size();
		}
		return s;
	}
};

template<typename T>
struct IndexedIO::DataSizeTraits
{
	static unsigned long size(const T& x)
	{
		return IndexedIODetail::size<T>();
	}
};

// Flatten
template<typename T>
struct IndexedIO::DataFlattenTraits
{
	static void flatten(const T& x, char *dst)
	{
		IndexedIODetail::OutputMemoryStream mstream(dst);
		IndexedIODetail::Writer<IndexedIODetail::MemoryStreamIO, IndexedIODetail::OutputMemoryStream, T>::write(mstream, x);
	}

	static void unflatten(const char *src, T &dst)
	{
		assert(src);
		IndexedIODetail::InputMemoryStream mstream(src);
		IndexedIODetail::Reader<IndexedIODetail::MemoryStreamIO, IndexedIODetail::InputMemoryStream, T>::read( mstream, dst );
	}

};


template<typename T>
struct IndexedIO::DataFlattenTraits<T*>
{
	static void flatten(const T* &x, unsigned long arrayLength, char *dst )
	{
		IndexedIODetail::OutputMemoryStream mstream(dst);

		for (unsigned long i = 0; i < arrayLength; i++)
		{
			IndexedIODetail::Writer<IndexedIODetail::MemoryStreamIO, IndexedIODetail::OutputMemoryStream, T>::write (mstream, x[i]);
		}
	}

	static void unflatten(const char *src, T *&dst, unsigned long arrayLength)
	{
		if (!dst)
		{
			dst = new T[arrayLength];
		}

		IndexedIODetail::InputMemoryStream mstream(src);

		for (unsigned long i = 0; i < arrayLength; i++)
		{
			IndexedIODetail::Reader<IndexedIODetail::MemoryStreamIO, IndexedIODetail::InputMemoryStream, T>::read( mstream, dst[i] );
		}
	}
};

template<>
struct IndexedIO::DataFlattenTraits<std::string>
{
	static void flatten(const std::string &x, char *dst)
	{
		strcpy( dst, x.c_str() );
	}

	static void unflatten(const char *x, std::string &dst)
	{
		assert(x);
		dst = std::string(x);
	}

};

template<>
struct IndexedIO::DataFlattenTraits<std::string*>
{
	static void flatten(const std::string *&x, unsigned long arrayLength, char *dst)
	{
		IndexedIODetail::OutputMemoryStream mstream(dst);

		for (unsigned i = 0; i < arrayLength; i++)
		{
			unsigned long stringLength = x[i].size();

			IndexedIODetail::Writer<IndexedIODetail::MemoryStreamIO, IndexedIODetail::OutputMemoryStream, unsigned long>::write (mstream, stringLength);

			memcpy( mstream.next(), x[i].c_str(), stringLength );

			mstream.skip(stringLength);
		}
	}

	static void unflatten(const char *src, std::string* &dst, unsigned long arrayLength)
	{
		if (!dst)
		{
			dst = new std::string[arrayLength];
		}

		IndexedIODetail::InputMemoryStream mstream(src);

		for (unsigned long i = 0; i < arrayLength; i++)
		{
			unsigned long stringLength = 0;
			IndexedIODetail::Reader<IndexedIODetail::MemoryStreamIO, IndexedIODetail::InputMemoryStream, unsigned long>::read (mstream, stringLength);

			dst[i] = std::string(mstream.next(), stringLength);
			assert(dst[i].size() == stringLength);

			mstream.skip(stringLength);
		}
	}
};

} // namespace IECore
