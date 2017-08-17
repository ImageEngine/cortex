//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/MessageHandler.h"

#include "IECoreAlembic/PrimitiveReader.h"
#include "IECoreAlembic/IGeomParamTraits.h"

using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace IECore;
using namespace IECoreAlembic;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

// Functor for setting the geometric interpretation of
// DataType based on the GeomParam type. The base version
// does nothing because not all Data classes have a
// `setInterpretation()` method. We then specialise this
// below for the GeometricTypedData classes where the
// method is available.
//
// Note that we could use `DataAlgo::setGeometricInterpretation()`
// to do this for us, but by doing it ourselves we avoid the
// overhead of the internal TypedDataDespatch that it would perform.
template<typename DataType, typename GeomParam>
struct ApplyGeometricInterpretation
{

	static void apply( DataType *data )
	{
	};

};

template<typename T, typename GeomParam>
struct ApplyGeometricInterpretation<GeometricTypedData<T>, GeomParam>
{

	static void apply( GeometricTypedData<T> *data )
	{
		data->setInterpretation( IGeomParamTraits<GeomParam>::geometricInterpretation() );
	};

};

} // namespace

//////////////////////////////////////////////////////////////////////////
// PrimitiveReader implementation
//////////////////////////////////////////////////////////////////////////

void PrimitiveReader::readArbGeomParams( const Alembic::Abc::ICompoundProperty &params, const Alembic::Abc::ISampleSelector &sampleSelector, IECore::Primitive *primitive ) const
{
	if( !params.valid() )
	{
		return;
	}

	for( size_t i = 0; i < params.getNumProperties(); ++i )
	{
		const PropertyHeader &header = params.getPropertyHeader( i );

		if( IFloatGeomParam::matches( header ) )
		{
			IFloatGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IDoubleGeomParam::matches( header ) )
		{
			IDoubleGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IV3dGeomParam::matches( header ) )
		{
			IV3dGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IInt32GeomParam::matches( header ) )
		{
			IInt32GeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IStringGeomParam::matches( header ) )
		{
			IStringGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IV2fGeomParam::matches( header ) )
		{
			IV2fGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IV3fGeomParam::matches( header ) )
		{
			IV3fGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IC3fGeomParam::matches( header ) )
		{
			IC3fGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IC4fGeomParam::matches( header ) )
		{
			IC4fGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IN3fGeomParam::matches( header ) )
		{
			IN3fGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IP3fGeomParam::matches( header ) )
		{
			IP3fGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IM44fGeomParam::matches( header ) )
		{
			IM44fGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else if( IBoolGeomParam::matches( header ) )
		{
			IBoolGeomParam p( params, header.getName() );
			readGeomParam( p, sampleSelector, primitive );
		}
		else
		{
			msg( Msg::Warning, "FromAlembicGeomBaseConverter::convertArbGeomParams", boost::format( "Param \"%s\" has unsupported type" ) % header.getName() );
		}
	}
}

template<typename T>
void PrimitiveReader::readGeomParam( const T &param, const Alembic::Abc::ISampleSelector &sampleSelector, IECore::Primitive *primitive ) const
{

	typedef typename T::prop_type::sample_ptr_type SamplePtr;
	typedef typename IGeomParamTraits<T>::DataType DataType;

	if( param.getArrayExtent() > 1 )
	{
		IECore::msg( IECore::Msg::Warning, "FromAlembicGeomBaseConverter::convertArbGeomParam", boost::format( "Param \"%s\" has unsupported array extent" ) % param.getHeader().getName() );
		return;
	}

	SamplePtr sample = param.getExpandedValue( sampleSelector ).getVals();

	typename DataType::Ptr data = new DataType();
	data->writable().resize( sample->size() );
	std::copy( sample->get(), sample->get() + sample->size(), data->writable().begin() );

	ApplyGeometricInterpretation<DataType, T>::apply( data.get() );

	PrimitiveVariable pv;
	pv.interpolation = interpolation( param.getScope() );
	pv.data = data;

	primitive->variables[param.getHeader().getName()] = pv;
}

IECore::PrimitiveVariable::Interpolation PrimitiveReader::interpolation( Alembic::AbcGeom::GeometryScope scope ) const
{
	switch( scope )
	{
		case kConstantScope :
			return PrimitiveVariable::Constant;
		case kUniformScope :
			return PrimitiveVariable::Uniform;
		case kVaryingScope :
			return PrimitiveVariable::Varying;
		case kVertexScope :
			return PrimitiveVariable::Vertex;
		case kFacevaryingScope :
			return PrimitiveVariable::FaceVarying;
		default :
			return PrimitiveVariable::Invalid;
	}
}
