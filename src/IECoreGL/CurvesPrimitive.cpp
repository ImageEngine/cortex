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
#include "IECoreGL/IECoreGL.h"

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

class CurvesPrimitive::MemberData : public IECore::RefCounted
{

	public :
	
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
				geometryShader = shaderLoader->create( geometryShader->vertexSource(), linearRibbonsGeometrySource(), geometryShader->fragmentSource() );		
			}
			else
			{
				geometryShader = shaderLoader->create( geometryShader->vertexSource(), cubicRibbonsGeometrySource(), geometryShader->fragmentSource() );		
			}
		}
		else
		{
			geometryShader = shaderLoader->create( geometryShader->vertexSource(), cubicLinesGeometrySource(), geometryShader->fragmentSource() );
		}
	}

	Shader::SetupPtr geometryShaderSetup = new Shader::Setup( geometryShader );
	shaderStateComponent->addParametersToShaderSetup( geometryShaderSetup.get() );
	addPrimitiveVariablesToShaderSetup( geometryShaderSetup.get() );
	geometryShaderSetup->addUniformParameter( "basis", new IECore::M44fData( m_memberData->basis.matrix ) );
	geometryShaderSetup->addUniformParameter( "width", new IECore::M44fData( m_memberData->width ) );

	m_memberData->geometrySetups.push_back( MemberData::GeometrySetup( shader, geometryShaderSetup, linear, ribbons ) );
	
	return geometryShaderSetup.get();
}

void CurvesPrimitive::render( const State *currentState, IECore::TypeId style ) const
{
	bool linear, ribbons;
	renderMode( currentState, linear, ribbons );
	
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
		glDrawElements( GL_LINES_ADJACENCY, m_memberData->numLinearAdjacencyVertIds, GL_UNSIGNED_INT, 0 );
	}
	else
	{
		ensureAdjacencyVertIds();
		Buffer::ScopedBinding indexBinding( *(m_memberData->adjacencyVertIdsBuffer), GL_ELEMENT_ARRAY_BUFFER );
		glDrawElements( GL_LINES_ADJACENCY, m_memberData->numAdjacencyVertIds, GL_UNSIGNED_INT, 0 );
	}
}

void CurvesPrimitive::renderInstances( size_t numInstances ) const
{
	ensureVertIds();
	Buffer::ScopedBinding indexBinding( *(m_memberData->vertIdsBuffer), GL_ELEMENT_ARRAY_BUFFER );
	glDrawElementsInstancedARB( GL_LINES, m_memberData->numVertIds, GL_UNSIGNED_INT, 0, numInstances );
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

const std::string &CurvesPrimitive::cubicLinesGeometrySource()
{
	static std::string s = 
	
		"#version 150 compatibility\n"
		""
		"#include \"IECoreGL/CurvesPrimitive.h\"\n"
		""
		"IECOREGL_CURVESPRIMITIVE_DECLARE_CUBIC_LINES_PARAMETERS\n"
		""
		"void main()"
		"{"
		""
		"	for( int i = 0; i < 10; i++ )"
		"	{"
		"		float t = float( i ) / 9.0;"
		"		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH"
		"		gl_Position = IECOREGL_CURVESPRIMITIVE_POSITION( t );"
		"		EmitVertex();"
		""
		"	}"
		"}";
		
	return s;
}

const std::string &CurvesPrimitive::cubicRibbonsGeometrySource()
{
	static std::string s = 
	
		"#version 150 compatibility\n"
		""
		"#include \"IECoreGL/CurvesPrimitive.h\"\n"
		""
		"IECOREGL_CURVESPRIMITIVE_DECLARE_CUBIC_RIBBONS_PARAMETERS\n"
		""
		"out vec3 fragmentI;"
		"out vec3 fragmentN;"
		"out vec2 fragmentuv;"
		""
		"void main()"
		"{"
		""
		"	for( int i = 0; i < 10; i++ )"
		"	{"
		""
		"		float t = float( i ) / 9.0;"
		"		vec4 p, n, uTangent, vTangent;"
		"		IECOREGL_CURVESPRIMITIVE_CUBICFRAME( t, p, n, uTangent, vTangent );"
		""
		"		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH"
		"		fragmentN = n.xyz;"
		"		fragmentI = -n.xyz;"
		"		fragmentuv = vec2( 0, t );"
		"		gl_Position = p + width * uTangent;"
		"		EmitVertex();"
		""
		"		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH"
		"		fragmentN = n.xyz;"
		"		fragmentI = -n.xyz;"
		"		fragmentuv = vec2( 1, t );"
		"		gl_Position = p - width * uTangent;"
		"		EmitVertex();"
		""
		"	}"
		"}";
		
	return s;
}

const std::string &CurvesPrimitive::linearRibbonsGeometrySource()
{
	static std::string s = 
	
		"#version 150 compatibility\n"
		""
		"#include \"IECoreGL/CurvesPrimitive.h\"\n"
		""
		"IECOREGL_CURVESPRIMITIVE_DECLARE_LINEAR_RIBBONS_PARAMETERS\n"
		""
		"out vec3 fragmentI;"
		"out vec3 fragmentN;"
		""
		"void main()"
		"{"
		
		"	for( int i = 0; i < 2; i++ )"
		"	{"

		"		vec4 p, n, uTangent, vTangent;"
		"		IECOREGL_CURVESPRIMITIVE_LINEARFRAME( i, p, n, uTangent, vTangent );"
				
		"		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH"
		"		fragmentN = n.xyz;"
		"		fragmentI = -n.xyz;"
		"		gl_Position = p + width * uTangent;"
		"		EmitVertex();"

		"		IECOREGL_ASSIGN_VERTEX_PASS_THROUGH"
		"		fragmentN = n.xyz;"
		"		fragmentI = -n.xyz;"
		"		gl_Position = p - width * uTangent;"
		"		EmitVertex();"
		""
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
		int numSegments = IECore::CurvesPrimitive::numSegments( m_memberData->basis, m_memberData->periodic, numPoints );
		int pi = 0;
		for( int i=0; i<numSegments; i++ )
		{
			if( m_memberData->periodic )
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
