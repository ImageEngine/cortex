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

#include "IECoreHoudini/LiveScene.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

#include "IECoreScene/Group.h"

#include "IECore/TransformationMatrixData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBoxAlgo.h"
#include "OpenEXR/ImathMatrixAlgo.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "MGR/MGR_Node.h"
#include "MOT/MOT_Director.h"
#include "OBJ/OBJ_Node.h"
#include "OP/OP_Director.h"
#include "OP/OP_Input.h"
#include "UT/UT_Version.h"
#include "UT/UT_WorkArgs.h"

#include "boost/algorithm/string/join.hpp"

#include <string>
#include <unordered_set>
#include <vector>

#if UT_MAJOR_VERSION_INT >= 14

typedef GA_PrimitiveGroup GroupType;

#else

typedef GA_ElementGroup GroupType;

#endif

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

static InternedString contentName( "geo" );

PRM_Name LiveScene::pTags( "ieTags", "ieTags" );
static const UT_String tagGroupPrefix( "ieTag_" );

namespace
{

#if UT_MAJOR_VERSION_INT < 18
SOP_Node* renderNode( OBJ_Node* objNode )
{
	for (OP_Node *output : objNode->getOutputNodePtrs())
	{
		if (output->whichOutputNode() == 0)
		{
			if ( SOP_Node *sop = output->castToSOPNode() )
			{
				return sop;
			}
		}
	}
	return nullptr;
}
#endif

IECore::InternedString g_Tags( "tags" );

IECore::ConstObjectPtr removeTagsBlindData( IECore::ConstObjectPtr obj )
{
	if( auto blindDataHolder = IECore::runTimeCast<const IECore::BlindDataHolder>( obj ) )
	{
		auto blindDataHolderCopy = blindDataHolder->copy();
		blindDataHolderCopy->blindData()->writable().erase( g_Tags );
		return blindDataHolderCopy;

	}
	return obj;
}

void getUniquePrimitives(const GU_Detail *geo, std::set<std::string>& uniquePrimTypes)
{
	uniquePrimTypes.clear();
	const GA_PrimitiveList &primitives = geo->getPrimitiveList();

	GA_Offset start, end;
	for( GA_Iterator it( geo->getPrimitiveRange() ); it.blockAdvance( start, end ); )
	{
		for( GA_Offset offset = start; offset < end; ++offset )
		{
			const GA_Primitive *prim = primitives.get( offset );
			uniquePrimTypes.insert( prim->getTypeDef().getToken().toStdString() );
		}
	}

}
} // namespace

LiveScene::LiveScene() : m_rootIndex( 0 ), m_contentIndex( 0 ), m_defaultTime( std::numeric_limits<double>::infinity() )
{
	MOT_Director *motDirector = dynamic_cast<MOT_Director *>( OPgetDirector() );
	motDirector->getObjectManager()->getFullPath( m_nodePath );

	Path contentPath, rootPath;
	calculatePath( contentPath, rootPath );
}

LiveScene::LiveScene( const UT_String &nodePath, const Path &contentPath, const Path &rootPath, double defaultTime )
	: m_rootIndex( 0 ), m_contentIndex( 0 ), m_defaultTime( defaultTime )
{
	constructCommon( nodePath, contentPath, rootPath, nullptr );
}

LiveScene::LiveScene( const UT_String &nodePath, const Path &contentPath, const Path &rootPath, const LiveScene& parent )
	: m_rootIndex( 0 ), m_contentIndex( 0 ), m_splitter( parent.m_splitter ), m_defaultTime( parent.m_defaultTime )
{
	constructCommon( nodePath, contentPath, rootPath, m_splitter.get() );
}

void LiveScene::constructCommon( const UT_String &nodePath, const Path &contentPath, const Path &rootPath, DetailSplitter *splitter )
{
	m_nodePath = nodePath;
	m_nodePath.hardenIfNeeded();

	if ( OP_Node *contentNode = locateContent( retrieveNode() ) )
	{
		if ( !m_splitter )
		{
			double adjustedTime = adjustedDefaultTime();
			m_splitter = new DetailSplitter( contentNode->castToOBJNode(), adjustedTime , /* key */ "name", /* useHoudiniSegment */ false);
		}
	}

	calculatePath( contentPath, rootPath );
}

const OP_Node *LiveScene::node() const
{
	return retrieveNode( false, NullIfMissing );
}

bool LiveScene::embedded() const
{
	return m_contentIndex;
}

double LiveScene::adjustedDefaultTime() const
{
	if ( m_defaultTime == std::numeric_limits<double>::infinity() )
	{
		return adjustTime( CHgetEvalTime() );
	}

	return adjustTime( m_defaultTime );
}

double LiveScene::getDefaultTime() const
{
	return m_defaultTime;
}

