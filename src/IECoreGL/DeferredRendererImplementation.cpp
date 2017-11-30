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

#include "IECoreGL/private/DeferredRendererImplementation.h"
#include "IECoreGL/private/Display.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/State.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/Primitive.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/Texture.h"

#include "IECore/MessageHandler.h"
#include "boost/noncopyable.hpp"

#include "tbb/task.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/atomic.h"

#include <cassert>

using namespace IECoreGL;
using namespace Imath;
using namespace std;

DeferredRendererImplementation::DeferredRendererImplementation()
{

	m_defaultContext = new RenderContext();
	m_defaultContext->transformStack.push( M44f() );

	m_scene = new Scene;
}

DeferredRendererImplementation::~DeferredRendererImplementation()
{
}

void DeferredRendererImplementation::addCamera( CameraPtr camera )
{
	m_scene->setCamera( camera );
}

void DeferredRendererImplementation::addDisplay( ConstDisplayPtr display )
{
	IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::addDisplay", "Not implemented" );
}

void DeferredRendererImplementation::worldBegin()
{
	if( m_defaultContext->transformStack.size()>1 )
	{
		IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::worldBegin", "Mismatched transformBegin/transformEnd detected." );
	}
	if( m_defaultContext->stateStack.size()>0 )
	{
		IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::worldBegin", "Mismatched attributeBegin/attributeEnd detected." );
	}

	m_defaultContext->localTransform = M44f();
	m_defaultContext->transformStack = TransformStack();
	m_defaultContext->transformStack.push( M44f() );

	m_defaultContext->stateStack.push_back( new State( false ) );

	m_defaultContext->groupStack.push( m_scene->root() ); /// \todo this group should have the attribute state accumulated before worldBegin
}

void DeferredRendererImplementation::worldEnd()
{
	// check if no procedural threads are running.
	if ( m_threadContextPool.size() != 0 )
	{
		IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::worldEnd", "Detected threads for procedural rendering that are still running!" );
	}

	if( m_defaultContext->transformStack.size()!=1 )
	{
		IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::worldEnd", "Bad nesting of transformBegin/transformEnd detected." );
	}

	if( m_defaultContext->stateStack.size()!=1 )
	{
		IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::worldEnd", "Bad nesting of attributeBegin/attributeEnd detected." );
	}

	m_defaultContext->localTransform = M44f();
	m_defaultContext->transformStack = TransformStack();
	m_defaultContext->transformStack.push( M44f() );

	/// \todo This is where we would do our rendering and saving of images.
}

void DeferredRendererImplementation::transformBegin()
{
	RenderContext *curContext = currentContext();

	GroupPtr g = new Group;
	g->setTransform( curContext->localTransform );
	{
		IECoreGL::Group::Mutex::scoped_lock lock( curContext->groupStack.top()->mutex() );
		curContext->groupStack.top()->addChild( g );
	}
	curContext->groupStack.push( g );

	curContext->transformStack.push( curContext->localTransform * curContext->transformStack.top() );
	curContext->localTransform = Imath::M44f();
}

void DeferredRendererImplementation::transformEnd()
{
	RenderContext *curContext = currentContext();

	if( curContext->transformStack.size()<=1 )
	{
		IECore::msg( IECore::Msg::Warning, "DeferredRendererImplementation::transformEnd", "Bad nesting." );
		return;
	}

	// recover local transform from group and remove group from stack
	curContext->transformStack.pop();
	curContext->localTransform = curContext->groupStack.top()->getTransform();
	curContext->groupStack.pop();

}

void DeferredRendererImplementation::setTransform( const Imath::M44f &m )
{
	RenderContext *curContext = currentContext();

	// figure out the local transformation from the last group that lead us to the given matrix.
	curContext->localTransform = m * curContext->transformStack.top().inverse();

}

Imath::M44f DeferredRendererImplementation::getTransform() const
{
	const RenderContext *curContext = currentContext();
	// return the current world matrix
	return curContext->localTransform * curContext->transformStack.top();
}

