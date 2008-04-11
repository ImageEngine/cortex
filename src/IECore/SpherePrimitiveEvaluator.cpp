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

#include <cassert>

#include "OpenEXR/ImathBoxAlgo.h"
#include "OpenEXR/ImathLineAlgo.h"

#include "IECore/PrimitiveVariable.h"
#include "IECore/Exception.h"
#include "IECore/SpherePrimitiveEvaluator.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace Imath;

static PrimitiveEvaluator::Description< SpherePrimitiveEvaluator > g_registraar = PrimitiveEvaluator::Description< SpherePrimitiveEvaluator >();

PrimitiveEvaluatorPtr SpherePrimitiveEvaluator::create( ConstPrimitivePtr primitive )
{
	ConstSpherePrimitivePtr sphere = runTimeCast< const SpherePrimitive >( primitive );
	assert ( sphere );

	return new SpherePrimitiveEvaluator( sphere );
}

SpherePrimitiveEvaluator::Result::Result()
{
}

V3f SpherePrimitiveEvaluator::Result::point() const
{
	return m_p;
}
								
V3f SpherePrimitiveEvaluator::Result::normal() const
{
	return m_p.normalized();
}

V2f SpherePrimitiveEvaluator::Result::uv() const
{
	V3f pn = point().normalized();
	
	/// \todo Once we support partial spheres we'll need to get these quantities from the primitive
	const float zMin = -1.0f;
	const float zMax = 1.0f;
	const float thetaMax = 2.0f * M_PI;
		
	const float phiMin = asin( zMin );
	const float phiMax = asin( zMax );		
	
	/// Simple rearrangement of equations in Renderman specification
	float phi = asin( pn.z );	
	float theta = acos( pn.x / cos(phi) ) ;
	float u = theta / thetaMax;
	float v = ( phi - phiMin ) / ( phiMax - phiMin );

	return V2f( u, v );
}

V3f SpherePrimitiveEvaluator::Result::uTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

V3f SpherePrimitiveEvaluator::Result::vTangent() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

V3f SpherePrimitiveEvaluator::Result::vectorPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< V3f >( pv );
}

float SpherePrimitiveEvaluator::Result::floatPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< float >( pv );
}

int SpherePrimitiveEvaluator::Result::intPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< int >( pv );
}

const std::string &SpherePrimitiveEvaluator::Result::stringPrimVar( const PrimitiveVariable &pv ) const
{
	ConstStringDataPtr data = runTimeCast< const StringData >( pv.data );
		
	if (data)
	{
		return data->readable();
	}	
	else
	{
		ConstStringVectorDataPtr data = runTimeCast< const StringVectorData >( pv.data );
		
		if (data)
		{		
			return data->readable()[0];
		}
	}
	
	throw InvalidArgumentException( "Could not retrieve primvar data for SpherePrimitiveEvaluator" );
}
	
Color3f SpherePrimitiveEvaluator::Result::colorPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< Color3f >( pv );
}

half SpherePrimitiveEvaluator::Result::halfPrimVar( const PrimitiveVariable &pv ) const
{
	return getPrimVar< half >( pv );
}

template<typename T>
T SpherePrimitiveEvaluator::Result::getPrimVar( const PrimitiveVariable &pv ) const
{
	if ( pv.interpolation == PrimitiveVariable::Constant )
	{
		typedef TypedData<T> Data;
		typedef typename Data::Ptr DataPtr;
		
		DataPtr data = runTimeCast< Data >( pv.data );
		
		if (data)
		{
			return data->readable();
		}
	}
	
	typedef TypedData< std::vector<T> > VectorData;
	typedef typename VectorData::Ptr VectorDataPtr;
	VectorDataPtr data = runTimeCast< VectorData >( pv.data );

	if (!data)
	{
		throw InvalidArgumentException( "Could not retrieve primvar data for SpherePrimitiveEvaluator" );
	}

	switch ( pv.interpolation )
	{
		case PrimitiveVariable::Constant :
		case PrimitiveVariable::Uniform :		
			assert( data->readable().size() == 1 );
			
			return data->readable()[0];

		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying:
		case PrimitiveVariable::FaceVarying:
			{		
				assert( data->readable().size() == 4 );
				
				V2f uvCoord = uv();
				
				float u = uvCoord.x;
				float v = uvCoord.y;
				
				/// These match 3delight
				const T &f00 = data->readable()[0];
				const T &f10 = data->readable()[1];
				const T &f01 = data->readable()[2];
				const T &f11 = data->readable()[3];
				
				/// Bilinear interpolation
				return T( f00 * (1-u) * (1-v) + f10 * u * (1-v) + f01 * (1-u) * v + f11 * u * v );
			}
			break;
		
		default :
			/// Unimplemented primvar interpolation
			assert( false );
			return T();

	}
	
	throw NotImplementedException( __PRETTY_FUNCTION__ );			
}

SpherePrimitiveEvaluator::SpherePrimitiveEvaluator( ConstSpherePrimitivePtr sphere ) 
{
	assert( sphere );
	
	m_sphere = sphere->copy();
}

SpherePrimitiveEvaluator::~SpherePrimitiveEvaluator()
{
}

PrimitiveEvaluator::ResultPtr SpherePrimitiveEvaluator::createResult() const
{
      return new Result();
}

void SpherePrimitiveEvaluator::validateResult( const PrimitiveEvaluator::ResultPtr &result ) const
{
	if (! boost::dynamic_pointer_cast< SpherePrimitiveEvaluator::Result >( result ) )
	{
		throw InvalidArgumentException("SpherePrimitiveEvaluator: Invalid PrimitiveEvaulator result type");
	}
}

