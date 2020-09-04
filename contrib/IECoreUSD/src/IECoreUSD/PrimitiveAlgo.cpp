//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#include "PrimitiveAlgo.h"

#include "DataAlgo.h"

#include "IECore/DataAlgo.h"
#include "IECore/MessageHandler.h"

#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/matrix3d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/matrix4d.h"

/// \todo Use the standard PXR_VERSION instead. We can't do that until
/// everyone is using USD 19.11 though, because prior to that PXR_VERSION
/// was malformed (octal, and not comparable in any way).
#define USD_VERSION ( PXR_MAJOR_VERSION * 10000 + PXR_MINOR_VERSION * 100 + PXR_PATCH_VERSION )

using namespace std;
using namespace pxr;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Writing primitive variables
//////////////////////////////////////////////////////////////////////////

void IECoreUSD::PrimitiveAlgo::writePrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, const pxr::UsdGeomPrimvarsAPI &primvarsAPI, pxr::UsdTimeCode time )
{
	if( name == "uv" && runTimeCast<const V2fVectorData>( primitiveVariable.data ) )
	{
		writePrimitiveVariable( "st", primitiveVariable, primvarsAPI, time );
		return;
	}

	const pxr::TfToken usdInterpolation = toUSD( primitiveVariable.interpolation );
	if( usdInterpolation.IsEmpty() )
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", boost::format( "Invalid Interpolation on %1%" ) % name );
		return;
	}

	const pxr::VtValue value = DataAlgo::toUSD( primitiveVariable.data.get() );
	const pxr::SdfValueTypeName valueTypeName = DataAlgo::valueTypeName( primitiveVariable.data.get() );

	pxr::UsdGeomPrimvar usdPrimVar = primvarsAPI.CreatePrimvar( pxr::TfToken( name ), valueTypeName, usdInterpolation );
	usdPrimVar.Set( value, time );

	if( primitiveVariable.indices )
	{
		usdPrimVar.SetIndices( DataAlgo::toUSD( primitiveVariable.indices.get() ).Get<pxr::VtIntArray>() );
	}
}

void IECoreUSD::PrimitiveAlgo::writePrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &value, const pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time )
{
	if( name == "P" )
	{
		pointBased.CreatePointsAttr().Set( PrimitiveAlgo::toUSDExpanded( value ), time );
	}
	else if( name == "N" )
	{
		pointBased.CreateNormalsAttr().Set( PrimitiveAlgo::toUSDExpanded( value ), time );
	}
	else if( name == "velocity" )
	{
		pointBased.CreateVelocitiesAttr().Set( PrimitiveAlgo::toUSDExpanded( value ), time );
	}
#if USD_VERSION >= 1911
	else if( name == "acceleration" )
	{
		pointBased.CreateAccelerationsAttr().Set( PrimitiveAlgo::toUSDExpanded( value ), time );
	}
#endif
	else
	{
		writePrimitiveVariable( name, value, pxr::UsdGeomPrimvarsAPI( pointBased.GetPrim() ), time );
	}
}

namespace
{

struct VtValueFromExpandedData
{

	template<typename T>
	VtValue operator()( const IECore::TypedData<vector<T>> *data, const IECore::IntVectorData *indices, typename std::enable_if<!std::is_void<typename CortexTypeTraits<T>::USDType>::value>::type *enabler = nullptr ) const
	{
		using USDType = typename CortexTypeTraits<T>::USDType;
		using ArrayType = VtArray<USDType>;
		ArrayType array;
		array.reserve( indices->readable().size() );
		for( const auto &e : PrimitiveVariable::IndexedView<T>( data->readable(), &indices->readable() ) )
		{
			array.push_back( DataAlgo::toUSD( static_cast<const T &>( e ) ) );
		}
		return VtValue( array );
	}

	VtValue operator()( const IECore::Data *data, const IECore::IntVectorData *indices ) const
	{
		return VtValue();
	}

};

} // namespace

pxr::VtValue IECoreUSD::PrimitiveAlgo::toUSDExpanded( const IECoreScene::PrimitiveVariable &primitiveVariable )
{
	if( !primitiveVariable.indices )
	{
		return DataAlgo::toUSD( primitiveVariable.data.get() );
	}
	else
	{
		return IECore::dispatch( primitiveVariable.data.get(), VtValueFromExpandedData(), primitiveVariable.indices.get() );
	}
}

pxr::TfToken IECoreUSD::PrimitiveAlgo::toUSD( IECoreScene::PrimitiveVariable::Interpolation interpolation )
{
	switch( interpolation )
	{
		case IECoreScene::PrimitiveVariable::Constant :
			return pxr::UsdGeomTokens->constant;
		case IECoreScene::PrimitiveVariable::Uniform :
			return pxr::UsdGeomTokens->uniform;
		case IECoreScene::PrimitiveVariable::Vertex :
			return pxr::UsdGeomTokens->vertex;
		case IECoreScene::PrimitiveVariable::Varying :
			return pxr::UsdGeomTokens->varying;
		case IECoreScene::PrimitiveVariable::FaceVarying :
			return pxr::UsdGeomTokens->faceVarying;
		default :
			return pxr::TfToken();
	}
}

//////////////////////////////////////////////////////////////////////////
// Reading primitive variables
//////////////////////////////////////////////////////////////////////////

