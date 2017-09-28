//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/GL.h" // must come first so glew.h is included before gl.h

#include "IECoreNuke/SceneCacheReader.h"

#include "boost/regex.hpp"

#include "DDImage/SceneView_KnobI.h"
#include "DDImage/Enumeration_KnobI.h"
#include "DDImage/GeoSelectKnobI.h"

#include "IECore/TransformOp.h"
#include "IECore/SceneCache.h"
#include "IECore/SceneInterface.h"
#include "IECore/SharedSceneInterfaces.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/AttributeBlock.h"

#include "IECoreNuke/Hash.h"
#include "IECoreNuke/Convert.h"
#include "IECoreNuke/ToNukeGeometryConverter.h"

#include "IECoreGL/IECoreGL.h"
#include "IECoreGL/State.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/Group.h"

#include <iostream>
#include <sstream>

using namespace DD::Image;
using namespace IECoreNuke;
using namespace std;
using namespace Imath;

static const char* const CLASS = "ieSceneCacheReader";

static const char* const HELP = "Loads and displays geometry from a scene cache file.";

DD::Image::Op *SceneCacheReader::build( Node *node ){ return new SceneCacheReader( node ); }

const Op::Description SceneCacheReader::m_description( CLASS, SceneCacheReader::build );

const char *SceneCacheReader::Class() const { return m_description.name; }

const char *SceneCacheReader::node_help() const { return HELP; }

typedef	std::map< std::string, std::vector< unsigned int > > TagMap;

class SceneCacheReader::SharedData
{
	public :
		SharedData() :
			m_evaluatedFilePath( "" ),
			m_rootText( "/" ),
			m_filterText( "" ),
			m_filterTagText( "" ),
			m_pathPrefix( "" ),
			m_pathPrefixLength( 0 ),
			m_scriptFinishedLoading( false ),
			m_isFirstRun( true )
		{
		}

		std::string m_evaluatedFilePath; // Holds the SceneCache file path after any TCL scripts have been evaluated..
		std::string m_rootText; // Holds the processed root item in the SceneCache.
		std::string m_filterText; // Processed text to filter the scene with.
		std::string m_filterTagText; // Processed text to filter the tags with.

		// Hashes that are used to both provide an early-out to some methods
		// and also contribute towards a hash for the geometry.
		DD::Image::Hash m_selectionHash;
		DD::Image::Hash m_filterHash;
		DD::Image::Hash m_sceneHash;

		// When buildSceneView is called to parse the scene cache and generate a list of entries for the SceneView_knob,
		// this map is also populated. It holds a mapping of tag names to the indices of items which have that tag.
		// It is used within the filterScene method to quickly filter items with a particular tag.
		TagMap m_tagMap;

		// When specifying a root we store the path to it's parent item along with the length of it. We do this so that when
		// we are building the list of items in the SceneView_knob we can strip this path quickly from the front of the
		// name and easily restore it later to load it from the SceneCache. This ensures that the names of the items in the
		// SceneView_knob are kept short.
		std::string m_pathPrefix;
		unsigned int m_pathPrefixLength;

		/// The SceneView_knob holds a list of all leaf items in the scene. When filtering the SceneView we specify indices into
		/// this list. When setting or querying the selected items in the SceneView_knob we need to use indices into the list of
		/// filtered (visible) items. This means that we have to keep a look-up table of mappings between indices in the filtered
		/// list of items and the index within the complete list of items in the scene.
		std::map<int, int> m_itemToFiltered; // Mapping of the index within the full scene list and the filtered scene list.
		std::vector<unsigned int> m_filteredToItem; // Mapping from an index in the filtered scene list to the complete scene list.
		std::vector<unsigned int> m_selectedItems; // An array of selected items

		 // A flag which is set when all of the knobs have been loaded from the script.
		bool m_scriptFinishedLoading;

		// A flag which is used to initialize the internal data structures the first time the node is run.
		bool m_isFirstRun;
};

// A simple function for comparing two strings. Used when sorting a vector of strings.
bool compareNoCase( const string& s1, const string& s2 )
{
	return strcasecmp( s1.c_str(), s2.c_str() ) <= 0;
}

// Used when removing redundant slashes from a path.
bool bothSlashes( char a, char b )
{
	    return a == '/' && b == '/';
}

