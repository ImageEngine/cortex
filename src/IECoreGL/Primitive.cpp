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

#include "IECoreGL/Primitive.h"

#include "IECoreGL/Buffer.h"
#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/NumericTraits.h"
#include "IECoreGL/Selector.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/State.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/UniformFunctions.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"

#include "boost/format.hpp"

using namespace IECoreGL;
using namespace std;
using namespace boost;
using namespace Imath;

//////////////////////////////////////////////////////////////////////////
// StateComponents
//////////////////////////////////////////////////////////////////////////

namespace IECoreGL
{

IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawBound, PrimitiveBoundTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawWireframe, PrimitiveWireframeTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::WireframeWidth, PrimitiveWireframeWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawSolid, PrimitiveSolidTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawOutline, PrimitiveOutlineTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::OutlineWidth, PrimitiveOutlineWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::DrawPoints, PrimitivePointsTypeId, bool, false );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::PointWidth, PrimitivePointWidthTypeId, float, 1.0f );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::Selectable, PrimitiveSelectableTypeId, bool, true );
IECOREGL_TYPEDSTATECOMPONENT_SPECIALISEANDINSTANTIATE( Primitive::TransparencySort, PrimitiveTransparencySortStateComponentTypeId, bool, true );

} // namespace IECoreGL

//////////////////////////////////////////////////////////////////////////
// Utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

struct FlatConstant
{

	FlatConstant( const IECore::MurmurHash &hash, const Shader::ConstSetupPtr &shaderSetup = Shader::ConstSetupPtr() )
		:	hash( hash ), shaderSetup( shaderSetup )
	{
	}

	IECore::MurmurHash hash;
	Shader::ConstSetupPtr shaderSetup;

	bool operator < ( const FlatConstant &rhs ) const
	{
		return hash < rhs.hash;
	}

};

// Makes a constant shader by taking the vertex and geometry
// shader from the state and combining them with a flat fragment
// shader.
const Shader::Setup *flatConstantShaderSetup( State *state, bool forIDRender )
{
	ShaderStateComponent *shaderStateComponent = state->get<ShaderStateComponent>();

	// Get a hash to represent the shader we're about to make.
	IECore::MurmurHash hash = shaderStateComponent->hash();
	hash.append( forIDRender );

	// If we've made an equivalent shader before, then
	// just return it.
	static vector<FlatConstant> flatConstants;
	vector<FlatConstant>::iterator it = lower_bound(
		flatConstants.begin(),
		flatConstants.end(),
		hash
	);
	if( it != flatConstants.end() && it->hash == hash )
	{
		return it->shaderSetup.get();
	}

	// If we haven't, then make one.

	static const std::string constantFragmentSource =

		"uniform vec3 Cs;" // get colour from uniform Cs, bypassing vertexCs
		""
		"void main()"
		"{"
		"	gl_FragColor = vec4( Cs, 1 );"
		"}";

	const std::string &fragmentSource = forIDRender ? Selector::defaultIDShader()->fragmentSource() : constantFragmentSource;

	const Shader *originalShader = shaderStateComponent->shaderSetup()->shader();
	ShaderLoader *shaderLoader = shaderStateComponent->shaderLoader();
	ConstShaderPtr shader = shaderLoader->create( originalShader->vertexSource(), originalShader->geometrySource(), fragmentSource );
	Shader::SetupPtr shaderSetup = new Shader::Setup( shader );
	shaderStateComponent->addParametersToShaderSetup( shaderSetup.get() );

	// Put it in our store so we don't have to remake it next time.

	flatConstants.insert( it, FlatConstant( hash, shaderSetup.get() ) );

	return shaderSetup.get();
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// Primitive implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( Primitive );

Primitive::Primitive()
{
}

Primitive::~Primitive()
{
}

void Primitive::addPrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primVar )
{
	if ( primVar.interpolation == IECoreScene::PrimitiveVariable::Constant )
	{
		addUniformAttribute( name, primVar.expandedData() );
	}
	else
	{
		addVertexAttribute( name, primVar.expandedData() );
	}
}

