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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include <cassert>
#include <iostream>

#include "IECore/IndexedIOInterface.h"
#include "IECore/FileSystemIndexedIO.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/MemoryIndexedIO.h"
#include "IECore/VectorTypedData.h"

#include "IECore/bindings/RefCountedBinding.h"

using namespace boost::python;
using namespace IECore;

void bindIndexedIOEntry(const char *bindName);
void bindIndexedIOEntryList(const char *bindName);

void bindIndexedIOFilter(const char *bindName);
void bindIndexedIONullFilter(const char *bindName);
void bindIndexedIOEntryTypeFilter(const char *bindName);
void bindIndexedIORegexFilter(const char *bindName);

void bindIndexedIOInterface(const char *bindName);
void bindFileSystemIndexedIO(const char *bindName);
void bindFileIndexedIO(const char *bindName);
void bindMemoryIndexedIO(const char *bindName);

void bindIndexedIO()
{
	bindIndexedIOEntry("IndexedIOEntry");
	bindIndexedIOEntryList("IndexedIOEntryList");

	bindIndexedIOFilter("IndexedIOFilter");
	bindIndexedIONullFilter("IndexedIONullFilter");
	bindIndexedIOEntryTypeFilter("IndexedIOEntryTypeFilter");
	bindIndexedIORegexFilter("IndexedIORegexFilter");

	bindIndexedIOInterface("IndexedIOInterface");
	bindFileSystemIndexedIO("FileSystemIndexedIO");
	bindFileIndexedIO("FileIndexedIO");
	bindMemoryIndexedIO("MemoryIndexedIO");
}

struct IndexedIOInterfaceHelper
{
	static IndexedIO::Entry ls(IndexedIOInterfacePtr p, const IndexedIO::EntryID &name)
	{
		assert(p);

		return p->ls(name);
	}

	static IndexedIO::EntryList ls(IndexedIOInterfacePtr p)
	{
		assert(p);

		return p->ls();
	}

	static IndexedIO::EntryList ls(IndexedIOInterfacePtr p, IndexedIOFilterPtr f)
	{
		assert(p);

		return p->ls(f);
	}

	template<typename T>
	static void writeVector(IndexedIOInterfacePtr p, const IndexedIO::EntryID &name,
		const typename TypedData < T >::Ptr &x)
	{
		assert(p);

		const typename T::value_type *data = &(x->readable())[0];
		p->write( name, data, (unsigned long)x->readable().size() );
	}

	template<typename T>
	static typename TypedData<T>::Ptr readSingle(IndexedIOInterfacePtr p, const IndexedIO::EntryID &name, const IndexedIO::Entry &entry)
	{
		T data;
		p->read(name, data);
		return new TypedData<T>( data );
	}

	template<typename T>
	static typename TypedData< std::vector<T> >::Ptr readArray(IndexedIOInterfacePtr p, const IndexedIO::EntryID &name, const IndexedIO::Entry &entry)
	{
		unsigned long count = entry.arrayLength();
		typename TypedData<std::vector<T> >::Ptr x = new TypedData<std::vector<T> > ();
		x->writable().resize( entry.arrayLength() );
		T *data = &(x->writable()[0]);
		p->read(name, data, count);

		return x;
	}

	static DataPtr read(IndexedIOInterfacePtr p, const IndexedIO::EntryID &name)
	{
		assert(p);

		IndexedIO::Entry entry = p->ls(name);

		switch( entry.dataType() )
		{
			case IndexedIO::Float:
				return readSingle<float>(p, name, entry);
			case IndexedIO::Double:
				return readSingle<double>(p, name, entry);
			case IndexedIO::Int:
				return readSingle<int>(p, name, entry);
			case IndexedIO::Long:
				return readSingle<int>(p, name, entry);
			case IndexedIO::String:
				return readSingle<std::string>(p, name, entry);
			case IndexedIO::StringArray:
				return readArray<std::string>(p, name, entry);
			case IndexedIO::FloatArray:
				return readArray<float>(p, name, entry);
			case IndexedIO::DoubleArray:
				return readArray<double>(p, name, entry);
			case IndexedIO::IntArray:
				return readArray<int>(p, name, entry);
			case IndexedIO::LongArray:
				return readArray<int>(p, name, entry);
			case IndexedIO::UInt:
				return readSingle<unsigned int>(p, name, entry);
			case IndexedIO::UIntArray:
				return readArray<unsigned int>(p, name, entry);
			case IndexedIO::Char:
				return readSingle<char>(p, name, entry);
			case IndexedIO::CharArray:
				return readArray<char>(p, name, entry);
			case IndexedIO::UChar:
				return readSingle<unsigned char>(p, name, entry);
			case IndexedIO::UCharArray:
				return readArray<unsigned char>(p, name, entry);
			default:
				throw IOException(name);
		}
	}