static void buildSceneView( std::vector< std::string > &list, TagMap &tagMap, const IECore::ConstSceneInterfacePtr sceneInterface, int rootPrefixLen );

SceneCacheReader::SceneCacheReader( Node *node )
	:	SourceGeo( node ),
		m_filePath( "" ),
		m_root( "/" ),
		m_filter( "" ),
		m_worldSpace( false ),
		m_filePathKnob( NULL ),
		m_baseParentMatrixKnob( NULL ),
		m_sceneKnob( NULL ),
		m_tagFilterKnob( NULL ),
		m_sceneFilterKnob( NULL ),
		m_rootKnob( NULL ),
		m_data(NULL)
{
	m_baseParentMatrix.makeIdentity();

	if ( firstOp() == this )
	{
		m_data = new SharedData();
	}
}

SceneCacheReader::~SceneCacheReader()
{
	delete m_data;
}

SceneCacheReader *SceneCacheReader::firstReader()
{
	return dynamic_cast<SceneCacheReader*>(firstOp());
}

const SceneCacheReader *SceneCacheReader::firstReader() const
{
	return dynamic_cast<SceneCacheReader*>(firstOp());
}

SceneCacheReader::SharedData *SceneCacheReader::sharedData()
{
	return firstReader()->m_data;
}

const SceneCacheReader::SharedData *SceneCacheReader::sharedData() const
{
	return firstReader()->m_data;
}

void SceneCacheReader::_validate( bool forReal )
{
	if ( firstReader() != this )
	{
		SourceGeo::_validate( forReal );
		return;
	}

	m_data->m_scriptFinishedLoading = true;

	if( m_data->m_isFirstRun )
	{
		Knob *k = knob("loadAll");
		if( k != NULL )
		{
			k->set_value( true );
		}

		m_data->m_isFirstRun = false;
		loadAllFromKnobs();
	}
	else
	{
		filterScene( m_data->m_filterText, m_data->m_filterTagText );
	}

	SourceGeo::_validate( forReal );
}

void SceneCacheReader::knobs( DD::Image::Knob_Callback f )
{
	SourceGeo::knobs( f );

	m_filePathKnob = File_knob( f, &m_filePath, "file", "File" );
	SetFlags( f, DD::Image::Knob::MODIFIES_GEOMETRY | DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS );
	Tooltip( f,
		"File name for the scene cache."
	);

	m_rootKnob = String_knob( f, &m_root, "sceneRoot", "Root" );
	SetFlags( f, DD::Image::Knob::MODIFIES_GEOMETRY | DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS | DD::Image::Knob::NO_ANIMATION );
	Tooltip( f,
		"Root path for the scene cache."
	);

	int p = 0;
	const char* tagNames[2] = { "None", 0 };
	m_tagFilterKnob = Enumeration_knob( f, &p, tagNames, "filterByTag", "Filter Tag" );
	SetFlags( f, DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS );
	Tooltip( f,
		"Filter items in the scene by their tagged attributes."
	);

	m_sceneFilterKnob = String_knob( f, &m_filter, "filterByName", "Filter Name" );
	SetFlags( f, DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS );
	Tooltip( f,
		"Filter items in the scene using full or partial matches of their names against this text."
	);

	const char* e = { 0 };
	m_sceneKnob = SceneView_knob( f, &p, &e, "sceneView", "Scene Hierarchy" );
	SetFlags( f, DD::Image::Knob::RESIZABLE | DD::Image::Knob::MODIFIES_GEOMETRY | DD::Image::Knob::SAVE_MENU | DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS
		| DD::Image::Knob::KNOB_CHANGED_RIGHTCONTEXT | DD::Image::Knob::NO_ANIMATION
	);

	Bool_knob( f, &m_worldSpace, "worldSpace", "World Space" );
	Tooltip( f,
		"Use world space as opposed to root."
	);

	// transform knobs
	m_baseParentMatrixKnob = Axis_knob(f, &m_baseParentMatrix, "transform");
	if ( m_baseParentMatrixKnob != NULL)
	{
		if (GeoOp::selectable() == true)
		{
			m_baseParentMatrixKnob->enable();
		}
		else
		{
			m_baseParentMatrixKnob->disable();
		}
	}

	// This knob should never be changed by the user. It provides a mechanism for us to know when the
	// item's knobs have been fully loaded through either a script load or a copy/paste. We need this
	// because in order to rebuild our internal data structures correctly we require all of the knob's
	// values to be loaded. Look at the knob_changed() method for more information.
	bool b = false;
	Bool_knob( f, &b, "loadAll" );
	SetFlags( f, DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS | DD::Image::Knob::INVISIBLE );

}

