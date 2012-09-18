//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "OpenEXR/ImathFun.h"
#include "OpenEXR/ImathMatrixAlgo.h"

#include <algorithm>

using namespace IECoreGL;
using namespace IECore;
using namespace Imath;
using namespace std;

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointsPrimitive::UseGLPoints, PointsPrimitiveUseGLPointsTypeId, GLPointsUsage, ForPointsOnly );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( PointsPrimitive::GLPointWidth, PointsPrimitiveGLPointWidthTypeId, float, 1.0f );

}

IE_CORE_DEFINERUNTIMETYPED( PointsPrimitive );

float PointsPrimitive::g_defaultWidth = 1;
float PointsPrimitive::g_defaultHeight = 1;
float PointsPrimitive::g_defaultRotation = 0;

PointsPrimitive::PointsPrimitive( Type type )
	:	m_type( type ), m_recomputeBound( true )
{
}

PointsPrimitive::~PointsPrimitive()
{
}

void PointsPrimitive::updateBounds() const
{
	if ( !m_recomputeBound )
		return;

	m_recomputeBound = false;

	unsigned int wStep = 0;
	const float *w = dataAndStride( m_widths, &g_defaultWidth, wStep );
	unsigned int hStep = 0;
	const float *h = dataAndStride( m_heights, &g_defaultHeight, hStep );
	if( !m_heights )
	{
		h = 0;
	}
	m_bound.makeEmpty();
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

void PointsPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
	if ( name == "P" )
	{
		m_recomputeBound = true;
		m_points = IECore::runTimeCast< IECore::V3fVectorData >( primVar.data->copy() );
	}
	else if ( name == "width" )
	{
		m_recomputeBound = true;
		m_widths = primVar.data->copy();
	}
	else if ( name == "height" )
	{
		m_recomputeBound = true;
		m_heights = primVar.data->copy();
	}
	else if ( name == "patchrotation" )
	{
		m_rotations = primVar.data->copy();
	}
	Primitive::addPrimitiveVariable( name, primVar );
}

void PointsPrimitive::render( const State * state, IECore::TypeId style ) const
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

	Type type = m_type;
	switch( state->get<UseGLPoints>()->value() )
	{
		case ForPointsOnly :
			break;
		case ForPointsAndDisks :
			if( type==Disk )
			{
				type = Point;
			}
			break;
		case ForAll :
			type = Point;
			break;
	}

	switch( type )
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
	updateBounds();
	return m_bound;
}

template<typename T>
const T *PointsPrimitive::dataAndStride( const IECore::Data *data, T *defaultValue, unsigned int &stride )
{
	stride = 0;
	if( !data )
	{
		stride = 0;
		return defaultValue;
	}
	IECore::TypeId t = data->typeId();
	if ( t == IECore::TypedData< T >::staticTypeId() )
	{
		return &(static_cast< const IECore::TypedData< T > * >( data )->readable());
	}
	if ( t == IECore::TypedData< std::vector< T > >::staticTypeId() )
	{
		stride = 1;
		return &(static_cast< const IECore::TypedData< std::vector<T> > * >( data )->readable()[0]);
	}
	return defaultValue;
}

void PointsPrimitive::renderPoints( const State * state, IECore::TypeId style ) const
{
	const std::vector<V3f> &p = m_points->readable();

	glPointSize( state->get<GLPointWidth>()->value() );

	if( style==Primitive::DrawSolid::staticTypeId() )
	{
		setVertexAttributes( p.size() );
	}
	glDrawArrays( GL_POINTS, 0, p.size() );
}

void PointsPrimitive::renderDisks( const State * state, IECore::TypeId style ) const
{
	DiskPrimitivePtr disk = new DiskPrimitive();

	V3f cameraCentre = Camera::positionInObjectSpace();
	V3f cameraUp = Camera::upInObjectSpace();
	V3f cameraView = -Camera::viewDirectionInObjectSpace();
	bool perspective = Camera::perspectiveProjection();
	
	const std::vector<V3f> &p = m_points->readable();

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

		if( style==Primitive::DrawSolid::staticTypeId() )
		{
			setVertexAttributesAsUniforms( p.size(), i );
		}

		M44f aim = alignZAxisWithTargetDir( perspective ? cameraCentre - p[i] : cameraView, cameraUp );
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

void PointsPrimitive::renderQuads( const State * state, IECore::TypeId style ) const
{
	QuadPrimitivePtr quad = new QuadPrimitive();
	
	V3f cameraCentre = Camera::positionInObjectSpace();
	V3f cameraUp = Camera::upInObjectSpace();
	V3f cameraView = -Camera::viewDirectionInObjectSpace();
	bool perspective = Camera::perspectiveProjection();

	const std::vector<V3f> &p = m_points->readable();

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

		if( style==Primitive::DrawSolid::staticTypeId() )
		{
			setVertexAttributesAsUniforms( p.size(), i );
		}

		M44f aim = alignZAxisWithTargetDir( perspective ? cameraCentre - p[i] : cameraView, cameraUp );

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

void PointsPrimitive::renderSpheres( const State * state, IECore::TypeId style ) const
{
	SpherePrimitivePtr sphere = new SpherePrimitive();
	const std::vector<V3f> &p = m_points->readable();

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

		if( style==Primitive::DrawSolid::staticTypeId() )
		{
			setVertexAttributesAsUniforms( p.size(), i );
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

/// \todo Use IECore::RadixSort (might still want to use std::sort for small numbers of points - profile to check this)
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
