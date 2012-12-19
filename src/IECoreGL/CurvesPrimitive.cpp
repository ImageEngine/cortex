//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2012, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathMatrixAlgo.h"
#include "OpenEXR/ImathVecAlgo.h"
#include "OpenEXR/ImathFun.h"

#include "IECore/MessageHandler.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreGL/CurvesPrimitive.h"
#include "IECoreGL/Buffer.h"
#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// StateComponents
//////////////////////////////////////////////////////////////////////////

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CurvesPrimitive::IgnoreBasis, CurvesPrimitiveIgnoreBasisTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CurvesPrimitive::UseGLLines, CurvesPrimitiveUseGLLinesTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( CurvesPrimitive::GLLineWidth, CurvesPrimitiveGLLineWidthTypeId, float, 1 );

} // namespace IECoreGL

//////////////////////////////////////////////////////////////////////////
// MemberData
//////////////////////////////////////////////////////////////////////////

struct CurvesPrimitive::MemberData : public IECore::RefCounted
{

	MemberData( const IECore::CubicBasisf &b, bool p, IECore::ConstIntVectorDataPtr v, float w )
		:	basis( b ), periodic( p ), vertsPerCurve( v->copy() ), width( w )
	{
	}

	Imath::Box3f bound;
	IECore::CubicBasisf basis;
	bool periodic;
	IECore::IntVectorDataPtr vertsPerCurve;
	float width;
	IECore::V3fVectorData::ConstPtr points;

	mutable IECoreGL::ConstBufferPtr vertIdsBuffer;
	mutable GLuint numVertIds;

	mutable IECoreGL::ConstBufferPtr adjacencyVertIdsBuffer;
	mutable GLuint numAdjacencyVertIds;

	struct GeometrySetup
	{
		GeometrySetup( ConstShaderPtr os, Shader::SetupPtr ss )
			:	originalShader( os ), shaderSetup( ss )
		{
		}
		ConstShaderPtr originalShader;
		Shader::SetupPtr shaderSetup;
	};

	typedef std::vector<GeometrySetup> GeometrySetupVector;
	mutable GeometrySetupVector geometrySetups;

};

//////////////////////////////////////////////////////////////////////////
// CurvesPrimitive
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( CurvesPrimitive );

CurvesPrimitive::CurvesPrimitive( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr vertsPerCurve, float width )
	:	m_memberData( new MemberData( basis, periodic, vertsPerCurve, width ) )
{
}

CurvesPrimitive::~CurvesPrimitive()
{
}

Imath::Box3f CurvesPrimitive::bound() const
{
	return m_memberData->bound;
}

void CurvesPrimitive::addPrimitiveVariable( const std::string &name, const IECore::PrimitiveVariable &primVar )
{
	if ( name == "P" )
	{
		m_memberData->points = IECore::runTimeCast< const IECore::V3fVectorData >( primVar.data->copy() );
		if ( m_memberData->points )
		{
			const vector<V3f> &pd = m_memberData->points->readable();
			for( vector<V3f>::const_iterator it=pd.begin(); it!=pd.end(); it++ )
			{
				m_memberData->bound.extendBy( *it );
			}
		}
	}
	Primitive::addPrimitiveVariable( name, primVar );
}

