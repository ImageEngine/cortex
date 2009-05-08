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

#include <cassert>

#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/State.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshPrimitive );

MeshPrimitive::MeshPrimitive( IECore::ConstIntVectorDataPtr vertIds, IECore::ConstV3fVectorDataPtr points )
	:	m_vertIds( vertIds->copy() )
{
	assert( points );

	m_points = points->copy();
	assert( m_points );

	const vector<V3f> &p = m_points->readable();
	for( unsigned int i=0; i<p.size(); i++ )
	{
		m_bound.extendBy( p[i] );
	}
}

MeshPrimitive::~MeshPrimitive()
{

}

IECore::ConstIntVectorDataPtr MeshPrimitive::vertexIds() const
{
	return m_vertIds;
}

size_t MeshPrimitive::vertexAttributeSize() const
{
	return m_vertIds->readable().size();
}

void MeshPrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{
	assert( m_points );

	setVertexAttributes( state );

	const vector<V3f> &points = m_points->readable();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer( 3, GL_FLOAT, 0, &points[0] );

	if ( m_normals )
	{
		const vector<V3f> &normals = m_normals->readable();
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer( GL_FLOAT, 0, &normals[0] );
	}

	if ( m_texCoords )
	{
		const vector<V2f> &texCoords = m_texCoords->readable();
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &texCoords[0] );
	}

	if ( m_colors )
	{
		const vector<Color3f> &colors = m_colors->readable();
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 0, &colors[0] );
	}

	glDrawArrays( GL_TRIANGLES, 0, vertexAttributeSize() );

	glDisableClientState(GL_VERTEX_ARRAY);
	if ( m_normals )
	{
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	if ( m_texCoords )
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if ( m_colors )
	{
		glDisableClientState(GL_COLOR_ARRAY);
	}
}

Imath::Box3f MeshPrimitive::bound() const
{
	return m_bound;
}
