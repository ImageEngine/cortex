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

#include "IECoreGL/CurvesPrimitive.h"

#include "IECoreGL/Buffer.h"
#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/IECoreGL.h"

#include "IECoreScene/CurvesPrimitive.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "Imath/ImathMatrixAlgo.h"
#include "Imath/ImathVecAlgo.h"
#include "Imath/ImathFun.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// GLSL source
//////////////////////////////////////////////////////////////////////////

namespace
{

const string g_cubicLinesGeometrySource = R"(

#version 150 compatibility
#include "IECoreGL/CurvesPrimitive.h"

IECOREGL_CURVESPRIMITIVE_DECLARE_CUBIC_LINES_PARAMETERS

void main()
{
	IECOREGL_CURVESPRIMITIVE_SELECT_BASIS;

	for( int i = 0; i < 10; i++ )
	{
		float t = float( i ) / 9.0;
		vec4 coeffs = IECOREGL_CURVESPRIMITIVE_COEFFICIENTS( t );
		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH_CUBIC( coeffs );
		gl_Position = IECOREGL_CURVESPRIMITIVE_POSITION( coeffs );
		EmitVertex();
	}
}

)";

const string g_cubicRibbonsGeometrySource = R"(

#version 150 compatibility
#include "IECoreGL/CurvesPrimitive.h"

IECOREGL_CURVESPRIMITIVE_DECLARE_CUBIC_RIBBONS_PARAMETERS

out vec3 fragmentI;
out vec3 fragmentN;
out vec2 fragmentuv;

void main()
{
	IECOREGL_CURVESPRIMITIVE_SELECT_BASIS;

	for( int i = 0; i < 10; i++ )
	{

		float t = float( i ) / 9.0;
		vec4 coeffs = IECOREGL_CURVESPRIMITIVE_COEFFICIENTS( t );
		vec4 derivCoeffs = IECOREGL_CURVESPRIMITIVE_DERIVATIVE_COEFFICIENTS( t );
		vec4 p, n, uTangent, vTangent;
		IECOREGL_CURVESPRIMITIVE_CUBICFRAME( coeffs, derivCoeffs, p, n, uTangent, vTangent );

		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH_CUBIC( coeffs )
		fragmentN = n.xyz;
		fragmentI = -n.xyz;
		fragmentuv = vec2( 0, t );
		gl_Position = p + width * uTangent;
		EmitVertex();

		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH_CUBIC( coeffs )
		fragmentN = n.xyz;
		fragmentI = -n.xyz;
		fragmentuv = vec2( 1, t );
		gl_Position = p - width * uTangent;
		EmitVertex();

	}
}

)";

const string g_linearRibbonsGeometrySource = R"(

#version 150 compatibility

#include "IECoreGL/CurvesPrimitive.h"

IECOREGL_CURVESPRIMITIVE_DECLARE_LINEAR_RIBBONS_PARAMETERS

out vec3 fragmentI;
out vec3 fragmentN;

void main()
{

	for( int i = 0; i < 2; i++ )
	{

		vec4 p, n, uTangent, vTangent;
		IECOREGL_CURVESPRIMITIVE_LINEARFRAME( i, p, n, uTangent, vTangent );

		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH_LINEAR( i )
		fragmentN = n.xyz;
		fragmentI = -n.xyz;
		gl_Position = p + width * uTangent;
		EmitVertex();

		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH_LINEAR( i )
		fragmentN = n.xyz;
		fragmentI = -n.xyz;
		gl_Position = p - width * uTangent;
		EmitVertex();

	}
}

)";

} // namespace

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

class CurvesPrimitive::MemberData : public IECore::RefCounted
{

	public :

		MemberData( const IECore::CubicBasisf &b, IECoreScene::CurvesPrimitive::Wrap wrap, IECore::ConstIntVectorDataPtr v, float w )
			:	basis( b ), wrap( wrap ), vertsPerCurve( v->copy() ), width( w )
		{
			if( wrap == IECoreScene::CurvesPrimitive::Wrap::Pinned )
			{
				if( basis != IECore::CubicBasisf::catmullRom() && basis != IECore::CubicBasisf::bSpline() )
				{
					// Pinning doesn't apply to this basis.
					wrap = IECoreScene::CurvesPrimitive::Wrap::NonPeriodic;
				}
			}
		}

		Imath::Box3f bound;
		IECore::CubicBasisf basis;
		IECoreScene::CurvesPrimitive::Wrap wrap;
		IECore::IntVectorDataPtr vertsPerCurve;
		float width;
		IECore::V3fVectorData::ConstPtr points;

		mutable IECoreGL::ConstBufferPtr vertIdsBuffer;
		mutable GLuint numVertIds;

		mutable IECoreGL::ConstBufferPtr adjacencyVertIdsBuffer;
		mutable GLuint numAdjacencyVertIds;

