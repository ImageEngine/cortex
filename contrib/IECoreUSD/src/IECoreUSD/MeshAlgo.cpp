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

#include "DataAlgo.h"
#include "ObjectAlgo.h"
#include "PrimitiveAlgo.h"

#include "IECoreScene/MeshPrimitive.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/mesh.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readMesh( pxr::UsdGeomMesh &mesh, pxr::UsdTimeCode time )
{
	pxr::UsdAttribute subdivSchemeAttr = mesh.GetSubdivisionSchemeAttr();

	pxr::TfToken subdivScheme;
	subdivSchemeAttr.Get( &subdivScheme );

	pxr::VtIntArray faceVertexCounts;
	mesh.GetFaceVertexCountsAttr().Get( &faceVertexCounts, time );
	IECore::IntVectorDataPtr vertexCountData = DataAlgo::fromUSD( faceVertexCounts );

	pxr::VtIntArray faceVertexIndices;
	mesh.GetFaceVertexIndicesAttr().Get( &faceVertexIndices, time  );
	IECore::IntVectorDataPtr vertexIndicesData = DataAlgo::fromUSD( faceVertexIndices );

	IECoreScene::MeshPrimitivePtr newMesh = new IECoreScene::MeshPrimitive( vertexCountData, vertexIndicesData );
	PrimitiveAlgo::readPrimitiveVariables( mesh, time, newMesh.get() );

	if( subdivScheme == pxr::UsdGeomTokens->catmullClark )
	{
		newMesh->setInterpolation( "catmullClark" );
	}

	// Corners

	pxr::VtIntArray cornerIndices;
	pxr::VtFloatArray cornerSharpnesses;
	mesh.GetCornerIndicesAttr().Get( &cornerIndices, time );
	mesh.GetCornerSharpnessesAttr().Get( &cornerSharpnesses, time );
	if( cornerIndices.size() )
	{
		IECore::IntVectorDataPtr cornerIndicesData = DataAlgo::fromUSD( cornerIndices );
		IECore::FloatVectorDataPtr cornerSharpnessesData = DataAlgo::fromUSD( cornerSharpnesses );
		newMesh->setCorners( cornerIndicesData.get(), cornerSharpnessesData.get() );
	}

	// Creases

	pxr::VtIntArray creaseLengths;
	pxr::VtIntArray creaseIndices;
	pxr::VtFloatArray creaseSharpnesses;
	mesh.GetCreaseLengthsAttr().Get( &creaseLengths, time );
	mesh.GetCreaseIndicesAttr().Get( &creaseIndices, time );
	mesh.GetCreaseSharpnessesAttr().Get( &creaseSharpnesses, time );
	if( creaseLengths.size() )
	{
		if( creaseSharpnesses.size() == creaseLengths.size() )
		{
			IECore::IntVectorDataPtr creaseLengthsData = DataAlgo::fromUSD( creaseLengths );
			IECore::IntVectorDataPtr creaseIndicesData = DataAlgo::fromUSD( creaseIndices );
			IECore::FloatVectorDataPtr creaseSharpnessesData = DataAlgo::fromUSD( creaseSharpnesses );
			newMesh->setCreases( creaseLengthsData.get(), creaseIndicesData.get(), creaseSharpnessesData.get() );
		}
		else
		{
			// USD documentation suggests that it is possible to author a sharpness per edge
			// within a single crease, rather than just a sharpness per crease. We don't know how
			// we would author one of these in practice (certainly not in Maya), and we're not sure
			// why we'd want to. For now we ignore them.
			IECore::msg( IECore::Msg::Warning, "USDScene", "Ignoring creases with varying sharpness" );
		}
	}

	return newMesh;
}

bool meshMightBeTimeVarying( pxr::UsdGeomMesh &mesh )
{
	return
		mesh.GetSubdivisionSchemeAttr().ValueMightBeTimeVarying() ||
		mesh.GetFaceVertexCountsAttr().ValueMightBeTimeVarying() ||
		mesh.GetFaceVertexIndicesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCornerIndicesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCornerSharpnessesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCreaseLengthsAttr().ValueMightBeTimeVarying() ||
		mesh.GetCreaseIndicesAttr().ValueMightBeTimeVarying() ||
		mesh.GetCreaseSharpnessesAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( mesh )
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomMesh> g_curvesReaderDescription( pxr::TfToken( "Mesh" ), readMesh, meshMightBeTimeVarying );

} // namespace