	static std::string readString(IndexedIOInterfacePtr p, const IndexedIO::EntryID &name)
	{
		assert(p);

		std::string x;
		p->read(name, x);
		return x;
	}

	static list supportedExtensions()
	{
		list result;
		std::vector<std::string> e;
		IndexedIOInterface::supportedExtensions( e );
		for( unsigned int i=0; i<e.size(); i++ )
		{
			result.append( e[i] );
		}
		return result;
	}

};

void bindIndexedIOInterface(const char *bindName)
{
	IndexedIO::EntryList (*lsNoFilter)(IndexedIOInterfacePtr) = &IndexedIOInterfaceHelper::ls;
	IndexedIO::EntryList (*lsFilter)(IndexedIOInterfacePtr, IndexedIOFilterPtr) = &IndexedIOInterfaceHelper::ls;
	IndexedIO::Entry (*lsEntry)(IndexedIOInterfacePtr, const IndexedIO::EntryID &) = &IndexedIOInterfaceHelper::ls;

	void (IndexedIOInterface::*writeFloat)(const IndexedIO::EntryID &, const float &) = &IndexedIOInterface::write;
	void (IndexedIOInterface::*writeDouble)(const IndexedIO::EntryID &, const double &) = &IndexedIOInterface::write;
	void (IndexedIOInterface::*writeInt)(const IndexedIO::EntryID &, const int &) = &IndexedIOInterface::write;
	void (IndexedIOInterface::*writeString)(const IndexedIO::EntryID &, const std::string &) = &IndexedIOInterface::write;
#if 0
	void (IndexedIOInterface::*writeUInt)(const IndexedIO::EntryID &, const unsigned int &) = &IndexedIOInterface::write;
	void (IndexedIOInterface::*writeChar)(const IndexedIO::EntryID &, const char &) = &IndexedIOInterface::write;
	void (IndexedIOInterface::*writeUChar)(const IndexedIO::EntryID &, const unsigned char &) = &IndexedIOInterface::write;
#endif


	enum_< IndexedIO::OpenModeFlags> ("IndexedIOOpenMode")
		.value("Read", IndexedIO::Read)
		.value("Write", IndexedIO::Write)
		.value("Append", IndexedIO::Append)
		.value("Shared", IndexedIO::Shared)
		.value("Exclusive", IndexedIO::Exclusive)
		.export_values()
	;

	enum_< IndexedIO::EntryType > ("IndexedIOEntryType")
		.value("Directory", IndexedIO::Directory)
		.value("File", IndexedIO::File)
		.export_values()
	;

	enum_< IndexedIO::DataType > ("IndexedIODataType")
		.value("Float", IndexedIO::Float)
		.value("FloatArray", IndexedIO::FloatArray)
		.value("Double", IndexedIO::Double)
		.value("DoubleArray", IndexedIO::DoubleArray)
		.value("Int", IndexedIO::Int)
		.value("IntArray", IndexedIO::IntArray)
		.value("Long", IndexedIO::Long)
		.value("LongArray", IndexedIO::LongArray)
		.value("String", IndexedIO::String)
		.value("UInt", IndexedIO::UInt)
		.value("UIntArray", IndexedIO::UIntArray)
		.value("Char", IndexedIO::Char)
		.value("CharArray", IndexedIO::CharArray)
		.value("UChar", IndexedIO::UChar)
		.value("UChar", IndexedIO::UCharArray)
		.export_values()
	;

	RefCountedClass<IndexedIOInterface, RefCounted>( bindName )
		.def("openMode", &IndexedIOInterface::openMode)
		.def("resetRoot", &IndexedIOInterface::resetRoot)
		.def("chdir", &IndexedIOInterface::chdir)
		.def("mkdir", &IndexedIOInterface::mkdir)
		.def("pwd", &IndexedIOInterface::pwd)
		.def("rm", &IndexedIOInterface::rm)
		.def("ls", lsFilter)
		.def("ls", lsNoFilter)
		.def("ls", lsEntry)
		.def("write", &IndexedIOInterfaceHelper::writeVector<std::vector<float> >)
		.def("write", &IndexedIOInterfaceHelper::writeVector<std::vector<double> >)
		.def("write", &IndexedIOInterfaceHelper::writeVector<std::vector<int> >)
		.def("write", &IndexedIOInterfaceHelper::writeVector<std::vector<std::string> >)
		.def("write", writeFloat)
		.def("write", writeDouble)
		.def("write", writeInt)
		.def("write", writeString)
#if 0
		// We dont really want to bind these because they don't represent natural Python datatypes
		.def("write", writeUInt)
		.def("write", writeChar)
		.def("write", writeUChar)
#endif
		.def("read", &IndexedIOInterfaceHelper::read)
		.def("create", &IndexedIOInterface::create ).staticmethod("create")
		.def("supportedExtensions", &IndexedIOInterfaceHelper::supportedExtensions ).staticmethod("supportedExtensions")

	;

}