void LiveScene::setDefaultTime( double time )
{
	m_defaultTime = time;
}

std::string LiveScene::fileName() const
{
	throw Exception( "IECoreHoudini::LiveScene does not support fileName()." );
}

SceneInterface::Name LiveScene::name() const
{
	if ( m_path.empty() || m_rootIndex == m_path.size() )
	{
		return SceneInterface::rootName;
	}

	return *m_path.rbegin();
}

void LiveScene::path( Path &p ) const
{
	p.resize( m_path.size() - m_rootIndex );
	std::copy( m_path.begin() + m_rootIndex, m_path.end(), p.begin() );
}

void LiveScene::calculatePath( const Path &contentPath, const Path &rootPath )
{
	OP_Node *node = retrieveNode();
	if ( node->isManager() )
	{
		return;
	}

	UT_String tmp( m_nodePath );
	UT_WorkArgs workArgs;
	tmp.tokenize( workArgs, "/" );

	OP_Node *current = dynamic_cast<MOT_Director *>( OPgetDirector() )->getObjectManager();
	// skipping the token for the OBJ manager
	for ( int i = 1; i < workArgs.getArgc(); ++i )
	{
		current = current->getChild( workArgs[i] );

		/// recursively collect all input connections
		OP_Node *parent = current->getInput( 0 );
		UT_StringArray parentNames;
		while ( parent )
		{
			parentNames.append( parent->getName() );
			parent = parent->getInput( 0 );
		}

		// add them in reverse order
		for ( int j = parentNames.entries() - 1; j >= 0; --j )
		{
			m_path.push_back( Name( parentNames( j ) ) );
		}

		if ( ( i < workArgs.getArgc() - 1 ) || Name( workArgs[i] ) != contentName )
		{
			m_path.push_back( Name( workArgs[i] ) );
		}
	}

	if ( !contentPath.empty() )
	{
		m_contentIndex = m_path.size();
		m_path.resize( m_path.size() + contentPath.size() );
		std::copy( contentPath.begin(), contentPath.end(), m_path.begin() + m_contentIndex );
	}

	if ( m_path.size() < rootPath.size() )
	{
		std::string pStr, rStr;
		pathToString( m_path, pStr );
		pathToString( rootPath, rStr );
		throw Exception( "IECoreHoudini::LiveScene: Path \"" + pStr + "\" is not a valid child of root \"" + rStr + "\"." );
	}

	for ( size_t i = 0; i < rootPath.size(); ++i )
	{
		if ( rootPath[i] != m_path[i] )
		{
			std::string pStr, rStr;
			pathToString( m_path, pStr );
			pathToString( rootPath, rStr );
			throw Exception( "IECoreHoudini::LiveScene: Path \"" + pStr + "\" is not a valid child of root \"" + rStr + "\"." );
		}
	}

	m_rootIndex = rootPath.size();
}

Imath::Box3d LiveScene::readBound( double time ) const
{
	OP_Node *node = retrieveNode( true );

	Imath::Box3d bounds;
	UT_BoundingBox box;
	OP_Context context( adjustTime( time ) );
	/// \todo: this doesn't account for SOPs containing multiple shapes
	/// if we fix it, we need to fix the condition below as well
	if ( node->getBoundingBox( box, context ) )
	{
		bounds = IECore::convert<Imath::Box3d>( box );
	}

	// paths embedded within a sop already have bounds accounted for
	if ( m_contentIndex )
	{
		return bounds;
	}

	NameList children;
	childNames( children );
	for ( NameList::iterator it=children.begin(); it != children.end(); ++it )
	{
		ConstSceneInterfacePtr childScene = child( *it );
		Imath::Box3d childBound = childScene->readBound( time );
		if ( !childBound.isEmpty() )
		{
			bounds.extendBy( Imath::transform( childBound, childScene->readTransformAsMatrix( time ) ) );
		}
	}

	return bounds;
}

void LiveScene::writeBound( const Imath::Box3d &bound, double time )
{
	throw Exception( "IECoreHoudini::LiveScene is read-only" );
}

ConstDataPtr LiveScene::readTransform( double time ) const
{
	Imath::V3d s, h, r, t;
	Imath::M44d matrix = readTransformAsMatrix( time );
	Imath::extractSHRT( matrix, s, h, r, t, true );

	return new TransformationMatrixdData( TransformationMatrixd( s, r, t ) );
}

