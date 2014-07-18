//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

	typename TypedData<T>::Ptr p0, p1;

	typename TypedData<T>::Ptr result = new TypedData<T>();

	p0 = new TypedData<T>(T(0.0));
	p1 = new TypedData<T>(T(1.0));
	interp( p0.get(), p1.get(), 0.5, result);
	BOOST_CHECK_EQUAL(T(0.5), result->readable());

	interp( p0.get(), p1.get(), 0.0, result);
	BOOST_CHECK_EQUAL(T(0.0), result->readable());

	interp( p0.get(), p1.get(), 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result->readable());

	p0 = new TypedData<T>(T(50.0));
	p1 = new TypedData<T>(T(80.0));
	interp( p0.get(), p1.get(), 0.5, result);
	BOOST_CHECK_EQUAL(T(65.0), result->readable());

	p0 = new TypedData<T>(T(10.0));
	p1 = new TypedData<T>(T(20.0));
	interp(p0.get(), p1.get(), 0.6, result);
	BOOST_CHECK_EQUAL(T(16.0), result->readable());

	p0 = new TypedData<T>(T(-10.0));
	p1 = new TypedData<T>(T(5.0));
	interp( p0.get(), p1.get(), 0.5, result);
	BOOST_CHECK_EQUAL(T(-2.5), result->readable());

}

template<typename T>
void LinearInterpolatorTest<T>::testVector()
{
	LinearInterpolator<TypedData<std::vector<T> > > interp;

	typename TypedData<std::vector<T> >::Ptr p0, p1, result;

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

	interp( p0.get(), p1.get(), 0.5, result );

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

	typename TypedData<T>::Ptr result = new TypedData<T>();

	typename TypedData<T>::Ptr p0, p1, p2, p3;

	p0 = new TypedData<T>( T(0.0));
	p1 = new TypedData<T>( T(0.0));
	p2 = new TypedData<T>( T(1.0));
	p3 = new TypedData<T>( T(1.0));

	interp( p0.get(), p1.get(), p2.get(), p3.get(), 0.5, result);
	BOOST_CHECK_EQUAL(T(0.5), result->readable());

	interp( p0.get(), p1.get(), p2.get(), p3.get(), 0.0, result);
	BOOST_CHECK_EQUAL(T(0.0), result->readable());

	interp( p0.get(), p1.get(), p2.get(), p3.get(), 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result->readable());

	p0 = new TypedData<T>( T(-1.0));
	p1 = new TypedData<T>( T(0.0));
	p2 = new TypedData<T>( T(1.0));
	p3 = new TypedData<T>( T(2.0));
	interp( p0.get(), p1.get(), p2.get(), p3.get(), 1.0, result);
	BOOST_CHECK_EQUAL(T(1.0), result->readable());

	p0 = new TypedData<T>( T(-15.0));
	p1 = new TypedData<T>( T(0.0));
	p2 = new TypedData<T>( T(-1.0));
	p3 = new TypedData<T>( T(4.0));
	interp( p0.get(), p1.get(), p2.get(), p3.get(), 0.5, result);
	BOOST_CHECK_EQUAL(T(0.75), result->readable());

}

template<typename T>
void CubicInterpolatorTest<T>::testVector()
{
	CubicInterpolator<TypedData<std::vector<T> > > interp;

	typename TypedData<std::vector<T> >::Ptr p0, p1, p2, p3, result;

	p0 = new TypedData<std::vector<T> >();
	p1 = new TypedData<std::vector<T> >();
	p2 = new TypedData<std::vector<T> >();
	p3 = new TypedData<std::vector<T> >();

	result = new TypedData<std::vector<T> >();

	p0->writable().push_back( T(-128.0) );
	p1->writable().push_back( T(-32.0) );
	p2->writable().push_back( T(32.0) );
	p3->writable().push_back( T(128.0) );

	interp( p0.get(), p1.get(), p2.get(), p3.get(), 0.75, result );

	BOOST_CHECK_EQUAL( result->readable().size(), 1u );

	BOOST_CHECK_EQUAL( T(7.0), result->readable()[0] );

}






