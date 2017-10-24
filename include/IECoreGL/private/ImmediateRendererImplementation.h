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
		~ImmediateRendererImplementation() override;

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

		void addProcedural( IECore::Renderer::ProceduralPtr proc, IECore::RendererPtr renderer ) override;

		void addInstance( GroupPtr grp ) override;

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