Imath::M44d LiveScene::readTransformAsMatrix( double time ) const
{
	OP_Node *node = retrieveNode();
	if ( node->isManager() )
	{
		return Imath::M44d();
	}

	OBJ_Node *objNode = node->castToOBJNode();
	if ( !objNode )
	{
		return Imath::M44d();
	}

	// paths embedded within a sop always have identity transforms
	if ( m_contentIndex )
	{
		return Imath::M44d();
	}

	UT_DMatrix4 matrix;
	OP_Context context( adjustTime( time ) );
	if ( !objNode->getParmTransform( context, matrix ) )
	{
		return Imath::M44d();
	}

	return IECore::convert<Imath::M44d>( matrix );
}

ConstDataPtr LiveScene::readWorldTransform( double time ) const
{
	Imath::V3d s, h, r, t;
	Imath::M44d matrix = readWorldTransformAsMatrix( time );
	Imath::extractSHRT( matrix, s, h, r, t, true );

	return new TransformationMatrixdData( TransformationMatrixd( s, r, t ) );
}

Imath::M44d LiveScene::readWorldTransformAsMatrix( double time ) const
{
	OP_Node *node = retrieveNode();
	if ( node->isManager() )
	{
		return Imath::M44d();
	}

	OBJ_Node *objNode = node->castToOBJNode();
	if ( !objNode )
	{
		return Imath::M44d();
	}

	UT_DMatrix4 matrix;
	OP_Context context( adjustTime( time ) );
	if ( !objNode->getWorldTransform( matrix, context ) )
	{
		return Imath::M44d();
	}

	return IECore::convert<Imath::M44d>( matrix );
}

void LiveScene::writeTransform( const Data *transform, double time )
{
	throw Exception( "IECoreHoudini::LiveScene is read-only" );
}

bool LiveScene::hasAttribute( const Name &name ) const
{
	OP_Node *node = retrieveNode();

	const std::vector<CustomAttributeReader> &attributeReaders = customAttributeReaders();
	for ( std::vector<CustomAttributeReader>::const_iterator it = attributeReaders.begin(); it != attributeReaders.end(); ++it )
	{
		if ( embedded() && !it->m_callEmbedded )
		{
			continue;
		}

		NameList names;
		it->m_names( node, names );
		if ( std::find( names.begin(), names.end(), name ) != names.end() )
		{
			return true;
		}
	}

	return false;
}

void LiveScene::attributeNames( NameList &attrs ) const
{
	attrs.clear();

	OP_Node *node = retrieveNode();

	const std::vector<CustomAttributeReader> &attributeReaders = customAttributeReaders();
	for ( std::vector<CustomAttributeReader>::const_iterator it = attributeReaders.begin(); it != attributeReaders.end(); ++it )
	{
		NameList names;
		if ( embedded() && !it->m_callEmbedded )
		{
			continue;
		}

		it->m_names( node, names );
		/// \todo: investigate using a set here if performance becomes an issue
		for ( NameList::const_iterator nIt = names.begin(); nIt != names.end(); ++nIt )
		{
			if ( std::find( attrs.begin(), attrs.end(), *nIt ) == attrs.end() )
			{
				attrs.push_back( *nIt );
			}
		}
	}
}

ConstObjectPtr LiveScene::readAttribute( const Name &name, double time ) const
{
	OP_Node *node = retrieveNode();

	// iterate attribute readers in reverse order so the ones registered later take precedence:
	const std::vector<CustomAttributeReader> &attributeReaders = customAttributeReaders();
	for ( std::vector<CustomAttributeReader>::const_reverse_iterator it = attributeReaders.rbegin(); it != attributeReaders.rend(); ++it )
	{
		if ( embedded() && !it->m_callEmbedded )
		{
			continue;
		}

		if ( IECore::ConstObjectPtr object = it->m_read( node, name, time ) )
		{
			return object;
		}
	}

	return nullptr;
}

void LiveScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw Exception( "IECoreHoudini::LiveScene is read-only" );
}

