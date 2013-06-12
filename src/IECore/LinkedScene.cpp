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

#include "IECore/LinkedScene.h"
#include "IECore/SceneCache.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/SharedSceneInterfaces.h"

using namespace IECore;

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
const InternedString LinkedScene::g_fileNameLinkAttribute("fileName");
const InternedString LinkedScene::g_rootLinkAttribute("root");
const InternedString LinkedScene::g_timeAttribute("time");

LinkedScene::LinkedScene( const std::string &fileName, IndexedIO::OpenMode mode ) : m_mainScene(0), m_linkedScene(0), m_rootLinkDepth(0), m_readOnly(mode & IndexedIO::Read), m_atLink(false), m_sampled(true), m_timeRemapped(false)
{
	if( mode & IndexedIO::Append )
	{
		throw InvalidArgumentException( "Append mode not supported" );
	}

	m_mainScene = new SceneCache( fileName, mode );
}

LinkedScene::LinkedScene( ConstSceneInterfacePtr mainScene ) : m_mainScene(const_cast<SceneInterface*>(mainScene.get())), m_linkedScene(0), m_rootLinkDepth(0), m_readOnly(true), m_atLink(false), m_timeRemapped(false)
{
	m_sampled = (runTimeCast<const SampledSceneInterface>(mainScene.get()) != NULL);
}

LinkedScene::LinkedScene( SceneInterface *mainScene, const SceneInterface *linkedScene, int rootLinkDepth, bool readOnly, bool atLink, bool timeRemapped ) : m_mainScene(mainScene), m_linkedScene(linkedScene), m_rootLinkDepth(rootLinkDepth), m_readOnly(readOnly), m_atLink(atLink), m_timeRemapped(timeRemapped)
{
	if ( !mainScene )
	{
		throw Exception( "NULL main scene!" );
	}

	if ( linkedScene )
	{
		m_sampled = (runTimeCast<const SampledSceneInterface>(linkedScene) != NULL);
	}
	else
	{
		m_sampled = (runTimeCast<const SampledSceneInterface>(mainScene) != NULL);
	}
	
}

LinkedScene::~LinkedScene()
{
}

void LinkedScene::writeLink( const SceneInterface *scene )
{
	writeAttribute( linkAttribute, linkAttributeData(scene), 0 );
}

IECore::CompoundDataPtr LinkedScene::linkAttributeData( const SceneInterface *scene )
{
	std::string f = scene->fileName();
	InternedStringVectorDataPtr r = new InternedStringVectorData();
	scene->path( r->writable() );
	CompoundDataPtr d = new CompoundData();
	d->writable()[g_fileNameLinkAttribute] = new StringData(f);
	d->writable()[g_rootLinkAttribute] = r;
	return d;
}

