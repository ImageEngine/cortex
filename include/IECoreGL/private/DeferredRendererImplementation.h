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

#ifndef IECOREGL_DEFERREDRENDERERIMPLEMENTATION_H
#define IECOREGL_DEFERREDRENDERERIMPLEMENTATION_H

#include "IECoreGL/private/RendererImplementation.h"

#include <stack>
#include <vector>

#include "tbb/enumerable_thread_specific.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Scene );
IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( Group );

class DeferredRendererImplementation : public RendererImplementation
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( DeferredRendererImplementation, DeferredRendererImplementationTypeId, RendererImplementation );

		DeferredRendererImplementation();
		~DeferredRendererImplementation() override;

		void addCamera( CameraPtr camera ) override;
		void addDisplay( ConstDisplayPtr display ) override;

		void worldBegin() override;
		void worldEnd() override;

		void transformBegin() override;
		void transformEnd() override;
		void setTransform( const Imath::M44f &m ) override;
		Imath::M44f getTransform() const override;
		void concatTransform( const Imath::M44f &matrix ) override;

		void attributeBegin() override;
		void attributeEnd() override;

		void addState( StateComponentPtr state ) override;
		StateComponent *getState( IECore::TypeId type ) override;

		void addUserAttribute( const IECore::InternedString &name, IECore::DataPtr value ) override;
		IECore::Data *getUserAttribute( const IECore::InternedString &name ) override;

		void addPrimitive( ConstPrimitivePtr primitive ) override;

		void addProcedural( IECoreScene::Renderer::ProceduralPtr proc, IECoreScene::RendererPtr renderer ) override;

		void addInstance( GroupPtr grp ) override;

		ScenePtr scene();

	private :

		ScenePtr m_scene;

		typedef std::stack<Imath::M44f> TransformStack;
		typedef std::vector<StatePtr> StateStack;
		typedef std::stack<GroupPtr> GroupStack;

		struct RenderContext : public RefCounted
		{
			// relative transformation from top of transformStack to current renderer state.
			Imath::M44f localTransform;
			// stack of world space matrices
			TransformStack transformStack;
			// stack of incomplete states
			StateStack stateStack;
			// stack of groups being built
			GroupStack groupStack;
		};
		IE_CORE_DECLAREPTR( RenderContext );

		// render context used by renderer outside procedural rendering.
		RenderContextPtr m_defaultContext;

		typedef tbb::enumerable_thread_specific< std::stack< RenderContextPtr > > ThreadRenderContext;

		// render contexts used by renderer while running procedurals in multiple threads.
		mutable ThreadRenderContext m_threadContextPool;

		// returns at any given thread, the current context ( from procedural or not ).
		RenderContext *currentContext();
		const RenderContext *currentContext() const;
		// push method for procedural's context
		void pushContext( RenderContextPtr context );
		// pop method for procedural's context
		RenderContextPtr popContext();

		class ProceduralTask;
		struct ScopedRenderContext;
};

IE_CORE_DECLAREPTR( DeferredRendererImplementation );

} // namespace IECoreGL

#endif // IECOREGL_DEFERREDRENDERERIMPLEMENTATION_H
