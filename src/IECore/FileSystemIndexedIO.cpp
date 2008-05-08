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

#include <cassert>
#include <iostream>
#include <string>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp"

#include "IECore/Exception.h"
#include "IECore/FileSystemIndexedIO.h"

using namespace IECore;
namespace fs = boost::filesystem;

static IndexedIOInterface::Description<FileSystemIndexedIO> registrar(".fs");

IndexedIOInterfacePtr FileSystemIndexedIO::create(const std::string &path, const IndexedIO::EntryID&root, IndexedIO::OpenMode mode)
{
	return new FileSystemIndexedIO(path, root, mode);
}

FileSystemIndexedIO::FileSystemIndexedIO(const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode)
{
	validateOpenMode(mode);
	m_mode = mode;
	
	IndexedIOPath tmp( path );
	tmp.append(root);
	
	
	m_currentDirectory = IndexedIOPath( tmp.fullPath() );
		
	std::string d = m_currentDirectory.appended(".").fullPath();

	if (mode & IndexedIO::Read) 
	{
		// Reading
		if (!fs::exists(d))
		{
			throw IOException(d);
		}
	}
	else
	{		
		// Writing
		if (mode & IndexedIO::Write)
		{
			if (fs::exists(d))
			{
				rm(".");
			}
		}
		
		mkdir(".");
	}
	
	if (!fs::exists(d))
	{
		throw IOException(d);
	}
	
	chdir(".");
}	
		
FileSystemIndexedIO::~FileSystemIndexedIO()
{
}

IndexedIO::OpenMode FileSystemIndexedIO::openMode() const
{
	return m_mode;
}

IndexedIOInterfacePtr FileSystemIndexedIO::resetRoot() const
{	
	IndexedIO::OpenMode mode = m_mode;
	
	if (mode & IndexedIO::Write)
	{
		mode &= ~IndexedIO::Write;
		assert( (mode & IndexedIO::Shared) == (m_mode & IndexedIO::Shared) );
		assert( (mode & IndexedIO::Exclusive) == (m_mode & IndexedIO::Exclusive) );
		assert( !(mode & IndexedIO::Write) );	
		mode |= IndexedIO::Append;
	}
	
	return new FileSystemIndexedIO( m_currentDirectory.fullPath(), "/", mode);
}
		
void FileSystemIndexedIO::chdir(const IndexedIO::EntryID &name)
{
	readable(name);
	
	const std::string d = m_currentDirectory.appended(name).fullPath();
		
	if ( !fs::exists( d ) )
	{
		throw FileNotFoundIOException(name);
	}

	m_currentDirectory.append(name);
}
	
void FileSystemIndexedIO::mkdir(const IndexedIO::EntryID &name)
{
	const std::string d = m_currentDirectory.appended(name).fullPath();
	
	if ( fs::exists( d ) )
	{
		return;
	}
	
	fs::create_directory(d);
	
	if ( !fs::exists( d ) )
	{
		throw IOException(name);
	}
}
		
IndexedIO::EntryID FileSystemIndexedIO::pwd()
{
	readable(".");
	
	return m_currentDirectory.relativePath();
}

IndexedIO::EntryList FileSystemIndexedIO::ls(IndexedIOFilterPtr f)
{	
	IndexedIO::EntryList result;
	
	readable(".");
	
	const std::string fullPath = m_currentDirectory.fullPath();
	if ( !fs::exists( fullPath ) )
	{
		throw FileNotFoundIOException( fullPath );
	}
	
	fs::directory_iterator dirIt (fullPath);
	const fs::directory_iterator endIt;
	for (; dirIt != endIt; ++dirIt)
	{
		if ( fs::is_directory( *dirIt ) )
		{			
			result.push_back( IndexedIO::Entry( dirIt->leaf(), IndexedIO::Directory, IndexedIO::Invalid, 0 ) );
		}
		else
		{			
			fs::fstream f( *dirIt, std::ios::binary | std::ios::in );
			
			if (! f.is_open() || !f.good())
			{
				throw IOException( fullPath );
			}
			
			unsigned long size = 0;
			IndexedIO::DataType datatype = IndexedIO::Invalid;
	
			f.read( reinterpret_cast<char *>(&size), sizeof(unsigned long ) );
			if (f.fail()) throw IOException( fullPath );
			
			unsigned long arrayLength = 0;
			f.read( reinterpret_cast<char *>(&arrayLength), sizeof(unsigned long) );
			if (f.fail()) throw IOException( fullPath );
				
			f.read( reinterpret_cast<char *>(&datatype), sizeof(IndexedIO::DataType) );
			if (f.fail()) throw IOException( fullPath );
			
			f.close();
			
			result.push_back( IndexedIO::Entry( dirIt->leaf(), IndexedIO::File, datatype, arrayLength) );
		}
	}
	
	if (f)
	{
		f->apply(result);
	}	
	
	return result;
}