void DeferredRendererImplementation::concatTransform( const Imath::M44f &matrix )
{
	RenderContext *curContext = currentContext();

	curContext->localTransform = matrix * curContext->localTransform;
}

void DeferredRendererImplementation::attributeBegin()
{
	RenderContext *curContext = currentContext();

	GroupPtr g = new Group;
	g->setTransform( curContext->localTransform );
	g->setState( new State( **(curContext->stateStack.rbegin()) ) );
	{
		IECoreGL::Group::Mutex::scoped_lock lock( curContext->groupStack.top()->mutex() );
		curContext->groupStack.top()->addChild( g );
	}
	curContext->groupStack.push( g );

	curContext->transformStack.push( curContext->localTransform * curContext->transformStack.top() );
	curContext->localTransform = Imath::M44f();
	curContext->stateStack.push_back( new State( false ) );
}

void DeferredRendererImplementation::attributeEnd()
{
	RenderContext *curContext = currentContext();

	if( curContext->stateStack.size()<=1 )
	{
		IECore::msg( IECore::Msg::Warning, "DeferredRendererImplementation::attributeEnd", "Bad nesting." );
		return;
	}
	curContext->stateStack.pop_back();

	// recover local transform from group and remove group from stack
	curContext->transformStack.pop();
	curContext->localTransform = curContext->groupStack.top()->getTransform();
	curContext->groupStack.pop();
}

void DeferredRendererImplementation::addState( StateComponentPtr state )
{
	RenderContext *curContext = currentContext();

	(*(curContext->stateStack).rbegin())->add( state );
}

StateComponent *DeferredRendererImplementation::getState( IECore::TypeId type )
{
	RenderContext *curContext = currentContext();

	for( StateStack::reverse_iterator it=curContext->stateStack.rbegin(); it!=curContext->stateStack.rend(); it++ )
	{
		StateComponentPtr c = (*it)->get( type );
		if( c )
		{
			return c.get();
		}
	}
	return const_cast<StateComponent *>( State::defaultState()->get( type ) );
}

void DeferredRendererImplementation::addUserAttribute( const IECore::InternedString &name, IECore::DataPtr value )
{
	RenderContext *curContext = currentContext();

	(*(curContext->stateStack).rbegin())->userAttributes()->writable()[ name ] = value;
}

IECore::Data *DeferredRendererImplementation::getUserAttribute( const IECore::InternedString &name )
{
	RenderContext *curContext = currentContext();

	for( StateStack::reverse_iterator it=curContext->stateStack.rbegin(); it!=curContext->stateStack.rend(); it++ )
	{
		IECore::CompoundDataMap &attrs = (*it)->userAttributes()->writable();
		IECore::CompoundDataMap::iterator attrIt = attrs.find( name );
		if( attrIt != attrs.end() )
		{
			return attrIt->second.get();
		}
	}
	return nullptr;
}

void DeferredRendererImplementation::addPrimitive( ConstPrimitivePtr primitive )
{
	bool visible = static_cast<CameraVisibilityStateComponent *>( getState( CameraVisibilityStateComponent::staticTypeId() ) )->value();
	if( !visible )
	{
		return;
	}

	RenderContext *curContext = currentContext();

	GroupPtr g = new Group;
	g->setTransform( curContext->localTransform );
	g->setState( new State( **(curContext->stateStack.rbegin()) ) );
	/// \todo Make Groups have only const access to children and we won't need this
	/// cast. This is going to be particularly important going forwards, as the key
	/// to decent speed and memory usage is going to be automatically instanced primitives,
	/// each referencing potentially shared vertex buffer objects. Modifying one of those
	/// could have bad consequences for the others.
	g->addChild( boost::const_pointer_cast<Primitive>( primitive ) );

	{
		IECoreGL::Group::Mutex::scoped_lock lock( curContext->groupStack.top()->mutex() );
		curContext->groupStack.top()->addChild( g );
	}
}