template<typename T>
void MatrixLinearInterpolatorTest<T>::testSimple()
{
	typedef typename Imath::Matrix44< T > Matrix;
	typedef typename Imath::Vec3< T > Vector;
	
	LinearInterpolator< Matrix > interp;
	LinearInterpolator< Vector > vectorInterp;
	
	Vector s0( 1, 1, 1 ), h0( 0, 0, 0 ), r0( 0, 0, 0 ), t0( 5, 0, 0 );
	Matrix m0;
	m0[3][0] = 5;
	
	Matrix m1;
	Vector s1( 1, 2, 3 ), h1( 1, 2, 3 ), r1( 0, 1, 0 ), t1( 10, 0, 0 );
	m1.setEulerAngles( r1 );
	m1.shear( h1 );
	m1.scale( s1 );
	m1[3][0] = 10;
	
	for( int i = 0; i < 11; ++i )
	{
		Matrix result;
		interp( m0, m1, T(i)/10, result );
		Vec3<T> s, h, r, t;
		extractSHRT( result, s, h, r, t );
		
		Vec3<T> sTrue, hTrue, rTrue, tTrue;
		vectorInterp( s0, s1, T(i)/10, sTrue );
		vectorInterp( h0, h1, T(i)/10, hTrue );
		vectorInterp( r0, r1, T(i)/10, rTrue );
		vectorInterp( t0, t1, T(i)/10, tTrue );
		
		BOOST_CHECK_CLOSE( rTrue[1], r[1], 1.e-4 );
		BOOST_CHECK_CLOSE( tTrue[0], t[0], 1.e-4 );
		
		for( int j=0; j < 3; ++j )
		{
			BOOST_CHECK_CLOSE( sTrue[j], s[j], 1.e-4 );
			BOOST_CHECK_CLOSE( hTrue[j], h[j], 1.e-4 );
		}
	}

}

template<typename T>
void MatrixLinearInterpolatorTest<T>::testTyped()
{
	typedef typename Imath::Matrix44< T > Matrix;
	typedef typename Imath::Vec3< T > Vector;
	
	typedef TypedData< Matrix > MatrixData;
	typedef typename MatrixData::Ptr MatrixDataPtr;
	
	LinearInterpolator< MatrixData > interp;
	LinearInterpolator< Vector > vectorInterp;
	
	Vector s0( 1, 1, 1 ), h0( 0, 0, 0 ), r0( 0, 0, 0 ), t0( 5, 0, 0 );
	MatrixDataPtr m0 = new MatrixData;
	m0->writable()[3][0] = 5;
	
	MatrixDataPtr m1 = new MatrixData;
	Vector s1( 1, 2, 3 ), h1( 1, 2, 3 ), r1( 0, 1, 0 ), t1( 10, 0, 0 );
	m1->writable().setEulerAngles( r1 );
	m1->writable().shear( h1 );
	m1->writable().scale( s1 );
	m1->writable()[3][0] = 10;
	
	for( int i = 0; i < 11; ++i )
	{
		MatrixDataPtr result = new MatrixData;
		interp( m0.get(), m1.get(), T(i)/10, result );
		Vector s, h, r, t;
		extractSHRT( result->readable(), s, h, r, t );
		
		Vector sTrue, hTrue, rTrue, tTrue;
		vectorInterp( s0, s1, T(i)/10, sTrue );
		vectorInterp( h0, h1, T(i)/10, hTrue );
		vectorInterp( r0, r1, T(i)/10, rTrue );
		vectorInterp( t0, t1, T(i)/10, tTrue );
		
		BOOST_CHECK_CLOSE( rTrue[1], r[1], 1.e-4 );
		BOOST_CHECK_CLOSE( tTrue[0], t[0], 1.e-4 );
		
		for( int j=0; j < 3; ++j )
		{
			BOOST_CHECK_CLOSE( sTrue[j], s[j], 1.e-4 );
			BOOST_CHECK_CLOSE( hTrue[j], h[j], 1.e-4 );
		}
	}
}

