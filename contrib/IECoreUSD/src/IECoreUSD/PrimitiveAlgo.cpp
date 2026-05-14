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

#include "IECoreUSD/PrimitiveAlgo.h"

#include "IECoreUSD/AttributeAlgo.h"
#include "IECoreUSD/DataAlgo.h"

#include "IECore/DataAlgo.h"
#include "IECore/MessageHandler.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/gf/matrix3f.h"
#include "pxr/base/gf/matrix3d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/subset.h"
#include "pxr/usd/usdSkel/animQuery.h"
#include "pxr/usd/usdSkel/bindingAPI.h"
#include "pxr/usd/usdSkel/blendShapeQuery.h"
#include "pxr/usd/usdSkel/cache.h"
#include "pxr/usd/usdSkel/skeletonQuery.h"
#include "pxr/usd/usdSkel/skinningQuery.h"
#include "pxr/usd/usdSkel/root.h"
#include "pxr/usd/usdSkel/utils.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/split.hpp"

#include "fmt/ostream.h"

using namespace std;
using namespace pxr;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Writing primitive variables
//////////////////////////////////////////////////////////////////////////

namespace {

pxr::TfToken toUSDElementType( const pxr::UsdPrim &prim )
{
	if( prim.IsA<pxr::UsdGeomMesh>() )
	{
		return pxr::UsdGeomTokens->face;
	}
	return pxr::TfToken();
}

void writeGeomSubsetIndices( const pxr::TfToken &familyName, const StringVectorData *data, const IntVectorData *indicesData, const pxr::TfToken &elementType, pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time )
{
	int32_t subsetNameIdx = 0;
	pxr::TfToken familyType = pxr::UsdGeomTokens->partition;

	for( const std::string &subsetName : data->readable() )
	{
		if( subsetName.empty() && subsetNameIdx == 0 )
		{
			familyType = pxr::UsdGeomTokens->nonOverlapping;
			subsetNameIdx++;
			continue;
		}
		pxr::VtIntArray usdIndices;
		int32_t idx = 0;
		for( const int32_t &faceIdx : indicesData->readable() )
		{
			if( faceIdx == subsetNameIdx )
			{
				usdIndices.push_back( idx );
			}
			idx++;
		}
		pxr::UsdGeomSubset::CreateGeomSubset( pointBased, pxr::TfToken( subsetName ), elementType, usdIndices, familyName, familyType );
		++subsetNameIdx;
	}
}

}; // namespace

void IECoreUSD::PrimitiveAlgo::writePrimitiveVariable( const IECoreScene::PrimitiveVariable &primitiveVariable, pxr::UsdGeomPrimvar &primVar, pxr::UsdTimeCode time )
{
	const pxr::TfToken usdInterpolation = toUSD( primitiveVariable.interpolation );
	if( !usdInterpolation.IsEmpty() )
	{
		primVar.SetInterpolation( usdInterpolation );
	}
	else
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "Invalid Interpolation for {}", primVar.GetPrimvarName().GetString() );
	}

	if ( usdInterpolation == pxr::UsdGeomTokens->constant )
	{
		primVar.GetAttr().SetMetadata( AttributeAlgo::cortexPrimitiveVariableMetadataToken(), true );
	}

	const pxr::VtValue value = DataAlgo::toUSD( primitiveVariable.data.get(), /* arrayRequired = */ primVar.GetAttr().GetTypeName().IsArray() );
	primVar.Set( value, time );

	if( primitiveVariable.indices )
	{
		primVar.SetIndices( DataAlgo::toUSD( primitiveVariable.indices.get() ).Get<pxr::VtIntArray>() );
	}
}

void IECoreUSD::PrimitiveAlgo::writePrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, const pxr::UsdGeomPrimvarsAPI &primvarsAPI, pxr::UsdTimeCode time )
{
	if( name == "uv" && runTimeCast<const V2fVectorData>( primitiveVariable.data ) )
	{
		writePrimitiveVariable( "st", primitiveVariable, primvarsAPI, time );
		return;
	}
	else if( name == "N" && runTimeCast<const V3fVectorData>( primitiveVariable.data ) )
	{
		writePrimitiveVariable( "normals", primitiveVariable, primvarsAPI, time );
		return;
	}

	const pxr::SdfValueTypeName valueTypeName = DataAlgo::valueTypeName( primitiveVariable.data.get() );
	pxr::UsdGeomPrimvar usdPrimVar = primvarsAPI.CreatePrimvar( pxr::TfToken( name ), valueTypeName );
	writePrimitiveVariable( primitiveVariable, usdPrimVar, time );
}