void DeferredRendererImplementation::addInstance( GroupPtr grp )
{
	bool visible = static_cast<CameraVisibilityStateComponent *>( getState( CameraVisibilityStateComponent::staticTypeId() ) )->value();
	if( !visible )
	{
		return;
	}

	RenderContext *curContext = currentContext();

	GroupPtr g = new Group;
	g->setTransform( curContext->localTransform );
	g->setState( new State( **(curContext->stateStack.rbegin()) ) );
	g->addChild( grp );

	{
		IECoreGL::Group::Mutex::scoped_lock lock( curContext->groupStack.top()->mutex() );
		curContext->groupStack.top()->addChild( g );
	}
}

// Class that sets the scope of a RenderContext on the renderer thread.
struct DeferredRendererImplementation::ScopedRenderContext : private boost::noncopyable
{
	public:

		// Sets the context on the given renderer, to be reverted on the destructor.
		// context should be a RenderContext with just one element on each stack.
		// msgContext is used on error messages only.
		ScopedRenderContext( RenderContextPtr context, DeferredRendererImplementation &renderer, const char *msgContext ) : m_renderer(renderer), m_context( context ), m_msgContext( msgContext )
		{
			if ( m_context->transformStack.size() != 1 ||
					m_context->stateStack.size() != 1 ||
					m_context->groupStack.size() != 1 )
			{
				throw IECore::Exception( "The given context must have just one element at each stack member!" );
			}

			renderer.pushContext( context );
		}

		~ScopedRenderContext()
		{
			// unregisters this thread context.
			RenderContextPtr removedContext = m_renderer.popContext();

			if ( m_context.get() != removedContext.get() )
			{
				IECore::msg( IECore::Msg::Error, m_msgContext, "Corrupted render context stack!" );
			}

			if ( m_context->transformStack.size() != 1 )
			{
				IECore::msg( IECore::Msg::Error, m_msgContext, "Bad nesting of transformBegin/transformEnd detected." );
			}
			if ( m_context->stateStack.size() != 1 )
			{
				IECore::msg( IECore::Msg::Error, m_msgContext, "Bad nesting of attributeBegin/attributeEnd detected."  );
			}
		}

	private:

		DeferredRendererImplementation &m_renderer;
		RenderContextPtr m_context;
		const char *m_msgContext;
};

class DeferredRendererImplementation::ProceduralTask : public tbb::task, private boost::noncopyable
{
	public:

		ProceduralTask( DeferredRendererImplementation &renderer, IECoreScene::Renderer::ProceduralPtr proc, IECoreScene::RendererPtr param ) :
			m_renderer(renderer), m_procedural(proc), m_param(param), m_taskList(nullptr)
		{
			m_numSubtasks = 0;
			RenderContext *curContext = m_renderer.currentContext();

			// create a RenderContext for a new Procedural based on the current context
			StatePtr completeState = new State( false );
			for ( StateStack::iterator it = curContext->stateStack.begin(); it != curContext->stateStack.end(); it++ )
			{
				completeState->add( *it );
			}
			m_proceduralContext = new RenderContext();
			m_proceduralContext->localTransform = curContext->localTransform;
			m_proceduralContext->transformStack.push( curContext->transformStack.top() );
			m_proceduralContext->stateStack.push_back( completeState );
			m_proceduralContext->groupStack.push( curContext->groupStack.top() );
		}

		~ProceduralTask() override
		{
		}

		task* execute() override
		{
			tbb::task_list taskList;

			m_taskList = &taskList;

			// activates the render context on the task's thread.
			ScopedRenderContext scopedProceduralContext( m_proceduralContext, m_renderer, "DeferredRendererImplementation::ProceduralTask::execute" );
			m_procedural->render( m_param.get() );
			set_ref_count( m_numSubtasks + 1 );
			spawn_and_wait_for_all(taskList);
			taskList.clear();
			m_taskList = nullptr;
			return nullptr;
		}

		void addSubtask( ProceduralTask& subtask )
		{
			if ( !m_taskList )
			{
				IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::ProceduralTask::addSubtask", "No tasklist!" );
				return;
			}
			m_taskList->push_back( subtask );
			m_numSubtasks++;
		}

	private :

