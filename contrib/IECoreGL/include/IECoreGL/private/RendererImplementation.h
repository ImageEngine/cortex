#ifndef IECOREGL_RENDERERIMPLEMENTATION_H
#define IECOREGL_RENDERERIMPLEMENTATION_H

#include "IECoreGL/TypeIds.h"

#include "IECore/RunTimeTyped.h"

#include "OpenEXR/ImathMatrix.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( StateComponent );
IE_CORE_FORWARDDECLARE( Primitive );
IE_CORE_FORWARDDECLARE( Camera );
IE_CORE_FORWARDDECLARE( Display );

/// RendererImplementation classes are used by the Renderer
/// class to do some of it's work.
class RendererImplementation : public IECore::RunTimeTyped
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( RendererImplementation, RendererImplementationTypeId, IECore::RunTimeTyped )

		RendererImplementation();
		virtual ~RendererImplementation();

		virtual void addCamera( CameraPtr camera ) = 0;
		virtual void addDisplay( ConstDisplayPtr display ) = 0;

		virtual void worldBegin() = 0;
		virtual void worldEnd() = 0;

		virtual void transformBegin() = 0;
		virtual void transformEnd() = 0;
		virtual void concatTransform( const Imath::M44f &matrix ) = 0;
		
		virtual void attributeBegin() = 0;
		virtual void attributeEnd() = 0;
		
		virtual void addState( StateComponentPtr state ) = 0;
		virtual StateComponentPtr getState( IECore::TypeId type ) = 0;
		template <class T>
		typename T::Ptr getState();

		virtual void addPrimitive( PrimitivePtr primitive ) = 0;

};

IE_CORE_DECLAREPTR( RendererImplementation );

} // namespace IECoreGL

#include "IECoreGL/private/RendererImplementation.inl"

#endif // IECOREGL_RENDERERIMPLEMENTATION_H