void IECoreUSD::PrimitiveAlgo::writePrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, const pxr::UsdGeomGprim &gPrim, pxr::UsdTimeCode time )
{
	if( name == "Cs" )
	{
		UsdGeomPrimvar displayColorPrimvar = gPrim.GetDisplayColorPrimvar();
		writePrimitiveVariable( primitiveVariable, displayColorPrimvar, time );
		return;
	}

	writePrimitiveVariable( name, primitiveVariable, pxr::UsdGeomPrimvarsAPI( gPrim.GetPrim() ), time );
}

void IECoreUSD::PrimitiveAlgo::writeGeomSubsets( const pxr::TfToken &familyName, const IECoreScene::PrimitiveVariable &primitiveVariable, pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time )
{
	if( !primitiveVariable.indices )
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "No index data for UsdGeomSubset family {}", familyName.GetString() );
		return;
	}

	if( pointBased.GetPrim().IsA<pxr::UsdGeomMesh>() &&
		primitiveVariable.interpolation != IECoreScene::PrimitiveVariable::Uniform )
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "Invalid Interpolation {} for UsdGeomSubset family {}", primitiveVariable.interpolation, familyName.GetString() );
		return;
	}

	const pxr::TfToken elementType = toUSDElementType( pointBased.GetPrim() );

	if( elementType.IsEmpty() )
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "Invalid elementType for GeomSubset family {}", familyName.GetString() );
		return;
	}

	if( const StringVectorData *data = static_cast<const StringVectorData *>( primitiveVariable.data.get() ) )
	{
		writeGeomSubsetIndices( familyName, data, primitiveVariable.indices.get(), elementType, pointBased, time );
	}
}