const Shader::Setup *CurvesPrimitive::shaderSetup( const Shader *shader, State *state ) const
{
	if( state->get<UseGLLines>()->value() )
	{
		if( m_memberData->basis==IECore::CubicBasisf::linear() || state->get<IgnoreBasis>()->value() )
		{
			return Primitive::shaderSetup( shader, state );
		}
		else
		{
			for( MemberData::GeometrySetupVector::const_iterator it = m_memberData->geometrySetups.begin(), eIt = m_memberData->geometrySetups.end(); it != eIt; it++ )
			{
				if( it->originalShader == shader )
				{
					return it->shaderSetup.get();
				}
			}
		
			ConstShaderPtr geometryShader = shader;
			if( geometryShader->geometrySource() == "" )
			{
				// if the current shader has a specific geometry shader component,
				// then we assume the user has provided one capable of doing the tesselation,
				// but if not then we substitute in our own geometry shader.
				ShaderLoader *shaderLoader = state->get<ShaderStateComponent>()->shaderLoader();
				geometryShader = shaderLoader->create( geometryShader->vertexSource(), geometrySource(), geometryShader->fragmentSource() );
			}
			
			static Shader::SetupPtr geometryShaderSetup = new Shader::Setup( geometryShader );
			addPrimitiveVariablesToShaderSetup( geometryShaderSetup );
			geometryShaderSetup->addUniformParameter( "basis", new IECore::M44fData( m_memberData->basis.matrix ) );
			
			m_memberData->geometrySetups.push_back( MemberData::GeometrySetup( shader, geometryShaderSetup ) );
			
			return geometryShaderSetup;
		}
	}
	else
	{
		/// \todo Implement ribbon rendering properly
		return Primitive::shaderSetup( shader, state );
	}
}

void CurvesPrimitive::render( const State *currentState, IECore::TypeId style ) const
{
	if( currentState->get<UseGLLines>()->value() )
	{
		glLineWidth( currentState->get<GLLineWidth>()->value() );
		if( (m_memberData->basis==IECore::CubicBasisf::linear() || currentState->get<IgnoreBasis>()->value()) )
		{
			renderInstances( 1 );
		}
		else
		{
			ensureAdjacencyVertIds();
			Buffer::ScopedBinding indexBinding( *(m_memberData->adjacencyVertIdsBuffer), GL_ELEMENT_ARRAY_BUFFER );
			glDrawElements( GL_LINES_ADJACENCY, m_memberData->numAdjacencyVertIds, GL_UNSIGNED_INT, 0 );
		}
	}
	else
	{
		//renderRibbons( currentState, style );
	}
}

void CurvesPrimitive::renderInstances( size_t numInstances ) const
{
	ensureVertIds();
	Buffer::ScopedBinding indexBinding( *(m_memberData->vertIdsBuffer), GL_ELEMENT_ARRAY_BUFFER );
	glDrawElementsInstanced( GL_LINES, m_memberData->numVertIds, GL_UNSIGNED_INT, 0, numInstances );
}

const std::string &CurvesPrimitive::geometrySource()
{
	static std::string s = 
	
		"#version 150\n"
		""
		"layout( lines_adjacency ) in;"
		"layout( line_strip, max_vertices = 10 ) out;"
		""
		"uniform mat4x4 basis;"
		""
		"void main()"
		"{"
		""
		"	for( int i = 0; i < 10; i++ )"
		"	{"
		"		float t = float( i ) / 9.0;"
		"		float t2 = t * t;"
		"		float t3 = t2 * t;"
		""
		"		float c0 = basis[0][0] * t3 + basis[1][0] * t2 + basis[2][0] * t + basis[3][0];"
		"		float c1 = basis[0][1] * t3 + basis[1][1] * t2 + basis[2][1] * t + basis[3][1];"
		"		float c2 = basis[0][2] * t3 + basis[1][2] * t2 + basis[2][2] * t + basis[3][2];"
		"		float c3 = basis[0][3] * t3 + basis[1][3] * t2 + basis[2][3] * t + basis[3][3];"
		""
		"		gl_Position ="
		""
		"			gl_in[0].gl_Position * c0 +"
		"			gl_in[1].gl_Position * c1 +"
		"			gl_in[2].gl_Position * c2 +"
		"			gl_in[3].gl_Position * c3;"
		""
		"		EmitVertex();"
		"	}"
		"}";
		
	return s;
}

