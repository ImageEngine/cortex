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

#include <cassert>

#include "IECoreGL/GL.h"
#include "IECoreGL/State.h"
#include "IECoreGL/SpherePrimitive.h"
#include "IECoreGL/ConePrimitive.h"
#include "IECore/PrimitiveVariable.h"
#include "IECoreGL/SkeletonPrimitive.h"

#include "OpenEXR/ImathMatrixAlgo.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( SkeletonPrimitive );


SkeletonPrimitive::SkeletonPrimitive() :
	m_parentIds( new IECore::IntVectorData ),
	m_globalMatrices( new IECore::M44fVectorData )
{
	m_jointsAxis = false;
	m_jointsRadius = 1.0;
}

SkeletonPrimitive::SkeletonPrimitive(
		IECore::ConstM44fVectorDataPtr globalMatrices, IECore::ConstIntVectorDataPtr parentIds,
		bool displayAxis, float jointsSize, const IECore::PrimitiveVariableMap &primVars)
{
	m_parentIds = parentIds->copy();
	m_globalMatrices = globalMatrices->copy();

	IECore::PrimitiveVariableMap primVarsCopy = primVars;

	m_jointsAxis = displayAxis;
	m_jointsRadius = jointsSize;

	synchVectorIds();
}

SkeletonPrimitive::~SkeletonPrimitive()
{

}

void SkeletonPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
	if ( primVar.interpolation==IECore::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.data );
	}
	if ( primVar.interpolation==IECore::PrimitiveVariable::Uniform )
	{
		addUniformAttribute( name, primVar.data );
	}
	if ( primVar.interpolation==IECore::PrimitiveVariable::Vertex )
	{
		addVertexAttribute( name, primVar.data );
	}
	if ( primVar.interpolation==IECore::PrimitiveVariable::FaceVarying )
	{
		addVertexAttribute( name, primVar.data );
	}
}

void SkeletonPrimitive::render( const State *state, IECore::TypeId style ) const
{
	Imath::V3f from_vec(0.0, 0.0, 1.0), up(0.0, 1.0, 0.0);
	JointPrimitive jointPrimitive( m_jointsRadius, 1.0 );

	// loop over global transforms
	for (unsigned int i=0; i<m_globalMatrices->readable().size(); i++)
	{
		Imath::M44f child_mtx;

		unsigned int numChildren = m_childrenIds[i].size();
		if (numChildren > 0)
		{
			for (unsigned int j=0; j< numChildren; j++ )
			{
				child_mtx = m_globalMatrices->readable()[ m_childrenIds[i][j] ];

				Imath::V3f aim_vec = child_mtx.translation() - m_globalMatrices->readable()[i].translation();
				float bone_length = aim_vec.length();
				jointPrimitive.setLength( bone_length );

				Imath::V3f up_vec = up*m_globalMatrices->readable()[i] - m_globalMatrices->readable()[i].translation();

				Imath::M44f bone_mtx = Imath::rotationMatrixWithUpDir( from_vec, aim_vec.normalize(), up_vec );
				Imath::M44f bone_offset_mtx;
				bone_offset_mtx.translate( m_globalMatrices->readable()[i].translation() );

				// draw the jointPrimitive
				glPushMatrix();
					glMultMatrixf( bone_offset_mtx.getValue() );
					glMultMatrixf( bone_mtx.getValue() );
					jointPrimitive.render( state, style );
				glPopMatrix();
			}
		}
		else
		{
			// a Null or Locator shape when the joint has no children
			glPushMatrix();
				Imath::M44f mat = m_globalMatrices->readable()[i];
				Imath::removeScaling(mat, false);
				glMultMatrixf( mat.getValue() );
				glBegin( GL_LINES );
					glVertex3f(-m_jointsRadius, 0.0, 0.0);
					glVertex3f( m_jointsRadius, 0.0, 0.0);
				glEnd();
				glBegin( GL_LINES );
					glVertex3f(0.0, -m_jointsRadius, 0.0);
					glVertex3f(0.0,  m_jointsRadius, 0.0);
				glEnd();
				glBegin( GL_LINES );
					glVertex3f(0.0, 0.0, -m_jointsRadius);
					glVertex3f(0.0, 0.0,  m_jointsRadius);
				glEnd();
			glPopMatrix();
		}

		if ( m_jointsAxis == true)
		{
			float l = m_jointsRadius*3.0;

			//// draw the axis lines for debug porpose /////
			glPushMatrix();
				Imath::M44f matNoSCale = m_globalMatrices->readable()[i];
				Imath::removeScaling(matNoSCale, false);
				glMultMatrixf( matNoSCale.getValue() );

				///// store the current color and lighting mode /////
				GLboolean light;
				float color[4];
				glGetBooleanv(GL_LIGHTING, &light);
				glGetFloatv(GL_CURRENT_COLOR, color);

				glDisable(GL_LIGHTING);
				glBegin( GL_LINES );
					glColor3ub(255, 0, 0);
					glVertex3f(0.0, 0.0, 0.0);
					glVertex3f(l, 0.0, 0.0);
				glEnd();
				glBegin( GL_LINES );
					glColor3ub(0, 255, 0);
					glVertex3f(0.0, 0.0, 0.0);
					glVertex3f(0.0, l, 0.0);
				glEnd();
				glBegin( GL_LINES );
					glColor3ub(0, 0, 255);
					glVertex3f(0.0, 0.0, 0.0);
					glVertex3f(0.0, 0.0, l);
				glEnd();

				//// restore the color and the lighting modo to their initial state /////
				glColor4f(color[0], color[1], color[2], color[3]);
				if (light==true) { glEnable(GL_LIGHTING); }
			glPopMatrix();
		}
	}
}

Imath::Box3f SkeletonPrimitive::bound() const
{
	Imath::Box3f bbox;
	for (unsigned int i=0; i<m_globalMatrices->readable().size(); i++)
	{
		bbox.extendBy( m_globalMatrices->readable()[i].translation() );
	}

	//std::cerr << bbox.min << ", " << bbox.max << std::endl;

	// add a little on for joint radius
	bbox.extendBy( bbox.max + Imath::V3f(1,1,1) );
	bbox.extendBy( bbox.min - Imath::V3f(1,1,1) );

	//std::cerr << bbox.min << ", " << bbox.max << std::endl;

	return bbox;
}

void SkeletonPrimitive::synchVectorIds()
{
	m_childrenIds.resize( m_parentIds->readable().size() );

	for (unsigned int i=0; i<m_parentIds->readable().size(); i++)
	{
		int thisParentId = m_parentIds->readable()[i];
		if ( thisParentId >= 0)
		{
			m_childrenIds[ thisParentId ].push_back( i );
		}
	}
}


