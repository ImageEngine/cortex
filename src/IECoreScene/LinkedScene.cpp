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

#include "IECoreScene/LinkedScene.h"

#include "IECoreScene/SceneCache.h"
#include "IECoreScene/SharedSceneInterfaces.h"

#include "IECore/FileIndexedIO.h"
#include "IECore/MessageHandler.h"

#include "boost/foreach.hpp"

#include <set>
using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( LinkedScene )

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

// Register FileIndexedIO as the handler for .lsc files so that people
// can use the filename constructor as a convenience over the IndexedIO
// constructor.
static IndexedIO::Description<FileIndexedIO> extensionDescription( ".lscc" );
// Register the .scc extension in the factory function for LinkedScene on Read and Write modes
static SceneInterface::FileFormatDescription<LinkedScene> registrar(".lscc", IndexedIO::Read | IndexedIO::Write);

const SceneInterface::Name &LinkedScene::linkAttribute = InternedString( "sceneInterface:link" );

const SceneInterface::Name &LinkedScene::fileNameLinkAttribute("sceneInterface:link.fileName");
const SceneInterface::Name &LinkedScene::rootLinkAttribute("sceneInterface:link.root");
const SceneInterface::Name &LinkedScene::timeLinkAttribute("sceneInterface:link.time");

const InternedString LinkedScene::g_fileName("fileName");
const InternedString LinkedScene::g_root("root");
const InternedString LinkedScene::g_time("time");

LinkedScene::LinkedScene( const std::string &fileName, IndexedIO::OpenMode mode )
	: m_mainScene( nullptr ),
	m_linkedScene( nullptr ),
	m_rootLinkDepth( 0 ),
	m_readOnly( mode & IndexedIO::Read ),
	m_atLink( false ),
	m_sampled( true ),
	m_timeRemapped( false ),
	m_linkLocationsData( new IECore::PathMatcherData() )
{
	if( mode & IndexedIO::Append )
	{
		throw InvalidArgumentException( "Append mode not supported" );
	}

	m_mainScene = new SceneCache( fileName, mode );

	if( m_readOnly && m_mainScene->hasAttribute( "linkLocations" ) )
	{
		IECore::ConstObjectPtr linkLocationObj = m_mainScene->readAttribute( "linkLocations", 0 );
		m_linkLocationsData = runTimeCast<const IECore::PathMatcherData>( linkLocationObj.get() )->copy();
	}

}

LinkedScene::LinkedScene( ConstSceneInterfacePtr mainScene )
	: m_mainScene( const_cast<SceneInterface *>(mainScene.get()) ),
	m_linkedScene( nullptr ),
	m_rootLinkDepth( 0 ),
	m_readOnly( true ),
	m_atLink( false ),
	m_timeRemapped( false ),
	m_linkLocationsData( new IECore::PathMatcherData() )
{
	if( SceneCachePtr scc = runTimeCast<SceneCache>( m_mainScene ) )
	{
		m_readOnly = scc->readOnly();
	}

	m_sampled = (runTimeCast<const SampledSceneInterface>(mainScene.get()) != nullptr);
}

LinkedScene::LinkedScene(
	SceneInterface *mainScene,
	const SceneInterface *linkedScene,
	IECore::PathMatcherDataPtr linkLocationsData,
	int rootLinkDepth,
	bool readOnly,
	bool atLink,
	bool timeRemapped
)
	: m_mainScene( mainScene ),
	m_linkedScene( linkedScene ),
	m_rootLinkDepth( rootLinkDepth ),
	m_readOnly( readOnly ),
	m_atLink( atLink ),
	m_timeRemapped( timeRemapped ),
	m_linkLocationsData( linkLocationsData )
{
	if ( !mainScene )
	{
		throw Exception( "NULL main scene!" );
	}

	if ( linkedScene )
	{
		m_sampled = (runTimeCast<const SampledSceneInterface>(linkedScene) != nullptr);
	}
	else
	{
		m_sampled = (runTimeCast<const SampledSceneInterface>(mainScene) != nullptr);
	}

}

LinkedScene::~LinkedScene()
{
	if( !m_readOnly  )
	{
		SceneInterface::Path p;
		LinkedScene::path( p );

		if ( p.empty() )
		{
			m_mainScene->writeAttribute( "linkLocations", m_linkLocationsData.get(), 0 );
		}
	}
}

void LinkedScene::writeLink( const SceneInterface *scene )
{
	CompoundDataPtr d = linkAttributeData( scene );
	writeAttribute( linkAttribute, d.get(), 0 );
}

