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
	RendererTypeId = 108007,
	PrimitiveOpTypeId = 108008,
	ParticleReaderTypeId = 108009,
	ParticleWriterTypeId = 108010,
	MotionPrimitiveTypeId = 108011,
	TransformTypeId = 108012,
	MatrixTransformTypeId = 108013,
	MotionTransformTypeId = 108014,
	MatrixMotionTransformTypeId = 108015,
	GroupTypeId = 108016,
	AttributeStateTypeId = 108017,
	VisibleRenderableTypeId = 108018,
	StateRenderableTypeId = 108019,
	OBJReaderTypeId = 108020,
	PointNormalsOpTypeId = 108021,
	PointDensitiesOpTypeId = 108022,
	StateRenderableParameterTypeId = 108023,
	AttributeStateParameterTypeId = 108024,
	ShaderParameterTypeId = 108025,
	TransformParameterTypeId = 108026,
	MatrixMotionTransformParameterTypeId = 108027,
	MatrixTransformParameterTypeId = 108028,
	VisibleRenderableParameterTypeId = 108029,
	GroupParameterTypeId = 108030,
	MotionPrimitiveParameterTypeId = 108031,
	PrimitiveParameterTypeId = 108032,
	MeshPrimitiveParameterTypeId = 108033,
	PointsPrimitiveParameterTypeId = 108034,
	PreWorldRenderableTypeId = 108035,
	CameraTypeId = 108036,
	NURBSPrimitiveTypeId = 108037,
	PointBoundsOpTypeId = 108038,
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
	CurveExtrudeOpTypeId = 108058,
	ParameterisedProceduralTypeId = 108059,
	NParticleReaderTypeId = 108060,
	CurvesMergeOpTypeId = 108061,
	CurvesPrimitiveEvaluatorTypeId = 108062,
	CurvesPrimitiveOpTypeId = 108063,
	CurveLineariserTypeId = 108064,
	CurveTangentsOpTypeId = 108065,
	SmoothSkinningDataTypeId = 108066,
	FaceVaryingPromotionOpTypeId = 108067,
	MeshDistortionsOpTypeId = 108068,
	PointVelocityDisplaceOpTypeId = 108069,
	SmoothSkinningDataParameterTypeId = 108070,
	CompressSmoothSkinningDataOpTypeId = 108071,
	DecompressSmoothSkinningDataOpTypeId = 108072,
	NormalizeSmoothSkinningWeightsOpTypeId = 108073,
	ReorderSmoothSkinningInfluencesOpTypeId = 108074,
	RemoveSmoothSkinningInfluencesOpTypeId = 108075,
	SmoothSmoothSkinningWeightsOpTypeId = 108076,
	MixSmoothSkinningWeightsOpTypeId = 108077,
	PointSmoothSkinningOpTypeId = 108078,
	AddSmoothSkinningInfluencesOpTypeId = 108079,
	LimitSmoothSkinningInfluencesOpTypeId = 108080,
	PointsPrimitiveEvaluatorTypeId = 108081,
	PointsMotionOpTypeId = 108082,
	CapturingRendererTypeId = 108083,
	LightTypeId = 108084,
	ContrastSmoothSkinningWeightsOpTypeId = 108085,
	PointDistributionOpTypeId = 108086,
	DiskPrimitiveTypeId = 108087,
	MeshFaceFilterOpTypeId = 108088,
	OptionsTypeId = 108089,
	SceneInterfaceTypeId = 108090,
	SampledSceneInterfaceTypeId = 108091,
	LinkedSceneTypeId = 108092,
	ExternalProceduralTypeId = 108093,
	DisplayTypeId = 108094,
	SceneCacheTypeId = 108095,
	TransferSmoothSkinningWeightsOpTypeId = 108096,
	RenderableParameterTypeId = 108097,
	LastCoreSceneTypeId = 108999
};

} // namespace IECoreScene

#endif // IECORESCENE_TYPEIDS_H