		mutable IECoreGL::ConstBufferPtr linearAdjacencyVertIdsBuffer;
		mutable GLuint numLinearAdjacencyVertIds;

		struct GeometrySetup
		{
			GeometrySetup( ConstShaderPtr os, Shader::SetupPtr ss, bool l, bool r )
				:	originalShader( os ), shaderSetup( ss ), linear( l ), ribbons( r )
			{
			}
			ConstShaderPtr originalShader;
			Shader::SetupPtr shaderSetup;
			bool linear;
			bool ribbons;
		};

		typedef std::vector<GeometrySetup> GeometrySetupVector;
		mutable GeometrySetupVector geometrySetups;

};

//////////////////////////////////////////////////////////////////////////
// CurvesPrimitive
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( CurvesPrimitive );

CurvesPrimitive::CurvesPrimitive( const IECore::CubicBasisf &basis, IECoreScene::CurvesPrimitive::Wrap wrap, IECore::ConstIntVectorDataPtr vertsPerCurve, float width )
	:	m_memberData( new MemberData( basis, wrap, vertsPerCurve, width ) )
{
}

CurvesPrimitive::CurvesPrimitive( const IECore::CubicBasisf &basis, bool periodic, IECore::ConstIntVectorDataPtr vertsPerCurve, float width )
	:	CurvesPrimitive( basis, periodic ? IECoreScene::CurvesPrimitive::Wrap::Periodic : IECoreScene::CurvesPrimitive::Wrap::NonPeriodic, vertsPerCurve, width )
{
}

CurvesPrimitive::~CurvesPrimitive()
{
}

Imath::Box3f CurvesPrimitive::bound() const
{
	return m_memberData->bound;
}

void CurvesPrimitive::addPrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primVar )
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
	bool linear, ribbons;
	renderMode( state, linear, ribbons );

	if( linear && !ribbons  )
	{
		// we just render in the standard way.
		return Primitive::shaderSetup( shader, state );
	}

	// we're rendering with ribbons and/or cubic interpolation. we need
	// to substitute in a geometry shader to do the work.

	for( MemberData::GeometrySetupVector::const_iterator it = m_memberData->geometrySetups.begin(), eIt = m_memberData->geometrySetups.end(); it != eIt; it++ )
	{
		if( it->originalShader == shader && it->linear == linear && it->ribbons == ribbons )
		{
			return it->shaderSetup.get();
		}
	}

	ConstShaderPtr geometryShader = shader;
	ShaderStateComponent *shaderStateComponent = state->get<ShaderStateComponent>();
	if( geometryShader->geometrySource() == "" )
	{
		// if the current shader has a specific geometry shader component,
		// then we assume the user has provided one capable of doing the tesselation,
		// but if not then we substitute in our own geometry shader.
		ShaderLoader *shaderLoader = shaderStateComponent->shaderLoader();
		if( ribbons )
		{
			if( linear )
			{
				geometryShader = shaderLoader->create( geometryShader->vertexSource(), g_linearRibbonsGeometrySource, geometryShader->fragmentSource() );
			}
			else
			{
				geometryShader = shaderLoader->create( geometryShader->vertexSource(), g_cubicRibbonsGeometrySource, geometryShader->fragmentSource() );
			}
		}
		else
		{
			// When drawing as lines, default to the constant fragment source, because using a facing ratio
			// shader doesn't work
			geometryShader = shaderLoader->create(
				geometryShader->vertexSource(), g_cubicLinesGeometrySource,
				geometryShader->fragmentSource() != "" ? geometryShader->fragmentSource() : Shader::constantFragmentSource()
			);
		}
	}

	Shader::SetupPtr geometryShaderSetup = new Shader::Setup( geometryShader );
	shaderStateComponent->addParametersToShaderSetup( geometryShaderSetup.get() );
	addPrimitiveVariablesToShaderSetup( geometryShaderSetup.get() );
	geometryShaderSetup->addUniformParameter( "basis", new IECore::M44fData( m_memberData->basis.matrix ) );
	geometryShaderSetup->addUniformParameter( "width", new IECore::M44fData( m_memberData->width ) );

	if( m_memberData->wrap == IECoreScene::CurvesPrimitive::Wrap::Pinned && !linear )
	{
		// To handle "phantom vertices" in the geometry shader, we need to know which
		// vertices correspond to the original endpoints of the curve. We do that using
		// the `vertexIsCurveEndPoint` attribute.
		IECore::IntVectorDataPtr isEndPointData = new IECore::IntVectorData();
		vector<int> &isEndPoint = isEndPointData->writable();
		isEndPoint.resize( m_memberData->points->readable().size(), 0 );

		int i = 0;
		for( auto c : m_memberData->vertsPerCurve->readable() )
		{
			isEndPoint[i] = 1;
			isEndPoint[i+c-1] = 1;
			i += c;
		}
		geometryShaderSetup->addVertexAttribute( "vertexIsCurveEndPoint", isEndPointData );

		// And we use specially adjusted basis matrices to give the effect of the
		// phantom vertices being in the required position.
		M44f phantomBasis0, phantomBasis1, phantomBasis2;
		for( int x = 0; x < 4; ++x )
		{
			// Basis for first segment. The phantom vertex `p[0]` is required to be at `2 * p[1] - p[2]`,
			// so we distribute its weights onto `p[1]` and `p[2]` appropriately.
			phantomBasis0[x][0] = 0.0f;
			phantomBasis0[x][1] = m_memberData->basis.matrix[x][1] + 2 * m_memberData->basis.matrix[x][0];
			phantomBasis0[x][2] = m_memberData->basis.matrix[x][2] - m_memberData->basis.matrix[x][0];
			phantomBasis0[x][3] = m_memberData->basis.matrix[x][3];
			// Basis for last segment. The phantom vertex `p[3]` is required to be at `2 * p[2] - p[1]`.
			phantomBasis1[x][0] = m_memberData->basis.matrix[x][0];
			phantomBasis1[x][1] = m_memberData->basis.matrix[x][1] - m_memberData->basis.matrix[x][3];
			phantomBasis1[x][2] = m_memberData->basis.matrix[x][2] + 2 * m_memberData->basis.matrix[x][3];
			phantomBasis1[x][3] = 0.0f;
			// Basis for single segment in which both endpoints are pinned.
			phantomBasis2[x][0] = 0.0f;
			phantomBasis2[x][1] = m_memberData->basis.matrix[x][1] + 2 * m_memberData->basis.matrix[x][0] - m_memberData->basis.matrix[x][3];
			phantomBasis2[x][2] = m_memberData->basis.matrix[x][2] - m_memberData->basis.matrix[x][0] + 2 * m_memberData->basis.matrix[x][3];
			phantomBasis2[x][3] = 0.0f;
		}

		geometryShaderSetup->addUniformParameter( "phantomBasis0", new IECore::M44fData( phantomBasis0 ) );
		geometryShaderSetup->addUniformParameter( "phantomBasis1", new IECore::M44fData( phantomBasis1 ) );
		geometryShaderSetup->addUniformParameter( "phantomBasis2", new IECore::M44fData( phantomBasis2 ) );
	}

	m_memberData->geometrySetups.push_back( MemberData::GeometrySetup( shader, geometryShaderSetup, linear, ribbons ) );

	return geometryShaderSetup.get();
}

