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

#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/GL.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

MeshPrimitive::MeshPrimitive( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, IECore::ConstV3fVectorDataPtr points )
	:	m_vertsPerFace( vertsPerFace->copy() ), m_vertIds( vertIds->copy() ), m_points( points->copy() )
{
	const vector<V3f> &p = m_points->readable();
	for( unsigned int i=0; i<p.size(); i++ )
	{
		m_bound.extendBy( p[i] );
	}
}

MeshPrimitive::~MeshPrimitive()
{

}
				
void MeshPrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{
	const vector<int> &vertsPerFace = m_vertsPerFace->readable();
	const vector<int> &vertIds = m_vertIds->readable();
	const vector<V3f> &points = m_points->readable();
	
	unsigned int vi = 0;
	for( unsigned int i=0; i<vertsPerFace.size(); i++ )
	{
		glBegin( GL_POLYGON );

			/// \todo Accept per vertex and facevarying normals
			/// as input. Write an IECore::CalculateNormalsOp to
			/// be used in IECoreGL::Renderer to calculate normals
			/// for subdiv meshes.
			const V3f &p0 = points[vertIds[vi]];
			const V3f &p1 = points[vertIds[vi+1]];
			const V3f &p2 = points[vertIds[vi+2]];
			V3f n = (p2-p1).cross(p0-p1);
			glNormal3f( n.x, n.y, n.z );
			
			for( int j=0; j<vertsPerFace[i]; j++ )
			{
				const V3f &p = points[vertIds[vi++]];
				glVertex3f( p.x, p.y, p.z );
			}
		glEnd();
	}
}

Imath::Box3f MeshPrimitive::bound() const
{
	return m_bound;
}
