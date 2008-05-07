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

#include <string.h>

#include <OpenEXR/ImfInt64.h>
#include <OpenEXR/ImfXdr.h>
#include <boost/static_assert.hpp>

namespace IECore 
{
namespace IndexedIO
{
	
template<>
struct DataTypeTraits<float>
{
	static IndexedIO::DataType type() { return IndexedIO::Float; }
};
	
template<>
struct DataTypeTraits<float*>
{
	static IndexedIO::DataType type() { return IndexedIO::FloatArray; }
};

template<>
struct DataTypeTraits<double>
{
	static IndexedIO::DataType type() { return IndexedIO::Double; }
};

template<>
struct DataTypeTraits<double*>
{
	static IndexedIO::DataType type() { return IndexedIO::DoubleArray; }
};

template<>
struct DataTypeTraits<half>
{
	static IndexedIO::DataType type() { return IndexedIO::Half; }
};

template<>
struct DataTypeTraits<half*>
{
	static IndexedIO::DataType type() { return IndexedIO::HalfArray; }
};

template<>
struct DataTypeTraits<int>
{
	static IndexedIO::DataType type() { return IndexedIO::Int; }
};

template<>
struct DataTypeTraits<int*>
{
	static IndexedIO::DataType type() { return IndexedIO::IntArray; }
};

template<>
struct DataTypeTraits<long>
{
	static IndexedIO::DataType type() { return IndexedIO::Long; }
};

template<>
struct DataTypeTraits<long*>
{
	static IndexedIO::DataType type() { return IndexedIO::LongArray; }
};

template<>
struct DataTypeTraits<std::string>
{
	static IndexedIO::DataType type() { return IndexedIO::String; }
};

template<>
struct DataTypeTraits<std::string*>
{
	static IndexedIO::DataType type() { return IndexedIO::StringArray; }
};

template<>
struct DataTypeTraits<unsigned int>
{
	static IndexedIO::DataType type() { return IndexedIO::UInt; }
};

template<>
struct DataTypeTraits<unsigned int*>
{
	static IndexedIO::DataType type() { return IndexedIO::UIntArray; }
};

template<>
struct DataTypeTraits<char>
{
	static IndexedIO::DataType type() { return IndexedIO::Char; }
};

template<>
struct DataTypeTraits<char*>
{
	static IndexedIO::DataType type() { return IndexedIO::CharArray; }
};

template<>
struct DataTypeTraits<unsigned char>
{
	static IndexedIO::DataType type() { return IndexedIO::UChar; }
};

template<>
struct DataTypeTraits<unsigned char*>
{
	static IndexedIO::DataType type() { return IndexedIO::UCharArray; }
};

template<>
struct DataTypeTraits<short>
{
	static IndexedIO::DataType type() { return IndexedIO::Short; }
};

template<>
struct DataTypeTraits<short*>
{
	static IndexedIO::DataType type() { return IndexedIO::ShortArray; }
};

template<>
struct DataTypeTraits<unsigned short>
{
	static IndexedIO::DataType type() { return IndexedIO::UShort; }
};

template<>
struct DataTypeTraits<unsigned short*>
{
	static IndexedIO::DataType type() { return IndexedIO::UShortArray; }
};

/// Size

template<typename T>
struct DataSizeTraits<T*>
{
	static unsigned long size(const T *&x, unsigned long arrayLength)
	{ 
		return arrayLength * Imf::Xdr::size<T>();  
	}
};

template<>
struct DataSizeTraits<std::string>
{
	static unsigned long size(const std::string& x) 
	{ 	
		return x.length() + 1; 
	}
};

template<>
struct DataSizeTraits<std::string*>
{
	static unsigned long size(const std::string *&x, unsigned long arrayLength)
	{ 
		// String lengths are stored as unsigned longs
		unsigned long s = arrayLength * Imf::Xdr::size<unsigned long>();
		
		for (unsigned i = 0; i < arrayLength; i++)
		{
			s += x[i].size();
		}
		return s;
	}
};

template<typename T>
struct DataSizeTraits
{
	static unsigned long size(const T& x)
	{ 
		return Imf::Xdr::size<T>(); 
	}		
};

template<typename T>
struct DataTypeTraits
{
	static IndexedIO::DataType type()
	{ 
		assert(0);
		return IndexedIO::Invalid;
	}
};

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


// Flatten
template<typename T>
struct DataFlattenTraits
{
	static const char* flatten(const T& x)
	{ 
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
		
		char *data = new char[size];
		assert(data);
						
		OutputMemoryStream mstream(data);
		Imf::Xdr::write<MemoryStreamIO> (mstream, x);		
			
		return mstream.head();
	}
		
	static void unflatten(const char *src, T &dst)
	{		
		assert(src);
		InputMemoryStream mstream(src);
		Imf::Xdr::read<MemoryStreamIO>( mstream, dst );
	}
		
	static void free(const char *x)
	{
		assert(x);	
		delete[] x;
	}
};

template<typename T>
struct DataFlattenTraits<T*>
{
	static const char* flatten(const T* &x, unsigned long arrayLength)
	{ 	
		unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
		
		OutputMemoryStream mstream(new char[size]);
		
		for (unsigned long i = 0; i < arrayLength; i++)
		{
			Imf::Xdr::write<MemoryStreamIO> (mstream, x[i]);		
		}
			
		return mstream.head();
	}
	
	static void unflatten(const char *src, T *&dst, unsigned long arrayLength)
	{	
		if (!dst)
		{
			dst = new T[arrayLength];
		}
		
		InputMemoryStream mstream(src);
		
		for (unsigned long i = 0; i < arrayLength; i++)
		{
			Imf::Xdr::read<MemoryStreamIO>( mstream, dst[i] );
		}
	}

	static void free(const char *x)
	{	
		if (x)
		{
			delete[] x;
		}
	}
};


template<>
struct DataFlattenTraits<std::string>
{
	static const char* flatten(const std::string &x)
	{ 
		return x.c_str();	
	}
	
	static void unflatten(const char *x, std::string &dst)
	{
		assert(x);
		dst = std::string(x);
	}

	static void free(const char *x)
	{	
		// Nothing to free
	}
};

template<>
struct DataFlattenTraits<std::string*>
{
	static const char* flatten(const std::string *&x, unsigned long arrayLength)
	{ 
		unsigned long size = IndexedIO::DataSizeTraits<std::string*>::size(x, arrayLength);	
	
		OutputMemoryStream mstream(new char[size]);
		
		for (unsigned i = 0; i < arrayLength; i++)
		{
			unsigned long stringLength = x[i].size();
			
			Imf::Xdr::write<MemoryStreamIO> (mstream, stringLength);
		
			memcpy( mstream.next(), x[i].c_str(), stringLength );
			
			mstream.skip(stringLength);
		}
		
		return mstream.head();
	}
	
	static void unflatten(const char *src, std::string* &dst, unsigned long arrayLength)
	{
		if (!dst)
		{
			dst = new std::string[arrayLength];			
		}
		
		InputMemoryStream mstream(src);
		
		for (unsigned long i = 0; i < arrayLength; i++)
		{
			unsigned long stringLength = 0;
			Imf::Xdr::read<MemoryStreamIO> (mstream, stringLength);
				
			dst[i] = std::string(mstream.next(), stringLength);
			assert(dst[i].size() == stringLength);
			
			mstream.skip(stringLength);
		}
	}
	
	static void free(const char *x)
	{
		if (x)
		{
			// Delete the data we allocated in ::flatten
			delete[] x;
		}
	}
};

} // namespace IndexedIO
} // namespace IECore
