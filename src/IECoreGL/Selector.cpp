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

#include "IECoreGL/Selector.h"

#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/DepthTexture.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/FrameBuffer.h"
#include "IECoreGL/HitRecord.h"
#include "IECoreGL/IECoreGL.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/Primitive.h"
#include "IECoreGL/Shader.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/TextureLoader.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/UIntTexture.h"

#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"

#include "boost/format.hpp"
#include "boost/timer.hpp"

using namespace IECoreGL;

//////////////////////////////////////////////////////////////////////////
// Selector::Implementation
//////////////////////////////////////////////////////////////////////////

class Selector::Implementation : public IECore::RefCounted
{

	public :

		Implementation( Selector *parent, const Imath::Box2f &region, Mode mode, std::vector<HitRecord> &hits )
			:	m_mode( mode ), m_hits( hits ), m_baseState( new State( true /* complete */ ) ), m_currentName( 0 ), m_nextGeneratedName( 1 ), m_currentIDShader( nullptr )
		{
			// we don't want preexisting errors to trigger exceptions
			// from error checking code in the begin*() methods, because
			// we'd then be throwing in a half constructed state, our destructor
			// wouldn't be run, and we'd be unable to restore the gl state
			// changes we'd made so far. so we throw immediately if there is a
			// preexisting error.
			IECoreGL::Exception::throwIfError();

			if( g_currentSelector )
			{
				throw( IECore::Exception( "Another Selector is already active" ) );
			}

			g_currentSelector = parent;

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
			glGetDoublev( GL_PROJECTION_MATRIX, m_postProjectionMatrix.getValue() );
			glMultMatrixd( projectionMatrix );
			glMatrixMode( GL_MODELVIEW );

			// fall back to GLSelect mode if we can't
			// support IDRender mode.
			if( m_mode == IDRender && glslVersion() < 330 )
			{
				m_mode = GLSelect;
			}

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

		~Implementation() override
		{
			// we don't want preexisting errors to
			// trigger exceptions from error checking code
			// in the end*() methods, because it would prevent
			// us destructing completely. the best we can do
			// is to log the error and carry on.
			GLenum error;
			while( ( error = glGetError()) != GL_NO_ERROR )
			{
				IECore::msg( IECore::Msg::Error, "IECoreGL::Selector end", (const char *)gluErrorString( error ) );
			}

			g_currentSelector = nullptr;

			glPopAttrib();

			switch( m_mode )
			{
				case GLSelect :
					endGLSelect();
					break;
				case IDRender :
					endIDRender();
					break;
				case OcclusionQuery :
					endOcclusionQuery();
					break;
				default :
					assert( 0 );
			}
		}

		Mode mode() const
		{
			return m_mode;
		}

		const Imath::M44d &postProjectionMatrix()
		{
			return m_postProjectionMatrix;
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

			m_currentName = name;
		}

		GLuint loadName()
		{
			const GLuint name = m_nextGeneratedName++;
			loadName( name );
			return name;
		}

		State *baseState()
		{
			return m_baseState.get();
		}

		void pushIDShader( const IECoreGL::Shader *shader )
		{
			bindIDShader( shader );
			m_IDShaderStack.push( shader );
		}

		void popIDShader()
		{
			m_IDShaderStack.pop();
			if( m_IDShaderStack.size() )
			{
				bindIDShader( m_IDShaderStack.top().get() );
			}
		}

		static const Shader *defaultIDShader()
		{
			const char *fragmentSource =

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

		static Selector *currentSelector()
		{
			return g_currentSelector;
		}

	private :

		Mode m_mode;
		Imath::M44d m_postProjectionMatrix;
		std::vector<HitRecord> &m_hits;
		StatePtr m_baseState;
		GLuint m_currentName;
		GLuint m_nextGeneratedName;

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

		void endGLSelect()
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
				m_hits.push_back( h );
				hitRecord += h.offsetToNext();
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// IDRender mode
		//////////////////////////////////////////////////////////////////////////

		FrameBufferPtr m_frameBuffer;
		boost::shared_ptr<FrameBuffer::ScopedBinding> m_frameBufferBinding;
		GLint m_prevProgram;
		ConstShaderPtr m_currentIDShader;
		std::stack<ConstShaderPtr> m_IDShaderStack;
		GLint m_prevViewport[4];
		GLint m_nameUniformLocation;

		void beginIDRender()
		{
			m_frameBuffer = new FrameBuffer();
			m_frameBuffer->setColor( new UIntTexture( 128, 128 ) );
			m_frameBuffer->setDepth( new DepthTexture( 128, 128 ) );
			m_frameBuffer->validate();
			m_frameBufferBinding = boost::shared_ptr<FrameBuffer::ScopedBinding>( new FrameBuffer::ScopedBinding( *m_frameBuffer ) );

			glGetIntegerv( GL_VIEWPORT, m_prevViewport );
			glViewport( 0, 0, 128, 128 );

			GLfloat prevClearColor[4];
			GLfloat prevClearDepth;
			glGetFloatv( GL_COLOR_CLEAR_VALUE, prevClearColor );
			glGetFloatv( GL_DEPTH_CLEAR_VALUE, &prevClearDepth );
			glClearColor( 0.0, 0.0, 0.0, 1.0 );
			glClearDepth( 1.0 );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			glClearColor( prevClearColor[0], prevClearColor[1], prevClearColor[2], prevClearColor[3] );
			glClearDepth( prevClearDepth );

			glGetIntegerv( GL_CURRENT_PROGRAM, &m_prevProgram );
			pushIDShader( defaultIDShader() );
		}

		void loadNameIDRender( GLuint name )
		{
			IECoreGL::Exception::throwIfError();
			glUniform1ui( m_nameUniformLocation, name );
		}

		void endIDRender()
		{
			glUseProgram( m_prevProgram );
			glViewport( m_prevViewport[0], m_prevViewport[1], m_prevViewport[2], m_prevViewport[3] );
			m_frameBufferBinding.reset();

			IECoreImage::ImagePrimitivePtr idsImage = m_frameBuffer->getColor()->imagePrimitive();
			const IECore::UIntVectorData *idsData = static_cast<const IECore::UIntVectorData *>( idsImage->channels["Y"].get() );
			const std::vector<unsigned int> ids = idsData->readable();

			IECoreImage::ImagePrimitivePtr zImage = m_frameBuffer->getDepth()->imagePrimitive();
			const IECore::FloatVectorData *zData = static_cast<const IECore::FloatVectorData *>( zImage->channels["Z"].get() );
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
					HitRecord r( Imath::limits<float>::max(), Imath::limits<float>::min(), ids[i] );
					it = idRecords.insert( std::pair<unsigned int, HitRecord>( ids[i], r ) ).first;
				}
				it->second.depthMin = std::min( it->second.depthMin, z[i] );
				it->second.depthMax = std::max( it->second.depthMax, z[i] );
			}

			m_hits.clear();
			m_hits.reserve( idRecords.size() );
			for( std::map<unsigned int, HitRecord>::const_iterator it = idRecords.begin(), eIt = idRecords.end(); it != eIt; it++ )
			{
				m_hits.push_back( it->second );
			}
		}

