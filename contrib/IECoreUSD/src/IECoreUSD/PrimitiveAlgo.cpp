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

#include "IECore/MessageHandler.h"

using namespace IECore;

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
		pointBased.CreatePointsAttr().Set( DataAlgo::toUSD( value.data.get() ), time );
	}
	else if( name == "N" )
	{
		pointBased.CreateNormalsAttr().Set( DataAlgo::toUSD( value.data.get() ), time );
	}
	else if( name == "velocity" )
	{
		pointBased.CreateVelocitiesAttr().Set( DataAlgo::toUSD( value.data.get() ), time );
	}
	else if( name == "acceleration" )
	{
		pointBased.CreateAccelerationsAttr().Set( DataAlgo::toUSD( value.data.get() ), time );
	}
	else
	{
		writePrimitiveVariable( name, value, pxr::UsdGeomPrimvarsAPI( pointBased.GetPrim() ), time );
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