bool LiveScene::hasTag( const Name &name, int filter ) const
{
	const OP_Node *node = retrieveNode();
	if ( !node )
	{
		return false;
	}

	if ( filter & SceneInterface::LocalTag )
	{
		// check for user supplied tags if we're not inside a SOP
		if ( !m_contentIndex && node->hasParm( pTags.getToken() ) )
		{
			UT_String parmTags;
			node->evalString( parmTags, pTags.getToken(), 0, 0 );
			if ( UT_String( name.c_str() ).multiMatch( parmTags ) )
			{
				return true;
			}
		}
	}

	// check with the registered tag readers
	std::vector<CustomTagReader> &tagReaders = customTagReaders();
	for ( std::vector<CustomTagReader>::const_iterator it = tagReaders.begin(); it != tagReaders.end(); ++it )
	{
		if ( embedded() && !it->m_callEmbedded )
		{
			continue;
		}

		if ( it->m_has( node, name, filter ) )
		{
			return true;
		}
	}

	if ( filter & SceneInterface::LocalTag )
	{
		// check tags based on primitive groups
		OBJ_Node *contentNode = retrieveNode( true )->castToOBJNode();
		if ( contentNode && contentNode->getObjectType() == OBJ_GEOMETRY )
		{
			std::string pathStr;
			SceneInterface::Path path;
			relativeContentPath( path );
			pathToString( path, pathStr );

			if ( m_splitter )
			{
				if( auto splitObject = runTimeCast<Primitive>( m_splitter->splitObject( pathStr ) ) )
				{
					const auto &readableBlindData = splitObject->blindData()->readable();
					auto tagsIt = readableBlindData.find( g_Tags );
					if( tagsIt == readableBlindData.end() )
					{
						return false;
					}
					const IECore::InternedStringVectorData *tagsVector = runTimeCast<const IECore::InternedStringVectorData>( tagsIt->second.get() );

					if( !tagsVector )
					{
						return false;
					}
					const auto &readableTagsVector = tagsVector->readable();

					return std::find( readableTagsVector.begin(), readableTagsVector.end(), name ) != readableTagsVector.end();
				}
			}

			GU_DetailHandle newHandle = contentHandle();
			if ( !newHandle.isNull() )
			{
				GU_DetailHandleAutoReadLock readHandle( newHandle );
				if ( const GU_Detail *geo = readHandle.getGdp() )
				{
					GA_Range prims = geo->getPrimitiveRange();

					for ( GA_GroupTable::iterator<GroupType> it=geo->primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
					{
						GA_PrimitiveGroup *group = static_cast<GA_PrimitiveGroup*>( it.group() );
						if ( group->getInternal() || group->isEmpty() )
						{
							continue;
						}

						const UT_String groupName = group->getName().c_str();
						if ( groupName.startsWith( tagGroupPrefix ) && group->containsAny( prims ) )
						{
							UT_String tag;
							groupName.substr( tag, tagGroupPrefix.length() );
							tag.substitute( "_", ":" );
							if ( tag.equal( name.c_str() ) )
							{
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

void LiveScene::readTags( NameList &tags, int filter ) const
{
	tags.clear();

	const OP_Node *node = retrieveNode();

	if ( !node )
	{
		return;
	}

	std::unordered_set< Name > uniqueTags;

	if ( filter & SceneInterface::LocalTag )
	{
		// add user supplied tags if we're not inside a SOP
		if ( !m_contentIndex && node->hasParm( pTags.getToken() ) )
		{
			UT_String parmTagStr;
			node->evalString( parmTagStr, pTags.getToken(), 0, 0 );
			if ( !parmTagStr.equal( UT_String::getEmptyString() ) )
			{
				UT_WorkArgs tokens;
				parmTagStr.tokenize( tokens, " " );
				for ( int i = 0; i < tokens.getArgc(); ++i )
				{
					uniqueTags.insert( tokens[i] );
				}
			}
		}
	}

	// add tags from the registered tag readers
	std::vector<CustomTagReader> &tagReaders = customTagReaders();
	for ( std::vector<CustomTagReader>::const_iterator it = tagReaders.begin(); it != tagReaders.end(); ++it )
	{
		NameList values;
		if ( embedded() && !it->m_callEmbedded )
		{
			continue;
		}

		it->m_read( node, values, filter );
		uniqueTags.insert( values.begin(), values.end() );
	}

	if ( filter & SceneInterface::LocalTag )
	{

		// add tags based on primitive groups
		OBJ_Node *contentNode = retrieveNode( true )->castToOBJNode();
		if ( contentNode && contentNode->getObjectType() == OBJ_GEOMETRY && m_splitter )
		{

			std::string pathStr;
			SceneInterface::Path path;
			relativeContentPath( path );
			pathToString( path, pathStr );

			if( auto splitObject = runTimeCast<Primitive>( m_splitter->splitObject( pathStr ) ) )
			{
				const auto &readableBlindData = splitObject->blindData()->readable();
				auto tagsIt = readableBlindData.find( g_Tags );
				if( tagsIt != readableBlindData.end() )
				{
					if( const IECore::InternedStringVectorData *tagsVector = runTimeCast<const IECore::InternedStringVectorData>( tagsIt->second.get() ) )
					{
						const auto &readableTagsVector = tagsVector->readable();
						uniqueTags.insert( readableTagsVector.begin(), readableTagsVector.end() );
					}
				}
			}
			else
			{
				GU_DetailHandle newHandle = contentHandle();
				if( !newHandle.isNull() )
				{
					GU_DetailHandleAutoReadLock readHandle( newHandle );
					if( const GU_Detail *geo = readHandle.getGdp() )
					{
						GA_Range prims = geo->getPrimitiveRange();

						for( GA_GroupTable::iterator<GroupType> it = geo->primitiveGroups().beginTraverse(); !it.atEnd(); ++it )
						{
							GA_PrimitiveGroup *group = static_cast<GA_PrimitiveGroup *>( it.group() );
							if( group->getInternal() || group->isEmpty() )
							{
								continue;
							}

							const UT_String groupName( group->getName() );
							if( groupName.startsWith( tagGroupPrefix ) && group->containsAny( prims ) )
							{
								UT_String tag;
								groupName.substr( tag, tagGroupPrefix.length() );
								tag.substitute( "_", ":" );
								uniqueTags.insert( tag.buffer() );
							}
						}
					}
				}
			}
		}
	}

	tags.insert( tags.end(), uniqueTags.begin(), uniqueTags.end() );
}

void LiveScene::writeTags( const NameList &tags )
{
	throw Exception( "IECoreHoudini::LiveScene::writeTags not supported" );
}



SceneInterface::NameList LiveScene::setNames( bool includeDescendantSets ) const
{
	return SceneInterface::NameList();
}

IECore::PathMatcher LiveScene::readSet( const Name &name, bool includeDescendantSets, const IECore::Canceller *canceller ) const
{
	throw Exception( "IECoreHoudini::LiveScene::readSet not supported" );
}

void LiveScene::writeSet( const Name &name, const IECore::PathMatcher &set )
{
	throw Exception( "IECoreHoudini::LiveScene::writeSet not supported" );
}

void LiveScene::hashSet( const Name& setName, IECore::MurmurHash &h ) const
{
	SceneInterface::hashSet( setName, h );
}


static const char *emptyString = "";

bool LiveScene::hasObject() const
{
	OP_Node *node = retrieveNode( true );
	if ( node->isManager() )
	{
		return false;
	}

	OBJ_Node *objNode = node->castToOBJNode();
	if ( !objNode )
	{
		return false;
	}

	OBJ_OBJECT_TYPE type = objNode->getObjectType();
	if ( type == OBJ_GEOMETRY  )
	{
		OP_Context context( adjustedDefaultTime() );
#if UT_MAJOR_VERSION_INT >= 18
		const GU_Detail *geo = objNode->getRenderGeometry( context, false );
#else
		SOP_Node* sopNode = renderNode( objNode );
		const GU_Detail *geo = ( sopNode ) ? sopNode->getCookedGeo( context, false ) : objNode->getRenderGeometry( context, false );
#endif
		if ( !geo )
		{
			return false;
		}

		IECoreScene::SceneInterface::Path parentPath ( m_path.begin() + m_contentIndex, m_path.end());

		const IECoreScene::SceneInterface::Path &path = m_contentIndex ? parentPath : IECoreScene::SceneInterface::rootPath;

		if ( m_splitter && m_splitter->hasPaths() )
		{
			return m_splitter->hasPath( path );
		}
		else
		{
#if UT_MAJOR_VERSION_INT >= 18
			GU_DetailHandle handle = objNode->getRenderGeometryHandle( context, false );
#else
			GU_DetailHandle handle = ( sopNode ) ? sopNode->getCookedGeoHandle( context, false ) : objNode->getRenderGeometryHandle( context, false );
#endif
			GU_DetailHandle newHandle = contentHandle();

			FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( ( newHandle.isNull() ) ? handle : newHandle );
			bool hasConvertableGeometry = converter != nullptr;

			if ( !hasConvertableGeometry )
			{
				// display some diagnostics about why this SOP cannot be converted.

				std::set<std::string> uniquePrimTypes;
				getUniquePrimitives( geo, uniquePrimTypes );

				throw IECore::Exception(
					boost::str(
						boost::format( "Error converting SOP: '%1%' to scc. Potentially unsupported prim types found: [ %2% ]" ) %
							objNode->getFullPath().c_str() %
							boost::algorithm::join( uniquePrimTypes, ", " )
					)
				);
			}

			return hasConvertableGeometry;
		}
	}

	/// \todo: need to account for OBJ_CAMERA and OBJ_LIGHT

	return false;
}

ConstObjectPtr LiveScene::readObject( double time, const IECore::Canceller *canceller ) const
{
	OBJ_Node *objNode = retrieveNode( true )->castToOBJNode();
	if ( !objNode )
	{
		return nullptr;
	}

	if ( objNode->getObjectType() == OBJ_GEOMETRY )
	{
		double adjustedTime =  adjustTime( time );
		OP_Context context( adjustedTime );
#if UT_MAJOR_VERSION_INT >= 18
		GU_DetailHandle handle = objNode->getRenderGeometryHandle( context, false );
#else
		SOP_Node* sopNode = renderNode( objNode );
		GU_DetailHandle handle = ( sopNode ) ? sopNode->getCookedGeoHandle( context, false ) : objNode->getRenderGeometryHandle( context, false );
#endif
		if ( !handle )
		{
			return nullptr;
		}

		if ( !m_splitter )
		{
			m_splitter = new DetailSplitter( objNode, adjustedTime, /* key */ "name", /* useHoudiniSegment */ false);
		}
		else
		{
			m_splitter->update( objNode, adjustedTime );
		}

		std::string name;
		SceneInterface::Path path;
		relativeContentPath( path );
		pathToString( path, name );

		if (auto obj = m_splitter->splitObject( name ))
		{
			return removeTagsBlindData( obj );
		}

		GU_DetailHandle newHandle = contentHandle();
		FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( ( newHandle.isNull() ) ? handle : newHandle );
		if ( !converter )
		{
			Path path;
			this->path( path );
			pathToString( path, name );

			GU_DetailHandleAutoReadLock readHandle( newHandle );
			std::set<std::string> uniquePrimTypes;
			getUniquePrimitives( readHandle.getGdp(), uniquePrimTypes );

			throw IECore::Exception
			(
				boost::str( boost::format( "Unable to split geometry. [%1%] prim types have the same name" ) % boost::algorithm::join( uniquePrimTypes, ", " ) )
			);
		}

		return converter->convert();
	}

	/// \todo: need to account for cameras and lights

	return nullptr;
}

PrimitiveVariableMap LiveScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	// \todo Optimize this function, adding special cases such as for Meshes.
	ConstPrimitivePtr prim = runTimeCast< const Primitive >( readObject( time ) );
	if ( !prim )
	{
		throw Exception( "Object does not have primitive variables!" );
	}
	return prim->variables;
}

void LiveScene::writeObject( const Object *object, double time )
{
	throw Exception( "IECoreHoudini::LiveScene is read-only" );
}

void LiveScene::childNames( NameList &childNames ) const
{
	OP_Node *node = retrieveNode();
	OBJ_Node *objNode = node->castToOBJNode();
	OBJ_Node *contentNode = retrieveNode( true )->castToOBJNode();

	// add subnet children
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OBJ_Node *child = node->getChild( i )->castToOBJNode();

			// ignore children that have incoming connections, as those are actually grandchildren
			// also ignore the contentNode, which is actually an extension of ourself
			if ( child && child != contentNode && !hasInput( child ) )
			{
				childNames.push_back( Name( child->getName() ) );
			}
		}
	}

	if ( !contentNode )
	{
		return;
	}

#if UT_MAJOR_VERSION_INT >= 16

	// add connected outputs
	OP_NodeList childList;
	contentNode->getOutputNodes(childList);
	for ( OP_Node * child : childList)
	{
		childNames.push_back( Name( child->getName() ) );
	}

#else

	// add connected outputs
	for ( unsigned i=0; i < contentNode->nOutputs(); ++i )
	{
		childNames.push_back( Name( contentNode->getOutput( i )->getName() ) );
	}

#endif


	// add child shapes within the geometry
	if ( contentNode->getObjectType() != OBJ_GEOMETRY )
	{
		return;
	}

	IECoreScene::SceneInterface::Path parentPath ( m_path.begin() + m_contentIndex, m_path.end());

	if ( m_splitter )
	{
		DetailSplitter::Names names = m_splitter->getNames( m_contentIndex ? parentPath : IECoreScene::SceneInterface::rootPath );

		for( const auto &c : names )
		{
			childNames.push_back( c );
		}
	}

}

bool LiveScene::hasChild( const Name &name ) const
{
	Path contentPath;
	return (bool)retrieveChild( name, contentPath, NullIfMissing );
}

SceneInterfacePtr LiveScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	Path contentPath;
	OP_Node *child = retrieveChild( name, contentPath, missingBehaviour );
	if ( !child )
	{
		return 0;
	}

	UT_String nodePath;
	child->getFullPath( nodePath );

	Path rootPath;
	rootPath.resize( m_rootIndex );
	std::copy( m_path.begin(), m_path.begin() + m_rootIndex, rootPath.begin() );

	/// \todo: is this really what we want? can we just pass rootIndex and contentIndex instead?
	return duplicate( nodePath, contentPath, rootPath);
}

ConstSceneInterfacePtr LiveScene::child( const Name &name, MissingBehaviour missingBehaviour ) const
{
	return const_cast<LiveScene *>( this )->child( name, missingBehaviour );
}

SceneInterfacePtr LiveScene::createChild( const Name &name )
{
	throw Exception( "IECoreHoudini::LiveScene is read-only" );
}

ConstSceneInterfacePtr LiveScene::scene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	return retrieveScene( path, missingBehaviour );
}

SceneInterfacePtr LiveScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	return retrieveScene( path, missingBehaviour );
}

void LiveScene::hash( HashType hashType, double time, MurmurHash &h ) const
{
	throw Exception( "Hashes currently not supported in IECoreHoudini::LiveScene objects." );
}

OP_Node *LiveScene::retrieveNode( bool content, MissingBehaviour missingBehaviour ) const
{
	OP_Node *node = OPgetDirector()->findNode( m_nodePath );
	if ( node && content )
	{
		if ( OP_Node *contentNode = locateContent( node ) )
		{
			node = contentNode;
		}
	}

	if ( missingBehaviour == ThrowIfMissing )
	{
		if ( !node )
		{
			throw Exception( "IECoreHoudini::LiveScene: Node \"" + m_nodePath.toStdString() + "\" no longer exists." );
		}

		if ( !node->isManager() && !node->castToOBJNode() )
		{
			throw Exception( "IECoreHoudini::LiveScene: Node \"" + m_nodePath.toStdString() + "\" is not a valid OBJ." );
		}
	}

	return node;
}

OP_Node *LiveScene::locateContent( OP_Node *node ) const
{
	OBJ_Node *objNode = node->castToOBJNode();
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OP_Node *child = node->getChild( i );
			if ( child->getName().equal( contentName.c_str() ) )
			{
				return child;
			}
		}
	}
	else if ( objNode && objNode->getObjectType() == OBJ_GEOMETRY )
	{
		return objNode;
	}

	return 0;
}

OP_Node *LiveScene::retrieveChild( const Name &name, Path &contentPath, MissingBehaviour missingBehaviour ) const
{
	OP_Node *node = retrieveNode( false, missingBehaviour );
	OP_Node *contentBaseNode = retrieveNode( true, missingBehaviour );
	if ( !node || !contentBaseNode )
	{
		return 0;
	}

	OBJ_Node *objNode = node->castToOBJNode();
	OBJ_Node *contentNode = contentBaseNode->castToOBJNode();

	// check subnet children
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OBJ_Node *child = node->getChild( i )->castToOBJNode();
			// the contentNode is actually an extension of ourself
			if ( child == contentNode )
			{
				continue;
			}

			if ( child && child->getName().equal( name.c_str() ) && !hasInput( child ) )
			{
				return child;
			}
		}
	}

	if ( contentNode )
	{
		// check connected outputs
#if UT_MAJOR_VERSION_INT >= 16
		OP_NodeList childList;
		contentNode->getOutputNodes(childList);
		for ( OP_Node * child : childList)
		{
			if ( child->getName().equal( name.c_str() ) )
			{
				return child;
			}
		}
#else
		for ( unsigned i=0; i < contentNode->nOutputs(); ++i )
		{
			OP_Node *child = contentNode->getOutput( i );
			if ( child->getName().equal( name.c_str() ) )
			{
				return child;
			}
		}
#endif
		// check child shapes within the geo
		if ( contentNode->getObjectType() == OBJ_GEOMETRY )
		{
			OP_Context context( adjustedDefaultTime() );

			IECoreScene::SceneInterface::Path parentPath ( m_path.begin() + m_contentIndex, m_path.end());

			parentPath = m_contentIndex ? parentPath : IECoreScene::SceneInterface::rootPath;
			parentPath.push_back(name);

			if ( m_splitter->hasPath( parentPath, false ) )
			{
				size_t contentSize = ( m_contentIndex ) ? m_path.size() - m_contentIndex : 0;
				if ( contentSize )
				{
					contentPath.resize( contentSize );
					std::copy( m_path.begin() + m_contentIndex, m_path.end(), contentPath.begin() );
				}

				contentPath.push_back( name );

				return contentNode;
			}
		}
	}

	if ( missingBehaviour == SceneInterface::ThrowIfMissing )
	{
		Path p;
		path( p );
		std::string pStr;
		pathToString( p, pStr );
		throw Exception( "IECoreHoudini::LiveScene::retrieveChild: Path \"" + pStr + "\" has no child named " + name.string() + "." );
	}

	return 0;
}