ConstPrimitivePtr SpherePrimitiveEvaluator::primitive() const
{
	return m_sphere;
}

bool SpherePrimitiveEvaluator::closestPoint( const V3f &p, const PrimitiveEvaluator::ResultPtr &result ) const
{		
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	
	ResultPtr sr = boost::static_pointer_cast< Result >( result );
	
	sr->m_p = p.normalized() * m_sphere->radius();

	return true;	
}

bool SpherePrimitiveEvaluator::pointAtUV( const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const
{
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	
	ResultPtr sr = boost::static_pointer_cast< Result >( result );
	
	/// \todo Once we support partial spheres we'll need to get these quantities from the primitive
	const float zMin = -1.0f;
	const float zMax = 1.0f;
	const float thetaMax = 2.0f * M_PI;
	
	/// This is from the Renderman specification
	const float phiMin = asin( zMin );
	const float phiMax = asin( zMax );
	
	float phi = phiMin + uv.y  * ( phiMax - phiMin );
	float theta = uv.x * thetaMax;
	
	sr->m_p = m_sphere->radius() * V3f(
		 cos( theta ) * cos( phi ),
		 sin( theta ) * cos( phi ),
		 sin( phi )
	);
	
	return true;		
}

/// Implementation derived from Wild Magic (Version 2) Software Library, available
/// from http://www.geometrictools.com/Downloads/WildMagic2p5.zip under free license
bool SpherePrimitiveEvaluator::intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction, 
	const PrimitiveEvaluator::ResultPtr &result, float maxDistance ) const
{
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	
	ResultPtr sr = boost::static_pointer_cast< Result >( result );
	
	Imath::V3f dir = direction.normalized();
	(void)direction;
	float a0 = origin.dot( origin ) - m_sphere->radius() * m_sphere->radius();
	float a1 = dir.dot( origin );
	float discr = a1 * a1 - a0;
	
	if (discr < 0.0)
	{
		return false;
	}
	
	if ( discr >= Imath::limits<float>::epsilon() )
	{
		float root = sqrt( discr );
		float t0 = -a1 - root;
		float t1 = -a1 + root;
		
		Imath::V3f p0 = origin + t0 * dir;
		Imath::V3f p1 = origin + t1 * dir;
		
		if ( t0 >= 0.0 )
		{
			if ( t1 >= 0.0 )
			{
				if ( (origin - p0).length2() < ( origin - p1 ).length2() )
				{
					sr->m_p = p0;
				}
				else
				{
					sr->m_p = p1;
				}
			}
			else
			{
				sr->m_p = p0;
			}
		}
		else if ( t1 >= 0.0 )
		{
			sr->m_p = p1;
		}
		else
		{
			return false;
		}
	}
	else
	{
		float t = -a1;
		
		if ( t >= 0.0 )
		{		
			sr->m_p = origin + t * dir;		
		}
		else
		{
			return false;
		}
	}
	
	if ( (sr->m_p - origin).length() < maxDistance)
	{		
		return true;
	}
	
	return false;
}

/// Implementation derived from Wild Magic (Version 2) Software Library, available
/// from http://www.geometrictools.com/Downloads/WildMagic2p5.zip under free license			
int SpherePrimitiveEvaluator::intersectionPoints( const Imath::V3f &origin, const Imath::V3f &direction, 
	std::vector<PrimitiveEvaluator::ResultPtr> &results, float maxDistance ) const
{	
	results.clear();

	Imath::V3f dir = direction.normalized();
	(void)direction;
	float a0 = origin.dot(origin) - m_sphere->radius() * m_sphere->radius();
	float a1 = dir.dot( origin );
	float discr = a1 * a1 - a0;
	
	if (discr < 0.0)
	{
		return 0;
	}
	
	if ( discr >= Imath::limits<float>::epsilon() )
	{
		float root = sqrt( discr );
		float t0 = -a1 - root;
		
		if ( t0 >= 0.0 )
		{
			Imath::V3f p0 = origin + t0 * dir;
			if ( (origin - p0).length() < maxDistance )
			{
				ResultPtr r = boost::static_pointer_cast< Result > ( createResult() );
				r->m_p = p0;
				
				results.push_back( r );
			}
		}
		
		float t1 = -a1 + root;
		if ( t1 >= 0.0 )
		{
			Imath::V3f p1 = origin + t1 * dir;
			if ( (origin - p1).length() < maxDistance )
			{
				ResultPtr r = boost::static_pointer_cast< Result > ( createResult() );
				r->m_p = p1;
				
				results.push_back( r );
			}
		}
	}
	else
	{
		float t = -a1;
		
		if ( t >= 0.0 )
		{		
			Imath::V3f p = origin + t * dir;
			if ( (origin - p).length() < maxDistance )
			{
				ResultPtr r = boost::static_pointer_cast< Result > ( createResult() );							
				r->m_p = p;

				results.push_back( r );
			}	
		}
	}
	
	assert( results.size() >= 0 );
	assert( results.size() <= 2 );	
	
	return results.size();	
}

float SpherePrimitiveEvaluator::volume() const
{
	float r = m_sphere->radius();
	
	return 4.0/3.0 * M_PI * r*r*r ;
}

V3f SpherePrimitiveEvaluator::centerOfGravity() const
{
	return V3f( 0.0f, 0.0f, 0.0f );
}

float SpherePrimitiveEvaluator::surfaceArea() const
{
	float r = m_sphere->radius();
	
	return 4.0 * M_PI * r*r ;
}
