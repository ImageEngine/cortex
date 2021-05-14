//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECoreAlembic/AlembicScene.h"

#include "IECoreAlembic/ObjectReader.h"
#include "IECoreAlembic/ObjectWriter.h"

#include "IECoreScene/SampledSceneInterface.h"

#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/PathMatcherData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/Version.h"

#include "Alembic/AbcCoreFactory/IFactory.h"
#include "Alembic/AbcCoreOgawa/ReadWrite.h"
#include "Alembic/AbcGeom/ArchiveBounds.h"
#include "Alembic/AbcGeom/ICamera.h"
#include "Alembic/AbcGeom/IGeomBase.h"
#include "Alembic/AbcGeom/IXform.h"
#include "Alembic/AbcGeom/OXform.h"
#include "Alembic/AbcGeom/OGeomParam.h"

#include "Alembic/AbcCollection/ICollections.h"
#include "Alembic/AbcCollection/OCollections.h"

#include "boost/tokenizer.hpp"

#include "tbb/spin_mutex.h"

#include <memory>
#include <unordered_map>

using namespace Alembic::Abc;
using namespace Alembic::AbcCoreFactory;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcCollection;

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;

namespace
{

bool isAnimated( ICompoundProperty &compoundProperty )
{
	for( size_t i = 0; i < compoundProperty.getNumProperties(); ++i )
	{
		const auto &header = compoundProperty.getPropertyHeader( i );
		if( header.isScalar() )
		{
			if( !IScalarProperty( compoundProperty, header.getName() ).isConstant() )
			{
				return true;
			}
		}
		else
		{
			// Other property types not yet supported by AlembicReader,
			// so no need to check them.
		}
	}

	return false;
}

//! Alembic uses the "interpretation" metadata key to store the semantic of a given type.
GeometricData::Interpretation convertInterpretation( const std::string &interpretation )
{
	if( interpretation == "point" )
	{
		return GeometricData::Interpretation::Point;
	}
	else if( interpretation == "vector" )
	{
		return GeometricData::Interpretation::Vector;
	}
	else if( interpretation == "normal" )
	{
		return GeometricData::Interpretation::Normal;
	}

	return GeometricData::Interpretation::None;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// AlembicIO
//////////////////////////////////////////////////////////////////////////

// Basic AlembicIO class. This provides the internal implementation
// on behalf of AlembicScene. The base class provides methods useful
// with all OpenModes, and derived classes provide methods
// specific to reading and writing.
class AlembicScene::AlembicIO : public IECore::RefCounted
{

	public :

		~AlembicIO() override {}

		virtual std::string fileName() const = 0;
		virtual SceneInterface::Name name() const = 0;

		virtual void path( SceneInterface::Path &path ) const = 0;
		virtual void childNames( SceneInterface::NameList &childNames ) const = 0;
		virtual AlembicIOPtr child( const SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour ) = 0;

};

// Internal implementation class used when reading
class AlembicScene::AlembicReader : public AlembicIO
{

	public :

		AlembicReader( const std::string &fileName )
		{
			IFactory factory;
			// Increasing the number of streams gives better
			// multithreaded performance, because Ogawa locks
			// around the stream. But each stream consumes an
			// additional file handle, so we choose a fairly
			// conservative number of streams, rather than simply
			// matching the core count.
			//
			// I believe that Alembic 1.7.2 removes the locking
			// entirely at which point the number of streams is
			// irrelevant - see https://github.com/alembic/alembic/issues/124
			// for more details.
			factory.setOgawaNumStreams( 4 );
			m_archive = std::make_shared<IArchive>( factory.getArchive( fileName ) );
			if( !m_archive->valid() )
			{
				// Even though the default policy for IFactory is kThrowPolicy, this appears not to
				// be applied when it fails to load an archive - instead it returns an invalid archive.
				throw IECore::Exception( boost::str( boost::format( "Unable to open file \"%s\"" ) % fileName ) );
			}
		}

		// AlembicIO implementation
		// ========================

		std::string fileName() const override
		{
			return m_archive->getName();
		}

		SceneInterface::Name name() const override
		{
			return SceneInterface::Name( m_xform ? m_xform.getName() : "" );
		}

		void path( SceneInterface::Path &path ) const override
		{
			path.clear();
			if( !m_xform )
			{
				return;
			}

			typedef boost::tokenizer<boost::char_separator<char>> Tokenizer;
			Tokenizer tokenizer( m_xform.getFullName(), boost::char_separator<char>( "/" ) );
			for( const string &t : tokenizer )
			{
				path.push_back( t );
			}
		}

		void childNames( IECoreScene::SceneInterface::NameList &childNames ) const override
		{
			IObject p = m_xform;
			if( !p.valid() )
			{
				p = m_archive->getTop();
			}

			for( size_t i = 0, s = p.getNumChildren(); i < s; ++i )
			{
				const AbcA::ObjectHeader &childHeader = p.getChildHeader( i );
				if( IXform::matches( childHeader ) )
				{
					childNames.push_back( childHeader.getName() );
				}
			}
		}

		AlembicIOPtr child( const IECoreScene::SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour ) override
		{
			ChildMapMutex::scoped_lock lock( m_childrenMutex );

			const std::pair<ChildMap::iterator, bool> insertion = m_children.insert( ChildMap::value_type( name, nullptr ) );
			if( insertion.second )
			{
				IObject c = m_xform ? m_xform.getChild( name ) : m_archive->getTop().getChild( name );
				if( c && IXform::matches( c.getMetaData() ) )
				{
					insertion.first->second = new AlembicReader( m_archive, IXform( c, kWrapExisting ) );
				}
			}

			if( !insertion.first->second )
			{
				switch( missingBehaviour )
				{
					case SceneInterface::NullIfMissing :
						return nullptr;
					case SceneInterface::ThrowIfMissing :
						throw IOException( "Child \"" + name.string() + "\" does not exist" );
					case SceneInterface::CreateIfMissing :
						throw InvalidArgumentException( "Child creation not supported" );
				}
			}

			return insertion.first->second;
		}

		ConstAlembicIOPtr child( const IECoreScene::SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour ) const
		{
			return const_cast<AlembicReader *>( this )->child( name, missingBehaviour );
		}

		// Bounds
		// ======

		bool hasBound() const
		{
			return boundProperty();
		}

		size_t numBoundSamples() const
		{
			Abc::IBox3dProperty p = boundProperty();
			if( !p )
			{
				return 0;
			}
			return p.getNumSamples();
		}

		double boundSampleTime( size_t sampleIndex ) const
		{
			Abc::IBox3dProperty p = boundProperty();
			if( !p )
			{
				throw IECore::Exception( "No stored bounds available" );
			}
			return p.getTimeSampling()->getSampleTime( sampleIndex );
		}

		Imath::Box3d boundAtSample( size_t sampleIndex ) const
		{
			Abc::IBox3dProperty p = boundProperty();
			if( !p )
			{
				throw IECore::Exception( "No stored bounds available" );
			}
			return p.getValue( sampleIndex );
		}

		double boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			Abc::IBox3dProperty p = boundProperty();
			if( !p )
			{
				throw IECore::Exception( "No stored bounds available" );
			}
			return sampleInterval( p.getTimeSampling().get(), p.getNumSamples(), time, floorIndex, ceilIndex );
		}

		void boundHash( double time, IECore::MurmurHash &h ) const
		{
			Abc::IBox3dProperty p = boundProperty();
			if( !p )
			{
				throw IECore::Exception( "No stored bounds available" );
			}
			h.append( fileName() );
			h.append( m_xform ? m_xform.getFullName() : "/" );
			if( p.getNumSamples() > 1 )
			{
				h.append( time );
			}
		}

		// Attribute
		// =--------

		bool hasAttribute( const Name &name ) const
		{
			if( !m_xform )
			{
				return false;
			}

			ICompoundProperty userProperties = m_xform.getSchema().getUserProperties();

			if( !userProperties.valid() )
			{
				return false;
			}

			const AbcA::PropertyHeader *propertyHeader = userProperties.getPropertyHeader( name.string() );
			return propertyHeader && propertyHeader->isScalar();
		}

		void attributeNames( NameList &attrs ) const
		{
			attrs.clear();

			if( !m_xform )
			{
				return;
			}

			ICompoundProperty userProperties = m_xform.getSchema().getUserProperties();

			if( !userProperties.valid() )
			{
				return;
			}

			attrs.reserve( userProperties.getNumProperties() );
			for( size_t i = 0; i < userProperties.getNumProperties(); ++i )
			{
				const auto &header = userProperties.getPropertyHeader( i );
				if( header.isScalar() )
				{
					attrs.push_back( header.getName() );
				}
			}
		}

		size_t numAttributeSamples( const Name &name ) const
		{
			if( !m_xform )
			{
				return 0;
			}

			ICompoundProperty userProperties = m_xform.getSchema().getUserProperties();

			if ( !userProperties.valid() )
			{
				return 0;
			}

			Abc::CompoundPropertyReaderPtr propertyReader = GetCompoundPropertyReaderPtr( userProperties );

			if ( !propertyReader )
			{
				return 0;
			}

			ScalarPropertyReaderPtr scalarPropertyReader = propertyReader->getScalarProperty( name.string() );

			if( !scalarPropertyReader )
			{
				return 0;
			}

			return scalarPropertyReader->getNumSamples();
		}

		double attributeSampleTime( const Name &name, size_t sampleIndex ) const
		{
			if( !m_xform )
			{
				return 0.0;
			}

			ICompoundProperty userProperties = m_xform.getSchema().getUserProperties();

			if ( !userProperties.valid() )
			{
				return 0;
			}

			Abc::CompoundPropertyReaderPtr propertyReader = GetCompoundPropertyReaderPtr( userProperties );

			if ( !propertyReader )
			{
				return 0;
			}

			ScalarPropertyReaderPtr scalarPropertyReader = propertyReader->getScalarProperty( name.string() );

			if ( !scalarPropertyReader )
			{
				return 0;
			}
			auto timeSampling = scalarPropertyReader->getTimeSampling();
			return timeSampling->getSampleTime( sampleIndex );
		}

		double attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			if( !m_xform )
			{
				return 0.0;
			}

			ICompoundProperty userProperties = m_xform.getSchema().getUserProperties();

			if ( !userProperties.valid() )
			{
				return 0;
			}

			Abc::CompoundPropertyReaderPtr propertyReader = GetCompoundPropertyReaderPtr( userProperties );

			if ( !propertyReader )
			{
				return 0.0;
			}

			ScalarPropertyReaderPtr scalarPropertyReader = propertyReader->getScalarProperty( name.string() );

			if ( !scalarPropertyReader )
			{
				return 0.0;
			}

			return sampleInterval( scalarPropertyReader->getTimeSampling().get(), scalarPropertyReader->getNumSamples(), time, floorIndex, ceilIndex );
		}

