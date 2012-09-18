//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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
#include "IECoreGL/JointPrimitive.h"
#include "IECoreGL/GL.h"

#include "OpenEXR/ImathMath.h"
#include "OpenEXR/ImathFun.h"

#include <algorithm>

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( JointPrimitive );

JointPrimitive::JointPrimitive( float radius, float length )
{
	float h = radius*0.75;
	p0.setValue(0.0, 0.0, 0.0);
	p1.setValue( radius,  radius, h);
	p2.setValue(-radius,  radius, h);
	p3.setValue(-radius, -radius, h);
	p4.setValue( radius, -radius, h);
	p5.setValue(0.0f, 0.0f, length);
}

JointPrimitive::~JointPrimitive()
{}

void JointPrimitive::setRadius( float radius )
{
	float h = radius*0.75;
	p1.setValue( radius,  radius, h);
	p2.setValue(-radius,  radius, h);
	p3.setValue(-radius, -radius, h);
	p4.setValue( radius, -radius, h);
}

float JointPrimitive::getRadius() const
{
	return p1.x;
}

void JointPrimitive::setLength( float length )
{
	p5.z = length;
}

float JointPrimitive::getLength() const
{
	return p5.z;
}

void getNormal(const Imath::V3f &vec1, const Imath::V3f &vec2, const Imath::V3f &vec3, Imath::V3f &retNormal)
{
	retNormal = ( vec2-vec1 ).cross( vec3-vec1 );
	retNormal .normalize();
}

void JointPrimitive::render( const State *state, IECore::TypeId style ) const
{
	Imath::V3f n;
	glBegin( GL_TRIANGLES );
		///// The top small pyramid /////
		getNormal(p0, p4, p3, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p4.x, p4.y, p4.z);
		glVertex3f(p3.x, p3.y, p3.z);

		getNormal(p0, p3, p2, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p3.x, p3.y, p3.z);
		glVertex3f(p2.x, p2.y, p2.z);

		getNormal(p0, p2, p1, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p1.x, p1.y, p1.z);

		getNormal(p0, p1, p4, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p0.x, p0.y, p0.z);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p4.x, p4.y, p4.z);

		///// The body long pyramid /////
		getNormal(p5, p1, p2, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);

		getNormal(p5, p2, p3, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glVertex3f(p3.x, p3.y, p3.z);

		getNormal(p5, p3, p4, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glVertex3f(p3.x, p3.y, p3.z);
		glVertex3f(p4.x, p4.y, p4.z);

		getNormal(p5, p4, p1, n);
		glNormal3f(n.x, n.y, n.z);
		glVertex3f(p5.x, p5.y, p5.z);
		glVertex3f(p4.x, p4.y, p4.z);
		glVertex3f(p1.x, p1.y, p1.z);
	glEnd();
}

Imath::Box3f JointPrimitive::bound() const
{
	return Imath::Box3f( Imath::V3f( p1.x, 0.0f, p1.x ), Imath::V3f( -p1.x, p5.z, -p1.x ) );
}


void JointPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
	if ( primVar.interpolation==IECore::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.data );
	}
	if ( primVar.interpolation==IECore::PrimitiveVariable::Uniform )
	{
		addUniformAttribute( name, primVar.data );
	}
}

