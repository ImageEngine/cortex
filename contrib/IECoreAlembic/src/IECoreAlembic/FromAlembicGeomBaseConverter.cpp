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

#include "Alembic/AbcGeom/IGeomParam.h"

#include "IECore/PrimitiveVariable.h"
#include "IECore/MessageHandler.h"

#include "IECoreAlembic/FromAlembicGeomBaseConverter.h"

using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace IECore;
using namespace IECoreAlembic;

IE_CORE_DEFINERUNTIMETYPED( FromAlembicGeomBaseConverter );

FromAlembicGeomBaseConverter::FromAlembicGeomBaseConverter( const std::string &description, Alembic::Abc::IObject iGeom )
	:	FromAlembicConverter( description, iGeom )
{
}

void FromAlembicGeomBaseConverter::convertArbGeomParams( Alembic::Abc::ICompoundProperty &params, IECore::Primitive *primitive ) const
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
			convertArbGeomParam<IFloatGeomParam>( params, header, primitive );
		}
		else if( IDoubleGeomParam::matches( header ) )
		{
			convertArbGeomParam<IDoubleGeomParam>( params, header, primitive );
		}
		else if( IV3dGeomParam::matches( header ) )
		{
			convertArbGeomParam<IV3dGeomParam>( params, header, primitive );
		}
		else if( IInt32GeomParam::matches( header ) )
		{
			convertArbGeomParam<IInt32GeomParam>( params, header, primitive );
		}
		else if( IStringGeomParam::matches( header ) )
		{
			convertArbGeomParam<IStringGeomParam>( params, header, primitive );
		}
		else if( IV2fGeomParam::matches( header ) )
		{
			convertArbGeomParam<IV2fGeomParam>( params, header, primitive );
		}
		else if( IV3fGeomParam::matches( header ) )
		{
			convertArbGeomParam<IV3fGeomParam>( params, header, primitive );
		}
		else if( IC3fGeomParam::matches( header ) )
		{
			convertArbGeomParam<IC3fGeomParam>( params, header, primitive );
		}
		else if( IC4fGeomParam::matches( header ) )
		{
			convertArbGeomParam<IC4fGeomParam>( params, header, primitive );
		}
		else if( IM44fGeomParam::matches( header ) )
		{
			convertArbGeomParam<IM44fGeomParam>( params, header, primitive );
		}
		else
		{
			msg( Msg::Warning, "FromAlembicGeomBaseConverter::convertArbGeomParams", boost::format( "Param \"%s\" has unsupported type" ) % header.getName() );
		}
	}
}

IECore::PrimitiveVariable::Interpolation FromAlembicGeomBaseConverter::interpolationFromScope( Alembic::AbcGeom::GeometryScope scope ) const
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
		
template<typename T>
void FromAlembicGeomBaseConverter::convertArbGeomParam( Alembic::Abc::ICompoundProperty &params, const Alembic::Abc::PropertyHeader &paramHeader, IECore::Primitive *primitive ) const
{
	typedef typename T::prop_type::sample_ptr_type SamplePtr;
	typedef typename T::prop_type::sample_type::value_vector ValueType;
	typedef TypedData<ValueType> DataType;

	T param( params, paramHeader.getName() );
	if( param.getArrayExtent() > 1 )
	{
		IECore::msg( IECore::Msg::Warning, "FromAlembicGeomBaseConverter::convertArbGeomParam", boost::format( "Param \"%s\" has unsupported array extent" ) % paramHeader.getName() );
		return;
	}
	
	SamplePtr sample = param.getExpandedValue().getVals();
	
  	typename DataType::Ptr data = new TypedData<ValueType>();
  	data->writable().resize( sample->size() );
  	std::copy( sample->get(), sample->get() + sample->size(), data->writable().begin() );
        
	PrimitiveVariable pv;
	pv.interpolation = interpolationFromScope( param.getScope() );
	pv.data = data;
	
	primitive->variables[paramHeader.getName()] = pv;
}

