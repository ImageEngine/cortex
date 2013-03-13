//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/Exception.h"

#include "IECoreGL/HitRecord.h"
#include "IECoreGL/Selector.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/UIntTexture.h"
#include "IECoreGL/DepthTexture.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/FrameBuffer.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/Primitive.h"

using namespace IECoreGL;

//////////////////////////////////////////////////////////////////////////
// Selector::Implementation
//////////////////////////////////////////////////////////////////////////

class Selector::Implementation : public IECore::RefCounted
{

	public :
	
		Implementation( Selector *parent )
			:	m_parent( parent ), m_mode( Invalid ), m_baseState( new State( true /* complete */ ) )
		{
		}
	
		void begin( const Imath::Box2f &region, Mode mode = GLSelect )
		{
			if( g_currentSelector )
			{
				throw( IECore::Exception( "Another Selector is already active" ) );
			}

			g_currentSelector = this->m_parent;

			GLdouble projectionMatrix[16];
			glGetDoublev( GL_PROJECTION_MATRIX, projectionMatrix );
			GLint viewport[4];
			glGetIntegerv( GL_VIEWPORT, viewport );

			Imath::V2f regionCenter = region.center();
			Imath::V2f regionSize = region.size();
			regionCenter.x = viewport[0] + viewport[2] * regionCenter.x;
			regionCenter.y = viewport[1] + viewport[3] * (1.0f - regionCenter.y);
			regionSize.x *= viewport[2];
			regionSize.y *= viewport[3];

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPickMatrix( regionCenter.x, regionCenter.y, regionSize.x, regionSize.y, viewport );
			glMultMatrixd( projectionMatrix );
			glMatrixMode( GL_MODELVIEW );

			m_mode = mode;
			switch( m_mode )
			{
				case GLSelect :
					beginGLSelect();
					break;
				case IDRender :
					beginIDRender();
					break;
				case OcclusionQuery :
					beginOcclusionQuery();
					break;
				default :
					assert( 0 );
			}

			glPushAttrib( GL_ALL_ATTRIB_BITS );
		
		}
		
		void loadName( GLuint name )
		{
			switch( m_mode )
			{
				case GLSelect :
					loadNameGLSelect( name );
					break;
				case IDRender :
					loadNameIDRender( name );
					break;
				case OcclusionQuery :
					loadNameOcclusionQuery( name );
					break;
				default :
					assert( 0 );
			}
		}
		
		size_t end( std::vector<HitRecord> &hits )
		{
			glPopAttrib();
			g_currentSelector = 0;
			const Mode mode = m_mode;
			m_mode = Invalid;
			
			switch( mode )
			{
				case GLSelect :
					return endGLSelect( hits );
				case IDRender :
					return endIDRender( hits );
				case OcclusionQuery :
					return endOcclusionQuery( hits );
				default :
					assert( 0 );
			}

			return 0;
		}

		State *baseState()
		{
			return m_baseState;
		}

		void loadIDShader( const IECoreGL::Shader *shader )
		{
			const IECoreGL::Shader::Parameter *nameParameter = shader->uniformParameter( "ieCoreGLNameIn" );
			if( !nameParameter )
			{
				throw IECore::Exception( "ID shader does not have an ieCoreGLNameIn parameter" );
			}
			
			GLint fragDataLocation = glGetFragDataLocation( shader->program(), "ieCoreGLNameOut" );
			if( fragDataLocation < 0 )
			{
				throw IECore::Exception( "ID shader does not have an ieCoreGLNameOut output" );			
			}
			
			m_nameUniformLocation = nameParameter->location;
			
			glUseProgram( shader->program() );
					
			std::vector<GLenum> buffers;
			buffers.resize( fragDataLocation + 1, GL_NONE );
			buffers[buffers.size()-1] = GL_COLOR_ATTACHMENT0;
			glDrawBuffers( buffers.size(), &buffers[0] );
		}

		static Selector *currentSelector()
		{
			return g_currentSelector;
		}

	private :

		Selector *m_parent;
		Mode m_mode;
		StatePtr m_baseState;

		static Selector *g_currentSelector;

		//////////////////////////////////////////////////////////////////////////
		// GLSelect mode
		//////////////////////////////////////////////////////////////////////////

		std::vector<GLuint> m_selectBuffer;
		