		IECore::ConstObjectPtr readAttributeAtSample( const Name &name, size_t sampleIndex ) const
		{
			if( !m_xform )
			{
				return nullptr;
			}

			const IXformSchema &schema = m_xform.getSchema();
			auto userProperties = schema.getUserProperties();

			const PropertyHeader *propertyHeader = userProperties.getPropertyHeader( name.string() );
			if( !propertyHeader )
			{
				return nullptr;
			}

			if( propertyHeader->getPropertyType() != kScalarProperty )
			{
				IECore::msg(
					IECore::Msg::Warning,
					"AlembicScene::readAttributeAtSample",
					boost::format( "Unsupported property type :%1%. Only scalar properties are currently supported: %2%" ) %
						propertyHeader->getPropertyType() %
						kScalarProperty
				);
			}

			CompoundPropertyReaderPtr propertyReader = GetCompoundPropertyReaderPtr( userProperties );

			ScalarPropertyReaderPtr scalarPropertyReader = propertyReader->getScalarProperty( name.string() );

			if ( !scalarPropertyReader )
			{
				IECore::msg(
					IECore::Msg::Warning,
					"AlembicScene::readAttributeAtSample",
					boost::format( "Unable to read scalar property '%1%'" ) % name
				);

				return nullptr;
			}

			const DataType dataType = scalarPropertyReader->getDataType();
			const PlainOldDataType pod = dataType.getPod();
			const uint8_t extent = dataType.getExtent();
			const MetaData &metaData = scalarPropertyReader->getMetaData();

			auto getInterpretation = [ &metaData ]() -> std::string
			{
				std::string interpretation = metaData.get( "interpretation" );
				return interpretation;
			};

			auto getCortexInterpretation = [ &metaData ] () -> GeometricData::Interpretation
			{
				std::string interpretation = metaData.get( "interpretation" );
				return convertInterpretation( interpretation );
			};

			switch( pod )
			{
				case kBooleanPOD:
				{
					bool_t value;
					scalarPropertyReader->getSample( sampleIndex, &value);
					return new IECore::BoolData( value );
				}
				case kUint8POD:
				{
					if( extent == 1 )
					{
						uint8_t tmpUChar;
						scalarPropertyReader->getSample( sampleIndex, &tmpUChar );
						return new IECore::UCharData( tmpUChar );
					}
					if( extent == 3 && getInterpretation() == "rgb" )
					{
						uint8_t value[3];
						scalarPropertyReader->getSample( sampleIndex, &value );
						return new IECore::Color3fData( C3f( value[0] / 255.0f, value[1] / 255.0f, value[2] / 255.0f ) );
					}
					else if( extent == 4 && getInterpretation() == "rgba" )
					{
						uint8_t value[4];
						scalarPropertyReader->getSample( sampleIndex, &value );
						return new IECore::Color4fData( C4f( value[0] / 255.0f, value[1] / 255.0f, value[2] / 255.0f, value[3] / 255.0f ) );
					}
				}
				case kInt8POD:
				{
					int8_t tmpChar;
					scalarPropertyReader->getSample( sampleIndex, &tmpChar );
					return new IECore::CharData( tmpChar );
				}
				case kUint16POD:
				{
					uint16_t value;
					scalarPropertyReader->getSample( sampleIndex, &value );
					return new IECore::UShortData( value );
				}
				case kInt16POD:
				{
					switch( extent )
					{
						case 1:
						{
							int16_t value;
							scalarPropertyReader->getSample( sampleIndex, &value );
							return new IECore::ShortData( value );
						}
						case 2:
						{
							int16_t value[2];
							scalarPropertyReader->getSample( sampleIndex, &value );
							return new IECore::V2iData( V2i( value[0], value[1] ), getCortexInterpretation() );
						}
						case 3:
						{
							int16_t value[3];
							scalarPropertyReader->getSample( sampleIndex, &value );
							return new IECore::V3iData( V3i( value[0], value[1], value[2] ), getCortexInterpretation() );
						}
						case 4:
						{
							int16_t value[4];
							scalarPropertyReader->getSample( sampleIndex, &value[0] );
							return new IECore::Box2iData( Box2i( V2i( value[0], value[1] ), V2i( value[2], value[3] ) ) );
						}
						case 6:
						{
							int16_t value[6];
							scalarPropertyReader->getSample( sampleIndex, &value[0] );
							return new IECore::Box3iData( Box3i( V3i( value[0], value[1], value[2] ), V3i( value[3], value[4], value[5] ) ) );
						}
						default:
							break;
					}
				}
				case kUint32POD:
				{
					uint32_t value;
					scalarPropertyReader->getSample( sampleIndex, &value );
					return new IECore::UIntData( value );
				}
				case kInt32POD:
				{
					switch( extent )
					{
						case 1:
						{
							int32_t value;
							scalarPropertyReader->getSample( sampleIndex, &value );
							return new IECore::IntData( value );
						}
						case 2:
						{
							int32_t value[2];
							scalarPropertyReader->getSample( sampleIndex, &value[0] );
							return new IECore::V2iData( V2i( value[0], value[1] ), getCortexInterpretation() );
						}
						case 3:
						{
							int32_t value[3];
							scalarPropertyReader->getSample( sampleIndex, &value[0] );
							return new IECore::V3iData( V3i( value[0], value[1], value[2] ), getCortexInterpretation() );
						}
						case 4:
						{
							int32_t tmpValue[4];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::Box2iData( Box2i( V2i( tmpValue[0], tmpValue[1] ), V2i( tmpValue[2], tmpValue[3] ) ) );
						}
						case 6:
						{
							int32_t tmpValue[6];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::Box3iData( Box3i( V3i( tmpValue[0], tmpValue[1], tmpValue[2] ), V3i( tmpValue[3], tmpValue[4], tmpValue[5] ) ) );
						}
						default:
							break;
					}
				}
				case kUint64POD:
				{
					uint64_t value;
					scalarPropertyReader->getSample( sampleIndex, &value );
					return new IECore::UInt64Data( value );
				}
				case kInt64POD:
				{
					int64_t value;
					scalarPropertyReader->getSample( sampleIndex, &value );
					return new IECore::Int64Data( value );
				}
				case kFloat16POD:
				{
					if( extent == 1 )
					{
						half value;
						scalarPropertyReader->getSample( sampleIndex, &value );
						return new IECore::HalfData( value );
					}
					else if( extent == 3 )
					{
						half value[3];
						scalarPropertyReader->getSample( sampleIndex, &value );
						if( getInterpretation() == "rgb" )
						{
							return new IECore::Color3fData( C3f( value[0], value[1], value[2] ) );
						}
					}
					else if( extent == 4 )
					{
						half value[4];
						scalarPropertyReader->getSample( sampleIndex, &value );
						if( getInterpretation() == "rgba" )
						{
							return new IECore::Color4fData( C4f( value[0], value[1], value[2], value[3] ) );
						}
					}
				}
				case kFloat32POD:
				{
					switch( extent )
					{
						case 1:
						{
							float32_t value;
							scalarPropertyReader->getSample( sampleIndex, &value );
							return new IECore::FloatData( value );
						}
						case 2:
						{
							float32_t value[2];
							scalarPropertyReader->getSample( sampleIndex, &value[0] );
							return new IECore::V2fData( V2f( value[0], value[1] ), getCortexInterpretation() );
						}
						case 3:
						{
							float32_t value[3];
							scalarPropertyReader->getSample( sampleIndex, &value[0] );

							if( getCortexInterpretation() != GeometricData::Interpretation::None )
							{
								return new IECore::V3fData( V3f( value[0], value[1], value[2] ), getCortexInterpretation()  );
							}
							else if( getInterpretation() == "rgb" )
							{
								return new IECore::Color3fData( C3f( value[0], value[1], value[2] ) );
							}
						}
						case 4:
						{
							float32_t tmpValue[4];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );

							if( getInterpretation() == "quat" )
							{
								return new IECore::QuatfData( Quatf( tmpValue[0], tmpValue[1], tmpValue[2], tmpValue[3] ) );
							}
							else if( getInterpretation() == "box" )
							{
								return new IECore::Box2fData( Box2f( V2f( tmpValue[0], tmpValue[1] ), V2f( tmpValue[2], tmpValue[3] ) ) );
							}
							else if( getInterpretation() == "rgba" )
							{
								return new IECore::Color4fData( C4f( tmpValue[0], tmpValue[1], tmpValue[2], tmpValue[3] ) );
							}
						}
						case 6:
						{
							float32_t tmpValue[6];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::Box3fData( Box3f( V3d( tmpValue[0], tmpValue[1], tmpValue[2] ), V3f( tmpValue[3], tmpValue[4], tmpValue[5] ) ) );
						}
						case 9:
						{
							float32_t tmpValue[9];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::M33fData(
								M33f(
									tmpValue[0], tmpValue[1], tmpValue[2], tmpValue[3], tmpValue[4], tmpValue[5], tmpValue[6], tmpValue[7], tmpValue[8]
								)
							);
						}
						case 16:
						{
							float32_t tmpValue[16];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::M44fData(
								M44f(
									tmpValue[0],
									tmpValue[1],
									tmpValue[2],
									tmpValue[3],
									tmpValue[4],
									tmpValue[5],
									tmpValue[6],
									tmpValue[7],
									tmpValue[8],
									tmpValue[9],
									tmpValue[10],
									tmpValue[11],
									tmpValue[12],
									tmpValue[13],
									tmpValue[14],
									tmpValue[15]
								)
							);
						}
						default:
							break;
					}
				}
				case kFloat64POD:
				{
					switch( extent )
					{
						case 1:
						{
							float64_t value;
							scalarPropertyReader->getSample( sampleIndex, &value );
							return new IECore::DoubleData( value );
						}
						case 2:
						{
							float64_t tmpValue[2];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::V2dData( V2d( tmpValue[0], tmpValue[1] ), getCortexInterpretation() );
						}
						case 3:
						{
							float64_t tmpValue[3];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::V3dData( V3d( tmpValue[0], tmpValue[1], tmpValue[2] ), getCortexInterpretation() );
						}
						case 4:
						{
							float64_t tmpValue[4];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							if( getInterpretation() == "quat" )
							{
								return new IECore::QuatdData( Quatd( tmpValue[0], tmpValue[1], tmpValue[2], tmpValue[3] ) );
							}
							else if( getInterpretation() == "box" )
							{
								return new IECore::Box2dData( Box2d( V2d( tmpValue[0], tmpValue[1] ), V2d( tmpValue[2], tmpValue[3] ) ) );
							}
						}
						case 6:
						{
							float64_t tmpValue[6];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::Box3dData( Box3d( V3d( tmpValue[0], tmpValue[1], tmpValue[2] ), V3d( tmpValue[3], tmpValue[4], tmpValue[5] ) ) );
						}
						case 9:
						{
							float64_t tmpValue[9];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::M33dData(
								M33d(
									tmpValue[0], tmpValue[1], tmpValue[2], tmpValue[3], tmpValue[4], tmpValue[5], tmpValue[6], tmpValue[7], tmpValue[8]
								)
							);
						}
						case 16:
						{
							float64_t tmpValue[16];
							scalarPropertyReader->getSample( sampleIndex, &tmpValue[0] );
							return new IECore::M44dData(
								M44d(
									tmpValue[0],
									tmpValue[1],
									tmpValue[2],
									tmpValue[3],
									tmpValue[4],
									tmpValue[5],
									tmpValue[6],
									tmpValue[7],
									tmpValue[8],
									tmpValue[9],
									tmpValue[10],
									tmpValue[11],
									tmpValue[12],
									tmpValue[13],
									tmpValue[14],
									tmpValue[15]
								)
							);
						}
						default:
							break;
					}

				}

				case kStringPOD:
				{
					std::string tmpStr;
					scalarPropertyReader->getSample( sampleIndex, &tmpStr );

					return new IECore::StringData( tmpStr );
				}

				case kWstringPOD:
				case kUnknownPOD:
				default:
					break;

			}

