//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include <map>

#include "OpenEXR/ImathMatrix.h"

#include "IECoreGL/IECoreGL.h"

#include "IECoreMaya/ParameterisedHolder.h"

#include "IECore/ParameterisedProcedural.h"
#include "IECore/Interned.h"

#include "maya/MPxComponentShape.h"
#include "maya/MArrayDataBuilder.h"

namespace IECoreGL
{
IE_CORE_FORWARDDECLARE( Scene );
IE_CORE_FORWARDDECLARE( State );
IE_CORE_FORWARDDECLARE( BoxPrimitive );
IE_CORE_FORWARDDECLARE( Primitive );
IE_CORE_FORWARDDECLARE( Group );
IE_CORE_FORWARDDECLARE( NameStateComponent );
IE_CORE_FORWARDDECLARE( Renderer );
}

namespace IECoreMaya
{

/// The ProceduralHolder class represents implementation of the IECore::Renderer::Procedural
/// class, presenting the procedural parameters as maya attributes. It also draws a bounding
/// box for the procedural in the scene.
class ProceduralHolder : public ParameterisedHolderComponentShape
{
	friend class ProceduralHolderUI;

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
		virtual MStatus compute( const MPlug &plug, MDataBlock &dataBlock );
		
		virtual void componentToPlugs( MObject &component, MSelectionList &selectionList ) const;
		virtual MatchResult matchComponent( const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list );


		virtual MStatus setParameterised( IECore::RunTimeTypedPtr p );
		virtual MStatus updateParameterised();
		virtual IECore::RunTimeTypedPtr getParameterised( std::string *classNameOut, int *classVersionOut, std::string *searchPathEnvVarOut = 0 );


		/// Calls setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS" ).
		MStatus setProcedural( const std::string &className, int classVersion );
		/// Returns runTimeCast<ParameterisedProcedural>( getProcedural( className, classVersion ) ).
		IECore::ParameterisedProceduralPtr getProcedural( std::string *className = 0, int *classVersion = 0 );
		/// creates a user attribute called "useDisplayLists" if requested
		MStatus createDisplaylistAttribute();
		
		/// Returns an up to date scene from the procedural
		IECoreGL::ConstScenePtr scene();
		
		/// Returns the number of times the opengl scene has been regenerated
		int getSceneUpdateCount();
		
		/// Returns whether or not to draw using display lists
		bool useDisplayLists();
		
		static MObject aGLPreview;
		static MObject aCulling;
		static MObject aTransparent;
		static MObject aDrawBound;
		static MObject aDrawCoordinateSystems;
		static MObject aProceduralComponents;
		
		static MObject aComponentQueries;
		
		static MObject aComponentTransform;
		static MObject aComponentTranslate;
		static MObject aComponentTranslateX;
		static MObject aComponentTranslateY;
		static MObject aComponentTranslateZ;
		static MObject aComponentRotate;
		static MObject aComponentRotateX;
		static MObject aComponentRotateY;
		static MObject aComponentRotateZ;
		static MObject aComponentScale;
		static MObject aComponentScaleX;
		static MObject aComponentScaleY;
		static MObject aComponentScaleZ;
		
		static MObject aComponentBound;
		static MObject aComponentBoundMin;
		static MObject aComponentBoundMinX;
		static MObject aComponentBoundMinY;
		static MObject aComponentBoundMinZ;
		static MObject aComponentBoundMax;
		static MObject aComponentBoundMaxX;
		static MObject aComponentBoundMaxY;
		static MObject aComponentBoundMaxZ;
		static MObject aComponentBoundCenter;
		static MObject aComponentBoundCenterX;
		static MObject aComponentBoundCenterY;
		static MObject aComponentBoundCenterZ;

	private :

		mutable bool m_boundDirty;
		mutable MBoundingBox m_bound;

		bool m_sceneDirty;
		IECoreGL::ScenePtr m_scene;
		IECoreGL::RendererPtr m_lastRenderer;
		
		/// \todo Use a boost::multi_index jobby to replace both these and the ClassData. It could store a nice
		/// struct with named fields instead of the hard to understand std::pairs.		
		typedef std::map<IECore::InternedString,  std::pair< unsigned int, IECoreGL::GroupPtr > > ComponentsMap;
		typedef std::map< int, std::set< std::pair< std::string, IECoreGL::GroupPtr > > > ComponentToGroupMap;
		typedef std::map<IECore::InternedString, Imath::M44f> ComponentTransformsMap;

		void buildComponents();
		void buildComponents( IECoreGL::ConstNameStateComponentPtr nameState, IECoreGL::GroupPtr group, const Imath::M44f &parentTransform );
		
		int m_sceneUpdateCount;
		
		ComponentsMap m_componentsMap;
		ComponentToGroupMap m_componentToGroupMap;
		ComponentTransformsMap m_componentTransforms;

};

} // namespace IECoreMaya

#endif // IECOREMAYA_PROCEDURALHOLDER_H
