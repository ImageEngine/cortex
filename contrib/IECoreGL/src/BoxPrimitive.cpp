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

#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/GL.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( BoxPrimitive );

BoxPrimitive::BoxPrimitive( const Imath::Box3f &box )
{
	setBox( box );
}

BoxPrimitive::~BoxPrimitive()
{
}

void BoxPrimitive::setBox( const Imath::Box3f &box )
{
	m_box = box;
}

const Imath::Box3f BoxPrimitive::getBox() const
{
	return m_box;
}

void BoxPrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{
	glBegin( GL_QUADS );

		glVertex3f( m_box.min.x, m_box.min.y, m_box.min.z );
		glVertex3f( m_box.max.x, m_box.min.y, m_box.min.z );
		glVertex3f( m_box.max.x, m_box.max.y, m_box.min.z );
		glVertex3f( m_box.min.x, m_box.max.y, m_box.min.z );

		glVertex3f( m_box.max.x, m_box.min.y, m_box.min.z );
		glVertex3f( m_box.max.x, m_box.min.y, m_box.max.z );
		glVertex3f( m_box.max.x, m_box.max.y, m_box.max.z );
		glVertex3f( m_box.max.x, m_box.max.y, m_box.min.z );

		glVertex3f( m_box.max.x, m_box.min.y, m_box.max.z );
		glVertex3f( m_box.min.x, m_box.min.y, m_box.max.z );
		glVertex3f( m_box.min.x, m_box.max.y, m_box.max.z );
		glVertex3f( m_box.max.x, m_box.max.y, m_box.max.z );

		glVertex3f( m_box.min.x, m_box.min.y, m_box.max.z );
		glVertex3f( m_box.min.x, m_box.min.y, m_box.min.z );
		glVertex3f( m_box.min.x, m_box.max.y, m_box.min.z );
		glVertex3f( m_box.min.x, m_box.max.y, m_box.max.z );

		glVertex3f( m_box.min.x, m_box.max.y, m_box.min.z );
		glVertex3f( m_box.max.x, m_box.max.y, m_box.min.z );
		glVertex3f( m_box.max.x, m_box.max.y, m_box.max.z );
		glVertex3f( m_box.min.x, m_box.max.y, m_box.max.z );

		glVertex3f( m_box.min.x, m_box.min.y, m_box.min.z );
		glVertex3f( m_box.min.x, m_box.min.y, m_box.max.z );
		glVertex3f( m_box.max.x, m_box.min.y, m_box.max.z );
		glVertex3f( m_box.max.x, m_box.min.y, m_box.min.z );

	glEnd();
}

Imath::Box3f BoxPrimitive::bound() const
{
	return m_box;
}