			IECore::msg(
				IECore::Msg::Warning,
				"AlembicScene::readAttributeAtSample",
				boost::format( "Unsupported attribute type datatype: \"%1%\" extend:%2% interpretation:\"%3%\"" ) % pod % extent % getInterpretation()
			);

			return nullptr;
		}

		void attributeHash( double time, IECore::MurmurHash &h ) const
		{
			if( !m_xform )
			{
				return;
			}

			const IXformSchema &schema = m_xform.getSchema();
			ICompoundProperty compoundProperty = schema.getUserProperties();

			if( compoundProperty.valid() )
			{
				h.append( fileName() );
				h.append( m_xform ? m_xform.getFullName() : "/" );

				if( isAnimated( compoundProperty ) )
				{
					h.append( time );
				}
			}
		}


		// Transforms
		// ==========

		size_t numTransformSamples() const
		{
			if( !m_xform )
			{
				return 0;
			}
			return m_xform.getSchema().getNumSamples();
		}

		double transformSampleTime( size_t sampleIndex ) const
		{
			if( !m_xform )
			{
				return 0.0;
			}

			return m_xform.getSchema().getTimeSampling()->getSampleTime( sampleIndex );
		}

		Imath::M44d transformAtSample( size_t sampleIndex ) const
		{
			if( !m_xform )
			{
				return M44d();
			}

			const IXformSchema &schema = m_xform.getSchema();
			XformSample sample;
			schema.get( sample, ISampleSelector( (index_t)sampleIndex ) );
			return sample.getMatrix();
		}

		Imath::M44d transformAtTime( double time ) const
		{
			if( !m_xform )
			{
				return M44d();
			}

			const IXformSchema &schema = m_xform.getSchema();
			size_t index0, index1;
			double lerpFactor = sampleInterval( schema.getTimeSampling().get(), schema.getNumSamples(), time, index0, index1 );

			if( index0 == index1 )
			{
				return transformAtSample( index0 );
			}

			XformSample sample0;
			schema.get( sample0, ISampleSelector( (index_t)index0 ) );
			XformSample sample1;
			schema.get( sample1, ISampleSelector( (index_t)index1 ) );

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
						Imath::lerp( op0.getChannelValue( channelIndex ), op1.getChannelValue( channelIndex ), lerpFactor )
					);
				}

				interpolatedSample.addOp( interpolatedOp );
			}

			return interpolatedSample.getMatrix();
		}

		double transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			if( !m_xform )
			{
				return 0.0;
			}
			const IXformSchema &schema = m_xform.getSchema();
			return sampleInterval( schema.getTimeSampling().get(), schema.getNumSamples(), time, floorIndex, ceilIndex );
		}

		void transformHash( double time, IECore::MurmurHash &h ) const
		{
			if( m_xform )
			{
				Alembic::Util::Digest digest;
				if( const_cast<IXform &>( m_xform ).getPropertiesHash( digest ) )
				{
					h.append( digest.words, 2 );
				}
				else
				{
					h.append( fileName() );
					h.append( m_xform ? m_xform.getFullName() : "/" );
				}

				const IXformSchema &schema = m_xform.getSchema();
				if( schema.getNumSamples() > 1 )
				{
					h.append( time );
				}
			}
		}

		// Objects
		// =======

		bool hasObject() const
		{
			return static_cast<bool>( m_objectReader );
		}

		size_t numObjectSamples() const
		{
			return m_objectReader ? m_objectReader->readNumSamples() : 0;
		}

		double objectSampleTime( size_t sampleIndex ) const
		{
			if( !m_objectReader )
			{
				return 0.0;
			}
			return m_objectReader->readTimeSampling()->getSampleTime( sampleIndex );
		}

		IECore::ConstObjectPtr objectAtSample( size_t sampleIndex ) const
		{
			return m_objectReader ? m_objectReader->readSample( sampleIndex ) : nullptr;
		}

		double objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			if( !m_objectReader )
			{
				return 0.0;
			}
			const size_t numSamples = m_objectReader->readNumSamples();
			TimeSamplingPtr timeSampling = m_objectReader->readTimeSampling();
			return sampleInterval( timeSampling.get(), numSamples, time, floorIndex, ceilIndex );
		}

		void objectHash( double time, IECore::MurmurHash &h ) const
		{
			if( m_objectReader )
			{
				Alembic::Util::Digest digest;
				if( const_cast<IObject &>( m_objectReader->object() ).getPropertiesHash( digest ) )
				{
					h.append( digest.words, 2 );
				}
				else
				{
					h.append( fileName() );
					h.append( m_xform.getFullName() );
				}

				if( m_objectReader->readNumSamples() > 1 )
				{
					h.append( time );
				}
			}
		}

		// Sets
		// ====

		NameList setNames( bool includeDescendantSets ) const
		{
			NameList setNames;

			if( !m_xform )
			{
				return setNames;
			}

			if( m_xform.getChildHeader( "sets" ) )
			{
				ICollections collections( m_xform, "sets" );
				ICollectionsSchema &collectionsSchema = collections.getSchema();

				size_t numCollections = collectionsSchema.getNumCollections();
				setNames.reserve( numCollections );
				for( size_t i = 0; i < numCollections; ++i )
				{
					setNames.emplace_back( collectionsSchema.getCollectionName( i ) );
				}
			}

			if( includeDescendantSets )
			{
				NameList children;
				childNames( children );
				for( const SceneInterface::Name &childName : children )
				{
					ConstAlembicIOPtr c = child( childName, NullIfMissing );

					NameList childSetNames = dynamic_cast<const AlembicReader *> ( c.get() )->setNames( includeDescendantSets );
					setNames.insert( setNames.begin(), childSetNames.begin(), childSetNames.end() );
				}
			}

			// ensure our set names are unique
			std::sort( setNames.begin(), setNames.end() );
			return NameList( setNames.begin(), std::unique( setNames.begin(), setNames.end() ) );

			return setNames;
		}

		IECore::PathMatcher readSet( const Name &name, bool includeDescendantSets ) const
		{
			SceneInterface::Path prefix;
			PathMatcher pathMatcher;
			recurseReadSet( prefix, name, pathMatcher, includeDescendantSets );

			return pathMatcher;
		}

		void hashSet( const SceneInterface::Name& setName, IECore::MurmurHash &h ) const
		{
			SceneInterface::Path p;

			// read the current path into p
			path( p );

			h.append( fileName() );
			h.append( &p[0], p.size() );
			h.append( setName );
		}

		// Additional hashes
		// =================

		void childNamesHash( IECore::MurmurHash &h ) const
		{
			if( m_objectReader && m_xform.getNumChildren() == 1 )
			{
				// Leaf. There are no children so we
				// can use the same hash as all other leaves.
			}
			else
			{
				h.append( fileName() );
				h.append( m_xform ? m_xform.getFullName() : "/" );
			}
		}

		void hierarchyHash( double time, IECore::MurmurHash &h ) const
		{
			IObject o = m_xform;
			if( !o )
			{
				o = m_archive->getTop();
			}

			Alembic::Util::Digest propertiesDigest;
			Alembic::Util::Digest childrenDigest;
			if( o.getPropertiesHash( propertiesDigest ) && o.getChildrenHash( childrenDigest ) )
			{
				h.append( propertiesDigest.words, 2 );
				h.append( childrenDigest.words, 2 );
			}
			else
			{
				h.append( fileName() );
				h.append( m_xform ? m_xform.getFullName() : "/" );
			}

			if( m_archive->getNumTimeSamplings() > 1 )
			{
				h.append( time );
			}
		}

	private :

		AlembicReader( const std::shared_ptr<IArchive> &archive, const IXform &xform = IXform() )
			:	m_archive( archive ), m_xform( xform )
		{
			if( !m_xform )
			{
				return;
			}
			for( size_t i = 0, s = m_xform.getNumChildren(); i < s; ++i )
			{
				const AbcA::ObjectHeader &childHeader = m_xform.getChildHeader( i );
				if( !IXform::matches( childHeader ) )
				{
					m_objectReader = ObjectReader::create( m_xform.getChild( i ) );
					return;
				}
			}
		}

		Abc::IBox3dProperty boundProperty() const
		{
			if( !m_xform )
			{
				// Top of archive
				return GetIArchiveBounds( *m_archive, Abc::ErrorHandler::kQuietNoopPolicy );
			}
			else if( m_objectReader && m_xform.getNumChildren() == 1 )
			{
				// Leaf object
				return m_objectReader->readBoundProperty();
			}
			else
			{
				// Intermediate transform
				return m_xform.getSchema().getChildBoundsProperty();
			}

		}

		double sampleInterval( const TimeSampling *timeSampling, size_t numSamples, double time, size_t &floorIndex, size_t &ceilIndex ) const
		{
			if( !timeSampling || !numSamples )
			{
				floorIndex = ceilIndex = 0;
				return 0;
			}

			std::pair<Alembic::AbcCoreAbstract::index_t, chrono_t> f = timeSampling->getFloorIndex( time, numSamples );
			if( fabs( time - f.second ) < 0.0001 )
			{
				// It's going to be very common to be reading on the whole frame, so we want to make sure
				// that anything thereabouts is loaded as a single uninterpolated sample for speed.
				floorIndex = ceilIndex = f.first;
				return 0.0;
			}

			std::pair<Alembic::AbcCoreAbstract::index_t, chrono_t> c = timeSampling->getCeilIndex( time, numSamples );
			if( f.first == c.first || fabs( time - c.second ) < 0.0001 )
			{
				// Return a result not needing interpolation if possible. Either we only had one sample
				// to pick from or the ceiling sample was close enough to perfect.
				floorIndex = ceilIndex = c.first;
				return 0.0;
			}

			floorIndex = f.first;
			ceilIndex = c.first;

			return ( time - f.second ) / ( c.second - f.second );
		}

		void recurseReadSet( const SceneInterface::Path &prefix, const Name &name, IECore::PathMatcher &pathMatcher, bool includeDescendantSets ) const
		{
			if( PathMatcherDataPtr pathMatcherData = readLocalSet( name ) )
			{
				pathMatcher.addPaths( pathMatcherData->readable(), prefix );
			}

			if ( !includeDescendantSets )
			{
				return;
			}

			NameList children;
			childNames( children );

			SceneInterface::Path childPrefix = prefix;
			childPrefix.resize( prefix.size() + 1 );

			for( InternedString &childName : children )
			{
				*childPrefix.rbegin() = childName;

				ConstAlembicIOPtr c = child( childName, SceneInterface::ThrowIfMissing );

				dynamic_cast<const AlembicReader*>( c.get() )->recurseReadSet( childPrefix, name, pathMatcher, includeDescendantSets );
			}
		}

		IECore::PathMatcherDataPtr readLocalSet( const Name &name ) const
		{
			PathMatcherDataPtr pathMatcher = new PathMatcherData();

			if ( !m_xform )
			{
				return pathMatcher;
			}

			if ( !m_xform.getChildHeader("sets") )
			{
				return pathMatcher;
			}

			ICollections collections(m_xform, "sets");
			ICollectionsSchema& collectionsSchema = collections.getSchema();

			IStringArrayProperty setProperty = collectionsSchema.getCollection( name.string() );

			if ( !setProperty )
			{
				return pathMatcher;
			}

			IStringArrayProperty::sample_ptr_type samplePtr = setProperty.getValue();

			PathMatcher &writablePathMatcher = pathMatcher->writable();

			if ( samplePtr )
			{
				const IStringArrayProperty::sample_type &sample = *samplePtr;
				for (size_t i = 0; i < sample.size(); ++i)
				{
					writablePathMatcher.addPath ( sample[i] );
				}
			}

			return pathMatcher;
		}

		std::shared_ptr<IArchive> m_archive;
		IXform m_xform; // Empty when we're at the root
		std::unique_ptr<IECoreAlembic::ObjectReader> m_objectReader; // Null when there's no object

		typedef std::unordered_map<IECoreScene::SceneInterface::Name, AlembicReaderPtr> ChildMap;
		ChildMap m_children;
		typedef tbb::spin_mutex ChildMapMutex;
		ChildMapMutex m_childrenMutex;

};

