//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "pointcloud.h"

#include "IECoreRI/PTCParticleReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/FileNameParameter.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/MatrixAlgo.h"
#include "IECoreScene/ParticleReader.inl"
#include "IECoreScene/PointsPrimitive.h"

#include <algorithm>

// \todo implement all attribute types from PTC definition.

#include <fstream>
#include <cassert>

using namespace IECoreRI::PTCParticleIO;
using namespace IECoreRI;
using namespace IECoreScene;
using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( PTCParticleReader );

const Reader::ReaderDescription<PTCParticleReader> PTCParticleReader::m_readerDescription( "3Dbake 3DWbake ptc" );

PTCParticleReader::PTCParticleReader( )
	:	ParticleReader( "Reads Renderman point cloud format" ), m_ptcFile( nullptr ), m_userDataBuffer( nullptr )
{
}

PTCParticleReader::PTCParticleReader( const std::string &fileName )
	:	ParticleReader( "Reads Renderman point cloud format" ), m_ptcFile( nullptr ), m_userDataBuffer( nullptr )
{
	m_fileNameParameter->setTypedValue( fileName );
}

PTCParticleReader::~PTCParticleReader()
{
	close();
}

bool PTCParticleReader::canRead( const std::string &fileName )
{
	PtcPointCloud ptcFile;
	ptcFile = PtcSafeOpenPointCloudFile( (char *)fileName.c_str() );
	if (!ptcFile)
	{
		return false;
	}
	PtcClosePointCloudFile( ptcFile );
	return true;
}

void PTCParticleReader::close()
{
	if (m_ptcFile)
	{
		PtcClosePointCloudFile( m_ptcFile );
		m_ptcFile = nullptr;
	}
	if (m_userDataBuffer)
	{
		delete [] m_userDataBuffer;
		m_userDataBuffer = nullptr;
	}
}

