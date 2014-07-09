//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include <stack>
#include <fnmatch.h>

#include "boost/regex.hpp"
#include "boost/tokenizer.hpp"

#include "tbb/enumerable_thread_specific.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"

#include "IECore/CapturingRenderer.h"
#include "IECore/PointsPrimitive.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/SpherePrimitive.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/NURBSPrimitive.h"
#include "IECore/PatchMeshPrimitive.h"
#include "IECore/MessageHandler.h"
#include "IECore/Group.h"
#include "IECore/AttributeState.h"
#include "IECore/Shader.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Light.h"

using namespace std;
using namespace tbb;
using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( CapturingRenderer );

//////////////////////////////////////////////////////////////////////////
// implementation. this is a private class that holds all the data and
// does all the work.
//////////////////////////////////////////////////////////////////////////

class CapturingRenderer::Implementation
{
	public :
	
		Implementation()
			:	m_mainContext( 0 )
		{
			m_topLevelProceduralParent = new( tbb::task::allocate_root() ) tbb::empty_task;
		}
		
		~Implementation()
		{
			m_topLevelProceduralParent->destroy( *m_topLevelProceduralParent );
		}
	
		void setOption( const std::string &name, ConstDataPtr value )
		{
			ContextStack &contextStack = m_threadContexts.local();
			if( contextStack.size() )
			{
				msg( Msg::Warning, "CapturingRenderer::Implementation::setOption", "Cannot call setOption() after worldBegin()." );
				return;
			}
			
			m_options[name] = value->copy();
		}
		
		ConstDataPtr getOption( const std::string &name )
		{
			std::map<std::string, ConstDataPtr>::const_iterator it = m_options.find( name );
			if( it==m_options.end() )
			{
				return 0;
			}
			return it->second;
		}
	
		void worldBegin()
		{
			ContextStack &contextStack = m_threadContexts.local();
			if( contextStack.size() )
			{
				msg( Msg::Warning, "CapturingRenderer::Implementation::worldBegin", "Already in world." );
				return;
			}
			contextStack.push( new Context );
			m_world = 0;
			m_mainContext = contextStack.top().get();
			m_topLevelProceduralParent->set_ref_count( 1 ); // for the wait_for_all() in worldEnd()
		}
		
		void worldEnd()
		{		
			ContextStack &contextStack = m_threadContexts.local();
			if( contextStack.size()!=1 )
			{
				msg( Msg::Warning, "CapturingRenderer::Implementation::worldEnd", "Bad world nesting." );
				return;
			}
			if( contextStack.top()->stack.size()!=1 )
			{
				msg( Msg::Warning, "CapturingRenderer::Implementation::worldEnd", "Bad attribute/transform nesting." );
				return;
			}
												
			m_topLevelProceduralParent->wait_for_all(); // wait for all procedurals to finish
			
			collapseGroups( contextStack.top()->stack.back() );
			m_world = contextStack.top()->stack.back().group;
			contextStack.pop();
			m_mainContext = 0;
		}
		
		void attributeBegin()
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			context->stack.push_back( Context::State() );
			
