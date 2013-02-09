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

#include "tbb/concurrent_hash_map.h"
#include "OpenEXR/ImathBoxAlgo.h"
#include "IECore/SceneCache.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/HeaderGenerator.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"

using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( SceneCache )

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

// Register FileIndexedIO as the handler for .scc files so that people
// can use the filename constructor as a convenience over the IndexedIO
// constructor.
static IndexedIO::Description<FileIndexedIO> extensionDescription( ".scc" );
// Register the .scc extension in the factory function for SceneCache on Read and Write modes
static SceneInterface::FileFormatDescription<SceneCache> registrar(".scc", IndexedIO::Read | IndexedIO::Write);

static InternedString headerEntry("header");
static InternedString rootEntry("root");
static InternedString boundEntry("bound");
static InternedString transformEntry("transform");
static InternedString objectEntry("object");
static InternedString attributesEntry("attributes");
static InternedString childrenEntry("children");
static InternedString sampleTimesEntry("sampleTimes");

typedef std::vector<double> SampleTimes;

class SceneCache::Implementation : public RefCounted
{
	public :
	
		virtual ~Implementation()
		{
		}

		bool hasObject() const
		{
			return m_indexedIO->hasEntry( objectEntry );
		}

		bool hasAttribute( const Name &name ) const
		{
			ConstIndexedIOPtr attributes = m_indexedIO->subdirectory( attributesEntry, IndexedIO::NullIfMissing );
			if ( !attributes )
				return false;

			return attributes->hasEntry( name );
		}

		void readAttributeNames( NameList &attrsNames ) const
		{
			ConstIndexedIOPtr attributes = m_indexedIO->subdirectory( attributesEntry, IndexedIO::NullIfMissing );
			if ( !attributes )
			{
				// it's ok for an entry to not have children
				attrsNames.clear();
				return;
			}
			attributes->entryIds( attrsNames, IndexedIO::Directory );
		}

		void childNames( NameList &childNames ) const
		{
			ConstIndexedIOPtr children = m_indexedIO->subdirectory( childrenEntry, IndexedIO::NullIfMissing );
			if ( !children )
			{
				// it's ok for an entry to not have children
				childNames.clear();
				return;
			}
			children->entryIds( childNames, IndexedIO::Directory );
		}

		bool hasChild( const Name &name ) const
		{
			ConstIndexedIOPtr children = m_indexedIO->subdirectory( childrenEntry, IndexedIO::NullIfMissing );
			if ( !children )
			{
				return false;
			}
			return children->hasEntry( name );
		}

	protected :

		Implementation( IndexedIOPtr io ) : m_indexedIO(io)
		{
		}

		// This function converts an integer to a IndexedIO::EntryID object (InternedString)
		// \todo Remove this when InternedString nativelly supports a constructor with integers.
		static IndexedIO::EntryID sampleEntry( size_t sample )
		{
			return InternedString( ( boost::format("%d") % sample ).str() );
		}

		static inline Imath::M44d dataToMatrix( const Data *data )
		{
			switch ( data->typeId() )
			{
				case M44dDataTypeId:
					return static_cast< const M44dData * >(data)->readable();
				case TransformationMatrixdDataTypeId:
					return static_cast< const TransformationMatrixdData * >(data)->readable().transform();
				default:
					throw Exception( "Unsupported transform data type!" );
			}
		}

		IndexedIOPtr m_indexedIO;
};

/// Reader implementation for SceneCache
/// Child locations keep a refcount pointer to their parent, so they can always ask path, read global sample times, go up in the chain.
class SceneCache::ReaderImplementation : public SceneCache::Implementation
{
	public :

		IE_CORE_DECLAREPTR( ReaderImplementation )

		ReaderImplementation( IndexedIOPtr io, SceneCache::Implementation *parent = 0) : SceneCache::Implementation( io ), m_parent(static_cast< ReaderImplementation* >( parent )), m_sampleTimesMap(0), m_boundSampleTimes(0), m_transformSampleTimes(0), m_objectSampleTimes(0)
		{
			if ( m_parent )
			{
				// use same map from the root
				m_sampleTimesMap = m_parent->m_sampleTimesMap;
			}
			else
			{
				// only the root instance allocate the map.
				m_sampleTimesMap = new SampleTimesMap;
			}
		}
	
		virtual ~ReaderImplementation()
		{
			if ( m_sampleTimesMap && !m_parent )
			{
				delete m_sampleTimesMap;
			}
		}

		const SceneCache::Name &name() const
		{
			if ( m_parent )
			{
				return m_indexedIO->currentEntryId();
			}
			else
			{
				return SceneInterface::rootName;
			}
		}

		void path( SceneCache::Path &p ) const
		{
			if ( m_parent )
			{
				m_parent->path( p );
				p.push_back( m_indexedIO->currentEntryId() );
			}
		}

		const SampleTimes &boundSampleTimes() const
		{
			if ( !m_boundSampleTimes )
			{
				m_boundSampleTimes = restoreSampleTimes( boundEntry );
			}
			return *m_boundSampleTimes;
		}

		double boundSampleTime( size_t sampleIndex ) const
		{
			const SampleTimes &sampleTimes = boundSampleTimes();
			if ( sampleIndex >= sampleTimes.size() )
			{
				throw Exception( "Sample index out of bounds!" );
			}
			return sampleTimes[sampleIndex];
		}

		static inline double sampleInterval( const SampleTimes &sampleTimes, double time, size_t &floorIndex, size_t &ceilIndex )
		{
			SampleTimes::const_iterator it = sampleTimes.begin();
			for ( ; it != sampleTimes.end(); it++ )
			{
				if ( time > *it )
					break;
			}
			if ( it == sampleTimes.begin() )
			{
				ceilIndex = floorIndex = 0;
				return 0;
			}
			if ( it == sampleTimes.end() )
			{
				ceilIndex = floorIndex = sampleTimes.size() - 1;
				return 0;
			}
			ceilIndex = (it - sampleTimes.begin());
			floorIndex = ceilIndex - 1;
			return (time - sampleTimes[floorIndex]) / (sampleTimes[ceilIndex] - sampleTimes[floorIndex]);
		}

