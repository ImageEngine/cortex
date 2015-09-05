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

#include "boost/filesystem.hpp"
#include "boost/tokenizer.hpp"
#include "boost/format.hpp"

#include "OpenEXR/ImathBoxAlgo.h"

#include "Alembic/AbcCoreHDF5/ReadWrite.h"
#include "Alembic/Abc/IArchive.h"
#include "Alembic/Abc/IObject.h"
#include "Alembic/AbcGeom/IGeomBase.h"
#include "Alembic/AbcGeom/ArchiveBounds.h"
#include "Alembic/AbcGeom/IXform.h"
#include "Alembic/AbcGeom/ICamera.h"

#include "Alembic/Abc/OArchive.h"
#include "Alembic/AbcGeom/OXform.h"

#ifdef IECOREALEMBIC_WITH_OGAWA
#include "Alembic/AbcCoreFactory/IFactory.h"
#include "Alembic/AbcCoreOgawa/ReadWrite.h"
#endif

#include "IECoreAlembic/AlembicScene.h"
#include "IECoreAlembic/FromAlembicConverter.h"
#include "IECoreAlembic/ToAlembicConverter.h"

#include "IECore/NullObject.h"
#include "IECore/ComputationCache.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/ObjectInterpolator.h"

using namespace IECore;
using namespace IECoreAlembic;
using namespace Imath;
using namespace boost;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( AlembicScene )

//////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////

// Register the .scc extension in the factory function for AlembicScene on Read and Write modes
static SceneInterface::FileFormatDescription<AlembicScene> registrar(".abc", IndexedIO::Read | IndexedIO::Write );


class AlembicScene::AlembicIO : public IECore::RefCounted
{
	public:

		virtual ~AlembicIO() {}

		virtual void childNames( NameList &childNames ) const = 0;
		virtual Name name() const = 0;
		virtual void path( Path &p ) const = 0;
		virtual AlembicIOPtr child( const Name &name ) = 0;
		virtual ConstAlembicIOPtr child( const Name &name ) const = 0;
};

class AlembicScene::AlembicReader : public AlembicScene::AlembicIO
{

	public:
		
		AlembicReader( const std::string &fileName )
		{
			m_data = boost::shared_ptr<DataMembers>( new DataMembers );
			m_data->boundCache = new BoundCache( doReadBoundAtTime, boundHash,  10000 );

			// build a hash of the file and its last write time:
			m_data->fileHash.append( fileName );
			boost::filesystem::path p( fileName );
			m_data->fileHash.append( boost::filesystem::last_write_time( p ) );

		#ifdef IECOREALEMBIC_WITH_OGAWA
			Alembic::AbcCoreFactory::IFactory factory;
			m_data->archive = boost::shared_ptr<IArchive>( new IArchive( factory.getArchive( fileName ) ) );
			if( !m_data->archive->valid() )
			{
				// even though the default policy for IFactory is kThrowPolicy, this appears not to
				// be applied when it fails to load an archive - instead it returns an invalid archive.
				throw IECore::Exception( boost::str( boost::format( "Unable to open file \"%s\"" ) % fileName ) );
			}
		#else
			m_data->archive = boost::shared_ptr<IArchive>( new IArchive( ::Alembic::AbcCoreHDF5::ReadArchive(), fileName ) );
		#endif

			m_data->object = m_data->archive->getTop();
		}

		virtual ~AlembicReader() {}

		virtual void childNames( NameList &childNames ) const
		{
			size_t numChildren = this->numChildren();
			for( size_t i=0; i<numChildren; i++ )
			{
				childNames.push_back( m_data->object.getChildHeader( i ).getName() );
			}
		}

		virtual Name name() const
		{
			return m_data->object.getName();
		}

		virtual void path( Path &p ) const
		{
			p.clear();
			std::string path = m_data->object.getFullName();
			boost::tokenizer<boost::char_separator<char> > t( path, boost::char_separator<char>( "/" ) );
			for (
				boost::tokenizer<boost::char_separator<char> >::iterator it = t.begin();
				it != t.end();
				++it
			)
			{
				p.push_back( *it );
			}
		}

		virtual AlembicIOPtr child( const Name &name )
		{
			IObject c = m_data->object.getChild( name.string() );
			if( !c )
			{
				throw InvalidArgumentException( name.string() );
			}

			AlembicReaderPtr result = new AlembicReader();
			result->m_data = boost::shared_ptr<DataMembers>( new DataMembers );
			result->m_data->archive = m_data->archive;
			result->m_data->boundCache = m_data->boundCache;
			result->m_data->fileHash = m_data->fileHash;
			result->m_data->object = c;
			return result;
		}

