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

SceneCacheReader::SceneCacheReader( Node *node )
	:	SourceGeo( node ),
		m_filePath( "" ),
		m_evaluatedFilePath( "" ),
		m_root( "/" ),
		m_filterText( "" ),
		m_filterTagText( "" ),
		m_worldSpace( false ),
		m_pathPrefix( "" ),
		m_pathPrefixLength( 0 ),
		m_filePathKnob( NULL ),
		m_baseParentMatrixKnob( NULL ),
		m_sceneKnob( NULL ),
		m_tagFilterKnob( NULL ),
		m_sceneFilterKnob( NULL ),
		m_rootKnob( NULL ),
		m_scriptFinishedLoading( false ),
		m_isFirstRun( true )
{
	m_baseParentMatrix.makeIdentity();
}

SceneCacheReader::~SceneCacheReader()
{
}

void SceneCacheReader::_validate( bool forReal )
{
	m_evaluatedFilePath	= filePath();

	m_scriptFinishedLoading = true;
	if( m_isFirstRun )
	{
		Knob *k = knob("loadAll");
		if( k != NULL )
		{
			k->set_value( true );
		}

		m_isFirstRun = false;
		loadAllFromKnobs();
	}

	filterScene( m_filterText, m_filterTagText );
	rebuildSelection();

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

	m_rootKnob = String_knob( f, &m_root, "root", "Root" );
	SetFlags( f, DD::Image::Knob::MODIFIES_GEOMETRY | DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS );
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

	m_sceneFilterKnob = String_knob( f, &m_filterText, "filterByName", "Filter Name" );
	SetFlags( f, DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS );
	Tooltip( f,
		"Filter items in the scene using full or partial matches of their names against this text."
	);

	const char* e = { 0 };
	m_sceneKnob = SceneView_knob( f, &p, &e, "sceneView", "Scene Hierarchy" );
	SetFlags( f, DD::Image::Knob::RESIZABLE | DD::Image::Knob::MODIFIES_GEOMETRY | DD::Image::Knob::SAVE_MENU | DD::Image::Knob::ALWAYS_SAVE | DD::Image::Knob::KNOB_CHANGED_ALWAYS
		| DD::Image::Knob::KNOB_CHANGED_RIGHTCONTEXT
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

std::string SceneCacheReader::filePath() const
{
	if( m_filePathKnob )
	{
		std::stringstream pathStream;
		m_filePathKnob->to_script( pathStream, &(outputContext()), false );
		return pathStream.str();
	}
	else
	{
		return "";
	}
}

int SceneCacheReader::knob_changed(Knob* k)
{
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
			m_evaluatedFilePath	= filePath();
			rebuildSceneView();
			filterScene( m_filterText, m_filterTagText );
			return 1;
		}
		else if( m_rootKnob == k )
		{
			m_root = m_rootKnob->get_text();

			// Validate the root string by removing duplicate '/' and ensuring that it starts with a '/' but doesn't end with one.
			m_root.erase( std::unique( m_root.begin(), m_root.end(), bothSlashes ), m_root.end());

			if( m_root.size() > 1 && m_root[ m_root.size()-1 ] == '/' )
			{
				m_root = m_root.substr(0, m_root.size()-1);	
			}

			if( m_root.empty() )
			{
				m_root = std::string("/");
			}
			else if( m_root[0] != '/' )
			{
				m_root = std::string("/") + m_root;
			}

			// We would like the items in the SceneView_knob to be listed under the name of the root rather than it's full
			// path. This means that we need to modify all of the item strings that we pass to it by removing the unwanted
			// part of the path.
			// To make recovery of the full path easier we store the unwanted of the path as a member.
			IECore::SceneInterface::Path rootPath;
			IECore::SceneInterface::stringToPath( m_root, rootPath );
			if( rootPath.size() > 0 )
			{
				rootPath.erase( rootPath.end() );
			}
			IECore::SceneInterface::pathToString( rootPath, m_pathPrefix );

			// We keep the length of the unwanted path string so that we can use it to easily truncate the names of the items
			// that we use to populate the SceneView_knob.
			m_pathPrefixLength = m_pathPrefix.size();

			// Finally, update the UI with the validated string and rebuild the SceneView_knob.
			m_rootKnob->set_text( m_root.c_str() );

			rebuildSceneView();
			filterScene( m_filterText, m_filterTagText );

			return 1;
		}
		else if ( m_tagFilterKnob == k || m_sceneFilterKnob == k )
		{
			// As the filter expression or tag has changed, filter the scene again.
			if( m_sceneFilterKnob )
			{
				const char* c = m_sceneFilterKnob->get_text();
				if( c )
				{
					m_filterText = std::string( c );
				}
				else
				{
					m_filterText = "";
				}
			}
			
			// Get the tag's name.
			m_filterTagText = std::string("");
			if( m_tagFilterKnob )
			{
				tagSelection( m_filterTagText );
			}

			filterScene( m_filterText, m_filterTagText );
			
			return 1;
		}
		else if( m_sceneKnob == k )
		{
			rebuildSelection();
			return 1;
		}
		// This knob is only loaded when a script is pasted or loaded from a file.
		// As it is loaded last we know that the other knobs have already been set.
		// This means that we have enough information to build our internal data 
		// structures.
		else if( knob("loadAll") == k )
		{
			if( !m_scriptFinishedLoading )
			{
				m_scriptFinishedLoading = true;
			}

			return 1;
		}
		else if( std::string( k->name() ) == "hidePanel" || std::string( k->name() ) == "showPanel" )
		{
			if( !m_scriptFinishedLoading )
			{
				m_scriptFinishedLoading = true;
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
	if( !m_scriptFinishedLoading )
	{
		throw IECore::Exception( "SceneCacheReader: Cannot load item as the script hasn't finished loading." );
	}

	m_evaluatedFilePath	= filePath();

	SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );

	std::vector<unsigned int> selectionIndices;
	sceneView->getSelectedItems( selectionIndices );


	std::vector<unsigned int> filterIndices;
	sceneView->getImportedItems( filterIndices );

	std::vector<std::string> oldItems = sceneView->menu();

	// Load the scene cache file without any selection or filter.
	clearSceneViewSelection();
	rebuildSceneView();
	filterScene( "", "" );
	
	const std::vector<std::string> &items = sceneView->menu();
	
	// Remove any selected items which do not exist any longer. For all that do, get their index into the new SceneView_knob.
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
		
		newSelectionIndices.push_back( selectedIt - items.begin() ); 
	}

	// Select the new selection list.
	for( std::vector<unsigned int>::iterator it( newSelectionIndices.begin() ); it != newSelectionIndices.end(); ++it )
	{
		*it = m_itemToFiltered[*it];
	}
	
	sceneView->setSelectedItems( newSelectionIndices );
	rebuildSelection();

	// Finally force the filter of the scene again.
	m_filterHash = Hash();
	filterScene( m_filterText, m_filterTagText );
}

void SceneCacheReader::rebuildSelection()
{
	if( !m_scriptFinishedLoading )
	{
		return;
	}

	if( m_isFirstRun )
	{
		validate( false );
	}

	// We require an up-to-date scene so rebuild it if necessary.
	rebuildSceneView();

	// Only update the selection if the SceneView_knob exists.
	if( m_sceneKnob != NULL )
	{
		// Generate a hash from the SceneView_knob's selected entries.
		std::vector<unsigned int> selectionIndices;
		SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );
		sceneView->getSelectedItems( selectionIndices );

		Hash newSelectionHash( sceneHash() );

		for( std::vector<unsigned int>::const_iterator it( selectionIndices.begin() ); it != selectionIndices.end(); ++it )
		{
			newSelectionHash.append( m_filteredToItem[*it] );
		}

		// See if our selection has changed and if it has then update
		// our list of selected geometry. 
		if( m_selectionHash != newSelectionHash )
		{
			m_itemSelected.assign( m_itemSelected.size(), 0 );
			for( std::vector<unsigned int>::const_iterator it( selectionIndices.begin() ); it != selectionIndices.end(); ++it )
			{
				m_itemSelected[*it] = true;
			}
			
			m_selectionHash = newSelectionHash;
		}
	}
}