SceneInterfacePtr LiveScene::retrieveScene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	Path rootPath, emptyPath;
	rootPath.resize( m_rootIndex );
	std::copy( m_path.begin(), m_path.begin() + m_rootIndex, rootPath.begin() );

	LiveScenePtr rootScene = create();
	rootScene->setDefaultTime( m_defaultTime );
	for ( Path::const_iterator it = rootPath.begin(); it != rootPath.end(); ++it )
	{
		rootScene = IECore::runTimeCast<LiveScene>( rootScene->child( *it ) );
		if ( !rootScene )
		{
			return 0;
		}
	}

	UT_String rootNodePath;
	OP_Node *node = rootScene->retrieveNode();
	if ( !node )
	{
		return 0;
	}
	node->getFullPath( rootNodePath );

	/// \todo: is this really what we want? can we just pass rootIndex and contentIndex instead?
	SceneInterfacePtr scene = duplicate( rootNodePath, emptyPath, rootPath);
	for ( Path::const_iterator it = path.begin(); it != path.end(); ++it )
	{
		scene = scene->child( *it, missingBehaviour );
		if ( !scene )
		{
			return 0;
		}
	}

	return scene;
}

bool LiveScene::hasInput( const OP_Node *node ) const
{
	int numInputs = node->nInputs();
	for ( int j=0; j < numInputs; ++j )
	{
		OP_Input *input = node->getInputReferenceConst( j );
		if ( input && !input->isIndirect() )
		{
			return true;
		}
	}

	return false;
}