void CurvesPrimitive::ensureVertIds() const
{
	if( m_memberData->vertIdsBuffer )
	{
		return;
	}
	
	IECore::UIntVectorDataPtr vertIdsData = new IECore::UIntVectorData;
	vector<unsigned int> &vertIds = vertIdsData->writable();
	
	unsigned vertIndex = 0;
	const vector<int> &vertsPerCurve = m_memberData->vertsPerCurve->readable();
	for( vector<int>::const_iterator it = vertsPerCurve.begin(), eIt = vertsPerCurve.end(); it != eIt; it++ )
	{
		const int numSegments = *it - 1;
		for( int i = 0; i < numSegments; i++ )
		{
			vertIds.push_back( vertIndex );
			vertIds.push_back( ++vertIndex );			
		}
		if( m_memberData->periodic )
		{
			vertIds.push_back( vertIndex );
			vertIds.push_back( vertIndex - numSegments );
		}
		vertIndex++;
	}
	
	m_memberData->numVertIds = vertIds.size();
	
	CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();
	m_memberData->vertIdsBuffer = IECore::runTimeCast<const Buffer>( cachedConverter->convert( vertIdsData ) );
}

void CurvesPrimitive::ensureAdjacencyVertIds() const
{
	if( m_memberData->adjacencyVertIdsBuffer )
	{
		return;
	}
	
	IECore::UIntVectorDataPtr vertIdsData = new IECore::UIntVectorData;
	vector<unsigned int> &vertIds = vertIdsData->writable();
	
	int baseIndex = 0;
	const vector<int> &vertsPerCurve = m_memberData->vertsPerCurve->readable();
	for( vector<int>::const_iterator it = vertsPerCurve.begin(), eIt = vertsPerCurve.end(); it != eIt; it++ )
	{
		int numPoints = *it;
		int numSegments = IECore::CurvesPrimitive::numSegments( m_memberData->basis, m_memberData->periodic, numPoints );
		unsigned pi = 0;
		for( int i=0; i<numSegments; i++ )
		{
			vertIds.push_back( baseIndex + ( pi % numPoints ) );
			vertIds.push_back( baseIndex + ( (pi+1) % numPoints ) );
			vertIds.push_back( baseIndex + ( (pi+2) % numPoints ) );
			vertIds.push_back( baseIndex + ( (pi+3) % numPoints ) );
			pi += m_memberData->basis.step;
		}

		baseIndex += numPoints;
	
	}
	
	m_memberData->numAdjacencyVertIds = vertIds.size();
	
	CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();
	m_memberData->adjacencyVertIdsBuffer = IECore::runTimeCast<const Buffer>( cachedConverter->convert( vertIdsData ) );
}

//void CurvesPrimitive::renderLines( const State *currentState, IECore::TypeId style ) const
//{
	/*if( m_basis==IECore::CubicBasisf::linear() || currentState->get<IgnoreBasis>()->value() )
	{
		
	}*/
	

/*	const V3f *p = &(m_points->readable()[0]);
	const std::vector<int> &v = m_vertsPerCurve->readable();

	unsigned vertexCount = m_points->readable().size();
	glLineWidth( state->get<GLLineWidth>()->value() );

	if( m_basis==IECore::CubicBasisf::linear() || state->get<IgnoreBasis>()->value() )
	{

		if ( style == Primitive::DrawSolid::staticTypeId() )
		{
			setVertexAttributes( vertexCount );
		}

		int c = 0;
		int offset = 0;
		for( std::vector<int>::const_iterator vIt = v.begin(); vIt!=v.end(); vIt++, c++ )
		{
			if( style==Primitive::DrawSolid::staticTypeId() )
			{
				setVertexAttributesAsUniforms( v.size(), c );
			}

			glDrawArrays( m_periodic ? GL_LINE_LOOP : GL_LINE_STRIP, offset, *vIt );
			offset += *vIt;
		}

	}
	else
	{

		unsigned baseIndex = 0;
		int c = 0;
		for( std::vector<int>::const_iterator vIt = v.begin(); vIt!=v.end(); vIt++, c++ )
		{

			if( style==Primitive::DrawSolid::staticTypeId() )
			{
				setVertexAttributesAsUniforms( v.size(), c );
			}

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

	}*/
//}