bool PTCParticleReader::open()
{
	if( !m_ptcFile || m_streamFileName!=fileName() )
	{
		// sanity check...
		checkPTCParticleIO();
		close();

		m_header.valid = false;
		m_header.nvars = -1;
		m_header.attributes.clear();
		m_header.nPoints = 0;
		m_header.datasize = 0;
		for ( int i = 0; i < PTC_HEADER_BBOX_FLOATS; i++ )
		{
			m_header.bbox[ i ] = 0.0;
		}
		for ( int i = 0; i < PTC_HEADER_MATRIX_FLOATS; i++ )
		{
			m_header.world2eye[ i ] = 0.0;
			m_header.world2ndc[ i ] = 0.0;
		}
		for ( int i = 0; i < PTC_HEADER_FORMAT_FLOATS; i++ )
		{
			m_header.format[ i ] = 0.0;
		}
		m_header.hasBbox = m_header.hasWorld2eye = m_header.hasWorld2ndc = m_header.hasFormat = false;

		// This is ugly but necessary. Calling PtcOpenPointCloudFile with NULL pointers didn't failed but also didn't returned the number of variables anyways...
		/// \todo We should be using PtcSafeOpenPointCloudFile here instead. We could then clean up the stuff in the header where we
		/// use the nasty PTC_MAX_VARIABLES stuff.
		m_ptcFile = PtcOpenPointCloudFile( (char *)fileName().c_str(), &m_header.nvars, (PTCParticleIO::CharPtrPtr)m_header.vartypes, (PTCParticleIO::CharPtrPtr)m_header.varnames );
		if (!m_ptcFile)
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Could not open TRC file \"%s\"." ) % fileName() );
			return false;
		}

		if ( m_header.nvars < 0 )
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Invalid number of variables in TRC file \"%s\": %d." ) % fileName() % m_header.nvars );
			return false;
		}

		int position = 0;
		for (int i = 0; i < m_header.nvars; i++)
		{
			int c;
			for ( c = 0; c < VarTypeCount; c++ )
			{
				if ( ptcVariableTypes[ c ].name == m_header.vartypes[ i ] )
				{
					Record r;
					r.type = (VarType)c;
					r.position = position;
					m_header.attributes[ m_header.varnames[ i ] ] = r;
					position += ptcVariableTypes[ c ].nFloats;
					msg( Msg::Debug, "PTCParticleReader::open()", format( "attribute: %s  type: %s" ) % m_header.varnames[i] % m_header.vartypes[i] );
					break;
				}
			}
			if ( c == VarTypeCount )
			{
				msg( Msg::Error, "PTCParticleReader::open()", format( "Unrecognized type '%s'. Ignoring variable '%s' and subsequent variables." ) % m_header.vartypes[ i ] % m_header.varnames[ i ] );
				break;
			}
		}

		// from now on, assume the file is ok and contains all information. If we find any problem, then
		// we turn the flags off again.

		m_header.valid = true;
		m_header.hasBbox = m_header.hasWorld2eye = m_header.hasWorld2ndc = m_header.hasFormat = true;

		if (!PtcGetPointCloudInfo( m_ptcFile, "npoints", &m_header.nPoints ))
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Could not get number of particles from PTC file \"%s\"." ) % fileName() );
			m_header.valid = false;
		}

		// optional info
		if (!PtcGetPointCloudInfo( m_ptcFile, "bbox", &m_header.bbox[0] ))
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Could not get bounding box information from PTC file \"%s\"." ) % fileName() );
			m_header.hasBbox = false;
		}

		if (!PtcGetPointCloudInfo( m_ptcFile, "datasize", &m_header.datasize ))
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Could not get particle data size information from PTC file \"%s\"." ) % fileName() );
			m_header.valid = false;
		}

		m_userDataBuffer = new float[ m_header.datasize ];

		// optional info
		if (!PtcGetPointCloudInfo( m_ptcFile, "world2eye", &m_header.world2eye[0] ))
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Could not get world2eye matrix from PTC file \"%s\"." ) % fileName() );
			m_header.hasWorld2eye = false;
		}

		// optional info
		if (!PtcGetPointCloudInfo( m_ptcFile, "world2ndc", &m_header.world2ndc[0] ))
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Could not get world2ndc matrix from PTC file \"%s\"." ) % fileName() );
			m_header.hasWorld2ndc = false;
		}

		// optional info
		if (!PtcGetPointCloudInfo( m_ptcFile, "format", &m_header.format[0] ))
		{
			msg( Msg::Warning, "PTCParticleReader::open()", format( "Could not get format information from PTC file \"%s\"." ) % fileName() );
			// Ignore this info if it's not available...
			m_header.hasFormat = false;
		}

		m_streamFileName = fileName();
	}
	return m_ptcFile && m_header.valid;
}

unsigned long PTCParticleReader::numParticles()
{
	if( open() )
	{
		return m_header.nPoints;
	}
	return 0;
}

void PTCParticleReader::attributeNames( std::vector<std::string> &names )
{
	names.clear();
	if( open() )
	{
		// add always available attributes...
		names.push_back( "P" );
		names.push_back( "N" );
		names.push_back( "width" );
		map<string, Record>::const_iterator it;
		for( it=m_header.attributes.begin(); it!=m_header.attributes.end(); it++ )
		{
			names.push_back( it->first );
		}
	}
}