void SceneCacheReader::clearSceneViewSelection()
{
	if( m_sceneKnob != NULL )
	{
		std::vector<unsigned int> emptySelection;
		SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );
		sceneView->setSelectedItems( emptySelection );
		m_itemSelected.assign( m_itemSelected.size(), 0 );
		m_selectionHash = sceneHash();
	}
}

Hash SceneCacheReader::sceneHash() const
{
	if( m_filePathKnob == NULL || m_rootKnob == NULL || !m_filePath || !getSceneInterface() )
	{
		return Hash();
	}

	Hash newHash;
	newHash.append( m_evaluatedFilePath );
	newHash.append( m_filePath );
	newHash.append( m_root );
	return newHash;
}

void SceneCacheReader::rebuildSceneView()
{
	m_evaluatedFilePath	= filePath();

	if( !m_scriptFinishedLoading )
	{
		return;
	}

	if( m_isFirstRun )
	{
		validate( false );
	}

	Hash newSceneHash( sceneHash() );

	// Check to see if the scene has changed. If it has then we need to 
	// rebuild our internal representation of it.
	if( m_sceneHash != newSceneHash )
	{
		IECore::ConstSceneInterfacePtr sceneInterface = getSceneInterface();
		SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );

		// If we have a selection, clear it!
		if( m_itemSelected.size() != 0 )
		{			
			clearSceneViewSelection();
		}

		// Reset our internal data structures.
		m_tagMap.clear();
	
		// Clear the SceneView_knob.
		std::vector<std::string> sceneItems;
		sceneView->menu( sceneItems );
		
		// Validate our scene.
		if( !sceneInterface || !sceneView )
		{
			return;
		}

		// Rebuild our list of items which we will use to populate the SceneView_knob.	
		buildSceneView( sceneItems, sceneInterface );
		
		// Reset the list of selected entries and populate the SceneView_knob.
		m_itemSelected.clear();
		if( !sceneItems.empty() )
		{
			m_itemSelected.resize( sceneItems.size(), 0 );
			sceneView->addItems( sceneItems );
			updateTagFilterKnob();
		}

		// Set the new hash.
		m_sceneHash = newSceneHash;
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
	SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );
	const std::vector<std::string> &items( sceneView->menu() );
	return items[index];
}