class AlembicScene::AlembicWriter : public AlembicIO
{

	public :

		AlembicWriter( const std::string &fileName )
		{
			m_root = std::make_shared<Root>();
			m_root->archive = Abc::CreateArchiveWithInfo( ::Alembic::AbcCoreOgawa::WriteArchive(), fileName, std::string("Cortex ") + IECore::versionString(), "");;
		}

		~AlembicWriter() override
		{
			/// \todo Do better. We don't want to be storing huge sample times vectors
			/// when a long animation is being written. We need to somehow detect uniform and
			/// cyclic sampling patterns on the fly and create TimeSamplings to reflect that.
			if( m_xformSampleTimes.size() )
			{
				TimeSamplingPtr ts( new TimeSampling( TimeSamplingType( TimeSamplingType::kAcyclic ), m_xformSampleTimes ) );
				m_xform.getSchema().setTimeSampling( ts );
			}
			if( m_boundSampleTimes.size() )
			{
				TimeSamplingPtr ts( new TimeSampling( TimeSamplingType( TimeSamplingType::kAcyclic ), m_boundSampleTimes ) );
				if( haveXform() )
				{
					m_xform.getSchema().getChildBoundsProperty().setTimeSampling( ts );
				}
				else
				{
					m_root->boundProperty().setTimeSampling( ts );
				}
			}
			if( m_objectSampleTimes.size() && m_objectWriter )
			{
				TimeSamplingPtr ts( new TimeSampling( TimeSamplingType( TimeSamplingType::kAcyclic ), m_objectSampleTimes ) );
				m_objectWriter->writeTimeSampling( ts );
			}

			for( const auto &attributeTimeSamples : m_attributeSampleTimes )
			{
				if( attributeTimeSamples.second.size() > 1 )
				{
					TimeSamplingPtr ts( new TimeSampling( TimeSamplingType( TimeSamplingType::kAcyclic ), attributeTimeSamples.second ) );
					m_scalarProperties[attributeTimeSamples.first].setTimeSampling( ts );
				}
			}

		}

