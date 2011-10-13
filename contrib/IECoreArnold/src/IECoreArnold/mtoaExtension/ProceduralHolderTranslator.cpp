//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "maya/MFnDagNode.h"
#include "maya/MBoundingBox.h"
#include "maya/MPlugArray.h"

#include "extension/Extension.h"
#include "translators/NodeTranslator.h"

#include "IECore/CompoundParameter.h"

#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/IECoreBinding.h"

#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/PythonCmd.h"

class ProceduralHolderTranslator : public CDagTranslator
{

	public :
	
		virtual AtNode *CreateArnoldNodes()
		{
			return AddArnoldNode( "procedural" );
		}
		
		virtual void Export( AtNode *node )
		{
			// do basic node export
			
			ExportMatrix( node, 0 );
			
			AiNodeSetPtr( node, "shader", arnoldShader() );
			
			AiNodeSetInt( node, "visibility", ComputeVisibility() );
			
			MPlug plug = FindMayaObjectPlug( "receiveShadows" );
			if( !plug.isNull() )
			{
				AiNodeSetBool( node, "receive_shadows", plug.asBool() );
			}
			
			plug = FindMayaObjectPlug( "aiSelfShadows" );
			if( !plug.isNull() )
			{
				AiNodeSetBool( node, "self_shadows", plug.asBool() );
			}
			
			plug = FindMayaObjectPlug( "aiOpaque" );
			if( !plug.isNull() )
			{
				AiNodeSetBool( node, "opaque", plug.asBool() );
			}
			
			// now set the procedural-specific parameters
			
			MFnDagNode fnDagNode( m_dagPath );
			MBoundingBox bound = fnDagNode.boundingBox();
			
			AiNodeSetPnt( node, "min", bound.min().x, bound.min().y, bound.min().z );
			AiNodeSetPnt( node, "max", bound.max().x, bound.max().y, bound.max().z );
			
			const char *dsoPath = getenv( "IECOREARNOLD_PROCEDURAL_PATH" );
			AiNodeSetStr( node, "dso", dsoPath ? dsoPath : "ieProcedural.so" );
			
			AiNodeDeclare( node, "className", "constant STRING" );
			AiNodeDeclare( node, "classVersion", "constant INT" );
			AiNodeDeclare( node, "parameterValues", "constant ARRAY STRING" );
			
			// cast should be ok as we're registered to only work on procedural holders
			IECoreMaya::ProceduralHolder *pHolder = static_cast<IECoreMaya::ProceduralHolder *>( fnDagNode.userNode() );
			
			std::string className;
			int classVersion;
			IECore::ParameterisedProceduralPtr procedural = pHolder->getProcedural( &className, &classVersion );
			
			AiNodeSetStr( node, "className", className.c_str() );
			AiNodeSetInt( node, "classVersion", classVersion );
			
			IECorePython::ScopedGILLock gilLock;
			try
			{
				boost::python::object parser = IECoreMaya::PythonCmd::globalContext()["IECore"].attr( "ParameterParser" )();
				boost::python::object serialised = parser.attr( "serialise" )( procedural->parameters() );
				
				size_t numStrings = IECorePython::len( serialised );
				AtArray *stringArray = AiArrayAllocate( numStrings, 1, AI_TYPE_STRING );
				for( size_t i=0; i<numStrings; i++ )
				{
					std::string s = boost::python::extract<std::string>( serialised[i] );
					// hack to workaround ass parsing errors
					/// \todo Remove when we get the Arnold version that fixes this
					for( size_t c = 0; c<s.size(); c++ )
					{
						if( s[c] == '#' )
						{
							s[c] = '@';
						}
					}
					AiArraySetStr( stringArray, i, s.c_str() );
				}
				
				AiNodeSetArray( node, "parameterValues", stringArray );
			}
			catch( boost::python::error_already_set )
			{
				PyErr_Print();
			}
		
		}

		virtual bool RequiresMotionData()
		{
			return IsMotionBlurEnabled( MTOA_MBLUR_OBJECT ) && IsLocalMotionBlurEnabled();
		}
		
		virtual void ExportMotion( AtNode *node, AtUInt step )
		{
   			if( !IsMotionBlurEnabled() )
			{
				return;
			}
			
			ExportMatrix( node, step );
		}
		
		static void nodeInitialiser( CAbTranslator context )
		{
			CExtensionAttrHelper helper( context.maya, "procedural" );
			MakeArnoldVisibilityFlags( helper );
			
			helper.MakeInput( "self_shadows" );
			helper.MakeInput( "opaque" );

		}

		static void *creator()
		{
			return new ProceduralHolderTranslator();
		}
	
	protected :
	
		/// Returns the arnold shader assigned to the procedural. This duplicates
		/// code in GeometryTranslator.h, but there's not much can be done about that
		/// since the GeometryTranslator isn't part of the MtoA public API.
		AtNode *arnoldShader()
		{
			MFnDependencyNode fnDN( m_dagPath.node() );
			
			// get the shading group
			
			MPlug instObjGroups = fnDN.findPlug( "instObjGroups" );
			unsigned instNumber = m_dagPath.isInstanced() ? m_dagPath.instanceNumber() : 0;
			
			MPlugArray connections;
			instObjGroups.elementByLogicalIndex( instNumber ).connectedTo( connections, false, true );
			
			MObject shadingGroup;
			for( unsigned i=0; i<connections.length(); i++ )
			{
				MObject connectedNode = connections[i].node();
				if( connectedNode.apiType() == MFn::kShadingEngine )
				{
					shadingGroup = connectedNode;
				}
			}
			
			if( shadingGroup.isNull() )
			{
				return 0;
			}
			
			// get the surface shader from it
			
			fnDN.setObject( shadingGroup );
			MPlug shaderPlug = fnDN.findPlug( "surfaceShader" );
			
			connections.clear();
			shaderPlug.connectedTo( connections, true, false );
			if( !connections.length() )
			{
				return 0;
			}
			
			MObject shader = connections[0].node();
			return m_session->ExportNode( shader );
		}

};

extern "C"
{

DLLEXPORT void initializeExtension( CExtension &extension )
{
	extension.Requires( "ieCore" );
	extension.RegisterTranslator(
		"ieProceduralHolder",
		"",
		ProceduralHolderTranslator::creator,
		ProceduralHolderTranslator::nodeInitialiser
	);
}

DLLEXPORT void deinitializeExtension( CExtension &extension )
{
}

} // extern "C"


