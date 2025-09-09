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

#ifndef IECORESCENE_TYPEIDS_H
#define IECORESCENE_TYPEIDS_H

namespace IECoreScene
{

enum TypeId
{
	RenderableTypeId = 108000,
	PrimitiveTypeId = 108001,
	PointsPrimitiveTypeId = 108002,
	MeshPrimitiveTypeId = 108003,
	ShaderTypeId = 108004,
	PDCParticleReaderTypeId = 108005,
	PDCParticleWriterTypeId = 108006,
	RendererTypeId = 108007, // Obsolete, available for reuse
	PrimitiveOpTypeId = 108008,
	ParticleReaderTypeId = 108009,
	ParticleWriterTypeId = 108010,
	MotionPrimitiveTypeId = 108011, // Obsolete, available for reuse
	TransformTypeId = 108012, // Obsolete, available for reuse
	MatrixTransformTypeId = 108013, // Obsolete, available for reuse
	MotionTransformTypeId = 108014, // Obsolete, available for reuse
	MatrixMotionTransformTypeId = 108015, // Obsolete, available for reuse
	GroupTypeId = 108016, // Obsolete, available for reuse
	AttributeStateTypeId = 108017, // Obsolete, available for reuse
	VisibleRenderableTypeId = 108018,
	StateRenderableTypeId = 108019, // Obsolete, available for reuse
	OBJReaderTypeId = 108020,
	PointNormalsOpTypeId = 108021, // Obsolete, available for reuse
	PointDensitiesOpTypeId = 108022, // Obsolete, available for reuse
	StateRenderableParameterTypeId = 108023, // Obsolete, available for reuse
	AttributeStateParameterTypeId = 108024, // Obsolete, available for reuse
	ShaderParameterTypeId = 108025,
	TransformParameterTypeId = 108026, // Obsolete, available for reuse
	MatrixMotionTransformParameterTypeId = 108027, // Obsolete, available for reuse
	MatrixTransformParameterTypeId = 108028, // Obsolete, available for reuse
	VisibleRenderableParameterTypeId = 108029,
	GroupParameterTypeId = 108030, // Obsolete, available for reuse
	MotionPrimitiveParameterTypeId = 108031, // Obsolete, available for reuse
	PrimitiveParameterTypeId = 108032,
	MeshPrimitiveParameterTypeId = 108033,
	PointsPrimitiveParameterTypeId = 108034,
	PreWorldRenderableTypeId = 108035, // Obsolete, available for reuse
	CameraTypeId = 108036,
	NURBSPrimitiveTypeId = 108037,
	PointBoundsOpTypeId = 108038, // Obsolete, available for reuse
	RandomRotationOpTypeId = 108039,
	ClippingPlaneTypeId = 108040,
	MeshPrimitiveOpTypeId = 108041,
	PrimitiveEvaluatorTypeId = 108042,
	MeshPrimitiveEvaluatorTypeId = 108043,
	TriangulateOpTypeId = 108044,
	SpherePrimitiveEvaluatorTypeId = 108045,
	SpherePrimitiveTypeId = 108046,
	MeshPrimitiveShrinkWrapOpTypeId = 108047,
	TransformOpTypeId = 108048,
	CurvesPrimitiveTypeId = 108049,
	CoordinateSystemTypeId = 108050,
	MeshNormalsOpTypeId = 108051,
	MeshMergeOpTypeId = 108052,
	FontTypeId = 108053,
	MeshVertexReorderOpTypeId = 108054,
	MeshTangentsOpTypeId = 108055,
	PatchMeshPrimitiveTypeId = 108056,
	CurvesPrimitiveParameterTypeId = 108057,
	CurveExtrudeOpTypeId = 108058, // Obsolete, available for reuse
	ParameterisedProceduralTypeId = 108059,
	NParticleReaderTypeId = 108060,
	CurvesMergeOpTypeId = 108061,
	CurvesPrimitiveEvaluatorTypeId = 108062,
	CurvesPrimitiveOpTypeId = 108063,
	CurveLineariserTypeId = 108064,
	CurveTangentsOpTypeId = 108065,
	SmoothSkinningDataTypeId = 108066, // Obsolete, available for reuse
	FaceVaryingPromotionOpTypeId = 108067,
	MeshDistortionsOpTypeId = 108068,
	PointVelocityDisplaceOpTypeId = 108069, // Obsolete, available for reuse
	SmoothSkinningDataParameterTypeId = 108070, // Obsolete, available for reuse
	CompressSmoothSkinningDataOpTypeId = 108071, // Obsolete, available for reuse
	DecompressSmoothSkinningDataOpTypeId = 108072, // Obsolete, available for reuse
	NormalizeSmoothSkinningWeightsOpTypeId = 108073, // Obsolete, available for reuse
	ReorderSmoothSkinningInfluencesOpTypeId = 108074, // Obsolete, available for reuse
	RemoveSmoothSkinningInfluencesOpTypeId = 108075, // Obsolete, available for reuse
	SmoothSmoothSkinningWeightsOpTypeId = 108076, // Obsolete, available for reuse
	MixSmoothSkinningWeightsOpTypeId = 108077, // Obsolete, available for reuse
	PointSmoothSkinningOpTypeId = 108078, // Obsolete, available for reuse
	AddSmoothSkinningInfluencesOpTypeId = 108079, // Obsolete, available for reuse
	LimitSmoothSkinningInfluencesOpTypeId = 108080, // Obsolete, available for reuse
	PointsPrimitiveEvaluatorTypeId = 108081,
	PointsMotionOpTypeId = 108082, // Obsolete, available for reuse
	ShaderNetworkTypeId = 108083,
	LightTypeId = 108084, // Obsolete, available for reuse
	ContrastSmoothSkinningWeightsOpTypeId = 108085, // Obsolete, available for reuse
	PointDistributionOpTypeId = 108086,
	DiskPrimitiveTypeId = 108087,
	MeshFaceFilterOpTypeId = 108088,
	OptionsTypeId = 108089, // Obsolete, available for reuse
	SceneInterfaceTypeId = 108090,
	SampledSceneInterfaceTypeId = 108091,
	LinkedSceneTypeId = 108092,
	ExternalProceduralTypeId = 108093,
	OutputTypeId = 108094,
	SceneCacheTypeId = 108095,
	TransferSmoothSkinningWeightsOpTypeId = 108096, // Obsolete, available for reuse
	RenderableParameterTypeId = 108097,
	LastCoreSceneTypeId = 108999
};

} // namespace IECoreScene

#endif // IECORESCENE_TYPEIDS_H