int SceneCacheReader::knob_changed(Knob* k)
{
	if ( firstReader() != this )
	{
		return SourceGeo::knob_changed(k);
	}

	if ( k != NULL )
	{
		if ( knob("selectable") == k )
		{
			if ( GeoOp::selectable() == true )
			{
				m_baseParentMatrixKnob->enable();
			}
			else
			{
				m_baseParentMatrixKnob->disable();
			}
			return 1;
		}
		else if( m_filePathKnob == k )
		{
			// during knob_changed we cannot query m_filePath because it returns the previous value.
			if ( script_expand( m_filePathKnob->get_text() ) )
			{
				m_data->m_evaluatedFilePath = script_result();
				script_unlock();
			}

			if ( m_data->m_scriptFinishedLoading )
			{
				// we want to keep current selection...
				loadAllFromKnobs();
			}
			return 1;
		}
		else if( m_rootKnob == k )
		{
			std::string root = m_rootKnob->get_text();

			// Validate the root string by removing duplicate '/' and ensuring that it starts with a '/' but doesn't end with one.
			root.erase( std::unique( root.begin(), root.end(), bothSlashes ), root.end());

			if( root.size() > 1 && root[ root.size()-1 ] == '/' )
			{
				root = root.substr(0, root.size()-1);
			}

			if( root.empty() )
			{
				root = std::string("/");
			}
			else if( root[0] != '/' )
			{
				root = std::string("/") + root;
			}

			// We would like the items in the SceneView_knob to be listed under the name of the root rather than it's full
			// path. This means that we need to modify all of the item strings that we pass to it by removing the unwanted
			// part of the path.
			// To make recovery of the full path easier we store the unwanted of the path as a member.
			IECore::SceneInterface::Path rootPath;
			IECore::SceneInterface::stringToPath( root, rootPath );
			m_data->m_pathPrefix.clear();
			if( rootPath.size() > 0 )
			{
				rootPath.pop_back();
				IECore::SceneInterface::pathToString( rootPath, m_data->m_pathPrefix );
			}

			// We keep the length of the unwanted path string so that we can use it to easily truncate the names of the items
			// that we use to populate the SceneView_knob.
			m_data->m_pathPrefixLength = m_data->m_pathPrefix.size();
			m_data->m_rootText = root;
			m_rootKnob->set_text( root.c_str() );

			// Finally, update the UI with the validated string and rebuild the SceneView_knob.
			if ( m_data->m_scriptFinishedLoading )
			{
				// we want to keep current selection...
				loadAllFromKnobs();
			}
			return 1;
		}
		else if ( m_sceneFilterKnob == k )
		{
			// As the filter expression or tag has changed, filter the scene again.
			if( m_sceneFilterKnob )
			{
				const char* c = m_sceneFilterKnob->get_text();
				if( c )
				{
					m_data->m_filterText = std::string( c );
				}
				else
				{
					m_data->m_filterText = "";
				}
			}

			if ( m_data->m_scriptFinishedLoading )
			{
				filterScene( m_data->m_filterText, m_data->m_filterTagText );
			}
			return 1;
		}
		else if ( m_tagFilterKnob == k )
		{
			// Get the tag's name.
			m_data->m_filterTagText = std::string("");
			if( m_tagFilterKnob )
			{
				tagSelection( m_data->m_filterTagText );
			}

			if ( m_data->m_scriptFinishedLoading )
			{
				filterScene( m_data->m_filterText, m_data->m_filterTagText );
			}
			return 1;
		}
		else if( m_sceneKnob == k )
		{
			m_data->m_selectionHash = Hash();
			return 1;
		}
		// This knob is only loaded when a script is pasted or loaded from a file.
		// As it is loaded last we know that the other knobs have already been set.
		// This means that we have enough information to build our internal data
		// structures.
		else if( knob("loadAll") == k )
		{
			if( !m_data->m_scriptFinishedLoading )
			{
				m_data->m_scriptFinishedLoading = true;
				validate( false );
			}

			return 1;
		}
		else if( std::string( k->name() ) == "hidePanel" || std::string( k->name() ) == "showPanel" )
		{
			if( !m_data->m_scriptFinishedLoading )
			{
				m_data->m_scriptFinishedLoading = true;
				if( knob( "loadAll" ) != NULL )
				{
					validate( false );
				}
			}

			return SourceGeo::knob_changed(k);
		}
	}

	return SourceGeo::knob_changed(k);
}