		// AlembicIO implementation
		// ========================

		std::string fileName() const override
		{
			return m_root->archive.getName();
		}

		SceneInterface::Name name() const override
		{
			return SceneInterface::Name( haveXform() ? m_xform.getName() : "" );
		}

		void path( SceneInterface::Path &path ) const override
		{
			path.clear();
			if( !haveXform() )
			{
				return;
			}

			typedef boost::tokenizer<boost::char_separator<char>> Tokenizer;
			Tokenizer tokenizer( m_xform.getFullName(), boost::char_separator<char>( "/" ) );
			for( const string &t : tokenizer )
			{
				path.push_back( t );
			}
		}

		void childNames( IECoreScene::SceneInterface::NameList &childNames ) const override
		{
			OObject p = m_xform;
			if( !haveXform() )
			{
				p = m_root->archive.getTop();
			}

			for( size_t i = 0, s = p.getNumChildren(); i < s; ++i )
			{
				const AbcA::ObjectHeader &childHeader = p.getChildHeader( i );
				if( OXform::matches( childHeader ) )
				{
					childNames.push_back( childHeader.getName() );
				}
			}
		}

		AlembicIOPtr child( const IECoreScene::SceneInterface::Name &name, SceneInterface::MissingBehaviour missingBehaviour ) override
		{
			ChildMap::iterator it = m_children.find( name );
			if( it != m_children.end() )
			{
				return it->second;
			}
			switch( missingBehaviour )
			{
				case SceneInterface::NullIfMissing :
					return nullptr;
				case SceneInterface::ThrowIfMissing :
					throw IOException( "Child \"" + name.string() + "\" does not exist" );
				case SceneInterface::CreateIfMissing :
				{
					AlembicWriterPtr child = new AlembicWriter(
						m_root,
						OXform( haveXform() ? m_xform : m_root->archive.getTop(), name.string() )
					);
					m_children[name] = child;
					return child;
				}
				default :
				{
					// should never get here
					assert( nullptr );
					return nullptr;
				}
			}
		}

