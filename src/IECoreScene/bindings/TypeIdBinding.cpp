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

#include "boost/python.hpp"

#include "TypeIdBinding.h"

#include "IECoreScene/TypeIds.h"

#include "IECore/TypeIds.h"

using namespace boost::python;
using namespace IECoreScene;

namespace
{

struct CoreTypeIdFromSceneTypeId
{

	static void *convertible( PyObject *obj )
	{
		extract<IECoreScene::TypeId> e( obj );
		return e.check() ? obj : nullptr;
	}

	static void construct( PyObject *obj, converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ((converter::rvalue_from_python_storage<IECore::TypeId>*)data)->storage.bytes;
		const IECoreScene::TypeId typeId = extract<IECoreScene::TypeId>( obj );
		new (storage) IECore::TypeId( static_cast<IECore::TypeId>( typeId ) );
		data->convertible = storage;
	}

};

} // namespace

namespace IECoreSceneModule
{

void bindTypeId()
{
	enum_<TypeId>( "TypeId")
		.value( "Renderable", RenderableTypeId )
		.value( "Primitive", PrimitiveTypeId )
		.value( "PointsPrimitive", PointsPrimitiveTypeId )
		.value( "MeshPrimitive", MeshPrimitiveTypeId )
		.value( "Shader", ShaderTypeId )
		.value( "PDCParticleReader", PDCParticleReaderTypeId )
		.value( "PDCParticleWriter", PDCParticleWriterTypeId )
		.value( "Renderer", RendererTypeId )
		.value( "PrimitiveOp", PrimitiveOpTypeId )
		.value( "ParticleReader", ParticleReaderTypeId )
		.value( "ParticleWriter", ParticleWriterTypeId )
		.value( "MotionPrimitive", MotionPrimitiveTypeId )
		.value( "Transform", TransformTypeId )
		.value( "MatrixTransform", MatrixTransformTypeId )
		.value( "MotionTransform", MotionTransformTypeId )
		.value( "MatrixMotionTransform", MatrixMotionTransformTypeId )
		.value( "Group", GroupTypeId )
		.value( "AttributeState", AttributeStateTypeId )
		.value( "VisibleRenderable", VisibleRenderableTypeId )
		.value( "StateRenderable", StateRenderableTypeId )
		.value( "OBJReader", OBJReaderTypeId )
		.value( "PointNormalsOp", PointNormalsOpTypeId )
		.value( "PointDensitiesOp", PointDensitiesOpTypeId )
		.value( "StateRenderableParameter", StateRenderableParameterTypeId )
		.value( "AttributeStateParameter", AttributeStateParameterTypeId )
		.value( "ShaderParameter", ShaderParameterTypeId )
		.value( "TransformParameter", TransformParameterTypeId )
		.value( "MatrixMotionTransformParameter", MatrixMotionTransformParameterTypeId )
		.value( "MatrixTransformParameter", MatrixTransformParameterTypeId )
		.value( "VisibleRenderableParameter", VisibleRenderableParameterTypeId )
		.value( "GroupParameter", GroupParameterTypeId )
		.value( "MotionPrimitiveParameter", MotionPrimitiveParameterTypeId )
		.value( "PrimitiveParameter", PrimitiveParameterTypeId )
		.value( "MeshPrimitiveParameter", MeshPrimitiveParameterTypeId )
		.value( "PointsPrimitiveParameter", PointsPrimitiveParameterTypeId )
		.value( "PreWorldRenderable", PreWorldRenderableTypeId )
		.value( "Camera", CameraTypeId )
		.value( "NURBSPrimitive", NURBSPrimitiveTypeId )
		.value( "PointBoundsOp", PointBoundsOpTypeId )
		.value( "RandomRotationOp", RandomRotationOpTypeId )
		.value( "ClippingPlane", ClippingPlaneTypeId )
		.value( "MeshPrimitiveOp", MeshPrimitiveOpTypeId )
		.value( "PrimitiveEvaluator", PrimitiveEvaluatorTypeId )
		.value( "MeshPrimitiveEvaluator", MeshPrimitiveEvaluatorTypeId )
		.value( "TriangulateOp", TriangulateOpTypeId )
		.value( "SpherePrimitiveEvaluator", SpherePrimitiveEvaluatorTypeId )
		.value( "SpherePrimitive", SpherePrimitiveTypeId )
		.value( "MeshPrimitiveShrinkWrapOp", MeshPrimitiveShrinkWrapOpTypeId )
		.value( "TransformOp", TransformOpTypeId )
		.value( "CurvesPrimitive", CurvesPrimitiveTypeId )
		.value( "CoordinateSystem", CoordinateSystemTypeId )
		.value( "MeshNormalsOp", MeshNormalsOpTypeId )
		.value( "MeshMergeOp", MeshMergeOpTypeId )
		.value( "Font", FontTypeId )
		.value( "MeshVertexReorderOp", MeshVertexReorderOpTypeId )
		.value( "MeshTangentsOp", MeshTangentsOpTypeId )
		.value( "PatchMeshPrimitive", PatchMeshPrimitiveTypeId )
		.value( "CurvesPrimitiveParameter", CurvesPrimitiveParameterTypeId )
		.value( "CurveExtrudeOp", CurveExtrudeOpTypeId )
		.value( "ParameterisedProcedural", ParameterisedProceduralTypeId )
		.value( "NParticleReader", NParticleReaderTypeId )
		.value( "CurvesMergeOp", CurvesMergeOpTypeId )
		.value( "CurvesPrimitiveEvaluator", CurvesPrimitiveEvaluatorTypeId )
		.value( "CurvesPrimitiveOp", CurvesPrimitiveOpTypeId )
		.value( "CurveLineariser", CurveLineariserTypeId )
		.value( "CurveTangentsOp", CurveTangentsOpTypeId )
		.value( "SmoothSkinningData", SmoothSkinningDataTypeId )
		.value( "FaceVaryingPromotionOp", FaceVaryingPromotionOpTypeId )
		.value( "MeshDistortionsOp", MeshDistortionsOpTypeId )
		.value( "PointVelocityDisplaceOp", PointVelocityDisplaceOpTypeId )
		.value( "SmoothSkinningDataParameter", SmoothSkinningDataParameterTypeId )
		.value( "CompressSmoothSkinningDataOp", CompressSmoothSkinningDataOpTypeId )
		.value( "DecompressSmoothSkinningDataOp", DecompressSmoothSkinningDataOpTypeId )
		.value( "NormalizeSmoothSkinningWeightsOp", NormalizeSmoothSkinningWeightsOpTypeId )
		.value( "ReorderSmoothSkinningInfluencesOp", ReorderSmoothSkinningInfluencesOpTypeId )
		.value( "RemoveSmoothSkinningInfluencesOp", RemoveSmoothSkinningInfluencesOpTypeId )
		.value( "SmoothSmoothSkinningWeightsOp", SmoothSmoothSkinningWeightsOpTypeId )
		.value( "MixSmoothSkinningWeightsOp", MixSmoothSkinningWeightsOpTypeId )
		.value( "PointSmoothSkinningOp", PointSmoothSkinningOpTypeId )
		.value( "AddSmoothSkinningInfluencesOp", AddSmoothSkinningInfluencesOpTypeId )
		.value( "LimitSmoothSkinningInfluencesOp", LimitSmoothSkinningInfluencesOpTypeId )
		.value( "PointsPrimitiveEvaluator", PointsPrimitiveEvaluatorTypeId )
		.value( "PointsMotionOp", PointsMotionOpTypeId )
		.value( "CapturingRenderer", CapturingRendererTypeId )
		.value( "Light", LightTypeId )
		.value( "ContrastSmoothSkinningWeightsOp", ContrastSmoothSkinningWeightsOpTypeId )
		.value( "PointDistributionOp", PointDistributionOpTypeId )
		.value( "DiskPrimitive", DiskPrimitiveTypeId )
		.value( "MeshFaceFilterOp", MeshFaceFilterOpTypeId )
		.value( "Options", OptionsTypeId )
		.value( "SceneInterface", SceneInterfaceTypeId )
		.value( "SampledSceneInterface", SampledSceneInterfaceTypeId )
		.value( "LinkedScene", LinkedSceneTypeId )
		.value( "ExternalProcedural", ExternalProceduralTypeId )
		.value( "Output", OutputTypeId )
		.value( "SceneCache", SceneCacheTypeId )
		.value( "TransferSmoothSkinningWeightsOp", TransferSmoothSkinningWeightsOpTypeId )
		.value( "LastCoreScene", LastCoreSceneTypeId )
		.value( "ExternalProcedural", ExternalProceduralTypeId )
	;

	converter::registry::push_back(
		&CoreTypeIdFromSceneTypeId::convertible,
		&CoreTypeIdFromSceneTypeId::construct,
		type_id<IECore::TypeId>()
	);

}

} // namespace IECoreSceneModule