		void beginGLSelect()
		{
			m_selectBuffer.resize( 20000 ); // enough to select 5000 distinct objects
			glSelectBuffer( m_selectBuffer.size(), &(m_selectBuffer[0]) );
			glRenderMode( GL_SELECT );

			glInitNames();
			glPushName( 0 );
		}

		void loadNameGLSelect( GLuint name )
		{
			glLoadName( name );
		}

		size_t endGLSelect( std::vector<HitRecord> &hits )
		{		
			int numHits = glRenderMode( GL_RENDER );
			if( numHits < 0 )
			{
				IECore::msg( IECore::Msg::Warning, "IECoreGL::Selector::end", "Selection buffer overflow." );
				numHits *= -1;
			}

			// get the hits out of the select buffer.
			GLuint *hitRecord = &(m_selectBuffer[0]);
			for( int i=0; i<numHits; i++ )
			{
				HitRecord h( hitRecord );
				hits.push_back( h );
				hitRecord += h.offsetToNext();
			}
			return hits.size();
		}

		//////////////////////////////////////////////////////////////////////////
		// IDRender mode
		//////////////////////////////////////////////////////////////////////////

		FrameBufferPtr m_frameBuffer;
		boost::shared_ptr<FrameBuffer::ScopedBinding> m_frameBufferBinding;
		GLint m_prevProgram;
		GLint m_prevViewport[4];
		GLint m_nameUniformLocation;
		
		static Shader *idShader()
		{
			static const char *fragmentSource = 

				"#version 330\n"
				""
				"uniform uint ieCoreGLNameIn;"
				""
				"layout( location=0 ) out uint ieCoreGLNameOut;"
				""
				"void main()"
				"{"
				"	ieCoreGLNameOut = ieCoreGLNameIn;"
				"}";

			static ShaderPtr s = new Shader( "", fragmentSource );
			return s.get();
		}

		static std::vector<StateComponentPtr> &idStateComponents()
		{
			static std::vector<StateComponentPtr> s;
			if( !s.size() )
			{
				s.push_back( new ShaderStateComponent( new Shader::Setup( idShader() ) ) );
				s.push_back( new Primitive::DrawBound( false ) );
				s.push_back( new Primitive::DrawWireframe( false ) );
				s.push_back( new Primitive::DrawOutline( false ) );
				s.push_back( new Primitive::DrawPoints( false ) );
			}
			return s;
		}

		void beginIDRender()
		{
			m_frameBuffer = new FrameBuffer();
			m_frameBuffer->setColor( new UIntTexture( 128, 128 ) );
			m_frameBuffer->setDepth( new DepthTexture( 128, 128 ) );
			m_frameBuffer->validate();
			m_frameBufferBinding = boost::shared_ptr<FrameBuffer::ScopedBinding>( new FrameBuffer::ScopedBinding( *m_frameBuffer ) );
			
			glGetIntegerv( GL_VIEWPORT, m_prevViewport );
			glViewport( 0, 0, 128, 128 );
			
			glClearColor( 0.0, 0.0, 0.0, 1.0 );
			glClearDepth( 1.0 );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			
			const std::vector<StateComponentPtr> &stateComponents = idStateComponents();
			for( std::vector<StateComponentPtr>::const_iterator it = stateComponents.begin(), eIt = stateComponents.end(); it != eIt; it++ )
			{
				m_baseState->add( *it, true /* override */ );
			}
			
			glGetIntegerv( GL_CURRENT_PROGRAM, &m_prevProgram );
			loadIDShader( idShader() );	
		}

		void loadNameIDRender( GLuint name )
		{
			Exception::throwIfError();
			glUniform1ui( m_nameUniformLocation, name );
		}