IECore::CompoundDataPtr LinkedScene::linkAttributeData( const SceneInterface *scene )
{
	std::string f = scene->fileName();
	InternedStringVectorDataPtr r = new InternedStringVectorData();
	scene->path( r->writable() );
	CompoundDataPtr d = new CompoundData();
	d->writable()[g_fileName] = new StringData(f);
	d->writable()[g_root] = r;
	return d;
}

IECore::CompoundDataPtr LinkedScene::linkAttributeData( const SceneInterface *scene, double time )
{
	IECore::CompoundDataPtr d = linkAttributeData(scene);
	d->writable()[g_time] = new DoubleData(time);
	return d;
}

std::string LinkedScene::fileName() const
{
	return m_mainScene->fileName();
}

void LinkedScene::path( Path &p ) const
{
	p.clear();
	m_mainScene->path(p);

	if ( m_linkedScene )
	{
		Path tmp;
		m_linkedScene->path(tmp);
		p.insert( p.end(), tmp.begin() + m_rootLinkDepth, tmp.end() );
	}
}

SceneInterface::Name LinkedScene::name() const
{
	if ( m_linkedScene )
	{
		if ( m_atLink )
		{
			/// special case: we are exactly at the entry point for the linked scene,
			/// in that case, we want the name of the branch to come from the main scene.
			return m_mainScene->name();
		}
		return m_linkedScene->name();
	}
	else
	{
		return m_mainScene->name();
	}
}

size_t LinkedScene::numBoundSamples() const
{
	if (!m_sampled)
	{
		return 0;
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( timeLinkAttribute );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->numBoundSamples();
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numBoundSamples();
	}
}

double LinkedScene::boundSampleTime( size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "boundSampleTime not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( timeLinkAttribute, sampleIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->boundSampleTime(sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->boundSampleTime(sampleIndex);
	}
}

double LinkedScene::boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "boundSampleInterval not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( timeLinkAttribute, time, floorIndex, ceilIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->boundSampleInterval(time,floorIndex,ceilIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->boundSampleInterval(time,floorIndex,ceilIndex);
	}
}

Imath::Box3d LinkedScene::readBoundAtSample( size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "readBoundAtSample not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return m_linkedScene->readBound( remappedLinkTimeAtSample(sampleIndex) );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->readBoundAtSample(sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->readBoundAtSample(sampleIndex);
	}
}

Imath::Box3d LinkedScene::readBound( double time ) const
{
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			time = remappedLinkTime( time );
		}
		return m_linkedScene->readBound(time);
	}
	else
	{
		return m_mainScene->readBound(time);
	}
}

void LinkedScene::writeBound( const Imath::Box3d &bound, double time )
{
	if ( m_readOnly )
	{
		throw Exception( "No write access to scene file!" );
	}

	m_mainScene->writeBound(bound,time);
}

size_t LinkedScene::numTransformSamples() const
{
	if (!m_sampled)
	{
		return 0;
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( timeLinkAttribute );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->numTransformSamples();
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numTransformSamples();
	}
}

double LinkedScene::transformSampleTime( size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "transformSampleTime not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( timeLinkAttribute, sampleIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->transformSampleTime(sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->transformSampleTime(sampleIndex);
	}
}

double LinkedScene::transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "transformSampleInterval not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( timeLinkAttribute, time, floorIndex, ceilIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->transformSampleInterval(time,floorIndex,ceilIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->transformSampleInterval(time,floorIndex,ceilIndex);
	}
}

ConstDataPtr LinkedScene::readTransformAtSample( size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "readTransformAtSample not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return m_linkedScene->readTransform( remappedLinkTimeAtSample(sampleIndex) );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->readTransformAtSample(sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->readTransformAtSample(sampleIndex);
	}
}

Imath::M44d LinkedScene::readTransformAsMatrixAtSample( size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "readTransformAsMatrixAtSample not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return m_linkedScene->readTransformAsMatrix( remappedLinkTimeAtSample(sampleIndex) );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->readTransformAsMatrixAtSample(sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->readTransformAsMatrixAtSample(sampleIndex);
	}
}

ConstDataPtr LinkedScene::readTransform( double time ) const
{
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			time = remappedLinkTime( time );
		}
		return m_linkedScene->readTransform(time);
	}
	else
	{
		return m_mainScene->readTransform(time);
	}
}

Imath::M44d LinkedScene::readTransformAsMatrix( double time ) const
{
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			time = remappedLinkTime( time );
		}
		return m_linkedScene->readTransformAsMatrix(time);
	}
	else
	{
		return m_mainScene->readTransformAsMatrix(time);
	}
}

void LinkedScene::writeTransform( const Data *transform, double time )
{
	if ( m_readOnly )
	{
		throw Exception( "No write access to scene file!" );
	}

	m_mainScene->writeTransform(transform,time);
}