template<typename T>
void MatrixLinearInterpolatorTest<T>::testVector()
{
	typedef typename Imath::Matrix44< T > Matrix;
	typedef typename Imath::Vec3< T > Vector;
	
	typedef std::vector< Matrix > MatrixVector;
	typedef std::vector< Vector > VectorVector;
	
	typedef TypedData< MatrixVector > MatrixVectorData;
	typedef typename MatrixVectorData::Ptr MatrixVectorDataPtr;
	
	typedef TypedData< VectorVector > VectorVectorData;
	typedef typename VectorVectorData::Ptr VectorVectorDataPtr;
	
	
	LinearInterpolator< MatrixVectorData > interp;
	LinearInterpolator< VectorVectorData > vectorInterp;
	
	VectorVectorDataPtr s0 = new VectorVectorData;
	VectorVectorDataPtr h0 = new VectorVectorData;
	VectorVectorDataPtr r0 = new VectorVectorData;
	VectorVectorDataPtr t0 = new VectorVectorData;
	MatrixVectorDataPtr m0 = new MatrixVectorData;
	
	VectorVectorDataPtr s1 = new VectorVectorData;
	VectorVectorDataPtr h1 = new VectorVectorData;
	VectorVectorDataPtr r1 = new VectorVectorData;
	VectorVectorDataPtr t1 = new VectorVectorData;
	MatrixVectorDataPtr m1 = new MatrixVectorData;
	
	for( int i=0; i < 4; ++i )
	{
		s0->writable().push_back( Vector( 1,1,1 ) );
		h0->writable().push_back( Vector( 0,0,0 ) );
		r0->writable().push_back( Vector( 0,0,0 ) );
		t0->writable().push_back( Vector( i + 1,0,0 ) );
		
		Matrix m;
		m[3][0] = t0->readable().back()[0];
		
		m0->writable().push_back( m );
		
		s1->writable().push_back( Vector( i+1,i+2,i+3 ) );
		h1->writable().push_back( Vector( i+1,i+2,i+3 ) );
		r1->writable().push_back( Vector( 0, T( 0.5 * 3.1415926 * ( i + 1 ) / 4 ),0 ) );
		t1->writable().push_back( Vector( i + 10,0,0 ) );
		
		m = Matrix();
		m.setEulerAngles( r1->readable().back() );
		m.shear( h1->readable().back() );
		m.scale( s1->readable().back() );
		m[3][0] = t1->readable().back()[0];
		
		m1->writable().push_back( m );
		
	}
	
	for( int i = 0; i < 11; ++i )
	{
		MatrixVectorDataPtr result = new MatrixVectorData;
		
		interp( m0.get(), m1.get(), T(i)/10, result );
		
		VectorVectorDataPtr sTrue = new VectorVectorData;
		VectorVectorDataPtr hTrue = new VectorVectorData;
		VectorVectorDataPtr rTrue = new VectorVectorData;
		VectorVectorDataPtr tTrue = new VectorVectorData;
		
		vectorInterp( s0.get(), s1.get(), T(i)/10, sTrue );
		vectorInterp( h0.get(), h1.get(), T(i)/10, hTrue );
		vectorInterp( r0.get(), r1.get(), T(i)/10, rTrue );
		vectorInterp( t0.get(), t1.get(), T(i)/10, tTrue );
		
		
		for( int j=0; j < 4; ++j )
		{
			
			Vector s, h, r, t;
			extractSHRT( result->readable()[j], s, h, r, t );
			
			BOOST_CHECK_CLOSE( rTrue->readable()[j][1], r[1], 1.e-4 );
			BOOST_CHECK_CLOSE( tTrue->readable()[j][0], t[0], 1.e-4 );

			for( int k=0; k < 3; ++k )
			{
				BOOST_CHECK_CLOSE( sTrue->readable()[j][k], s[k], 1.e-4 );
				BOOST_CHECK_CLOSE( hTrue->readable()[j][k], h[k], 1.e-4 );
			}
		}
	
	}

}