		double boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			const SampleTimes &sampleTimes = boundSampleTimes();
			return sampleInterval( sampleTimes, time, floorIndex, ceilIndex );
		}

		size_t numBoundSamples() const
		{
			const SampleTimes &sampleTimes = boundSampleTimes();
			return sampleTimes.size();
		}

		Imath::Box3d readBoundAtSample( size_t sampleIndex ) const
		{
			IndexedIOPtr io = m_indexedIO->subdirectory( boundEntry, IndexedIO::NullIfMissing );
			if ( !io )
			{
				if ( sampleIndex==0 )
				{
					return g_defaults.defaultBox;
				}
				else
				{
					throw Exception( "Sample index out of bounds!" );
				}
			}
			Box3d result;
			double *resultAddress = result.min.getValue();
			io->read( sampleEntry(sampleIndex), resultAddress, 6 );
			return result;
		}

		Imath::Box3d readBound( double time ) const
		{
			size_t sample1, sample2;
			double x = boundSampleInterval( time, sample1, sample2 );
			if ( x == 0 )
			{
				return readBoundAtSample( sample1 );
			} 
			if ( x == 1 )
			{
				return readBoundAtSample( sample2 );
			}

			Imath::Box3d box1 = readBoundAtSample( sample1 );
			Imath::Box3d box2 = readBoundAtSample( sample2 );
			Imath::Box3d box;
			LinearInterpolator<Imath::Box3d>()(box1, box2, x, box);
			return box;
		}

		inline const SampleTimes &transformSampleTimes() const
		{
			if ( !m_transformSampleTimes )
			{
				m_transformSampleTimes = restoreSampleTimes( transformEntry );
			}
			return *m_transformSampleTimes;
		}

		size_t numTransformSamples() const
		{
			const SampleTimes &sampleTimes = transformSampleTimes();
			return sampleTimes.size();
		}

		double transformSampleTime( size_t sampleIndex ) const
		{
			const SampleTimes &sampleTimes = transformSampleTimes();
			if ( sampleIndex >= sampleTimes.size() )
			{
				throw Exception( "Sample index out of bounds!" );
			}
			return sampleTimes[sampleIndex];
		}

		double transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			const SampleTimes &sampleTimes = transformSampleTimes();
			return sampleInterval( sampleTimes, time, floorIndex, ceilIndex );
		}

		DataPtr readTransformAtSample( size_t sampleIndex ) const
		{
			IndexedIOPtr io = m_indexedIO->subdirectory( transformEntry, IndexedIO::NullIfMissing );
			if ( !io )
			{
				if ( sampleIndex==0 )
				{
					return g_defaults.defaultTransform;
				}
				else
				{
					throw Exception( "Sample index out of bounds!" );
				}
			}
			return runTimeCast<Data>( Object::load( io, sampleEntry(sampleIndex) ) );
		}

		Imath::M44d readTransformAsMatrixAtSample( size_t sampleIndex ) const
		{
			return dataToMatrix( readTransformAtSample( sampleIndex ) );
		}

		DataPtr readTransform( double time ) const
		{
			size_t sample1, sample2;
			double x = transformSampleInterval( time, sample1, sample2 );
			if ( x == 0 )
			{
				return readTransformAtSample( sample1 );
			}
			if ( x == 1 )
			{
				return readTransformAtSample( sample2 );
			}
			DataPtr transformData1 = readTransformAtSample( sample1 );
			DataPtr transformData2 = readTransformAtSample( sample2 );
			DataPtr transformData = runTimeCast< Data >( linearObjectInterpolation( transformData1, transformData2, x ) );
			if ( !transformData )
			{
				// failed to interpolate, return the closest one
				return ( x >= 0.5 ? transformData2 : transformData1 );
			}
			return transformData;
		}

		Imath::M44d readTransformAsMatrix( double time ) const
		{
			return dataToMatrix( readTransform( time ) );
		}

		inline const SampleTimes &attributeSampleTimes( const SceneCache::Name &name ) const
		{
			// \todo consider tbb::concurrent_hash_map instead
			std::pair< AttributeSamplesMap::iterator, bool > it = m_attributeSampleTimes.insert( std::pair< IndexedIO::EntryID, SampleTimes* >( name, NULL ) );
			if ( it.second )
			{
				it.first->second = restoreSampleTimes( attributesEntry, false, &name );
			}
			if ( !it.first->second )
			{
				throw Exception( ( boost::format( "No samples for attribute %s available" ) % name.value() ).str() );
			}
			return *(it.first->second);
		}

		size_t numAttributeSamples( const SceneCache::Name &name ) const
		{
			const SampleTimes &sampleTimes = attributeSampleTimes(name);
			return sampleTimes.size();
		}

		double attributeSampleTime( const SceneCache::Name &name, size_t sampleIndex ) const
		{
			const SampleTimes &sampleTimes = attributeSampleTimes( name );
			if ( sampleIndex >= sampleTimes.size() )
			{
				throw Exception( "Sample index out of bounds!" );
			}
			return sampleTimes[sampleIndex];
		}

		double attributeSampleInterval( const SceneCache::Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			const SampleTimes &sampleTimes = attributeSampleTimes( name );
			return sampleInterval( sampleTimes, time, floorIndex, ceilIndex );
		}

		ObjectPtr readAttributeAtSample( const SceneCache::Name &name, size_t sampleIndex )
		{
			return Object::load( m_indexedIO->subdirectory(attributesEntry)->subdirectory(name), sampleEntry(sampleIndex) );
		}

		ObjectPtr readAttribute( const SceneCache::Name &name, double time )
		{
			size_t sample1, sample2;
			double x = attributeSampleInterval( name, time, sample1, sample2 );
			if ( x == 0 )
			{
				return readAttributeAtSample( name, sample1 );
			}
			if ( x == 1 )
			{
				return readAttributeAtSample( name, sample2 );
			}

			ObjectPtr attributeObj1 = readAttributeAtSample( name, sample1 );
			ObjectPtr attributeObj2 = readAttributeAtSample( name, sample2 );
			ObjectPtr attributeObj = linearObjectInterpolation( attributeObj1, attributeObj2, x );
			if ( !attributeObj )
			{
				// failed to interpolate, return the closest one
				return ( x >= 0.5 ? attributeObj2 : attributeObj1 );
			}
			return attributeObj;
		}

		inline const SampleTimes &objectSampleTimes() const
		{
			if ( !m_objectSampleTimes )
			{
				m_objectSampleTimes = restoreSampleTimes( objectEntry, true );
			}
			return *m_objectSampleTimes;
		}

		size_t numObjectSamples() const
		{
			const SampleTimes &sampleTimes = objectSampleTimes();
			return sampleTimes.size();
		}

		double objectSampleTime( size_t sampleIndex ) const
		{
			const SampleTimes &sampleTimes = objectSampleTimes();
			if ( sampleIndex >= sampleTimes.size() )
			{
				throw Exception( "Sample index out of bounds!" );
			}
			return sampleTimes[sampleIndex];
		}

		double objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			const SampleTimes &sampleTimes = objectSampleTimes();
			return sampleInterval( sampleTimes, time, floorIndex, ceilIndex );
		}

		ObjectPtr readObjectAtSample( size_t sampleIndex ) const
		{
			return Object::load( m_indexedIO->subdirectory( objectEntry ), sampleEntry(sampleIndex) );
		}

		ObjectPtr readObject( double time ) const
		{
			size_t sample1, sample2;
			double x = objectSampleInterval( time, sample1, sample2 );
			if ( x == 0 )
			{
				return readObjectAtSample( sample1 );
			}
			if ( x == 1 )
			{
				return readObjectAtSample( sample2 );
			}

			ObjectPtr object1 = readObjectAtSample( sample1 );
			ObjectPtr object2 = readObjectAtSample( sample2 );
			ObjectPtr object = linearObjectInterpolation( object1, object2, x );
			if ( !object )
			{
				// failed to interpolate, return the closest one
				return ( x >= 0.5 ? object2 : object1 );
			}
			return object;
		}

		ReaderImplementationPtr child( const Name &name, MissingBehaviour missingBehaviour )
		{
			IndexedIOPtr children = m_indexedIO->subdirectory( childrenEntry, (IndexedIO::MissingBehaviour)missingBehaviour );
			if ( !children )
			{
				return 0;
			}
			IndexedIOPtr childIO = children->subdirectory( name, (IndexedIO::MissingBehaviour)missingBehaviour );
			if ( !childIO )
			{
				return 0;
			}
			return new ReaderImplementation( childIO, this );
		}

		SceneCache::ImplementationPtr scene( const Path &path, MissingBehaviour missingBehaviour )
		{
			// go to root of the scene and than CD to the location...
			ReaderImplementation *root = this;
			while( root->m_parent )
			{
				root = root->m_parent.get();
			}

			ReaderImplementationPtr location = root;			
			for ( Path::const_iterator it = path.begin(); location && it != path.end(); it++) 
			{
				location = location->child( *it, missingBehaviour );
			}
			return location;
		}

		static ReaderImplementation *reader( Implementation *impl, bool throwException = true )
		{
			ReaderImplementation *reader = dynamic_cast< ReaderImplementation* >( impl );
			if ( throwException && !reader )
			{
				throw Exception( "Function not supported when writing a scene file." );
			}
			return reader;
		}

	private :
	
		// \todo Consider using concurrent_vector for constant access time.
		typedef tbb::concurrent_hash_map< uint64_t, SampleTimes > SampleTimesMap;
		typedef std::map< IndexedIO::EntryID, const SampleTimes* > AttributeSamplesMap;

		ReaderImplementationPtr m_parent;
		mutable SampleTimesMap *m_sampleTimesMap;

		/// pointers to values in m_sampleTimesMap.
		mutable const SampleTimes *m_boundSampleTimes;
		mutable const SampleTimes *m_transformSampleTimes;
		mutable AttributeSamplesMap m_attributeSampleTimes;
		mutable const SampleTimes *m_objectSampleTimes;

		IndexedIOPtr globalSampleTimes() const
		{
			if ( m_parent )
			{
				return m_parent->globalSampleTimes();
			}
			return m_indexedIO->parentDirectory()->subdirectory( sampleTimesEntry );
		}

		const SampleTimes *restoreSampleTimes( const IndexedIO::EntryID &childName, bool throwExceptions = false, const IndexedIO::EntryID *attribName = 0 ) const
		{
			IndexedIOPtr location = m_indexedIO->subdirectory( childName, IndexedIO::NullIfMissing );
			if ( location && attribName )
			{
				location = m_indexedIO->subdirectory( *attribName, IndexedIO::NullIfMissing );
			}
			if ( !location || !location->hasEntry( sampleTimesEntry ) )
			{
				if ( throwExceptions )
				{
					throw Exception( (boost::format("No %s samples available") % childName.value()).str() );
				}
				return &g_defaults.implicitSample;
			}

			uint64_t sampleTimesIndex = 0;
			location->read( sampleTimesEntry, sampleTimesIndex );

			{
				SampleTimesMap::const_accessor cit;
				if ( m_sampleTimesMap->find( cit, sampleTimesIndex ) )
				{
					return &(cit->second);
				}
			}

			const SceneInterface::Name &sampleEntryId = sampleEntry(sampleTimesIndex);
			// never loaded before...
			// change our reading location to the global location.
			location = globalSampleTimes();
			IndexedIO::Entry e = location->entry( sampleEntryId );
			// adds an item to the map
			SampleTimes times;
			times.resize( e.arrayLength() );
			double *ptrTimes = &times[0];
			// and loads the sample times on to the map before returning
			location->read( sampleEntryId, ptrTimes, times.size() );
			SampleTimesMap::accessor it;
			if ( m_sampleTimesMap->insert( it, sampleTimesIndex ) )
			{
				it->second = times;
			}
			return &(it->second);
		}

		/// Determine defaults when transform and bounds are not stored in the file.
		/// The reader will return one sample at time 0 with empty bounding box and
		/// with identity transform.
		static struct Defaults
		{
			SampleTimes implicitSample;
			M44dDataPtr defaultTransform;
			Imath::Box3d defaultBox;

			Defaults()
			{
				implicitSample.push_back( 0.0 );
				defaultTransform = new M44dData();
				defaultTransform->writable().makeIdentity();
				defaultBox.makeEmpty();
			}
		} g_defaults;
};