void SceneCacheReader::loadAllFromKnobs()
{
	assert ( firstReader() == this );

	if( !m_data->m_scriptFinishedLoading )
	{
		return;
	}

	SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );

	std::vector<unsigned int> selectionIndices;
	sceneView->getSelectedItems( selectionIndices );

	std::vector<unsigned int> filterIndices;
	sceneView->getImportedItems( filterIndices );

	std::vector<std::string> oldItems = sceneView->menu();

	rebuildSceneView();

	// filter of the scene without worrying about selection
	m_data->m_filterHash = Hash();
	filterScene( m_data->m_filterText, m_data->m_filterTagText, false );

	const std::vector< std::string > &items = sceneView->menu();

	// Try to remap the previous selection to the current items available.
	std::vector<unsigned int> newSelectionIndices;
	for( std::vector<unsigned int>::const_iterator it( selectionIndices.begin() ); it != selectionIndices.end(); ++it )
	{
		int itemIndex( filterIndices[*it] );
		const std::string &itemName( oldItems[itemIndex] );

		std::vector<std::string>::const_iterator selectedIt( std::find( items.begin(), items.end(), itemName ) );
		if ( selectedIt == items.end() )
		{
			warning( ( std::string( "WARNING: Could not load selected geometry \"" ) + itemName + std::string( "\" as it no longer exists in the scene cache." ) ).c_str() );
			continue;
		}

		unsigned int index = (selectedIt - items.begin());

		if ( index < m_data->m_itemToFiltered.size() )
		{
			index = m_data->m_itemToFiltered[index];
		}
		newSelectionIndices.push_back( index );
	}

	sceneView->setSelectedItems( newSelectionIndices );
	m_data->m_selectionHash = Hash();
}

void SceneCacheReader::clearSceneViewSelection()
{
	assert ( firstReader() == this );

	if( m_sceneKnob != NULL )
	{
		std::vector<unsigned int> emptySelection;
		SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );
		sceneView->setSelectedItems( emptySelection );
		m_data->m_selectionHash = Hash();
	}
}

Hash SceneCacheReader::sceneHash() const
{
	if ( firstReader() != this )
	{
		return firstReader()->sceneHash();
	}

	Hash newHash;
	newHash.append( m_data->m_evaluatedFilePath );
	newHash.append( m_data->m_rootText );
	newHash.append( outputContext().frame() );
	return newHash;
}

Hash SceneCacheReader::selectionHash( bool force ) const
{
	if ( firstReader() != this )
	{
		return firstReader()->selectionHash(false);
	}

	if ( force || m_data->m_selectionHash == Hash() )
	{
		Hash newHash;

		SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );

		std::vector<unsigned int> indices;
		sceneView->getImportedItems( indices );

		newHash.append( (unsigned int)indices.size() );
		for( std::vector<unsigned int>::const_iterator cit( indices.begin() ); cit != indices.end(); ++cit )
		{
			newHash.append( *cit );
		}

		m_data->m_selectedItems.clear();
		sceneView->getSelectedItems( m_data->m_selectedItems );
		newHash.append( (unsigned int)m_data->m_selectedItems.size() );
		for( std::vector<unsigned int>::const_iterator cit( m_data->m_selectedItems.begin() ); cit != m_data->m_selectedItems.end(); ++cit )
		{
			newHash.append( *cit );
		}

		m_data->m_selectionHash = newHash;
	}
	return m_data->m_selectionHash;
}

