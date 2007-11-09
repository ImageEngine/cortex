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

#include "IECoreGL/SpherePrimitive.h"
#include "IECoreGL/GL.h"

#include "OpenEXR/ImathMath.h"
#include "OpenEXR/ImathFun.h"

#include <algorithm>

using namespace IECoreGL;
using namespace Imath;
using namespace std;

SpherePrimitive::SpherePrimitive( float radius, float zMin, float zMax, float thetaMax )
{
	setRadius( radius );
	setZMin( zMin );
	setZMax( zMax );
	setThetaMax( thetaMax );
}

SpherePrimitive::~SpherePrimitive()
{

}

void SpherePrimitive::setRadius( float radius )
{
	m_radius = radius;
}

float SpherePrimitive::getRadius() const
{
	return m_radius;
}

void SpherePrimitive::setZMin( float zMin )
{
	m_zMin = zMin;
}

float SpherePrimitive::getZMin() const
{
	return m_zMin;
}

void SpherePrimitive::setZMax( float zMax )
{
	m_zMax = zMax;
}

float SpherePrimitive::getZMax() const
{
	return m_zMax;
}

void SpherePrimitive::setThetaMax( float thetaMax )
{
	m_thetaMax = thetaMax;
}

float SpherePrimitive::getThetaMax() const
{
	return m_thetaMax;
}
				
void SpherePrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{
	float oMin = Math<float>::asin( m_zMin );
	float oMax = Math<float>::asin( m_zMax );
	const unsigned int nO = max( 4u, (unsigned int)( 20.0f * m_radius * (oMax - oMin) / M_PI ) );
	
	float thetaMax = m_thetaMax/180.0f * M_PI;
	const unsigned int nT = max( 7u, (unsigned int)( m_radius * 40.0f * thetaMax / (M_PI*2) ) );

	for( unsigned int i=0; i<nO-1; i++ )
	{
		float v0 = (float)i/(float)(nO-1);
		float v1 = (float)(i+1)/(float)(nO-1);
		float o0 = lerp( oMin, oMax, v0 );
		float o1 = lerp( oMin, oMax, v1 );
		float z0 = m_radius * Math<float>::sin( o0 );
		float z1 = m_radius * Math<float>::sin( o1 );
		float r0 = m_radius * Math<float>::cos( o0 );
		float r1 = m_radius * Math<float>::cos( o1 );
		glBegin( GL_TRIANGLE_STRIP );
			for( unsigned int j=0; j<nT; j++ )
			{
				float u = (float)j/(float)(nT-1);
				float t = thetaMax * u;
				float st = Math<float>::sin( t );
				float ct = Math<float>::cos( t );
				V3f p0( r0 * ct, r0 * st, z0 );
				V3f p1( r1 * ct, r1 * st, z1 );
				glTexCoord2f( u, v1 );
				glNormal3f( p1.x, p1.y, p1.z );
				glVertex3f( p1.x, p1.y, p1.z );
				glTexCoord2f( u, v0 );
				glNormal3f( p0.x, p0.y, p0.z );
				glVertex3f( p0.x, p0.y, p0.z );
			}
		glEnd();
	}

}

Imath::Box3f SpherePrimitive::bound() const
{
	float thetaMax = m_thetaMax/180.0f * M_PI;
	float minX = m_radius * ( thetaMax < M_PI ? Math<float>::cos( thetaMax ) : -1.0f );
	float maxY = m_radius * ( thetaMax < M_PI/2 ? Math<float>::sin( thetaMax ) : 1.0f );
	float minY = m_radius * ( thetaMax > 3 * M_PI/2 ? -1.0f : min( 0.0f, Math<float>::sin( thetaMax ) ) );

	return Imath::Box3f( V3f( minX, minY, m_zMin * m_radius ), V3f( m_radius, maxY, m_zMax * m_radius ) );
}
