//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#ifndef SOP_PROCEDURALHOLDER_H_
#define SOP_PROCEDURALHOLDER_H_

// Houdini
#include <SOP/SOP_Node.h>
#include <PRM/PRM_Name.h>

// Cortex
#include <IECore/Parameterised.h>
#include <IECore/ClassData.h>

// IECoreHoudini
#include "SOP_ParameterisedHolder.h"

// IECoreGL
#include "IECoreGL/Scene.h"

namespace IECoreHoudini
{
	/// SOP class for representing a IECore::ParameterisedProcedural
	/// in Houdini. Inherits directly from SOP_ParameterisedHolder
	/// and is visualised by the GR_Cortex render hook.
	class SOP_ProceduralHolder : public SOP_ParameterisedHolder
	{
		friend class GR_Cortex;
		public:
			/// standard houdini ctor and parameter variables
			static OP_Node *myConstructor( OP_Network *net,
					const char *name,
					OP_Operator *op );
			static PRM_Template myParameters[];
			static CH_LocalVariable myVariables[];

			/// static members for our non-dynamic parameters
			static PRM_Name opTypeParm;
			static PRM_Name opVersionParm;
			static PRM_Name opParmEval;
			static PRM_Name opMatchString;
			static PRM_Default opMatchStringDefault;
			static PRM_Name opReloadBtn;
			static PRM_Name switcherName;
			static PRM_Default switcherDefaults[];
			static PRM_ChoiceList typeMenu;
			static PRM_ChoiceList versionMenu;

			/// build dynamic procedural menu
			static void buildTypeMenu( void *data, PRM_Name *menu, int maxSize,
					const PRM_SpareData *, PRM_Parm * );
			static void buildVersionMenu( void *data, PRM_Name *menu, int maxSize,
					const PRM_SpareData *, PRM_Parm * );

			/// callback for when the type/version parameter changes
			static int reloadClassCallback( void *data, int index, float time,
					const PRM_Template *tplate);

			/// callback for when we click the reload button
			static int reloadButtonCallback( void *data, int index, float time,
					const PRM_Template *tplate);

			/// class names based on match string
			virtual void refreshClassNames();

			/// handle loading our SOP from disk (i.e. when a hip is loaded)
			virtual bool load( UT_IStream &is, const char *ext,
					const char *path );

			/// creates and sets a particular type/version of class on this sop
			void loadProcedural( const std::string &type, int version, bool update_gui=true );

			virtual void setParameterised( IECore::RunTimeTypedPtr p, const std::string &type, int version );

            /// returns a GL scene, rendering it if necessary
            IECoreGL::ConstScenePtr scene();

            /// mark this procedural's scene as dirty
            void dirty(){ m_renderDirty = true; }
            bool isDirty(){ return m_renderDirty; }

		protected:
			SOP_ProceduralHolder( OP_Network *net,
					const char *name,
					OP_Operator *op );
			virtual ~SOP_ProceduralHolder();
			virtual OP_ERROR cookMySop( OP_Context &context );

		private:
			// our cache GL scene
			IECoreGL::ScenePtr m_scene;
    		bool m_renderDirty;

			// cache the procedural names
    		std::string m_matchString;
			std::vector<std::string> m_cachedProceduralNames;
	};

} // namespace IECoreHoudini

#endif /* SOP_PROCEDURALHOLDER_H_ */