SceneCache::ReaderImplementation::Defaults SceneCache::ReaderImplementation::g_defaults;

/// Writer implementation for SceneCache
/// Each location keeps refcount pointers to their child locations, so they can always return the same (unfinished child) and when the root is destroyed, it
/// can trigger the recursive computation of bounding boxes and the global storage of all sampleTime vectors used in the file.
class SceneCache::WriterImplementation : public SceneCache::Implementation
{
	public :

		IE_CORE_DECLAREPTR( WriterImplementation )

		WriterImplementation( IndexedIOPtr io, Implementation *parent = 0) : SceneCache::Implementation( io ), m_parent(static_cast< WriterImplementation* >( parent ))
		{
			if ( m_parent )
			{
				// use same map from the root
				m_sampleTimesMap = m_parent->m_sampleTimesMap;
			}
			else
			{
				// only the root instance allocate the map.
				m_sampleTimesMap = new SampleTimesMap;
			}
		}

		virtual ~WriterImplementation()
		{
			// the root location destruction triggers the flush on the file.
			if ( !m_parent )
			{
				flush();
			}
		}

		const SceneCache::Name &name() const
		{
			if ( m_parent )
			{
				return m_indexedIO->currentEntryId();
			}
			return SceneInterface::rootName;
		}

		void path( SceneCache::Path &p ) const
		{
			IndexedIO::EntryIDList ioPath;
			m_indexedIO->path( ioPath );
			IndexedIO::EntryIDList::const_iterator pIt = ioPath.begin();
			pIt++;	// skip root entry
			while( pIt != ioPath.end() )
			{
				pIt++; // skip children entry
				p.push_back( *pIt );
				pIt++; // skip child name entry
			}
		}