			Context::State &currentState = context->stack.back();
			Context::State &prevState = context->stack[context->stack.size()-2];
			currentState.worldTransform = prevState.worldTransform;
			addChild( prevState, currentState.group );			
		}
		
		void attributeEnd()
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			collapseGroups( context->stack.back() );
			context->stack.pop_back();
		}
		
		void concatTransform( const Imath::M44f &transform )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
		
			Context::State &state = context->stack.back();
			state.worldTransform = transform * state.worldTransform;
			state.localTransform = transform * state.localTransform;
			
			if( state.group->children().size() )
			{
				state.canCollapseGroups = false;
			}
		}
		
		void setTransform( const Imath::M44f &transform )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			Context::State &currentState = context->stack.back();
			Context::State &prevState = context->stack[context->stack.size()-2];
			
			currentState.worldTransform = transform;
			currentState.localTransform = transform * prevState.worldTransform.inverse();
			
			if( currentState.group->children().size() )
			{
				currentState.canCollapseGroups = false;
			}
		}
		
		const Imath::M44f getTransform()
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return M44f();
			}
			
			return context->stack.back().worldTransform;
		}
		
		void setAttribute( const std::string &name, ConstDataPtr value )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			Context::State &state = context->stack.back();
			state.attributes[name] = value->copy();
			if( state.group->children().size() )
			{
				state.canCollapseGroups = false;
			}
		}
		
		IECore::ConstDataPtr getAttribute( const std::string &name )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return 0;
			}
			
			const Context::StateStack &stack = context->stack;
			for( Context::StateStack::const_reverse_iterator it = stack.rbegin(); it!=stack.rend(); it++ )
			{
				CompoundDataMap::const_iterator dIt = it->attributes.find( name );
				if( dIt != it->attributes.end() )
				{
					return dIt->second;
				}
			}
			
			// if the attribute's not defined in the local state, maybe it's defined on the group, or one
			// of its parents?
			return stack.back().group->getAttribute( name );
		}
		
		void light( const std::string &name, const std::string &handle, const CompoundDataMap &parameters )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			Context::State &state = context->stack.back();
			state.lights.push_back( new Light( name, handle, parameters ) );
			if( state.group->children().size() )
			{
				state.canCollapseGroups = false;
			}
		}
		
		void shader( const std::string &type, const std::string &name, const CompoundDataMap &parameters )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			Context::State &state = context->stack.back();
			state.shaders.push_back( new Shader( name, type, parameters ) );
			if( state.group->children().size() )
			{
				state.canCollapseGroups = false;
			}
		}
		
		
		void primitive( PrimitivePtr primitive, const PrimitiveVariableMap &primVars )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			// test current object against object filter option (if specified)
			if( !testFilter() )
			{
				return;
			}
			
			for( PrimitiveVariableMap::const_iterator it=primVars.begin(); it!=primVars.end(); it++ )
			{
				primitive->variables[it->first] = PrimitiveVariable( it->second, true /* deep copy */ );
			}
						
			addChild( context->stack.back(), primitive );
		}
		
		void procedural( Renderer::ProceduralPtr procedural, CapturingRendererPtr renderer )
		{
			ContextPtr context = currentContext();
			if( !context )
			{
				return;
			}
			
			// test current object against object filter option (if specified)
			if( !testFilter() )
			{
				return;
			}
			
			ConstBoolDataPtr reentrant = IECore::runTimeCast<const BoolData>( getAttribute( "cp:procedural:reentrant" ) );
			if ( reentrant ? reentrant->readable() : true )
			{
				ContextPtr proceduralContext = new Context( context.get() );
				addChild( context->stack.back(), proceduralContext->stack.back().group );
				
				if( context == m_mainContext )
				{
					// this is a top level procedural.
					tbb::task *proceduralTask = new( m_topLevelProceduralParent->allocate_additional_child_of( *m_topLevelProceduralParent ) ) ProceduralTask( renderer, procedural, proceduralContext );
					m_topLevelProceduralParent->spawn( *proceduralTask );
				}
				else
				{
					// this is a child of another procedural. 
					tbb::task &parentProceduralTask = tbb::task::self();
					tbb::task *proceduralTask = new( parentProceduralTask.allocate_additional_child_of( parentProceduralTask ) ) ProceduralTask( renderer, procedural, proceduralContext );
					parentProceduralTask.spawn( *proceduralTask ); // the parent procedural will wait for this task in its execute() method
				}				
			}
			else
			{
				// enclose this in an attribute block to prevent leaking:
				attributeBegin();
				procedural->render( renderer.get() );
				attributeEnd();
			}
		}
	
		ConstGroupPtr world()
		{
			if( !m_world )
			{
				throw Exception( "Cannot call world() before worldEnd()." );
			}
			return m_world;
		}

	private :
	
		struct Context : public IECore::RefCounted
		{
			struct State
			{
				State()
					:	group( new Group ), canCollapseGroups( true )
				{
				}
				
				GroupPtr group;
				CompoundDataMap attributes;
				std::vector< ShaderPtr > shaders;
				std::vector<LightPtr> lights;
				M44f localTransform;
				M44f worldTransform;
				bool canCollapseGroups;
			
			};
		
			Context()
			{
				stack.push_back( State() );
			}
			
			Context( const Context *parent )
			{
				stack.push_back( State() );
				State &state = stack.back();
				state.worldTransform = parent->stack.back().worldTransform;
			}
		
			typedef std::vector<State> StateStack;
			
			StateStack stack;
		};
		IE_CORE_DECLAREPTR( Context );
		
		class ProceduralTask : public tbb::task
		{
		
			public :
			
				ProceduralTask( CapturingRendererPtr renderer, Renderer::ProceduralPtr procedural, ContextPtr context )
					:	m_renderer( renderer ), m_procedural( procedural ), m_context( context )
				{			
				}
				
				virtual ~ProceduralTask()
				{
				}
				
				virtual task *execute()
				{					
					ContextStack &contextStack = m_renderer->m_implementation->m_threadContexts.local();
					contextStack.push( m_context );
					
					set_ref_count( 1 );
					
					try
					{
						m_procedural->render( m_renderer.get() );
						wait_for_all();
						m_renderer->m_implementation->collapseGroups( m_context->stack.back() );
					}
					catch( ... )
					{
						contextStack.pop();
						throw;
					}
					
					contextStack.pop();
					
					return 0;
				}
		
			private :
			
				CapturingRendererPtr m_renderer;
				Renderer::ProceduralPtr m_procedural;
				ContextPtr m_context;
		
		};
		
		tbb::task_scheduler_init init; // necessary for tbb < 2.2.
		tbb::task *m_topLevelProceduralParent;
	
		typedef std::stack<ContextPtr> ContextStack;
		typedef tbb::enumerable_thread_specific<ContextStack> ThreadSpecificContext;
		ThreadSpecificContext m_threadContexts;
		Context *m_mainContext; // the one created in worldBegin()

		ContextPtr currentContext()
		{
			ContextStack &stack = m_threadContexts.local();
			if( stack.size() == 0 )
			{
				msg( Msg::Error, "CapturingRenderer::Implementation::currentContext", "No context for this thread." );
				return 0;
			}
			return stack.top();
		}
		
		typedef boost::tokenizer<boost::char_separator<char> > PathTokenizer;
		
		int countTokens( const PathTokenizer& tok ) const
		{
			int count = 0;
			for( PathTokenizer::const_iterator it=tok.begin(); it != tok.end(); ++it )
			{
				++count;
			}
			
			return count;
		}
		
		bool matchToFilter( const std::string& filter, const std::string& name ) const
		{
			
			boost::char_separator<char> sep("/");
			PathTokenizer filterPath( filter, sep );
			PathTokenizer namePath( name, sep );
			
			// if the paths are of differing lengths, this aint a match:
			if( countTokens( filterPath ) > countTokens( namePath ) )
			{
				return false;
			}
			
			PathTokenizer::iterator filterIter = filterPath.begin();
			PathTokenizer::iterator nameIter = namePath.begin();
			
			for( ; filterIter != filterPath.end(); ++filterIter, ++nameIter )
			{
				if( fnmatch( filterIter->c_str(), nameIter->c_str(), 0 ) )
				{
					// this means the tokens don't match: lets quit.
					return false;
				}
				
				// if the last token of the filter is a "*", that means we want to
				// match all children of the filter, so lets just return a positive:
				if( *filterIter == "*" )
				{
					PathTokenizer::iterator filterNext = filterIter;
					++filterNext;

					if( filterNext == filterPath.end() )
					{
						return true;
					}
				}
			}
			
			if( countTokens( filterPath ) < countTokens( namePath ) )
			{
				return false;
			}
			
			// match!!!
			return true;

		}
		
		
		bool matchToParents( const std::string& filter, const std::string& name ) const
		{
			
			boost::char_separator<char> sep("/");
			PathTokenizer filterPath( filter, sep );
			PathTokenizer namePath( name, sep );
			
			// we're expecting the filter path to be longer than the name path.
			// otherwise, "name" can't be a parent of "filter", can it???
			if( countTokens( namePath ) >= countTokens( filterPath ) )
			{
				return false;
			}
			
			PathTokenizer::iterator filterIter = filterPath.begin();
			PathTokenizer::iterator nameIter = namePath.begin();
			
			for( ; nameIter != namePath.end(); ++filterIter, ++nameIter )
			{
				if( fnmatch( filterIter->c_str(), nameIter->c_str(), 0 ) )
				{
					// this means the tokens don't match: lets quit.
					return false;
				}
			}
			
			return true;
		}
		
		bool testFilter()
		{
		
			ConstStringDataPtr name = IECore::runTimeCast<const StringData>( getAttribute( "name" ) );
			
			if( name )
			{
				ConstStringVectorDataPtr objectFilter = IECore::runTimeCast<const StringVectorData>( getOption( "cp:objectFilter" ) );
				
				if( objectFilter )
				{
					
					const std::vector< std::string >& filters = objectFilter->readable();
					
					for( size_t i = 0; i < filters.size(); ++i )
					{
						if( matchToFilter( filters[i], name->readable() ) )
						{
							// if the name directly matches the filter, then yeah, we want to render:
							return true;
						}
						
						if( matchToParents( filters[i], name->readable() ) )
						{
							// if the name could be a parent of the filter, then yeah, we also want to render:
							return true;
						}
					}
					
					return false;
					
				}
			}
			
			return true;
			
		}
		
		
		void addChild( Context::State &state, VisibleRenderablePtr child )
		{
			// at the point we're adding a child, we don't know what will follow in the attribute
			// state after it. attributes might change again and other children might be emitted.
			// we therefore have to wrap the children up in a group containing the current state
			// to insulate them from that possibility. when we're done with a state we can see if
			// the attribute pollution we were worried about is really a problem or not, and promote
			// things out of their little wrapper if possible - we do that in collapseGroups().
		
			GroupPtr wrapper = new Group();
			wrapper->addChild( child );
			
			if( state.attributes.size() )
			{
				AttributeStatePtr wrapperAttributeState = new AttributeState();
				for( CompoundDataMap::const_iterator it = state.attributes.begin(); it!=state.attributes.end(); it++ )
				{
					wrapperAttributeState->attributes()[it->first] = it->second->copy();
				}
				wrapper->addState( wrapperAttributeState );
			}
			if( state.shaders.size() )
			{
				for( std::vector< ShaderPtr >::const_iterator it = state.shaders.begin(); it!=state.shaders.end(); it++ )
				{
					wrapper->addState( (*it)->copy() );
				}
			}
			
			for( std::vector<LightPtr>::const_iterator it = state.lights.begin(); it!=state.lights.end(); it++ )
			{
				wrapper->addState( (*it)->copy() );
			}
			
			if( state.localTransform != M44f() )
			{
				wrapper->setTransform( new MatrixTransform( state.localTransform ) );
			}
			
			state.group->addChild( wrapper );
		}
		
		void collapseGroups( Context::State &state )
		{
			if( !state.canCollapseGroups )
			{
				return;
			}
			
			if( state.attributes.size() )
			{
				AttributeStatePtr attributeState = new AttributeState();
				for( CompoundDataMap::const_iterator it = state.attributes.begin(); it!=state.attributes.end(); it++ )
				{
					attributeState->attributes()[it->first] = it->second->copy();
				}
				state.group->addState( attributeState );
			}
			if( state.shaders.size() )
			{
				for( std::vector< ShaderPtr >::const_iterator it = state.shaders.begin(); it!=state.shaders.end(); it++ )
				{
					state.group->addState( (*it)->copy() );
				}
			}
			
			for( std::vector<LightPtr>::const_iterator it = state.lights.begin(); it!=state.lights.end(); it++ )
			{
				state.group->addState( (*it)->copy() );
			}
				
			if( state.localTransform != M44f() )
			{
				state.group->setTransform( new MatrixTransform( state.localTransform ) );
			}
			
			Group::ChildContainer subGroups = state.group->children();
			state.group->clearChildren();
			
			for( Group::ChildContainer::const_iterator it = subGroups.begin(); it!=subGroups.end(); it++ )
			{
				Group::ChildContainer subGroupChildren = boost::static_pointer_cast<Group>( *it )->children();
				for( Group::ChildContainer::const_iterator it2 = subGroupChildren.begin(); it2!=subGroupChildren.end(); it2++ )
				{
					state.group->addChild( *it2 );
				}
			}
		}
		
		std::map<std::string, ConstDataPtr> m_options;
		GroupPtr m_world;
			
};

