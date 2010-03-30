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
		virtual ~DeferredRendererImplementation();

		virtual void addCamera( CameraPtr camera );
		virtual void addDisplay( ConstDisplayPtr display );

		virtual void worldBegin();
		virtual void worldEnd();

		virtual void transformBegin();
		virtual void transformEnd();
		virtual void setTransform( const Imath::M44f &m );
		virtual Imath::M44f getTransform() const;
		virtual void concatTransform( const Imath::M44f &matrix );

		virtual void attributeBegin();
		virtual void attributeEnd();

		virtual void addState( StateComponentPtr state );
		virtual StateComponentPtr getState( IECore::TypeId type );

		virtual void addUserAttribute( const IECore::InternedString &name, IECore::DataPtr value );
		virtual IECore::DataPtr getUserAttribute( const IECore::InternedString &name );

		virtual void addPrimitive( PrimitivePtr primitive );

		virtual void procedural( IECore::Renderer::ProceduralPtr proc, IECore::RendererPtr renderer );

		ScenePtr scene();

	private :

		ScenePtr m_scene;

		typedef std::stack<Imath::M44f> TransformStack;
		typedef std::vector<StatePtr> StateStack;
		typedef std::stack<GroupPtr> GroupStack;

		struct RenderContext : public RefCounted
		{
			TransformStack transformStack;
			StateStack stateStack;
			GroupStack groupStack;
		};
		IE_CORE_DECLAREPTR( RenderContext );

		// render context used by renderer outside procedural rendering.		
		RenderContextPtr m_defaultContext;

		typedef tbb::enumerable_thread_specific< std::stack< RenderContextPtr > > ThreadRenderContext;

		// render contexts used by renderer while running procedurals in multiple threads.
		ThreadRenderContext m_threadContextPool;

		// returns at any given thread, the current context ( from procedural or not ).
		RenderContext *currentContext();
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
