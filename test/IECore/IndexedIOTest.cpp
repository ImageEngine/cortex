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

#include <iostream>

#include <IECore/IndexedIOInterface.h>
#include <IECore/FileIndexedIO.h>

#include "IndexedIOTest.h"


namespace IECore
{

template<>
std::string IndexedIOTestSuite<FileIndexedIO>::extension() const
{
	return "fio";
}

template<>
void IndexedIOTestSuite<FileIndexedIO>::getFilenames( FilenameList &filenames )
{
	filenames.clear();
	
	filenames.push_back("./test/IECore/data/fioFiles/2.13.0/rhel4.i686/types.fio");
	filenames.push_back("./test/IECore/data/fioFiles/2.13.0/osx104.i686/types.fio");
	filenames.push_back("./test/IECore/data/fioFiles/3.0.0/cent5.x86_64/types.fio");			
}

/// float 
template<>
std::string IndexedIOTestDataTraits<float>::name() 
{
	return "float";
}

template<>
float IndexedIOTestDataTraits<float>::value()
{
	return 5.0f;
} 

template<>
std::string IndexedIOTestDataTraits<float*>::name() 
{
	return "floatArray";
}

template<>
float* IndexedIOTestDataTraits<float*>::value()
{
	static float v[10] = {1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f,10.0f};
	
	return v;
} 


/// double 
template<>
std::string IndexedIOTestDataTraits<double>::name() 
{
	return "double";
}

template<>
double IndexedIOTestDataTraits<double>::value()
{
	return -14.0;
} 

template<>
std::string IndexedIOTestDataTraits<double*>::name() 
{
	return "doubleArray";
}

template<>
double* IndexedIOTestDataTraits<double*>::value()
{
	static double v[10] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0};
	
	return v;
} 

/// half 
template<>
std::string IndexedIOTestDataTraits<half>::name() 
{
	return "half";
}

template<>
half IndexedIOTestDataTraits<half>::value()
{
	return -14.0;
} 

template<>
std::string IndexedIOTestDataTraits<half*>::name() 
{
	return "halfArray";
}

template<>
half* IndexedIOTestDataTraits<half*>::value()
{
	static half v[10] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0};
	
	return v;
} 

/// int 
template<>
std::string IndexedIOTestDataTraits<int>::name() 
{
	return "int";
}

template<>
int IndexedIOTestDataTraits<int>::value()
{
	return 3;
} 

template<>
std::string IndexedIOTestDataTraits<int*>::name() 
{
	return "intArray";
}

template<>
int* IndexedIOTestDataTraits<int*>::value()
{
	static int v[10] = {1,2,3,4,5,6,7,8,9,10};
	
	return v;
} 

/// long 
template<>
std::string IndexedIOTestDataTraits<long>::name() 
{
	return "long";
}

template<>
long IndexedIOTestDataTraits<long>::value()
{
	return -222;
} 

template<>
std::string IndexedIOTestDataTraits<long*>::name() 
{
	return "longArray";
}

template<>
long* IndexedIOTestDataTraits<long*>::value()
{
	static long v[10] = {1,2,3,4,5,6,7,8,9,10};
	
	return v;
} 

/// string 
template<>
std::string IndexedIOTestDataTraits<std::string>::name() 
{
	return "string";
}

template<>
std::string IndexedIOTestDataTraits<std::string>::value()
{
	return "testString";
} 

template<>
std::string IndexedIOTestDataTraits<std::string*>::name() 
{
	return "stringArray";
}

template<>
std::string* IndexedIOTestDataTraits<std::string*>::value()
{
	static std::string v[10] = {"s1","s2","s3","s4","s5","s6","s7","s8","s9","s10"};
	
	return v;
} 

/// unsigned int 
template<>
std::string IndexedIOTestDataTraits<unsigned int>::name() 
{
	return "unsignedInt";
}

template<>
unsigned int IndexedIOTestDataTraits<unsigned int>::value()
{
	return 555;
} 

template<>
std::string IndexedIOTestDataTraits<unsigned int*>::name() 
{
	return "unsignedIntArray";
}

template<>
unsigned int* IndexedIOTestDataTraits<unsigned int*>::value()
{
	static unsigned int v[10] = {1,2,3,4,5,6,7,8,9,10};
	
	return v;
} 

/// char 
template<>
std::string IndexedIOTestDataTraits<char>::name() 
{
	return "char";
}

template<>
char IndexedIOTestDataTraits<char>::value()
{
	return 'f';
} 

template<>
std::string IndexedIOTestDataTraits<char*>::name() 
{
	return "charArray";
}

template<>
char* IndexedIOTestDataTraits<char*>::value()
{
	static char v[10] = {'a','b','c','d','e','f','g','h','i','j'};
	
	return v;
} 


/// unsigned char 
template<>
std::string IndexedIOTestDataTraits<unsigned char>::name() 
{
	return "unsignedChar";
}

template<>
unsigned char IndexedIOTestDataTraits<unsigned char>::value()
{
	return 'f';
} 

template<>
std::string IndexedIOTestDataTraits<unsigned char*>::name() 
{
	return "unsignedCharArray";
}

template<>
unsigned char* IndexedIOTestDataTraits<unsigned char*>::value()
{
	static unsigned char v[10] = {'a','b','c','d','e','f','g','h','i','j'};
	
	return v;
} 

/// short 
template<>
std::string IndexedIOTestDataTraits<short>::name() 
{
	return "short";
}

template<>
short IndexedIOTestDataTraits<short>::value()
{
	return 12;
} 

template<>
std::string IndexedIOTestDataTraits<short*>::name() 
{
	return "shortArray";
}

template<>
short* IndexedIOTestDataTraits<short*>::value()
{
	static short v[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	
	return v;
} 


/// unsigned short 
template<>
std::string IndexedIOTestDataTraits<unsigned short>::name() 
{
	return "unsignedshort";
}

template<>
unsigned short IndexedIOTestDataTraits<unsigned short>::value()
{
	return 5;
} 

template<>
std::string IndexedIOTestDataTraits<unsigned short*>::name() 
{
	return "unsignedshortArray";
}

template<>
unsigned short* IndexedIOTestDataTraits<unsigned short*>::value()
{
	static unsigned short v[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	
	return v;
} 

void addIndexedIOTest(boost::unit_test::test_suite* test)
{
	test->add( new IndexedIOTestSuite<FileIndexedIO>() );
}

}