//////////////////////////////////////////////////////////////////////////
// CapturingRenderer. this just forwards calls to the implementation
// class.
//////////////////////////////////////////////////////////////////////////

CapturingRenderer::CapturingRenderer()
	:	m_implementation( new Implementation() )
{
}

CapturingRenderer::~CapturingRenderer()
{
}

void CapturingRenderer::setOption( const std::string &name, ConstDataPtr value )
{
	m_implementation->setOption( name, value );
}

ConstDataPtr CapturingRenderer::getOption( const std::string &name ) const
{
	return m_implementation->getOption( name );
}

void CapturingRenderer::camera( const std::string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "CapturingRenderer::camera", "Not implemented" );
}

void CapturingRenderer::display( const std::string &name, const std::string &type, const std::string &data, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "CapturingRenderer::display", "Not implemented" );
}

void CapturingRenderer::worldBegin()
{
	m_implementation->worldBegin();
}

void CapturingRenderer::worldEnd()
{
	m_implementation->worldEnd();
}

void CapturingRenderer::transformBegin()
{
	m_implementation->attributeBegin();
}

void CapturingRenderer::transformEnd()
{
	m_implementation->attributeEnd();
}

void CapturingRenderer::setTransform( const Imath::M44f &m )
{
	m_implementation->setTransform( m );
}

