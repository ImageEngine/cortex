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

#include "IECoreGL/DiskPrimitive.h"
#include "IECoreGL/GL.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreGL;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( DiskPrimitive );

DiskPrimitive::DiskPrimitive( float radius, float z, float thetaMax )
	:	m_radius( radius ), m_z( z ), m_thetaMax( thetaMax )
{
}

DiskPrimitive::~DiskPrimitive()
{

}

void DiskPrimitive::setRadius( float radius )
{
	m_radius = radius;
}

float DiskPrimitive::getRadius() const
{
	return m_radius;
}

void DiskPrimitive::setZ( float z )
{
	m_z = z;
}

float DiskPrimitive::getZ() const
{
	return m_z;
}

void DiskPrimitive::setThetaMax( float thetaMax )
{
	m_thetaMax = thetaMax;
}

float DiskPrimitive::getThetaMax() const
{
	return m_thetaMax;
}

void DiskPrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{
	glBegin( GL_TRIANGLE_FAN );

		glNormal3f( 0, 0, 1.0f );

		glTexCoord2f( 0.5f, 0.5f );
		glVertex3f( 0.0f, 0.0f, m_z );
		const unsigned int n = 20;
		float thetaMax = m_thetaMax/180.0f * M_PI;
		for( unsigned int i=0; i<n; i++ )
		{
			float t = thetaMax * i/(n-1);
			float x = Math<float>::cos( t );
			float y = Math<float>::sin( t );
			glTexCoord2f( x/2.0f + 0.5f, y/2.0f + 0.5f );
			glVertex3f( m_radius * x, m_radius * y, m_z );
		}

	glEnd();
}

Imath::Box3f DiskPrimitive::bound() const
{
	return Imath::Box3f( V3f( -m_radius, -m_radius, 0 ), V3f( m_radius, m_radius, 0 ) );
}