		void writeBound( const Imath::Box3d &bound, double time )
		{
			writable();

			if ( m_boundSampleTimes.size() )
			{
				if ( *(m_boundSampleTimes.rbegin()) >= time )
				{
					throw Exception( "Times must be incremental amongst calls to writeBound!" );
				}
			}
			size_t sampleIndex = m_boundSampleTimes.size();
			m_boundSampleTimes.push_back( time );
			m_boundSamples.push_back( bound );
			IndexedIOPtr io = m_indexedIO->subdirectory( boundEntry, IndexedIO::CreateIfMissing );
			io->write( sampleEntry(sampleIndex), bound.min.getValue(), 6 );
		}

		void writeTransform( const Data *transform, double time )
		{
			writable();

			if ( !m_parent )
			{
				throw Exception( "Call to writeTransform at the root scene is not allowed!" );
			}
			switch (transform->typeId())
			{
				case M44dDataTypeId:
				case TransformationMatrixdDataTypeId:
					break;
				default:
					throw Exception( "Invalid transform data type! Must be M44dData or TransformationMatrixdData." );
			}
			if ( m_transformSampleTimes.size() )
			{
				if ( *(m_transformSampleTimes.rbegin()) >= time )
				{
					throw Exception( "Times must be incremental amongst calls to writeTransform!" );
				}
			}
			size_t sampleIndex = m_transformSampleTimes.size();
			m_transformSampleTimes.push_back( time );
			IndexedIOPtr io = m_indexedIO->subdirectory( transformEntry, IndexedIO::CreateIfMissing );
			((const Object *)transform)->save( io, sampleEntry(sampleIndex) );
			m_transformSamples.push_back( transform );
		}

		void writeAttribute( const SceneCache::Name &name, const Object *attribute, double time )
		{
			writable();

			std::pair< AttributeSamplesMap::iterator, bool > it = m_attributeSampleTimes.insert( std::pair< SceneCache::Name, SampleTimes >( name, SampleTimes() ) );
			SampleTimes &sampleTimes = it.first->second;
			if ( !it.second )
			{
				if ( *(sampleTimes.rbegin()) >= time )
				{
					throw Exception( "Times must be incremental amongst calls to writeAttribute for the same attribute!" );
				}
			}
			size_t sampleIndex = sampleTimes.size();
			sampleTimes.push_back( time );
			IndexedIOPtr io = m_indexedIO->subdirectory( attributesEntry, IndexedIO::CreateIfMissing );
			io = io->subdirectory( name, IndexedIO::CreateIfMissing );
			attribute->save( io, sampleEntry(sampleIndex) );
		}

		void writeObject( const Object *object, double time )
		{
			writable();

			if ( !m_parent )
			{
				throw Exception( "Call to writeObject at the root scene is not allowed!" );
			}
			if ( m_objectSampleTimes.size() )
			{
				if ( *(m_objectSampleTimes.rbegin()) >= time )
				{
					throw Exception( "Times must be incremental amongst calls to writeObject!" );
				}
			}
			size_t sampleIndex = m_objectSampleTimes.size();
			m_objectSampleTimes.push_back( time );
			IndexedIOPtr io = m_indexedIO->subdirectory( objectEntry, IndexedIO::CreateIfMissing );
			object->save( io, sampleEntry(sampleIndex) );
			const VisibleRenderable *renderable = runTimeCast< const VisibleRenderable >( object );
			if ( renderable )
			{
				if ( !m_objectSamples.size() && m_objectSampleTimes.size() > 1 )
				{
					throw Exception( "Either all object samples must have bounds (VisibleRenderable) or none of them!" );
				}
				Box3f bf = renderable->bound();
				Box3d bd(
					V3d( bf.min.x, bf.min.y, bf.min.z ),
					V3f( bf.max.x, bf.max.y, bf.max.z )
				);
				m_objectSamples.push_back( bd );
			}
			else
			{
				if ( m_objectSamples.size() )
				{
					throw Exception( "Either all object samples must have bounds (VisibleRenderable) or none of them!" );
				}
			}
		}

		WriterImplementationPtr child( const Name &name, MissingBehaviour missingBehaviour )
		{
			if ( missingBehaviour == SceneInterface::CreateIfMissing )
			{
				writable();
			}

			std::map< SceneCache::Name, WriterImplementationPtr >::const_iterator it = m_children.find( name );
			if ( it != m_children.end() )
			{
				return it->second;
			}

			IndexedIOPtr children = m_indexedIO->subdirectory( childrenEntry, (IndexedIO::MissingBehaviour)missingBehaviour );
			if ( !children )
			{
				return 0;
			}
			IndexedIOPtr childIO = children->subdirectory( name, (IndexedIO::MissingBehaviour)missingBehaviour );
			if ( !childIO )
			{
				return 0;
			}
			WriterImplementationPtr result = new WriterImplementation( childIO, this );
			this->m_children[ name ] = result;
			return result;
		}