void CapturingRenderer::setTransform( const std::string &coordinateSystem )
{
	msg( Msg::Warning, "CapturingRenderer::setTransform", "Not implemented" );
}

Imath::M44f CapturingRenderer::getTransform() const
{
	return m_implementation->getTransform();
}

Imath::M44f CapturingRenderer::getTransform( const std::string &coordinateSystem ) const
{
	msg( Msg::Warning, "CapturingRenderer::getTransform", "Not implemented" );
	return Imath::M44f();
}

void CapturingRenderer::concatTransform( const Imath::M44f &m )
{
	m_implementation->concatTransform( m );
}

void CapturingRenderer::coordinateSystem( const std::string &name )
{
	msg( Msg::Warning, "CapturingRenderer::coordinateSystem", "Not implemented" );
}

void CapturingRenderer::attributeBegin()
{
	m_implementation->attributeBegin();
}

void CapturingRenderer::attributeEnd()
{
	m_implementation->attributeEnd();
}

void CapturingRenderer::setAttribute( const std::string &name, ConstDataPtr value )
{
	m_implementation->setAttribute( name, value );
}

ConstDataPtr CapturingRenderer::getAttribute( const std::string &name ) const
{
	return m_implementation->getAttribute( name );
}

void CapturingRenderer::shader( const std::string &type, const std::string &name, const CompoundDataMap &parameters )
{
	m_implementation->shader( type, name, parameters );
}