void CurvesPrimitive::render( const State *currentState, IECore::TypeId style ) const
{
	bool linear, ribbons;
	renderMode( currentState, linear, ribbons );

	if( style == DrawPoints::staticTypeId() )
	{
		if( linear && !ribbons )
		{
			glDrawArrays( GL_POINTS, 0, m_memberData->points->readable().size() );
		}
		else
		{
			// We can't implement point drawing here because our `shaderSetup()`
			// override will have created a geometry shader unsuitable for
			// rendering points.
			/// \todo Redesign the Primitive classes to make this possible - see
			/// comments on `Primitive::shaderSetup()`.
		}
		return;
	}

	if( !ribbons && style != DrawWireframe::staticTypeId() && currentState->get<DrawWireframe>()->value() )
	{
		// If we're going to be drawing wireframe, then don't draw anything else
		// as otherwise our wireframe will be depth culled. Wireframe might seem like
		// an odd mode for lines, but in practice it is used for rendering selection.
		/// \todo Consider representing selection more explicitly.
		return;
	}

	if( !ribbons )
	{
		glLineWidth( currentState->get<GLLineWidth>()->value() );
		if( linear )
		{
			renderInstances( 1 );
			return;
		}
	}

	if( linear )
	{
		ensureLinearAdjacencyVertIds();
		Buffer::ScopedBinding indexBinding( *(m_memberData->linearAdjacencyVertIdsBuffer), GL_ELEMENT_ARRAY_BUFFER );
		glDrawElements( GL_LINES_ADJACENCY, m_memberData->numLinearAdjacencyVertIds, GL_UNSIGNED_INT, nullptr );
	}
	else
	{
		ensureAdjacencyVertIds();
		Buffer::ScopedBinding indexBinding( *(m_memberData->adjacencyVertIdsBuffer), GL_ELEMENT_ARRAY_BUFFER );
		glDrawElements( GL_LINES_ADJACENCY, m_memberData->numAdjacencyVertIds, GL_UNSIGNED_INT, nullptr );
	}
}

void CurvesPrimitive::renderInstances( size_t numInstances ) const
{
	ensureVertIds();
	Buffer::ScopedBinding indexBinding( *(m_memberData->vertIdsBuffer), GL_ELEMENT_ARRAY_BUFFER );
	glDrawElementsInstancedARB( GL_LINES, m_memberData->numVertIds, GL_UNSIGNED_INT, nullptr, numInstances );
}