void SceneCacheReader::rebuildSceneView()
{
	assert ( firstReader() == this );

	if( !m_data->m_scriptFinishedLoading )
	{
		return;
	}

	if( m_data->m_isFirstRun )
	{
		validate( false );
	}

	Hash newSceneHash( sceneHash() );

	// Check to see if the scene has changed. If it has then we need to
	// rebuild our internal representation of it.
	if( m_data->m_sceneHash != newSceneHash )
	{
		// Set the new hash.
		m_data->m_sceneHash = newSceneHash;

		IECore::ConstSceneInterfacePtr sceneInterface = getSceneInterface();
		SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );

		// If we have a selection, clear it!
		if( m_data->m_selectedItems.size() != 0 )
		{
			clearSceneViewSelection();
		}

		// Reset our internal data structures.
		m_data->m_tagMap.clear();

		// Clear the SceneView_knob.
		std::vector<std::string> sceneItems;
		sceneView->menu( sceneItems );

		if( sceneInterface && sceneView )
		{
			// Rebuild our list of items which we will use to populate the SceneView_knob.
			buildSceneView( sceneItems, m_data->m_tagMap, sceneInterface, m_data->m_pathPrefixLength );
		}

		updateTagFilterKnob();

		m_data->m_selectionHash = Hash();

		if( !sceneItems.empty() )
		{
			// Reset the list of selected entries and populate the SceneView_knob.
			sceneView->addItems( sceneItems );
		}
	}
}

void SceneCacheReader::build_handles(ViewerContext* ctx)
{
	// Call build_matrix_handle to multiply the context model matrix with the parent base matrix so the
	// items above it will display correctly.
	build_matrix_handles(ctx, m_baseParentMatrix);
}

const std::string &SceneCacheReader::itemName( int index ) const
{
	assert ( firstReader() == this );

	SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );
	const std::vector<std::string> &items( sceneView->menu() );
	return items[index];
}

// The purpose of the filterScene method is to both filter out unwanted items from the SceneView_knob
// and create a mapping between the indices of the resulting items and their position in the full
// list of items in the scene. We do this because when we query the SceneView_knob for the selected
// items, we are returned a list of indices within the filtered items. Therefore, to get the
// names of these items we need to use a LUT of filtered indices to indices within the list of names.
// These LUTs are the m_itemToFiltered and m_filteredToItem maps.
void SceneCacheReader::filterScene( const std::string &filterText, const std::string &filterTag, bool keepSelection )
{
	assert ( firstReader() == this );

	if( !m_data->m_scriptFinishedLoading )
	{
		return;
	}

	if( m_data->m_isFirstRun )
	{
		validate( false );
	}

	Hash newFilterHash( sceneHash() );
	newFilterHash.append( m_data->m_evaluatedFilePath );
	newFilterHash.append( m_data->m_rootText );
	newFilterHash.append( filterText );
	newFilterHash.append( filterTag );

	if( m_data->m_filterHash == newFilterHash )
	{
		return;
	}
	m_data->m_filterHash = newFilterHash;

	SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );

	// Get the item indices of the currently selected items so that
	// we can add them to the newly filtered scene.
	std::vector<unsigned int> newSelection;

	if ( keepSelection )
	{
		sceneView->getSelectedItems( newSelection ); // Set the vector to the indices of the filtered items.
		for( std::vector<unsigned int>::iterator it( newSelection.begin() ); it != newSelection.end(); ++it )
		{
			if ( *it < m_data->m_filteredToItem.size() )
			{
				*it = m_data->m_filteredToItem[*it]; // Convert the filter index to a item index.
			}
		}
	}

	// Reset our LUTs as we are going to rebuild them.
	m_data->m_itemToFiltered.clear();
	m_data->m_filteredToItem.clear();

	// Are we also filtering by tag?
	bool filterByTag = false;
	if( filterTag != "" && filterTag != "None" )
	{
		filterByTag = true;
	}

	// Remove duplicate slashes in the expression.
	std::string expr = filterText;
	expr.erase( std::unique( expr.begin(), expr.end(), bothSlashes ), expr.end());

	std::vector<unsigned int> filteredIndices;

	const std::vector<std::string> &sceneItems = sceneView->menu();

	if( expr != "/" && expr != "*" && expr != "" )
	{
		// Get the filter text that we will use to filter the scene.
		try
		{
			// Loop over all of the items in the scene and create a unique list of all entries whose name matches our regular expression.
			boost::regex expression( expr );

			if( filterByTag ) // Filter by tag and expression
			{
				for( std::vector<unsigned int>::const_iterator it = m_data->m_tagMap[filterTag].begin(); it != m_data->m_tagMap[filterTag].end(); ++it )
				{
					const std::string &itemName( sceneItems[*it] );
					boost::sregex_token_iterator iter( itemName.begin(), itemName.end(), expression, 0 );
					boost::sregex_token_iterator end;
					if( iter != end )
					{
						// Append the index to our vector of filtered items.
						filteredIndices.push_back( *it );
					}
				}
			}
			else // Only filter by expression.
			{
				for( std::vector<std::string>::const_iterator it = sceneItems.begin(); it != sceneItems.end(); ++it )
				{
					boost::sregex_token_iterator iter( it->begin(), it->end(), expression, 0 );
					boost::sregex_token_iterator end;
					if( iter != end )
					{
						// Append the index to our vector of filtered items.
						filteredIndices.push_back( it-sceneItems.begin() );
					}
				}
			}
		}
		catch(...)
		{
		}
	}
	else
	{
		filteredIndices.clear();
		if( !sceneItems.empty() )
		{
			if( filterByTag )
			{
				// Just filter the items with the chosen the tag.
				const std::vector<unsigned int> &tagIndices( m_data->m_tagMap[filterTag] );
				if( !tagIndices.empty() )
				{
					filteredIndices.reserve( tagIndices.size() );
					for( std::vector<unsigned int>::const_iterator it( tagIndices.begin() ); it != tagIndices.end(); ++it )
					{
						filteredIndices.push_back( *it );
					}
				}
			}
			else
			{
				// Don't filter any of the results.
				filteredIndices.reserve( sceneItems.size() );
				for( std::vector<std::string>::const_iterator it( sceneItems.begin() ); it != sceneItems.end(); ++it )
				{
					int index = it - sceneItems.begin();
					filteredIndices.push_back( index );
				}
			}
		}
	}

	// Add the old selection to the filtered results.
	if( !newSelection.empty() )
	{
		for( std::vector<unsigned int>::const_iterator it( newSelection.begin() ); it != newSelection.end(); ++it )
		{
			if ( *it <= sceneItems.size() )
			{
				filteredIndices.push_back( *it );
			}
		}

		// Make sure all indices in our selection are unique and sorted.
		std::set<unsigned int> selectionSet( filteredIndices.begin(), filteredIndices.end() );
		filteredIndices.clear();
		std::copy( selectionSet.begin(), selectionSet.end(), std::back_inserter(filteredIndices) );
		sort( filteredIndices.begin(), filteredIndices.end() );
	}

	m_data->m_filteredToItem = filteredIndices;

	// Restore the old selection.
	m_data->m_itemToFiltered.clear();

	// Set the filtered items.
	sceneView->setImportedItems( filteredIndices );

	// Create a mapping of item indices to filtered indices.
	for( std::vector<unsigned int>::const_iterator it( filteredIndices.begin() ); it != filteredIndices.end(); ++it )
	{
		m_data->m_itemToFiltered[ *it ] = it - filteredIndices.begin();
	}

	// we invalidate the selection hash to force recomputation since importted items changed.
	m_data->m_selectionHash = Hash();
}