/*void CurvesPrimitive::renderRibbons( const State *currentState, IECore::TypeId style ) const
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

		int c = 0;
		unsigned basePointIndex = 0;
		for( std::vector<int>::const_iterator vIt = vertsPerCurve.begin(); vIt!=vertsPerCurve.end(); vIt++, c++ )
		{

			if( style==Primitive::DrawSolid::staticTypeId() )
			{
				setVertexAttributesAsUniforms( vertsPerCurve.size(), c );
			}

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

					V3f normal;
					V3f uTangent = uTangentAndNormal( p1, cameraCentre, cameraView, perspective, vBefore + vAfter, normal );

					float sinTheta = uTangent.dot( vBefore );
					float cosTheta = sqrt( 1.0f - sinTheta * sinTheta );
					uTangent *= halfWidth / cosTheta;
					glNormal( normal );
					glTexCoord2f( 0.0f, 0.0f );
					glVertex( p1 - uTangent );
					glTexCoord2f( 1.0f, 0.0f );
					glVertex( p1 + uTangent );
				}

			glEnd();

			basePointIndex += numPoints;
		}

	}
	else
	{
		int c = 0;
		unsigned basePointIndex = 0;
		for( std::vector<int>::const_iterator vIt = vertsPerCurve.begin(); vIt!=vertsPerCurve.end(); vIt++, c++ )
		{

			if( style==Primitive::DrawSolid::staticTypeId() )
			{
				setVertexAttributesAsUniforms( vertsPerCurve.size(), c );
			}

			unsigned numPoints = *vIt;
			unsigned numSegments = IECore::CurvesPrimitive::numSegments( m_basis, m_periodic, numPoints );
			unsigned pi = 0;

			glBegin( GL_QUAD_STRIP );

				V3f lastP( 0 );
				V3f lastV( 0 );
				V3f firstP( 0 );
				V3f firstV( 0 );
				V3f firstUTangent( 0 );
				V3f firstNormal( 0 );
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
									V3f normal;
									V3f uTangent = uTangentAndNormal( lastP, cameraCentre, cameraView, perspective, v, normal );
									uTangent *= halfWidth;
									glTexCoord2f( 0.0f, 0.0f );
									glVertex( lastP - uTangent );
									glTexCoord2f( 1.0f, 0.0f );
									glVertex( lastP + uTangent );
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
							V3f normal;
							V3f uTangent = uTangentAndNormal( lastP, cameraCentre, cameraView, perspective, vAvg, normal );
							uTangent *= halfWidth;

							glNormal( normal );
							glTexCoord2f( 0.0f, 0.0f );
							glVertex( lastP - uTangent );
							glTexCoord2f( 1.0f, 0.0f );
							glVertex( lastP + uTangent );

							if( i==0 && ti==2 )
							{
								// save for joining up periodic curves at the end
								firstP = lastP;
								firstUTangent = uTangent;
								firstNormal = normal;
							}

							if( lastSegment && ti==tiLimit )
							{
								// the last point of all.
								if( !m_periodic )
								{
									V3f normal;
									V3f uTangent = uTangentAndNormal( p, cameraCentre, cameraView, perspective, v, normal );
									uTangent *= halfWidth;
									glNormal( normal );
									glTexCoord2f( 0.0f, 0.0f );
									glVertex( p - uTangent );
									glTexCoord2f( 1.0f, 0.0f );
									glVertex( p + uTangent );
								}
								else
								{
									V3f vAvg = (v + firstV) / 2.0f;
									V3f normal;
									V3f uTangent = uTangentAndNormal( p, cameraCentre, cameraView, perspective, vAvg, normal );
									uTangent *= halfWidth;
									glNormal( normal );
									glTexCoord2f( 0.0f, 0.0f );
									glVertex( p - uTangent );
									glTexCoord2f( 1.0f, 0.0f );
									glVertex( p + uTangent );
									glNormal( firstNormal );
									glTexCoord2f( 0.0f, 0.0f );
									glVertex( firstP - firstUTangent );
									glTexCoord2f( 1.0f, 0.0f );
									glVertex( firstP + firstUTangent );
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
*/