double LiveScene::adjustTime( double time ) const
{
	return time - CHgetManager()->getSecsPerSample();
}

bool LiveScene::matchPattern( const char *value, const char *pattern ) const
{
	size_t size = strlen( pattern );

	// can't be a match unless its exactly the right length
	if ( ( strlen( value ) < size  ) || ( strlen( value ) > size && value[size] != '\0' && value[size] != '/' ) )
	{
		return false;
	}

	// all characters must match
	for ( size_t i = 0; i < size; ++i )
	{
		if ( value[i] != pattern[i] )
		{
			return false;
		}
	}

	return true;
}

const char *LiveScene::matchPath( const char *value ) const
{
	// looking for empty path
	if ( !m_contentIndex )
	{
		// houdini returns 0 for empty strings in some cases
		if ( value == 0 || !value[0] || !strcmp( value, "/" ) )
		{
			return emptyString;
		}

		return &value[0];
	}

	// looking for some value, so empty is a failed match
	if ( value == 0 )
	{
		return NULL;
	}

	size_t i = 0;
	for ( Path::const_iterator it = m_path.begin() + m_contentIndex; it != m_path.end(); ++it )
	{
		const char *current = it->c_str();

		if ( value[i] == '/' )
		{
			i++;
		}

		if ( !matchPattern( &value[i], current ) )
		{
			return NULL;
		}

		i += strlen( current );
	}

	return &value[i];
}

