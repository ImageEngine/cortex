//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_BINDABLE_H
#define IECOREGL_BINDABLE_H

#include "IECoreGL/TypeIds.h"
#include "IECoreGL/GL.h"

#include "IECore/RunTimeTyped.h"

#include "boost/utility.hpp"

namespace IECoreGL
{

/// The Bindable class provides an abstract base class
/// for all classes which can bind to OpenGL, modifying
/// the state in some way.
class Bindable : public IECore::RunTimeTyped, boost::noncopyable
{

	public :

		Bindable();
		virtual ~Bindable();
		
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Bindable, BindableTypeId, IECore::RunTimeTyped );
		
		/// Binds the object into the current OpenGL state in whatever way
		/// is relevant to that object (install a shader, make a texture current, enable/disable
		/// something etc.).
		virtual void bind() const = 0;
		/// Returns the bitmask that would have to be used with glPushAttrib() in order to
		/// save the state that will be modified by bind().
		virtual GLbitfield mask() const = 0;

};

IE_CORE_DECLAREPTR( Bindable );

} // namespace IECoreGL

#endif // IECOREGL_BINDABLE_H