void CapturingRenderer::light( const std::string &name, const std::string &handle, const CompoundDataMap &parameters )
{
	m_implementation->light( name, handle, parameters );
}

void CapturingRenderer::illuminate( const std::string &lightHandle, bool on )
{
	msg( Msg::Warning, "CapturingRenderer::illuminate", "Not implemented" );
}

void CapturingRenderer::motionBegin( const std::set<float> &times )
{
	msg( Msg::Warning, "CapturingRenderer::motionBegin", "Not implemented" );
}

void CapturingRenderer::motionEnd()
{
	msg( Msg::Warning, "CapturingRenderer::motionEnd", "Not implemented" );
}

void CapturingRenderer::points( size_t numPoints, const PrimitiveVariableMap &primVars )
{
	m_implementation->primitive( new PointsPrimitive( numPoints ), primVars );
}

void CapturingRenderer::disk( float radius, float z, float thetaMax, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "CapturingRenderer::disk", "Not implemented" );
}

void CapturingRenderer::curves( const CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	m_implementation->primitive( new CurvesPrimitive( numVertices, basis, periodic ), primVars );
}

void CapturingRenderer::text( const std::string &font, const std::string &text, float kerning, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "CapturingRenderer::text", "Not implemented" );
}

void CapturingRenderer::sphere( float radius, float zMin, float zMax, float thetaMax, const PrimitiveVariableMap &primVars )
{
	m_implementation->primitive( new SpherePrimitive( radius, zMin, zMax, thetaMax ), primVars );
}

