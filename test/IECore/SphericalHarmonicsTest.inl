//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathMath.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/Writer.h"
#include "IECore/SphericalToEuclidianTransform.h"
#include "IECore/EuclidianToSphericalTransform.h"
#include "IECore/Interpolator.h"

#include <algorithm>
#include <vector>

using namespace IECore;
using namespace Imath;
using namespace std;

namespace IECore
{


template<typename T >
void SphericalHarmonicsTest< T >::testFunctionEvaluation()
{
	T theta = 0.2;
	T phi = 0.3;
	T res;
	for ( unsigned int l = 0; l < 25; l++ )
	{
		for ( int m = -(int)l; m <= (int)l; m++ )
		{
			res = RealSphericalHarmonicFunction<T>::evaluate( phi, theta, l, m );
			BOOST_CHECK( !isnan( res ) );
			SphericalHarmonics< T > sh( l+1 );
			sh.coefficients()[ l*(l+1)+m ] = 1;
			BOOST_CHECK_EQUAL( sh( Imath::Vec2<T>( phi, theta ) ), res );
		}
	}
}

template<typename T >
void SphericalHarmonicsTest< T >::testConstruction()
{
	// default constructor
	SphericalHarmonics< T > sh1;
	BOOST_CHECK_EQUAL( sh1.bands(), (unsigned)0 );
	BOOST_CHECK_EQUAL( sh1.coefficients().size(), (unsigned)0 );
	SphericalHarmonics< T > sh2( 3 );
	BOOST_CHECK_EQUAL( sh2.bands(), (unsigned)3 );
	BOOST_CHECK_EQUAL( sh2.coefficients().size(), (unsigned)3*3 );
	BOOST_CHECK_EQUAL( sh2.coefficients()[ 2 ], 0 );

	// write access to coefficients
	for ( int i = 0; i < 3*3; i++ )
	{
		sh2.coefficients()[ i ] = i+1;
	}

	// copy constructor
	SphericalHarmonics< T > sh3( sh2 );
	BOOST_CHECK_EQUAL( sh3.bands(), (unsigned)3 );
	BOOST_CHECK_EQUAL( sh3.coefficients().size(), (unsigned)3*3 );
	for ( int i = 0; i < 3*3; i++ )
	{
		BOOST_CHECK_EQUAL( sh3.coefficients()[ i ], i + 1 ); 
	}
	
	// copy operation
	SphericalHarmonics< T > sh4( 1 );
	sh4 = sh2;
	BOOST_CHECK_EQUAL( sh4.bands(), (unsigned)3 );
	BOOST_CHECK_EQUAL( sh4.coefficients().size(), (unsigned)3*3 );
	for ( int i = 0; i < 3*3; i++ )
	{
		BOOST_CHECK_EQUAL( sh4.coefficients()[ i ], i + 1 ); 
	}

	// setting number of bands
	sh4.setBands( 2 );
	BOOST_CHECK_EQUAL( sh4.bands(), (unsigned)2 );
	BOOST_CHECK_EQUAL( sh4.coefficients().size(), (unsigned)2*2 );

	sh4.setBands( 4 );
	BOOST_CHECK_EQUAL( sh4.bands(), (unsigned)4 );
	BOOST_CHECK_EQUAL( sh4.coefficients().size(), (unsigned)4*4 );
	for ( int i = 0; i < 2*2; i++ )
	{
		BOOST_CHECK_EQUAL( sh4.coefficients()[ i ], i + 1 ); 
	}
	for ( int i = 2*2; i < 4*4; i++ )
	{
		BOOST_CHECK_EQUAL( sh4.coefficients()[ i ], 0 ); 
	}
}

template<typename T >
void SphericalHarmonicsTest< T >::testDotProduct()
{
	SphericalHarmonics< T > sh1, sh2( 3 );
	for ( int i = 0; i < 3*3; i++ )
	{
		sh2.coefficients()[ i ] = i+1;
	}

	// same size
	sh1 = sh2;
	sh1.setBands( 2 );

	BOOST_CHECK_EQUAL( (sh1.template dot< T, T >( sh1 )), 1*1 + 2*2 + 3*3 + 4*4 );
	BOOST_CHECK_EQUAL( (sh2.template dot< T, T >( sh2 )), 1*1 + 2*2 + 3*3 + 4*4 + 5*5 + 6*6 + 7*7 + 8*8 + 9*9 );
	
	// bigger
	BOOST_CHECK_EQUAL( (sh1.template dot< T, T >( sh2 )), 1*1 + 2*2 + 3*3 + 4*4 );

	// smaller
	BOOST_CHECK_EQUAL( ( sh2 ^ sh1 ), 1*1 + 2*2 + 3*3 + 4*4 );

}

template<typename T >
void SphericalHarmonicsTest< T >::testArithmeticOperations()
{

	SphericalHarmonics< T > sh1( 3 ), sh2, sh3, sh4, sh2b, sh3b, sh4b, sh5;
	for ( int i = 0; i < 3*3; i++ )
	{
		sh1.coefficients()[ i ] = i+1;
	}

	// plus
	sh2 = sh1 + sh1;
	// inplace plus
	sh2b = sh1;
	sh2b += sh1;
	// minus
	sh3 = sh1 - sh1;
	// inplace minus
	sh3b = sh1;
	sh3b -= sh1;
	// scale
	sh4 = sh1 * 3;
	// inplace scale
	sh4b = sh1;
	sh4b *= 3;
	// linear interpolation
	LinearInterpolator< SphericalHarmonics< T > >()( sh1, sh4, 0.5, sh5 );

	for ( int i = 0; i < 3*3; i++ )
	{
		
		BOOST_CHECK_EQUAL( sh2.coefficients()[i], (i+1)*2 );
		BOOST_CHECK_EQUAL( sh3.coefficients()[i], 0 );
		BOOST_CHECK_EQUAL( sh4.coefficients()[i], (i+1)*3 );

		T v = sh5.coefficients()[i];
		if ( fabs( v - (T)(i+1)*2) > 0.01 )
		{
			BOOST_CHECK_EQUAL( sh5.coefficients()[i], (T)(i+1)*2 );
		}

		// inplace checks
		BOOST_CHECK_EQUAL( sh2b.coefficients()[i], (i+1)*2 );
		BOOST_CHECK_EQUAL( sh3b.coefficients()[i], 0 );
		BOOST_CHECK_EQUAL( sh4b.coefficients()[i], (i+1)*3 );

	}

}

template<typename T, int bands, unsigned int samples >
T SphericalHarmonicsProjectorTest< T, bands, samples >::lightFunctor( const Imath::Vec2<T> &polar )
{
	return max( T(0), T(5) * Imath::Math<T>::cos( polar.y ) - T(4) ) + max( T(0), T(-4) * Imath::Math<T>::sin( polar.y - M_PI ) * Imath::Math<T>::cos( polar.x - T(2.5) ) - T(3) );
}

template<typename T, int bands, unsigned int samples >
T SphericalHarmonicsProjectorTest< T, bands, samples >::polar1DFunctor( const Imath::Vec2<T> &polar )
{
	T sinTheta = Imath::Math<T>::sin( polar.y );
	Imath::V3f pos( sinTheta*Imath::Math<T>::cos( polar.x), sinTheta*Imath::Math<T>::sin( polar.x ), Imath::Math<T>::cos( polar.y ));
	return euclidian1DFunctor( pos );
}

template<typename T, int bands, unsigned int samples >
T SphericalHarmonicsProjectorTest< T, bands, samples >::euclidian1DFunctor( const Imath::Vec3<T> &pos )
{
	Vec3<T> res = euclidian3DFunctor( pos );
	return res.length();
}

template<typename T, int bands, unsigned int samples >
Imath::Vec3<T> SphericalHarmonicsProjectorTest< T, bands, samples >::polar3DFunctor( const Imath::Vec2<T> &polar )
{
	T sinTheta = Imath::Math<T>::sin( polar.y );
	Imath::Vec3<T> pos( sinTheta*Imath::Math<T>::cos( polar.x), sinTheta*Imath::Math<T>::sin( polar.x ), Imath::Math<T>::cos( polar.y ));
	return euclidian3DFunctor( pos );
}

template<typename T, int bands, unsigned int samples >
Imath::Vec3<T> SphericalHarmonicsProjectorTest< T, bands, samples >::euclidian3DFunctor( const Imath::Vec3<T> &pos )
{
	Imath::Vec3<T> res( Imath::Math<T>::fabs(pos.x), Imath::Math<T>::fabs(pos.y), Imath::Math<T>::fabs(pos.z) );
	// project to a cube of side 2.
	return  pos * (2. / max( res.x, max( res.y, res.z ) ));
}

template<typename T, int bands, unsigned int samples >
void SphericalHarmonicsProjectorTest< T, bands, samples >::testProjection()
{
	T target[] = {
		0.39925,
		-0.21075, 0.28687, 0.28277,
		-0.31530, -0.0004, 0.13159, 0.00098, 0.09359,
		-0.25, -0.00072, 0.12290, 0.30458, -0.16427, -0.00062, -0.09126
	};

	// create normal distribution
	SphericalHarmonicsProjector<T> projector( samples );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// test 1D polar projection
	SphericalHarmonics<T> sh1D( 4 );
	typename SphericalHarmonics<T>::CoefficientVector::iterator it;

	for ( unsigned int i = 0; i < samples; i++ )
	{
		projector.template operator()<>( i, lightFunctor( projector.sphericalCoordinates()[i] ), sh1D );
	}

	T e = 0.01;
	int i = 0;
	for ( it = sh1D.coefficients().begin(); it != sh1D.coefficients().end(); it++, i++ )
	{
		if (!Imath::equalWithAbsError ( *it, target[i], e))
		{
			cout << "Failed on coefficient " << i << endl;
			BOOST_CHECK_EQUAL( *it, target[i] );
		}
	}

}

// extension class for SphericalHarmonicsProjector. Adds reconstruction for testing purposes.
template< typename V >
class SphericalHarmonicsProjectorExt : public SphericalHarmonicsProjector<V>
{
	public:

		SphericalHarmonicsProjectorExt( unsigned int samples ) :
			SphericalHarmonicsProjector<V>( samples )
		{
		}

		template< typename T >
		void reconstruction( const SphericalHarmonics< T > &sh, std::vector< T > &result )
		{
			result.resize( this->m_shEvaluations.size() );

			typename std::vector< T >::iterator rit = result.begin();
			typename SphericalHarmonics<T>::CoefficientVector::const_iterator shIt;
			typename SphericalHarmonicsProjector<V>::EvaluationVector::const_iterator eIt;
			typename SphericalHarmonicsProjector<V>::EvaluationSamples::const_iterator it;

			for ( it = this->m_shEvaluations.begin(); it != this->m_shEvaluations.end(); it++, rit++ )
			{
				T acc(0);
				// multiplies the spherical harmonics coefficients by their evaluations at each sampling point.
				for ( shIt = sh.coefficients().begin(), eIt = it->begin(); shIt != sh.coefficients().end() && eIt != it->end(); shIt++, eIt++ )
				{
					acc += (*shIt) * (*eIt);
				}
				*rit = acc;
			}
		}

};


template<typename T, int bands, unsigned int samples >
void SphericalHarmonicsProjectorTest< T, bands, samples >::testPolarProjection1D()
{
	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> projector( samples );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// test 1D polar projection
	std::vector< T > func1DValues;
	SphericalHarmonics<T> sh1D( bands );
	typename std::vector< T >::const_iterator it;

	projector.template polarProjection<>( polar1DFunctor, sh1D );
	projector.reconstruction( sh1D, func1DValues );

#ifdef SAVE_RECONSTRUCTION
	typename std::vector< Imath::V3f >::const_iterator eIt;
	PointsPrimitivePtr points = new PointsPrimitive( projector.euclidianCoordinates().size() );
	V3fVectorDataPtr POINTS = new V3fVectorData();
	FloatVectorDataPtr RADIUS = new FloatVectorData();
	POINTS->writable().resize( projector.euclidianCoordinates().size() );
	RADIUS->writable().resize( projector.euclidianCoordinates().size(), 0.04 );
	points->variables["P"].data = POINTS;
	points->variables["P"].interpolation = PrimitiveVariable::Vertex;
	points->variables["constantwidth"].data = new FloatData(0.04);
	points->variables["constantwidth"].interpolation = PrimitiveVariable::Constant;
	typename std::vector<Imath::V3f>::iterator PIT = POINTS->writable().begin();
	for ( it = func1DValues.begin(), eIt = projector.euclidianCoordinates().begin();
			it != func1DValues.end() && eIt != projector.euclidianCoordinates().end(); it++, eIt++, PIT++ )
	{
		*PIT = *eIt * ( *it );
	}
	Writer::create( points, "/tmp/reconstruction.cob" )->write();
#endif

	int errors = 0;
	T e = 0.15;
	for ( it = func1DValues.begin(), pIt = projector.sphericalCoordinates().begin();
			it != func1DValues.end() && pIt != projector.sphericalCoordinates().end(); it++, pIt++ )
	{
		if (!Imath::equalWithRelError ( polar1DFunctor(*pIt), *it, e))
		{
			cout << "Failed on sample " << ( pIt - projector.sphericalCoordinates().begin() ) << endl;
			BOOST_CHECK_EQUAL( *it, polar1DFunctor( *pIt ) );
			if ( ++errors > 10 )
			{
				cout << "Could have more errors..." << endl;
				break;
			}
		}
	}

}

template<typename T, int bands, unsigned int samples >
void SphericalHarmonicsProjectorTest< T, bands, samples >::testPolarProjection3D()
{
	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> projector( samples );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// test 3D polar projection
	SphericalHarmonics< Imath::Vec3<T> > sh3D( bands );
	std::vector< Imath::Vec3<T> > func3DValues;
	typename std::vector< Imath::Vec3<T> >::const_iterator it3d;

	projector.template polarProjection<>( polar3DFunctor, sh3D );
	projector.reconstruction( sh3D, func3DValues );

#ifdef SAVE_RECONSTRUCTION
	PointsPrimitivePtr points = new PointsPrimitive( projector.euclidianCoordinates().size() );
	V3fVectorDataPtr POINTS = new V3fVectorData();
	FloatVectorDataPtr RADIUS = new FloatVectorData();
	POINTS->writable().resize( projector.euclidianCoordinates().size() );
	RADIUS->writable().resize( projector.euclidianCoordinates().size(), 0.04 );
	points->variables["P"].data = POINTS;
	points->variables["P"].interpolation = PrimitiveVariable::Vertex;
	points->variables["constantwidth"].data = new FloatData(0.04);
	points->variables["constantwidth"].interpolation = PrimitiveVariable::Constant;
	typename std::vector<Imath::V3f>::iterator PIT = POINTS->writable().begin();
	for ( it3d = func3DValues.begin(); it3d != func3DValues.end(); it3d++, PIT++ )
	{
		*PIT = *it3d;
	}
	Writer::create( points, "/tmp/reconstruction3d.cob" )->write();
#endif

	int errors = 0;
	T e = 0.3;
	for ( it3d = func3DValues.begin(), pIt = projector.sphericalCoordinates().begin();
			it3d != func3DValues.end() && pIt != projector.sphericalCoordinates().end(); it3d++, pIt++ )
	{
		if ( !(*it3d).equalWithAbsError( polar3DFunctor( *pIt ), e) )
		{
			BOOST_CHECK_EQUAL( *it3d, polar3DFunctor( *pIt ) );
			if ( ++errors > 10 )
			{
				cout << "Could have more errors..." << endl;
				break;
			}
		}
	}

}

template<typename T, int bands, unsigned int samples >
void SphericalHarmonicsProjectorTest<T, bands, samples>::testEuclidianProjection1D()
{
	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> projector( samples );
	typename std::vector< Imath::Vec3<T> >::const_iterator eIt;

	std::vector< T > func1DValues;
	SphericalHarmonics<T> sh1D( bands );
	typename std::vector< T >::const_iterator it;

	// test 1D euclidian projection
	projector.template euclideanProjection<>( euclidian1DFunctor, sh1D );
	projector.reconstruction( sh1D, func1DValues );

	int errors = 0;
	T e = 0.15;
	for ( it = func1DValues.begin(), eIt = projector.euclidianCoordinates().begin();
			it != func1DValues.end() && eIt != projector.euclidianCoordinates().end(); it++, eIt++ )
	{
		if (!Imath::equalWithRelError ( *it, euclidian1DFunctor(*eIt), e))
		{
			BOOST_CHECK_EQUAL( *it, euclidian1DFunctor( *eIt ) );
			if ( ++errors > 10 )
			{
				cout << "Could have more errors..." << endl;
				break;
			}
		}
	}
}

template<typename T, int bands, unsigned int samples >
void SphericalHarmonicsProjectorTest<T, bands, samples>::testEuclidianProjection3D()
{
	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> projector( samples );
	typename std::vector< Imath::Vec3<T> >::const_iterator eIt;

	// test 3D euclidian projection
	SphericalHarmonics< Imath::Vec3<T> > sh3D( bands );
	std::vector< Imath::Vec3<T> > func3DValues;
	typename std::vector< Imath::Vec3<T> >::const_iterator it3d;

	projector.template euclideanProjection<>( euclidian3DFunctor, sh3D );
	projector.reconstruction( sh3D, func3DValues );

	int errors = 0;
	T e = 0.3;
	for ( it3d = func3DValues.begin(), eIt = projector.euclidianCoordinates().begin();
			it3d != func3DValues.end() && eIt != projector.euclidianCoordinates().end(); it3d++, eIt++ )
	{
		if ( !(*it3d).equalWithAbsError( euclidian3DFunctor( *eIt ), e) )
		{
			BOOST_CHECK_EQUAL( *it3d, euclidian3DFunctor( *eIt ) );
			if ( ++errors > 10 )
			{
				cout << "Could have more errors..." << endl;
				break;
			}
		}
	}
}

template< typename T >
Euler<T> SphericalHarmonicsRotationMatrixTest<T>::rotation()
{
	return Euler<T>(  M_PI * 1.8, M_PI * 0.8, M_PI * 0.5 );
}

template< typename T >
T SphericalHarmonicsRotationMatrixTest<T>::normalFunctor( const Imath::Vec3<T> &pos )
{
	return SphericalHarmonicsProjectorTest< T,10,20000 >::euclidian1DFunctor( pos );
}

template< typename T >
T SphericalHarmonicsRotationMatrixTest<T>::rotatedFunctor( const Imath::Vec3<T> &pos )
{
	return SphericalHarmonicsProjectorTest< T,10,20000 >::euclidian1DFunctor( pos * rotation().toMatrix44() );
}

template< typename T >
Imath::Vec3<T> SphericalHarmonicsRotationMatrixTest<T>::normal3dFunctor( const Imath::Vec3<T> &pos )
{
	return SphericalHarmonicsProjectorTest< T,10,20000 >::euclidian3DFunctor( pos );
}

template< typename T >
Imath::Vec3<T> SphericalHarmonicsRotationMatrixTest<T>::rotated3dFunctor( const Imath::Vec3<T> &pos )
{
	return SphericalHarmonicsProjectorTest< T,10,20000 >::euclidian3DFunctor( pos * rotation().toMatrix44() );
}

template< typename T >
void SphericalHarmonicsRotationMatrixTest<T>::testRotation()
{
	unsigned int bands = 5;

	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> projector( 5000 );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	SphericalHarmonics< T > sh( bands );
	projector.template euclideanProjection<>( normalFunctor, sh );

	SphericalHarmonics< T > shRot = sh;
	SphericalHarmonicsRotationMatrix<T> shRotation;

	Imath::Matrix44<T> m = rotation().toMatrix44();
	shRotation.setRotation( m );
	shRot *= shRotation;

	SphericalToEuclidianTransform< Imath::Vec2<T>, Imath::Vec3<T> > sph2euc;
	EuclidianToSphericalTransform< Imath::Vec3<T>, Imath::Vec2<T> > euc2sph;

	typename std::vector< Imath::Vec2< T > >::const_iterator it;
	int errors = 0;
	T e = 0.01;
	Imath::Vec3<T> euc;
	Imath::Vec2<T> sph;
	T rotSHValue, shValueRot;

	m.gjInvert();
	for ( it = projector.sphericalCoordinates().begin(); it != projector.sphericalCoordinates().end(); it++ )
	{
		euc = sph2euc.transform( *it );
		sph = euc2sph.transform( euc );

		if ( !(*it).equalWithRelError(sph, e ) )
		{
			BOOST_CHECK_EQUAL( *it, sph );
		}

		rotSHValue = shRot( *it );
		shValueRot = sh( euc2sph( euc * m ) ); 

		if (!Imath::equalWithRelError ( shValueRot,rotSHValue, e))
		{
			BOOST_CHECK_EQUAL( shValueRot, rotSHValue );
			if ( ++errors > 10 )
			{
				cout << "Could have more errors..." << endl;
				break;
			}
		}
	}
}

template< typename T >
void SphericalHarmonicsRotationMatrixTest<T>::testRotation3D()
{
	unsigned int bands = 5;

	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> projector( 5000 );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	SphericalHarmonics< Imath::Vec3<T> > sh( bands );
	projector.template euclideanProjection<>( normal3dFunctor, sh );

	SphericalHarmonics< Imath::Vec3<T> > shRot = sh;
	SphericalHarmonicsRotationMatrix<T> shRotation;

	Imath::Matrix44<T> m = rotation().toMatrix44();
	shRotation.setRotation( m );
	shRot *= shRotation;

	SphericalToEuclidianTransform< Imath::Vec2<T>, Imath::Vec3<T> > sph2euc;
	EuclidianToSphericalTransform< Imath::Vec3<T>, Imath::Vec2<T> > euc2sph;

	typename std::vector< Imath::Vec2< T > >::const_iterator it;
	int errors = 0;
	T e = 0.01;
	Imath::Vec3<T> euc;
	Imath::Vec2<T> sph;
	Imath::Vec3<T> rotSHValue, shValueRot;

	m.gjInvert();
	for ( it = projector.sphericalCoordinates().begin(); it != projector.sphericalCoordinates().end(); it++ )
	{
		euc = sph2euc.transform( *it );
		sph = euc2sph.transform( euc );

		if ( !(*it).equalWithRelError(sph, e ) )
		{
			BOOST_CHECK_EQUAL( *it, sph );
		}

		rotSHValue = shRot( *it );
		shValueRot = sh( euc2sph( euc * m ) ); 

		if ( !shValueRot.equalWithRelError(rotSHValue, e ) )
		{
			BOOST_CHECK_EQUAL( shValueRot, rotSHValue );
			if ( ++errors > 10 )
			{
				cout << "Could have more errors..." << endl;
				break;
			}
		}
	}
}

}