template<typename T>
void MatrixCubicInterpolatorTest<T>::testSimple()
{
	typedef typename Imath::Matrix44< T > Matrix;
	typedef typename Imath::Vec3< T > Vector;
	
	CubicInterpolator< Matrix > interp;
	CubicInterpolator< Vector > vectorInterp;
	
	Vector s0( 1, 1, 1 ), h0( 0, 0, 0 ), r0( 0, 0, 0 ), t0( 5, 0, 0 );
	Vector s1( 1, 2, 3 ), h1( 1, 2, 3 ), r1( 0, 1, 0 ), t1( 10, 0, 0 );
	Vector s2( 0.5, 1.4, 5 ), h2( 2, 3, 4 ), r2( 0, 0.5, 0 ), t2( 20, 0, 0 );
	Vector s3( 1, 2, 3 ), h3( 1, 2, 3 ), r3( 0, 1, 0 ), t3( 0, 0, 0 );
	
	Matrix m0, m1, m2, m3;
	m0[3][0] = t0[0];
	
	m1.setEulerAngles( r1 );
	m1.shear( h1 );
	m1.scale( s1 );
	m1[3][0] = t1[0];
	
	m2.setEulerAngles( r2 );
	m2.shear( h2 );
	m2.scale( s2 );
	m2[3][0] = t2[0];
	
	m3.setEulerAngles( r3 );
	m3.shear( h3 );
	m3.scale( s3 );
	m3[3][0] = t3[0];
	
	for( int i = 0; i < 11; ++i )
	{
		Matrix result;
		interp( m0, m1, m2, m3, T(i)/10, result );
		Vec3<T> s, h, r, t;
		extractSHRT( result, s, h, r, t );
		
		Vec3<T> sTrue, hTrue, rTrue, tTrue;
		vectorInterp( s0, s1, s2, s3, T(i)/10, sTrue );
		vectorInterp( h0, h1, h2, h3, T(i)/10, hTrue );
		vectorInterp( t0, t1, t2, t3, T(i)/10, tTrue );
		
		// \todo: quaternion cubic interpolation doesn't currently
		// give the cubic interpolated angle in this case, so we're not checking r.
		// Maybe derive a formula for it?
		
		BOOST_CHECK_CLOSE( tTrue[0], t[0], 1.e-4 );
		
		for( int j=0; j < 3; ++j )
		{
			BOOST_CHECK_CLOSE( sTrue[j], s[j], 1.e-4 );
			BOOST_CHECK_CLOSE( hTrue[j], h[j], 1.e-4 );
		}
	}


}

template<typename T>
void MatrixCubicInterpolatorTest<T>::testTyped()
{
	typedef typename Imath::Matrix44< T > Matrix;
	typedef typename Imath::Vec3< T > Vector;
	
	typedef TypedData< Matrix > MatrixData;
	typedef typename MatrixData::Ptr MatrixDataPtr;
	
	CubicInterpolator< MatrixData > interp;
	CubicInterpolator< Vector > vectorInterp;
	
	Vector s0( 1, 1, 1 ), h0( 0, 0, 0 ), r0( 0, 0, 0 ), t0( 5, 0, 0 );
	Vector s1( 1, 2, 3 ), h1( 1, 2, 3 ), r1( 0, 1, 0 ), t1( 10, 0, 0 );
	Vector s2( 0.5, 1.4, 5 ), h2( 2, 3, 4 ), r2( 0, 0.5, 0 ), t2( 20, 0, 0 );
	Vector s3( 1, 2, 3 ), h3( 1, 2, 3 ), r3( 0, 1, 0 ), t3( 0, 0, 0 );
	
	MatrixDataPtr m0 = new MatrixData;
	MatrixDataPtr m1 = new MatrixData;
	MatrixDataPtr m2 = new MatrixData;
	MatrixDataPtr m3 = new MatrixData;
	m0->writable()[3][0] = t0[0];
	
	m1->writable().setEulerAngles( r1 );
	m1->writable().shear( h1 );
	m1->writable().scale( s1 );
	m1->writable()[3][0] = t1[0];
	
	m2->writable().setEulerAngles( r2 );
	m2->writable().shear( h2 );
	m2->writable().scale( s2 );
	m2->writable()[3][0] = t2[0];
	
	m3->writable().setEulerAngles( r3 );
	m3->writable().shear( h3 );
	m3->writable().scale( s3 );
	m3->writable()[3][0] = t3[0];
	
	for( int i = 0; i < 11; ++i )
	{
		MatrixDataPtr result = new MatrixData;
		interp( m0.get(), m1.get(), m2.get(), m3.get(), T(i)/10, result );
		Vec3<T> s, h, r, t;
		extractSHRT( result->readable(), s, h, r, t );
		
		Vec3<T> sTrue, hTrue, rTrue, tTrue;
		vectorInterp( s0, s1, s2, s3, T(i)/10, sTrue );
		vectorInterp( h0, h1, h2, h3, T(i)/10, hTrue );
		vectorInterp( t0, t1, t2, t3, T(i)/10, tTrue );
		
		// \todo: quaternion cubic interpolation doesn't currently
		// give the cubic interpolated angle in this case, so we're not checking r.
		// Maybe derive a formula for it?
		
		BOOST_CHECK_CLOSE( tTrue[0], t[0], 1.e-4 );
		
		for( int j=0; j < 3; ++j )
		{
			BOOST_CHECK_CLOSE( sTrue[j], s[j], 1.e-4 );
			BOOST_CHECK_CLOSE( hTrue[j], h[j], 1.e-4 );
		}
	}

}