void Primitive::render( State *state ) const
{
	const Selector *currentSelector = Selector::currentSelector();
	if( currentSelector && !state->get<Primitive::Selectable>()->value() )
	{
		// if we're not selectable then we don't need to do anything
		return;
	}

	/// \todo Really we want to remove use of this deprecated push/pop functionality.
	PushAttrib attributeBlock( GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT | GL_LINE_BIT | GL_POINT_BIT );

	// if we're in GL_SELECT render mode then just render solid
	// with a simple shader and early out.
	if( currentSelector && currentSelector->mode() == Selector::GLSelect && state->get<DrawSolid>()->value() )
	{
		const Shader::Setup *uniformSetup = flatConstantShaderSetup( state, /* forIDRender = */ false );
		Shader::Setup::ScopedBinding uniformBinding( *uniformSetup );
		const Shader::Setup *primitiveSetup = shaderSetup( uniformSetup->shader(), state );
		Shader::Setup::ScopedBinding primitiveBinding( *primitiveSetup );
		render( state, Primitive::DrawSolid::staticTypeId() );
		return;
	}

	// render the shaded primitive if requested
	///////////////////////////////////////////

	if( state->get<Primitive::DrawSolid>()->value() )
	{
		glDepthMask( !depthSortRequested( state ) );
		// the state itself will have a shader with some nice uniform parameter
		// values. we are responsible for binding this setup. unless we're performing
		// an id render for selection, in which case we're responsible for binding an
		// id shader.
		const Shader::Setup *uniformSetup = nullptr;
		if( currentSelector && currentSelector->mode() == Selector::IDRender )
		{
			uniformSetup = flatConstantShaderSetup( state, /* forIDRender = */ true );
		}
		else
		{
			uniformSetup = state->get<ShaderStateComponent>()->shaderSetup();
		}
		Shader::Setup::ScopedBinding uniformBinding( *uniformSetup );
		// we then bind our own setup on top, adding in the parameters
		// stored on the primitive itself.
		const Shader *shader = uniformSetup->shader();
		const Shader::Setup *primitiveSetup = shaderSetup( shader, state );
		Shader::Setup::ScopedBinding primitiveBinding( *primitiveSetup );
		// inherit Cs from the state if it isn't provided by the shader or a primitive variable
		if( !uniformSetup->hasCsValue() && !primitiveSetup->hasCsValue() )
		{
			if( const Shader::Parameter *csParameter = primitiveSetup->shader()->csParameter() )
			{
				glUniform3fv( csParameter->location, 1, state->get<Color>()->value().getValue() );
			}
		}
		// then we defer to the derived class to perform the draw call.
		render( state, Primitive::DrawSolid::staticTypeId() );
	}

	// then perform wireframe shading etc as requested
	//////////////////////////////////////////////////

	bool drawOutline = state->get<Primitive::DrawOutline>()->value();
	bool drawWireframe = state->get<Primitive::DrawWireframe>()->value();
	bool drawPoints = state->get<Primitive::DrawPoints>()->value();
	bool drawBound = state->get<Primitive::DrawBound>()->value();
	if( !(drawOutline || drawWireframe || drawPoints || drawBound) )
	{
		return;
	}

	// get a constant shader suitable for drawing wireframes, points etc.
	// we do this by taking the current shader from the state and overriding
	// just the fragment shader within it - we want to keep any vertex or
	// geometry shader the user has specified. again, if we're performing an
	// id render, we're responsible for binding an id shader instead.
	const Shader::Setup *uniformSetup = flatConstantShaderSetup( state, currentSelector && currentSelector->mode() == Selector::IDRender );
	Shader::Setup::ScopedBinding uniformBinding( *uniformSetup );

	const Shader::Setup *primitiveSetup = shaderSetup( uniformSetup->shader(), state );
	Shader::Setup::ScopedBinding primitiveBinding( *primitiveSetup );

	GLint csIndex = -1;
	if( const Shader::Parameter *csParameter = uniformSetup->shader()->csParameter() )
	{
		csIndex = csParameter->location;
	}

	// wireframe

	if( drawWireframe )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		float width = state->get<Primitive::WireframeWidth>()->value();
		glEnable( GL_POLYGON_OFFSET_LINE );
		glPolygonOffset( -1 * width, -1 );
		glLineWidth( width );
		if( csIndex >= 0 )
		{
			glUniform3fv( csIndex, 1, state->get<WireframeColorStateComponent>()->value().getValue() );
		}
		render( state, Primitive::DrawWireframe::staticTypeId() );
	}

	// points

	if( drawPoints )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
		float width = state->get<Primitive::PointWidth>()->value();
		glEnable( GL_POLYGON_OFFSET_POINT );
		glPolygonOffset( -2 * width, -1 );
		glPointSize( width );
		if( csIndex >= 0 )
		{
			glUniform3fv( csIndex, 1, state->get<PointColorStateComponent>()->value().getValue() );
		}
		render( state, Primitive::DrawPoints::staticTypeId() );
	}

	// outline

	if( drawOutline )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glEnable( GL_POLYGON_OFFSET_LINE );
		float width = 2 * state->get<Primitive::OutlineWidth>()->value();
		glPolygonOffset( 2 * width, 1 );
		glLineWidth( width );
		if( csIndex >= 0 )
		{
			glUniform3fv( csIndex, 1, state->get<OutlineColorStateComponent>()->value().getValue() );
		}
		render( state, Primitive::DrawOutline::staticTypeId() );
	}

	// bound

	if( drawBound && ( !currentSelector || currentSelector->mode() != Selector::IDRender ) )
	{
		/// \todo Support IDRender selection mode.
		Shader::Setup::ScopedBinding boundSetupBinding( *boundSetup() );
		glLineWidth( 1 );
		if( csIndex >= 0 )
		{
			glUniform3fv( csIndex, 1, state->get<BoundColorStateComponent>()->value().getValue() );
		}
		glDrawArrays( GL_LINES, 0, 24 );
	}

}