std::pair<const char *, size_t> LiveScene::nextWord( const char *value ) const
{
	std::pair<const char *, size_t> result( value, 0 );

	if ( value[0] == '/' )
	{
		result.first = &value[1];
		result.second = 1;
	}

	size_t size = strlen( value );
	for ( ; result.second < size; ++result.second )
	{
		if ( value[result.second] == '/' || value[result.second] == '\0' )
		{
			result.second--;
			break;
		}
	}

	return result;
}

void LiveScene::relativeContentPath( SceneInterface::Path &path ) const
{
	path.clear();

	if ( !m_contentIndex )
	{
		return;
	}

	path.reserve( m_path.size() - m_contentIndex );
	path.insert( path.begin(), m_path.begin() + m_contentIndex, m_path.end() );
}

GU_DetailHandle LiveScene::contentHandle() const
{
	std::string name;
	SceneInterface::Path path;
	relativeContentPath( path );
	pathToString( path, name );

	GU_DetailHandle handle = m_splitter->split( name.c_str() );

	// we need to try again, in case the user didn't use a / prefix on the shape name
	if ( handle.isNull() && m_contentIndex == 1 && !path.empty() && path[0] != "" )
	{
		handle = m_splitter->split( &name.c_str()[1] );
	}

	return handle;
}