		SceneCache::ImplementationPtr createChild( const SceneCache::Name &name )
		{
			writable();
			IndexedIOPtr children = m_indexedIO->subdirectory( childrenEntry, IndexedIO::CreateIfMissing );
			if ( children->hasEntry( name ) )
			{
				throw Exception( "Child already exists!" );
			}
			IndexedIOPtr childIO = children->createSubdirectory( name );
			WriterImplementationPtr result = new WriterImplementation( childIO, this );
			this->m_children[ name ] = result;
			return result;
		}

		static WriterImplementation *writer( Implementation *impl, bool throwException = true )
		{
			WriterImplementation *writer = dynamic_cast< WriterImplementation* >( impl );
			if ( throwException && !writer )
			{
				throw Exception( "Function not supported when loading a scene file." );
			}
			return writer;
		}

	private :

		typedef std::vector< Imath::Box3d > BoxSamples;

		IndexedIOPtr globalSampleTimes()
		{
			if ( m_parent )
			{
				return m_parent->globalSampleTimes();
			}
			return m_indexedIO->parentDirectory()->subdirectory( sampleTimesEntry );
		}

		void writable() const
		{
			if ( !m_sampleTimesMap )
			{
				throw Exception( "This scene has already been flushed to disk. You can't make further changes to it." );
			}
		}

		// Function to store intelligently the given sample times in the file location.
		// It actually saves the index there, and stores the unique sample times in a global shared location.
		void storeSampleTimes( const SampleTimes &sampleTimes, IndexedIOPtr location )
		{
			assert( m_sampleTimesMap );
			uint64_t sampleTimesIndex = 0;
			std::pair< SampleTimesMap::iterator, bool > it = m_sampleTimesMap->insert( std::pair< SampleTimes, uint64_t >( sampleTimes, 0 ) );
			if ( it.second )
			{
				// Unique Id for the sampleTimes (incremental integer)
				sampleTimesIndex = m_sampleTimesMap->size() - 1;
				// store the uniqueId in the global map
				it.first->second = sampleTimesIndex;
				// find the global location for the sample times from the root Scene.
				IndexedIOPtr sampleTimesIO = globalSampleTimes();
				// write the sampleTimes in the file
				sampleTimesIO->write( sampleEntry(sampleTimesIndex), &sampleTimes[0], sampleTimes.size() );
			}
			else
			{
				// already saved in the global sample times section...
				sampleTimesIndex = it.first->second;
			}
			location->write( sampleTimesEntry, sampleTimesIndex );
		}

		// function called when bounding boxes were not explicitly defined in this scene location.
		// the function accumulates bounding box samples in the variables m_boundSampleTimes and m_boundSamples.
		void accumulateBoxSamples( const SampleTimes &sampleTimes, const BoxSamples &boxSamples )
		{
			/// simple case: zero new samples
			if ( !sampleTimes.size() )
			{
				return;
			}

			/// simple case: first time we simply copy the arrays...
			if ( !m_boundSampleTimes.size() )
			{
				m_boundSampleTimes = sampleTimes;
				m_boundSamples = boxSamples;
				return;
			}

			SampleTimes::const_iterator newTimeIt = sampleTimes.begin();
			BoxSamples::const_iterator newBoxIt = boxSamples.begin();

			//
			// Prepending phase: 
			//
			// All the new samples that exist prior to all the registered samples should  
			// be prepended to the current samples buffer. Compute the union of the new samples 
			// with the oldest sample known
			//
			double oldestKnownTime = m_boundSampleTimes[0];
			Imath::Box3d oldestKnownBox = m_boundSamples[0];
			for ( ; newTimeIt != sampleTimes.end() && *newTimeIt < oldestKnownTime; newTimeIt++ )
			{
				break;
			}

			SampleTimes::iterator currTimeIt = m_boundSampleTimes.begin();
			BoxSamples::iterator currBoxIt = m_boundSamples.begin();

			size_t prependCount = (newTimeIt - sampleTimes.begin());
			m_boundSampleTimes.insert( m_boundSampleTimes.begin(), sampleTimes.begin(), newTimeIt );
			m_boundSamples.insert( m_boundSamples.begin(), prependCount, oldestKnownBox );
			for ( size_t i = 0; i < prependCount; i++, newBoxIt++ )
			{
				m_boundSamples[i].extendBy( *newBoxIt );
			}
			currTimeIt += prependCount;
			currBoxIt += prependCount;

			// check if there's still samples to add...
			if ( newTimeIt == sampleTimes.end() )
			{
				return;
			}
			
			//
			// Mixing phase: 
			//
			// New samples that match the time stamp for a known sample or is defined
			// between two known samples.
			//
			double prevNewSampleTime = ( newTimeIt == sampleTimes.begin() ? *newTimeIt : *(newTimeIt - 1) );
			Imath::Box3d prevNewSampleBox = ( newBoxIt == boxSamples.begin() ? *newBoxIt : *(newBoxIt - 1) );
			double newestKnownTime = *(m_boundSampleTimes.rbegin());
			Imath::Box3d newestKnownBox = *(m_boundSamples.rbegin());

			Imath::Box3d tmpBox;
			LinearInterpolator<Box3d> boxInterpolator;

			for( ; newTimeIt != sampleTimes.end() && *newTimeIt <= newestKnownTime; newTimeIt++, newBoxIt++ )
			{
				while ( *newTimeIt > *currTimeIt )
				{
					// the new sample comes after the current known sample, so we expand the known
					// sample's bounding box by the interpolated new sample bounding box.
					double x = ( (*currTimeIt > prevNewSampleTime) ? (*currTimeIt-prevNewSampleTime)/(*newTimeIt-prevNewSampleTime) : 0 );
					boxInterpolator( prevNewSampleBox, *newBoxIt, x, tmpBox );
					currBoxIt->extendBy( tmpBox );
					currTimeIt++;
					currBoxIt++;
				}
				if ( *newTimeIt == *currTimeIt )
				{
					// the new sample matches perfectly with a known sample, so we just apply union on the bboxes
					currBoxIt->extendBy( *newBoxIt );
					currTimeIt++;
					currBoxIt++;
				}
				else
				{
					// the new sample comes prior to the current known sample, so we must insert in the array of known samples.
					if ( currTimeIt == m_boundSampleTimes.begin() )
					{
						// this is the first known sample so nothing to interpolate...
						tmpBox = *currBoxIt;
					}
					else
					{
						// interpolate known samples.
						double prevKnownTime = *(currTimeIt-1);
						double x = (*newTimeIt -prevKnownTime) /((*currTimeIt)-prevKnownTime);
						boxInterpolator( *(currBoxIt-1), *currBoxIt, x, tmpBox );
					}
					tmpBox.extendBy( *newBoxIt );
					// add the new box to the array of known samples
					currTimeIt = m_boundSampleTimes.insert( currTimeIt, *newTimeIt ) + 1;
					currBoxIt = m_boundSamples.insert( currBoxIt, tmpBox ) + 1;
				}
				prevNewSampleTime = *newTimeIt;
				prevNewSampleBox = *newBoxIt;
			}

			// check if there's still samples to add...
			if ( newTimeIt == sampleTimes.end() )
			{
				return;
			}

			//
			// Appending phase: 
			//
			// New samples that succeeds any known sample should append to the sample list
			// by doing the union with the most recent known bounding box.
			//

			size_t appendCount = (sampleTimes.end() - newTimeIt);
			m_boundSampleTimes.insert( currTimeIt, newTimeIt, sampleTimes.end() );
			m_boundSamples.insert( currBoxIt, appendCount, newestKnownBox );
			for ( size_t i = 0; i < appendCount; i++, currBoxIt++, newBoxIt++ )
			{
				currBoxIt->extendBy( *newBoxIt );
			}
		}