bool LinkedScene::hasAttribute( const Name &name ) const
{
	// \todo: remove this when it's no longer relevant
	if ( name == linkAttribute )
	{
		return false;
	}

	if ( m_linkedScene && !m_atLink )
	{
		return m_linkedScene->hasAttribute(name);
	}
	else
	{
		if( name == timeLinkAttribute && m_linkedScene )
		{
			return true;
		}
		return m_mainScene->hasAttribute(name);
	}
}

void LinkedScene::attributeNames( NameList &attrs ) const
{
	if ( m_linkedScene && !m_atLink )
	{
		m_linkedScene->attributeNames(attrs);
	}
	else
	{
		m_mainScene->attributeNames(attrs);

		for ( NameList::iterator it = attrs.begin(); it != attrs.end(); it++ )
		{
			// \todo: remove "*it == linkAttribute" when it's no longer relevant
			if ( *it == linkAttribute || *it == fileNameLinkAttribute || *it == rootLinkAttribute || *it == timeLinkAttribute )
			{
				attrs.erase( it );
				--it;
			}
		}
	}
}

size_t LinkedScene::numAttributeSamples( const Name &name ) const
{
	if (!m_sampled)
	{
		return 0;
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( timeLinkAttribute );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->numAttributeSamples(name);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples(name);
	}
}

double LinkedScene::attributeSampleTime( const Name &name, size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "attributeSampleTime not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( timeLinkAttribute, sampleIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->attributeSampleTime(name, sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime(name, sampleIndex);
	}
}

double LinkedScene::attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "attributeSampleInterval not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( timeLinkAttribute, time, floorIndex, ceilIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->attributeSampleInterval(name,time,floorIndex,ceilIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval(name,time,floorIndex,ceilIndex);
	}
}

ConstObjectPtr LinkedScene::readAttributeAtSample( const Name &name, size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "readAttributeAtSample not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			return m_linkedScene->readAttribute( name, remappedLinkTimeAtSample(sampleIndex) );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->readAttributeAtSample(name,sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->readAttributeAtSample(name,sampleIndex);
	}
}

ConstObjectPtr LinkedScene::readAttribute( const Name &name, double time ) const
{
	if ( m_linkedScene && !m_atLink )
	{
		if ( m_timeRemapped )
		{
			time = remappedLinkTime( time );
		}
		return m_linkedScene->readAttribute(name,time);
	}
	else
	{
		if( name == timeLinkAttribute && !m_mainScene->hasAttribute( timeLinkAttribute ) )
		{
			return new DoubleData( time );
		}
		return m_mainScene->readAttribute(name,time);
	}
}

void LinkedScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	if ( m_readOnly )
	{
		throw Exception( "No write access to scene file!" );
	}

	if ( name == linkAttribute )
	{
		bool firstTime = !m_mainScene->hasAttribute( fileNameLinkAttribute );

		if ( firstTime )
		{
			// if it's the first time, we better check if this level already has objects, tags or children
			// and raise exceptions to prevent weird configurations...
			if ( m_mainScene->hasObject() )
			{
				throw Exception( "Links to external scenes cannot be created on locations where there's already an object saved!" );
			}
		}

		// we are creating a link!
		const CompoundData *d = runTimeCast< const CompoundData >(attribute);
		if ( !d )
		{
			throw Exception( "SceneInterface:link attribute must be of type CompoundData!" );
		}

		// open the linked scene
		int linkDepth;
		ConstDoubleDataPtr timeData = d->member< const DoubleData >( g_time );
		const StringData *fileName = d->member< const StringData >( g_fileName );
		const InternedStringVectorData *sceneRoot = d->member< const InternedStringVectorData >( g_root );
		m_linkedScene = expandLink( fileName, sceneRoot, linkDepth );
		if ( !m_linkedScene )
		{
			throw Exception( "Trying to store a broken link!" );
		}

		// check for child name clashes:
		NameList mainSceneChildren;
		NameList linkedSceneChildren;

		m_mainScene->childNames( mainSceneChildren );
		m_linkedScene->childNames( linkedSceneChildren );
		std::sort( mainSceneChildren.begin(), mainSceneChildren.end() );
		std::sort( linkedSceneChildren.begin(), linkedSceneChildren.end() );

		std::set<Name> intersection;
		std::set_intersection(mainSceneChildren.begin(),mainSceneChildren.end(),linkedSceneChildren.begin(),linkedSceneChildren.end(), std::inserter(intersection,intersection.begin()) );

		if( intersection.size() )
		{
			std::string intersectionString;
			for( std::set<Name>::iterator it = intersection.begin(); it != intersection.end(); ++it )
			{
				intersectionString += " " + it->string();
			}
			throw Exception( "Attempting to write link will cause the following child name clashes: " + intersectionString );
		}

		// get the bounds of the linked scene
		const SampledSceneInterface *sampledScene = runTimeCast< const SampledSceneInterface >(m_linkedScene.get());
		if ( sampledScene && !timeData )
		{
			// When there's no time remapping we get all the bounding box samples from the linked scene, using the same time.
			if ( firstTime )
			{
				size_t bounds = sampledScene->numBoundSamples();
				for ( size_t b = 0; b < bounds; b++ )
				{
					m_mainScene->writeBound( sampledScene->readBoundAtSample(b), sampledScene->boundSampleTime(b) );
				}
			}
		}
		else
		{
			/// we store just the current bounding box
			if ( timeData )
			{
				m_mainScene->writeBound( m_linkedScene->readBound(timeData->readable()), time );
			}
			else
			{
				m_mainScene->writeBound( m_linkedScene->readBound(time), time );
			}
		}

		if ( firstTime )
		{
			// save the tags from the linked file to the current location so it gets propagated to the root.
			NameList tags;

			// Check if the position of the file we are trying to link to, has ancestor tags.
			// This situation is undesirable, as it will make LinkedScene return inconsistent ancestor tags before and after the link location.
			m_linkedScene->readTags(tags, SceneInterface::AncestorTag );
			if ( tags.size() )
			{
				std::string pathStr;
				SceneInterface::pathToString( sceneRoot->readable(), pathStr );
				msg( Msg::Debug, "LinkedScene::writeAttribute", ( boost::format( "Detected ancestor tags while creating link to file %s at location %s." ) % fileName->readable() % pathStr ).str() );
			}
			tags.clear();

			/// copy all descendent and local tags as descendent tags (so we can distinguish from tags added in the LinkedScene)
			m_linkedScene->readTags(tags, SceneInterface::LocalTag|SceneInterface::DescendantTag );
			static_cast< SceneCache *>(m_mainScene.get())->writeTags(tags, true);

			m_mainScene->writeAttribute( fileNameLinkAttribute, d->member< const StringData >( g_fileName ), time );
			m_mainScene->writeAttribute( rootLinkAttribute, d->member< const InternedStringVectorData >( g_root ), time );

			SceneInterface::Path currentPath;
			path( currentPath );

			m_linkLocationsData->writable().addPath( currentPath );
		}

		/// we keep the information this level has a link, so we can prevent attempts to
		/// create children or save objects at this level.
		m_atLink = true;

		if( timeData )
		{
			m_mainScene->writeAttribute( timeLinkAttribute, timeData.get(), time );
		}
		return;
	}

	m_mainScene->writeAttribute(name,attribute,time);
}

bool LinkedScene::hasTag( const Name &name, int filter ) const
{
	if ( m_linkedScene )
	{
		if ( m_linkedScene->hasTag( name, filter ) )
		{
			return true;
		}

		int mainFilter = filter & ( SceneInterface::AncestorTag | ( m_atLink ? SceneInterface::LocalTag : 0 ) );
		if ( !m_atLink && (filter & SceneInterface::AncestorTag) )
		{
			/// child locations inside the link consider all the local tags at the link location as ancestor tags as well.
			mainFilter |= SceneInterface::LocalTag;
		}
		return m_mainScene->hasTag(name, mainFilter);
	}
	else
	{
		return m_mainScene->hasTag( name, filter );
	}
}

void LinkedScene::readTags( NameList &tags, int filter ) const
{
	if ( filter!=SceneInterface::LocalTag && !m_readOnly )
	{
		throw Exception( "readTags with filter != LocalTag is only supported when reading the scene file!" );
	}

	if ( m_linkedScene )
	{
		m_linkedScene->readTags( tags, filter );

		/// Only queries ancestor tags and local tags (if at the link location) from the main scene.
		int mainFilter = filter & ( SceneInterface::AncestorTag | ( m_atLink ? SceneInterface::LocalTag : 0 ) );
		if ( !m_atLink && (filter & SceneInterface::AncestorTag) )
		{
			/// child locations inside the link consider all the local tags at the link location as ancestor tags as well.
			mainFilter |= SceneInterface::LocalTag;
		}
		if ( mainFilter )
		{
			NameList mainTags;
			m_mainScene->readTags( mainTags, mainFilter );
			tags.insert( tags.end(), mainTags.begin(), mainTags.end() );
		}

		// remove duplicates:
		std::sort( tags.begin(), tags.end() );
		tags.erase( std::unique( tags.begin(), tags.end() ), tags.end() );
	}
	else
	{
		m_mainScene->readTags( tags, filter );
	}
}

void LinkedScene::writeTags( const NameList &tags )
{
	if ( m_readOnly )
	{
		throw Exception( "No write access to scene file!" );
	}

	m_mainScene->writeTags(tags);
}

