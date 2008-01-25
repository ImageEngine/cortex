//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_CLASSDATA_INL
#define IE_CORE_CLASSDATA_INL

#include <cassert>

namespace IECore
{


template< typename ClassType, typename DataType, typename DataDeletePolicy >
ClassData<ClassType, DataType, DataDeletePolicy>::~ClassData()
{
	DataDeletePolicy deleter;

	for (typename ClassDataMap::iterator it = m_classDataMap.begin(); it != m_classDataMap.end(); ++it )
	{
		deleter( it->second );
	}

	m_classDataMap.clear();
}

template< typename ClassType, typename DataType, typename DataDeletePolicy >
DataType &ClassData<ClassType, DataType, DataDeletePolicy>::create( const ClassType* classOwner )
{
	assert( classOwner );
	assert( m_classDataMap.find( classOwner ) == m_classDataMap.end() );

	return m_classDataMap[ classOwner ];
}

template< typename ClassType, typename DataType, typename DataDeletePolicy >
DataType &ClassData<ClassType, DataType, DataDeletePolicy>::create( const ClassType* classOwner, const DataType &d )
{
	assert( classOwner );
	assert( m_classDataMap.find( classOwner ) == m_classDataMap.end() );
	m_classDataMap[ classOwner ] = d;

	return m_classDataMap[ classOwner ];
}

template< typename ClassType, typename DataType, typename DataDeletePolicy >
const DataType &ClassData<ClassType, DataType, DataDeletePolicy>::operator []( const ClassType *classOwner ) const
{
	assert( classOwner );
	typename ClassDataMap::const_iterator it = m_classDataMap.find( classOwner );
	assert( it != m_classDataMap.end() );

	return it->second;			
}

template< typename ClassType, typename DataType, typename DataDeletePolicy >
DataType &ClassData<ClassType, DataType, DataDeletePolicy>::operator []( const ClassType *classOwner )
{		
	assert( classOwner );
	typename ClassDataMap::iterator it = m_classDataMap.find( classOwner );
	assert( it != m_classDataMap.end() );

	return it->second;
}

template< typename ClassType, typename DataType, typename DataDeletePolicy >
void ClassData<ClassType, DataType, DataDeletePolicy>::erase( const ClassType *classOwner )
{
	assert( classOwner );
	typename ClassDataMap::iterator it = m_classDataMap.find( classOwner );
	assert( it != m_classDataMap.end() );

	DataDeletePolicy deleter;
	deleter( it->second );
	m_classDataMap.erase( classOwner );
}


}

#endif