void bindFileSystemIndexedIO(const char *bindName)
{
	RefCountedClass<FileSystemIndexedIO, IndexedIOInterface>( bindName )
		.def(init<const std::string &, const std::string &, IndexedIO::OpenMode >())
	;
}

void bindFileIndexedIO(const char *bindName)
{
	RefCountedClass<FileIndexedIO, IndexedIOInterface>( bindName )
		.def(init<const std::string &, const std::string &, IndexedIO::OpenMode >())
	;
}

CharVectorDataPtr memoryIndexedIOBufferWrapper( MemoryIndexedIOPtr io )
{
	assert( io );
	return io->buffer()->copy();
}

void bindMemoryIndexedIO(const char *bindName)
{
	RefCountedClass<MemoryIndexedIO, IndexedIOInterface>( bindName )
		.def(init<ConstCharVectorDataPtr, const std::string &, IndexedIO::OpenMode >())
		.def( "buffer", memoryIndexedIOBufferWrapper )
	;
}

void bindIndexedIOEntry(const char *bindName)
{
	class_< IndexedIO::Entry>(bindName, no_init)
		.def("id", &IndexedIO::Entry::id, return_value_policy<copy_const_reference>())
		.def("entryType", &IndexedIO::Entry::entryType)
		.def("dataType", &IndexedIO::Entry::dataType)
		.def("arrayLength", &IndexedIO::Entry::arrayLength)
		;
}

struct EntryListAccessor
{
	static IndexedIO::Entry get(const IndexedIO::EntryList &x, int i)
	{
		if (i < 0)
			i += x.size();

		if (i >= 0 && i < static_cast<int>(x.size()))
		{
			int idx = 0;

			IndexedIO::EntryList::const_iterator it = x.begin();
			while (idx != i)
				it++, idx++;

			assert(it != x.end());

			return *it;
		}
		else
		{
			throw std::out_of_range("");
		}
	}

	static int len(const IndexedIO::EntryList &x)
	{
		return static_cast<int>(x.size());
	}
};

void bindIndexedIOEntryList(const char *bindName)
{
	class_< IndexedIO::EntryList>(bindName, no_init)
		.def("__getitem__", &EntryListAccessor::get)
		.def("__len__", &EntryListAccessor::len)
	;
}

void bindIndexedIOFilter(const char *bindName)
{
	class_< IndexedIOFilter, boost::noncopyable, IndexedIOFilterPtr>(bindName, no_init)
		.def("add", &IndexedIOFilter::add )
		.def("apply", &IndexedIOFilter::apply )
		.def("filter", &IndexedIOFilter::filter )
	;
}

void bindIndexedIONullFilter(const char *bindName)
{
	class_<IndexedIONullFilter, IndexedIONullFilterPtr, bases<IndexedIOFilter> >(bindName, no_init)
		.def(init<>())
	;

	// Ensure we can upcast from a IndexedIOEntryTypeFilterPtr to a IndexedIOFilterPtr
	implicitly_convertible< IndexedIONullFilterPtr, IndexedIOFilterPtr >();
}


void bindIndexedIOEntryTypeFilter(const char *bindName)
{
	class_<IndexedIOEntryTypeFilter, IndexedIOEntryTypeFilterPtr, bases<IndexedIOFilter> >(bindName, no_init)
		.def(init<IndexedIO::EntryType>())
	;

	// Ensure we can upcast from a IndexedIOEntryTypeFilterPtr to a IndexedIOFilterPtr
	implicitly_convertible< IndexedIOEntryTypeFilterPtr, IndexedIOFilterPtr >();
}

void bindIndexedIORegexFilter(const char *bindName)
{
	class_<IndexedIORegexFilter, IndexedIORegexFilterPtr, bases<IndexedIOFilter> >(bindName, no_init)
		.def(init<const std::string &>())
	;

	// Ensure we can upcast from a IndexedIOEntryTypeFilterPtr to a IndexedIOFilterPtr
	implicitly_convertible< IndexedIORegexFilterPtr, IndexedIOFilterPtr >();
}
