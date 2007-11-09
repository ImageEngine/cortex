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

#ifndef IECOREGL_STATECOMPONENT_H
#define IECOREGL_STATECOMPONENT_H

#include "IECoreGL/State.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( StateComponent );

/// \todo We shouldn't be using TypeId objects for the StateComponent identifiers - we should
/// use summink else (we need to have a Shader and a FixedShader component that both occupy the
/// same slot).
class StateComponent : public Bindable
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( StateComponent, StateComponentTypeId, Bindable );
		
		virtual ~StateComponent();

	protected :

		StateComponent();
		
		template<typename T>
		class Description
		{
			public :
				Description();
			private :
				static StateComponentPtr creator();
		};

};

IE_CORE_DECLAREPTR( StateComponent );

} // namespace IECoreGL

#include "IECoreGL/StateComponent.inl"

#endif // IECOREGL_STATECOMPONENT_H
