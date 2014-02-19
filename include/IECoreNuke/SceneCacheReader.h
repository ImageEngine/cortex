//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SceneInterface.h"

#include "OpenEXR/ImathMatrix.h"

namespace IECoreNuke
{

/// Loads and displays geometry from a scene cache file.
class SceneCacheReader : public DD::Image::SourceGeo
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
		IECore::ConstSceneInterfacePtr getSceneInterface() const;
		// Returns a SceneInterface for the item at path.
		IECore::ConstSceneInterfacePtr getSceneInterface( const std::string &path ) const;
		//@}

		//! @name Methods which control the SceneView_knob
		/// These methods populate the SceneView_knob with the names or the items
		/// taken from the SceneCache, filter and select them. They are called
		/// from the knob_changed() method to synchronize the SceneView_knob and
		/// the internal lists of selected and filtered items.
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
		/// This recursive method is called from rebuildSceneView() and traverses the
		/// sceneInterface to build a list of item names and a mapping of the tags to 
		/// the indices in the items.
		void buildSceneView( std::vector< std::string > &list, const IECore::ConstSceneInterfacePtr sceneInterface );
		/// Rebuilds the sceneView to show only items which are already selected or have
		/// a names that matches filterText and a tag which matches tagText.
		/// Passing an empty string to either filterText or tagText will disable the
		/// filtering of the names and tags respectively. This should be called
		/// immediately after any call to rebuildSceneView().
		void filterScene( const std::string &filterText, const std::string &tagText );
		/// Rebuilds the current geometry selection from the entries that are selected
		/// in the SceneView_knob.
		void rebuildSelection();
		/// Clear any selected geometry from the SceneView_knob.
		void clearSceneViewSelection();
		/// Clears the current filters applied to the scene..
		void clearSceneViewFilter();
		//@}

		/// Updates the Enumeration_knob of available tags from the internal list of tags
		/// and updates the currently selected tag to ensure that it is valid.
		void updateTagFilterKnob();
		/// Loads a primitive from the scene cache and adds it to the GeometryList.
		void loadPrimitive( DD::Image::GeometryList &out, const std::string &path );
		/// Get the hash of the file path and root knob.
		DD::Image::Hash sceneHash() const;
		/// Evaluates the file path for the current context and returns the result.
		std::string filePath() const;
		
		Imath::M44d worldTransform( IECore::ConstSceneInterfacePtr scene, IECore::SceneInterface::Path root, double time );
	
		/// Returns an InternedString with the name of the geometry tag.	
		static const IECore::InternedString &geometryTag();

		// Knob Members.	
		const char *m_filePath; // Holds the raw SceneCache file path.
		std::string m_evaluatedFilePath; // Holds the SceneCache file path after any TCL scripts have been evaluated..
		std::string m_root; // Holds the root item in the SceneCache.
		std::string m_filterText; // The text to filter the scene with.
		std::string m_filterTagText; // The text to filter the tags with. This is set from the Enumeration_knob.
		bool m_worldSpace; // Set to ignore local transforms..
		DD::Image::Matrix4 m_baseParentMatrix; // The global matrix that is applied to the geo.
		
		// Hashes that are used to both provide an early-out to some methods
		// and also contribute towards a hash for the geometry. 
		DD::Image::Hash m_selectionHash;
		DD::Image::Hash m_filterHash;
		DD::Image::Hash m_sceneHash;

		// When buildSceneView is called to parse the scene cache and generate a list of entries for the SceneView_knob,
		// this map is also populated. It holds a mapping of tag names to the indices of items which have that tag. 
		// It is used within the filterScene method to quickly filter items with a particular tag.
		typedef	std::map< std::string, std::vector< unsigned int > > TagMap;
		TagMap m_tagMap;
		
		// When specifying a root we store the path to it's parent item along with the length of it. We do this so that when
		// we are building the list of items in the SceneView_knob we can strip this path quickly from the front of the
		// name and easily restore it later to load it from the SceneCache. This ensures that the names of the items in the
		// SceneView_knob are kept short. 
		std::string m_pathPrefix;
		unsigned int m_pathPrefixLength;
	
		// Pointers to various knobs.	
		DD::Image::Knob *m_filePathKnob;
		DD::Image::Knob *m_baseParentMatrixKnob;
		DD::Image::Knob *m_sceneKnob;
		DD::Image::Knob *m_tagFilterKnob;
		DD::Image::Knob *m_sceneFilterKnob;
		DD::Image::Knob *m_rootKnob;

		 // A flag which is set when all of the knobs have been loaded from the script.
		bool m_scriptLoaded;
		
		/// The SceneView_knob holds a list of all leaf items in the scene. When filtering the SceneView we specify indices into
		/// this list. When setting or querying the selected items in the SceneView_knob we need to use indices into the list of 
		/// filtered (visible) items. This means that we have to keep a look-up table of mappings between indices in the filtered
		/// list of items and the index within the complete list of items in the scene.
		std::map<int, int> m_itemToFiltered; // Mapping of the index within the full scene list and the filtered scene list.
		std::vector<unsigned int> m_filteredToItem; // Mapping from an index in the filtered scene list to the complete scene list. 
		std::vector< bool > m_itemSelected; // An array of flags which indicate whether an item in the filtered list is selected or not.
};

} // namespace IECoreNuke

#endif // IENUKE_SCENECACHEREADER_H

