//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_SOPPROCEDURALHOLDER_H
#define IECOREHOUDINI_SOPPROCEDURALHOLDER_H

#ifdef IECOREHOUDINI_WITH_GL

	#include "IECoreGL/Scene.h"

#endif

#include "IECoreHoudini/SOP_ParameterisedHolder.h"

namespace IECoreHoudini
{

/// SOP class for representing a IECore::ParameterisedProcedural in Houdini
class SOP_ProceduralHolder : public SOP_ParameterisedHolder
{
	public :

		/// standard houdini ctor and parameter variables
		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );

#ifdef IECOREHOUDINI_WITH_GL

		/// returns a GL scene, rendering it if necessary
		IECoreGL::ConstScenePtr scene();

#endif

	protected :

		SOP_ProceduralHolder( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~SOP_ProceduralHolder();

		virtual OP_ERROR cookMySop( OP_Context &context );

	private :

#ifdef IECOREHOUDINI_WITH_GL

		// our cache GL scene
		IECoreGL::ScenePtr m_scene;

#endif

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_SOPPROCEDURALHOLDER_H
