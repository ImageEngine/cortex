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

#ifndef IE_CORE_INDEXEDIOTEST_H
#define IE_CORE_INDEXEDIOTEST_H

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <IECore/IECore.h>
#include <IECore/IndexedIOInterface.h>

namespace IECore
{

void addIndexedIOTest(boost::unit_test::test_suite* test);

typedef std::vector<std::string> FilenameList;

template<typename T>
struct IndexedIOTestDataTraits
{
	static std::string name()
	{
		assert(false);
		return "";
	}
	
	static T value()
	{
		assert(false);
		return T();
	}
	
	static void check(const T& v1)
	{
		BOOST_CHECK_EQUAL(v1, value());
	}
};

template<typename T>
struct IndexedIOTestDataTraits<T*>
{
	static std::string name()
	{
		assert(false);
		return "";
	}
	
	static T* value()
	{
		assert(false);
		return 0;
	}
	
	static void check(T *v1)
	{
		for (int i = 0; i < 10; i++)
		{
			BOOST_CHECK_EQUAL(v1[i], value()[i]);
		}
	}
};

template<typename T>
struct IndexedIOTest
{
	
	IndexedIOTest(const FilenameList &filenames) : m_filenames(filenames) {};
	
	template<typename D>
	void test()
	{
		for (FilenameList::const_iterator it = m_filenames.begin(); it != m_filenames.end(); ++it)
		{
			IndexedIOInterfacePtr io = new T(*it, "/", IndexedIO::Read );
		
			D v;
			io->read(IndexedIOTestDataTraits<D>::name(), v );
			IndexedIOTestDataTraits<D>::check(v);
		}
	}
	
	template<typename D>
	void testArray()
	{
		for (FilenameList::const_iterator it = m_filenames.begin(); it != m_filenames.end(); ++it)
		{
			IndexedIOInterfacePtr io = new T(*it, "/", IndexedIO::Read );
		
			D *v = new D[10] ;
			io->read(IndexedIOTestDataTraits<D*>::name(), v, 10 );
			
			IndexedIOTestDataTraits<D*>::check(v);
			
			delete[] v;
		}
	}
	
	template<typename D>
	void write( IndexedIOInterfacePtr io)
	{
		assert(io);
		
		io->write( IndexedIOTestDataTraits<D>::name(), IndexedIOTestDataTraits<D>::value() ); 
	}
	
	template<typename D>
	void writeArray( IndexedIOInterfacePtr io)
	{
		assert(io);
		
		io->write( IndexedIOTestDataTraits<D*>::name(), IndexedIOTestDataTraits<D*>::value(), 10 ); 
	}
	
	void write(const std::string &filename)
	{
		IndexedIOInterfacePtr io = new T(filename, "/", IndexedIO::Write );
		
		write<float>(io);
		write<double>(io);
		write<half>(io);
		write<int>(io);
		write<long>(io);
		write<std::string>(io);
		write<unsigned int>(io);
		write<char>(io);
		write<unsigned char>(io);
		write<short>(io);
		write<unsigned short>(io);		
		
		
		writeArray<float>(io);
		writeArray<double>(io);
		writeArray<half>(io);
		writeArray<int>(io);
		writeArray<long>(io);
		writeArray<std::string>(io);
		writeArray<unsigned int>(io);
		writeArray<char>(io);
		writeArray<unsigned char>(io);
		writeArray<short>(io);
		writeArray<unsigned short>(io);		
		
	}
	
	FilenameList m_filenames;
};

template<typename T>
struct IndexedIOTestSuite : public boost::unit_test::test_suite
{
	
	IndexedIOTestSuite() : boost::unit_test::test_suite("IndexedIOTestSuite")
	{
		FilenameList filenames;
		getFilenames(filenames);
			
		static boost::shared_ptr<IndexedIOTest<T> > instance(new IndexedIOTest<T>(filenames));
		
		/// Uncomment this line to write out new test data - change architecture first
		//instance->write("./test/IECore/data/" + extension() + "Files/" + IECore::versionString() + "/osx104.ppc/types." + extension());		
		
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<float>, instance ) );				
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<double>, instance ) );		
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<half>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<int>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<long>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<std::string>, instance ) );								
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<unsigned int>, instance ) );				
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<char>, instance ) );				
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template test<unsigned char>, instance ) );				

		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<float>, instance ) );				
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<double>, instance ) );		
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<half>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<int>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<long>, instance ) );
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<std::string>, instance ) );								
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<unsigned int>, instance ) );				
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<char>, instance ) );				
		add( BOOST_CLASS_TEST_CASE( &IndexedIOTest<T>::template testArray<unsigned char>, instance ) );				


	}
	
	std::string extension() const;
	
	void getFilenames( FilenameList &filenames );
};
}


#endif

