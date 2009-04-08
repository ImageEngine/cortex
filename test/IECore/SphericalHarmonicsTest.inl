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

#include <algorithm>

using namespace IECore;
using namespace Imath;
using namespace std;

namespace IECore
{


template<typename T >
void SphericalHarmonicsFunctionTest< T >::testEvaluation()
{
	T theta = 0.2;
	T phi = 0.3;
	T res;
	for ( unsigned int l = 0; l < 50; l++ )
	{
		for ( unsigned int m = -m; m <= l; m++ )
		{
			res = RealSphericalHarmonicFunction<T>::evaluate( l, m, theta, phi );
			BOOST_CHECK( !isnan( res ) );
		}
	}
}


template<typename T, int bands, unsigned int samples >
T SphericalHarmonicsProjectorTest< T, bands, samples >::lightFunctor( const Imath::Vec2<T> &polar )
{
	return max( T(0), T(5) * Imath::Math<T>::cos( polar.x ) - T(4) ) + max( T(0), T(-4) * Imath::Math<T>::sin( polar.x - M_PI ) * Imath::Math<T>::cos( polar.y - T(2.5) ) - T(3) );
}

template<typename T, int bands, unsigned int samples >
T SphericalHarmonicsProjectorTest< T, bands, samples >::polar1DFunctor( const Imath::Vec2<T> &polar )
{
	T sinTheta = Imath::Math<T>::sin( polar.x );
	Imath::V3f pos( sinTheta*Imath::Math<T>::cos( polar.y), sinTheta*Imath::Math<T>::sin( polar.y ), Imath::Math<T>::cos( polar.x ));
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
	T sinTheta = Imath::Math<T>::sin( polar.x );
	Imath::Vec3<T> pos( sinTheta*Imath::Math<T>::cos( polar.y), sinTheta*Imath::Math<T>::sin( polar.y ), Imath::Math<T>::cos( polar.x ));
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
	SphericalHarmonicsProjector<T> sampler( samples );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// test 1D polar projection
	SphericalHarmonics<T> sh1D( 4 );
	typename SphericalHarmonics<T>::CoefficientVector::iterator it;

	sampler.template polarProjection( lightFunctor, sh1D );
	
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
	SphericalHarmonicsProjectorExt<T> sampler( samples );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// test 1D polar projection
	std::vector< T > func1DValues;
	SphericalHarmonics<T> sh1D( bands );
	typename std::vector< T >::const_iterator it;

	sampler.template polarProjection( polar1DFunctor, sh1D );
	sampler.reconstruction( sh1D, func1DValues );

#ifdef SAVE_RECONSTRUCTION
	typename std::vector< Imath::V3f >::const_iterator eIt;
	PointsPrimitivePtr points = new PointsPrimitive( sampler.euclidianCoordinates().size() );
	V3fVectorDataPtr POINTS = new V3fVectorData();
	FloatVectorDataPtr RADIUS = new FloatVectorData();
	POINTS->writable().resize( sampler.euclidianCoordinates().size() );
	RADIUS->writable().resize( sampler.euclidianCoordinates().size(), 0.04 );
	points->variables["P"].data = POINTS;
	points->variables["P"].interpolation = PrimitiveVariable::Vertex;
	points->variables["constantwidth"].data = new FloatData(0.04);
	points->variables["constantwidth"].interpolation = PrimitiveVariable::Constant;
	typename std::vector<Imath::V3f>::iterator PIT = POINTS->writable().begin();
	for ( it = func1DValues.begin(), eIt = sampler.euclidianCoordinates().begin(); 
			it != func1DValues.end() && eIt != sampler.euclidianCoordinates().end(); it++, eIt++, PIT++ )
	{
		*PIT = *eIt * ( *it );
	}	
	Writer::create( points, "/tmp/reconstruction.cob" )->write();
#endif
	
	int errors = 0;
	T e = 0.15;
	for ( it = func1DValues.begin(), pIt = sampler.sphericalCoordinates().begin(); 
			it != func1DValues.end() && pIt != sampler.sphericalCoordinates().end(); it++, pIt++ )
	{
		if (!Imath::equalWithRelError ( polar1DFunctor(*pIt), *it, e))
		{
			cout << "Failed on sample " << ( pIt - sampler.sphericalCoordinates().begin() ) << endl;
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
	SphericalHarmonicsProjectorExt<T> sampler( samples );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// test 3D polar projection
	SphericalHarmonics< Imath::Vec3<T> > sh3D( bands );
	std::vector< Imath::Vec3<T> > func3DValues;
	typename std::vector< Imath::Vec3<T> >::const_iterator it3d;

	sampler.template polarProjection( polar3DFunctor, sh3D );
	sampler.reconstruction( sh3D, func3DValues );

#ifdef SAVE_RECONSTRUCTION
	PointsPrimitivePtr points = new PointsPrimitive( sampler.euclidianCoordinates().size() );
	V3fVectorDataPtr POINTS = new V3fVectorData();
	FloatVectorDataPtr RADIUS = new FloatVectorData();
	POINTS->writable().resize( sampler.euclidianCoordinates().size() );
	RADIUS->writable().resize( sampler.euclidianCoordinates().size(), 0.04 );
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
	for ( it3d = func3DValues.begin(), pIt = sampler.sphericalCoordinates().begin(); 
			it3d != func3DValues.end() && pIt != sampler.sphericalCoordinates().end(); it3d++, pIt++ )
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
	SphericalHarmonicsProjectorExt<T> sampler( samples );
	typename std::vector< Imath::Vec3<T> >::const_iterator eIt;

	std::vector< T > func1DValues;
	SphericalHarmonics<T> sh1D( bands );
	typename std::vector< T >::const_iterator it;

	// test 1D euclidian projection
	sampler.template euclideanProjection( euclidian1DFunctor, sh1D );
	sampler.reconstruction( sh1D, func1DValues );
	
	int errors = 0;
	T e = 0.15;
	for ( it = func1DValues.begin(), eIt = sampler.euclidianCoordinates().begin(); 
			it != func1DValues.end() && eIt != sampler.euclidianCoordinates().end(); it++, eIt++ )
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
	SphericalHarmonicsProjectorExt<T> sampler( samples );
	typename std::vector< Imath::Vec3<T> >::const_iterator eIt;

	// test 3D euclidian projection
	SphericalHarmonics< Imath::Vec3<T> > sh3D( bands );
	std::vector< Imath::Vec3<T> > func3DValues;
	typename std::vector< Imath::Vec3<T> >::const_iterator it3d;

	sampler.template euclideanProjection( euclidian3DFunctor, sh3D );
	sampler.reconstruction( sh3D, func3DValues );

	int errors = 0;
	T e = 0.3;
	for ( it3d = func3DValues.begin(), eIt = sampler.euclidianCoordinates().begin(); 
			it3d != func3DValues.end() && eIt != sampler.euclidianCoordinates().end(); it3d++, eIt++ )
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
	return Euler<T>( M_PI / 3, M_PI / 5, M_PI / 10 );
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

	SphericalHarmonicsRotationMatrix<T> shRotation;
	shRotation.setRotation( rotation().toMatrix44() );

	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> sampler( 5000 );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// apply euclidian projection
	SphericalHarmonics< T > sh( bands );

	std::vector< T > normalValues;
	std::vector< T > rotatedValues;
	typename std::vector< T >::const_iterator itN;
	typename std::vector< T >::const_iterator itR;

	sampler.template euclideanProjection( rotatedFunctor, sh );
	sampler.reconstruction( sh, rotatedValues );

	sampler.template euclideanProjection( normalFunctor, sh );
	sh *= shRotation;
	sampler.reconstruction( sh, normalValues );

#ifdef SAVE_ROTATION
	typename std::vector< Imath::Vec3<T> >::const_iterator eIt;
	PointsPrimitivePtr points = new PointsPrimitive( sampler.euclidianCoordinates().size() );
	V3fVectorDataPtr POINTS = new V3fVectorData();
	FloatVectorDataPtr RADIUS = new FloatVectorData();
	POINTS->writable().resize( sampler.euclidianCoordinates().size() );
	RADIUS->writable().resize( sampler.euclidianCoordinates().size(), 0.04 );
	points->variables["P"].data = POINTS;
	points->variables["P"].interpolation = PrimitiveVariable::Vertex;
	points->variables["constantwidth"].data = new FloatData(0.04);
	points->variables["constantwidth"].interpolation = PrimitiveVariable::Constant;
	typename std::vector<Imath::V3f>::iterator PIT = POINTS->writable().begin();
	for ( itN = normalValues.begin(), eIt = sampler.euclidianCoordinates().begin(); 
			itN != normalValues.end() && eIt != sampler.euclidianCoordinates().end(); itN++, eIt++, PIT++ )
	{
		*PIT = *eIt * ( *itN );
	}
	Writer::create( points, "/tmp/rotation.cob" )->write();
#endif

	int errors = 0;
	T e = 0.1;
	Imath::Matrix44<T> m = rotation().toMatrix44();
	for ( itN = normalValues.begin(), itR = rotatedValues.begin(); itN != normalValues.end(); itN++, itR++ )
	{
		if ( !Imath::equalWithRelError(*itR, *itN, e ) )
		{
			BOOST_CHECK_EQUAL( *itR, *itN );
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

	SphericalHarmonicsRotationMatrix<T> shRotation;
	shRotation.setRotation( rotation().toMatrix44() );

	// create normal distribution of samples
	SphericalHarmonicsProjectorExt<T> sampler( 5000 );
	typename std::vector< Imath::Vec2<T> >::const_iterator pIt;

	// apply euclidian projection
	SphericalHarmonics< Imath::Vec3<T> > sh( bands );

	std::vector< Imath::Vec3<T> > normalValues;
	std::vector< Imath::Vec3<T> > rotatedValues;
	typename std::vector< Imath::Vec3<T> >::const_iterator itN;
	typename std::vector< Imath::Vec3<T> >::const_iterator itR;

	sampler.template euclideanProjection( rotated3dFunctor, sh );
	sampler.reconstruction( sh, rotatedValues );

	sampler.template euclideanProjection( normal3dFunctor, sh );
	sh *= shRotation;

	sampler.reconstruction( sh, normalValues );

#ifdef SAVE_ROTATION
	PointsPrimitivePtr points = new PointsPrimitive( sampler.euclidianCoordinates().size() );
	V3fVectorDataPtr POINTS = new V3fVectorData();
	FloatVectorDataPtr RADIUS = new FloatVectorData();
	POINTS->writable().resize( sampler.euclidianCoordinates().size() );
	RADIUS->writable().resize( sampler.euclidianCoordinates().size(), 0.04 );
	points->variables["P"].data = POINTS;
	points->variables["P"].interpolation = PrimitiveVariable::Vertex;
	points->variables["constantwidth"].data = new FloatData(0.04);
	points->variables["constantwidth"].interpolation = PrimitiveVariable::Constant;
	typename std::vector<Imath::V3f>::iterator PIT = POINTS->writable().begin();
	for ( itN = normalValues.begin(); itN != normalValues.end(); itN++, PIT++ )
	{
		*PIT = *itN;
	}	
	Writer::create( points, "/tmp/rotation3d.cob" )->write();
#endif

	int errors = 0;
	T e = 0.3;
	Imath::Matrix44<T> m = rotation().toMatrix44();
	for ( itN = normalValues.begin(), itR = rotatedValues.begin(); itN != normalValues.end(); itN++, itR++ )
	{
		if ( !(*itR).equalWithAbsError(*itN, e ) )
		{
			BOOST_CHECK_EQUAL( *itR, *itN );
			if ( ++errors > 10 )
			{
				cout << "Could have more errors..." << endl;
				break;
			}
		}
	}
}

}