		// Called from the destructor of the root location. 
		// It triggers flush recursivelly on all the child locations.
		// It also sets m_sampleTimesMap to NULL which prevents further modification on this and all child scene interface objects through their call to writable().
		// Responsible for writing missing data such as all the sample 
		// times from object,transform,attributes and bounds. And also computes the 
		// animated bounding boxes in case they were not explicitly writen.
		//
		void flush()
		{
			/// first call flush recursively on children...
			for ( std::map< SceneCache::Name, WriterImplementationPtr >::const_iterator cit = m_children.begin(); cit != m_children.end(); cit++ )
			{
				cit->second->flush();
			}

			IndexedIOPtr io;
			// save the transform sample times
			if ( m_transformSampleTimes.size() )
			{
				io = m_indexedIO->subdirectory( transformEntry, IndexedIO::CreateIfMissing );
				storeSampleTimes( m_transformSampleTimes, io );
			}
			// save the attribute sample times
			if ( m_attributeSampleTimes.size() )
			{
				io = m_indexedIO->subdirectory( attributesEntry, IndexedIO::CreateIfMissing );
				for ( AttributeSamplesMap::const_iterator it = m_attributeSampleTimes.begin(); it != m_attributeSampleTimes.end(); it++ )
				{
					storeSampleTimes( it->second, io->subdirectory( it->first, IndexedIO::CreateIfMissing ) );
				}
			}
			// save the object sample times
			if ( m_objectSampleTimes.size() )
			{
				io = m_indexedIO->subdirectory( objectEntry, IndexedIO::CreateIfMissing );
				storeSampleTimes( m_objectSampleTimes, io );				
			}
			// We have to compute the bounding box over time for the object and each child if there's no bound overrides writen.
			bool computedBounds = false;
			if ( m_boundSampleTimes.size() == 0 )
			{
				computedBounds = true;
				for ( std::map< SceneCache::Name, WriterImplementationPtr >::const_iterator cit = m_children.begin(); cit != m_children.end(); cit++ )
				{
					const SampleTimes &childBoundTimes = cit->second->m_boundSampleTimes;
					const BoxSamples &childBoxSamples = cit->second->m_boundSamples;
					const SampleTimes &childTransformTimes = cit->second->m_transformSampleTimes;
					const std::vector< TransformSample > &childTransformSamples = cit->second->m_transformSamples; 

					if ( childBoundTimes.size() == 0 )
					{
						continue;
					}

					if ( childTransformTimes.size() == 0 )
					{
						// no transform or animation applied to this child... we just accumulate it.
						accumulateBoxSamples( childBoundTimes, childBoxSamples );
					}
					else if ( childTransformTimes.size() == 1 )
					{
						M44d m = dataToMatrix( childTransformSamples[0].get() );
						// there's just one constant transform applied to the children, very simple case
						BoxSamples transformedChildBoxes;
						transformedChildBoxes.reserve( childBoundTimes.size() );
						for ( BoxSamples::const_iterator cbit = childBoxSamples.begin(); cbit != childBoxSamples.end(); cbit++ )
						{
							transformedChildBoxes.push_back( transform( *cbit, m ) );
						}
						// accumulate the resulting transformed bounding boxes
						accumulateBoxSamples( childBoundTimes, transformedChildBoxes );
					}
					else // childTransformTimes.size() > 1
					{
						BoxSamples transformedChildBoxes;

						if ( childBoundTimes.size() > 1 )
						{
							SampleTimes transformedChildSamples;

							// complex case: animated transforms. 
							// Apply transform interpolation whenever bbox changes, and apply bbox interpolation 
							// whenever transform changes.
							transformedChildSamples.reserve( childBoundTimes.size() + childTransformTimes.size() );
							transformedChildBoxes.reserve( childBoundTimes.size() + childTransformTimes.size() );
							SampleTimes::const_iterator transformTimeIt, childTimeIt;
							BoxSamples::const_iterator childBoxIt;
							transformTimeIt = childTransformTimes.begin();
							childTimeIt = childBoundTimes.begin();
							std::vector< TransformSample >::const_iterator transformIt = childTransformSamples.begin();
							childBoxIt = childBoxSamples.begin();
							Imath::Box3d tmpBox;
							LinearInterpolator<Box3d> boxInterpolator;

							while( childTimeIt != childBoundTimes.end() && transformTimeIt != childTransformTimes.end() )
							{
								if ( *childTimeIt < *transformTimeIt )
								{
									// Situation: child sample comes before the transform sample: interpolate transform and apply to child bbox.
									ConstDataPtr t;
									if ( transformIt == childTransformSamples.begin() )
									{
										t = *transformIt;
									}
									else
									{
										double prevTransformTime = *(transformTimeIt-1);
										double x = (*childTimeIt - prevTransformTime)/(*transformTimeIt - prevTransformTime);
										ConstObjectPtr o1 = *(transformIt-1);
										ConstObjectPtr o2 = *transformIt;
										ObjectPtr a;
										ObjectPtr obj = linearObjectInterpolation( a, a, x );
										t = runTimeCast< Data >( obj );
									}
									transformedChildSamples.push_back( *childTimeIt );
									transformedChildBoxes.push_back( transform( *childBoxIt, dataToMatrix( t.get() ) ) );
									childTimeIt++;
									childBoxIt++;
								}
								else if ( *transformTimeIt < *childTimeIt )
								{
									// Situation: transform sample comes before the child sample: interpolate child bbox and apply transform.
									if ( childBoxIt == childBoxSamples.begin() )
									{
										// this is the first known sample so nothing to interpolate...
										tmpBox = *childBoxIt;
									}
									else
									{
										// interpolate known samples.
										double prevChildTime = *(childTimeIt-1);
										double x = (*transformTimeIt -prevChildTime) /((*childTimeIt)-prevChildTime);
										boxInterpolator( *(childBoxIt-1), *childBoxIt, x, tmpBox );
									}
									transformedChildSamples.push_back( *transformTimeIt );
									transformedChildBoxes.push_back( transform( tmpBox, dataToMatrix( transformIt->get() ) ) );
									transformTimeIt++;
									transformIt++;
								}
								else
								{
									// Situation: child sample matches the time of the transform sample: transform child bbox.
									transformedChildSamples.push_back( *childTimeIt );
									transformedChildBoxes.push_back( transform( *childBoxIt, dataToMatrix( transformIt->get() ) ) );
									childTimeIt++;
									childBoxIt++;
									transformTimeIt++;
									transformIt++;
								}
							}

							M44d lastTransformMatrix = dataToMatrix( *childTransformSamples.rbegin() );
							while( childTimeIt != childBoundTimes.end() )
							{
								// Situation: child samples exist after all the transform samples.
								transformedChildSamples.push_back( *childTimeIt );
								transformedChildBoxes.push_back( transform( *childBoxIt, lastTransformMatrix ) );
								childTimeIt++;
								childBoxIt++;
							}

							tmpBox = *(childBoxSamples.rbegin());
							while( transformTimeIt != childTransformTimes.end() )
							{
								// Situation: transform samples exist after all the child samples
								transformedChildSamples.push_back( *transformTimeIt );
								transformedChildBoxes.push_back( transform( tmpBox, dataToMatrix( transformIt->get() ) ) );
								transformTimeIt++;
								transformIt++;
							}

							// \todo We also want to add some border in the sampled bounding boxes to 
							// guarantee that the interpolated rotations that trace curves in space
							// would still be included in the linear interpolated bounding boxes.

							// accumulate the resulting transformed bounding boxes
							accumulateBoxSamples( transformedChildSamples, transformedChildBoxes );
						}
						else
						{
							Imath::Box3d tmpBox;
							if ( childBoxSamples.size() )
							{
								tmpBox = childBoxSamples[0];
							}
							// the child has no animation, so we just have to transform at each transform sample.
							transformedChildBoxes.reserve( childTransformTimes.size() );
							for ( std::vector< TransformSample >::const_iterator tit = childTransformSamples.begin(); tit != childTransformSamples.end(); tit++ )
							{
								transformedChildBoxes.push_back( transform( tmpBox, dataToMatrix( tit->get() ) ) );
							}
							// \todo We also want to add some border in the sampled bounding boxes to 
							// guarantee that the interpolated rotations that trace curves in space
							// would still be included in the linear interpolated bounding boxes.

							// accumulate the resulting transformed bounding boxes
							accumulateBoxSamples( childTransformTimes, transformedChildBoxes );							
						}
					}
				}

				if ( m_objectSampleTimes.size() && m_objectSamples.size() )
				{
					// union all the bounding box samples from the child and also from the optional object stored in this location
					accumulateBoxSamples( m_objectSampleTimes, m_objectSamples );
				}

			}
			if ( m_boundSampleTimes.size() )
			{
				// save the bound sample times
				io = m_indexedIO->subdirectory( boundEntry, IndexedIO::CreateIfMissing );
				storeSampleTimes( m_boundSampleTimes, io );
				if ( computedBounds )
				{
					// store computed bounds in file
					uint64_t sampleIndex = 0;
					for ( BoxSamples::const_iterator bit = m_boundSamples.begin(); bit != m_boundSamples.end(); bit++, sampleIndex++ )
					{
						io->write( sampleEntry(sampleIndex), bit->min.getValue(), 6 );
					}
				}
			}
			// deallocate children since we now computed everything from them anyways...
			m_children.clear();

			if ( !m_parent && m_sampleTimesMap )
			{
				// deallocate samples map stored in the root object.
				delete m_sampleTimesMap;
			}
			m_sampleTimesMap = 0;
		}

