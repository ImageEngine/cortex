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

#include "IECoreGL/PointsPrimitive.h"
#include "IECoreGL/DiskPrimitive.h"
#include "IECoreGL/QuadPrimitive.h"
#include "IECoreGL/SpherePrimitive.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/State.h"
#include "IECoreGL/GL.h"

#include "IECore/MessageHandler.h"

#include "OpenEXR/ImathFun.h"
#include "OpenEXR/ImathMatrixAlgo.h"

#include <algorithm>

using namespace IECoreGL;
using namespace IECore;
using namespace Imath;
using namespace std;

float PointsPrimitive::g_defaultWidth = 1;
float PointsPrimitive::g_defaultHeight = 1;
float PointsPrimitive::g_defaultRotation = 1;

PointsPrimitive::PointsPrimitive( Type type,
	IECore::ConstV3fVectorDataPtr p,
	IECore::ConstColor3fVectorDataPtr colors,
	IECore::ConstFloatVectorDataPtr alphas,
	IECore::ConstFloatVectorDataPtr widths,
	IECore::ConstFloatVectorDataPtr heights,
	IECore::ConstFloatVectorDataPtr rotations )
	:	m_points( p->copy() ), m_type( type )
{
	m_colors = colors ? colors->copy() : 0;
	m_alphas = alphas ? alphas->copy() : 0;
	m_widths = widths ? widths->copy() : 0;
	m_heights = heights ? heights->copy() : 0;
	m_rotations = rotations ? rotations->copy() : 0;
	
	unsigned int wStep = 0;
	const float *w = dataAndStride( m_widths, &g_defaultWidth, wStep );
	unsigned int hStep = 0;
	const float *h = dataAndStride( m_heights, &g_defaultHeight, hStep );
	if( !m_heights )
	{
		h = 0;
	}
	const vector<V3f> &pd = m_points->readable();
	for( unsigned int i=0; i<pd.size(); i++ )
	{
		float r = *w; w += wStep;
		if( h )
		{
			r = max( r, *h ); h += hStep;
		}
		m_bound.extendBy( Box3f( pd[i] - V3f( r ), pd[i] + V3f( r ) ) );
	}
}

PointsPrimitive::~PointsPrimitive()
{
}
		
void PointsPrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{	
	if( depthSortRequested( state ) )
	{
		depthSort();
		m_renderSorted = true;
	}
	else
	{
		m_renderSorted = false;
	}
	switch( m_type )
	{
		case Point :
			renderPoints( state, style );
			break;
		case Disk :
			renderDisks( state, style );
			break;
		case Quad :
			renderQuads( state, style );
			break;
		case Sphere :
			renderSpheres( state, style );
	}	
}

Imath::Box3f PointsPrimitive::bound() const
{
	return m_bound;
}

size_t PointsPrimitive::vertexAttributeSize() const
{
	return m_points->readable().size();
}

const Imath::Color3f *PointsPrimitive::setOrReturnColor() const
{
	if( m_colors )
	{
		if( m_colors->readable().size()==1 )
		{
			// constant - set the color once
			const Color3f &cc = m_colors->readable()[0];
			glColor3f( cc[0], cc[1], cc[2] );
		}
		else
		{
			return &m_colors->readable()[0];
		}
	}
	return 0;
}

template<typename T>
const T *PointsPrimitive::dataAndStride( typename IECore::TypedData<std::vector<T> >::ConstPtr data, T *defaultValue, unsigned int &stride )
{
	stride = 0;
	if( !data )
	{
		stride = 0;
		return defaultValue;
	}
	if( data->readable().size()>1 )
	{
		stride = 1;
	}
	return &data->readable()[0];
}
	
void PointsPrimitive::renderPoints( ConstStatePtr state, IECore::TypeId style ) const
{
	const std::vector<V3f> &p = m_points->readable();
	
	const Color3f *c = setOrReturnColor();
	
	glBegin( GL_POINTS );
		for( unsigned int i=0; i<p.size(); i++ )
		{
			if( c )
			{
				glColor3f( (*c)[0], (*c)[1], (*c)[2] );
				c++;
			}
			glVertex3f( p[i][0], p[i][1], p[i][2] );
		}
	glEnd();
}

void PointsPrimitive::renderDisks( ConstStatePtr state, IECore::TypeId style ) const
{
	DiskPrimitivePtr disk = new DiskPrimitive();
	
	V3f cameraCentre = Camera::positionInObjectSpace();
	V3f cameraUp = Camera::upInObjectSpace();
	
	const std::vector<V3f> &p = m_points->readable();
	
	const Color3f *c = setOrReturnColor();
	
	unsigned int wStep = 0;
	const float *w = dataAndStride( m_widths, &g_defaultWidth, wStep );
	if( !wStep )
	{
		disk->setRadius( *w / 2.0f );
		w = 0;
	}

	for( unsigned int j=0; j<p.size(); j++ )
	{
		unsigned int i = m_renderSorted ? m_depthOrder[j] : j;

		if( style==PrimitiveSolid::staticTypeId() )
		{
			setVertexAttributesAsUniforms( i );
			if( c )
			{
				glColor3f( c[i][0], c[i][1], c[i][2] );
			}
		}
		
		M44f aim = alignZAxisWithTargetDir( cameraCentre - p[i], cameraUp );
		glPushMatrix();
			glTranslatef( p[i][0], p[i][1], p[i][2] );
			glMultMatrixf( aim.getValue() );
			if( w )
			{
				disk->setRadius( w[i] / 2.0f );
			}
			disk->render( state, style );
		glPopMatrix();
	}
	
}