namespace
{

void readPrimitiveVariable( const pxr::UsdGeomPrimvar &primVar, pxr::UsdTimeCode time, IECoreScene::Primitive *primitive )
{
	IECoreScene::PrimitiveVariable::Interpolation interpolation = IECoreUSD::PrimitiveAlgo::fromUSD( primVar.GetInterpolation() );
	if( interpolation == IECoreScene::PrimitiveVariable::Invalid )
	{
		IECore::msg(IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", boost::format( "Invalid Interpolation on %1%" ) % primVar.GetName().GetString() );
		return;
	}

	pxr::VtValue value;
	if( !primVar.Get( &value, time ) )
	{
		return;
	}

	IECore::DataPtr data = DataAlgo::fromUSD( value, primVar.GetTypeName() );
	if( !data )
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", boost::format( "PrimVar: %1% type: %2% not supported - skipping" ) % primVar.GetName().GetString() % primVar.GetTypeName() );
		return;
	}

	pxr::VtIntArray srcIndices;
	primVar.GetIndices( &srcIndices, time );
	IECore::IntVectorDataPtr indices;
	if( !srcIndices.empty() )
	{
		indices = DataAlgo::fromUSD( srcIndices );
	}

	primitive->variables[primVar.GetPrimvarName().GetString()] = IECoreScene::PrimitiveVariable( interpolation, data, indices );
}

} // namespace

void IECoreUSD::PrimitiveAlgo::readPrimitiveVariables( const pxr::UsdGeomPrimvarsAPI &primvarsAPI, pxr::UsdTimeCode time, IECoreScene::Primitive *primitive )
{
	for( const auto &primVar : primvarsAPI.GetPrimvars() )
	{
		readPrimitiveVariable( primVar, time, primitive );
	}

	// USD uses "st" for the primary texture coordinates and we use "uv",
	// so we convert. Ironically, we used to use the "st" terminology too,
	// but moved to "uv" after years of failing to make it stick with
	// Maya users. Perhaps USD will win everyone round.

	auto it = primitive->variables.find( "st" );
	if( it != primitive->variables.end() )
	{
		if( auto d = runTimeCast<V2fVectorData>( it->second.data ) )
		{
			// Force the interpretation, since some old USD files
			// use `float[2]` rather than `texCoord2f`.
			d->setInterpretation( GeometricData::UV );
			primitive->variables["uv"] = it->second;
			primitive->variables.erase( it );
		}
	}
}

void IECoreUSD::PrimitiveAlgo::readPrimitiveVariables( const pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time, IECoreScene::Primitive *primitive )
{
	readPrimitiveVariables( pxr::UsdGeomPrimvarsAPI( pointBased.GetPrim() ), time, primitive );

	if( auto p = boost::static_pointer_cast<V3fVectorData>( DataAlgo::fromUSD( pointBased.GetPointsAttr(), time ) ) )
	{
		primitive->variables["P"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, p );
	}

	if( auto n = boost::static_pointer_cast<V3fVectorData>( DataAlgo::fromUSD( pointBased.GetNormalsAttr(), time ) ) )
	{
		primitive->variables["N"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, n );
	}

	if( auto v = boost::static_pointer_cast<V3fVectorData>( DataAlgo::fromUSD( pointBased.GetVelocitiesAttr(), time ) ) )
	{
		primitive->variables["velocity"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, v );
	}

#if USD_VERSION >= 1911
	if( auto a = boost::static_pointer_cast<V3fVectorData>( DataAlgo::fromUSD( pointBased.GetAccelerationsAttr(), time ) ) )
	{
		primitive->variables["acceleration"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, a );
	}
#endif
}

bool IECoreUSD::PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( const pxr::UsdGeomPrimvarsAPI &primvarsAPI )
{
	for( const auto &primVar : primvarsAPI.GetPrimvars() )
	{
		if( primVar.ValueMightBeTimeVarying() )
		{
			return true;
		}
	}
	return false;
}

bool IECoreUSD::PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( const pxr::UsdGeomPointBased &pointBased )
{
	return
		pointBased.GetPointsAttr().ValueMightBeTimeVarying() ||
		pointBased.GetNormalsAttr().ValueMightBeTimeVarying() ||
		pointBased.GetVelocitiesAttr().ValueMightBeTimeVarying() ||
#if USD_VERSION >= 1911
		pointBased.GetAccelerationsAttr().ValueMightBeTimeVarying() ||
#endif
		primitiveVariablesMightBeTimeVarying( pxr::UsdGeomPrimvarsAPI( pointBased.GetPrim() ) );
	;
}

IECoreScene::PrimitiveVariable::Interpolation IECoreUSD::PrimitiveAlgo::fromUSD( pxr::TfToken interpolationToken )
{
	if( interpolationToken == pxr::UsdGeomTokens->varying )
	{
		return IECoreScene::PrimitiveVariable::Varying;
	}
	if( interpolationToken == pxr::UsdGeomTokens->vertex )
	{
		return IECoreScene::PrimitiveVariable::Vertex;
	}
	if( interpolationToken == pxr::UsdGeomTokens->uniform )
	{
		return IECoreScene::PrimitiveVariable::Uniform;
	}
	if( interpolationToken == pxr::UsdGeomTokens->faceVarying )
	{
		return IECoreScene::PrimitiveVariable::FaceVarying;
	}
	if( interpolationToken == pxr::UsdGeomTokens->constant )
	{
		return IECoreScene::PrimitiveVariable::Constant;
	}

	return IECoreScene::PrimitiveVariable::Invalid;
}
