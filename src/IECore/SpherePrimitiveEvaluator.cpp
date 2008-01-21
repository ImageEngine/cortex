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

SpherePrimitiveEvaluator::Result::Result()
{
}

V3f SpherePrimitiveEvaluator::Result::point() const
{
	return m_p;
}
								
V3f SpherePrimitiveEvaluator::Result::normal() const
{
	return m_n;
}

V2f SpherePrimitiveEvaluator::Result::uv() const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
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
	throw NotImplementedException( __PRETTY_FUNCTION__ );
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
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

SpherePrimitiveEvaluator::SpherePrimitiveEvaluator( const Imath::V3f &center, float radius ) : m_center( center ), m_radius( radius )
{
}

SpherePrimitiveEvaluator::~SpherePrimitiveEvaluator()
{
}

PrimitiveEvaluator::ResultPtr SpherePrimitiveEvaluator::createResult() const
{
      return new Result();
}

bool SpherePrimitiveEvaluator::closestPoint( const V3f &p, const PrimitiveEvaluator::ResultPtr &result ) const
{		
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	
	ResultPtr sr = boost::static_pointer_cast< Result >( result );
	
	sr->m_n = (p - m_center).normalized();
	sr->m_p = (sr->m_n * m_radius) + m_center;

	return true;	
}

bool SpherePrimitiveEvaluator::pointAtUV( const Imath::V2f &uv, const PrimitiveEvaluator::ResultPtr &result ) const
{
	throw NotImplementedException( __PRETTY_FUNCTION__ );
}

/// Implementation derived from Wild Magic (Version 2) Software Library, available
/// from http://www.geometrictools.com/Downloads/WildMagic2p5.zip under free license
bool SpherePrimitiveEvaluator::intersectionPoint( const Imath::V3f &origin, const Imath::V3f &direction, 
	const PrimitiveEvaluator::ResultPtr &result, float maxDistance ) const
{
	assert( boost::dynamic_pointer_cast< Result >( result ) );
	
	ResultPtr sr = boost::static_pointer_cast< Result >( result );

	Imath::V3f diff = origin - m_center;
	float a0 = diff.dot(diff) - m_radius * m_radius;
	float a1 = direction.dot( diff );
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
		
		Imath::V3f p0 = origin + t0 * direction;
		Imath::V3f p1 = origin + t1 * direction;
		
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
			sr->m_p = origin + t * direction;		
		}
		else
		{
			return false;
		}
	}
	
	if ( (sr->m_p - origin).length() < maxDistance)
	{		
		sr->m_n = ( sr->m_p - m_center ).normalized();
		
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

	Imath::V3f diff = origin - m_center;
	float a0 = diff.dot(diff) - m_radius * m_radius;
	float a1 = direction.dot( diff );
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
			Imath::V3f p0 = origin + t0 * direction;
			if ( (origin - p0).length() < maxDistance )
			{
				ResultPtr r = boost::static_pointer_cast< Result > ( createResult() );
				r->m_p = p0;
				r->m_n = ( r->m_p - m_center ).normalized();

				results.push_back( r );
			}
		}
		
		float t1 = -a1 + root;
		if ( t1 >= 0.0 )
		{
			Imath::V3f p1 = origin + t1 * direction;
			if ( (origin - p1).length() < maxDistance )
			{
				ResultPtr r = boost::static_pointer_cast< Result > ( createResult() );
				r->m_p = p1;
				r->m_n = ( r->m_p - m_center ).normalized();

				results.push_back( r );
			}
		}
	}
	else
	{
		float t = -a1;
		
		if ( t >= 0.0 )
		{		
			Imath::V3f p = origin + t * direction;
			if ( (origin - p).length() < maxDistance )
			{
				ResultPtr r = boost::static_pointer_cast< Result > ( createResult() );							
				r->m_p = p;
				r->m_n = ( r->m_p - m_center ).normalized();

				results.push_back( r );
			}	
		}
	}
	
	assert( results.size() >= 0 );
	assert( results.size() <= 2 );	
	
	return results.size();	
}

