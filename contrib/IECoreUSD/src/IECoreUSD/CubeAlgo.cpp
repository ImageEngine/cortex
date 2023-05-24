//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2023, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreUSD/ObjectAlgo.h"
#include "IECoreUSD/PrimitiveAlgo.h"

#include "IECoreScene/MeshPrimitive.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/cube.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readCube( pxr::UsdGeomCube &cube, pxr::UsdTimeCode time, const Canceller *canceller )
{
	double size = 2.0f;
	cube.GetSizeAttr().Get( &size, time );
	IECoreScene::MeshPrimitivePtr result = IECoreScene::MeshPrimitive::createBox( Box3f( V3f( -size / 2.0 ), V3f( size / 2.0 ) ) );
	PrimitiveAlgo::readPrimitiveVariables( pxr::UsdGeomPrimvarsAPI( cube.GetPrim() ), time, result.get(), canceller );
	return result;
}

bool cubeMightBeTimeVarying( pxr::UsdGeomCube &cube )
{
	return
		cube.GetSizeAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( pxr::UsdGeomPrimvarsAPI( cube.GetPrim() ) )
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomCube> g_cubeReaderDescription( pxr::TfToken( "Cube" ), readCube, cubeMightBeTimeVarying );

} // namespace