template<typename T>
void MatrixCubicInterpolatorTest<T>::testVector()
{
	typedef typename Imath::Matrix44< T > Matrix;
	typedef typename Imath::Vec3< T > Vector;
	
	typedef std::vector< Matrix > MatrixVector;
	typedef std::vector< Vector > VectorVector;
	
	typedef TypedData< MatrixVector > MatrixVectorData;
	typedef typename MatrixVectorData::Ptr MatrixVectorDataPtr;
	
	typedef TypedData< VectorVector > VectorVectorData;
	typedef typename VectorVectorData::Ptr VectorVectorDataPtr;
	
	CubicInterpolator< MatrixVectorData > interp;
	CubicInterpolator< VectorVectorData > vectorInterp;
	
	// create vectors of s,h,r,t samples, fill them with arbitrary values and build matrices out of them:
	VectorVectorDataPtr sSamples[4];
	VectorVectorDataPtr hSamples[4];
	VectorVectorDataPtr rSamples[4];
	VectorVectorDataPtr tSamples[4];
	MatrixVectorDataPtr mSamples[4];
	
	for( int i=0; i < 4; ++i )
	{
		sSamples[i] = new VectorVectorData;
		hSamples[i] = new VectorVectorData;
		rSamples[i] = new VectorVectorData;
		tSamples[i] = new VectorVectorData;
		mSamples[i] = new MatrixVectorData;
		
		for( int j=0; j < 6; ++j )
		{
			T f = float( i * i ) / 16;
			sSamples[i]->writable().push_back( Vector( 1+f,1+f,1+f ) );
			hSamples[i]->writable().push_back( Vector( f,f,f ) );
			rSamples[i]->writable().push_back( Vector( 0,f + 0.5,0 ) );
			tSamples[i]->writable().push_back( Vector( f + 1,0,0 ) );
			
			Matrix mat;
			mat.setEulerAngles( rSamples[i]->readable().back() );
			mat.shear( hSamples[i]->readable().back() );
			mat.scale( sSamples[i]->readable().back() );
			mat[3][0] = tSamples[i]->readable().back()[0];

			mSamples[i]->writable().push_back( mat );
		}
	}
	
	for( int i = 0; i < 11; ++i )
	{
		MatrixVectorDataPtr result = new MatrixVectorData;
		
		interp( mSamples[0].get(), mSamples[1].get(), mSamples[2].get(), mSamples[3].get(), T(i)/10, result );
		
		VectorVectorDataPtr sTrue = new VectorVectorData;
		VectorVectorDataPtr hTrue = new VectorVectorData;
		VectorVectorDataPtr tTrue = new VectorVectorData;
		
		vectorInterp( sSamples[0].get(), sSamples[1].get(), sSamples[2].get(), sSamples[3].get(), T(i)/10, sTrue );
		vectorInterp( hSamples[0].get(), hSamples[1].get(), hSamples[2].get(), hSamples[3].get(), T(i)/10, hTrue );
		vectorInterp( tSamples[0].get(), tSamples[1].get(), tSamples[2].get(), tSamples[3].get(), T(i)/10, tTrue );
		
		// \todo: quaternion cubic interpolation doesn't currently
		// give the cubic interpolated angle in this case, so we're not checking r.
		// Maybe derive a formula for it?
		
		for( int j=0; j < 6; ++j )
		{
			
			Vector s, h, r, t;
			extractSHRT( result->readable()[j], s, h, r, t );
			
			//BOOST_CHECK_CLOSE( rTrue->readable()[j][1], r[1], 1.e-4 );
			BOOST_CHECK_CLOSE( tTrue->readable()[j][0], t[0], 1.e-4 );

			for( int k=0; k < 3; ++k )
			{
				BOOST_CHECK_CLOSE( sTrue->readable()[j][k], s[k], 1.e-4 );
				BOOST_CHECK_CLOSE( hTrue->readable()[j][k], h[k], 1.e-4 );
			}
		}
	
	}
	
}










}