ObjectPtr PTCParticleReader::doOperation( const CompoundObject * operands )
{
	vector<string> attributes;
	particleAttributes( attributes );
	size_t nParticles = numParticles();
	PointsPrimitivePtr result = new PointsPrimitive( nParticles );

	CompoundDataPtr attributeObjects = readAttributes( attributes );
	if ( !attributeObjects )
	{
		throw Exception( ( format( "Failed to load \"%s\"." ) % fileName() ).str() );

	}

	for( vector<string>::const_iterator it = attributes.begin(); it!=attributes.end(); it++ )
	{
		CompoundDataMap::const_iterator itData = attributeObjects->readable().find( *it );
		if ( itData == attributeObjects->readable().end() )
		{
			msg( Msg::Warning, "ParticleReader::doOperation", format( "Attribute %s expected but not found." ) % *it );
			continue;
		}

		DataPtr d = itData->second;

		PrimitiveVariable::Interpolation interp = PrimitiveVariable::Invalid;
		if( despatchTraitsTest<TypeTraits::IsSimpleTypedData>( d.get() ) )
		{
			interp = PrimitiveVariable::Constant;
		}
		else if( despatchTraitsTest<TypeTraits::IsVectorTypedData>( d.get() ) )
		{
			interp = PrimitiveVariable::Vertex;
		}

		if ( interp == PrimitiveVariable::Invalid )
		{
			msg( Msg::Warning, "ParticleReader::doOperation", format( "Ignoring attribute \"%s\" due to unsupported type \"%s\"." ) % *it % d->typeName() );
		}
		else
		{
			result->variables.insert( PrimitiveVariableMap::value_type( *it, PrimitiveVariable( interp, d ) ) );
		}

	}

	// set blindData in the PointPrimitive object.
	CompoundDataPtr blindData = new CompoundData();

	if (m_header.hasBbox)
	{
		blindData->writable()[ "boundingBox" ] = new Box3fData( Imath::Box3f( Imath::V3f( m_header.bbox[0], m_header.bbox[1], m_header.bbox[2] ),
																		Imath::V3f( m_header.bbox[3], m_header.bbox[4], m_header.bbox[5] ) ) );
	}
	if (m_header.hasWorld2eye)
	{
		blindData->writable()[ "worldToEye" ] = new M44fData(
				Imath::M44f( m_header.world2eye[0], m_header.world2eye[1], m_header.world2eye[2], m_header.world2eye[3],
							m_header.world2eye[4], m_header.world2eye[5], m_header.world2eye[6], m_header.world2eye[7],
							m_header.world2eye[8], m_header.world2eye[9], m_header.world2eye[10], m_header.world2eye[11],
							m_header.world2eye[12], m_header.world2eye[13], m_header.world2eye[14], m_header.world2eye[15] ) );
	}
	if (m_header.hasWorld2ndc)
	{
		blindData->writable()[ "worldToNdc" ] = new M44fData(
				Imath::M44f( m_header.world2ndc[0], m_header.world2ndc[1], m_header.world2ndc[2], m_header.world2ndc[3],
							m_header.world2ndc[4], m_header.world2ndc[5], m_header.world2ndc[6], m_header.world2ndc[7],
							m_header.world2ndc[8], m_header.world2ndc[9], m_header.world2ndc[10], m_header.world2ndc[11],
							m_header.world2ndc[12], m_header.world2ndc[13], m_header.world2ndc[14], m_header.world2ndc[15] ) );
	}
	if (m_header.hasFormat)
	{
		blindData->writable()[ "xResolution" ] = new FloatData( m_header.format[0] );
		blindData->writable()[ "yResolution" ] = new FloatData( m_header.format[1] );
		blindData->writable()[ "aspectRatio" ] = new FloatData( m_header.format[2] );
	}

	CompoundDataPtr varTypes = new CompoundData();
	map<string, Record>::const_iterator it;
	for (it = m_header.attributes.begin(); it != m_header.attributes.end(); it++ )
	{
		varTypes->writable()[ it->first ] = new StringData( ptcVariableTypes[ it->second.type ].name.c_str() );
	}
	blindData->writable()[ "variableTypes" ] = varTypes;

	result->blindData()->writable()[ "PTCParticleIO" ] = blindData;

	return result;
}

