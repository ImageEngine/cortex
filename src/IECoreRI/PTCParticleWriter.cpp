//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/PTCParticleWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/FileNameParameter.h"
#include "IECore/PointsPrimitive.h"

#include "boost/format.hpp"

#include <fstream>

using namespace IECoreRI::PTCParticleIO;
using namespace IECoreRI;
using namespace IECore;
using namespace std;
using namespace boost;

const Writer::WriterDescription<PTCParticleWriter> PTCParticleWriter::m_writerDescription( "3Dbake 3DWbake" );

PTCParticleWriter::PTCParticleWriter( )
	:	ParticleWriter( "PTCParticleWriter", "Creates files in renderman point cloud format" )
{
}

PTCParticleWriter::PTCParticleWriter( ObjectPtr object, const std::string &fileName )
	:	ParticleWriter( "PTCParticleWriter", "Creates files in renderman point cloud format" )
{
	m_objectParameter->setValue( object );
	m_fileNameParameter->setTypedValue( fileName );
}

template<class T>
static void write16DAttr( float *target, intrusive_ptr<T> attr, int index )
{
	const typename T::ValueType &v = attr->readable();
	const typename T::ValueType::value_type &item = v[index];
	target[0] = item[0][0]; target[1] = item[0][1];	target[2] = item[0][2];	target[3] = item[0][3];
	target[4] = item[1][0];	target[5] = item[1][1];	target[6] = item[1][2];	target[7] = item[1][3];
	target[8] = item[2][0];	target[9] = item[2][1];	target[10] = item[2][2];target[11] = item[2][3];
	target[12] = item[3][0];target[13] = item[3][1];target[14] = item[3][2];target[15] = item[3][3];
}

template<class T>
static void write3DAttr( float *target, intrusive_ptr<T> attr, int index )
{
	const typename T::ValueType &v = attr->readable();
	const typename T::ValueType::value_type &item = v[index];
	target[0] = item[0];
	target[1] = item[1];
	target[2] = item[2];
}

template<class T>
static void write1DAttr( float *target, intrusive_ptr<T> attr, int index )
{
	const typename T::ValueType &v = attr->readable();
	*target = v[index];
}

static void writeAttribute( float *target, DataPtr attr, int index )
{
	switch( attr->typeId() )
	{
		case DoubleVectorDataTypeId :
			{
				DoubleVectorDataPtr d = static_pointer_cast<DoubleVectorData>( attr );
				write1DAttr<DoubleVectorData>( target, d, index );
			}
			break;
		case FloatVectorDataTypeId :
			{
				FloatVectorDataPtr f = static_pointer_cast<FloatVectorData>( attr );
				write1DAttr<FloatVectorData>( target, f, index );
			}
			break;
			
		case V3dVectorDataTypeId :
			{
				V3dVectorDataPtr d = static_pointer_cast<V3dVectorData>( attr );
				write3DAttr<V3dVectorData>( target, d, index );
			}
			break;
		case V3fVectorDataTypeId :
			{
				V3fVectorDataPtr f = static_pointer_cast<V3fVectorData>( attr );
				write3DAttr<V3fVectorData>( target, f, index );
			}
			break;

		case M44fVectorDataTypeId :
			{
				M44fVectorDataPtr f = static_pointer_cast<M44fVectorData>( attr );
				write16DAttr<M44fVectorData>( target, f, index );
			}
			break;
		case M44dVectorDataTypeId :
			{
				M44dVectorDataPtr d = static_pointer_cast<M44dVectorData>( attr );
				write16DAttr<M44dVectorData>( target, d, index );
			}
			break;
		default :
			throw Exception( ( format( "Unable to convert type \"%s\"." ) % attr->typeName() ).str() );
	}
}

