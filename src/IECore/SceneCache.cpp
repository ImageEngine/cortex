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

#include "boost/tuple/tuple.hpp"
#include "tbb/concurrent_hash_map.h"

#include "OpenEXR/ImathBoxAlgo.h"

#include "IECore/SceneCache.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/HeaderGenerator.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/ObjectInterpolator.h"
#include "IECore/Primitive.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/SharedSceneInterfaces.h"
#include "IECore/MessageHandler.h"
#include "IECore/ComputationCache.h"

using namespace IECore;
using namespace Imath;
using namespace boost;

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
static InternedString tagsEntry("tags");
static InternedString localTagsEntry("localTags");
static InternedString ancestorTagsEntry("ancestorTags");
static InternedString descendentTagsEntry("descendentTags");

const SceneInterface::Name &SceneCache::animatedObjectTopologyAttribute = InternedString( "sceneInterface:animatedObjectTopology" );
const SceneInterface::Name &SceneCache::animatedObjectPrimVarsAttribute = InternedString( "sceneInterface:animatedObjectPrimVars" );

typedef std::vector<double> SampleTimes;

class SceneCache::Implementation : public RefCounted
{
	public :

		virtual ~Implementation()
		{
		}

		std::string fileName() const
		{
			if ( m_indexedIO->typeId() == FileIndexedIOTypeId )
			{
				return static_cast< FileIndexedIO * >( m_indexedIO.get() )->fileName();
			}
			throw Exception( "File name not available in scene cache!" );
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

		void attributeNames( NameList &attrsNames ) const
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

		bool hasTag( const Name &name, int filter ) const
		{
			if ( !filter )
			{
				return false;
			}

			if ( filter & SceneInterface::LocalTag )
			{
				ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( localTagsEntry, IndexedIO::NullIfMissing );
				if ( tagsIO && tagsIO->hasEntry( name ) )
				{
					return true;
				}
			}

			if ( filter & SceneInterface::DescendantTag )
			{
				ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( descendentTagsEntry, IndexedIO::NullIfMissing );
				if ( tagsIO && tagsIO->hasEntry( name ) )
				{
					return true;
				}
			}

			if ( filter & SceneInterface::AncestorTag )
			{
				ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( ancestorTagsEntry, IndexedIO::NullIfMissing );
				if ( tagsIO && tagsIO->hasEntry( name ) )
				{
					return true;
				}
			}

			/// provided for backward compatibility.
			ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( tagsEntry, IndexedIO::NullIfMissing );
			if ( tagsIO && tagsIO->hasEntry( name ) )
			{
				if ( filter == SceneInterface::LocalTag )
				{
					return ( tagsIO->entry(name).entryType() == IndexedIO::File );
				}
				return true;
			}

			return false;
		}

		void readTags( NameList &tags, int filter ) const
		{
			tags.clear();

			if ( !filter )
			{
				return;
			}

			if ( (filter & SceneInterface::LocalTag) )
			{
				ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( localTagsEntry, IndexedIO::NullIfMissing );
				if ( tagsIO )
				{
					tagsIO->entryIds( tags );
				}
			}

			NameList tmpTags;

			if ( (filter & SceneInterface::AncestorTag) )
			{
				ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( ancestorTagsEntry, IndexedIO::NullIfMissing );
				if ( tagsIO )
				{
					tagsIO->entryIds( tmpTags );
					tags.insert( tags.end(), tmpTags.begin(), tmpTags.end() );
				}
			}

			if ( (filter & SceneInterface::DescendantTag) )
			{
				ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( descendentTagsEntry, IndexedIO::NullIfMissing );
				if ( tagsIO )
				{
					tagsIO->entryIds( tmpTags );
					tags.insert( tags.end(), tmpTags.begin(), tmpTags.end() );
				}
			}

			if ( !tags.size() )
			{
				/// provided for backward compatibility...
				ConstIndexedIOPtr tagsIO = m_indexedIO->subdirectory( tagsEntry, IndexedIO::NullIfMissing );
				if ( tagsIO )
				{
					if ( filter == SceneInterface::LocalTag )
					{
						tagsIO->entryIds( tags, IndexedIO::File );
					}
					else
					{
						tagsIO->entryIds( tags );
					}
				}
			}
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
			return InternedString( sample );
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

		ReaderImplementation( IndexedIOPtr io, SceneCache::Implementation *parent = 0) : SceneCache::Implementation( io ), m_parent(static_cast< ReaderImplementation* >( parent )), m_sharedData(0), m_boundSampleTimes(0), m_transformSampleTimes(0), m_objectSampleTimes(0)
		{
			if ( m_parent )
			{
				// use same map from the root
				m_sharedData = m_parent->m_sharedData;
			}
			else
			{
				// only the root instance allocate the map.
				m_sharedData = new SharedData;
			}
		}

		virtual ~ReaderImplementation()
		{
			if ( m_sharedData && !m_parent )
			{
				delete m_sharedData;
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
				if ( !m_boundSampleTimes )
				{
					m_boundSampleTimes = &g_defaults.implicitSample;
				}
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
				if ( time <= *it )
				{
					break;
				}
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
			double x = (time - sampleTimes[floorIndex]) / (sampleTimes[ceilIndex] - sampleTimes[floorIndex]);
			if ( x < 1e-4 )
			{
				x = 0;
			}
			else if ( x > 1-1e-4)
			{
				x = 1;
			}
			return x;
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

		inline const SampleTimes &transformSampleTimes() const
		{
			if ( !m_transformSampleTimes )
			{
				m_transformSampleTimes = restoreSampleTimes( transformEntry );
				if ( !m_transformSampleTimes )
				{
					m_transformSampleTimes = &g_defaults.implicitSample;
				}
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

		ConstDataPtr readTransformAtSample( size_t sampleIndex ) const
		{
			return m_sharedData->readTransformAtSample( this, sampleIndex );
		}

		Imath::M44d readTransformAsMatrixAtSample( size_t sampleIndex ) const
		{
			return dataToMatrix( readTransformAtSample( sampleIndex ).get() );
		}

		inline const SampleTimes &attributeSampleTimes( const SceneCache::Name &name ) const
		{
			AttributeMapMutex::scoped_lock lock( m_attributeMutex, false );
			AttributeSamplesMap::const_iterator cit = m_attributeSampleTimes.find( name );
			if ( cit != m_attributeSampleTimes.end() )
			{
				return *(cit->second);
			}

			lock.upgrade_to_writer();

			std::pair< AttributeSamplesMap::iterator, bool > it = m_attributeSampleTimes.insert( std::pair< IndexedIO::EntryID, SampleTimes* >( name, NULL ) );
			if ( it.second )
			{
				it.first->second = restoreSampleTimes( attributesEntry, false, &name );
			}
			if ( !it.first->second )
			{
				m_attributeSampleTimes.erase( it.first );
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

		ConstObjectPtr readAttributeAtSample( const SceneCache::Name &name, size_t sampleIndex ) const
		{
			return m_sharedData->readAttributeAtSample( this, name, sampleIndex );
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

		ConstObjectPtr readObjectAtSample( size_t sampleIndex ) const
		{
			return m_sharedData->readObjectAtSample( this, sampleIndex );
		}

		static PrimitiveVariableMap readObjectPrimitiveVariablesAtSample( const IndexedIOPtr &io, const std::vector<InternedString> &primVarNames, size_t sample )
		{
			return Primitive::loadPrimitiveVariables( io->subdirectory( objectEntry ).get(), sampleEntry(sample), primVarNames );
		}

		PrimitiveVariableMap readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
		{
			size_t sample1, sample2;
			double x = objectSampleInterval( time, sample1, sample2 );

			if ( x == 0 )
			{
				return readObjectPrimitiveVariablesAtSample(m_indexedIO, primVarNames, sample1);
			}
			if ( x == 1 )
			{
				return readObjectPrimitiveVariablesAtSample(m_indexedIO, primVarNames, sample2);
			}

			IndexedIOPtr objectIO = m_indexedIO->subdirectory( objectEntry );
			PrimitiveVariableMap map1 = Primitive::loadPrimitiveVariables( objectIO.get(), sampleEntry(sample1), primVarNames );
			PrimitiveVariableMap map2 = Primitive::loadPrimitiveVariables( objectIO.get(), sampleEntry(sample2), primVarNames );

			for ( PrimitiveVariableMap::iterator it1 = map1.begin(); it1 != map1.end(); it1++ )
			{
				PrimitiveVariableMap::const_iterator it2 = map2.find( it1->first );
				if ( it2 == map2.end() )
				{
					continue;
				}
				it1->second.data = boost::static_pointer_cast< Data >( linearObjectInterpolation( it1->second.data.get(), it2->second.data.get(), x ) );
			}
			return map1;
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

		void hash( HashType hashType, double time, MurmurHash &h, bool ignoreSceneHash = false ) const
		{
			size_t s0, s1;
			double x;

			h.append( (unsigned char)hashType );

			// all kinds of hashes, except the child names depend on time.
			switch( hashType )
			{
				case TransformHash:

					if ( m_indexedIO->hasEntry( transformEntry ) )
					{
						x = transformSampleInterval( time, s0, s1 );
						h.append( lerp( (double)s0, (double)s1, x ) );
					}
					else
					{
						// return a simple hash for the identity transform (which does not include the scene location).
						return;
					}
					break;

				case AttributesHash:
					{
						NameList attrs;
						attributeNames( attrs );
						if ( !attrs.size() )
						{
							// return a simple hash for no attributes (which does not include the scene location).
							return;
						}
						for ( NameList::const_iterator aIt = attrs.begin(); aIt != attrs.end(); aIt++ )
						{
							x = attributeSampleInterval( *aIt, time, s0, s1 );
							h.append( lerp( (double)s0, (double)s1, x ) );
						}
					}
					break;

				case BoundHash:

					x = boundSampleInterval( time, s0, s1 );
					h.append( lerp( (double)s0, (double)s1, x ) );
					break;

				case ObjectHash:

					if ( m_indexedIO->hasEntry( objectEntry ) )
					{
						x = objectSampleInterval( time, s0, s1 );
						h.append( lerp( (double)s0, (double)s1, x ) );
					}
					else
					{
						// return a simple hash for no object (which does not include the scene location).
						return;
					}
					break;

				case ChildNamesHash:

					// child names do not depend on time.
					break;

				case HierarchyHash:

					if ( m_indexedIO->hasEntry( childrenEntry ) )
					{
						// we currently have no way to know if child locations are animated, we have to assume so...
						// \todo Consider writing animatedHierarchy tag at locations where there's animation and use it here.
						h.append( time );
					}
					else
					{
						// For leaf locations, we can find out if they are time dependent by adding the individual hashes for the location here.
						hash( AttributesHash, time, h, true );
						hash( BoundHash, time, h, true );
						hash( ObjectHash, time, h, true );
						hash( TransformHash, time, h, true );
					}
					break;
			}
			if ( !ignoreSceneHash )
			{
				// Because the hash computed so far is not based on the contents of the file, we have to add to the hash something that identifies the file and the location in the hierarchy.
				sceneHash( h );
			}
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
		typedef tbb::spin_rw_mutex AttributeMapMutex;

		typedef std::pair< const ReaderImplementation *, size_t > SimpleCacheKey;
		typedef tuple< const ReaderImplementation *, const SceneCache::Name &, size_t > AttributeCacheKey;

		typedef IECore::ComputationCache< SimpleCacheKey > SimpleCache;
		typedef IECore::ComputationCache< AttributeCacheKey > AttributeCache;

		/// Hold pointers to values allocated/deallocated by the root scene object (the last one to die)
		class SharedData : public RefCounted
		{
			public :

				SharedData() :
					objectCache( new SimpleCache( doReadObjectAtSample, simpleHash,  10000 )  ),
					attributeCache( new AttributeCache( doReadAttributeAtSample, attributeHash, 1000) ),
					transformCache( new SimpleCache(  doReadTransformAtSample, simpleHash, 1000) )
				{
				}

				/// utility function used by the ReaderImplementation to use the LRUCache for transform reading
				IECore::ConstDataPtr readTransformAtSample( const ReaderImplementation *reader, size_t sample )
				{
					return runTimeCast< const Data >( transformCache->get( SimpleCacheKey(reader, sample) ) );
				}

				/// utility function used by the ReaderImplementation to use the LRUCache for object reading
				IECore::ConstObjectPtr readObjectAtSample( const ReaderImplementation *reader, size_t sample )
				{
					const size_t defaultSample = -1;
					SimpleCacheKey currentKey( reader, sample );

					// if constant topology and the object is not in the cache, we try to build it from another frame
					if ( reader->hasAttribute(animatedObjectPrimVarsAttribute) )
					{
						/// Could not create the object from another time sample... so we load the entire object
						SimpleCacheKey defaultKey( reader, defaultSample );

						ConstObjectPtr obj = objectCache->get( currentKey, SimpleCache::NullIfMissing );
						if ( !obj )
						{
							/// ok, try to build the object from another frame...
							ConstObjectPtr defaultObj = objectCache->get( defaultKey, SimpleCache::NullIfMissing );
							if ( defaultObj )
							{
								IECore::ConstInternedStringVectorDataPtr varNames = runTimeCast<const InternedStringVectorData>( reader->readAttributeAtSample(animatedObjectPrimVarsAttribute, 0) );
								if ( varNames )
								{
									PrimitivePtr prim= runTimeCast< Primitive >( defaultObj->copy() );
									if ( prim )
									{
										// we managed to load the object from a different time sample from the cache, just have to load the changing prim vars...
										mergeMaps( prim->variables, readObjectPrimitiveVariablesAtSample( reader->m_indexedIO, varNames->readable(), sample ) );
										objectCache->set( currentKey, prim.get(), ObjectPool::StoreReference );
										return prim;
									}
								}
							}
							/// ok, we don't have the object even from other times in the cache... load it from the file then.
							obj = objectCache->get( currentKey );
						}
						/// register the object as the default, so next frames could reuse them
						objectCache->set( defaultKey, obj.get(), ObjectPool::StoreReference );
						return obj;
					}
					/// The object has animated topology... so we load the entire object
					ConstObjectPtr obj = objectCache->get(currentKey);
					return obj;
				}

				/// utility function used by the ReaderImplementation to use the LRUCache for attribute reading
				IECore::ConstObjectPtr readAttributeAtSample( const ReaderImplementation *reader, const SceneCache::Name &name, size_t sample )
				{
					return attributeCache->get( AttributeCacheKey(reader,name,sample) );
				}

				// \todo Consider adding "ReaderImplementation *rootScene" to optimize the scene() calls.
				SampleTimesMap sampleTimesMap;
				SimpleCache::Ptr objectCache;
				AttributeCache::Ptr attributeCache;
				SimpleCache::Ptr transformCache;

			private :

			// utility function that copies all the values from the rhs dictionary to the lhs.
			template< typename T >
			static void mergeMaps ( T& lhs, const T& rhs)
			{
	    		typename T::iterator lhsItr = lhs.begin();
				typename T::const_iterator rhsItr = rhs.begin();

				while (lhsItr != lhs.end() && rhsItr != rhs.end())
				{
					if (rhsItr->first < lhsItr->first)
					{
						lhs.insert(lhsItr, *rhsItr);
						++rhsItr;
					}
					else if (rhsItr->first == lhsItr->first)
					{
						lhsItr->second = rhsItr->second;
						++lhsItr;
						++rhsItr;
					}
					else
						++lhsItr;
				}
				lhs.insert(rhsItr, rhs.end());
			}

		};

		ReaderImplementationPtr m_parent;
		mutable SharedData *m_sharedData;

		/// pointers to values in m_sharedData->sampleTimesMap for the current scene location.
		mutable const SampleTimes *m_boundSampleTimes;
		mutable const SampleTimes *m_transformSampleTimes;
		mutable AttributeSamplesMap m_attributeSampleTimes;
		mutable AttributeMapMutex m_attributeMutex;
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
				location = location->subdirectory( *attribName, IndexedIO::NullIfMissing );
			}
			if ( !location || !location->hasEntry( sampleTimesEntry ) )
			{
				if ( throwExceptions )
				{
					throw Exception( (boost::format("No %s samples available") % childName.value()).str() );
				}
				return 0;
			}

			uint64_t sampleTimesIndex = 0;
			SceneInterface::Name sampleEntryId;
			if ( location->entry( sampleTimesEntry ).entryType() == IndexedIO::File )
			{
				/// Provided for backward compatibility.
				location->read( sampleTimesEntry, sampleTimesIndex );
				sampleEntryId = sampleEntry(sampleTimesIndex);
			}
			else
			{
				SceneInterface::NameList sampleList;
				location->subdirectory( sampleTimesEntry )->entryIds(sampleList);
				if ( sampleList.size() != 1 )
				{
					throw Exception( "Corrupted file! Could not find sample times key!!!" );
				}
				sampleEntryId = sampleList[0];
				sampleTimesIndex = atoi( sampleEntryId.value().c_str() );
			}

			{
				SampleTimesMap::const_accessor cit;
				if ( m_sharedData->sampleTimesMap.find( cit, sampleTimesIndex ) )
				{
					return &(cit->second);
				}
			}

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
			if ( m_sharedData->sampleTimesMap.insert( it, sampleTimesIndex ) )
			{
				it->second = times;
			}
			return &(it->second);
		}

		void sceneHash( MurmurHash &h ) const
		{
			if( FileIndexedIO *fileIndexedIO = runTimeCast<FileIndexedIO>( m_indexedIO.get() ) )
			{
				h.append( fileIndexedIO->fileName() );
			}
			else
			{
				/// \todo This isn't ideal as it isn't stable across different processes
				/// and might even produce non-unique hashes if this SceneCache is deleted
				/// and a new one is allocated with m_sharedData at the same memory address.
				/// If MemoryIndexedIO provided access to a cheap hash of its contents we
				/// could use that here. Alternatively we could compute the hashes of the data
				/// when writing it, and just load them here.
				h.append( (uint64_t)m_sharedData );
			}

			const ReaderImplementation *currScene = this;
			while( currScene->m_parent )
			{
				h.append( currScene->name() );
				currScene = currScene->m_parent.get();
			}
			h.append( currScene->name() );
		}

		static MurmurHash simpleHash( const SimpleCacheKey &key )
		{
			const ReaderImplementation *reader = key.first;
			size_t sample = key.second;
			MurmurHash h;
			reader->sceneHash( h );
			h.append( (uint64_t)sample );
			return h;
		}

		// static function used by the cache mechanism to actually load the object data from file.
		static ObjectPtr doReadTransformAtSample( const SimpleCacheKey &key )
		{
			IndexedIOPtr io = key.first->m_indexedIO->subdirectory( transformEntry, IndexedIO::NullIfMissing );
			if ( !io )
			{
				if ( key.second==0 )
				{
					return g_defaults.defaultTransform;
				}
				else
				{
					throw Exception( "Sample index out of bounds!" );
				}
			}
			return Object::load( io, sampleEntry(key.second) );
		}

		// static function used by the cache mechanism to actually load the object data from file.
		static ObjectPtr doReadObjectAtSample( const SimpleCacheKey &key )
		{
			return Object::load( key.first->m_indexedIO->subdirectory( objectEntry ), sampleEntry(key.second) );
		}

		static MurmurHash attributeHash( const AttributeCacheKey &key )
		{
			const ReaderImplementation *reader = get<0>( key );
			const SceneInterface::Name &name = get<1>( key );
			size_t sample = get<2>( key );

			MurmurHash h;
			reader->sceneHash( h );
			h.append(name.value());
			h.append( (uint64_t)sample );
			return h;
		}

		// static function used by the cache mechanism to actually load the attribute data from file.
		static ObjectPtr doReadAttributeAtSample( const AttributeCacheKey &key )
		{
			return Object::load( get<0>(key)->m_indexedIO->subdirectory(attributesEntry)->subdirectory(get<1>(key)), sampleEntry(get<2>(key)) );
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
				try
				{
					flush();
				}
				catch ( Exception &e )
				{
					msg( Msg::Error, "SceneCache::~SceneCache", ( boost::format( "Corrupted file resulted from exception while flushing data: %s." ) % e.what() ).str() );
				}
				catch (...)
				{
					msg( Msg::Error, "SceneCache::~SceneCache", "Corrupted file resulted from unknown exception while flushing data." );
				}
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
			m_boundSampleTimes.push_back( time );
			m_boundSamples.push_back( bound );
		}

		void writeTransform( const Data *transform, double time )
		{
			writable();

			if ( !transform )
			{
				throw Exception( "writeTransform: NULL transform data!" );
			}

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

			if ( !attribute )
			{
				throw Exception( "writeAttribute: NULL attribute data!" );
			}

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

		void writeLocalTag( const char *tag )
		{
			writable();
			IndexedIOPtr io = m_indexedIO->subdirectory( localTagsEntry, IndexedIO::CreateIfMissing );
			// we just create a IndexedIO::Directory
			io->subdirectory( tag, IndexedIO::CreateIfMissing );
		}

		void writeTags( const NameList &tags, int tagLocation = SceneInterface::LocalTag )
		{
			if ( !tags.size() )
			{
				return;
			}
			writable();
			IndexedIOPtr io(0);
			if ( tagLocation == SceneInterface::LocalTag )
			{
				io = m_indexedIO->subdirectory( localTagsEntry, IndexedIO::CreateIfMissing );
			}
			else if ( tagLocation == SceneInterface::AncestorTag )
			{
				io = m_indexedIO->subdirectory( ancestorTagsEntry, IndexedIO::CreateIfMissing );
			}
			else if ( tagLocation == SceneInterface::DescendantTag )
			{
				io = m_indexedIO->subdirectory( descendentTagsEntry, IndexedIO::CreateIfMissing );
			}
			else
			{
				assert(false);
			}
			for ( NameList::const_iterator tIt = tags.begin(); tIt != tags.end(); tIt++ )
			{
				// we just create a IndexedIO::Directory
				io->subdirectory( *tIt, IndexedIO::CreateIfMissing );
			}
		}

		void writeObject( const Object *object, double time )
		{
			writable();

			if ( !object )
			{
				throw Exception( "writeObject: NULL object data!" );
			}

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

				const Primitive *primitive = runTimeCast< const Primitive >( renderable );
				if ( primitive )
				{
					MurmurHash topologyHash;
					primitive->topologyHash( topologyHash );
					topologyHash.append( primitive->typeId() );
					if ( m_objectSamples.empty() )
					{
						m_animatedObjectTopology = AnimatedHashTest( topologyHash, false );
					}

					if ( topologyHash != m_animatedObjectTopology.first )
					{
						m_animatedObjectTopology.second = true;
					}

					for ( PrimitiveVariableMap::const_iterator it = primitive->variables.begin(); it != primitive->variables.end(); ++it )
					{
						Name primVarName = Name( it->first );

						MurmurHash hash;
						it->second.data->hash( hash );
						hash.append( it->second.interpolation );

						AnimatedPrimVarMap::iterator pIt = m_animatedObjectPrimVars.find( primVarName );
						if ( pIt == m_animatedObjectPrimVars.end() )
						{
							m_animatedObjectPrimVars.insert( AnimatedPrimVarMap::value_type( primVarName, AnimatedHashTest( hash, false ) ) );
						}
						else if ( hash != pIt->second.first )
						{
							pIt->second.second = true;
						}
					}
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

			if ( sampleIndex == 0 )
			{
				// save the type of object as a tag
				char objectTypeTag[128];
				strcpy( objectTypeTag, "ObjectType:");
				strcpy( &objectTypeTag[11], object->typeName() );
				writeLocalTag( objectTypeTag );
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
		typedef ConstDataPtr TransformSample;
		typedef std::vector< TransformSample > TransformSamples;

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
			uint64_t sampleTimesIndex = 	0;
			IndexedIO::EntryID samplesEntry;
			std::pair< SampleTimesMap::iterator, bool > it = m_sampleTimesMap->insert( std::pair< SampleTimes, uint64_t >( sampleTimes, 0 ) );
			if ( it.second )
			{
				// Unique Id for the sampleTimes (incremental integer)
				sampleTimesIndex = m_sampleTimesMap->size() - 1;
				// store the uniqueId in the global map
				it.first->second = sampleTimesIndex;
				// find the global location for the sample times from the root Scene.
				IndexedIOPtr sampleTimesIO = globalSampleTimes();
				samplesEntry = sampleEntry(sampleTimesIndex);
				// write the sampleTimes in the file
				sampleTimesIO->write( samplesEntry, &sampleTimes[0], sampleTimes.size() );
			}
			else
			{
				// already saved in the global sample times section...
				sampleTimesIndex = it.first->second;
				samplesEntry = sampleEntry(sampleTimesIndex);
			}
			location->createSubdirectory( sampleTimesEntry )->createSubdirectory( samplesEntry );
		}

		// Helper function which interpolates the time varying bounding box described by sampleTimes and boxSamples at time t,
		// then extends newSample by the resulting bounding box. "upper" should an iterator into sample times pointing to the
		// first element greater than t.
		void extendBoxByInterpolation( Imath::Box3d& box, const SampleTimes &sampleTimes, const BoxSamples &boxSamples, SampleTimes::const_iterator upper, double t )
		{
			if( upper == sampleTimes.begin() )
			{
				box.extendBy( boxSamples.front() );
			}
			else
			{
				SampleTimes::const_iterator lower = upper;
				--lower;

				Imath::Box3d tmpBox;
				double x = (t-*lower)/(*upper-*lower);
				const Imath::Box3d& lowerBox = boxSamples[ lower - sampleTimes.begin() ];
				const Imath::Box3d& upperBox = boxSamples[ upper - sampleTimes.begin() ];

				LinearInterpolator<Box3d> boxInterpolator;
				boxInterpolator(
					lowerBox,
					upperBox,
					x,
					tmpBox
				);
				box.extendBy( tmpBox );
			}
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

			// temporary storage for result:
			SampleTimes newSampleTimes;
			BoxSamples newBoxSamples;

			SampleTimes::const_iterator incomingTime = sampleTimes.begin();
			SampleTimes::const_iterator existingTime = m_boundSampleTimes.begin();

			// This algorithm takes the earliest unprocessed sample in either the incoming and existing bounding box samples.
			// It then finds the corresponding box in the other array by interpolation or extrapolation, extends the it box
			// by this interpolated box, and adds it to the result. This is repeated until everything is processed.
			while( incomingTime != sampleTimes.end() || existingTime != m_boundSampleTimes.end() )
			{
				if( incomingTime == sampleTimes.end() )
				{
					// no more incoming samples, just take the current existing sample and extend it by the last incoming sample:
					newSampleTimes.push_back( *existingTime );
					Imath::Box3d newSample = m_boundSamples[ existingTime - m_boundSampleTimes.begin() ];
					newSample.extendBy( boxSamples.back() );
					newBoxSamples.push_back( newSample );
					++existingTime;
				}
				else if( existingTime == m_boundSampleTimes.end() )
				{
					// no more existing samples, just take the current incoming sample and extend it by the last existing sample:
					newSampleTimes.push_back( *incomingTime );
					Imath::Box3d newSample = boxSamples[ incomingTime - sampleTimes.begin() ];
					newSample.extendBy( m_boundSamples.back() );
					newBoxSamples.push_back( newSample );
					++incomingTime;
				}
				else if( *incomingTime == *existingTime )
				{
					// coincident samples: combine the boxes!
					Imath::Box3d newSample = boxSamples[ incomingTime - sampleTimes.begin() ];
					newSample.extendBy( m_boundSamples[ existingTime - m_boundSampleTimes.begin() ] );
					newSampleTimes.push_back( *incomingTime );
					newBoxSamples.push_back( newSample );
					++incomingTime;
					++existingTime;
				}
				else if( *incomingTime < *existingTime )
				{
					// combine this incoming box with the interpolation of the existing boxes:
					Imath::Box3d newSample = boxSamples[ incomingTime - sampleTimes.begin() ];
					extendBoxByInterpolation( newSample, m_boundSampleTimes, m_boundSamples, existingTime, *incomingTime );
					newSampleTimes.push_back( *incomingTime );
					newBoxSamples.push_back( newSample );
					++incomingTime;
				}
				else
				{
					// combine this existing box with the interpolation of the incoming boxes:
					Imath::Box3d newSample = m_boundSamples[ existingTime - m_boundSampleTimes.begin() ];
					extendBoxByInterpolation( newSample, sampleTimes, boxSamples, incomingTime, *existingTime );
					newSampleTimes.push_back( *existingTime );
					newBoxSamples.push_back( newSample );
					++existingTime;
				}
			}

			m_boundSampleTimes.swap( newSampleTimes );
			m_boundSamples.swap( newBoxSamples );

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
			if ( m_parent )
			{
				NameList tags;
				// get ancestor tags from parent
				m_parent->readTags( tags, SceneInterface::LocalTag | SceneInterface::AncestorTag );
				writeTags( tags, SceneInterface::AncestorTag );
			}
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

			// detect if topology or prim vars are animated
			if ( !m_objectSampleTimes.empty() )
			{
				if ( m_animatedObjectTopology.second )
				{
					writeAttribute( animatedObjectTopologyAttribute, new BoolData( true ), 0 );
				}
				else
				{
					InternedStringVectorDataPtr primVarData = new InternedStringVectorData();
					std::vector<InternedString> &primVars = primVarData->writable();
					for ( AnimatedPrimVarMap::iterator it = m_animatedObjectPrimVars.begin(); it != m_animatedObjectPrimVars.end(); ++it )
					{
						if ( it->second.second )
						{
							primVars.push_back( it->first );
						}
					}

					writeAttribute( animatedObjectPrimVarsAttribute, primVarData.get(), 0 );
				}
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

			// We have to compute the bounding box over time for the object and each child.
			for ( std::map< SceneCache::Name, WriterImplementationPtr >::const_iterator cit = m_children.begin(); cit != m_children.end(); cit++ )
			{
				const SampleTimes &childBoundTimes = cit->second->m_boundSampleTimes;
				const BoxSamples &childBoxSamples = cit->second->m_boundSamples;
				const SampleTimes &childTransformTimes = cit->second->m_transformSampleTimes;
				const TransformSamples &childTransformSamples = cit->second->m_transformSamples;

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
					// (we can ignore it's time and just use the child box one)
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
						// complex case: animated transforms.

						// Step 1: Apply bbox interpolation for each transform sample that doesn't have a corresponding bbox sample.
						SampleTimes transformedChildSampleTimes;

						transformedChildSampleTimes.reserve( childBoundTimes.size() + childTransformTimes.size() );
						transformedChildBoxes.reserve( childBoundTimes.size() + childTransformTimes.size() );

						SampleTimes::const_iterator transformTimeIt, childTimeIt;
						BoxSamples::const_iterator childBoxIt;
						transformTimeIt = childTransformTimes.begin();
						childTimeIt = childBoundTimes.begin();
						TransformSamples::const_iterator transformIt = childTransformSamples.begin();
						childBoxIt = childBoxSamples.begin();
						Imath::Box3d tmpBox;
						LinearInterpolator<Box3d> boxInterpolator;

						while( childTimeIt != childBoundTimes.end() && transformTimeIt != childTransformTimes.end() )
						{
							if ( *childTimeIt < *transformTimeIt )
							{
								// Situation: child sample comes before the transform sample: interpolate transform.
								transformedChildSampleTimes.push_back( *childTimeIt );
								transformedChildBoxes.push_back( *childBoxIt );
								childTimeIt++;
								childBoxIt++;
							}
							else if ( *transformTimeIt < *childTimeIt )
							{
								// Situation: transform sample comes before the child sample: interpolate child bbox.
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
								transformedChildSampleTimes.push_back( *transformTimeIt );
								transformedChildBoxes.push_back( tmpBox );
								transformTimeIt++;
								transformIt++;
							}
							else
							{
								// Situation: child sample matches the time of the transform sample: transform child bbox.
								transformedChildSampleTimes.push_back( *childTimeIt );
								transformedChildBoxes.push_back( *childBoxIt );
								childTimeIt++;
								childBoxIt++;
								transformTimeIt++;
								transformIt++;
							}
						}

						while( childTimeIt != childBoundTimes.end() )
						{
							// Situation: child samples exist after all the transform samples.
							transformedChildSampleTimes.push_back( *childTimeIt );
							transformedChildBoxes.push_back( *childBoxIt );
							childTimeIt++;
							childBoxIt++;
						}

						tmpBox = *(childBoxSamples.rbegin());
						while( transformTimeIt != childTransformTimes.end() )
						{
							// Situation: transform samples exist after all the child samples
							transformedChildSampleTimes.push_back( *transformTimeIt );
							transformedChildBoxes.push_back( tmpBox );
							transformTimeIt++;
							transformIt++;
						}

						// We also want to add some border in the sampled bounding boxes to
						// guarantee that the interpolated rotations that trace curves in space
						// would still be included in the linear interpolated bounding boxes.
						// then we transform the child bboxes...
						transformAndExpandBounds( childTransformTimes, childTransformSamples, transformedChildSampleTimes, transformedChildBoxes );

						// accumulate the resulting transformed bounding boxes
						accumulateBoxSamples( transformedChildSampleTimes, transformedChildBoxes );
					}
					else
					{
						// the child object does not vary in time, so we just have to transform at each transform
						// sample (and we can ignore the sample time for the box - if existent)

						Imath::Box3d tmpBox;
						if ( childBoxSamples.size() )
						{
							tmpBox = childBoxSamples[0];
						}

						transformedChildBoxes.resize( childTransformTimes.size(), tmpBox );

						// We also want to add some border in the sampled bounding boxes to
						// guarantee that the interpolated rotations that trace curves in space
						// would still be included in the linear interpolated bounding boxes.
						// then we transform the child bboxes...
						transformAndExpandBounds( childTransformTimes, childTransformSamples, childTransformTimes, transformedChildBoxes );
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


			if ( m_boundSampleTimes.size() )
			{
				// save the bound sample times
				io = m_indexedIO->subdirectory( boundEntry, IndexedIO::CreateIfMissing );
				storeSampleTimes( m_boundSampleTimes, io );

				// store computed bounds in file
				uint64_t sampleIndex = 0;
				for ( BoxSamples::const_iterator bit = m_boundSamples.begin(); bit != m_boundSamples.end(); bit++, sampleIndex++ )
				{
					io->write( sampleEntry(sampleIndex), bit->min.getValue(), 6 );
				}
			}

			if ( m_parent )
			{
				NameList tags;
				// propagate tags to parent
				readTags( tags, SceneInterface::LocalTag | SceneInterface::DescendantTag );
				m_parent->writeTags( tags, SceneInterface::DescendantTag );
			}

			// deallocate children since we now computed everything from them anyways...
			m_children.clear();

			if ( !m_parent && m_sampleTimesMap )
			{
				// we are at the root...
				// deallocate samples map stored in the root object.
				delete m_sampleTimesMap;
				// and make sure the cache does not contain this file, forcing it to reload it.
				if ( m_indexedIO->typeId() == FileIndexedIOTypeId )
				{
					SharedSceneInterfaces::erase( static_cast< FileIndexedIO * >( m_indexedIO.get() )->fileName() );
				}
			}
			m_sampleTimesMap = 0;
		}

		/// This functions transforms the bounding boxes with the animated transforms and also scales the bounding boxes in a way that it
		/// guarantees that the original bounding boxes transformed at any time (which would trace curved trajectories in space),
		/// would always be fully included in the linear interpolation of the resulting transformed bounding boxes.
		/// So we garantee parent nodes can linearly interpolate child bounding boxes.
		static void transformAndExpandBounds( const SampleTimes &transformTimes, const TransformSamples &transformSamples, const SampleTimes &boxTimes, BoxSamples &boxSamples )
		{
			SampleTimes::const_iterator ttIt = transformTimes.begin();
			SampleTimes::const_iterator btIt = boxTimes.begin();
			TransformSamples::const_iterator tsIt = transformSamples.begin();
			BoxSamples::iterator bsIt = boxSamples.begin();

			if ( transformTimes.size() != transformSamples.size() )
			{
				throw Exception( "Mismatch number of transform samples!" );
			}

			if ( boxTimes.size() != boxSamples.size() )
			{
				throw Exception( "Mismatch number of box samples!" );
			}

			LinearInterpolator<Box3d> boxInterpolator;
			Imath::Box3d previousBox = *bsIt;
			TransformSample previousTransform = *tsIt;
			double previousTransformTime = *ttIt;
			Imath::M44d previousTransformMatrix = dataToMatrix( previousTransform.get() );
			Imath::Box3d previousTransformedBox;
			TransformSample nextTransform = 0;
			ttIt++;
			tsIt++;

			/// transform all box samples that come prior to the first transform as static boxes transformed by the first transform.
			while( btIt != boxTimes.end() && *btIt <= previousTransformTime )
			{
				previousBox = *bsIt;
				previousTransformedBox = transform(previousBox, previousTransformMatrix);
				*bsIt = previousTransformedBox;
				bsIt++;
				btIt++;
			}

			/// go on each bbox sample and increase them if necessary
			for( ; btIt != boxTimes.end() && ttIt != transformTimes.end(); btIt++, bsIt++ )
			{
				double boxTime = *btIt;
				double transformTime = *ttIt;

				if ( boxTime < transformTime )
				{
					// interpolate transform to the next box sample
					double x = (boxTime-previousTransformTime)/(transformTime-previousTransformTime);
					nextTransform = runTimeCast< Data >( linearObjectInterpolation( previousTransform.get(), tsIt->get(), x ) );
					if ( !nextTransform )
					{
						throw Exception( "Failed to interpolate transforms while computing bounding boxes! Different types?" );
					}
				}
				else if ( boxTime == transformTime )
				{
					nextTransform = *tsIt;
					tsIt++;
					ttIt++;
				}
				else
				{
					throw Exception( "Box sample without a transform sample!" );
				}

				Imath::Box3d nextBox = *bsIt;
				Imath::Box3d nextTransformedBox = transform( nextBox, dataToMatrix( nextTransform.get() ) );
				// we don't know how to handle empty bboxes...
				if ( !nextBox.isEmpty() && !previousTransformedBox.isEmpty() )
				{
					Imath::Box3d transfomedInterpBox;
					Imath::Box3d interpTransformedBoxes;
					Imath::Box3d totalExpansion;

					/// \todo consider making steps a parameter.
					int steps = 10;	/// subdivide interval between samples in X steps
					double xStep = 1.0 / steps;
					for ( double x = xStep; x < 1.0; x += xStep )
					{
						// compute the transformed interpolation of bounding boxes
						boxInterpolator( previousBox, nextBox, x, transfomedInterpBox );
						ObjectPtr obj = linearObjectInterpolation( previousTransform.get(), nextTransform.get(), x );
						if ( !obj )
						{
							throw Exception( "Failed to interpolate transforms while computing bounding boxes! Different types?" );
						}
						TransformSample interpTransform = runTimeCast< Data >( obj );
						transfomedInterpBox = transform( transfomedInterpBox, dataToMatrix( interpTransform.get() ) );
						// compute the interpolation of the transformed bounding boxes
						boxInterpolator( previousTransformedBox, nextTransformedBox, x, interpTransformedBoxes );
						// compute how much the transformed boxes (on both samples) have to expand to fully include transfomedInterpBox
						// when they are interpolated at the same ratio.
						Imath::Box3d extendedBBox = interpTransformedBoxes;
						extendedBBox.extendBy( transfomedInterpBox );
						Imath::Box3d expansion( extendedBBox.min - interpTransformedBoxes.min, extendedBBox.max - interpTransformedBoxes.max );
						totalExpansion.extendBy( expansion );
					}
					// we know how much the interpolated box should expand at any step. If we apply that to both samples, we guarantee any linear interpolation
					// is at least that amount bigger.
					previousTransformedBox = Imath::Box3d( previousTransformedBox.min + totalExpansion.min, previousTransformedBox.max + totalExpansion.max );
					nextTransformedBox = Imath::Box3d( nextTransformedBox.min + totalExpansion.min, nextTransformedBox.max + totalExpansion.max );
					// revisit last transformed bounding box
					*(bsIt-1) = previousTransformedBox;

					previousTransformedBox = nextTransformedBox;
				}
				*bsIt = nextTransformedBox;
				previousBox = nextBox;
				previousTransformedBox = nextTransformedBox;
				previousTransform = nextTransform;
				previousTransformTime = boxTime;
			}

			/// now transform all the following bound samples using the last transform available...
			previousTransformMatrix = dataToMatrix( previousTransform.get() );
			while( btIt != boxTimes.end() )
			{
				*bsIt = transform( *bsIt, previousTransformMatrix );
				btIt++;
				bsIt++;
			}
		}

		WriterImplementation* m_parent;
		std::map< SceneCache::Name, WriterImplementationPtr > m_children;

		typedef std::map< SampleTimes, uint64_t > SampleTimesMap;
		typedef std::map< SceneCache::Name, SampleTimes > AttributeSamplesMap;

		SampleTimesMap *m_sampleTimesMap;
		SampleTimes m_boundSampleTimes;		// implicit or explicit bound sample times
		SampleTimes m_transformSampleTimes;
		AttributeSamplesMap m_attributeSampleTimes;
		SampleTimes m_objectSampleTimes;
		// store the transform objects (we want to interpolate the transforms later)
		TransformSamples m_transformSamples;
		// store the object's bounding box (we want to transform them later)
		BoxSamples m_objectSamples;
		// overwriting bounding boxes (or used during flush to compute the final bounding boxes).
		BoxSamples m_boundSamples;

		typedef std::pair< MurmurHash, bool> AnimatedHashTest;
		typedef std::map< SceneCache::Name, AnimatedHashTest > AnimatedPrimVarMap;

		AnimatedHashTest m_animatedObjectTopology;
		AnimatedPrimVarMap m_animatedObjectPrimVars;
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

SceneCache::SceneCache( ImplementationPtr& impl )
	:	m_implementation( impl )
{
}

SceneCache::~SceneCache()
{
}

std::string SceneCache::fileName() const
{
	return m_implementation->fileName();
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

SceneCache::Name SceneCache::name() const
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

ConstDataPtr SceneCache::readTransformAtSample( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readTransformAtSample( sampleIndex );
}

Imath::M44d SceneCache::readTransformAsMatrixAtSample( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readTransformAsMatrixAtSample( sampleIndex );
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

void SceneCache::attributeNames( NameList &attrs ) const
{
	m_implementation->attributeNames(attrs);
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

ConstObjectPtr SceneCache::readAttributeAtSample( const Name &name, size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readAttributeAtSample( name, sampleIndex );
}

void SceneCache::writeAttribute( const Name &name, const Object *attribute, double time )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );

	if ( name == animatedObjectTopologyAttribute || name == animatedObjectPrimVarsAttribute )
	{
		// ignore reserved attribute names
		return;
	}

	writer->writeAttribute( name, attribute, time );
}

bool SceneCache::hasTag( const Name &name, int filter ) const
{
	return m_implementation->hasTag(name, filter);
}

void SceneCache::readTags( NameList &tags, int filter ) const
{
	if ( filter && filter != SceneInterface::LocalTag )
	{
		/// non Local tags is only supported in read mode.
		ReaderImplementation::reader( m_implementation.get() );
	}
	return m_implementation->readTags(tags, filter);
}

void SceneCache::writeTags( const NameList &tags )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	writer->writeTags( tags );
}

void SceneCache::writeTags( const NameList &tags, bool descendentTags )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	writer->writeTags( tags, descendentTags ? SceneInterface::DescendantTag : SceneInterface::LocalTag );
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

ConstObjectPtr SceneCache::readObjectAtSample( size_t sampleIndex ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readObjectAtSample( sampleIndex );
}

PrimitiveVariableMap SceneCache::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	return reader->readObjectPrimitiveVariables( primVarNames, time );
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

	return duplicate( impl );
}

ConstSceneInterfacePtr SceneCache::child( const Name &name, SceneCache::MissingBehaviour missingBehaviour ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	ImplementationPtr impl = reader->child( name, missingBehaviour );
	if ( !impl )
	{
        	return 0;
	}

	return duplicate( impl );
}

bool SceneCache::hasChild( const Name &name ) const
{
	return m_implementation->hasChild(name);
}

SceneInterfacePtr SceneCache::createChild( const Name &name )
{
	WriterImplementation *writer = WriterImplementation::writer( m_implementation.get() );
	ImplementationPtr impl = writer->createChild( name );
	return duplicate( impl );
}

SceneInterfacePtr SceneCache::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	ImplementationPtr impl = reader->scene( path, missingBehaviour );
	if ( !impl )
	{
        	return 0;
	}

	return duplicate( impl );
}

ConstSceneInterfacePtr SceneCache::scene( const Path &path, SceneCache::MissingBehaviour missingBehaviour ) const
{
	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	ImplementationPtr impl = reader->scene( path, missingBehaviour );
	if ( !impl )
	{
        	return 0;
	}

	return duplicate( impl );
}

void SceneCache::hash( HashType hashType, double time, MurmurHash &h ) const
{
	SceneInterface::hash( hashType, time, h );

	ReaderImplementation *reader = ReaderImplementation::reader( m_implementation.get() );
	reader->hash( hashType, time, h );
}

SceneCachePtr SceneCache::duplicate( ImplementationPtr& impl ) const
{
	return new SceneCache( impl );
}

bool SceneCache::readOnly() const
{
	return dynamic_cast< const ReaderImplementation* >( m_implementation.get() ) != NULL;
}
