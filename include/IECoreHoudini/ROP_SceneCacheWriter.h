//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_ROPSCENECACHEWRITER_H
#define IECOREHOUDINI_ROPSCENECACHEWRITER_H

#include "ROP/ROP_Node.h"

#include "IECore/SceneInterface.h"

#include "IECoreHoudini/SceneCacheNode.h"

namespace IECoreHoudini
{

/// Class for writing SceneCache files based on an existing Houdini hierarchy
class ROP_SceneCacheWriter : public ROP_Node
{
	public :
		
		ROP_SceneCacheWriter( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~ROP_SceneCacheWriter();
		
		static const char *typeName;
		
		static PRM_Name pFile;
		static PRM_Name pRootObject;
		static PRM_Name pForceObjects;
		
		static PRM_Default fileDefault;
		static PRM_Default rootObjectDefault;
		static PRM_SpareData forceObjectsSpareData;
		
		static OP_Node *create( OP_Network *net, const char *name, OP_Operator *op );
		static OP_TemplatePair *buildParameters();
	
	protected :
		
		virtual int startRender( int nframes, fpreal s, fpreal e );
		virtual ROP_RENDER_CODE renderFrame( fpreal time, UT_Interrupt *boss );
		virtual ROP_RENDER_CODE endRender();
		
		virtual bool updateParmsFlags();
		
		/// Called recursively to traverse the IECoreHoudini::LiveScene, starting with the Root Object,
		/// and write the hierarchy to the output file.
		virtual ROP_RENDER_CODE doWrite( const IECore::SceneInterface *liveScene, IECore::SceneInterface *outScene, double time, UT_Interrupt *progress );
	
	private :
		
		static const IECore::SceneInterface::Name &changingHierarchyAttribute;

		bool linked( const std::string &file ) const;
		
		enum Mode
		{
			NaturalLink = 0,
			ForcedLink,
			NaturalExpand,
			ForcedExpand
		};
		
		IECoreHoudini::LiveScenePtr m_liveHoudiniScene;
		IECore::ConstSceneInterfacePtr m_liveScene;
		IECore::SceneInterfacePtr m_outScene;
		UT_StringMMPattern *m_forceFilter;
		
		double m_startTime;
		double m_endTime;

};

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_ROPSCENECACHEWRITER_H
