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

#ifndef IECOREGL_IMMEDIATERENDERERIMPLEMENTATION_H
#define IECOREGL_IMMEDIATERENDERERIMPLEMENTATION_H

#include <stack>
#include <vector>

#include "IECoreGL/private/RendererImplementation.h"
#include "IECoreGL/FrameBuffer.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( Camera );
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
		virtual void setTransform( const Imath::M44f &m );
		virtual Imath::M44f getTransform() const;
		virtual void concatTransform( const Imath::M44f &matrix );

		virtual void attributeBegin();
		virtual void attributeEnd();

		virtual void addState( StateComponentPtr state );
		virtual StateComponent *getState( IECore::TypeId type );

		virtual void addUserAttribute( const IECore::InternedString &name, IECore::DataPtr value );
		virtual IECore::Data *getUserAttribute( const IECore::InternedString &name );

		virtual void addPrimitive( ConstPrimitivePtr primitive );

		virtual void addProcedural( IECore::Renderer::ProceduralPtr proc, IECore::RendererPtr renderer );

		virtual void addInstance( GroupPtr grp );

	private :

		CameraPtr m_camera;
		FrameBufferPtr m_frameBuffer;
		FrameBuffer::ScopedBinding *m_frameBufferBinding;

		std::vector<ConstDisplayPtr> m_displays;

		typedef std::stack<StatePtr> StateStack;
		StateStack m_stateStack;

};

IE_CORE_DECLAREPTR( ImmediateRendererImplementation );

} // namespace IECoreGL

#endif // IECOREGL_IMMEDIATERENDERERIMPLEMENTATION_H