bool LinkedScene::hasObject() const
{
	if ( m_linkedScene )
	{
		return m_linkedScene->hasObject();
	}
	else
	{
		return m_mainScene->hasObject();
	}
}

SceneInterface::NameList LinkedScene::setNames( bool includeDescendantSets ) const
{
	if( m_linkedScene )
	{
		return m_linkedScene->setNames( includeDescendantSets );
	}
	else
	{
		if ( !includeDescendantSets )
		{
			return NameList();
		}

		SceneInterface::NameList setNames = m_mainScene->setNames( includeDescendantSets );

		const PathMatcher links = runTimeCast<const LinkedScene>( scene( SceneInterface::rootPath ) )->linkLocations();
		for( PathMatcher::Iterator it = links.begin(); it != links.end(); ++it )
		{
			SceneInterface::NameList linkedSceneSetNames = scene( *it, SceneInterface::ThrowIfMissing )->setNames();
			setNames.insert( setNames.begin(), linkedSceneSetNames.begin(), linkedSceneSetNames.end() );
		}

		std::sort( setNames.begin(), setNames.end() );
		return NameList( setNames.begin(), std::unique( setNames.begin(), setNames.end() ) );
	}
}

IECore::PathMatcher LinkedScene::readSet( const SceneInterface::Name &name, bool includeDescendantSets ) const
{
	if( m_linkedScene )
	{
		return m_linkedScene->readSet( name, includeDescendantSets );
	}

	if ( includeDescendantSets )
	{
		// todo only copy if we need to
		IECore::PathMatcher result = m_mainScene->readSet( name );

		const PathMatcher links = runTimeCast<const LinkedScene>( scene( SceneInterface::rootPath ) )->linkLocations();
		for( PathMatcher::Iterator it = links.begin(); it != links.end(); ++it )
		{
			SceneInterface::Path p;
			path( p );
			std::string strPath;
			SceneInterface::pathToString( *it, strPath );
			IECore::PathMatcher linkedSceneSet = scene( *it, SceneInterface::ThrowIfMissing )->readSet( name );

			result.addPaths( linkedSceneSet, *it );
		}
		return result;
	}

	return IECore::PathMatcher();
}

void LinkedScene::writeSet( const SceneInterface::Name &name, const IECore::PathMatcher &set )
{
	if( m_linkedScene ) // todo check this condition.
	{
		SceneInterface::Path p;
		path( p );
		std::string strPath;
		SceneInterface::pathToString( p, strPath );
		throw IECore::Exception( boost::str( boost::format( "Unable to write set to linked scene location: '%1%'" ) % strPath ) );
	}
	else
	{
		m_mainScene->writeSet( name, set );
	}
}

void LinkedScene::hashSet( const SceneInterface::Name &setName, IECore::MurmurHash &h ) const
{
	SampledSceneInterface::hashSet( setName, h );

	if( m_linkedScene )
	{
		m_linkedScene->hashSet( setName, h );
	}
	else
	{
		m_mainScene->hashSet( setName, h );
	}
}

size_t LinkedScene::numObjectSamples() const
{
	if (!m_sampled)
	{
		return 0;
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( timeLinkAttribute );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->numObjectSamples();
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numObjectSamples();
	}
}

double LinkedScene::objectSampleTime( size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "objectSampleTime not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( timeLinkAttribute, sampleIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->objectSampleTime(sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->objectSampleTime(sampleIndex);
	}
}

double LinkedScene::objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "objectSampleInterval not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( timeLinkAttribute, time, floorIndex, ceilIndex );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->objectSampleInterval(time,floorIndex,ceilIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->objectSampleInterval(time,floorIndex,ceilIndex);
	}
}

ConstObjectPtr LinkedScene::readObjectAtSample( size_t sampleIndex ) const
{
	if (!m_sampled)
	{
		throw Exception( "readObjectAtSample not supported: LinkedScene is pointing to a non-sampled scene!" );
	}
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			return m_linkedScene->readObject( remappedLinkTimeAtSample(sampleIndex) );
		}
		else
		{
			return static_cast<const SampledSceneInterface*>(m_linkedScene.get())->readObjectAtSample(sampleIndex);
		}
	}
	else
	{
		return static_cast<const SampledSceneInterface*>(m_mainScene.get())->readObjectAtSample(sampleIndex);
	}
}

ConstObjectPtr LinkedScene::readObject( double time ) const
{
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			time = remappedLinkTime( time );
		}
		return m_linkedScene->readObject(time);
	}
	else
	{
		return m_mainScene->readObject(time);
	}
}