DataPtr PTCParticleReader::readAttribute( const std::string &name )
{
	std::vector< std::string > names;
	names.push_back( name );
	CompoundDataPtr result = readAttributes( names );
	if (!result)
	{
		return nullptr;
	}
	CompoundDataMap::const_iterator it = result->readable().find( name );
	if ( it == result->readable().end() )
	{
		return nullptr;
	}
	return it->second;
}

CompoundDataPtr PTCParticleReader::readAttributes( const std::vector<std::string> &names )
{
	if( !open() )
	{
		return nullptr;
	}

	CompoundDataPtr result = new CompoundData();

	std::map< std::string, struct AttrInfo > attrInfo;

	float *point = nullptr;
	float *normal = nullptr;
	float *radius = nullptr;
	float *userData = nullptr;
	float pointBuffer[ 3 ];
	float normalBuffer[ 3 ];
	float radiusBuffer[ 1 ];

	vector<string>::const_iterator it;
	for( it=names.begin(); it!=names.end(); it++ )
	{
		const std::string &name = *it;

		float *attributePtr;
		VarType type;

		if ( name == "P" )
		{
			point = &pointBuffer[ 0 ];
			attributePtr = point;
			type = PTCParticleIO::Point;
		}
		else if ( name == "N" )
		{
			normal = &normalBuffer[ 0 ];
			attributePtr = normal;
			type = PTCParticleIO::Normal;
		}
		else if ( name == "width" )
		{
			radius = &radiusBuffer[ 0 ];
			attributePtr = radius;
			type = PTCParticleIO::Float;
		}
		else
		{
			map<string, Record>::const_iterator it = m_header.attributes.find( name );
			if( it==m_header.attributes.end() )
			{
				return nullptr;
			}

			userData = m_userDataBuffer;
			attributePtr = &userData[ it->second.position ];
			type = it->second.type;
		}

		V3fVectorDataPtr v3fVector = nullptr;
		FloatVectorDataPtr floatVector = nullptr;
		M44fVectorDataPtr matrixVector = nullptr;
		DataPtr dataVector = nullptr;

		switch( type )
		{
		case PTCParticleIO::Color:
			{
				Color3fVectorDataPtr d = new Color3fVectorData();
				d->writable().resize( numParticles() );
				dataVector = d;
				break;
			}
		case PTCParticleIO::Point:
		case PTCParticleIO::Normal:
		case PTCParticleIO::Vector:
			v3fVector = new V3fVectorData();
			v3fVector->writable().resize( numParticles() );
			dataVector = v3fVector;
			break;
		case PTCParticleIO::Float:
			floatVector = new FloatVectorData();
			floatVector->writable().resize( numParticles() );
			dataVector = floatVector;
			break;
		case PTCParticleIO::Matrix:
			matrixVector = new M44fVectorData();
			matrixVector->writable().resize( numParticles() );
			dataVector = matrixVector;
		default:
			msg( Msg::Error, "PTCParticleReader::readAttributes()", format( "Internal error. Unrecognized type '%d' loading attribute %s." ) % type % name );
			return nullptr;
		}

		AttrInfo info = {
			type,
			attributePtr,
			dataVector,
		};

		attrInfo[ name ] = info;
	}

	for ( int i = 0; i < m_header.nPoints; i++)
	{
		const float *attributePtr;
		if ( !PtcReadDataPoint( m_ptcFile, point, normal, radius, userData ) )
		{
			msg( Msg::Warning, "PTCParticleReader::readAttributes", format( "Failed to read point %d." ) % i );
			return nullptr;
		}

		std::map< std::string, struct AttrInfo >::iterator it;
		for (it = attrInfo.begin(); it != attrInfo.end(); it++)
		{
			attributePtr = it->second.sourcePtr;
			switch (it->second.targetData->typeId())
			{
			case Color3fVectorDataTypeId :
				{
					Color3f &c = static_pointer_cast<Color3fVectorData>(it->second.targetData)->writable()[ i ];
					c[0] = attributePtr[0];
					c[1] = attributePtr[1];
					c[2] = attributePtr[2];
					break;
				}
			case V3fVectorDataTypeId:
				{
					V3f &p = static_pointer_cast<V3fVectorData>(it->second.targetData)->writable()[ i ];
					p[0] = attributePtr[0];
					p[1] = attributePtr[1];
					p[2] = attributePtr[2];
					break;
				}
			case FloatVectorDataTypeId:
				static_pointer_cast<FloatVectorData>(it->second.targetData)->writable()[ i ] = attributePtr[0];
				break;
			case M44fVectorDataTypeId:
				{
					M44f &m = static_pointer_cast<M44fVectorData>(it->second.targetData)->writable()[ i ];
					m = M44f(	attributePtr[0], attributePtr[1], attributePtr[2], attributePtr[3],
								attributePtr[4], attributePtr[5], attributePtr[6], attributePtr[7],
								attributePtr[8], attributePtr[9], attributePtr[9], attributePtr[10],
								attributePtr[12], attributePtr[13], attributePtr[14], attributePtr[15]  );
					break;
				}
			default:
				msg( Msg::Error, "PTCParticleReader::readAttributes()", format( "Internal error. Unrecognized typeId '%d'." ) % it->second.targetData->typeId() );
				return nullptr;
			}
		}
	}

	/// \todo do we have ids from ptc files to be used in the filtering?
	const Data *ids = nullptr;
	DataPtr filteredData = nullptr;
	// filter and convert each attribute individually.
	std::map< std::string, struct AttrInfo >::const_iterator attrIt;
	for( attrIt=attrInfo.begin(); attrIt!=attrInfo.end(); attrIt++ )
	{
		switch( attrIt->second.type )
		{
		case PTCParticleIO::Color:
			switch( realType() )
			{
				case ParticleReader::Native :
				case ParticleReader::Float :
					filteredData = filterAttr<Color3fVectorData, Color3fVectorData>( static_cast<Color3fVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
				case ParticleReader::Double :
					filteredData = filterAttr<Color3dVectorData, Color3fVectorData>( static_cast<Color3fVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
			}
			break;
		case PTCParticleIO::Point:
		case PTCParticleIO::Normal:
		case PTCParticleIO::Vector:
			switch( realType() )
			{
				case ParticleReader::Native :
				case ParticleReader::Float :
					filteredData = filterAttr<V3fVectorData, V3fVectorData>( static_cast<V3fVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
				case ParticleReader::Double :
					filteredData = filterAttr<V3dVectorData, V3fVectorData>( static_cast<V3fVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
			}
			break;
		case PTCParticleIO::Float:
			switch( realType() )
			{
				case ParticleReader::Native :
				case ParticleReader::Float :
					filteredData = filterAttr<FloatVectorData, FloatVectorData>( static_cast<FloatVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
				case ParticleReader::Double :
					filteredData = filterAttr<DoubleVectorData, FloatVectorData>( static_cast<FloatVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
			}
			break;
		case PTCParticleIO::Matrix:
			switch( realType() )
			{
				case ParticleReader::Native :
				case ParticleReader::Float :
					filteredData = filterAttr<M44fVectorData, M44fVectorData>( static_cast<M44fVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
				case ParticleReader::Double :
					filteredData = filterAttr<M44dVectorData, M44fVectorData>( static_cast<M44fVectorData *>( attrIt->second.targetData.get() ), particlePercentage(), ids );
					break;
			}
			break;
		default:
			msg( Msg::Error, "PTCParticleReader::readAttributes()", format( "Internal error. Unrecognized type '%d' converting attribute %s." ) % attrIt->second.type	% attrIt->first );
			return nullptr;
		}
		result->writable()[attrIt->first] = filteredData;
	}
	// close the file so that this function can be called again for other attribute...
	close();

	return result;
}

std::string PTCParticleReader::positionPrimVarName()
{
	return "P";
}