		virtual ConstAlembicIOPtr child( const Name &name ) const
		{
			return const_cast<AlembicReader*>( this )->child( name );
		}

		static const AlembicReader *reader( const AlembicIO *io, bool throwException = true )
		{
			const AlembicReader *reader = dynamic_cast< const AlembicReader* >( io );
			if ( throwException && !reader )
			{
				throw IECore::Exception( "Function not supported when writing a scene file." );
			}
			return reader;
		}

		/// Returns the number of samples.
		size_t numSamples() const
		{
			if( m_data->numSamples != -1 )
			{
				return m_data->numSamples;
			}

			// wouldn't it be grand if the different things we had to call getNumSamples()
			// on had some sort of base class where getNumSamples() was defined?
			/// \todo See todo in ensureTimeSampling().

			const MetaData &md = m_data->object.getMetaData();

			if( !m_data->object.getParent() )
			{
				// top of archive
				if( m_data->object.getProperties().getPropertyHeader( ".childBnds" ) )
				{
					Alembic::Abc::IBox3dProperty boundsProperty( m_data->object.getProperties(), ".childBnds" );
					m_data->numSamples = boundsProperty.getNumSamples();
				}
				else
				{
					m_data->numSamples = 0;
				}
			}
			else if( IXform::matches( md ) )
			{
				IXform iXForm( m_data->object, kWrapExisting );
				m_data->numSamples = iXForm.getSchema().getNumSamples();
			}
			else if( ICamera::matches( md ) )
			{
				ICamera iCamera( m_data->object, kWrapExisting );
				m_data->numSamples = iCamera.getSchema().getNumSamples();	
			}
			else
			{
				IGeomBaseObject geomBase( m_data->object, kWrapExisting );
				m_data->numSamples = geomBase.getSchema().getNumSamples();
			}

			return m_data->numSamples;
		}

		/// Returns the time associated with the specified sample.
		double timeAtSample( size_t sampleIndex ) const
		{
			if( sampleIndex >= numSamples() )
			{
				throw InvalidArgumentException( "Sample index out of range" );
			}
			ensureTimeSampling();
			return m_data->timeSampling->getSampleTime( sampleIndex );
		}

		/// Computes a sample interval suitable for use in producing interpolated
		/// values, returning the appropriate lerp factor between the two samples.
		/// In the case of time falling outside of the sample range, or coinciding
		/// nearly exactly with a single sample, 0 is returned and floorIndex==ceilIndex
		/// will hold.
		double sampleIntervalAtTime( double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			ensureTimeSampling();	

			std::pair<Alembic::AbcCoreAbstract::index_t, chrono_t> f = m_data->timeSampling->getFloorIndex( time, numSamples() );
			if( fabs( time - f.second ) < 0.0001 )
			{
				// it's going to be very common to be reading on the whole frame, so we want to make sure
				// that anything thereabouts is loaded as a single uninterpolated sample for speed.
				floorIndex = ceilIndex = f.first;
				return 0.0;
			}

			std::pair<Alembic::AbcCoreAbstract::index_t, chrono_t> c = m_data->timeSampling->getCeilIndex( time, numSamples() );
			if( f.first == c.first || fabs( time - c.second ) < 0.0001 )
			{
				// return a result not needing interpolation if possible. either we only had one sample
				// to pick from or the ceiling sample was close enough to perfect.
				floorIndex = ceilIndex = c.first;
				return 0.0;
			}

			floorIndex = f.first;
			ceilIndex = c.first;

			return ( time - f.second ) / ( c.second - f.second );
		}

		/// Alembic archives don't necessarily store bounding box
		/// information for every object in the scene graph. This method
		/// can be used to determine whether or not a bound has been
		/// stored for this object. You can NOT EXACTLY rely on having
		/// stored bounds at the top of the archive and at any geometry-containing
		/// nodes.
		bool hasStoredBound() const
		{
			const MetaData &md = m_data->object.getMetaData();
			if( !m_data->object.getParent() )
			{
				return m_data->object.getProperties().getPropertyHeader( ".childBnds" ) != 0x0;
			}
			else if( IXform::matches( md ) )
			{
				IXform iXForm( m_data->object, kWrapExisting );
				IXformSchema &iXFormSchema = iXForm.getSchema();
				return iXFormSchema.getChildBoundsProperty();
			}
			else if( IGeomBase::matches( md ) )
			{
				return true;
			}
			return false;
		}