// The purpose of the filterScene method is to both filter out unwanted items from the SceneView_knob
// and create a mapping between the indices of the resulting items and their position in the full
// list of items in the scene. We do this because when we query the SceneView_knob for the selected
// items, we are returned a list of indices within the filtered items. Therefore, to get the
// names of these items we need to use a LUT of filtered indices to indices within the list of names.
// There LUTs are the m_itemToFiltered and m_filteredToItem maps.
void SceneCacheReader::filterScene( const std::string &filterText, const std::string &filterTag )
{
	if( !m_scriptFinishedLoading )
	{
		return;
	}

	if( m_isFirstRun )
	{
		validate( false );
	}

	Hash newFilterHash( sceneHash() );
	newFilterHash.append( filterText );
	newFilterHash.append( filterTag );

	if( m_filterHash == newFilterHash )
	{
		return;
	}
	m_filterHash = newFilterHash;

	SceneView_KnobI *sceneView( m_sceneKnob->sceneViewKnob() );

	// Get the item indices of the currently selected items so that
	// we can add them to the newly filtered scene.
	std::vector<unsigned int> newSelection;
	sceneView->getSelectedItems( newSelection ); // Set the vector to the indices of the filtered items.
	for( std::vector<unsigned int>::iterator it( newSelection.begin() ); it != newSelection.end(); ++it )
	{
		*it = m_filteredToItem[*it]; // Convert the filter index to a item index.
	}	

	// Reset our LUTs as we are going to rebuild them.
	m_itemToFiltered.clear();	
	m_filteredToItem.clear();	

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
	
	if( expr != "/" && expr != "*" && expr != "" )
	{
		// Get the filter text that we will use to filter the scene.
		try
		{
			// Loop over all of the items in the scene and create a unique list of all entries whose name matches our regular expression.
			boost::regex expression( expr );
			const std::vector<std::string> &sceneItems = sceneView->menu();
			
			if( filterByTag ) // Filter by tag and expression
			{
				for( std::vector<unsigned int>::const_iterator it = m_tagMap[filterTag].begin(); it != m_tagMap[filterTag].end(); ++it )
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
		const std::vector<std::string> &sceneItems = sceneView->menu();
		filteredIndices.clear();
		if( !sceneItems.empty() )
		{
			if( filterByTag )
			{
				// Just filter the items with the chosen the tag.
				const std::vector<unsigned int> &tagIndices( m_tagMap[filterTag] );
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
			filteredIndices.push_back( *it );
		}

		// Make sure all indices in our selection are unique and sorted.
		std::set<unsigned int> selectionSet( filteredIndices.begin(), filteredIndices.end() );
		filteredIndices.clear();
		std::copy( selectionSet.begin(), selectionSet.end(), std::back_inserter(filteredIndices) );
		sort( filteredIndices.begin(), filteredIndices.end() );
	}

	m_filteredToItem = filteredIndices;

	// Restore the old selection.
	m_itemSelected.clear();
	if( !filteredIndices.empty() )
	{
		// Set the filtered items.
		sceneView->setImportedItems( filteredIndices );

		// Create a mapping of item indices to filtered indices.
		for( std::vector<unsigned int>::const_iterator it( filteredIndices.begin() ); it != filteredIndices.end(); ++it )
		{
			m_itemToFiltered[ *it ] = it - filteredIndices.begin();
		}

		m_itemSelected.resize( filteredIndices.size(), false );
		if( !newSelection.empty() )
		{
			for( std::vector<unsigned int>::const_iterator it( newSelection.begin() ); it != newSelection.end(); ++it )
			{
				m_itemSelected[ m_itemToFiltered[ *it ] ] = true;
			}
		}
	}
}

void SceneCacheReader::tagSelection( std::string &selection ) const
{
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
	if( m_tagFilterKnob )
	{
		std::vector<std::string> tagNames;
		tagNames.push_back( "None" );

		for( TagMap::const_iterator it = m_tagMap.begin(); it != m_tagMap.end(); ++it )
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

const IECore::InternedString &SceneCacheReader::geometryTag()
{
	static IECore::InternedString g_geometryTag( "ObjectType:MeshPrimitive" );
	return g_geometryTag;
}

void SceneCacheReader::buildSceneView( std::vector< std::string > &list, const IECore::ConstSceneInterfacePtr sceneInterface )
{
	if( sceneInterface )
	{
		//\todo: We currently only support mesh geomentry as there isn't an IECoreNuke curve or points primitive converter.
		// As a result we check that the object which we have encountered has a MeshPrimitive tag.
		// When IECoreNuke supports curves and points primitive converters, remove this assertion.
		if( sceneInterface->hasObject() && sceneInterface->hasTag( geometryTag() ) )
		{
			IECore::SceneInterface::NameList tagNames;
			sceneInterface->readTags( tagNames, IECore::SceneInterface::LocalTag );
			for ( IECore::SceneInterface::NameList::const_iterator it = tagNames.begin(); it != tagNames.end(); ++it )
			{
				m_tagMap[*it].push_back( list.size() );
			}

			IECore::SceneInterface::Path path;
			sceneInterface->path( path );
			std::string pathStr;
			IECore::SceneInterface::pathToString( path, pathStr );
		
			// The SceneView_knob requires that all entries belong to the same root item.
			// This is an issue as the SceneCache can have multiple entries at root level.
			// To resolve this issue we append "/root" to the item name when viewing the
			// tree at root level.
			if( m_root == "/" )
			{
				pathStr = std::string("/root") + pathStr;
			}
			else
			{
				pathStr = pathStr.substr( m_pathPrefixLength );
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
			buildSceneView( list, childScene );
		}
	}
}

void SceneCacheReader::append( DD::Image::Hash &hash )
{
	SourceGeo::append( hash );
	hash.append( m_sceneHash );
	hash.append( m_selectionHash );
	hash.append( m_evaluatedFilePath );
	hash.append( m_filePath );
	hash.append( m_root );
	hash.append( m_worldSpace );
	hash.append( outputContext().frame() );
}

void SceneCacheReader::get_geometry_hash()
{
	SourceGeo::get_geometry_hash();

	geo_hash[Group_Primitives].append( m_sceneHash );
	geo_hash[Group_Primitives].append( m_selectionHash );
	geo_hash[Group_Primitives].append( m_evaluatedFilePath );
	geo_hash[Group_Primitives].append( m_root );
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
		geo_hash[*g].append( m_sceneHash );
		geo_hash[*g].append( m_selectionHash );
		geo_hash[*g].append( m_evaluatedFilePath );
		geo_hash[*g].append( m_root );
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
	// Don't do any work if our hash hasn't changed.
	// this is important not only for speed, but also because
	// something in nuke assumes we won't change anything if
	// rebuilding isn't needed - we get crashes if we rebuild
	// when not necessary.
	if( !rebuild( Mask_Attributes ) && !rebuild( Mask_Matrix ) )
	{
		return;
	}

	if( !m_filePath || m_evaluatedFilePath == "" )
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
			for( std::vector<bool>::const_iterator it( m_itemSelected.begin() ); it != m_itemSelected.end(); ++it )
			{
				if( *it )
				{
					int index = it - m_itemSelected.begin();
					loadPrimitive( out, items[ m_filteredToItem[index] ] );
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
	std::string itemPath;
	if( m_root == "/" )
	{
		// Remove the "/root" prefix that was added to the path name.
		itemPath = path.substr( 5 ); // "/root" is 5 characters long...
	}
	else
	{
		// Add the prefix that we removed when creating the entry.
		itemPath = m_pathPrefix + path;
	}

	IECore::ConstSceneInterfacePtr sceneInterface( getSceneInterface( itemPath ) );
	if( sceneInterface )
	{
		double time = outputContext().frame() / 24.0;
		IECore::ConstObjectPtr object = sceneInterface->readObject( time );
		IECore::SceneInterface::Path rootPath;
		if( m_worldSpace )
		{
			IECore::SceneInterface::stringToPath( "/", rootPath );
		}
		else
		{
			IECore::SceneInterface::stringToPath( m_root, rootPath );
		}

		Imath::M44d transformd;
		transformd = worldTransform( sceneInterface, rootPath, time );
		IECore::TransformOpPtr transformer = new IECore::TransformOp();
		transformer->inputParameter()->setValue( const_cast< IECore::Object * >(object.get()) );	// safe const_cast because the Op will copy the input object.
		transformer->copyParameter()->setTypedValue( true );
		transformer->matrixParameter()->setValue( new IECore::M44dData( transformd ) );
		object = transformer->operate();
		
		IECoreNuke::ToNukeGeometryConverterPtr converter = IECoreNuke::ToNukeGeometryConverter::create( object );
		converter->convert( out );
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

IECore::ConstSceneInterfacePtr SceneCacheReader::getSceneInterface( const string &path ) const
{
	try
	{
		IECore::ConstSceneInterfacePtr scene = IECore::SharedSceneInterfaces::get( m_evaluatedFilePath );
		IECore::SceneInterface::Path itemPath;
		IECore::SceneInterface::stringToPath( path, itemPath );
		scene = scene->scene( itemPath );
		return scene;
	}
	catch( const std::exception &e )
	{
		return 0;
	}
}

IECore::ConstSceneInterfacePtr SceneCacheReader::getSceneInterface() const
{
	try
	{
		IECore::ConstSceneInterfacePtr scene = IECore::SharedSceneInterfaces::get( m_evaluatedFilePath );
		IECore::SceneInterface::Path rootPath;
		IECore::SceneInterface::stringToPath( m_root, rootPath );
		scene = scene->scene( rootPath );
		return scene;
	}
	catch( const std::exception &e )
	{
		return 0;
	}
}

