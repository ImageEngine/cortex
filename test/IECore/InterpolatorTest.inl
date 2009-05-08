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
void LinearInterpolatorTest<T>::testSimple()
{
	LinearInterpolator<T> interp;

	T result;

	interp( T(0.0), T(1.0), 0.5, result);
	BOOST_CHECK_EQUAL(T(0.5), result);

	interp( T(0.0), T(1.0), 0.0, result);
	BOOST_CHECK_EQUAL(T(0.0), result);

	interp( T(0.0), T(1.0), 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result);

	interp( T(100.0), T(200.0), 0.64, result);
	BOOST_CHECK_EQUAL(T(164.0), result);

	interp( T(100.0), T(200.0), 0.21, result);
	BOOST_CHECK_EQUAL(T(121.0), result);

	interp( T(-100.0), T(100.0), 0.5, result);
	BOOST_CHECK_EQUAL(T(0.0), result);
}

template<typename T>
void LinearInterpolatorTest<T>::testTyped()
{
	LinearInterpolator<TypedData<T> > interp;

	boost::intrusive_ptr<TypedData<T> > p0, p1;

	boost::intrusive_ptr<TypedData<T> > result = new TypedData<T>();

	p0 = new TypedData<T>(T(0.0));
	p1 = new TypedData<T>(T(1.0));
	interp( p0, p1, 0.5, result);
	BOOST_CHECK_EQUAL(T(0.5), result->readable());

	interp( p0, p1, 0.0, result);
	BOOST_CHECK_EQUAL(T(0.0), result->readable());

	interp( p0, p1, 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result->readable());

	p0 = new TypedData<T>(T(50.0));
	p1 = new TypedData<T>(T(80.0));
	interp( p0, p1, 0.5, result);
	BOOST_CHECK_EQUAL(T(65.0), result->readable());

	p0 = new TypedData<T>(T(10.0));
	p1 = new TypedData<T>(T(20.0));
	interp(p0, p1, 0.6, result);
	BOOST_CHECK_EQUAL(T(16.0), result->readable());

	p0 = new TypedData<T>(T(-10.0));
	p1 = new TypedData<T>(T(5.0));
	interp( p0, p1, 0.5, result);
	BOOST_CHECK_EQUAL(T(-2.5), result->readable());

}

template<typename T>
void LinearInterpolatorTest<T>::testVector()
{
	LinearInterpolator<TypedData<std::vector<T> > > interp;

	boost::intrusive_ptr<TypedData<std::vector<T> > > p0, p1, result;

	p0 = new TypedData<std::vector<T> >();
	p1 = new TypedData<std::vector<T> >();
	result = new TypedData<std::vector<T> >();

	p0->writable().push_back( T(0.0) );
	p1->writable().push_back( T(3.0) );

	p0->writable().push_back( T(5.0) );
	p1->writable().push_back( T(6.0) );

	p0->writable().push_back( T(-30.0) );
	p1->writable().push_back( T(4.0) );

	p0->writable().push_back( T(15.0) );
	p1->writable().push_back( T(4.0) );

	interp( p0, p1, 0.5, result );

	BOOST_CHECK_EQUAL( result->readable().size(), 4u );

	BOOST_CHECK_EQUAL( T(1.5),   result->readable()[0] );
	BOOST_CHECK_EQUAL( T(5.5),   result->readable()[1] );
	BOOST_CHECK_EQUAL( T(-13.0), result->readable()[2] );
	BOOST_CHECK_EQUAL( T(9.5),   result->readable()[3] );

}

template<typename T>
void CubicInterpolatorTest<T>::testSimple()
{
	CubicInterpolator<T> interp;

	T result;

	interp( T(0.0), T(0.0), T(1.0), T(1.0), 0.5, result);
	BOOST_CHECK_EQUAL(T(0.5), result);

	interp( T(0.0), T(0.0), T(1.0), T(1.0), 0.0, result);
	BOOST_CHECK_EQUAL(T(0.0), result);

	interp( T(0.0), T(0.0), T(1.0), T(1.0), 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result);

	interp( T(-1.0), T(0.0), T(1.0), T(2.0), 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result);

	interp( T(-15.0), T(0.0), T(-1.0), T(4.0), 0.5, result);
	BOOST_CHECK_EQUAL(T(0.75), result);

}

template<typename T>
void CubicInterpolatorTest<T>::testTyped()
{
	CubicInterpolator<TypedData<T > > interp;

	boost::intrusive_ptr<TypedData<T > > result = new TypedData<T>();

	boost::intrusive_ptr<TypedData<T> > p0, p1, p2, p3;

	p0 = new TypedData<T>( T(0.0));
	p1 = new TypedData<T>( T(0.0));
	p2 = new TypedData<T>( T(1.0));
	p3 = new TypedData<T>( T(1.0));

	interp( p0, p1, p2, p3, 0.5, result);
	BOOST_CHECK_EQUAL(T(0.5), result->readable());

	interp( p0, p1, p2, p3, 0.0, result);
	BOOST_CHECK_EQUAL(T(0.0), result->readable());

	interp( p0, p1, p2, p3, 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result->readable());

	p0 = new TypedData<T>( T(-1.0));
	p1 = new TypedData<T>( T(0.0));
	p2 = new TypedData<T>( T(1.0));
	p3 = new TypedData<T>( T(2.0));
	interp( p0, p1, p2, p3, 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result->readable());

	p0 = new TypedData<T>( T(-15.0));
	p1 = new TypedData<T>( T(0.0));
	p2 = new TypedData<T>( T(-1.0));
	p3 = new TypedData<T>( T(4.0));
	interp( p0, p1, p2, p3, 0.5, result);
	BOOST_CHECK_EQUAL(T(0.75), result->readable());

}

template<typename T>
void CubicInterpolatorTest<T>::testVector()
{
	CubicInterpolator<TypedData<std::vector<T> > > interp;

	boost::intrusive_ptr<TypedData<std::vector<T> > > p0, p1, p2, p3, result;

	p0 = new TypedData<std::vector<T> >();
	p1 = new TypedData<std::vector<T> >();
	p2 = new TypedData<std::vector<T> >();
	p3 = new TypedData<std::vector<T> >();

	result = new TypedData<std::vector<T> >();

	p0->writable().push_back( T(-128.0) );
	p1->writable().push_back( T(-32.0) );
	p2->writable().push_back( T(32.0) );
	p3->writable().push_back( T(128.0) );

	interp( p0, p1, p2, p3, 0.75, result );

	BOOST_CHECK_EQUAL( result->readable().size(), 1u );

	BOOST_CHECK_EQUAL( T(7.0), result->readable()[0] );

}

}