		typedef std::map< SampleTimes, uint64_t > SampleTimesMap;
		WriterImplementation* m_parent;
		SampleTimesMap *m_sampleTimesMap;

		// store the transform objects (we want to interpolate the transforms later)
		typedef ConstDataPtr TransformSample;
		// store the object's bounding box (we want to transform them later)
		typedef std::map< SceneCache::Name, SampleTimes > AttributeSamplesMap;

		SampleTimes m_boundSampleTimes;		// implicit or explicit bound sample times
		SampleTimes m_transformSampleTimes;
		AttributeSamplesMap m_attributeSampleTimes;
		SampleTimes m_objectSampleTimes;
		std::vector< TransformSample > m_transformSamples;
		BoxSamples m_objectSamples;
		std::map< SceneCache::Name, WriterImplementationPtr > m_children;
		BoxSamples m_boundSamples;
};

//////////////////////////////////////////////////////////////////////////
// SceneCache
//////////////////////////////////////////////////////////////////////////

SceneCache::SceneCache( const std::string &fileName, IndexedIO::OpenMode mode )
{
	if( mode & IndexedIO::Append )
	{
		throw InvalidArgumentException( "Append mode not supported" );
	}
	IndexedIOPtr indexedIO = IndexedIO::create( fileName, IndexedIO::rootPath, mode );

	if( indexedIO->openMode() & IndexedIO::Write )
	{
		ObjectPtr header = HeaderGenerator::header();
		header->save( indexedIO, headerEntry );
		indexedIO->subdirectory( sampleTimesEntry, IndexedIO::CreateIfMissing );
		indexedIO = indexedIO->subdirectory( rootEntry, IndexedIO::CreateIfMissing );
		indexedIO->removeAll();
		m_implementation = new WriterImplementation( indexedIO );
	}
	else
	{
		indexedIO = indexedIO->subdirectory( rootEntry );
		m_implementation = new ReaderImplementation( indexedIO );
	}
}

