#ifndef IECOREGL_DEFERREDRENDERERIMPLEMENTATION_H
#define IECOREGL_DEFERREDRENDERERIMPLEMENTATION_H

#include "IECoreGL/private/RendererImplementation.h"

#include <stack>
#include <vector>

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
		virtual void concatTransform( const Imath::M44f &matrix );
		
		virtual void attributeBegin();
		virtual void attributeEnd();
		
		virtual void addState( StateComponentPtr state );
		virtual StateComponentPtr getState( IECore::TypeId type );

		virtual void addPrimitive( PrimitivePtr primitive );

		ScenePtr scene();
				
	private :

		ScenePtr m_scene;
		
		typedef std::stack<Imath::M44f> TransformStack;
		TransformStack m_transformStack;
		
		typedef std::vector<StatePtr> StateStack;
		StateStack m_stateStack;
		
		typedef std::stack<GroupPtr> GroupStack;
		GroupStack m_groupStack;
		
};

IE_CORE_DECLAREPTR( DeferredRendererImplementation );

} // namespace IECoreGL

#endif // IECOREGL_DEFERREDRENDERERIMPLEMENTATION_H