IECore::CompoundDataPtr LinkedScene::linkAttributeData( const SceneInterface *scene, double time )
{
	IECore::CompoundDataPtr d = linkAttributeData(scene);
	d->writable()[g_timeAttribute] = new DoubleData(time);
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( linkAttribute );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( linkAttribute, sampleIndex );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( linkAttribute, time, floorIndex, ceilIndex );
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
	if ( m_linkedScene )
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( linkAttribute );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( linkAttribute, sampleIndex );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( linkAttribute, time, floorIndex, ceilIndex );
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

DataPtr LinkedScene::readTransformAtSample( size_t sampleIndex ) const
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

DataPtr LinkedScene::readTransform( double time ) const
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
			if ( *it == linkAttribute )
			{
				attrs.erase( it );
				break;
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( linkAttribute );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( linkAttribute, sampleIndex );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( linkAttribute, time, floorIndex, ceilIndex );
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

ObjectPtr LinkedScene::readAttributeAtSample( const Name &name, size_t sampleIndex ) const
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

ObjectPtr LinkedScene::readAttribute( const Name &name, double time ) const
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
		bool firstTime = !m_mainScene->hasAttribute( linkAttribute );

		if ( firstTime )
		{
			// if it's the first time, we better check if this level already has objects, tags or children
			// and raise exceptions to prevent weird configurations...
			if ( m_mainScene->hasObject() )
			{
				throw Exception( "Links to external scenes cannot be created on locations where there's already an object saved!" );
			}

			NameList names;
			m_mainScene->childNames( names );
			if ( names.size() )
			{
				throw Exception( "Links to external scenes cannot be created on locations where there are already child locations!" );
			}

			m_mainScene->readTags( names, false );
			if ( names.size() )
			{
				throw Exception( "Links to external scenes cannot be created on locations where there are already tags stored!" );
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
		bool timeRemapped;
		ConstSceneInterfacePtr linkedScene = expandLink( d, linkDepth, timeRemapped );
		if ( !linkedScene )
		{
			throw Exception( "Trying to store a broken link!" );
		}

		// get the bounds of the linked scene
		const SampledSceneInterface *sampledScene = runTimeCast< const SampledSceneInterface >(linkedScene.get());
		if ( sampledScene && !timeRemapped )
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
			// Query the bound at the remapped time
			ConstDoubleDataPtr t = d->member< DoubleData >(g_timeAttribute);
			/// we store just the current bounding box
			m_mainScene->writeBound( linkedScene->readBound(t->readable()), time );
		}

		if ( firstTime )
		{
			// save the tags from the linked file to the current location so it gets propagated to the root.
			NameList tags;
			linkedScene->readTags(tags);
			m_mainScene->writeTags( tags );
		}

		/// we keep the information this level has a link, so we can prevent attempts to 
		/// create children or save objects at this level.
		m_atLink = true;
	}

	m_mainScene->writeAttribute(name,attribute,time);
}

bool LinkedScene::hasTag( const Name &name ) const
{
	if ( m_linkedScene )
	{
		return m_linkedScene->hasTag( name );
	}
	else
	{
		return m_mainScene->hasTag( name );
	}
}

void LinkedScene::readTags( NameList &tags, bool includeChildren ) const
{
	if ( includeChildren && !m_readOnly )
	{
		throw Exception( "readTags with includeChildren option is only supported when reading the scene file!" );
	}

	if ( m_linkedScene )
	{
		return m_linkedScene->readTags( tags, includeChildren );
	}
	else
	{
		return m_mainScene->readTags( tags, includeChildren );
	}
}

void LinkedScene::writeTags( const NameList &tags )
{
	if ( m_readOnly )
	{
		throw Exception( "No write access to scene file!" );
	}
	if ( m_atLink )
	{
		throw Exception( "Locations with links to external scene cannot have tags themselves!" );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->numAttributeSamples( linkAttribute );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleTime( linkAttribute, sampleIndex );
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
			return static_cast<const SampledSceneInterface*>(m_mainScene.get())->attributeSampleInterval( linkAttribute, time, floorIndex, ceilIndex );
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

ObjectPtr LinkedScene::readObjectAtSample( size_t sampleIndex ) const
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

ObjectPtr LinkedScene::readObject( double time ) const
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
	if ( m_linkedScene )
	{
		return m_linkedScene->childNames(childNames);
	}
	else
	{
		return m_mainScene->childNames(childNames);
	}
}

ConstSceneInterfacePtr LinkedScene::expandLink( const CompoundData *linkData, int &linkDepth, bool &timeRemapped )
{
	/// we found the link attribute...
	const InternedStringVectorData *root = 0;
	if ( linkData )
	{
		const StringData *fileName = linkData->member< StringData >(g_fileNameLinkAttribute);
		root = linkData->member< InternedStringVectorData >(g_rootLinkAttribute);
		if ( fileName && root )
		{
			ConstSceneInterfacePtr l = SharedSceneInterfaces::get( fileName->readable() );
			linkDepth = root->readable().size();
			l = l->scene(root->readable(), NullIfMissing);
			if ( !l )
			{
				// \todo Consider throwing or printing error message.
				linkDepth = 0;
			}
			timeRemapped = bool(linkData->member< DoubleData >(g_timeAttribute));
			return l;
		}
	}
	linkDepth = 0;
	return 0;
}

double LinkedScene::remappedLinkTime( double time ) const
{
	ConstCompoundDataPtr d = runTimeCast< CompoundData >( m_mainScene->readAttribute( linkAttribute, time ) );
	if ( !d )
	{
		throw Exception( "Invalid link attribute when querying for time remapping!" );
	}
	ConstDoubleDataPtr t = d->member< DoubleData >(g_timeAttribute);
	if ( !t )
	{
		throw Exception( "Invalid time when querying for time remapping!" );
	}
	return t->readable();
}

double LinkedScene::remappedLinkTimeAtSample( size_t sampleIndex ) const
{
	ConstCompoundDataPtr d = runTimeCast< CompoundData >( static_cast<const SampledSceneInterface*>(m_mainScene.get())->readAttributeAtSample( linkAttribute, sampleIndex ) );
	if ( !d )
	{
		throw Exception( "Invalid link attribute when querying for time remapping!" );
	}
	ConstDoubleDataPtr t = d->member< DoubleData >(g_timeAttribute);
	if ( !t )
	{
		throw Exception( "Invalid time when querying for time remapping!" );
	}
	return t->readable();
}

SceneInterfacePtr LinkedScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	if ( missingBehaviour == SceneInterface::CreateIfMissing )
	{
		if ( m_readOnly )
		{
			throw Exception( "No write access to scene file!" );
		}
		if ( m_atLink )
		{
			throw Exception( "Locations with links to external scene cannot have child locations themselves!" );
		}
	}

	if ( m_linkedScene )
	{
		ConstSceneInterfacePtr c = m_linkedScene->child( name, missingBehaviour );
		if ( !c )
		{
			return 0;
		}
		return new LinkedScene( m_mainScene, c, m_rootLinkDepth, m_readOnly, false, m_timeRemapped );
	}
	else
	{
		SceneInterfacePtr c = m_mainScene->child( name, missingBehaviour );
		if ( !c )
		{
			return 0;
		}
		if ( m_readOnly && c->hasAttribute( linkAttribute ) )
		{
			ConstCompoundDataPtr d = runTimeCast< CompoundData >( c->readAttribute( linkAttribute, 0 ) );
			/// we found the link attribute...
			int linkDepth;
			bool timeRemapped;
			ConstSceneInterfacePtr l = expandLink( d, linkDepth, timeRemapped );
			if ( l )
			{
				return new LinkedScene( c, l, linkDepth, m_readOnly, true, timeRemapped );
			}
		}

		return new LinkedScene( c, 0, 0, m_readOnly, false, false );
	}
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
	ConstSceneInterfacePtr l = 0;
	int linkDepth = 0;
	bool atLink = (s->hasAttribute( linkAttribute ));
	bool timeRemapped = false;

	if ( atLink )
	{
		ConstCompoundDataPtr d = runTimeCast< CompoundData >( s->readAttribute( linkAttribute, 0 ) );
		l = expandLink( d, linkDepth, timeRemapped );
		if (!l)
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
				return 0;
			}
			throw Exception( "Could not find child '" + pIt->value() + "'"  );
		}

		for ( ; pIt != path.end(); pIt++ )
		{
			l = l->child( *pIt, missingBehaviour );
			if ( !l )
			{
				return 0;
			}
		}
		atLink = false;
	}
	return new LinkedScene( s, l, linkDepth, m_readOnly, atLink, timeRemapped );
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