void LiveScene::registerCustomAttributes( ReadNamesFn namesFn, ReadAttrFn readFn, bool callEmbedded )
{
	CustomAttributeReader r;
	r.m_names = namesFn;
	r.m_read = readFn;
	r.m_callEmbedded = callEmbedded;
	customAttributeReaders().push_back( r );
}

std::vector<LiveScene::CustomAttributeReader> &LiveScene::customAttributeReaders()
{
	static std::vector<LiveScene::CustomAttributeReader> readers;
	return readers;
}

void LiveScene::registerCustomTags( HasTagFn hasFn, ReadTagsFn readFn, bool callEmbedded )
{
	CustomTagReader r;
	r.m_has = hasFn;
	r.m_read = readFn;
	r.m_callEmbedded = callEmbedded;
	customTagReaders().push_back( r );
}

std::vector<LiveScene::CustomTagReader> &LiveScene::customTagReaders()
{
	static std::vector<LiveScene::CustomTagReader> readers;
	return readers;
}

LiveScenePtr LiveScene::create() const
{
	return new LiveScene();
}

LiveScenePtr LiveScene::duplicate( const UT_String &nodePath, const Path &contentPath, const Path &rootPath) const
{
	return new LiveScene( nodePath, contentPath, rootPath, *this);
}