void PTCParticleWriter::doWrite()
{

	checkPTCParticleIO();

	// grab the PTCParticleIO and variableTypes map.
	CompoundDataPtr blindData = particleObject()->blindData();
	CompoundDataPtr variableTypes = 0;
	CompoundDataMap::const_iterator itBlind = blindData->readable().find( "PTCParticleIO" );
	if ( itBlind == blindData->readable().end() )
	{
		blindData = 0;
	} else {
		if ( itBlind->second->typeId() == CompoundDataTypeId )
		{
			blindData = static_pointer_cast< CompoundData >(itBlind->second);
			itBlind = blindData->readable().find( "variableTypes" );
			if ( itBlind != blindData->readable().end() )
			{
				if ( itBlind->second->typeId() == CompoundDataTypeId )
				{
					variableTypes = static_pointer_cast< CompoundData >(itBlind->second);
				}
			}
		}
		else
		{
			blindData = 0;
		}
	}

	// check if each attribute is of an appropriate type
	m_header.nPoints = 0;
	m_header.nvars = 0;
	const PrimitiveVariableMap &pv = particleObject()->variables;
	vector<string> attrNames;
	particleAttributes( attrNames );
	int nPoints;
	int dataFloats = 0;

	if ( pv.find( "P" ) == pv.end() )
	{
		throw IOException( "No attribute \"P\" in the given PointPrimitive object!" );
	}

	m_header.attributes.clear();

	for( vector<string>::const_iterator it = attrNames.begin(); it!=attrNames.end(); it++ )
	{
		const PrimitiveVariable &primVar = pv.find( *it )->second;
		DataPtr attr = primVar.data;

		if ( primVar.interpolation != PrimitiveVariable::Vertex &&
			 primVar.interpolation != PrimitiveVariable::Varying &&
			 primVar.interpolation != PrimitiveVariable::FaceVarying )
		{
			msg( Msg::Warning, "PTCParticleWriter::write", format( "Ignoring non-varying attribute \"%s\"." ) % *it );
			continue;
		}

		IECore::TypeId t = attr->typeId();
		string typeStr;
		switch( t )
		{
		case DoubleVectorDataTypeId:
			nPoints = boost::static_pointer_cast< DoubleVectorData >( attr )->readable().size();
			typeStr = "float";
			break;
		case FloatVectorDataTypeId:
			nPoints = boost::static_pointer_cast< FloatVectorData >( attr )->readable().size();
			typeStr = "float";
			break;
		case V3dVectorDataTypeId:
			nPoints = boost::static_pointer_cast< V3dVectorData >( attr )->readable().size();
			typeStr = "vector";
			break;
		case V3fVectorDataTypeId:
			nPoints = boost::static_pointer_cast< V3fVectorData >( attr )->readable().size();
			typeStr = "vector";
			break;
		case M44fVectorDataTypeId:
			nPoints = boost::static_pointer_cast< M44fVectorData >( attr )->readable().size();
			typeStr = "matrix";
			break;
		case M44dVectorDataTypeId:
			nPoints = boost::static_pointer_cast< M44dVectorData >( attr )->readable().size();
			typeStr = "matrix";
			break;
		default:
			msg( Msg::Warning, "PTCParticleWriter::write", format( "Ignoring unsupported attribute \"%s\" of type \"%s\"." ) % *it % attr->typeName() );
			continue;
		}

		if ( m_header.nPoints == 0 )
		{
			m_header.nPoints = nPoints;
		}
		else if ( m_header.nPoints != nPoints )
		{
			msg( Msg::Warning, "PTCParticleWriter::write", format( "Attribute \"%s\" size (%d) does not match the number of particles %d." ) % *it % nPoints % m_header.nPoints );
			continue;
		}

		if ( *it != "P" && *it != "N" && *it != "width" )
		{
			if ( variableTypes && (itBlind = variableTypes->readable().find( *it )) != variableTypes->readable().end() && itBlind->second->typeId() == StringDataTypeId )
			{
				string blindTypeStr = static_pointer_cast<StringData>(itBlind->second)->readable();
				if ( blindTypeStr != typeStr )
				{
					if ( typeStr == "vector" && ( blindTypeStr == "color" || blindTypeStr == "point" || blindTypeStr == "normal" ) )
					{
						typeStr = blindTypeStr;
					}
					else
					{
						msg( Msg::Warning, "PTCParticleWriter::write", format( "Changing attribute \"%s\" type from %s to %s." ) % *it % blindTypeStr % typeStr );
					}
				}
			}
			int t;
			for ( t = 0; t < VarTypeCount; t++ )
			{
				if ( ptcVariableTypes[ t ].name == typeStr )
				{
					m_header.varnames[ m_header.nvars ] = it->c_str();
					m_header.vartypes[ m_header.nvars ] = ptcVariableTypes[ t ].name.c_str();
					m_header.nvars++;
					Record r;
					r.type = (VarType)t;
					r.position = dataFloats;
					m_header.attributes[ *it ] = r;
					dataFloats += ptcVariableTypes[ t ].nFloats;

					break;
				}
			}
			if ( t == VarTypeCount )
			{
				msg( Msg::Warning, "PTCParticleWriter::write", format( "Unrecognized attribute type \"%s\"." ) % typeStr );
			}		
		}
	}
	if (m_header.nPoints != (int)particleObject()->getNumPoints() )
	{
		throw IOException( ( format( "Array sizes (%d) differ from number of points (%d)!." ) % m_header.nPoints % particleObject()->getNumPoints() ).str() );
	}

	/// worldToEye
	Imath::M44f m;
	m.makeIdentity();
	if ( blindData && (itBlind = blindData->readable().find( "worldToEye" )) != blindData->readable().end() &&
			itBlind->second->typeId() == M44fDataTypeId )
	{
		m = static_pointer_cast< M44fData >( itBlind->second )->readable();
	}
	int k = 0;
	for ( int i = 0; i < 4; i++ )
		for ( int j = 0; j < 4; j++, k++ )
			m_header.world2eye[k] = m[i][j];

	/// worldToNdc
	m.makeIdentity();
	if ( blindData && (itBlind = blindData->readable().find( "worldToNdc" )) != blindData->readable().end() &&
			itBlind->second->typeId() == M44fDataTypeId )
	{
		
		m = static_pointer_cast< M44fData >( itBlind->second )->readable();
	}
	k = 0;
	for ( int i = 0; i < 4; i++ )
		for ( int j = 0; j < 4; j++, k++ )
			m_header.world2ndc[k] = m[i][j];

	/// format
	m_header.format[0] = m_header.format[1] = m_header.format[2] = 1.0;
	if ( blindData )
	{
		CompoundDataMap::const_iterator end = blindData->readable().end();
		CompoundDataMap::const_iterator xResolutionIt = blindData->readable().find( "xResolution" );
		CompoundDataMap::const_iterator yResolutionIt = blindData->readable().find( "yResolution" );
		CompoundDataMap::const_iterator aspectRatioIt = blindData->readable().find( "aspectRatio" );
		if ( xResolutionIt != end && yResolutionIt != end && aspectRatioIt != end && 
			 xResolutionIt->second->typeId() == FloatDataTypeId && yResolutionIt->second->typeId() == FloatDataTypeId &&
			aspectRatioIt->second->typeId() == FloatDataTypeId)
		{
			m_header.format[0] = static_pointer_cast< FloatData >( xResolutionIt->second )->readable();
			m_header.format[1] = static_pointer_cast< FloatData >( yResolutionIt->second )->readable();
			m_header.format[2] = static_pointer_cast< FloatData >( aspectRatioIt->second )->readable();
		}
	}

	/// Creating file
	PtcPointCloud ptcFile = PtcCreatePointCloudFile( fileName().c_str(), m_header.nvars, m_header.vartypes, m_header.varnames, &m_header.world2eye[0], &m_header.world2ndc[0], &m_header.format[0] );
	if ( !ptcFile )
	{
		throw IOException( ( format( "Unable to create file \%s\"." ) % fileName() ).str() );
	}

	/// preparing standart PTC information: point, normal and radius.
	float *userData = 0;
	float point[ 3 ];
	float normal[ 3 ];
	float radius;

	PrimitiveVariableMap::const_iterator pointsIt = pv.find( "P" );
	DataPtr pointVector = pointsIt->second.data;

	DataPtr normalVector = 0;
	DataPtr radiusVector = 0;

	PrimitiveVariableMap::const_iterator normalVectorIt = pv.find( "N" );
	if ( normalVectorIt != pv.end() )
	{
		normalVector = normalVectorIt->second.data;
	}
	else
	{
		normal[0] = 0.;
		normal[1] = 1.;
		normal[2] = 0.;
	}

	PrimitiveVariableMap::const_iterator radiusVectorIt = pv.find( "width" );
	if ( radiusVectorIt != pv.end() )
	{
		radiusVector = radiusVectorIt->second.data;
	}
	else
	{
		radius = 1.;
	}

	if ( dataFloats )
	{
		userData = new float[ dataFloats ];
	}

	DataPtr dataObj;
	int position;
	bool failed = false;

	for ( int i = 0; i < m_header.nPoints; i++ )
	{
		writeAttribute( &point[0], pointVector, i );

		if (normalVector)
		{
			writeAttribute( &normal[0], normalVector, i );
		}
		if (radiusVector)
		{
			writeAttribute( &radius, radiusVector, i );
		}
		
		map<string, Record>::const_iterator it;
		for( it=m_header.attributes.begin(); it!=m_header.attributes.end(); it++ )
		{
			dataObj = pv.find( it->first )->second.data;
			position = it->second.position;
			writeAttribute( &userData[ position ], dataObj, i );
		}

		if ( !PtcWriteDataPoint( ptcFile, point, normal, radius, userData ) )
		{
			failed = true;
			break;
		}

	}

	if ( dataFloats )
	{
		delete [] userData;
	}

	PtcFinishPointCloudFile( ptcFile );

	if (failed)
	{
		throw IOException( ( format( "Error saving data point in file \"%s\"." ) % fileName() ).str()  );
	}
}