void IECoreUSD::PrimitiveAlgo::writePrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &value, pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time )
{
	if( name == "P" )
	{
		pointBased.CreatePointsAttr().Set( PrimitiveAlgo::toUSDExpanded( value ), time );
	}
	else if( name == "velocity" )
	{
		pointBased.CreateVelocitiesAttr().Set( PrimitiveAlgo::toUSDExpanded( value ), time );
	}
	else if( name == "acceleration" )
	{
		pointBased.CreateAccelerationsAttr().Set( PrimitiveAlgo::toUSDExpanded( value ), time );
	}
	else if( boost::algorithm::starts_with( name, "geomSubset:" ) )
	{
		std::vector<std::string> split;
		boost::algorithm::split( split, name, boost::algorithm::is_any_of( ":" ) );
		if( split.size() > 1 )
		{
			writeGeomSubsets( pxr::TfToken( split[1] ), value, pointBased, time );
		}
	}
	else
	{
		writePrimitiveVariable( name, value, static_cast<pxr::UsdGeomGprim &>( pointBased ), time );
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
		// Using universal reference (`&&`) for iteration for compatibility with the
		// non-standard proxy returned by `vector<bool>`.
		for( auto &&e : PrimitiveVariable::IndexedView<T>( data->readable(), &indices->readable() ) )
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

pxr::VtValue IECoreUSD::PrimitiveAlgo::toUSDExpanded( const IECoreScene::PrimitiveVariable &primitiveVariable, bool arrayRequired )
{
	if( !primitiveVariable.indices )
	{
		return DataAlgo::toUSD( primitiveVariable.data.get(), arrayRequired );
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

void addPrimitiveVariableIfValid( IECoreScene::Primitive *primitive, const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, const UsdAttribute &source )
{
	if( !primitive->isPrimitiveVariableValid( primitiveVariable ) )
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "Ignoring invalid primitive variable \"{}\"", source.GetPath().GetAsString() );
		return;
	}

	primitive->variables[name] = primitiveVariable;
}

void readPrimitiveVariable( const pxr::UsdGeomPrimvar &primVar, pxr::UsdTimeCode time, const std::string &name, IECoreScene::Primitive *primitive, bool constantAcceptsArray )
{
	IECoreScene::PrimitiveVariable::Interpolation interpolation = IECoreUSD::PrimitiveAlgo::fromUSD( primVar.GetInterpolation() );
	if( interpolation == IECoreScene::PrimitiveVariable::Invalid )
	{
		IECore::msg(IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "Invalid Interpolation on {}", primVar.GetName().GetString() );
		return;
	}

	pxr::VtValue value;
	if( !primVar.Get( &value, time ) )
	{
		return;
	}

	IECore::DataPtr data = DataAlgo::fromUSD(
		value, primVar.GetTypeName(),
		/* arrayAccepted = */ interpolation != IECoreScene::PrimitiveVariable::Constant || constantAcceptsArray
	);
	if( !data )
	{
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "PrimVar: {} type: {} not supported - skipping", primVar.GetName().GetString(), fmt::streamed( primVar.GetTypeName() ) );
		return;
	}

	pxr::VtIntArray srcIndices;
	primVar.GetIndices( &srcIndices, time );
	IECore::IntVectorDataPtr indices;
	if( !srcIndices.empty() )
	{
		indices = DataAlgo::fromUSD( srcIndices );
	}

	addPrimitiveVariableIfValid(
		primitive, name, IECoreScene::PrimitiveVariable( interpolation, data, indices ), primVar
	);
}

pxr::UsdSkelCache *skelCache()
{
	// the SkelCache is thead-safe and is documented as intended to persist
	// and be used across stages, so we use a global cache for the process.
	static pxr::UsdSkelCache *g_skelCache = new pxr::UsdSkelCache;
	return g_skelCache;
}

void applyBlendShapes( const pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time, pxr::UsdSkelSkeletonQuery &skelQuery, pxr::UsdSkelSkinningQuery &skinningQuery, pxr::VtVec3fArray &points )
{
	if( !skinningQuery.HasBlendShapes() )
	{
		return;
	}

	const pxr::UsdSkelAnimQuery &animQuery = skelQuery.GetAnimQuery();
	if( !animQuery )
	{
		return;
	}

	VtFloatArray weights;
	if( !animQuery.ComputeBlendShapeWeights( &weights, time ) )
	{
		return;
	}

	VtFloatArray weightsForPrim;
	if( skinningQuery.GetBlendShapeMapper() )
	{
		if( !skinningQuery.GetBlendShapeMapper()->Remap( weights, &weightsForPrim ) )
		{
			return;
		}
	}
	else
	{
		weightsForPrim = weights;
	}

	VtFloatArray subShapeWeights;
	VtUIntArray blendShapeIndices;
	VtUIntArray subShapeIndices;
	pxr::UsdSkelBlendShapeQuery blendShapeQuery( pxr::UsdSkelBindingAPI( pointBased.GetPrim() ) );
	if( !blendShapeQuery.ComputeSubShapeWeights( weightsForPrim, &subShapeWeights, &blendShapeIndices, &subShapeIndices ) )
	{
		return;
	}

	blendShapeQuery.ComputeDeformedPoints(
		subShapeWeights,
		blendShapeIndices,
		subShapeIndices,
		blendShapeQuery.ComputeBlendShapePointIndices(),
		blendShapeQuery.ComputeSubShapePointOffsets(),
		points
	);
}

bool computeFaceVaryingSkinnedNormals( pxr::UsdSkelSkinningQuery &skinningQuery, const pxr::VtArray<pxr::GfMatrix4d> &xforms, pxr::VtVec3fArray *normals, pxr::UsdTimeCode time, const Canceller *canceller )
{
	const pxr::UsdGeomMesh mesh( skinningQuery.GetPrim() );
	if( !mesh )
	{
		return false;
	}

	Canceller::check( canceller );
	pxr::VtIntArray faceVertexIndices;
	mesh.GetFaceVertexIndicesAttr().Get( &faceVertexIndices, time );

	Canceller::check( canceller );
	pxr::VtIntArray jointIndices;
	pxr::VtFloatArray jointWeights;
	if( !skinningQuery.ComputeJointInfluences( &jointIndices, &jointWeights, time ) )
	{
		return false;
	}

	Canceller::check( canceller );
	pxr::VtArray<pxr::GfMatrix4d> orderedXforms = xforms;
	if( auto jointMapper = skinningQuery.GetJointMapper() )
	{
		if( !jointMapper->RemapTransforms( xforms, &orderedXforms ) )
		{
			return false;
		}
	}

	Canceller::check( canceller );
	pxr::VtArray<pxr::GfMatrix3d> invTransposeXforms( orderedXforms.size() );
	for( size_t i = 0; i < orderedXforms.size(); ++i )
	{
		invTransposeXforms[i] = orderedXforms[i].ExtractRotationMatrix().GetInverse().GetTranspose();
	}

	Canceller::check( canceller );
	return pxr::UsdSkelSkinFaceVaryingNormals(
		skinningQuery.GetSkinningMethod(),
		skinningQuery.GetGeomBindTransform( time ).ExtractRotationMatrix().GetInverse().GetTranspose(),
		invTransposeXforms, jointIndices, jointWeights,
		skinningQuery.GetNumInfluencesPerComponent(),
		faceVertexIndices, *normals
	);
}

bool readPrimitiveVariables( const pxr::UsdSkelRoot &skelRoot, const pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time, IECoreScene::Primitive *primitive, const Canceller *canceller )
{
	Canceller::check( canceller );
	pxr::UsdSkelSkeletonQuery skelQuery = ::skelCache()->GetSkelQuery( pxr::UsdSkelBindingAPI( pointBased.GetPrim() ).GetInheritedSkeleton() );
	if( !skelQuery )
	{
		return false;
	}

	Canceller::check( canceller );
	pxr::VtMatrix4dArray skinningXforms;
	if( !skelQuery.ComputeSkinningTransforms( &skinningXforms, time ) )
	{
		return false;
	}

	Canceller::check( canceller );
	::skelCache()->Populate( skelRoot, pxr::UsdTraverseInstanceProxies() );

	Canceller::check( canceller );
	pxr::UsdSkelSkinningQuery skinningQuery = ::skelCache()->GetSkinningQuery( pointBased.GetPrim() );
	if( !skinningQuery )
	{
		return false;
	}

	Canceller::check( canceller );
	pxr::VtVec3fArray points;
	if( !pointBased.GetPointsAttr().Get( &points, time ) )
	{
		return false;
	}

	// we'll consider blendshapes optional and continue skinning regardless of whether blendshapes were applied successfully
	Canceller::check( canceller );
	applyBlendShapes( pointBased, time, skelQuery, skinningQuery, points );

	Canceller::check( canceller );
	if( !skinningQuery.ComputeSkinnedPoints( skinningXforms, &points, time ) )
	{
		return false;
	}

	// The UsdSkelSkinningQuery gives us the points in skeleton space, but we have
	// computed the location transforms separately, so we transform the points by the
	// inverse bind matrix to put them back into prim-local space.
	// Note we're guessing this is correct based on the HumanFemale example from Pixar,
	// but UsdSkelBakeSkinning takes a different approach using the following formula:
	// 	`localSkinnedPoint = skelSkinnedPoint * skelLocalToWorld * inv(gprimLocalToWorld)`
	// However, the USD mechanisms to acquire those matrices are not thread-safe, and as
	// the only known example works with inverse GeomBindTransform, we're deferring the
	// issue until we have test data that requires the more complex mechanism.
	Canceller::check( canceller );
	pxr::GfMatrix4d inverseBind = skinningQuery.GetGeomBindTransform( time ).GetInverse();
	for( auto &p : points )
	{
		p = pxr::GfVec3f( inverseBind.Transform( pxr::GfVec3d( p ) ) );
	}

	Canceller::check( canceller );
	auto p = boost::static_pointer_cast<V3fVectorData>( DataAlgo::fromUSD( points ) );
	if( !p )
	{
		return false;
	}

	Canceller::check( canceller );
	p->setInterpretation( GeometricData::Point );
	addPrimitiveVariableIfValid(
		primitive, "P", IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, p ),
		pointBased.GetPointsAttr()
	);

	// Normals

	Canceller::check( canceller );
	pxr::VtVec3fArray normals;
	if( !pointBased.GetNormalsAttr().Get( &normals, time ) )
	{
		// Now that we've skinned "P", we'll always return true, regardless of
		// whether or not we can skin "N".
		return true;
	}

	const TfToken normalsInterpolation = pointBased.GetNormalsInterpolation();

	Canceller::check( canceller );
	bool normalsValid = false;
	if( normalsInterpolation == UsdGeomTokens->faceVarying )
	{
		// UsdGeomSkinningQuery doesn't support facevarying normals. But
		// there are lower-level functions we can use manually, so do that.
		normalsValid = computeFaceVaryingSkinnedNormals( skinningQuery, skinningXforms, &normals, time, canceller );
	}
	else
	{
		// UsdGeomSkinningQuery will do it all for us.
		normalsValid = skinningQuery.ComputeSkinnedNormals( skinningXforms, &normals, time );
	}

	if( normalsValid )
	{
		auto n = boost::static_pointer_cast<V3fVectorData>( DataAlgo::fromUSD( normals ) );
		n->setInterpretation( GeometricData::Normal );
		addPrimitiveVariableIfValid(
			primitive, "N", IECoreScene::PrimitiveVariable( PrimitiveAlgo::fromUSD( normalsInterpolation ), n ),
			pointBased.GetNormalsAttr()
		);
	}

	return true;
}