void CapturingRenderer::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const PrimitiveVariableMap &primVars )
{
	m_implementation->primitive( new ImagePrimitive( dataWindow, displayWindow ), primVars );
}

void CapturingRenderer::mesh( ConstIntVectorDataPtr vertsPerFace, ConstIntVectorDataPtr vertIds, const std::string &interpolation, const PrimitiveVariableMap &primVars )
{
	m_implementation->primitive( new MeshPrimitive( vertsPerFace, vertIds, interpolation ), primVars );
}

void CapturingRenderer::nurbs( int uOrder, ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const PrimitiveVariableMap &primVars )
{
	m_implementation->primitive( new NURBSPrimitive( uOrder, uKnot, uMin, uMax, vOrder, vKnot, vMin, vMax ), primVars );
}

void CapturingRenderer::patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars )
{
	m_implementation->primitive( new PatchMeshPrimitive( nu, nv, uBasis, vBasis, uPeriodic, vPeriodic ), primVars );
}

void CapturingRenderer::geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "CapturingRenderer::geometry", "Not implemented" );
}

void CapturingRenderer::procedural( ProceduralPtr proc )
{
	m_implementation->procedural( proc, this );
}

void CapturingRenderer::instanceBegin( const std::string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "CapturingRenderer::instanceBegin", "Not implemented" );
}

void CapturingRenderer::instanceEnd()
{
	msg( Msg::Warning, "CapturingRenderer::instanceEnd", "Not implemented" );
}

void CapturingRenderer::instance( const std::string &name )
{
	msg( Msg::Warning, "CapturingRenderer::instance", "Not implemented" );
}

DataPtr CapturingRenderer::command( const std::string &name, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "CapturingRenderer::command", "Not implemented" );
	return 0;
}

void CapturingRenderer::editBegin( const std::string &editType, const CompoundDataMap &parameters )
{
	msg( Msg::Warning, "CapturingRenderer::editBegin", "Not implemented" );
}

void CapturingRenderer::editEnd()
{
	msg( Msg::Warning, "CapturingRenderer::editEnd", "Not implemented" );
}
		
ConstGroupPtr CapturingRenderer::world()
{
	return m_implementation->world();
}