		// Transforms
		// ==========

		void writeTransform( const IECore::Data *transform, double time )
		{
			if( !haveXform() )
			{
				throw IECore::Exception( "Cannot write transform at root" );
			}

			XformSample sample;
			if( const M44dData *matrixData = runTimeCast<const M44dData>( transform ) )
			{
				sample.setMatrix( matrixData->readable() );
			}
			else if( const TransformationMatrixdData *transformationMatrixData = runTimeCast<const TransformationMatrixdData>( transform ) )
			{
				sample.setMatrix( transformationMatrixData->readable().transform() );
			}
			else
			{
				throw IECore::Exception( boost::str( boost::format( "Unsupported data type : '%1%'" ) % transform->typeName() ) );
			}

			if( m_xformSampleTimes.size() && m_xformSampleTimes.back() >= time )
			{
				throw IECore::Exception( "Samples must be written in time-increasing order" );
			}
			m_xformSampleTimes.push_back( time );

			OXformSchema &schema = m_xform.getSchema();
			schema.set( sample );
		}

		// Bounds
		// ======

		void writeBound( const Box3d &bound, double time )
		{
			if( m_boundSampleTimes.size() && m_boundSampleTimes.back() >= time )
			{
				throw IECore::Exception( "Samples must be written in time-increasing order" );
			}
			m_boundSampleTimes.push_back( time );

			if( haveXform() )
			{
				m_xform.getSchema().getChildBoundsProperty().set( bound );
			}
			else
			{
				m_root->boundProperty().set( bound );
			}
		}


		// Attribute
		// =--------

		template<typename T, typename D>
		void setProperty( const Name &name, double time, const D *data )
		{
			m_attributeSampleTimes[name].push_back( time );

			auto it = m_scalarProperties.find( name );

			if( it != m_scalarProperties.end() )
			{
				it->second.set( (void *) &data->readable() );
				return;
			}

			OXformSchema &schema = m_xform.getSchema();

			T prop( schema.getUserProperties(), name );
			m_scalarProperties.insert( std::make_pair( name, prop ) );

			prop.set( data->readable() );
		}

		void writeAttribute( const Name &name, const IECore::Object *attribute, double time )
		{
			if( !haveXform() )
			{
				IECore::msg(IECore::MessageHandler::Level::Warning, __func__,
					boost::str(
						boost::format( "Cannot write attribute ( attribute name: '%1%', attribute type: '%2%', time: %3% ) at root. " ) %
							name.string() %
							attribute->typeName() %
							time
					)
				);
				return;
			}

			if( const IECore::BoolData *data = runTimeCast<const IECore::BoolData>( attribute ) )
			{
				setProperty<OBoolProperty>( name, time, data );
			}
			else if( const IECore::UCharData *data = runTimeCast<const IECore::UCharData>( attribute ) )
			{
				setProperty<OUcharProperty>( name, time, data );
			}
			else if( const IECore::CharData *data = runTimeCast<const IECore::CharData>( attribute ) )
			{
				setProperty<OCharProperty>( name, time, data );
			}
			else if( const IECore::UShortData *data = runTimeCast<const IECore::UShortData>( attribute ) )
			{
				setProperty<OUInt16Property>( name, time, data );
			}
			else if( const IECore::ShortData *data = runTimeCast<const IECore::ShortData>( attribute ) )
			{
				setProperty<OInt16Property>( name, time, data );
			}
			else if( const IECore::UIntData *data = runTimeCast<const IECore::UIntData>( attribute ) )
			{
				setProperty<OUInt32Property>( name, time, data );
			}
			else if( const IECore::IntData *data = runTimeCast<const IECore::IntData>( attribute ) )
			{
				setProperty<OInt32Property>( name, time, data );
			}
			else if( const IECore::UInt64Data *data = runTimeCast<const IECore::UInt64Data>( attribute ) )
			{
				setProperty<OUInt64Property>( name, time, data );
			}
			else if( const IECore::Int64Data *data = runTimeCast<const IECore::Int64Data>( attribute ) )
			{
				setProperty<OInt64Property>( name, time, data );
			}
			else if( const IECore::HalfData *data = runTimeCast<const IECore::HalfData>( attribute ) )
			{
				setProperty<OHalfProperty>( name, time, data );
			}
			else if( const IECore::FloatData *data = runTimeCast<const IECore::FloatData>( attribute ) )
			{
				setProperty<OFloatProperty>( name, time, data );
			}
			else if( const IECore::DoubleData *data = runTimeCast<const IECore::DoubleData>( attribute ) )
			{
				setProperty<ODoubleProperty>( name, time, data );
			}
			else if( const IECore::StringData *data = runTimeCast<const IECore::StringData>( attribute ) )
			{
				setProperty<OStringProperty>( name, time, data );
			}
			else if( const IECore::V2iData *data = runTimeCast<const IECore::V2iData>( attribute ) )
			{
				if( data->getInterpretation() == GeometricData::Interpretation::Point )
				{
					setProperty<OP2iProperty>( name, time, data );
				}
				else
				{
					setProperty<OV2iProperty>( name, time, data );
				}
			}
			else if( const IECore::V2fData *data = runTimeCast<const IECore::V2fData>( attribute ) )
			{
				if( data->getInterpretation() == GeometricData::Interpretation::Point )
				{
					setProperty<OP2fProperty>( name, time, data );
				}
				else if( data->getInterpretation() == GeometricData::Interpretation::Normal )
				{
					setProperty<ON2fProperty>( name, time, data );
				}
				else
				{
					setProperty<OV2fProperty>( name, time, data );
				}
			}
			else if( const IECore::V2dData *data = runTimeCast<const IECore::V2dData>( attribute ) )
			{
				if( data->getInterpretation() == GeometricData::Interpretation::Point )
				{
					setProperty<OP2dProperty>( name, time, data );
				}
				else if( data->getInterpretation() == GeometricData::Interpretation::Normal )
				{
					setProperty<ON2dProperty>( name, time, data );
				}
				else
				{
					setProperty<OV2dProperty>( name, time, data );
				}
			}
			else if( const IECore::V3iData *data = runTimeCast<const IECore::V3iData>( attribute ) )
			{
				if( data->getInterpretation() == GeometricData::Interpretation::Point )
				{
					setProperty<OP3iProperty>( name, time, data );
				}
				else
				{
					setProperty<OV3iProperty>( name, time, data );
				}
			}
			else if( const IECore::V3fData *data = runTimeCast<const IECore::V3fData>( attribute ) )
			{
				if( data->getInterpretation() == GeometricData::Interpretation::Point )
				{
					setProperty<OP3fProperty>( name, time, data );
				}
				else if( data->getInterpretation() == GeometricData::Interpretation::Normal )
				{
					setProperty<ON3fProperty>( name, time, data );
				}
				else
				{
					setProperty<OV3fProperty>( name, time, data );
				}
			}
			else if( const IECore::V3dData *data = runTimeCast<const IECore::V3dData>( attribute ) )
			{

				if( data->getInterpretation() == GeometricData::Interpretation::Point )
				{
					setProperty<OP3dProperty>( name, time, data );
				}
				else if( data->getInterpretation() == GeometricData::Interpretation::Normal )
				{
					setProperty<ON3dProperty>( name, time, data );
				}
				else
				{
					setProperty<OV3dProperty>( name, time, data );
				}
			}
			else if( const IECore::Box2iData *data = runTimeCast<const IECore::Box2iData>( attribute ) )
			{
				setProperty<OBox2iProperty>( name, time, data );
			}
			else if( const IECore::Box2fData *data = runTimeCast<const IECore::Box2fData>( attribute ) )
			{
				setProperty<OBox2fProperty>( name, time, data );
			}
			else if( const IECore::Box2dData *data = runTimeCast<const IECore::Box2dData>( attribute ) )
			{
				setProperty<OBox2dProperty>( name, time, data );
			}
			else if( const IECore::Box3iData *data = runTimeCast<const IECore::Box3iData>( attribute ) )
			{
				setProperty<OBox3iProperty>( name, time, data );
			}
			else if( const IECore::Box3fData *data = runTimeCast<const IECore::Box3fData>( attribute ) )
			{
				setProperty<OBox3fProperty>( name, time, data );
			}
			else if( const IECore::Box3dData *data = runTimeCast<const IECore::Box3dData>( attribute ) )
			{
				setProperty<OBox3dProperty>( name, time, data );
			}
			else if( const IECore::M33fData *data = runTimeCast<const IECore::M33fData>( attribute ) )
			{
				setProperty<OM33fProperty>( name, time, data );
			}
			else if( const IECore::M33dData *data = runTimeCast<const IECore::M33dData>( attribute ) )
			{
				setProperty<OM33dProperty>( name, time, data );
			}
			else if( const IECore::M44fData *data = runTimeCast<const IECore::M44fData>( attribute ) )
			{
				setProperty<OM44fProperty>( name, time, data );
			}
			else if( const IECore::M44dData *data = runTimeCast<const IECore::M44dData>( attribute ) )
			{
				setProperty<OM44dProperty>( name, time, data );
			}
			else if( const IECore::QuatfData *data = runTimeCast<const IECore::QuatfData>( attribute ) )
			{
				setProperty<OQuatfProperty>( name, time, data );
			}
			else if( const IECore::QuatdData *data = runTimeCast<const IECore::QuatdData>( attribute ) )
			{
				setProperty<OQuatdProperty>( name, time, data );
			}
			else if( const IECore::Color3fData *data = runTimeCast<const IECore::Color3fData>( attribute ) )
			{
				setProperty<OC3fProperty>( name, time, data );
			}
			else if( const IECore::Color4fData *data = runTimeCast<const IECore::Color4fData>( attribute ) )
			{
				setProperty<OC4fProperty>( name, time, data );
			}
			else
			{
				Path path;
				std::string pathStr;
				this->path( path );
				pathToString( path, pathStr );
				IECore::msg(
						IECore::Msg::Warning,
						"AlembicScene::writeAttribute",
						boost::str(
							boost::format( "Cannot write attribute ( attribute name: '%1%', attribute type: '%2%', time: %3% ) at location '%4%'. " ) %
								name.string() %
								attribute->typeName() %
								time %
								pathStr
						)
					);

			}

		}