bool skelAnimMightBeTimeVarying( const pxr::UsdPrim &prim )
{
	pxr::UsdSkelSkeletonQuery skelQuery = ::skelCache()->GetSkelQuery( pxr::UsdSkelBindingAPI( prim ).GetInheritedSkeleton() );
	if( !skelQuery )
	{
		return false;
	}

	const pxr::UsdSkelAnimQuery &animQuery = skelQuery.GetAnimQuery();
	if( !animQuery )
	{
		return false;
	}

	return animQuery.JointTransformsMightBeTimeVarying() || animQuery.BlendShapeWeightsMightBeTimeVarying();
}

bool geomSubsetsMightBeTimeVarying( const pxr::UsdGeomPointBased &pointBased )
{
	for( const pxr::UsdGeomSubset &geomSubset : pxr::UsdGeomSubset::GetAllGeomSubsets( pointBased ) )
	{
		// We don't check ElementType here as it is considered invalid if it is TimeVarying.
		if( geomSubset.GetIndicesAttr().ValueMightBeTimeVarying() )
		{
			return true;
		}
		if( geomSubset.GetFamilyNameAttr().ValueMightBeTimeVarying() )
		{
			return true;
		}
	}
	return false;
}

void readGeomSubsetNamesAndIndices(
	const pxr::UsdGeomPointBased &pointBased,
	const pxr::TfToken &elementType,
	const pxr::TfToken &familyName,
	pxr::UsdTimeCode time,
	const IECoreScene::Primitive *primitive,
	std::vector<std::string> &geomSubsetNames,
	std::vector<int> &geomSubsetIndices,
	const Canceller *canceller
)
{
	int32_t geomSubsetIdx = 0;

	const pxr::TfToken familyType = pxr::UsdGeomSubset::GetFamilyType( pointBased, familyName );
	const pxr::VtIntArray unassignedIndices = pxr::UsdGeomSubset::GetUnassignedIndices( pointBased, elementType, familyName, time );
	if( unassignedIndices.size() && familyType == pxr::UsdGeomTokens->uniform )
	{
		// This shouldn't happen based on the ValidateFamily functions above, but leaving in here if it does happen.
		IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "FamilyName: {} familyType: {} has unassigned indices.", familyName.GetString(), familyType.GetString() );
	}

	const auto &geomSubsets = pxr::UsdGeomSubset::GetGeomSubsets( pointBased, elementType, familyName );
	geomSubsetNames.resize( unassignedIndices.size() || familyType == pxr::UsdGeomTokens->nonOverlapping ? geomSubsets.size() + 1 : geomSubsets.size() );
	geomSubsetIndices.resize( primitive->variableSize( IECoreUSD::PrimitiveAlgo::elementTypeFromUSD( elementType ) ) );

	if( unassignedIndices.size() || familyType == pxr::UsdGeomTokens->nonOverlapping )
	{
		Canceller::check( canceller );
		geomSubsetNames[geomSubsetIdx] = std::string();
		for( const int &idx : unassignedIndices )
		{
			geomSubsetIndices[idx] = geomSubsetIdx;
		}
		++geomSubsetIdx;
	}

	for( const pxr::UsdGeomSubset &geomSubset : geomSubsets )
	{
		Canceller::check( canceller );
		geomSubsetNames[geomSubsetIdx] = geomSubset.GetPrim().GetName().GetString();
		pxr::UsdAttribute indicesAttr = geomSubset.GetIndicesAttr();
		pxr::VtIntArray geomIndices;
		indicesAttr.Get( &geomIndices, time );
		for( const int &idx : geomIndices )
		{
			geomSubsetIndices[idx] = geomSubsetIdx;
		}
		++geomSubsetIdx;
	}
}

} // namespace

