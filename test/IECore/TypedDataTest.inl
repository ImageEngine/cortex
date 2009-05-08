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

using namespace IECore;
using namespace Imath;
using namespace std;

namespace IECore
{
template<typename T>
typename T::value_type VectorTypedDataTest<T>::f1(unsigned int i) const
{
	return static_cast<typename T::value_type>(sqrt((double)i));
}

template<typename T>
typename T::value_type VectorTypedDataTest<T>::f2(unsigned int i) const
{
	return static_cast<typename T::value_type>((double)i/3.0);
}

template<typename T>
unsigned int VectorTypedDataTest<T>::randomElementPos()
{
	return (int)(m_randGen.nextf() * m_size) % m_size;
}


template<typename T>
VectorTypedDataTest<T>::VectorTypedDataTest(unsigned int size) : m_size(size)
{
	m_data = new TypedData<T>();

	for (unsigned int i = 0; i < size; i++)
	{
		typename T::value_type v = f1(i);
		m_data->writable().push_back( v );
	}
}

template<typename T>
VectorTypedDataTest<T>::~VectorTypedDataTest()
{
}

template<typename T>
void VectorTypedDataTest<T>::testSize()
{
	BOOST_CHECK_EQUAL(m_data->readable().size(), m_size);
}

template<typename T>
void VectorTypedDataTest<T>::testRead()
{
	BOOST_CHECK_EQUAL(m_data->readable().size(), m_size);

	if (m_size)
	{
		// Check a random 5% of points
		const int numPoints = std::max<int>(1, (int)(m_size * 0.05));
		for (int i = 0; i < numPoints; i++)
		{
			int pos = randomElementPos();
			BOOST_CHECK_EQUAL(m_data->readable()[pos], f1(pos) );
		}
	}


}
template<typename T>
void VectorTypedDataTest<T>::testWrite()
{
	BOOST_CHECK_EQUAL(m_data->readable().size(), m_size);

	if (m_size)
	{
		// Check a random 5% of points
		const int numPoints = std::max<int>(1, (int)(m_size * 0.05));

		for (int i = 0; i < numPoints; i++)
		{
			int pos = randomElementPos();

			const typename T::value_type oldValue = m_data->readable()[pos];
			const typename T::value_type newValue = f2(pos);

			m_data->writable()[pos] = newValue;

			BOOST_CHECK_EQUAL(m_data->readable()[pos], newValue);

			m_data->writable()[pos] = oldValue;

			BOOST_CHECK_EQUAL(m_data->readable()[pos], oldValue);
		}
	}
}


template<typename T>
void VectorTypedDataTest<T>::testAssign()
{
	boost::intrusive_ptr<TypedData<T> > assign = new TypedData<T>();
	*assign = *m_data;

	BOOST_CHECK_EQUAL( assign->readable().size(), m_data->readable().size() );

	if (m_size)
	{
		// Check a random 5% of points
		const int numPoints = std::max<int>(1, (int)(m_size * 0.05));
		for (int i = 0; i < numPoints; i++)
		{
			BOOST_CHECK_EQUAL( assign->readable()[i], m_data->readable()[i] );
		}
	}
}


template<typename T>
SimpleTypedDataTest<T>::SimpleTypedDataTest()
{
	m_data = new TypedData<T>();
	m_data->writable() = static_cast<T>(3.0);
}

template<typename T>
SimpleTypedDataTest<T>::~SimpleTypedDataTest()
{
}

template<typename T>
void SimpleTypedDataTest<T>::testRead()
{
	BOOST_CHECK_EQUAL( m_data->readable(), static_cast<T>(3.0) );
}

template<typename T>
void SimpleTypedDataTest<T>::testWrite()
{
	m_data->writable() = static_cast<T>(5.0);
	BOOST_CHECK_EQUAL( m_data->readable(), static_cast<T>(5.0) );
	m_data->writable() = static_cast<T>(3.0);
	BOOST_CHECK_EQUAL( m_data->readable(), static_cast<T>(3.0) );
}

template<typename T>
void SimpleTypedDataTest<T>::testAssign()
{
	boost::intrusive_ptr<TypedData<T> > other = new TypedData<T>();
	*other = *m_data;

	BOOST_CHECK_EQUAL( other->readable(), static_cast<T>(3.0) );
}

template<typename T>
void SimpleTypedDataTest<T>::testMemoryUsage()
{
	BOOST_CHECK_EQUAL( m_data->memoryUsage(), sizeof(T) );
}

template<typename T>
void SimpleTypedDataTest<T>::testIsEqualTo()
{
	boost::intrusive_ptr<TypedData<T> > other = new TypedData<T>();
	other->writable() = static_cast<T>(3.0);

	BOOST_CHECK( m_data->isEqualTo(other) );
}

}