		/// Returns the local bounding box of this node stored for the specified
		/// sample. If hasStoredBound() is false then throws an Exception.
		Imath::Box3d boundAtSample( size_t sampleIndex ) const
		{
			const MetaData &md = m_data->object.getMetaData();

			if( !m_data->object.getParent() )
			{
				// top of archive
				return GetIArchiveBounds( *(m_data->archive) ).getValue( ISampleSelector( (index_t)sampleIndex ) );
			}
			else if( IXform::matches( md ) )
			{
				IXform iXForm( m_data->object, kWrapExisting );
				IXformSchema &iXFormSchema = iXForm.getSchema();

				if( !iXFormSchema.getChildBoundsProperty() )
				{
					throw IECore::Exception( "No stored bounds available" );
				}

				return iXFormSchema.getChildBoundsProperty().getValue( ISampleSelector( (index_t)sampleIndex ) );
			}
			else
			{
				IGeomBaseObject geomBase( m_data->object, kWrapExisting );
				return geomBase.getSchema().getValue( ISampleSelector( (index_t)sampleIndex ) ).getSelfBounds();
			}

			return Imath::Box3d();
		}

		/// Returns the interpolated local bounding box of this node at the
		/// specified point in time. If hasStoredBound() is false, then
		/// the archive is traversed and a bound computed recursively from
		/// all descendants of this node. Beware! This can be slow.
		Imath::Box3d boundAtTime( double time ) const
		{
			BoundCacheKey k( this, time );
			ConstObjectPtr cachedBound = m_data->boundCache->get( k );
			return runTimeCast<const Box3dData>( cachedBound.get() )->readable();
		}

		/// Returns the transformation matrix of this node if it has one,
		/// and the identity otherwise.
		Imath::M44d transformAtSample( size_t sampleIndex = 0 ) const
		{
			M44d result;
			if( IXform::matches( m_data->object.getMetaData() ) )
			{
				IXform iXForm( m_data->object, kWrapExisting );
				IXformSchema &iXFormSchema = iXForm.getSchema();
				XformSample sample;
				iXFormSchema.get( sample, ISampleSelector( (index_t)sampleIndex ) );
				return sample.getMatrix();
			}
			return result;
		}

		/// As above, but interpolating between samples where necessary.
		Imath::M44d transformAtTime( double time ) const
		{
			M44d result;

			if( IXform::matches( m_data->object.getMetaData() ) )
			{
				size_t index0, index1;
				double lerpFactor = sampleIntervalAtTime( time, index0, index1 );

				IXform iXForm( m_data->object, kWrapExisting );
				IXformSchema &iXFormSchema = iXForm.getSchema();

				if( index0 == index1 )
				{
					XformSample sample;
					iXFormSchema.get( sample, ISampleSelector( (index_t)index0 ) );
					result = sample.getMatrix();
				}
				else
				{
					XformSample sample0;
					iXFormSchema.get( sample0, ISampleSelector( (index_t)index0 ) );
					XformSample sample1;
					iXFormSchema.get( sample1, ISampleSelector( (index_t)index1 ) );

					if( sample0.getNumOps() != sample1.getNumOps() ||
						sample0.getNumOpChannels() != sample1.getNumOpChannels() 
					)
					{
						throw IECore::Exception( "Unable to interpolate samples of different sizes" );
					}

					XformSample interpolatedSample;
					for( size_t opIndex = 0; opIndex < sample0.getNumOps(); opIndex++ )
					{
						XformOp op0 = sample0.getOp( opIndex );
						XformOp op1 = sample1.getOp( opIndex );			
						XformOp interpolatedOp( op0.getType(), op0.getHint() );
						for( size_t channelIndex = 0; channelIndex < op0.getNumChannels(); channelIndex++ )
						{
							interpolatedOp.setChannelValue(
								channelIndex,
								lerp( op0.getChannelValue( channelIndex ), op1.getChannelValue( channelIndex ), lerpFactor )
							);
						}

						interpolatedSample.addOp( interpolatedOp );
					}

					result = interpolatedSample.getMatrix();
				}
			}

			return result;
		}

		/// Returns a converter capable of converting the Alembic object into
		/// the specified form, or 0 if no such converter exists. The converter
		/// is returned as a ToCoreConverter rather than a FromAlembicConverter
		/// as the latter exposes the underlying Alembic APIs, which we are
		/// deliberately hiding with the AlembicReader class.
		IECore::ToCoreConverterPtr converter( IECore::TypeId resultType = IECore::ObjectTypeId ) const
		{
			return FromAlembicConverter::create( m_data->object, resultType );
		}

