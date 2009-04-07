//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_PARAMETERISEDPROCEDURAL_H
#define IECORE_PARAMETERISEDPROCEDURAL_H

#include "IECore/VisibleRenderable.h"
#include "IECore/ParameterisedInterface.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( CompoundObject )

/// The ParameterisedProcedural class provides a means of implementing
/// a Renderer::Procedural at a slightly higher level. By deriving from
/// VisibleRenderable it allows procedurals to be embedded in groups with
/// other geometry and state and by deriving from ParameterisedInterface
/// it provides a consistent way of providing parameter values to subclasses.
class ParameterisedProcedural : public VisibleRenderable, public ParameterisedInterface
{
	public:
	
		ParameterisedProcedural();
		virtual ~ParameterisedProcedural();
				
		IE_CORE_DECLAREABSTRACTOBJECT( ParameterisedProcedural, VisibleRenderable );
				
		/// Calls Renderer::procedural using doBound() and doRender() to
		/// implement the bound() and render() methods required by the
		/// Renderer.
		virtual void render( RendererPtr renderer ) const;
		/// Forwards to doBound().
		virtual Imath::Box3f bound() const;

		virtual CompoundParameterPtr parameters();
		virtual ConstCompoundParameterPtr parameters() const;

	protected :
	
		/// Must be implemented by derived classes - the contents of args is
		/// guaranteed to have been validated.
		virtual Imath::Box3f doBound( ConstCompoundObjectPtr args ) const = 0;
		/// Must be implemented by derived classes - the contents of args is
		/// guaranteed to have been validated.
		virtual void doRender( RendererPtr renderer, ConstCompoundObjectPtr args ) const = 0;
			
	private:
	
		// Implements Renderer::Procedural to forward to one of these.
		class Forwarder;
		friend class Forwarder;
		
		CompoundParameterPtr m_parameters;
		
		static const unsigned int m_ioVersion;
		
};

IE_CORE_DECLAREPTR( ParameterisedProcedural );

}

#endif // IECORE_PARAMETERISEDPROCEDURAL_H