void CurvesPrimitive::renderMode( const State *state, bool &linear, bool &ribbons ) const
{
	if( glslVersion() < 150 )
	{
		linear = true;
		ribbons = false;
		return;
	}

	linear = m_memberData->basis==IECore::CubicBasisf::linear() || state->get<IgnoreBasis>()->value();
	ribbons = !state->get<UseGLLines>()->value();
}

bool CurvesPrimitive::renderUsesGLLines( const State *state ) const
{
	if( const auto s = state->get<UseGLLines>() )
	{
		if( s->value() )
		{
			return true;
		}
	}
	return glslVersion() < 150;
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
		if( m_memberData->wrap == IECoreScene::CurvesPrimitive::Wrap::Periodic )
		{
			vertIds.push_back( vertIndex );
			vertIds.push_back( vertIndex - numSegments );
		}
		vertIndex++;
	}

	m_memberData->numVertIds = vertIds.size();

	CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();
	m_memberData->vertIdsBuffer = IECore::runTimeCast<const Buffer>( cachedConverter->convert( vertIdsData.get() ) );
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
		int numSegments = IECoreScene::CurvesPrimitive::numSegments( m_memberData->basis, m_memberData->wrap, numPoints );
		unsigned pi = 0;
		for( int i=0; i<numSegments; i++ )
		{
			if( m_memberData->wrap == IECoreScene::CurvesPrimitive::Wrap::Pinned )
			{
				// Pinned curve. We give the first and last segment special
				// treatment so as to interpolate all the way to the endpoint.
				// This could be achieved by adding "phantom vertices", but we
				// don't want to have to preprocess all the vertex primitive
				// variables to add them. Instead we use custom basis matrices
				// which have the same effect, by distributing the endpoint
				// weights onto the two vertices from which the phantom vertex
				// would be constructed. This means that the endpoint vertices
				// are actually unused because their weights are zero.
				if( i == 0 )
				{
					vertIds.push_back( baseIndex + pi ); // Unused
					vertIds.push_back( baseIndex + pi );
					vertIds.push_back( baseIndex + pi + 1 );
					vertIds.push_back( baseIndex + pi + ( numSegments > 1 ? 2 : 1 ) );
					continue;
				}
				else if( i == numSegments - 1 )
				{
					vertIds.push_back( baseIndex + pi );
					vertIds.push_back( baseIndex + pi + 1 );
					vertIds.push_back( baseIndex + pi + 2 );
					vertIds.push_back( baseIndex + pi + 2 ); // Unused
					continue;
				}
			}

			// Regular segment.
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
	m_memberData->adjacencyVertIdsBuffer = IECore::runTimeCast<const Buffer>( cachedConverter->convert( vertIdsData.get() ) );
}

void CurvesPrimitive::ensureLinearAdjacencyVertIds() const
{
	if( m_memberData->linearAdjacencyVertIdsBuffer )
	{
		return;
	}

	IECore::UIntVectorDataPtr vertIdsData = new IECore::UIntVectorData;
	vector<unsigned int> &vertIds = vertIdsData->writable();

	int baseIndex = 0;
	const vector<int> &vertsPerCurve = m_memberData->vertsPerCurve->readable();
	for( vector<int>::const_iterator it = vertsPerCurve.begin(), eIt = vertsPerCurve.end(); it != eIt; it++ )
	{
		unsigned int numPoints = *it;
		int numSegments = IECoreScene::CurvesPrimitive::numSegments( m_memberData->basis, m_memberData->wrap, numPoints );
		int pi = 0;
		for( int i=0; i<numSegments; i++ )
		{
			if( m_memberData->wrap == IECoreScene::CurvesPrimitive::Wrap::Periodic )
			{
				vertIds.push_back( baseIndex + ( (pi-1) % numPoints ) );
				vertIds.push_back( baseIndex + ( (pi) % numPoints ) );
				vertIds.push_back( baseIndex + ( (pi+1) % numPoints ) );
				vertIds.push_back( baseIndex + ( (pi+2) % numPoints ) );
			}
			else
			{
				vertIds.push_back( baseIndex + max( pi-1, 0 ) );
				vertIds.push_back( baseIndex + pi );
				vertIds.push_back( baseIndex + pi + 1 );
				vertIds.push_back( baseIndex + min( pi+2, (int)numPoints-1 ) );
			}
			pi += m_memberData->basis.step;
		}

		baseIndex += numPoints;

	}

	m_memberData->numLinearAdjacencyVertIds = vertIds.size();

	CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();
	m_memberData->linearAdjacencyVertIdsBuffer = IECore::runTimeCast<const Buffer>( cachedConverter->convert( vertIdsData.get() ) );
}