void IECoreUSD::PrimitiveAlgo::readPrimitiveVariables( const pxr::UsdGeomPrimvarsAPI &primvarsAPI, pxr::UsdTimeCode time, IECoreScene::Primitive *primitive, const Canceller *canceller )
{
	for( const auto &primVar : primvarsAPI.GetPrimvars() )
	{
		Canceller::check( canceller );
		string name = primVar.GetPrimvarName().GetString();

		// Ignore the UsdSkel primvars as they are not valid Cortex PrimitiveVariables.
		// The skel primvars have N elements per vertex (1 per joint the prim is bound to),
		// but Cortex only supports 1 element per Vertex.
		if( primVar.GetNamespace() == "primvars:skel" )
		{
			continue;
		}

		if( AttributeAlgo::isCortexAttribute( primVar ) )
		{
			continue;
		}

		bool constantAcceptsArray = true;
		if( name == "displayColor" )
		{
			name = "Cs";
			constantAcceptsArray = false;
		}
		::readPrimitiveVariable( primVar, time, name, primitive, constantAcceptsArray );
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

	// USD uses "normals" for normals and we use "N".

	it = primitive->variables.find( "normals" );
	if( it != primitive->variables.end() )
	{
		if( auto d = runTimeCast<V3fVectorData>( it->second.data ) )
		{
			// Force the interpretation, since some USD files
			// use `vector3f` rather than `normal3f`. I'm looking
			// at you, `arnold-usd`.
			d->setInterpretation( GeometricData::Normal );
			primitive->variables["N"] = it->second;
			primitive->variables.erase( it );
		}
	}
}

void IECoreUSD::PrimitiveAlgo::readGeomSubsets( const pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time, IECoreScene::Primitive *primitive, const Canceller *canceller )
{
	// We're only interested in a subset of element types based on UsdGeom types.
	const pxr::TfToken elementType = toUSDElementType( pointBased.GetPrim() );

	if( elementType.IsEmpty() )
	{
		if( pxr::UsdGeomSubset::GetGeomSubsets( pointBased ).size() )
		{
			IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "Prim {} with UsdGeomSubsets has unsupported elementType - skipping", pointBased.GetPrim().GetName().GetString() );
		}
		return;
	}

	// Collect all valid family names.
	std::set<pxr::TfToken> validGeomSubsetFamilyNames;

	// UsdGeomSubsets without a familyName always default to familyType unrestricted so we don't bother accessing those.
	for( const auto &geomSubset : pxr::UsdGeomSubset::GetGeomSubsets( pointBased, elementType ) )
	{
		if( !geomSubset.GetFamilyNameAttr().HasAuthoredValue() )
		{
			IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "Prim {} has UsdGeomSubsets without familyName - skipping", pointBased.GetPrim().GetName().GetString() );
			break;
		}
	}

	for( const auto &familyName : pxr::UsdGeomSubset::GetAllGeomSubsetFamilyNames( pointBased ) )
	{
		std::string reason;
		pxr::TfToken familyType = pxr::UsdGeomSubset::GetFamilyType( pointBased, familyName );
		if( familyType == pxr::UsdGeomTokens->unrestricted )
		{
			IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "UsdGeomSubset family: {} familyType: {} not supported - skipping", familyName.GetString(), familyType.GetString() );
			continue;
		}
		else if( !pxr::UsdGeomSubset::ValidateFamily( pointBased, elementType, familyName, &reason ) )
		{
			IECore::msg( IECore::MessageHandler::Level::Warning, "IECoreUSD::PrimitiveAlgo", "UsdGeomSubset family: {} not supported - {}", familyName.GetString(), reason );
			continue;
		}
		else
		{
			validGeomSubsetFamilyNames.emplace( familyName );
		}
	}

	// Convert family GeomSubsets to indexed string primitive variables under geomSubset or geomSubset:<familyName>.
	for( const auto &familyName : validGeomSubsetFamilyNames )
	{
		Canceller::check( canceller );

		std::vector<std::string> geomSubsetNames;
		std::vector<int> geomSubsetIndices;
		readGeomSubsetNamesAndIndices( pointBased, elementType, familyName, time, primitive, geomSubsetNames, geomSubsetIndices, canceller );

		const std::string geomSubsetPrimvarName = std::string( "geomSubset:" ) + familyName.GetString();

		const StringVectorDataPtr data = new StringVectorData( geomSubsetNames );
		const IntVectorDataPtr indices = new IntVectorData( geomSubsetIndices );

		// Note: Using the pointsBased points attribute for the error print log.
		addPrimitiveVariableIfValid( primitive, geomSubsetPrimvarName, IECoreScene::PrimitiveVariable( IECoreUSD::PrimitiveAlgo::elementTypeFromUSD( elementType ), data, indices ), pointBased.GetPointsAttr() );
	}
}

