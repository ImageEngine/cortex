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

#ifndef IENUKE_SCENECACHEREADER_H
#define IENUKE_SCENECACHEREADER_H

#include "DDImage/SourceGeo.h"
#include "DDImage/Knobs.h"
#include "DDImage/Row.h"
#include "DDImage/Filter.h"
#include "IECore/LensModel.h"

#include "DDImage/Op.h"
#include "DDImage/Matrix4.h"
#include "DDImage/ViewerContext.h"

#include "IECoreGL/Scene.h"
#include "IECoreGL/Renderer.h"

#include "IECoreScene/SceneInterface.h"

#include "IECoreNuke/Export.h"

#include "OpenEXR/ImathMatrix.h"

namespace IECoreNuke
{

/// Loads and displays geometry from a scene cache file.
class IECORENUKE_API SceneCacheReader : public DD::Image::SourceGeo
{
	public :

		SceneCacheReader( Node *node );
		~SceneCacheReader();

		virtual void knobs( DD::Image::Knob_Callback f );
		virtual const char *Class() const;
		virtual const char *node_help() const;

	protected :

		virtual void _validate( bool forReal );
		virtual void append( DD::Image::Hash &hash );
		virtual void get_geometry_hash();
		virtual void geometry_engine( DD::Image::Scene& scene, DD::Image::GeometryList &out );
	 	virtual void create_geometry( DD::Image::Scene &scene, DD::Image::GeometryList &out );
		virtual int knob_changed( DD::Image::Knob* k);

	private :

		static const DD::Image::Op::Description m_description;
		static DD::Image::Op *build( Node *node );
		virtual void build_handles( DD::Image::ViewerContext* ctx);

		//! @name Accessors
		/// These methods return useful information about the state of the item and
		/// the items that it represents.
		//////////////////////////////////////////////////////////////
		//@{
		/// Returns the name of the currently selected tag filter.
		void tagSelection( std::string &selection ) const;
		// Returns whether the item at index itemIndex is selected in the SceneView_knob.
		inline bool itemSelected( int itemIndex ) const;
		// Returns the name of the item at index itemIndex in the SceneView_knob.
		const std::string &itemName( int itemIndex ) const;
		// Returns a SceneInterface for the root item.
		IECoreScene::ConstSceneInterfacePtr getSceneInterface();
		// Returns a SceneInterface for the item at path.
		IECoreScene::ConstSceneInterfacePtr getSceneInterface( const std::string &path );
		//@}

		//! @name Methods which control the SceneView_knob
		/// These methods populate the SceneView_knob with the names or the items
		/// taken from the SceneCache, filter and select them. They are called
		/// from the knob_changed() method to synchronize the SceneView_knob and
		/// the internal lists of selected and filtered items.
		/// These methods should only be called from the firstReader() returned instance.
		//////////////////////////////////////////////////////////////
		//@{
		/// Loads the internal data structures from the knobs and set up the
		/// SceneView_knob. This method is called when a item is loaded from a script
		/// or pasted.
		void loadAllFromKnobs();
		/// Rebuilds the SceneView_knob from the file and root specified in the knobs.
		/// If the file or root have not changed then the no work is done. If the scene
		/// is rebuilt then the selection will be lost. The filterScene() method must
		/// be called immediately afterwards.
		void rebuildSceneView();
		/// Rebuilds the sceneView to show only items which are already selected or have
		/// names that matches filterText and a tag which matches tagText.
		/// Passing an empty string to either filterText or tagText will disable the
		/// filtering of the names and tags respectively. This should be called
		/// immediately after any call to rebuildSceneView().
		void filterScene( const std::string &filterText, const std::string &tagText, bool keepSelection = true );
		/// Clear any selected geometry from the SceneView_knob.
		void clearSceneViewSelection();
		//@}

		/// Updates the Enumeration_knob of available tags from the internal list of tags
		/// and updates the currently selected tag to ensure that it is valid.
		void updateTagFilterKnob();
		/// Loads a primitive from the scene cache and adds it to the GeometryList.
		void loadPrimitive( DD::Image::GeometryList &out, const std::string &path );
		/// Get the hash of the file path and root knob.
		DD::Image::Hash sceneHash() const;
		/// Get the hash of the SceneView knob (the default hash implementation of that knob returns a constant hash...)
		DD::Image::Hash selectionHash( bool force = false ) const;

		Imath::M44d worldTransform( IECoreScene::ConstSceneInterfacePtr scene, IECoreScene::SceneInterface::Path root, double time );

		// uses firstOp to return the Op that has the up-to-date private data
		SceneCacheReader *firstReader();
		const SceneCacheReader *firstReader() const;

		class SharedData;

		// this function should only be called from the firstReader() object.
		SharedData *sharedData();
		const SharedData *sharedData() const;

		// Knob Members.
		const char *m_filePath; // Holds the raw SceneCache file path.
		std::string m_root; // Holds the root item in the SceneCache.
		std::string m_filter; // The text to filter the scene with.
		bool m_worldSpace; // Set to ignore local transforms..
		DD::Image::Matrix4 m_baseParentMatrix; // The global matrix that is applied to the geo.

		// Pointers to various knobs.
		DD::Image::Knob *m_filePathKnob;
		DD::Image::Knob *m_baseParentMatrixKnob;
		DD::Image::Knob *m_sceneKnob;
		DD::Image::Knob *m_tagFilterKnob;
		DD::Image::Knob *m_sceneFilterKnob;
		DD::Image::Knob *m_rootKnob;

		// only the first reader allocates the shared data
		SharedData *m_data;
};

} // namespace IECoreNuke

#endif // IENUKE_SCENECACHEREADER_H