		tbb::atomic<unsigned int> m_numSubtasks;
		RenderContextPtr m_proceduralContext;
		DeferredRendererImplementation &m_renderer;
		IECoreScene::Renderer::ProceduralPtr m_procedural;
		IECoreScene::RendererPtr m_param;
		tbb::task_list *m_taskList;
};



void DeferredRendererImplementation::addProcedural( IECoreScene::Renderer::ProceduralPtr proc, IECoreScene::RendererPtr renderer )
{
	bool visible = static_cast<CameraVisibilityStateComponent *>( getState( CameraVisibilityStateComponent::staticTypeId() ) )->value();
	if( !visible )
	{
		return;
	}

	bool withThreads = static_cast<ProceduralThreadingStateComponent *>( getState( ProceduralThreadingStateComponent::staticTypeId() ) )->value();
	if( withThreads )
	{
		bool mainProcedural = ( m_threadContextPool.size() == 0 );

		if ( mainProcedural )
		{
			// the init is necessary for tbb < 2.2, whereas it is automatically
			// created for us in tbb >= 2.2. tbb documentation indicates that it is
			// fine to create multiple instances though so we create one just in case.
			tbb::task_scheduler_init init;

			// create root task.
			ProceduralTask& a = *new( ProceduralTask::allocate_root()) ProceduralTask( *this, proc, renderer );
			tbb::task::spawn_root_and_wait(a);
			// check if all contexts were cleared
			for ( ThreadRenderContext::const_iterator it = m_threadContextPool.begin(); it != m_threadContextPool.end(); it++ )
			{
				if ( (*it).size() )
				{
					IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::procedural", "Non empty thread render context detected!" );
				}
			}
			m_threadContextPool.clear();
		}
		else
		{
			ProceduralTask *parentTask = dynamic_cast< ProceduralTask *>(&ProceduralTask::self());
			if ( parentTask )
			{
				// add a child task to the current task
				ProceduralTask& a = *new(parentTask->allocate_child()) ProceduralTask( *this, proc, renderer );
				// register this class on the parent task
				parentTask->addSubtask( a );
			}
			else
			{
				// Somehow the parent task is not ProceduralTask...
				IECore::msg( IECore::Msg::Error, "DeferredRendererImplementation::procedural", "Incompatible parent task type!" );
			}
		}
	}
	else
	{
		// threading not wanted - just execute immediately
		proc->render( renderer.get() );
	}
}

ScenePtr DeferredRendererImplementation::scene()
{
	return m_scene;
}

const DeferredRendererImplementation::RenderContext *DeferredRendererImplementation::currentContext() const
{
	if ( m_threadContextPool.size() == 0 )
	{
		// If no thread context created so far than there's no procedural being rendered. Returns the default context.
		return m_defaultContext.get();
	}

	ThreadRenderContext::reference myThreadContext = m_threadContextPool.local();
	if ( myThreadContext.size() == 0 )
	{
		// The user created a new thread from the procedural render call...
		// there's no way to know which context it should be using.
		throw IECore::Exception( "Invalid thread used on deferred render! Procedurals should not instantiate threads!" );
	}

	return myThreadContext.top().get();
}

DeferredRendererImplementation::RenderContext *DeferredRendererImplementation::currentContext()
{
	// get the const version of current context and avoid code duplication.
	return const_cast< RenderContext * >(
				static_cast< const DeferredRendererImplementation & >(*this).currentContext()
			);
}

void DeferredRendererImplementation::pushContext( RenderContextPtr context )
{
	ThreadRenderContext::reference myThreadContext = m_threadContextPool.local();

	myThreadContext.push( context );
}

DeferredRendererImplementation::RenderContextPtr DeferredRendererImplementation::popContext()
{
	ThreadRenderContext::reference myThreadContext = m_threadContextPool.local();
	if ( myThreadContext.size() == 0 )
	{
		throw IECore::Exception( "Corrupted thread context stack!" );
	}
	RenderContextPtr top = myThreadContext.top();

	myThreadContext.pop();

	return top;
}