		/// Converts the alembic object into Cortex form, preferring conversions
		/// yielding the specified result type.
		IECore::ObjectPtr objectAtSample( size_t sampleIndex = 0, IECore::TypeId resultType = IECore::ObjectTypeId ) const
		{
			FromAlembicConverterPtr c = FromAlembicConverter::create( m_data->object, resultType );
			if( c )
			{
				c->sampleIndexParameter()->setNumericValue( sampleIndex );
				return c->convert();
			}
			return 0;
		}

		/// As above, but performing linear interpolation between samples where necessary.
		IECore::ObjectPtr objectAtTime( double time, IECore::TypeId resultType = IECore::ObjectTypeId ) const
		{
			FromAlembicConverterPtr c = FromAlembicConverter::create( m_data->object, resultType );
			if( !c )
			{
				return 0;
			}

			size_t index0, index1;
			double lerpFactor = sampleIntervalAtTime( time, index0, index1 );
			if( index0==index1 )
			{
				c->sampleIndexParameter()->setNumericValue( index0 );
				return c->convert();
			}
			else
			{
				c->sampleIndexParameter()->setNumericValue( index0 );
				ObjectPtr object0 = c->convert();
				c->sampleIndexParameter()->setNumericValue( index1 );
				ObjectPtr object1 = c->convert();
				return linearObjectInterpolation( object0.get(), object1.get(), lerpFactor );
			}
		}

		/// Appends the object hash to h at the specified time, if the info exists
		/// Returns false if it doesn't:
		bool objectHash( double time, IECore::MurmurHash &h ) const
		{
			Digest alembicHash;
			if( m_data->object.getPropertiesHash( alembicHash ) )
			{
				h.append( alembicHash.words[0] );
				h.append( alembicHash.words[1] );
				if( numSamples() )
				{
					h.append( time );
				}
				return true;
			}
			return false;
		}

		size_t numChildren() const
		{
			if(
				!IXform::matches( m_data->object.getMetaData() ) &&
				m_data->object.getParent()
			)
			{
				// not a transform, and not the top of the archive.
				// we want to ignore any children, because they won't
				// be something we consider part of the hierarchy -
				// alembic implements face sets as objects parented to
				// a mesh for instance, whereas we would just think
				// of them as a property of the mesh.
				return 0;
			}
			return m_data->object.getNumChildren();
		}

	private:

		AlembicReader() {}
	
		void ensureTimeSampling() const		
		{
			if( m_data->timeSampling )
			{
				return;
			}
			const MetaData &md = m_data->object.getMetaData();

			/// \todo It's getting a bit daft having to cover all the
			/// types in here. We either need to find a generic way of
			/// doing it (seems like that might not be Alembic's style though)
			/// or perhaps we should have a timeSampling() method on the
			/// converters?
			if( !m_data->object.getParent() )
			{
				// top of archive
				Alembic::Abc::IBox3dProperty boundsProperty( m_data->object.getProperties(), ".childBnds" );
				m_data->timeSampling = boundsProperty.getTimeSampling();
			}
			else if( IXform::matches( md ) )
			{
				IXform iXForm( m_data->object, kWrapExisting );
				m_data->timeSampling = iXForm.getSchema().getTimeSampling();
			}
			else if( ICamera::matches( md ) )
			{
				ICamera iCamera( m_data->object, kWrapExisting );
				m_data->timeSampling = iCamera.getSchema().getTimeSampling();	
			}
			else
			{
				IGeomBaseObject geomBase( m_data->object, kWrapExisting );
				m_data->timeSampling = geomBase.getSchema().getTimeSampling();
			}
		}

		// If no explicit bounding box has been written at a location, the bounding box
		// is calculated from scratch.. This can lead to a full hierarchy traversal,
		// which can be unacceptably slow in some cases, eg if an alembic file is
		// loaded into an ieSceneShape in maya. These members are part of a caching
		// system to mitigate this.

		typedef std::pair< const AlembicReader *, double > BoundCacheKey;
		typedef IECore::ComputationCache< BoundCacheKey > BoundCache;

		AlembicReaderPtr child( size_t index ) const
		{
			AlembicReaderPtr result = new AlembicReader();
			result->m_data = boost::shared_ptr<DataMembers>( new DataMembers );
			result->m_data->archive = this->m_data->archive;
			result->m_data->boundCache = this->m_data->boundCache;
			result->m_data->fileHash = this->m_data->fileHash;
			/// \todo this is documented as not being the best way of doing things in
			/// the alembic documentation. I'm not sure what would be better though,
			/// and it appears to work fine so far.
			result->m_data->object = this->m_data->object.getChild( index );
			return result;
		}