const Shader::Setup *Primitive::shaderSetup( const Shader *shader, State *state ) const
{
	for( ShaderSetupVector::const_iterator it = m_shaderSetups.begin(), eIt = m_shaderSetups.end(); it != eIt; it++ )
	{
		if( (*it)->shader() == shader )
		{
			return it->get();
		}
	}

	Shader::SetupPtr setup = new Shader::Setup( shader );
	addPrimitiveVariablesToShaderSetup( setup.get() );

	m_shaderSetups.push_back( setup );
	return setup.get();
}

void Primitive::addPrimitiveVariablesToShaderSetup( Shader::Setup *shaderSetup, const std::string &vertexPrefix, GLuint vertexDivisor ) const
{
	for( AttributeMap::const_iterator it = m_vertexAttributes.begin(), eIt = m_vertexAttributes.end(); it != eIt; it++ )
	{
		shaderSetup->addVertexAttribute( vertexPrefix + it->first, it->second, vertexDivisor );
		shaderSetup->addUniformParameter( vertexPrefix + it->first + "Active", new IECore::BoolData( true ) );
	}
	for( AttributeMap::const_iterator it = m_uniformAttributes.begin(), eIt = m_uniformAttributes.end(); it != eIt; it++ )
	{
		shaderSetup->addUniformParameter( it->first, it->second );
	}
}

void Primitive::render( const State *currentState, IECore::TypeId style ) const
{
	renderInstances( 1 );
}

void Primitive::addUniformAttribute( const std::string &name, IECore::ConstDataPtr data )
{
	m_uniformAttributes[name] = data->copy();
}

void Primitive::addVertexAttribute( const std::string &name, IECore::ConstDataPtr data )
{
	m_vertexAttributes[name] = data->copy();
}

bool Primitive::depthSortRequested( const State * state ) const
{
	return state->get<Primitive::TransparencySort>()->value() &&
		state->get<TransparentShadingStateComponent>()->value();
}

const Shader::Setup *Primitive::boundSetup() const
{
	if( m_boundSetup )
	{
		return m_boundSetup.get();
	}

	Box3f b = bound();
	IECore::V3fVectorDataPtr pData = new IECore::V3fVectorData();
	std::vector<V3f> &p = pData->writable();

	p.push_back( V3f( b.min.x, b.min.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.min.y, b.min.z ) );

	p.push_back( V3f( b.max.x, b.min.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.max.y, b.min.z	) );

	p.push_back( V3f( b.max.x, b.max.y, b.min.z	) );
	p.push_back( V3f( b.min.x, b.max.y, b.min.z	) );

	p.push_back( V3f( b.min.x, b.max.y, b.min.z	) );
	p.push_back( V3f( b.min.x, b.min.y, b.min.z ) );

	p.push_back( V3f( b.min.x, b.min.y, b.max.z ) );
	p.push_back( V3f( b.max.x, b.min.y, b.max.z ) );

	p.push_back( V3f( b.max.x, b.min.y, b.max.z ) );
	p.push_back( V3f( b.max.x, b.max.y, b.max.z	) );

	p.push_back( V3f( b.max.x, b.max.y, b.max.z	) );
	p.push_back( V3f( b.min.x, b.max.y, b.max.z	) );

	p.push_back( V3f( b.min.x, b.max.y, b.max.z	) );
	p.push_back( V3f( b.min.x, b.min.y, b.max.z ) );

	p.push_back( V3f( b.min.x, b.min.y, b.min.z ) );
	p.push_back( V3f( b.min.x, b.min.y, b.max.z ) );

	p.push_back( V3f( b.max.x, b.min.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.min.y, b.max.z ) );

	p.push_back( V3f( b.max.x, b.max.y, b.min.z ) );
	p.push_back( V3f( b.max.x, b.max.y, b.max.z ) );

	p.push_back( V3f( b.min.x, b.max.y, b.min.z ) );
	p.push_back( V3f( b.min.x, b.max.y, b.max.z ) );

	m_boundSetup = new Shader::Setup( Shader::constant() );
	m_boundSetup->addVertexAttribute( "P", pData );

	return m_boundSetup.get();
}