SceneCache::SceneCache( IECore::IndexedIOPtr indexedIO )
{
	if( indexedIO->openMode() & IndexedIO::Append )
	{
		throw InvalidArgumentException( "Append mode not supported" );
	}
	IndexedIO::EntryIDList path;
	indexedIO->path( path );
	if ( path.size() )
	{
		throw InvalidArgumentException( "The given IndexedIO object is not at root!" );
	}

	if( indexedIO->openMode() & IndexedIO::Write )
	{
		ObjectPtr header = HeaderGenerator::header();
		header->save( indexedIO, headerEntry );
		indexedIO = indexedIO->subdirectory( rootEntry, IndexedIO::CreateIfMissing );
		indexedIO->removeAll();
		m_implementation = new WriterImplementation( indexedIO );
	}
	else
	{
		indexedIO = indexedIO->subdirectory( rootEntry );
		m_implementation = new ReaderImplementation( indexedIO );
	}
}

SceneCache::SceneCache( ImplementationPtr impl )
	:	m_implementation( impl )
{
}

SceneCache::~SceneCache()
{
}

void SceneCache::path( SceneCache::Path &p ) const
{
	p.clear();
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get(), false );
	if ( reader )
	{
		reader->path( p );
	}
	else
	{
		WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
		writer->path( p );
	}
}

const SceneCache::Name &SceneCache::name() const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get(), false );
	if ( reader )
	{
		return reader->name();
	}
	else
	{
		WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
		return writer->name();
	}
}

size_t SceneCache::numBoundSamples() const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->numBoundSamples();
}

double SceneCache::boundSampleTime( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->boundSampleTime( sampleIndex );
}

double SceneCache::boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->boundSampleInterval( time, floorIndex, ceilIndex );
}

Imath::Box3d SceneCache::readBoundAtSample( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readBoundAtSample( sampleIndex );
}

Imath::Box3d SceneCache::readBound( double time ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readBound( time );
}

void SceneCache::writeBound( const Imath::Box3d &bound, double time )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	writer->writeBound( bound, time );
}

size_t SceneCache::numTransformSamples() const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->numTransformSamples();
}

double SceneCache::transformSampleTime( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->transformSampleTime( sampleIndex );
}

double SceneCache::transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->transformSampleInterval( time, floorIndex, ceilIndex );
}

DataPtr SceneCache::readTransformAtSample( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readTransformAtSample( sampleIndex );
}

Imath::M44d SceneCache::readTransformAsMatrixAtSample( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readTransformAsMatrixAtSample( sampleIndex );
}

DataPtr SceneCache::readTransform( double time ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readTransform( time );
}

Imath::M44d SceneCache::readTransformAsMatrix( double time ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readTransformAsMatrix( time );
}

void SceneCache::writeTransform( const Data *transform, double time )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	writer->writeTransform( transform, time );
}

bool SceneCache::hasAttribute( const Name &name ) const
{
	return m_implementation->hasAttribute(name);
}

void SceneCache::readAttributeNames( NameList &attrs ) const
{
	m_implementation->readAttributeNames(attrs);
}

size_t SceneCache::numAttributeSamples( const Name &name ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->numAttributeSamples( name );
}

double SceneCache::attributeSampleTime( const Name &name, size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->attributeSampleTime( name, sampleIndex );
}

double SceneCache::attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->attributeSampleInterval( name, time, floorIndex, ceilIndex );
}

ObjectPtr SceneCache::readAttributeAtSample( const Name &name, size_t sampleIndex )
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readAttributeAtSample( name, sampleIndex );
}

ObjectPtr SceneCache::readAttribute( const Name &name, double time )
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readAttribute( name, time );
}

void SceneCache::writeAttribute( const Name &name, const Object *attribute, double time )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	writer->writeAttribute( name, attribute, time );
}

bool SceneCache::hasObject() const
{
	return m_implementation->hasObject();
}

size_t SceneCache::numObjectSamples() const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->numObjectSamples();
}

double SceneCache::objectSampleTime( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->objectSampleTime( sampleIndex );
}

double SceneCache::objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->objectSampleInterval( time, floorIndex, ceilIndex );
}

ObjectPtr SceneCache::readObjectAtSample( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readObjectAtSample( sampleIndex );
}

ObjectPtr SceneCache::readObject( double time ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readObject( time );
}

void SceneCache::writeObject( const Object *object, double time )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	writer->writeObject( object, time );
}

void SceneCache::childNames( NameList &childNames ) const
{
	return m_implementation->childNames(childNames);
}

SceneInterfacePtr SceneCache::child( const Name &name, SceneCache::MissingBehaviour missingBehaviour )
{
	ImplementationPtr impl;
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get(), false );
	if ( reader )
	{
		impl = reader->child( name, missingBehaviour );
	}
	else
	{
		WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
		impl = writer->child( name, missingBehaviour );
	}
	
	if ( !impl )
	{
        	return 0;
	}
	
	return new SceneCache( impl );
}

ConstSceneInterfacePtr SceneCache::child( const Name &name, SceneCache::MissingBehaviour missingBehaviour ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	ImplementationPtr impl = reader->child( name, missingBehaviour );
	if ( !impl )
	{
        	return 0;
	}
	
	return new SceneCache( impl );
}

bool SceneCache::hasChild( const Name &name ) const
{
	return m_implementation->hasChild(name);	
}

SceneInterfacePtr SceneCache::createChild( const Name &name )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	return new SceneCache( writer->createChild( name ) );
}

SceneInterfacePtr SceneCache::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	ImplementationPtr impl = reader->scene( path, missingBehaviour );
	if ( !impl )
	{
        	return 0;
	}
	
	return new SceneCache( impl );
}

ConstSceneInterfacePtr SceneCache::scene( const Path &path, SceneCache::MissingBehaviour missingBehaviour ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	ImplementationPtr impl = reader->scene( path, missingBehaviour );
	if ( !impl )
	{
        	return 0;
	}
	
	return new SceneCache( impl );
}