		// Object
		// ======

		void writeObject( const IECore::Object *object, double time )
		{
			if( !haveXform() )
			{
				throw IECore::Exception( "Cannot write object at root" );
			}

			if( m_objectSampleTimes.size() && m_objectSampleTimes.back() >= time )
			{
				throw IECore::Exception( "Samples must be written in time-increasing order" );
			}
			m_objectSampleTimes.push_back( time );

			if( !m_objectWriter )
			{
				m_objectWriter = ObjectWriter::create( object->typeId(), m_xform, "object" );
				if( !m_objectWriter )
				{
					IECore::msg(
						IECore::Msg::Warning,
						"AlembicScene::writeObject",
						boost::format( "Unsupported object type \"%1%\"" ) % object->typeName()
					);
					return;
				}
			}
			m_objectWriter->writeSample( object );
		}

		// Sets
		// ====

		void writeSet( const Name &name, IECore::PathMatcher set )
		{
			if ( !haveXform() )
			{
				// warning & return
				return;
			}

			if ( !m_collections )
			{
				m_collections.reset( new OCollections(m_xform , "sets") );
			}

			OCollectionsSchema &oCollectionsSchema = m_collections->getSchema();

			std::vector<std::string> setLocations;
			for (PathMatcher::Iterator it = set.begin(); it != set.end(); ++it)
			{
				std::string strPath;
				SceneInterface::pathToString( *it, strPath);

				setLocations.push_back( strPath );
			}

			OStringArrayProperty setProperty = oCollectionsSchema.createCollection( name.string() );
			setProperty.set( StringArraySample( setLocations ) );
		}

	private :

		struct Root;

		AlembicWriter( const std::shared_ptr<Root> &root, const OXform &xform = OXform() )
			:	m_root( root ), m_xform( xform )
		{
		}

		// If we're at the root, m_xform is empty. Ideally we would just use
		// the implicit `!m_xform` bool conversion to test for this, but
		// OXform::valid returns false until a sample has been written, so we
		// use this convenience function instead.
		bool haveXform() const
		{
			return m_xform.OObject::valid();
		}

		struct Root
		{

			OArchive archive;

			OBox3dProperty &boundProperty()
			{
				if( !m_boundProperty )
				{
					m_boundProperty = CreateOArchiveBounds( archive );
				}
				return m_boundProperty;
			}

			private :

				OBox3dProperty m_boundProperty;

		};

		std::shared_ptr<Root> m_root;
		OXform m_xform;
		std::unique_ptr<ObjectWriter> m_objectWriter;

		std::vector<chrono_t> m_xformSampleTimes;
		std::vector<chrono_t> m_boundSampleTimes;
		std::vector<chrono_t> m_objectSampleTimes;
		std::map<IECore::InternedString, std::vector<chrono_t> > m_attributeSampleTimes;
		std::map<IECore::InternedString, OScalarProperty> m_scalarProperties;

		std::unique_ptr<OCollections> m_collections;

		typedef std::unordered_map<IECoreScene::SceneInterface::Name, AlembicWriterPtr> ChildMap;
		ChildMap m_children;

};

//////////////////////////////////////////////////////////////////////////
// AlembicScene
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( AlembicScene );

AlembicScene::AlembicScene( const std::string &fileName, IECore::IndexedIO::OpenMode mode )
{
	switch( mode )
	{
		case IECore::IndexedIO::Read :
			m_io = m_root = new AlembicReader( fileName );
			break;
		case IECore::IndexedIO::Write :
			m_io = m_root = new AlembicWriter( fileName );
			break;
		default :
			throw IECore::Exception( "Unsupported OpenMode" );
	}
}

AlembicScene::AlembicScene( const AlembicIOPtr &root, const AlembicIOPtr &io )
	:	m_root( root ), m_io( io )
{
}

AlembicScene::~AlembicScene()
{
}

std::string AlembicScene::fileName() const
{
	return m_io->fileName();
}

SceneInterface::Name AlembicScene::name() const
{
	return m_io->name();
}

void AlembicScene::path( Path &p ) const
{
	return m_io->path( p );
}

// Bound
// =====

bool AlembicScene::hasBound() const
{
	return reader()->hasBound();
}

size_t AlembicScene::numBoundSamples() const
{
	return reader()->numBoundSamples();
}

double AlembicScene::boundSampleTime( size_t sampleIndex ) const
{
	return reader()->boundSampleTime( sampleIndex );
}

double AlembicScene::boundSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return reader()->boundSampleInterval( time, floorIndex, ceilIndex );
}

Imath::Box3d AlembicScene::readBoundAtSample( size_t sampleIndex ) const
{
	return reader()->boundAtSample( sampleIndex );
}

void AlembicScene::writeBound( const Imath::Box3d &bound, double time )
{
	writer()->writeBound( bound, time );
}

// Transform
// =========

size_t AlembicScene::numTransformSamples() const
{
	return reader()->numTransformSamples();
}