		static IECore::MurmurHash boundHash( const BoundCacheKey &key )
		{
			const AlembicReader* input = key.first;
			double time = key.second;

			// hash in scene location, sample index and the file hash,
			// which is built using the file name and the time stamp:
			IECore::MurmurHash h;
			h.append( input->m_data->object.getFullName() );
			h.append( time );
			h.append( input->m_data->fileHash );

			return h;
		}

		static IECore::ObjectPtr doReadBoundAtTime( const BoundCacheKey &key )
		{
			const AlembicReader* input = key.first;
			double time = key.second;

			if( input->hasStoredBound() )
			{
				size_t index0, index1;
				double lerpFactor = input->sampleIntervalAtTime( time, index0, index1 );
				if( index0 == index1 )
				{
					return new IECore::Box3dData( input->boundAtSample( index0 ) );
				}
				else
				{
					Box3d bound0 = input->boundAtSample( index0 );
					Box3d bound1 = input->boundAtSample( index1 );		
					Box3d result;
					result.min = lerp( bound0.min, bound1.min, lerpFactor );	
					result.max = lerp( bound0.max, bound1.max, lerpFactor );
					return new IECore::Box3dData( result );
				}
			}
			else
			{
				Box3d result;
				for( size_t i=0, n=input->numChildren(); i<n; i++ )
				{
					ConstAlembicReaderPtr c = reader( input->child( i ).get() );
					Box3d childBound = c->boundAtTime( time );
					childBound = Imath::transform( childBound, c->transformAtTime( time ) );
					result.extendBy( childBound );
				}
				return new IECore::Box3dData( result );
			}
		}

		struct DataMembers
		{

			DataMembers()
				: numSamples( -1 )
			{
			}

			IECore::MurmurHash fileHash;
			boost::shared_ptr<IArchive> archive;	
			IObject object;
			BoundCache::Ptr boundCache;
			int numSamples;
			TimeSamplingPtr timeSampling;
		};

		boost::shared_ptr<DataMembers> m_data;

};


class AlembicScene::AlembicWriter : public AlembicScene::AlembicIO
{

	public:

		
		AlembicWriter( const std::string &fileName )
		{
			m_data = boost::shared_ptr<DataMembers>( new DataMembers );

		#ifdef IECOREALEMBIC_WITH_OGAWA
			m_data->archive = boost::shared_ptr<OArchive>( new OArchive( ::Alembic::AbcCoreOgawa::WriteArchive(), fileName ) );
		#else
			m_data->archive = boost::shared_ptr<OArchive>( new OArchive( ::Alembic::AbcCoreHDF5::WriteArchive(), fileName ) );
		#endif

			m_data->object = m_data->archive->getTop();
		}

		virtual ~AlembicWriter() {}

		virtual Name name() const
		{
			return m_data->object.getName();
		}

		virtual void path( Path &p ) const
		{
			p.clear();
			std::string path = m_data->object.getFullName();
			boost::tokenizer<boost::char_separator<char> > t( path, boost::char_separator<char>( "/" ) );
			for (
				boost::tokenizer<boost::char_separator<char> >::iterator it = t.begin();
				it != t.end();
				++it
			)
			{
				p.push_back( *it );
			}
		}

		virtual AlembicIOPtr child( const Name &name )
		{
			DataMembers::ChildMap::iterator it = m_data->children.find( name );
			if( it != m_data->children.end() )
			{
				AlembicWriterPtr result = new AlembicWriter();
				result->m_data = it->second;
				return result;
			}
			return 0;
		}

		virtual ConstAlembicIOPtr child( const Name &name ) const
		{
			return const_cast<AlembicWriter*>( this )->child( name );
		}

		virtual void childNames( NameList &names ) const
		{
			names.clear();
			for( DataMembers::ChildMap::const_iterator it = m_data->children.begin(); it != m_data->children.end(); ++it )
			{
				names.push_back( it->first );
			}
		}

		AlembicIOPtr createChild( const Name &name )
		{
			AlembicWriterPtr result = new AlembicWriter();
			result->m_data = boost::shared_ptr<DataMembers>( new DataMembers );
			result->m_data->archive = m_data->archive;
			result->m_data->xform = OXform( m_data->object, name );
			result->m_data->object = result->m_data->xform;

			m_data->children[name] = result->m_data;

			return result;
		}

		void writeBoundAtTime( const Imath::Box3d& bound, double time )
		{
		}