void SceneCacheReader::tagSelection( std::string &selection ) const
{
	assert ( firstReader() == this );

	Enumeration_KnobI *listView( m_tagFilterKnob->enumerationKnob() );
	const std::vector<std::string> &currentTagList( listView->menu() );
	unsigned int tagIndex = static_cast<unsigned int>( m_tagFilterKnob->get_value() );
	if( tagIndex < currentTagList.size() )
	{
		selection = currentTagList[tagIndex];
		return;
	}
	selection = "";
}

void SceneCacheReader::updateTagFilterKnob()
{
	assert ( firstReader() == this );

	if( m_tagFilterKnob )
	{
		std::vector<std::string> tagNames;
		tagNames.push_back( "None" );

		for( TagMap::const_iterator it = m_data->m_tagMap.begin(); it != m_data->m_tagMap.end(); ++it )
		{
			tagNames.push_back( it->first );
		}

		Enumeration_KnobI *listView( m_tagFilterKnob->enumerationKnob() );
		listView->menu( tagNames );

		std::string currentTagSelection;
		tagSelection( currentTagSelection );
		if ( std::find( tagNames.begin(), tagNames.end(), currentTagSelection ) == tagNames.end() )
		{
			m_tagFilterKnob->set_value( 0 );
		}
	}
}