void PointsPrimitive::renderQuads( ConstStatePtr state, IECore::TypeId style ) const
{
	QuadPrimitivePtr quad = new QuadPrimitive();
	V3f cameraCentre = Camera::positionInObjectSpace();
	V3f cameraUp = Camera::upInObjectSpace();
	
	const std::vector<V3f> &p = m_points->readable();
	
	const Color3f *c = setOrReturnColor();

	unsigned int wStep = 0;
	const float *w = dataAndStride( m_widths, &g_defaultWidth, wStep );
	if( !wStep )
	{
		quad->setWidth( *w );
		w = 0;
	}
	
	unsigned int hStep = 0;
	const float *h = dataAndStride( m_heights, &g_defaultHeight, hStep );
	if( !hStep )
	{
		quad->setHeight( *h );
		h = 0;
	}
	
	unsigned int rStep = 0;
	const float *r = dataAndStride( m_rotations, &g_defaultRotation, rStep );
	
	for( unsigned int j=0; j<p.size(); j++ )
	{
		unsigned int i = m_renderSorted ? m_depthOrder[j] : j;
	
		if( style==PrimitiveSolid::staticTypeId() )
		{
			setVertexAttributesAsUniforms( i );
			if( c )
			{
				glColor3f( c[i][0], c[i][1], c[i][2] );
			}
		}

		M44f aim = alignZAxisWithTargetDir( cameraCentre - p[i], cameraUp );

		glPushMatrix();
			glTranslatef( p[i][0], p[i][1], p[i][2] );
			glMultMatrixf( aim.getValue() );
			glRotatef( -r[i*rStep], 0, 0, 1 );
			if( w )
			{
				quad->setWidth( w[i] );
			}
			if( h )
			{
				quad->setHeight( h[i] );
			}
			quad->render( state, style );
		glPopMatrix();
	}
	
}

void PointsPrimitive::renderSpheres( ConstStatePtr state, IECore::TypeId style ) const
{
	SpherePrimitivePtr sphere = new SpherePrimitive();
	const std::vector<V3f> &p = m_points->readable();
	
	const Color3f *c = setOrReturnColor();
	
	unsigned int wStep = 0;
	const float *w = dataAndStride( m_widths, &g_defaultWidth, wStep );
	if( !wStep )
	{
		sphere->setRadius( *w / 2.0f );
		w = 0;
	}

	for( unsigned int j=0; j<p.size(); j++ )
	{
		unsigned int i = m_renderSorted ? m_depthOrder[j] : j;
		
		if( style==PrimitiveSolid::staticTypeId() )
		{
			setVertexAttributesAsUniforms( i );
			if( c )
			{
				glColor3f( c[i][0], c[i][1], c[i][2] );
			}
		}

		glPushMatrix();
			glTranslatef( p[i][0], p[i][1], p[i][2] );
			if( w )
			{
				sphere->setRadius( w[i] / 2.0f );
			}
			sphere->render( state, style );
		glPopMatrix();
	}
	
}

struct SortFn
{
	SortFn( const vector<float> &d ) : depths( d ) {};
	bool operator()( int f, int s ) { return depths[f] > depths[s]; };
	const vector<float> &depths;
};

/// \todo Stick a radix sort in IECore and use that.
void PointsPrimitive::depthSort() const
{
	V3f cameraDirection = Camera::viewDirectionInObjectSpace();
	cameraDirection.normalize();

	const vector<V3f> &points = m_points->readable();
	if( !m_depthOrder.size() )
	{
		// never sorted before. initialize space.
		m_depthOrder.resize( points.size() );
		for( unsigned int i=0; i<m_depthOrder.size(); i++ )
		{
			m_depthOrder[i] = i;
		}
		m_depths.resize( points.size() );
	}
	else
	{
		// sorted before. see if the camera direction has changed enough
		// to warrant resorting.
		if( cameraDirection.dot( m_depthCameraDirection ) > 0.95 )
		{
			return;
		}
	}

	m_depthCameraDirection = cameraDirection;
	
	// calculate all distances
	for( unsigned int i=0; i<m_depths.size(); i++ )
	{
		m_depths[i] = points[i].dot( m_depthCameraDirection );
	}
	
	// sort based on those distances
	SortFn sorter( m_depths );
	sort( m_depthOrder.begin(), m_depthOrder.end(), sorter );
}