		void writeTransformAtTime( const Imath::M44d& matrix, double time )
		{
			if( !OXform::matches( m_data->object.getMetaData() ) )
			{
				throw IECore::Exception( "AlembicOutput::writeTransformAtTime: can't write transforms at the root" );
			}

			if( m_data->sampleTimes.size() && time <= m_data->sampleTimes.back() )
			{
				throw IECore::Exception( "AlembicOutput::writeTransformAtTime: transform sample times must be strictly increasing" );
			}	

			OXformSchema &oXFormSchema = m_data->xform.getSchema();

			m_data->sampleTimes.push_back( time );
			TimeSamplingPtr tSamp( new TimeSampling( TimeSamplingType( TimeSamplingType::kAcyclic ), m_data->sampleTimes ) );
			oXFormSchema.setTimeSampling( tSamp );

			XformSample sample;
			sample.setMatrix( matrix );
			oXFormSchema.set( sample );
		}

		void writeObjectAtTime( const IECore::Object *obj, double time )
		{
			if( !OXform::matches( m_data->object.getMetaData() ) )
			{
				throw IECore::Exception( "AlembicOutput::writeObjectAtTime: can't write objects at the root" );
			}

			if( !obj )
			{
				throw IECore::Exception( "AlembicOutput::writeObjectAtTime: tried to write null object pointer" );
			}

			// create converter at this transform, if none exists:
			if( !m_data->converter )
			{
				m_data->converter = ToAlembicConverter::create( obj->typeId(), m_data->object );
				if( !m_data->converter )
				{
					throw IECore::Exception( "AlembicOutput::writeObjectAtTime: No ToAlembicConverter for objects of type " + std::string( obj->typeName() ) );
				}
			}

			if( m_data->converter->supportedType() != obj->typeId() )
			{
				throw IECore::Exception( "AlembicOutput::writeObjectAtTime: Object type must be " + std::string( m_data->converter->srcParameter()->getValue()->typeName() ) );
			}

			m_data->converter->srcParameter()->setValue( const_cast<IECore::Object*>( obj ) );
			m_data->converter->timeParameter()->setNumericValue( time );
			m_data->converter->convert();
		}

		static AlembicWriter *writer( AlembicIO *io, bool throwException = true )
		{
			AlembicWriter *writer = dynamic_cast< AlembicWriter* >( io );
			if ( throwException && !writer )
			{
				throw IECore::Exception( "Function not supported when writing a scene file." );
			}
			return writer;
		}

	private:

		AlembicWriter() {}

		struct DataMembers
		{
			boost::shared_ptr<OArchive> archive;
			OObject object;

			// For some reason I can't cast/reinterpret "object" to an OXform, so I'm storing this:
			OXform xform;

			// previously written sample times:
			std::vector<chrono_t> sampleTimes;

			// we can't rely on OObject.getChild, because Alembic objects only contain
			// weak pointers to their children, meaning as soon as a child goes out of
			// scope we won't be able to access it again. Because of this, we need to
			// explicitly store the scene structure:
			typedef std::map<std::string, boost::shared_ptr<DataMembers> > ChildMap;
			ChildMap children;

			// converter for writing objects. ToAlembicConverters hold their own alembic
			// objects and need to persist, as we can't resurrect OObjects once we let
			// go of them.
			ToAlembicConverterPtr converter;
		};

		boost::shared_ptr<DataMembers> m_data;
};





//////////////////////////////////////////////////////////////////////////
// AlembicScene
//////////////////////////////////////////////////////////////////////////

AlembicScene::AlembicScene( const std::string &fileName, IndexedIO::OpenMode mode )
{
	if( mode == IndexedIO::Read )
	{
		m_io = new AlembicReader( fileName );
	}
	else if( mode == IndexedIO::Write )
	{
		m_io = new AlembicWriter( fileName );
	}
	else
	{
		throw IECore::Exception( "AlembicScene only supports Read and Write modes" );
	}

	m_fileNameHash.append( fileName );
	m_root = m_io;
}

AlembicScene::AlembicScene( AlembicIOPtr io, AlembicIOPtr root, MurmurHash fileNameHash ) : m_io( io ), m_root( root ), m_fileNameHash( fileNameHash )
{
}

AlembicScene::~AlembicScene()
{
}

std::string AlembicScene::fileName() const
{
	return "";
}

void AlembicScene::path( AlembicScene::Path &p ) const
{
	m_io->path( p );

}

AlembicScene::Name AlembicScene::name() const
{
	return m_io->name();
}

