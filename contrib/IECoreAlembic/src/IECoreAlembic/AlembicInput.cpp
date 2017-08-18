//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "OpenEXR/ImathBoxAlgo.h"

#include "Alembic/AbcCoreHDF5/ReadWrite.h"
#include "Alembic/Abc/IArchive.h"
#include "Alembic/Abc/IObject.h"
#include "Alembic/AbcGeom/IGeomBase.h"
#include "Alembic/AbcGeom/ArchiveBounds.h"
#include "Alembic/AbcGeom/IXform.h"
#include "Alembic/AbcGeom/ICamera.h"

#ifdef IECOREALEMBIC_WITH_OGAWA
#include "Alembic/AbcCoreFactory/IFactory.h"
#endif

#include "IECoreAlembic/AlembicInput.h"
#include "IECoreAlembic/ObjectReader.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/ObjectInterpolator.h"

using namespace Imath;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace IECoreAlembic;
using namespace IECore;

struct AlembicInput::DataMembers
{
	DataMembers()
		: numSamples( -1 )
	{
	}

	boost::shared_ptr<IArchive> archive;
	IObject object;
	int numSamples;
	TimeSamplingPtr timeSampling;
};

AlembicInput::AlembicInput( const std::string &fileName )
{
	m_data = boost::shared_ptr<DataMembers>( new DataMembers );

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

AlembicInput::AlembicInput()
{
}

AlembicInput::~AlembicInput()
{
}

const std::string &AlembicInput::name() const
{
	return m_data->object.getName();
}

const std::string &AlembicInput::fullName() const
{
	return m_data->object.getFullName();
}

IECore::CompoundDataPtr AlembicInput::metaData() const
{
	const MetaData &md = m_data->object.getMetaData();
	CompoundDataPtr resultData = new CompoundData;
	CompoundDataMap &resultMap = resultData->writable();
	for( MetaData::const_iterator it = md.begin(), eIt = md.end(); it!=eIt; it++ )
	{
		resultMap[it->first] = new StringData( it->second );
	}

	return resultData;
}

size_t AlembicInput::numSamples() const
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

double AlembicInput::timeAtSample( size_t sampleIndex ) const
{
	if( sampleIndex >= numSamples() )
	{
		throw InvalidArgumentException( "Sample index out of range" );
	}
	ensureTimeSampling();
	return m_data->timeSampling->getSampleTime( sampleIndex );
}

double AlembicInput::sampleIntervalAtTime( double time, size_t &floorIndex, size_t &ceilIndex ) const
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

bool AlembicInput::hasStoredBound() const
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

Imath::Box3d AlembicInput::boundAtSample( size_t sampleIndex ) const
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

	return Box3d();
}

Imath::Box3d AlembicInput::boundAtTime( double time ) const
{
	if( hasStoredBound() )
	{
		size_t index0, index1;
		double lerpFactor = sampleIntervalAtTime( time, index0, index1 );
		if( index0 == index1 )
		{
			return boundAtSample( index0 );
		}
		else
		{
			Box3d bound0 = boundAtSample( index0 );
			Box3d bound1 = boundAtSample( index1 );
			Box3d result;
			result.min = lerp( bound0.min, bound1.min, lerpFactor );
			result.max = lerp( bound0.max, bound1.max, lerpFactor );
			return result;
		}
	}
	else
	{
		Box3d result;
		for( size_t i=0, n=numChildren(); i<n; i++ )
		{
			AlembicInputPtr c = child( i );
			Box3d childBound = c->boundAtTime( time );
			childBound = Imath::transform( childBound, c->transformAtTime( time ) );
			result.extendBy( childBound );
		}
		return result;
	}
}

Imath::M44d AlembicInput::transformAtSample( size_t sampleIndex ) const
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

Imath::M44d AlembicInput::transformAtTime( double time ) const
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

IECore::ObjectPtr AlembicInput::objectAtSample( size_t sampleIndex, IECore::TypeId resultType ) const
{
	std::unique_ptr<ObjectReader> r = ObjectReader::create( m_data->object, resultType );
	return r ? r->readSample( sampleIndex ) : nullptr;
}

IECore::ObjectPtr AlembicInput::objectAtTime( double time, IECore::TypeId resultType ) const
{
	std::unique_ptr<ObjectReader> r = ObjectReader::create( m_data->object, resultType );
	if( !r )
	{
		return nullptr;
	}

	size_t index0, index1;
	const double lerpFactor = sampleIntervalAtTime( time, index0, index1 );
	if( index0==index1 )
	{
		return r->readSample( index0 );
	}
	else
	{
		ConstObjectPtr object0 = r->readSample( index0 );
		ConstObjectPtr object1 = r->readSample( index1 );
		if( object0 && object1 )
		{
			return linearObjectInterpolation( object0.get(), object1.get(), lerpFactor );
		}
		return NULL;
	}
}

size_t AlembicInput::numChildren() const
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

AlembicInputPtr AlembicInput::child( size_t index ) const
{
	AlembicInputPtr result = new AlembicInput();
	result->m_data = boost::shared_ptr<DataMembers>( new DataMembers );
	result->m_data->archive = this->m_data->archive;
	/// \todo this is documented as not being the best way of doing things in
	/// the alembic documentation. I'm not sure what would be better though,
	/// and it appears to work fine so far.
	result->m_data->object = this->m_data->object.getChild( index );
	return result;
}

IECore::StringVectorDataPtr AlembicInput::childNames() const
{
	StringVectorDataPtr resultData = new StringVectorData;
	std::vector<std::string> &resultVector = resultData->writable();
	size_t numChildren = this->numChildren();
	for( size_t i=0; i<numChildren; i++ )
	{
		resultVector.push_back( m_data->object.getChildHeader( i ).getName() );
	}
	return resultData;
}

AlembicInputPtr AlembicInput::child( const std::string &name ) const
{
	IObject c = m_data->object.getChild( name );
	if( !c )
	{
		throw InvalidArgumentException( name );
	}

	AlembicInputPtr result = new AlembicInput();
	result->m_data = boost::shared_ptr<DataMembers>( new DataMembers );
	result->m_data->archive = m_data->archive;
	result->m_data->object = c;
	return result;
}

void AlembicInput::ensureTimeSampling() const
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