unsigned long FileSystemIndexedIO::rm(const IndexedIO::EntryID &name)
{
	writable(name);
	
	const std::string d = m_currentDirectory.appended(name).fullPath();
	
	if ( !fs::exists( d ) )
	{
		throw FileNotFoundIOException(name);
	}
	
	return fs::remove_all( d );
}

IndexedIO::Entry FileSystemIndexedIO::ls(const IndexedIO::EntryID &name)
{
	readable(name);
	
	const std::string p = m_currentDirectory.appended(name).fullPath();
	
	if ( !fs::exists( p ) )
	{
		throw FileNotFoundIOException(name);
	}
	
	if ( fs::is_directory( p ) )
	{
		return IndexedIO::Entry( name, IndexedIO::Directory, IndexedIO::Invalid, 0 );
	}
	
	fs::fstream f( p, std::ios::binary | std::ios::in );
	if (!f.is_open() || f.fail()) throw IOException(name);
			
	unsigned long size = 0;
	IndexedIO::DataType datatype = IndexedIO::Invalid;
	
	f.read( reinterpret_cast<char *>(&size), sizeof(unsigned long) );
	if (f.fail()) throw IOException(name);
	
	unsigned long arrayLength = 0;	
	f.read( reinterpret_cast<char *>(&arrayLength), sizeof(unsigned long) );
	if (f.fail()) throw IOException(name);
		
	f.read( reinterpret_cast<char *>(&datatype), sizeof(IndexedIO::DataType) );
	if (f.fail()) throw IOException(name);
			
	f.close();
	
	return IndexedIO::Entry(name, IndexedIO::File, datatype, arrayLength);
}

/// Write an array of POD types
template<typename T>
void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	writable(name);
	
	const IndexedIO::DataType datatype = IndexedIO::DataTypeTraits<T*>::type();
	
	const std::string p = m_currentDirectory.appended(name).fullPath();
	
	if (fs::exists(p))
	{
		if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
		{
			rm(name);
		}
		else
		{
			throw IOException(name);
		}
	}

	fs::fstream f( p , std::ios::binary | std::ios::out );
	
	if (! f.is_open() )
	{
		throw IOException(name);
	}

	unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
	
	f.write( reinterpret_cast<const char *>(&size), sizeof(unsigned long) );
	if (f.fail()) throw IOException(name);
		
	f.write( reinterpret_cast<const char *>(&arrayLength), sizeof(unsigned long) );
	if (f.fail()) throw IOException(name);
		
	f.write( reinterpret_cast<const char *>(&datatype), sizeof(IndexedIO::DataType) );
	if (f.fail()) throw IOException(name);
		
	const char *data = IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength);
		
	f.write(data, size);
	
	IndexedIO::DataFlattenTraits<T*>::free(data);
	
	if (f.fail()) throw IOException(name);
		
	f.close();
}

/// Read an array of POD types
template<typename T>
void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	readable(name);
	
	const std::string p = m_currentDirectory.appended(name).fullPath();
	
	if (! fs::exists(p))
	{		
		throw FileNotFoundIOException(name);
	}
	
	fs::fstream f( p , std::ios::binary | std::ios::in);
	if (! f.is_open() || !f.good() )
	{
		throw IOException(name);
	}
	
	try
	{	
		unsigned long size = 0;
		IndexedIO::DataType datatype = IndexedIO::Invalid;
				
		f.read( reinterpret_cast<char *>(&size), sizeof(unsigned long) );
		if (f.fail()) throw IOException(name);
			
		unsigned long length = 0;
		f.read( reinterpret_cast<char *>(&length), sizeof(unsigned long) );
		if (f.fail()) throw IOException(name);
			
		if (length != arrayLength)
		{
			throw IOException(name);
		}
			
		f.read( reinterpret_cast<char *>(&datatype), sizeof(IndexedIO::DataType) );
		if (f.fail()) throw IOException(name);
		
		if (datatype != IndexedIO::DataTypeTraits<T*>::type())
		{
			throw IOException(name);
		}
		
		char *r = 0;
		try
		{
			r = new char[size];
			f.read(r, size);
			if (f.fail()) throw IOException(name);
		
			IndexedIO::DataFlattenTraits<T*>::unflatten(r, x, arrayLength);
		}
		catch (...)
		{
			if (r)
			{
				delete [] r;
			}
			throw;
		}
		
		delete[] r;
	}	
	catch (std::exception &e)
	{
		f.close();
		throw;
	}

	f.close();
}

/// Write a POD type
template<typename T>
void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);

	IndexedIO::DataType datatype = IndexedIO::DataTypeTraits<T>::type();
	
	std::string p = m_currentDirectory.appended(name).fullPath();
	if (fs::exists(p))
	{
		if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
		{
			rm(name);
		}
		else
		{
			throw IOException(name);
		}
	}
	
	fs::fstream f( p , std::ios::binary | std::ios::out);
	
	if (! f.is_open() )
	{
		throw IOException(name);
	}

	try
	{		
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
	
		f.write( reinterpret_cast<const char *>(&size), sizeof(unsigned long) );
		if (f.fail()) throw IOException(name);
			
		unsigned long arrayLength = 0;
		f.write( reinterpret_cast<const char *>(&arrayLength), sizeof(unsigned long) );
		if (f.fail()) throw IOException(name);
	
		f.write( reinterpret_cast<const char *>(&datatype), sizeof(IndexedIO::DataType) );
		if (f.fail()) throw IOException(name);
			
		const char *data = IndexedIO::DataFlattenTraits<T>::flatten(x);
		f.write( data, size );
		
		//IndexedIO::DataFlattenTraits<T>::free(data);
		if (f.fail()) throw IOException(name);			
	} 
	catch (std::exception &e)
	{
		f.close();
		
		throw;
	}

	f.close();		
}
		