void IECoreUSD::PrimitiveAlgo::readPrimitiveVariables( const pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time, IECoreScene::Primitive *primitive, const Canceller *canceller )
{
	readPrimitiveVariables( pxr::UsdGeomPrimvarsAPI( pointBased.GetPrim() ), time, primitive, canceller );

	pxr::UsdSkelRoot skelRoot = pxr::UsdSkelRoot::Find( pointBased.GetPrim() );
	if( !skelRoot || !::readPrimitiveVariables( skelRoot, pointBased, time, primitive, canceller ) )
	{
		Canceller::check( canceller );
		readPrimitiveVariable( pointBased.GetPointsAttr(), time, primitive, "P" );

		Canceller::check( canceller );
		if( !primitive->variables.count( "N" ) )
		{
			// Only load `PointBased::GetNormalsAttr()` if we didn't already load `primvars:normals`.
			// From the USD API docs : "If normals and primvars:normals are both specified, the latter has precedence."
			readPrimitiveVariable( pointBased.GetNormalsAttr(), time, primitive, "N", PrimitiveAlgo::fromUSD( pointBased.GetNormalsInterpolation() ) );
		}
	}

	Canceller::check( canceller );
	readPrimitiveVariable( pointBased.GetVelocitiesAttr(), time, primitive, "velocity" );

	Canceller::check( canceller );
	readPrimitiveVariable( pointBased.GetAccelerationsAttr(), time, primitive, "acceleration" );

	readGeomSubsets( pointBased, time, primitive, canceller );
}

