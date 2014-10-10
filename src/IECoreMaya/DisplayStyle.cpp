//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/Primitive.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/TextureLoader.h"

#include "IECoreMaya/DisplayStyle.h"

using namespace IECoreMaya;

struct DisplayStyle::Data
{
	IECoreGL::StatePtr wireframeState;
	IECoreGL::StatePtr shadedState;
	IECoreGL::StatePtr pointsState;
	IECoreGL::StatePtr boundsState;
	IECoreGL::StatePtr litState;
};

DisplayStyle::DisplayStyle()
{
	m_data = new Data;
	m_data->wireframeState = new IECoreGL::State( true );
	m_data->shadedState = new IECoreGL::State( true );
	m_data->pointsState = new IECoreGL::State( true );
	m_data->boundsState = new IECoreGL::State( true );
	m_data->litState = new IECoreGL::State( true );
	
	m_data->wireframeState->add( new IECoreGL::Primitive::DrawSolid( false ) );
	m_data->wireframeState->add( new IECoreGL::Primitive::DrawWireframe( true ) );

	m_data->pointsState->add( new IECoreGL::Primitive::DrawSolid( false ) );
	m_data->pointsState->add( new IECoreGL::Primitive::DrawPoints( true ) );
	m_data->pointsState->add( new IECoreGL::Primitive::PointWidth( 2.0f ) );

	m_data->boundsState->add( new IECoreGL::Primitive::DrawSolid( false ) );
	m_data->boundsState->add( new IECoreGL::Primitive::DrawBound( true ) );

	m_data->litState->add(
		new IECoreGL::ShaderStateComponent(
			IECoreGL::ShaderLoader::defaultShaderLoader(),
			IECoreGL::TextureLoader::defaultTextureLoader(),
			IECoreGL::Shader::defaultVertexSource(),
			IECoreGL::Shader::defaultGeometrySource(),
			IECoreGL::Shader::lambertFragmentSource(),
			new IECore::CompoundObject()
		),
		/// \todo: by setting true here, we are forcing an override of all other
		/// ShaderStateComponents in the hierarchy. Is this desirable in all cases?
		true
	);
}

DisplayStyle::~DisplayStyle()
{
	delete m_data;
}

IECoreGL::State *DisplayStyle::baseState( M3dView::DisplayStyle style, M3dView::LightingMode lightingMode, bool transferCurrentColor )
{
	switch( style )
	{

		case M3dView::kBoundingBox :
			if( transferCurrentColor )
			{
				Imath::Color4f wc; glGetFloatv( GL_CURRENT_COLOR, wc.getValue() );
				m_data->boundsState->add( new IECoreGL::BoundColorStateComponent( wc ) );
			}
			return m_data->boundsState.get();

		case M3dView::kFlatShaded :
		case M3dView::kGouraudShaded :
			if ( lightingMode != M3dView::kLightDefault )
			{
				return m_data->litState.get();
			}
			return m_data->shadedState.get();

		case M3dView::kWireFrame :
			if( transferCurrentColor )
			{
				Imath::Color4f wc; glGetFloatv( GL_CURRENT_COLOR, wc.getValue() );
				m_data->wireframeState->add( new IECoreGL::WireframeColorStateComponent( wc ) );
			}
			return m_data->wireframeState.get();

		case M3dView::kPoints :
			if( transferCurrentColor )
			{
				Imath::Color4f wc; glGetFloatv( GL_CURRENT_COLOR, wc.getValue() );
				m_data->pointsState->add( new IECoreGL::PointColorStateComponent( wc ) );
			}
			return m_data->pointsState.get();

		default :
			return m_data->shadedState.get();

	}
}
