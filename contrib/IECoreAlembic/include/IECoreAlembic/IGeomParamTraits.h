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

#ifndef IECOREALEMBIC_IGEOMPARAMTRAITS_H
#define IECOREALEMBIC_IGEOMPARAMTRAITS_H

#include "Alembic/AbcGeom/IGeomParam.h"

#include "IECore/VectorTypedData.h"

namespace IECoreAlembic
{

/// Provides traits for mapping from AbcGeom::GeomParams to
/// equivalent Cortex types.
template<typename T>
struct IGeomParamTraits
{

	typedef typename T::prop_type::sample_type::value_vector ValueType;

	typedef IECore::TypedData<ValueType> DataType;

	static IECore::GeometricData::Interpretation geometricInterpretation()
	{
		return IECore::GeometricData::None;
	}

};

#define IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( GEOMPARAM, DATATYPE, GEOMETRICINTERPRETATION ) \
	template<> \
	struct IGeomParamTraits<GEOMPARAM> \
	{ \
		typedef DATATYPE::ValueType ValueType; \
		typedef DATATYPE DataType; \
		static IECore::GeometricData::Interpretation geometricInterpretation() \
		{ \
			return GEOMETRICINTERPRETATION; \
		} \
	};

IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IV2fGeomParam, IECore::V2fVectorData, IECore::GeometricData::Vector )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IV2dGeomParam, IECore::V2dVectorData, IECore::GeometricData::Vector )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IV2iGeomParam, IECore::V2iVectorData, IECore::GeometricData::Vector )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IV3fGeomParam, IECore::V3fVectorData, IECore::GeometricData::Vector )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IV3dGeomParam, IECore::V3dVectorData, IECore::GeometricData::Vector )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IV3iGeomParam, IECore::V3iVectorData, IECore::GeometricData::Vector )

IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IP2fGeomParam, IECore::V2fVectorData, IECore::GeometricData::Point )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IP2dGeomParam, IECore::V2dVectorData, IECore::GeometricData::Point )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IP2iGeomParam, IECore::V2iVectorData, IECore::GeometricData::Point )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IP3fGeomParam, IECore::V3fVectorData, IECore::GeometricData::Point )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IP3dGeomParam, IECore::V3dVectorData, IECore::GeometricData::Point )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IP3iGeomParam, IECore::V3iVectorData, IECore::GeometricData::Point )

IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IN2fGeomParam, IECore::V2fVectorData, IECore::GeometricData::Normal )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IN2dGeomParam, IECore::V2dVectorData, IECore::GeometricData::Normal )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IN3fGeomParam, IECore::V3fVectorData, IECore::GeometricData::Normal )
IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IN3dGeomParam, IECore::V3dVectorData, IECore::GeometricData::Normal )

IECOREALEMBIC_SPECIALISEGEOMPARAMTRAITS( Alembic::AbcGeom::IBoolGeomParam, IECore::BoolVectorData, IECore::GeometricData::None )

} // namespace IECoreAlembic

#endif // IECOREALEMBIC_IGEOMPARAMTRAITS_H