		void bindIDShader( const IECoreGL::Shader *shader )
		{
			if( shader == m_currentIDShader )
			{
				// early out to avoid the relatively expensive operations
				// below if we've already loaded the shader.
				return;
			}

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

			m_currentIDShader = shader;
			glUseProgram( m_currentIDShader->program() );

			std::vector<GLenum> buffers;
			buffers.resize( fragDataLocation + 1, GL_NONE );
			buffers[buffers.size()-1] = GL_COLOR_ATTACHMENT0;
			glDrawBuffers( buffers.size(), &buffers[0] );

			loadNameIDRender( m_currentName );
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

			GLfloat prevClearColor[4];
			GLfloat prevClearDepth;
			glGetFloatv( GL_COLOR_CLEAR_VALUE, prevClearColor );
			glGetFloatv( GL_DEPTH_CLEAR_VALUE, &prevClearDepth );
			glClearColor( 0.0, 0.0, 0.0, 1.0 );
			glClearDepth( 1.0 );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			glClearColor( prevClearColor[0], prevClearColor[1], prevClearColor[2], prevClearColor[3] );
			glClearDepth( prevClearDepth );

			m_baseState->add( depthTestStateComponent(), true /* override */ );
		}

		void loadNameOcclusionQuery( GLuint name )
		{
			if( m_queries.size() )
			{
				glEndQueryARB( GL_SAMPLES_PASSED_ARB );
			}
			m_queries.push_back( 0 );
			glGenQueriesARB( 1, &(m_queries[m_queries.size()-1]) );
			glBeginQueryARB( GL_SAMPLES_PASSED_ARB, m_queries[m_queries.size()-1] );
			m_queryNames.push_back( name );
		}

		void endOcclusionQuery()
		{
			if( m_queries.size() )
			{
				glEndQueryARB( GL_SAMPLES_PASSED_ARB );
			}

			for( size_t i = 0, e = m_queries.size(); i < e; i++ )
			{
				GLuint samplesPassed = 0;
				glGetQueryObjectuivARB( m_queries[i], GL_QUERY_RESULT_ARB, &samplesPassed );
				if( samplesPassed )
				{
					m_hits.push_back( HitRecord( 0, 0, m_queryNames[i] ) );
				}
			}

			glDeleteQueriesARB( m_queries.size(), &(m_queries[0]) );
			m_baseState->add( const_cast<DepthTestStateComponent *>( State::defaultState()->get<DepthTestStateComponent>() ), false /* no override */ );
		}

};

Selector *Selector::Implementation::g_currentSelector = nullptr;

//////////////////////////////////////////////////////////////////////////
// Selector
//////////////////////////////////////////////////////////////////////////

Selector::Selector( const Imath::Box2f &region, Mode mode, std::vector<HitRecord> &hits )
	:	m_implementation( new Implementation( this, region, mode, hits ) )
{
}

Selector::~Selector()
{
}

Selector::Mode Selector::mode() const
{
	return m_implementation->mode();
}

const Imath::M44d &Selector::postProjectionMatrix()
{
	return m_implementation->postProjectionMatrix();
}

void Selector::loadName( GLuint name )
{
	m_implementation->loadName( name );
}

GLuint Selector::loadName()
{
	return m_implementation->loadName();
}

State *Selector::baseState()
{
	return m_implementation->baseState();
}

void Selector::pushIDShader( const IECoreGL::Shader *idShader )
{
	m_implementation->pushIDShader( idShader );
}

void Selector::popIDShader()
{
	m_implementation->popIDShader();
}

const Shader *Selector::defaultIDShader()
{
	return Implementation::defaultIDShader();
}

Selector *Selector::currentSelector()
{
	return Implementation::currentSelector();
}