PrimitiveVariableMap LinkedScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	if ( m_linkedScene )
	{
		if ( m_timeRemapped )
		{
			time = remappedLinkTime( time );
		}
		return m_linkedScene->readObjectPrimitiveVariables( primVarNames, time );
	}
	else
	{
		return m_mainScene->readObjectPrimitiveVariables( primVarNames, time );
	}
}

void LinkedScene::writeObject( const Object *object, double time )
{
	if ( m_readOnly )
	{
		throw Exception( "No write access to scene file!" );
	}
	if ( m_atLink )
	{
		throw Exception( "Locations with links to external scene cannot have objects themselves!" );
	}

	m_mainScene->writeObject(object,time);
}

void LinkedScene::childNames( NameList &childNames ) const
{
	if( m_atLink )
	{
		// concatenate link child names with main scene child names:
		m_linkedScene->childNames(childNames);
		NameList mainSceneChildNames;
		m_mainScene->childNames(mainSceneChildNames);
		childNames.insert( childNames.end(), mainSceneChildNames.begin(), mainSceneChildNames.end() );
		return;
	}

	if ( m_linkedScene )
	{
		return m_linkedScene->childNames(childNames);
	}
	else
	{
		return m_mainScene->childNames(childNames);
	}
}

ConstSceneInterfacePtr LinkedScene::expandLink( const StringData *fileName, const InternedStringVectorData *root, int &linkDepth )
{
	if ( fileName && root )
	{
		ConstSceneInterfacePtr l = nullptr;
		try
		{
			l = SharedSceneInterfaces::get( fileName->readable() );
		}
		catch ( IECore::Exception &e )
		{
			IECore::msg( IECore::MessageHandler::Error, "LinkedScene::expandLink", std::string( e.what() ) + " when expanding link from file \"" + m_mainScene->fileName() + "\"" );
			linkDepth = 0;
			return nullptr;
		}

		linkDepth = root->readable().size();
		l = l->scene(root->readable(), NullIfMissing);
		if ( !l )
		{
			// \todo Consider throwing or printing error message.
			linkDepth = 0;
		}
		return l;
	}
	linkDepth = 0;
	return nullptr;
}

double LinkedScene::remappedLinkTime( double time ) const
{
	if( m_mainScene->hasAttribute( timeLinkAttribute ) )
	{
		ConstDoubleDataPtr t = runTimeCast< const DoubleData >( m_mainScene->readAttribute( timeLinkAttribute, time ) );
		if ( !t )
		{
			throw Exception( "Invalid time when querying for time remapping!" );
		}
		return t->readable();
	}
	else
	{
		ConstCompoundDataPtr d = runTimeCast< const CompoundData >( m_mainScene->readAttribute( linkAttribute, time ) );
		if( !d )
		{
			throw Exception( "Invalid time when querying for time remapping!" );
		}
		ConstDoubleDataPtr t = d->member<DoubleData>( g_time );
		if( !t )
		{
			throw Exception( "Invalid time when querying for time remapping!" );
		}
		return t->readable();
	}
}

double LinkedScene::remappedLinkTimeAtSample( size_t sampleIndex ) const
{
	if( m_mainScene->hasAttribute( timeLinkAttribute ) )
	{
		ConstDoubleDataPtr t = runTimeCast< const DoubleData >( static_cast<const SampledSceneInterface*>(m_mainScene.get())->readAttributeAtSample( timeLinkAttribute, sampleIndex ) );
		if ( !t )
		{
			throw Exception( "Invalid time when querying for time remapping!" );
		}
		return t->readable();
	}
	else
	{
		ConstCompoundDataPtr d = runTimeCast< const CompoundData >( static_cast<const SampledSceneInterface*>(m_mainScene.get())->readAttributeAtSample( linkAttribute, sampleIndex ) );
		if( !d )
		{
			throw Exception( "Invalid time when querying for time remapping!" );
		}
		ConstDoubleDataPtr t = d->member<DoubleData>( g_time );
		if( !t )
		{
			throw Exception( "Invalid time when querying for time remapping!" );
		}
		return t->readable();
	}
}