		size_t endIDRender( std::vector<HitRecord> &hits )
		{
			glUseProgram( m_prevProgram );
			glViewport( m_prevViewport[0], m_prevViewport[1], m_prevViewport[2], m_prevViewport[3] );
			m_frameBufferBinding.reset();

			IECore::ImagePrimitivePtr idsImage = m_frameBuffer->getColor()->imagePrimitive();
			const IECore::UIntVectorData *idsData = static_cast<const IECore::UIntVectorData *>( idsImage->variables["Y"].data.get() );
			const std::vector<unsigned int> ids = idsData->readable();

			IECore::ImagePrimitivePtr zImage = m_frameBuffer->getDepth()->imagePrimitive();
			const IECore::FloatVectorData *zData = static_cast<const IECore::FloatVectorData *>( zImage->variables["Z"].data.get() );
			const std::vector<float> z = zData->readable();

			std::map<unsigned int, HitRecord> idRecords;	
			for( size_t i = 0, e = ids.size(); i < e; i++ )
			{
				if( ids[i] == 0 )
				{
					continue;
				}
				std::map<unsigned int, HitRecord>::iterator it = idRecords.find( ids[i] );
				if( it == idRecords.end() )
				{
					HitRecord r( Imath::limits<float>::max(), Imath::limits<float>::min(), NameStateComponent::nameFromGLName( ids[i] ) );
					it = idRecords.insert( std::pair<unsigned int, HitRecord>( ids[i], r ) ).first;
				}
				it->second.depthMin = std::min( it->second.depthMin, z[i] );
				it->second.depthMax = std::max( it->second.depthMax, z[i] );		
			}

			hits.clear();
			hits.reserve( idRecords.size() );
			for( std::map<unsigned int, HitRecord>::const_iterator it = idRecords.begin(), eIt = idRecords.end(); it != eIt; it++ )
			{
				hits.push_back( it->second );
			}

			const std::vector<StateComponentPtr> &stateComponents = idStateComponents();
			for( std::vector<StateComponentPtr>::const_iterator it = stateComponents.begin(), eIt = stateComponents.end(); it != eIt; it++ )
			{
				m_baseState->add( const_cast<StateComponent *>( State::defaultState()->get( (*it)->typeId() ) ), false /* no override */ );
			}

			return hits.size();
		}

		//////////////////////////////////////////////////////////////////////////
		// OcclusionQuery
		//////////////////////////////////////////////////////////////////////////
		
		std::vector<GLuint> m_queries;
		std::vector<GLuint> m_queryNames;
		
		static DepthTestStateComponent *depthTestStateComponent()
		{
			static DepthTestStateComponentPtr d = new DepthTestStateComponent( false );
			return d.get();
		}
		
		void beginOcclusionQuery()
		{
			m_queries.resize( 0 );
			m_queryNames.resize( 0 );
			glClearColor( 0.0, 0.0, 0.0, 1.0 );
			glClearDepth( 1.0 );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			m_baseState->add( depthTestStateComponent(), true /* override */ );
		}

		void loadNameOcclusionQuery( GLuint name )
		{
			if( m_queries.size() )
			{
				glEndQuery( GL_ANY_SAMPLES_PASSED );
			}
			m_queries.push_back( 0 );
			glGenQueries( 1, &(m_queries[m_queries.size()-1]) );
			glBeginQuery( GL_ANY_SAMPLES_PASSED, m_queries[m_queries.size()-1] );
			m_queryNames.push_back( name );
		}

		size_t endOcclusionQuery( std::vector<HitRecord> &hits )
		{	
			if( m_queries.size() )
			{
				glEndQuery( GL_ANY_SAMPLES_PASSED );
			}
		
			for( size_t i = 0, e = m_queries.size(); i < e; i++ )
			{
				GLuint samplesPassed = 0;
				glGetQueryObjectuiv( m_queries[i], GL_QUERY_RESULT, &samplesPassed );
				if( samplesPassed )
				{
					hits.push_back( HitRecord( 0, 0, NameStateComponent::nameFromGLName( m_queryNames[i] ) ) );
				}
			}
		
			glDeleteQueries( m_queries.size(), &(m_queries[0]) );
			m_baseState->add( const_cast<DepthTestStateComponent *>( State::defaultState()->get<DepthTestStateComponent>() ), false /* no override */ );
			return hits.size();
		}

};

Selector *Selector::Implementation::g_currentSelector = 0;

//////////////////////////////////////////////////////////////////////////
// Selector
//////////////////////////////////////////////////////////////////////////

Selector::Selector()
	:	m_implementation( new Implementation( this ) )
{
}

Selector::~Selector()
{
}

void Selector::begin( const Imath::Box2f &region, Mode mode )
{
	m_implementation->begin( region, mode );
}

void Selector::loadName( GLuint name )
{
	m_implementation->loadName( name );
}

size_t Selector::end( std::vector<HitRecord> &hits )
{
	return m_implementation->end( hits );
}

State *Selector::baseState()
{
	return m_implementation->baseState();
}

void Selector::loadIDShader( const IECoreGL::Shader *idShader )
{
	m_implementation->loadIDShader( idShader );
}

Selector *Selector::currentSelector()
{
	return Implementation::currentSelector();
}
