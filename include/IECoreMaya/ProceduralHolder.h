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

#ifndef IECOREMAYA_PROCEDURALHOLDER_H
#define IECOREMAYA_PROCEDURALHOLDER_H

#include "IECoreMaya/ParameterisedHolder.h"

#include "IECore/Renderer.h"

#include "maya/MPxComponentShape.h"

namespace IECoreGL
{
IE_CORE_FORWARDDECLARE( Scene );
IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( BoxPrimitive );
}

namespace IECoreMaya
{

/// \addtogroup environmentgroup
///
/// <b>IECORE_PROCEDURAL_PATHS</b><br>
/// This environment variable is used to find procedural classes to be held by the ProceduralHolder
/// node.

/// The ProceduralHolder class represents implementation of the IECore::Renderer::Procedural
/// class, presenting the procedural parameters as maya attributes. It also draws a bounding
/// box for the procedural in the scene.
/// \todo Implement component selection in some useful way - this will require support from IECoreGL.
class ProceduralHolder : public ParameterisedHolderComponentShape
{

	public :
	
		ProceduralHolder();
		virtual ~ProceduralHolder();

		virtual void postConstructor();

		static void *creator();				
		static MStatus initialize();		
		static MTypeId id;
		
		virtual bool isBounded() const;
		virtual MBoundingBox boundingBox() const;
		virtual MStatus setDependentsDirty( const MPlug &plug, MPlugArray &plugArray );

		/// Calls setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS" ).
		MStatus setProcedural( const std::string &className, int classVersion );
		/// Returns runTimeCast<ParameterisedProcedural>( getProcedural( className, classVersion ) ). 
		IECore::Renderer::ProceduralPtr getProcedural( std::string *className = 0, int *classVersion = 0 );
	
		/// Returns an up to date scene from the procedural
		IECoreGL::ConstScenePtr scene();
		
		static MObject aGLPreview;
		static MObject aTransparent;
		static MObject aDrawBound;
	
	private :
	
		mutable bool m_boundDirty;
		mutable MBoundingBox m_bound;
		
		bool m_sceneDirty;
		IECoreGL::ScenePtr m_scene;
		/// \todo This can probably be merged with scene()
		void updateScene();
				
};

} // namespace IECoreMaya

#endif // IECOREMAYA_PROCEDURALHOLDER_H