SceneInterfacePtr LinkedScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	if ( missingBehaviour == SceneInterface::CreateIfMissing )
	{
		if ( m_readOnly )
		{
			throw Exception( "No write access to scene file!" );
		}
	}

	if ( m_linkedScene )
	{
		ConstSceneInterfacePtr c = m_linkedScene->child( name, SceneInterface::NullIfMissing );
		if ( c )
		{
			return new LinkedScene( m_mainScene.get(), c.get(), m_linkLocationsData, m_rootLinkDepth, m_readOnly, false, m_timeRemapped );
		}
		if( !m_atLink )
		{
			if( missingBehaviour == SceneInterface::ThrowIfMissing )
			{
				throw Exception( "IECore::LinkedScene::child(): Couldn't find child named " + name.string() );
			}
			else if( missingBehaviour == SceneInterface::NullIfMissing )
			{
				return nullptr;
			}
		}
	}

	if( missingBehaviour == SceneInterface::CreateIfMissing && m_atLink )
	{
		if( !m_mainScene->hasChild( name ) )
		{
			// check for name clashes with the link:
			NameList linkChildNames;
			m_linkedScene->childNames( linkChildNames );

			if( std::find( linkChildNames.begin(), linkChildNames.end(), name ) != linkChildNames.end() )
			{
				throw Exception( "IECore::LinkedScene::child(): Child name " + name.string() + " clashes with a child in the link" );
			}
		}
	}


	SceneInterfacePtr c = m_mainScene->child( name, missingBehaviour );
	if ( !c )
	{
		return nullptr;
	}
	if ( m_readOnly )
	{
		if( c->hasAttribute( fileNameLinkAttribute ) && c->hasAttribute( rootLinkAttribute ) )
		{
			ConstStringDataPtr fileName = runTimeCast< const StringData >( c->readAttribute( fileNameLinkAttribute, 0 ) );
			ConstInternedStringVectorDataPtr root = runTimeCast< const InternedStringVectorData >( c->readAttribute( rootLinkAttribute, 0 ) );

			/// we found the link attribute...
			int linkDepth;
			bool timeRemapped = c->hasAttribute( timeLinkAttribute );
			ConstSceneInterfacePtr l = expandLink( fileName.get(), root.get(), linkDepth );
			if ( l )
			{
				return new LinkedScene( c.get(), l.get(), m_linkLocationsData, linkDepth, m_readOnly, true, timeRemapped );
			}
		}
		else if( c->hasAttribute( linkAttribute ) )
		{
			// read from old school link attribute.
			// \todo: remove this when it doesn't break everyone's stuff!
			ConstCompoundDataPtr d = runTimeCast< const CompoundData >( c->readAttribute( linkAttribute, 0 ) );
			/// we found the link attribute...
			int linkDepth;
			bool timeRemapped = ( d->member<DoubleData>( g_time ) != nullptr );
			ConstSceneInterfacePtr l = expandLink( d->member< const StringData >( g_fileName ), d->member< const InternedStringVectorData >( g_root ), linkDepth );
			if ( l )
			{
				return new LinkedScene( c.get(), l.get(), m_linkLocationsData, linkDepth, m_readOnly, true, timeRemapped );
			}
		}
	}

	return new LinkedScene( c.get(), nullptr, m_linkLocationsData, 0, m_readOnly, false, false );

}

ConstSceneInterfacePtr LinkedScene::child( const Name &name, MissingBehaviour missingBehaviour ) const
{
	if ( !m_readOnly )
	{
		throw Exception( "Const-child method called on write-only LinkedScene!" );
	}
	if ( missingBehaviour == SceneInterface::CreateIfMissing )
	{
		throw Exception( "No write access to scene file!" );
	}
	return const_cast< LinkedScene * >(this)->child( name, missingBehaviour );
}

bool LinkedScene::hasChild( const Name &name ) const
{
	if ( m_linkedScene )
	{
		return m_linkedScene->hasChild(name);
	}
	else
	{
		return m_mainScene->hasChild(name);
	}
}

SceneInterfacePtr LinkedScene::createChild( const Name &name )
{
	if ( hasChild(name) )
	{
		throw Exception( "Child already exist!" );
	}
	return child( name, SceneInterface::CreateIfMissing );
}