/// This recursive method traverses the sceneInterface to build a list of item names and a mapping of the tags to the indices in the items.
static void buildSceneView( std::vector< std::string > &list, TagMap &tagMap, const IECore::ConstSceneInterfacePtr sceneInterface, int rootPrefixLen )
{
	if( sceneInterface )
	{
		if( sceneInterface->hasObject()  )
		{
			IECore::SceneInterface::NameList tagNames;
			sceneInterface->readTags( tagNames, IECore::SceneInterface::LocalTag );
			for ( IECore::SceneInterface::NameList::const_iterator it = tagNames.begin(); it != tagNames.end(); ++it )
			{
				tagMap[*it].push_back( list.size() );
			}

			IECore::SceneInterface::Path path;
			sceneInterface->path( path );
			std::string pathStr;
			IECore::SceneInterface::pathToString( path, pathStr );

			// The SceneView_knob requires that all entries belong to the same root item.
			// This is an issue as the SceneCache can have multiple entries at root level.
			// To resolve this issue we append "/root" to the item name when viewing the
			// tree at root level.
			if( rootPrefixLen == 0 ) // This means m_data->m_rootText == "/"
			{
				pathStr = std::string("/root") + pathStr;
			}
			else
			{
				pathStr = pathStr.substr( rootPrefixLen );
			}
			list.push_back( pathStr );
		}

		IECore::SceneInterface::NameList childNames;
		sceneInterface->childNames( childNames );
		sort( childNames.begin(), childNames.end(), compareNoCase );

		for ( IECore::SceneInterface::NameList::const_iterator it = childNames.begin(); it != childNames.end(); ++it )
		{
			IECore::SceneInterface::Name name = *it;
			const IECore::ConstSceneInterfacePtr childScene = sceneInterface->child( name );
			buildSceneView( list, tagMap, childScene, rootPrefixLen );
		}
	}
}

void SceneCacheReader::append( DD::Image::Hash &hash )
{
	SourceGeo::append( hash );

	hash.append( sceneHash() );
	hash.append( selectionHash(true) );
	hash.append( m_worldSpace );
	hash.append( outputContext().frame() );
}

void SceneCacheReader::get_geometry_hash()
{
	SourceGeo::get_geometry_hash();
	geo_hash[Group_Primitives].append( sceneHash() );
	geo_hash[Group_Primitives].append( selectionHash() );
	geo_hash[Group_Primitives].append( m_worldSpace );

	IECore::ConstSceneCachePtr sceneInterface = IECore::runTimeCast< const IECore::SceneCache >(getSceneInterface());
	bool isAnimated = false;
	if( sceneInterface && sceneInterface -> numBoundSamples() > 1 )
	{
		isAnimated = true;
		geo_hash[Group_Primitives].append( outputContext().frame() );
	}

	static const int groups[] = { Group_Points, Group_Attributes, Group_Matrix, Group_None };
	for( const int *g = groups; *g!=Group_None; g++ )
	{
		geo_hash[*g].append( sceneHash() );
		geo_hash[*g].append( selectionHash() );
		geo_hash[*g].append( m_worldSpace );
		if( isAnimated )
		{
			geo_hash[*g].append( outputContext().frame() );
		}
	}

	geo_hash[Group_Matrix].append(m_baseParentMatrix.a00);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a01);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a02);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a03);

	geo_hash[Group_Matrix].append(m_baseParentMatrix.a10);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a11);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a12);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a13);

	geo_hash[Group_Matrix].append(m_baseParentMatrix.a20);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a21);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a22);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a23);

	geo_hash[Group_Matrix].append(m_baseParentMatrix.a30);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a31);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a32);
	geo_hash[Group_Matrix].append(m_baseParentMatrix.a33);

}

// Apply the concatenated matrix to all the GeoInfos.
void SceneCacheReader::geometry_engine( DD::Image::Scene& scene, GeometryList& out)
{
	SourceGeo::geometry_engine(scene, out);

	for( unsigned i = 0; i < out.size(); i++ )
	{
		out[i].matrix = m_baseParentMatrix;
	}
}