size_t AlembicScene::numBoundSamples() const
{
	return AlembicReader::reader( m_io.get() )->numSamples();
}

double AlembicScene::boundSampleTime( size_t sampleIndex ) const
{
	return AlembicReader::reader( m_io.get() )->timeAtSample( sampleIndex );
}

double AlembicScene::boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return AlembicReader::reader( m_io.get() )->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

Imath::Box3d AlembicScene::readBoundAtSample( size_t sampleIndex ) const
{
	return AlembicReader::reader( m_io.get() )->boundAtSample( sampleIndex );
}

Imath::Box3d AlembicScene::readBound( double time ) const
{
	return AlembicReader::reader( m_io.get() )->boundAtTime( time );
}

void AlembicScene::writeBound( const Imath::Box3d &bound, double time )
{
	AlembicWriter::writer( m_io.get() )->writeBoundAtTime( bound, time );
}

size_t AlembicScene::numTransformSamples() const
{
	return AlembicReader::reader( m_io.get() )->numSamples();
}

double AlembicScene::transformSampleTime( size_t sampleIndex ) const
{
	return AlembicReader::reader( m_io.get() )->timeAtSample( sampleIndex );
}

double AlembicScene::transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return AlembicReader::reader( m_io.get() )->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

ConstDataPtr AlembicScene::readTransformAtSample( size_t sampleIndex ) const
{
	return new IECore::M44dData( readTransformAsMatrixAtSample( sampleIndex ) );
}

Imath::M44d AlembicScene::readTransformAsMatrixAtSample( size_t sampleIndex ) const
{
	return AlembicReader::reader( m_io.get() )->transformAtSample( sampleIndex );
}

ConstDataPtr AlembicScene::readTransform( double time ) const
{
	return new IECore::M44dData( readTransformAsMatrix( time ) );
}

Imath::M44d AlembicScene::readTransformAsMatrix( double time ) const
{
	return AlembicReader::reader( m_io.get() )->transformAtTime( time );
}

void AlembicScene::writeTransform( const Data *transform, double time )
{
	const M44dData *matrixData = runTimeCast<const M44dData>( transform );
	if( !matrixData )
	{
		throw IECore::Exception( "AlembicScene::writeTransform(): expects M44dData" );
	}

	AlembicWriter::writer( m_io.get() )->writeTransformAtTime( matrixData->readable(), time );
}

bool AlembicScene::hasAttribute( const Name &name ) const
{
	return false;
}

void AlembicScene::attributeNames( NameList &attrs ) const
{
	attrs.clear();
}

size_t AlembicScene::numAttributeSamples( const Name &name ) const
{
	return AlembicReader::reader( m_io.get() )->numSamples();
}

double AlembicScene::attributeSampleTime( const Name &name, size_t sampleIndex ) const
{
	return AlembicReader::reader( m_io.get() )->timeAtSample( sampleIndex );
}

double AlembicScene::attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return AlembicReader::reader( m_io.get() )->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

ConstObjectPtr AlembicScene::readAttributeAtSample( const Name &name, size_t sampleIndex ) const
{
	return IECore::NullObject::defaultNullObject();
}

ConstObjectPtr AlembicScene::readAttribute( const Name &name, double time ) const
{
	return IECore::NullObject::defaultNullObject();
}

void AlembicScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw IECore::Exception( "AlembicScene::writeAttribute not supported" );
}

bool AlembicScene::hasTag( const Name &name, int filter ) const
{
	return false;
}

void AlembicScene::readTags( NameList &tags, int filter ) const
{
	tags.clear();
}

void AlembicScene::writeTags( const NameList &tags )
{
	throw IECore::Exception( "AlembicScene::writeTags not supported" );
}

bool AlembicScene::hasObject() const
{
	return AlembicReader::reader( m_io.get() )->converter( IECore::RenderableTypeId );
}

size_t AlembicScene::numObjectSamples() const
{
	return AlembicReader::reader( m_io.get() )->numSamples();
}

double AlembicScene::objectSampleTime( size_t sampleIndex ) const
{
	return AlembicReader::reader( m_io.get() )->timeAtSample( sampleIndex );
}

double AlembicScene::objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return AlembicReader::reader( m_io.get() )->sampleIntervalAtTime( time, floorIndex, ceilIndex );
}

ConstObjectPtr AlembicScene::readObjectAtSample( size_t sampleIndex ) const
{
	ConstObjectPtr o = AlembicReader::reader( m_io.get() )->objectAtSample( sampleIndex, IECore::RenderableTypeId );
	if( o )
	{
		return o;
	}
	return IECore::NullObject::defaultNullObject();
}

