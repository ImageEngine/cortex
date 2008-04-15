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

#include "IECoreGL/CurvesPrimitive.h"
#include "IECoreGL/Camera.h"

#include "IECore/MessageHandler.h"
#include "IECore/CurvesPrimitive.h"

#include "OpenEXR/ImathMatrixAlgo.h"
#include "OpenEXR/ImathFun.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CurvesPrimitive::IgnoreBasis, CurvesPrimitiveIgnoreBasisTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CurvesPrimitive::UseGLLines, CurvesPrimitiveUseGLLinesTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CurvesPrimitive::GLLineWidth, CurvesPrimitiveGLLineWidthTypeId, float, 1 );

}

CurvesPrimitive::CurvesPrimitive( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr vertsPerCurve, IECore::ConstV3fVectorDataPtr points, float width )
	:	m_basis( basis ), m_periodic( periodic ), m_vertsPerCurve( vertsPerCurve->copy() ), m_points( points->copy() ), m_width( width )
{
	const vector<V3f> &pd = m_points->readable();
	for( vector<V3f>::const_iterator it=pd.begin(); it!=pd.end(); it++ )
	{
		m_bound.extendBy( *it );
	}
}

CurvesPrimitive::~CurvesPrimitive()
{
}

Imath::Box3f CurvesPrimitive::bound() const
{
	return m_bound;
}

size_t CurvesPrimitive::vertexAttributeSize() const
{
	return m_points->readable().size();
}

void CurvesPrimitive::render( ConstStatePtr state, IECore::TypeId style ) const
{
	if( state->get<UseGLLines>()->value() )
	{
		renderLines( state, style );
	}
	else
	{
		renderRibbons( state, style );
	}	
}

void CurvesPrimitive::renderLines( ConstStatePtr state, IECore::TypeId style ) const
{
	const V3f *p = &(m_points->readable()[0]);
	const std::vector<int> &v = m_vertsPerCurve->readable();
	
	glLineWidth( state->get<GLLineWidth>()->value() );
	
	if( m_basis==IECore::CubicBasisf::linear() || state->get<IgnoreBasis>()->value() )
	{
		
		glEnableClientState( GL_VERTEX_ARRAY );

			for( std::vector<int>::const_iterator vIt = v.begin(); vIt!=v.end(); vIt++ )
			{
				glVertexPointer( 3, GL_FLOAT, 0, p );
				glDrawArrays( m_periodic ? GL_LINE_LOOP : GL_LINE_STRIP, 0, *vIt );
				p += *vIt;
			}

		glDisableClientState( GL_VERTEX_ARRAY );

	}
	else
	{
	
		unsigned baseIndex = 0;
		for( std::vector<int>::const_iterator vIt = v.begin(); vIt!=v.end(); vIt++ )
		{
		
			unsigned numPoints = *vIt;
			unsigned numSegments = IECore::CurvesPrimitive::numSegments( m_basis, m_periodic, numPoints );
			unsigned pi = 0;
			for( unsigned i=0; i<numSegments; i++ )
			{
				const V3f &p0 = p[baseIndex+(pi%numPoints)];
				const V3f &p1 = p[baseIndex+((pi+1)%numPoints)];
				const V3f &p2 = p[baseIndex+((pi+2)%numPoints)];
				const V3f &p3 = p[baseIndex+((pi+3)%numPoints)];
				
				glBegin( GL_LINE_STRIP );
				
					unsigned steps = 10; /// \todo Calculate this based on projected size on the screen
					for( unsigned ti=0; ti<=steps; ti++ )
					{
						float t = (float)ti/(float)steps;
						V3f pp = m_basis( t, p0, p1, p2, p3 );
						glVertex3f( pp[0], pp[1], pp[2] );
					}
					
				glEnd();
				pi += m_basis.step;
			}
			
			baseIndex += numPoints;
		
		}
		
	}
}

static inline V3f toCamera( const V3f &p, const V3f &cameraCentre, const V3f &cameraView, bool perspective )
{
	return perspective ? cameraCentre - p : cameraView;
}