void SceneCacheReader::create_geometry( DD::Image::Scene &scene, DD::Image::GeometryList &out )
{
	SharedData *data = sharedData();

	// Don't do any work if our hash hasn't changed.
	// this is important not only for speed, but also because
	// something in nuke assumes we won't change anything if
	// rebuilding isn't needed - we get crashes if we rebuild
	// when not necessary.
	if( !rebuild( Mask_Attributes ) && !rebuild( Mask_Matrix ) )
	{
		return;
	}

	if( !m_filePath || data->m_evaluatedFilePath == "" )
	{
		// Get rid of the old stuff, and return.
		out.delete_objects();
		return;
	}

	try
	{
		if( rebuild( Mask_Primitives ) )
		{
			out.delete_objects();

			// Loop over the selected items in the SceneView_knob and add them to the geometry list.
			SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );
			const std::vector<std::string> &items = sceneView->menu();

			for( std::vector<unsigned int>::const_iterator it( data->m_selectedItems.begin() ); it != data->m_selectedItems.end(); ++it )
			{
				unsigned int index = *it;
				if ( index < data->m_filteredToItem.size() )
				{
					loadPrimitive( out, items[ data->m_filteredToItem[index] ] );
				}
			}
		}

	}
	catch( const std::exception &e )
	{
		error( e.what() );
		return;
	}
}

void SceneCacheReader::loadPrimitive( DD::Image::GeometryList &out, const std::string &path )
{
	SharedData *data = sharedData();

	std::string itemPath;
	if( data->m_rootText == "/" )
	{
		// Remove the "/root" prefix that was added to the path name.
		itemPath = path.substr( 5 ); // "/root" is 5 characters long...
	}
	else
	{
		// Add the prefix that we removed when creating the entry.
		itemPath = data->m_pathPrefix + path;
	}

	IECore::ConstSceneInterfacePtr sceneInterface( getSceneInterface( itemPath ) );
	if( sceneInterface )
	{
		double time = outputContext().frame() / DD::Image::root_real_fps();
		IECore::ConstObjectPtr object = sceneInterface->readObject( time );
		IECore::SceneInterface::Path rootPath;
		if( m_worldSpace )
		{
			IECore::SceneInterface::stringToPath( "/", rootPath );
		}
		else
		{
			IECore::SceneInterface::stringToPath( data->m_rootText, rootPath );
		}

		Imath::M44d transformd;
		transformd = worldTransform( sceneInterface, rootPath, time );
		IECore::TransformOpPtr transformer = new IECore::TransformOp();
		transformer->inputParameter()->setValue( const_cast< IECore::Object * >(object.get()) );	// safe const_cast because the Op will copy the input object.
		transformer->copyParameter()->setTypedValue( true );
		transformer->matrixParameter()->setValue( new IECore::M44dData( transformd ) );
		object = transformer->operate();

		IECoreNuke::ToNukeGeometryConverterPtr converter = IECoreNuke::ToNukeGeometryConverter::create( object );
		if (converter)
		{
			converter->convert( out );
		}
	}
}

Imath::M44d SceneCacheReader::worldTransform( IECore::ConstSceneInterfacePtr scene, IECore::SceneInterface::Path root, double time )
{
	IECore::SceneInterface::Path p;
	scene->path( p );

	IECore::ConstSceneInterfacePtr tmpScene = scene->scene( root );
	IECore::SceneInterface::Path pRoot;
	tmpScene->path( pRoot );
	Imath::M44d result;

	for ( IECore::SceneInterface::Path::const_iterator it = p.begin()+pRoot.size(); tmpScene && it != p.end(); ++it )
	{
		tmpScene = tmpScene->child( *it, IECore::SceneInterface::NullIfMissing );
		if ( !tmpScene )
		{
			break;
		}

		result = tmpScene->readTransformAsMatrix( time ) * result;
	}

	return result;
}

IECore::ConstSceneInterfacePtr SceneCacheReader::getSceneInterface( const string &path )
{
	const SharedData *data = sharedData();
	IECore::ConstSceneInterfacePtr scene(0);
	try
	{
		scene = IECore::SharedSceneInterfaces::get( data->m_evaluatedFilePath );
	}
	catch( const std::exception &e )
	{
		error( (std::string("Could not open file ") + data->m_evaluatedFilePath ).c_str() );
		return 0;
	}

	try
	{
		IECore::SceneInterface::Path itemPath;
		IECore::SceneInterface::stringToPath( path, itemPath );
		scene = scene->scene( itemPath );
	}
	catch( const std::exception &e )
	{
		error( (std::string("Could not find root \"" + path + "\" in ") + data->m_evaluatedFilePath ).c_str() );
		return 0;
	}

	return scene;
}

IECore::ConstSceneInterfacePtr SceneCacheReader::getSceneInterface()
{
	return getSceneInterface( sharedData()->m_rootText );
}