void IECoreUSD::PrimitiveAlgo::readPrimitiveVariable( const pxr::UsdAttribute &attribute, pxr::UsdTimeCode timeCode, IECoreScene::Primitive *primitive, const std::string &name, IECoreScene::PrimitiveVariable::Interpolation interpolation )
{
	if( auto d = DataAlgo::fromUSD( attribute, timeCode, /* arrayAccepted = */ interpolation != PrimitiveVariable::Constant ) )
	{
		addPrimitiveVariableIfValid( primitive, name, IECoreScene::PrimitiveVariable( interpolation, d ), attribute );
	}
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
		pointBased.GetAccelerationsAttr().ValueMightBeTimeVarying() ||
		primitiveVariablesMightBeTimeVarying( pxr::UsdGeomPrimvarsAPI( pointBased.GetPrim() ) ) ||
		skelAnimMightBeTimeVarying( pointBased.GetPrim() ) ||
		geomSubsetsMightBeTimeVarying( pointBased )
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

IECoreScene::PrimitiveVariable::Interpolation IECoreUSD::PrimitiveAlgo::elementTypeFromUSD( pxr::TfToken elementType )
{
	if( elementType == pxr::UsdGeomTokens->face )
	{
		return IECoreScene::PrimitiveVariable::Uniform;
	}
	return IECoreScene::PrimitiveVariable::Invalid;
}
