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

#include "Alembic/AbcGeom/OGeomParam.h"

#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"

#include "IECoreAlembic/PrimitiveWriter.h"

using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;

using namespace IECore;
using namespace IECoreAlembic;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

template<typename DataType, typename GeomParamType>
void setProperty( Alembic::Abc::OArrayProperty &property, const DataType *data )
{
	typename GeomParamType::prop_type::sample_type arraySample( data->readable() );
	property.set( arraySample );
}

template<>
void setProperty<BoolVectorData, OBoolGeomParam>( Alembic::Abc::OArrayProperty &property, const BoolVectorData *data )
{
	OBoolGeomParam::prop_type::sample_type arraySample(
		OBoolGeomParam::prop_type::sample_type::value_vector( data->readable().begin(), data->readable().end() )
	);
	property.set( arraySample );
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// PrimitiveWriter implementation
//////////////////////////////////////////////////////////////////////////

void PrimitiveWriter::writeArbGeomParams( const IECore::Primitive *primitive, Alembic::Abc::OCompoundProperty &params, const char **namesToIgnore )
{
	for( const auto &p : primitive->variables )
	{
		if( namesToIgnore )
		{
			bool skip = false;
			for( const char **n = namesToIgnore; *n; n++ )
			{
				if( p.first == *n )
				{
					skip = true;
					break;
				}
			}
			if( skip )
			{
				continue;
			}
		}

 		switch( p.second.data->typeId() )
		{
			case BoolVectorDataTypeId :
				writeArbGeomParam<BoolVectorData, OBoolGeomParam>( p.first, p.second, params );
				break;
			case IntVectorDataTypeId :
				writeArbGeomParam<IntVectorData, OInt32GeomParam>( p.first, p.second, params );
				break;
			case FloatVectorDataTypeId :
				writeArbGeomParam<FloatVectorData, OFloatGeomParam>( p.first, p.second, params );
				break;
			case DoubleVectorDataTypeId :
				writeArbGeomParam<DoubleVectorData, ODoubleGeomParam>( p.first, p.second, params );
				break;
			case StringVectorDataTypeId :
				writeArbGeomParam<StringVectorData, OStringGeomParam>( p.first, p.second, params );
				break;
			case Color3fVectorDataTypeId :
				writeArbGeomParam<Color3fVectorData, OC3fGeomParam>( p.first, p.second, params );
				break;
			case Color4fVectorDataTypeId :
				writeArbGeomParam<Color4fVectorData, OC4fGeomParam>( p.first, p.second, params );
				break;
			case M44fVectorDataTypeId :
				writeArbGeomParam<M44fVectorData, OM44fGeomParam>( p.first, p.second, params );
				break;
			case V2fVectorDataTypeId :
				switch( static_cast<const V2fVectorData *>( p.second.data.get() )->getInterpretation() )
				{
					case GeometricData::Normal :
						writeArbGeomParam<V2fVectorData, ON2fGeomParam>( p.first, p.second, params );
						break;
					case GeometricData::Point :
						writeArbGeomParam<V2fVectorData, OP2fGeomParam>( p.first, p.second, params );
						break;
					default :
						writeArbGeomParam<V2fVectorData, OV2fGeomParam>( p.first, p.second, params );
				}
				break;
			case V3fVectorDataTypeId :
				switch( static_cast<const V3fVectorData *>( p.second.data.get() )->getInterpretation() )
				{
					case GeometricData::Normal :
						writeArbGeomParam<V3fVectorData, ON3fGeomParam>( p.first, p.second, params );
						break;
					case GeometricData::Point :
						writeArbGeomParam<V3fVectorData, OP3fGeomParam>( p.first, p.second, params );
						break;
					default :
						writeArbGeomParam<V3fVectorData, OV3fGeomParam>( p.first, p.second, params );
				}
				break;
			case V2dVectorDataTypeId :
				switch( static_cast<const V2dVectorData *>( p.second.data.get() )->getInterpretation() )
				{
					case GeometricData::Normal :
						writeArbGeomParam<V2dVectorData, ON2dGeomParam>( p.first, p.second, params );
						break;
					case GeometricData::Point :
						writeArbGeomParam<V2dVectorData, OP2dGeomParam>( p.first, p.second, params );
						break;
					default :
						writeArbGeomParam<V2dVectorData, OV2dGeomParam>( p.first, p.second, params );
				}
				break;
			case V3dVectorDataTypeId :
				switch( static_cast<const V3dVectorData *>( p.second.data.get() )->getInterpretation() )
				{
					case GeometricData::Normal :
						writeArbGeomParam<V3dVectorData, ON3dGeomParam>( p.first, p.second, params );
						break;
					case GeometricData::Point :
						writeArbGeomParam<V3dVectorData, OP3dGeomParam>( p.first, p.second, params );
						break;
					default :
						writeArbGeomParam<V3dVectorData, OV3dGeomParam>( p.first, p.second, params );
				}
				break;
			default :
				IECore::msg( IECore::Msg::Warning, "PrimitiveWriter::writeArbGeomParams", boost::format( "Variable \"%1%\" has unsupported type \"%2%" ) % p.first % p.second.data->typeName() );
				break;
		}
 	}
}

template<typename DataType, typename GeomParamType>
void PrimitiveWriter::writeArbGeomParam( const std::string &name, const IECore::PrimitiveVariable &primitiveVariable, Alembic::Abc::OCompoundProperty &arbGeomParams )
{
	GeomParamMap::iterator it = m_geomParams.find( name );
	if( it == m_geomParams.end() )
	{
		bool isIndexed = (primitiveVariable.indices != nullptr);

		GeomParamType geomParam(
			arbGeomParams,
			name,
			isIndexed,
			geometryScope( primitiveVariable.interpolation ),
			/* arrayExtent = */ 0
		);

		if( isIndexed )
		{
			const std::vector<int> &indexValues = primitiveVariable.indices->readable();
			std::vector<unsigned int> indices = std::vector<unsigned int>( indexValues.begin(), indexValues.end() );
			OUInt32ArrayProperty indexProperty = geomParam.getIndexProperty();
			indexProperty.set( indices );
		}

		it = m_geomParams.insert( GeomParamMap::value_type( name, geomParam.getValueProperty() ) ).first;
	}

	setProperty<DataType, GeomParamType>( it->second, static_cast<const DataType *>( primitiveVariable.data.get() ) );
}


Alembic::AbcGeom::GeometryScope PrimitiveWriter::geometryScope( IECore::PrimitiveVariable::Interpolation interpolation )
{
	switch( interpolation )
	{
		case PrimitiveVariable::Constant :
			return kConstantScope;
		case PrimitiveVariable::Uniform :
			return kUniformScope;
		case PrimitiveVariable::Varying :
			return kVaryingScope;
		case PrimitiveVariable::Vertex :
			return kVertexScope;
		case PrimitiveVariable::FaceVarying :
			return kFacevaryingScope;
		default :
			return kUnknownScope;
	}
}