SceneInterfacePtr LinkedScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	if ( missingBehaviour == SceneInterface::CreateIfMissing )
	{
		throw Exception( "createIfMissing is not supported!" );
	}

	SceneInterfacePtr s = m_mainScene->scene( SceneInterface::rootPath );

	Path::const_iterator pIt;
	/// first try to get as close as possible using the m_mainScene...
	for ( pIt = path.begin(); pIt != path.end(); pIt++ )
	{
		SceneInterfacePtr n = s->child( *pIt, SceneInterface::NullIfMissing );
		if ( !n )
		{
			break;
		}
		s = n;
	}
	ConstSceneInterfacePtr l = nullptr;
	int linkDepth = 0;
	bool atLink = false;
	bool timeRemapped = false;

	if ( s->hasAttribute( fileNameLinkAttribute ) && s->hasAttribute( rootLinkAttribute ) )
	{
		atLink = true;
		timeRemapped = s->hasAttribute( timeLinkAttribute );
		ConstStringDataPtr fileName = runTimeCast< const StringData >( s->readAttribute( fileNameLinkAttribute, 0 ) );
		ConstInternedStringVectorDataPtr root = runTimeCast< const InternedStringVectorData >( s->readAttribute( rootLinkAttribute, 0 ) );

		l = expandLink( fileName.get(), root.get(), linkDepth );
		if (!l)
		{
			atLink = false;
			timeRemapped = false;
		}
	}
	else if( s->hasAttribute( linkAttribute ) )
	{
		atLink = true;
		ConstCompoundDataPtr d = runTimeCast< const CompoundData >( s->readAttribute( linkAttribute, 0 ) );
		l = expandLink( d->member< const StringData >( g_fileName ), d->member< const InternedStringVectorData >( g_root ), linkDepth );
		if ( !l )
		{
			atLink = false;
		}
	}

	if ( pIt != path.end() )
	{
		if ( !atLink )
		{
			if ( missingBehaviour == SceneInterface::NullIfMissing )
			{
				return nullptr;
			}
			throw Exception( "Could not find child '" + pIt->value() + "'"  );
		}

		for ( ; pIt != path.end(); pIt++ )
		{
			l = l->child( *pIt, missingBehaviour );
			if ( !l )
			{
				return nullptr;
			}
		}
		atLink = false;
	}
	return new LinkedScene( s.get(), l.get(), m_linkLocationsData, linkDepth, m_readOnly, atLink, timeRemapped );
}

ConstSceneInterfacePtr LinkedScene::scene( const Path &path, LinkedScene::MissingBehaviour missingBehaviour ) const
{
	if ( !m_readOnly )
	{
		throw Exception( "Const-scene method called on write-only LinkedScene!" );
	}
	if ( missingBehaviour == SceneInterface::CreateIfMissing )
	{
		throw Exception( "No write access to scene file!" );
	}

	SceneInterfacePtr (LinkedScene::*nonConstSceneFn)(const Path &, MissingBehaviour) = &LinkedScene::scene;
	return (const_cast<LinkedScene*>(this)->*nonConstSceneFn)( path, missingBehaviour );
}

void LinkedScene::mainSceneHash( HashType hashType, double time, MurmurHash &h ) const
{
	// We add the base class hash so that it does not collide with hashes returned as if the main scene was opened directly as a SceneCache.
	SceneInterface::hash( hashType, time, h );
	m_mainScene->hash(hashType, time, h);
}

void LinkedScene::hash( HashType hashType, double time, MurmurHash &h ) const
{
	if ( !m_readOnly )
	{
		throw Exception( "Hashes not available on write-only LinkedScene!" );
	}

	if ( m_linkedScene )
	{
		if ( m_atLink )
		{
			/// special cases: we are exactly at the entry point for the linked scene.
			switch( hashType )
			{
				case TransformHash:
					mainSceneHash( hashType, time, h );
					// Link locations override the transform and bound, so we return without adding the hash from the linked scene.
					return;
				case AttributesHash:
				case HierarchyHash:
				case ChildNamesHash:
					// Attributes, ChildNames and Hierarchy are affected by both the main scene and the linked scene
					mainSceneHash( hashType, time, h );
					break;
				case ObjectHash:
					// Let the object come from the linked location:
					break;
				case BoundHash:
					// if there are extra children at the link, we have to read the bound directly from the main scene,
					// so we hash from there and return (which isn't particularly smart at the moment, so it's going to look
					// like all bounds are different when maybe they aren't). Otherwise we get the bound hash from m_linkedScene
					// so we can recognize repetition.
					NameList childNames;
					m_mainScene->childNames( childNames );
					if( childNames.size() )
					{
						mainSceneHash( hashType, time, h );
						return;
					}
					break;
			};
		}
		if ( m_timeRemapped )
		{
			time = remappedLinkTime( time );
		}
		m_linkedScene->hash(hashType, time, h);
	}
	else
	{
		mainSceneHash(hashType, time, h);
	}
}

/// serialise this into the linked scene cache so it can just be loaded directly without having to traverse the entire scene
IECore::PathMatcher LinkedScene::linkLocations() const
{
	return m_linkLocationsData->readable();
}

void LinkedScene::recurseLinkLocations( PathMatcher &pathMatcher ) const
{
	SceneInterface::NameList children;
	childNames( children );

	bool isLinkLocation = hasAttribute( fileNameLinkAttribute ) && hasAttribute( rootLinkAttribute );

	if( isLinkLocation )
	{
		SceneInterface::Path p;
		path( p );
		pathMatcher.addPath( p );
	}

	for( const SceneInterface::Name &c : children )
	{
		ConstSceneInterfacePtr childScene = child( c, SceneInterface::ThrowIfMissing );
		runTimeCast<const LinkedScene>( childScene.get() )->recurseLinkLocations( pathMatcher );
	}
}