double AlembicScene::transformSampleTime( size_t sampleIndex ) const
{
	return reader()->transformSampleTime( sampleIndex );
}

double AlembicScene::transformSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return reader()->transformSampleInterval( time, floorIndex, ceilIndex );
}

IECore::ConstDataPtr AlembicScene::readTransformAtSample( size_t sampleIndex ) const
{
	return new IECore::M44dData( readTransformAsMatrixAtSample( sampleIndex ) );
}

Imath::M44d AlembicScene::readTransformAsMatrixAtSample( size_t sampleIndex ) const
{
	return reader()->transformAtSample( sampleIndex );
}

IECore::ConstDataPtr AlembicScene::readTransform( double time ) const
{
	return new IECore::M44dData( readTransformAsMatrix( time ) );
}

Imath::M44d AlembicScene::readTransformAsMatrix( double time ) const
{
	return reader()->transformAtTime( time );
}

void AlembicScene::writeTransform( const IECore::Data *transform, double time )
{
	writer()->writeTransform( transform, time );
}

// Attributes
// ==========

bool AlembicScene::hasAttribute( const Name &name ) const
{
	return reader()->hasAttribute( name );
}

void AlembicScene::attributeNames( NameList &attrs ) const
{
	reader()->attributeNames( attrs );
}

size_t AlembicScene::numAttributeSamples( const Name &name ) const
{
	return reader()->numAttributeSamples( name );
}

double AlembicScene::attributeSampleTime( const Name &name, size_t sampleIndex ) const
{
	return reader()->attributeSampleTime( name, sampleIndex );
}

double AlembicScene::attributeSampleInterval( const Name &name, double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return reader()->attributeSampleInterval( name, time, floorIndex, ceilIndex );
}

IECore::ConstObjectPtr AlembicScene::readAttributeAtSample( const Name &name, size_t sampleIndex ) const
{
	return reader()->readAttributeAtSample( name, sampleIndex );
}

void AlembicScene::writeAttribute( const Name &name, const IECore::Object *attribute, double time )
{
	writer()->writeAttribute( name, attribute, time );
}

// Tags
// ====

bool AlembicScene::hasTag( const Name &name, int filter ) const
{
	return false;
}

void AlembicScene::readTags( NameList &tags, int filter ) const
{
	/// \todo Implement using AbcCollection. This may be better achieved
	/// if we abandon tags and use Gaffer style sets instead.
	tags.clear();
}

void AlembicScene::writeTags( const NameList &tags )
{
	IECore::msg( IECore::Msg::Warning, "AlembicScene::writeTags", "Not implemented" );
}


// Sets
// ====

SceneInterface::NameList AlembicScene::setNames( bool includeDescendantSets ) const
{
	return reader()->setNames( includeDescendantSets );
}

IECore::PathMatcher AlembicScene::readSet( const Name &name, bool includeDescendantSets ) const
{
	return reader()->readSet( name, includeDescendantSets );
}

void AlembicScene::writeSet( const Name &name, const IECore::PathMatcher &set )
{
	writer()->writeSet( name, set );
}

void AlembicScene::hashSet( const Name& setName, IECore::MurmurHash &h ) const
{
	SampledSceneInterface::hashSet( setName, h );

	reader()->hashSet( setName, h);
}

// Object
// ======

bool AlembicScene::hasObject() const
{
	return reader()->hasObject();
}

size_t AlembicScene::numObjectSamples() const
{
	return reader()->numObjectSamples();
}

double AlembicScene::objectSampleTime( size_t sampleIndex ) const
{
	return reader()->objectSampleTime( sampleIndex );
}

double AlembicScene::objectSampleInterval( double time, size_t &floorIndex, size_t &ceilIndex ) const
{
	return reader()->objectSampleInterval( time, floorIndex, ceilIndex );
}

IECore::ConstObjectPtr AlembicScene::readObjectAtSample( size_t sampleIndex ) const
{
	return reader()->objectAtSample( sampleIndex );
}

IECoreScene::PrimitiveVariableMap AlembicScene::readObjectPrimitiveVariables( const std::vector<IECore::InternedString> &primVarNames, double time ) const
{
	/// \todo I cannot find a single use of this function anywhere, but we've had to implement
	/// it for no end of SceneInterface classes. Can we just remove it?
	throw IECore::NotImplementedException( "AlembicScene::readObjectPrimitiveVariables" );
}

void AlembicScene::writeObject( const IECore::Object *object, double time )
{
	return writer()->writeObject( object, time );
}

// Hierarchy
// =========

bool AlembicScene::hasChild( const Name &name ) const
{
	return static_cast<bool>( m_io->child( name, SceneInterface::NullIfMissing ) );
}

void AlembicScene::childNames( NameList &childNames ) const
{
	m_io->childNames( childNames );
}

IECoreScene::SceneInterfacePtr AlembicScene::child( const Name &name, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour )
{
	AlembicIOPtr child = m_io->child( name, missingBehaviour );
	if( !child )
	{
		return nullptr;
	}
	return new AlembicScene( m_root, child );
}

IECoreScene::ConstSceneInterfacePtr AlembicScene::child( const Name &name, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour ) const
{
	if( missingBehaviour == CreateIfMissing )
	{
		throw IECore::Exception( "Cannot create child from const method" );
	}

	AlembicIOPtr child = m_io->child( name, missingBehaviour );
	if( !child )
	{
		return nullptr;
	}
	return new AlembicScene( m_root, child );
}

IECoreScene::SceneInterfacePtr AlembicScene::createChild( const Name &name )
{
	AlembicWriter *writer = this->writer();
	if( writer->child( name, NullIfMissing ) )
	{
		throw IECore::Exception( "Child already exists" );
	}
	return new AlembicScene( m_root, writer->child( name, CreateIfMissing ) );
}

IECoreScene::SceneInterfacePtr AlembicScene::scene( const Path &path, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour )
{
	AlembicIOPtr io = m_root;
	for( const Name &name : path )
	{
		io = io->child( name, missingBehaviour );
		if( !io )
		{
			return nullptr;
		}
	}
	return new AlembicScene( m_root, io );
}

IECoreScene::ConstSceneInterfacePtr AlembicScene::scene( const Path &path, IECoreScene::SceneInterface::MissingBehaviour missingBehaviour ) const
{
	return const_cast<AlembicScene *>( this )->scene( path, missingBehaviour );
}

void AlembicScene::hash( HashType hashType, double time, IECore::MurmurHash &h ) const
{
	SampledSceneInterface::hash( hashType, time, h );
	h.append( hashType );

	switch( hashType )
	{
		case BoundHash :
			reader()->boundHash( time, h );
			break;
		case TransformHash :
			reader()->transformHash( time, h );
			break;
		case AttributesHash :
			reader()->attributeHash( time, h );
			break;
		case ObjectHash :
			reader()->objectHash( time, h );
			break;
		case ChildNamesHash :
			reader()->childNamesHash( h );
			break;
		case HierarchyHash :
			reader()->hierarchyHash( time, h );
			break;
	}
}

const AlembicScene::AlembicReader *AlembicScene::reader() const
{
	const AlembicReader *reader = dynamic_cast<const AlembicReader *>( m_io.get() );
	if( !reader )
	{
		throw IECore::Exception( "Function not available when writing" );
	}
	return reader;
}

AlembicScene::AlembicWriter *AlembicScene::writer()
{
	AlembicWriter *writer = dynamic_cast<AlembicWriter *>( m_io.get() );
	if( !writer )
	{
		throw IECore::Exception( "Function not available when reading" );
	}
	return writer;
}

namespace
{

IECoreScene::SceneInterface::FileFormatDescription<AlembicScene> g_description( ".abc", IECore::IndexedIO::Read | IECore::IndexedIO::Write );

} // namespace