void CurvesPrimitive::renderRibbons( ConstStatePtr state, IECore::TypeId style ) const
{
	float halfWidth = m_width/2.0f;
	const V3f *points = &(m_points->readable()[0]);
	const std::vector<int> &vertsPerCurve = m_vertsPerCurve->readable();
	
	V3f cameraCentre = Camera::positionInObjectSpace();
	V3f cameraView = -Camera::viewDirectionInObjectSpace();
	bool perspective = Camera::perspectiveProjection();	
	
	if( m_basis==IECore::CubicBasisf::linear() || state->get<IgnoreBasis>()->value() )
	{
		
		// linear. implemented separately because for this we don't have to worry about
		// subdividing etc.
		
		unsigned basePointIndex = 0;
		for( std::vector<int>::const_iterator vIt = vertsPerCurve.begin(); vIt!=vertsPerCurve.end(); vIt++ )
		{
			unsigned numPoints = *vIt;
			unsigned numSegments = IECore::CurvesPrimitive::numSegments( m_basis, m_periodic, numPoints );
			
			glBegin( GL_QUAD_STRIP );

				for( unsigned i=0; i<=numSegments; i++ )
				{
					int pi0 = i-1;
					int pi1 = i;
					int pi2 = i+1;
					if( m_periodic )
					{
						pi0 = pi0 % numPoints;
						pi1 = pi1 % numPoints;
						pi2 = pi2 % numPoints;
					}
					else
					{
						pi0 = clamp( pi0, 0, (int)numPoints-1 );
						pi1 = clamp( pi1, 0, (int)numPoints-1 );
						pi2 = clamp( pi2, 0, (int)numPoints-1 );
					}
				
					const V3f &p0 = points[basePointIndex+pi0];
					const V3f &p1 = points[basePointIndex+pi1];
					const V3f &p2 = points[basePointIndex+pi2];
					
					V3f vBefore = (p1-p0).normalized();
					V3f vAfter = (p2-p1).normalized();
					
					M44f aim = Imath::alignZAxisWithTargetDir( toCamera( p1, cameraCentre, cameraView, perspective ), vBefore + vAfter );
					V3f o( 1.0f, 0, 0 ); aim.multDirMatrix( o, o );
					
					float sinTheta = o.dot( vBefore );
					float cosTheta = sqrt( 1.0f - sinTheta * sinTheta );					
					o *= halfWidth / cosTheta;
					
					glVertex( p1 - o );
					glVertex( p1 + o );
				}

			glEnd();
			
			basePointIndex += numPoints;
		}
			
	}
	else
	{
	
		unsigned basePointIndex = 0;
		for( std::vector<int>::const_iterator vIt = vertsPerCurve.begin(); vIt!=vertsPerCurve.end(); vIt++ )
		{
		
			unsigned numPoints = *vIt;
			unsigned numSegments = IECore::CurvesPrimitive::numSegments( m_basis, m_periodic, numPoints );
			unsigned pi = 0;
			
			glBegin( GL_QUAD_STRIP );
							
				V3f lastP( 0 );
				V3f lastV( 0 );
				V3f firstP( 0 );
				V3f firstV( 0 );
				V3f firstO( 0 );
				for( unsigned i=0; i<numSegments; i++ )
				{
					const V3f &p0 = points[basePointIndex+(pi%numPoints)];
					const V3f &p1 = points[basePointIndex+((pi+1)%numPoints)];
					const V3f &p2 = points[basePointIndex+((pi+2)%numPoints)];
					const V3f &p3 = points[basePointIndex+((pi+3)%numPoints)];
			
					unsigned steps = 10; /// \todo Calculate this based on projected size on the screen
					bool lastSegment = i==(numSegments-1);
					unsigned tiLimit = steps - 1;
					if( lastSegment )
					{
						// we only evaluate the last point for the last segment, as otherwise
						// it would be repeated by the first point of the next segment.
						tiLimit = steps;
					}
					for( unsigned ti=0; ti<=tiLimit; ti++ )
					{

						float t = (float)ti/(float)steps;
						V3f p = m_basis( t, p0, p1, p2, p3 );
						V3f v = p - lastP;

						if( i==0 && ti<2 )
						{
							// we're doing the awkward first few points.
							if( ti==0 )
							{
								// first point of all, we don't have enough information to
								// do anything yet.
							}
							else if( ti==1 )
							{
								// second point ever. we've got enough to emit the first point,
								// but only if we're not periodic.
								if( !m_periodic )
								{
									M44f aim = Imath::alignZAxisWithTargetDir( toCamera( lastP, cameraCentre, cameraView, perspective ), v );
									V3f o( halfWidth, 0, 0 ); aim.multDirMatrix( o, o );
									glVertex( lastP - o );
									glVertex( lastP + o );
								}
								else
								{
									// save for use with the last point of all.
									firstV = v;
								}
							}
						}
						else
						{
						
							V3f vAvg = (v + lastV) / 2.0f;
							M44f aim = Imath::alignZAxisWithTargetDir( toCamera( lastP, cameraCentre, cameraView, perspective ), vAvg );

							V3f o( halfWidth, 0, 0 ); aim.multDirMatrix( o, o );

							glVertex( lastP - o );
							glVertex( lastP + o );
							
							if( i==0 && ti==2 )
							{
								// save for joining up periodic curves at the end
								firstP = lastP;
								firstO = o;
							}
							
							if( lastSegment && ti==tiLimit )
							{
								// the last point of all.
								if( !m_periodic )
								{
									M44f aim = Imath::alignZAxisWithTargetDir( toCamera( p, cameraCentre, cameraView, perspective ), v );
									V3f o( halfWidth, 0, 0 ); aim.multDirMatrix( o, o );
									glVertex( p - o );
									glVertex( p + o );
								}
								else
								{
									V3f vAvg = (v + firstV) / 2.0f;
									M44f aim = Imath::alignZAxisWithTargetDir( toCamera( p, cameraCentre, cameraView, perspective ), vAvg );
									V3f o( halfWidth, 0, 0 ); aim.multDirMatrix( o, o );
									glVertex( p - o );
									glVertex( p + o );
									glVertex( firstP - firstO );
									glVertex( firstP + firstO );
								}
							}
							
						}
						
						lastP = p;
						lastV = v;
					}

					pi += m_basis.step;
				}
			
			glEnd();
			basePointIndex += numPoints;
		
		}
		
	}

}
