#ifndef IECOREGL_IMMEDIATERENDERERIMPLEMENTATION_H
#define IECOREGL_IMMEDIATERENDERERIMPLEMENTATION_H

#include "IECoreGL/private/RendererImplementation.h"

#include <stack>
#include <vector>

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( Camera );
IE_CORE_FORWARDDECLARE( FrameBuffer );
IE_CORE_FORWARDDECLARE( Display );

class ImmediateRendererImplementation : public RendererImplementation
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ImmediateRendererImplementation, ImmediateRendererImplementationTypeId, RendererImplementation );

		ImmediateRendererImplementation();
		virtual ~ImmediateRendererImplementation();

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

	private :

		CameraPtr m_camera;
		FrameBufferPtr m_frameBuffer;

		std::vector<ConstDisplayPtr> m_displays;

		typedef std::stack<StatePtr> StateStack;
		StateStack m_stateStack;

};

IE_CORE_DECLAREPTR( ImmediateRendererImplementation );

} // namespace IECoreGL

#endif // IECOREGL_IMMEDIATERENDERERIMPLEMENTATION_H