ConstObjectPtr AlembicScene::readObject( double time ) const
{
	ConstObjectPtr o = AlembicReader::reader( m_io.get() )->objectAtTime( time, IECore::RenderableTypeId );
	if( o )
	{
		return o;
	}
	return IECore::NullObject::defaultNullObject();
}

PrimitiveVariableMap AlembicScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	return PrimitiveVariableMap();
}

void AlembicScene::writeObject( const Object *object, double time )
{
	AlembicWriter::writer( m_io.get() )->writeObjectAtTime( object, time );
}

void AlembicScene::childNames( NameList &childNames ) const
{
	m_io->childNames( childNames );
}

SceneInterfacePtr AlembicScene::child( const Name &name, AlembicScene::MissingBehaviour missingBehaviour )
{
	AlembicIOPtr c = m_io->child( name );
	if( c )
	{
		return new AlembicScene( c, m_root, m_fileNameHash );
	}
	
	if( missingBehaviour == NullIfMissing )
	{
		return 0;
	}
	else if( missingBehaviour == CreateIfMissing )
	{
		return createChild( name );
	}

	Path p;
	path( p );
	std::string pathStr;
	pathToString( p, pathStr );
	throw IECore::Exception( ( boost::format( "AlembicScene::child: no child called %s at %s" ) % name.value() % pathStr ).str() );
}

ConstSceneInterfacePtr AlembicScene::child( const Name &name, AlembicScene::MissingBehaviour missingBehaviour ) const
{
	return const_cast<AlembicScene*>( this )->child( name, missingBehaviour );
}

bool AlembicScene::hasChild( const Name &name ) const
{
	NameList names;
	childNames( names );
	return std::find( names.begin(), names.end(), name ) != names.end();
}

SceneInterfacePtr AlembicScene::createChild( const Name &name )
{
	return new AlembicScene( AlembicWriter::writer( m_io.get() )->createChild( name ), m_root, m_fileNameHash );
}

SceneInterfacePtr AlembicScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	AlembicIOPtr s = m_root;
	for( size_t i=0; i < path.size(); ++i )
	{
		s = s->child( path[i] );
		if( !s )
		{
			if( missingBehaviour == NullIfMissing )
			{
				return 0;
			}
			std::string currentPathStr = "/";
			for( size_t j=0; j < i; ++j )
			{
				currentPathStr += path[j].value() + "/";
			}
			throw IECore::Exception( ( boost::format( "AlembicScene::scene: no child called %s at %s" ) % path[i].value() % currentPathStr ).str() );
		}
	}
	return new AlembicScene( s, m_root, m_fileNameHash );
}

ConstSceneInterfacePtr AlembicScene::scene( const Path &path, AlembicScene::MissingBehaviour missingBehaviour ) const
{
	return const_cast<AlembicScene*>( this )->scene( path, missingBehaviour );
}

void AlembicScene::hash( HashType hashType, double time, MurmurHash &h ) const
{
	const AlembicReader* input = AlembicReader::reader( m_io.get() );

	SceneInterface::hash( hashType, time, h );
	h.append( int(hashType) );
	switch( hashType )
	{
		case TransformHash:
		{
			h.append( readTransformAsMatrix( time ) );
			break;
		}
		case BoundHash:
		{
			if( input->hasStoredBound() )
			{
				// this read will be quick as the bound has been stored, so we can use it in the hash:
				h.append( readBound( time ) );
			}
			else
			{
				// fall back and just hash in a bunch of stuff:
				h.append( m_fileNameHash );
				Path p;
				input->path( p );
				h.append( p.data(), p.size() );
				if( numBoundSamples() > 1 )
				{
					h.append( time );
				}
			}
			break;
		}
		case ObjectHash:
		{
			// try and get the stored object hash from Alembic:
			if( !input->objectHash( time, h ) )
			{
				// fallback: 
				Path p;
				input->path( p );
				h.append( p.data(), p.size() );
				if( numObjectSamples() > 1 )
				{
					h.append( time );
				}
			}
			break;
		}
		case ChildNamesHash:
		{
			NameList names;
			input->childNames( names );
			h.append( names.data(), names.size() );
			break;
		}
		case HierarchyHash:
		{
			// \todo: use inbuilt hierarchy hashes
			h.append( m_fileNameHash );
			Path p;
			input->path( p );
			h.append( p.data(), p.size() );
			h.append( time );
			break;
		}
		case AttributesHash: break;
	}
}

