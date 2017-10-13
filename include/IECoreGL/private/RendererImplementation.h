//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_RENDERERIMPLEMENTATION_H
#define IECOREGL_RENDERERIMPLEMENTATION_H

#include "IECoreGL/TypeIds.h"

#include "IECore/RunTimeTyped.h"
#include "IECore/InternedString.h"
#include "IECore/Data.h"
#include "IECore/Renderer.h"

#include "OpenEXR/ImathMatrix.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( StateComponent );
IE_CORE_FORWARDDECLARE( Primitive );
IE_CORE_FORWARDDECLARE( Camera );
IE_CORE_FORWARDDECLARE( Display );
IE_CORE_FORWARDDECLARE( Group );

/// RendererImplementation classes are used by the Renderer
/// class to do some of it's work.
class RendererImplementation : public IECore::RunTimeTyped
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( RendererImplementation, RendererImplementationTypeId, IECore::RunTimeTyped )

		RendererImplementation();
		~RendererImplementation() override;

		/// Guaranteed to be called at least once before worldBegin(),
		/// and never after worldBegin().
		virtual void addCamera( CameraPtr camera ) = 0;
		/// Never called after worldBegin().
		virtual void addDisplay( ConstDisplayPtr display ) = 0;

		virtual void worldBegin() = 0;
		virtual void worldEnd() = 0;

		/// Transform calls guaranteed only to be called after worldBegin().
		virtual void transformBegin() = 0;
		virtual void transformEnd() = 0;

		virtual void setTransform( const Imath::M44f &m ) = 0;
		virtual Imath::M44f getTransform() const = 0;
		virtual void concatTransform( const Imath::M44f &matrix ) = 0;

		virtual void attributeBegin() = 0;
		virtual void attributeEnd() = 0;

		virtual void addState( StateComponentPtr state ) = 0;
		virtual StateComponent *getState( IECore::TypeId type ) = 0;
		template <class T>
		T *getState();

		// Set a custom state
		virtual void addUserAttribute( const IECore::InternedString &name, IECore::DataPtr value ) = 0;
		// Get a custom state or 0 if not defined.
		virtual IECore::Data *getUserAttribute( const IECore::InternedString &name ) = 0;
		// Get a casted custom state or 0 if not present or incompatible type.
		template <class T>
		T *getUserAttribute( const IECore::InternedString &name );

		virtual void addPrimitive( ConstPrimitivePtr primitive ) = 0;

		virtual void addProcedural( IECore::Renderer::ProceduralPtr proc, IECore::RendererPtr renderer ) = 0;

		virtual void addInstance( IECoreGL::GroupPtr grp ) = 0;
};

IE_CORE_DECLAREPTR( RendererImplementation );

} // namespace IECoreGL

#include "IECoreGL/private/RendererImplementation.inl"

#endif // IECOREGL_RENDERERIMPLEMENTATION_H