/// Read a POD type
template<typename T>
void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, T &x) const
{
	readable(name);
	
	const std::string p = m_currentDirectory.appended(name).fullPath();
	
	if (! fs::exists(p))
	{
		throw FileNotFoundIOException(name);
	}
	
	fs::fstream f( p , std::ios::binary | std::ios::in);
	if (! f.is_open() || !f.good() )
	{
		throw IOException(name);
	}
	
	try
	{	
		unsigned long size = 0;
		IndexedIO::DataType datatype = IndexedIO::Invalid;
	
		f.read( reinterpret_cast<char *>(&size), sizeof(unsigned long) );
		if (f.fail()) throw IOException(name);
			
		unsigned long arrayLength = 0;
		f.read( reinterpret_cast<char *>(&arrayLength), sizeof(unsigned long) );
		if (f.fail()) throw IOException(name);
	
		f.read( reinterpret_cast<char *>(&datatype), sizeof(IndexedIO::DataType) );
		if (f.fail()) throw IOException(name);
	
		if (datatype != IndexedIO::DataTypeTraits<T>::type())
		{
			throw IOException(name);
		}

		char *r = 0;
		try
		{
			r = new char[size];			
			f.read(r, size);
			if (f.fail()) throw IOException(name);		 
			
			IndexedIO::DataFlattenTraits<T>::unflatten( r, x );
		} 
		catch (...)
		{
			if (r)
			{
				delete[] r;
			}
			throw;
		}
		
		delete[] r;
	}
	catch (std::exception &e)
	{
		f.close();
		
		throw;
	}	
	
	f.close();
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const float *x, unsigned long arrayLength)
{
	write<float>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const double *x, unsigned long arrayLength)
{	
	write<double>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const half *x, unsigned long arrayLength)
{	
	write<half>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const int *x, unsigned long arrayLength)
{
	write<int>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const int64_t *x, unsigned long arrayLength)
{
	write<int64_t>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const uint64_t *x, unsigned long arrayLength)
{
	write<uint64_t>(name, x, arrayLength);
}
	
void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const float &x)
{
	write<float>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const double &x)
{
	write<double>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const half &x)
{
	write<half>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const int &x)
{
	write<int>(name, x);
}	

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const int64_t &x)
{
	write<int64_t>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const uint64_t &x)
{
	write<uint64_t>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const std::string &x)
{
	write<std::string>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int &x)
{
	write<unsigned int>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const char &x)
{
	write<char>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char &x)
{
	write<unsigned char>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const short &x)
{
	write<short>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const unsigned short &x)
{
	write<unsigned short>(name, x);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int *x, unsigned long arrayLength)
{
	write<unsigned int>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const char *x, unsigned long arrayLength)
{
	write<char>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char *x, unsigned long arrayLength)
{
	write<unsigned char>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const std::string *x, unsigned long arrayLength)
{
	write<std::string>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const short *x, unsigned long arrayLength)
{
	write<short>(name, x, arrayLength);
}

void FileSystemIndexedIO::write(const IndexedIO::EntryID &name, const unsigned short *x, unsigned long arrayLength)
{
	write<unsigned short>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength)
{
	read<float>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength)
{
	read<double>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength)
{
	read<half>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength)
{
	read<int>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, int64_t *&x, unsigned long arrayLength)
{
	read<int64_t>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, uint64_t *&x, unsigned long arrayLength)
{
	read<uint64_t>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength)
{
	read<unsigned int>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength)
{
	read<char>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength)
{
	read<unsigned char>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, short *&x, unsigned long arrayLength)
{
	read<short>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, unsigned short *&x, unsigned long arrayLength)
{
	read<unsigned short>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength)
{
	read<std::string>(name, x, arrayLength);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, float &x)
{
	read<float>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, double &x)
{
	read<double>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, half &x)
{
	read<half>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, int &x)
{
	read<int>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, int64_t &x)
{
	read<int64_t>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, uint64_t &x)
{
	read<uint64_t>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, std::string &x)
{
	read<std::string>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, unsigned int &x)
{
	read<unsigned int>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, char &x)
{
	read<char>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, unsigned char &x)
{
	read<unsigned char>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, short &x)
{
	read<short>(name, x);
}

void FileSystemIndexedIO::read(const IndexedIO::EntryID &name, unsigned short &x)
{
	read<unsigned short>(name, x);
}
