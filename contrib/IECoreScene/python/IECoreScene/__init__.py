##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

"""
Forward compatibility with Cortex 10 for IECoreScene
"""
import IECore

AddSmoothSkinningInfluencesOp = IECore.AddSmoothSkinningInfluencesOp
AttributeBlock = IECore.AttributeBlock
AttributeState = IECore.AttributeState
AttributeStateParameter = IECore.AttributeStateParameter
Camera = IECore.Camera
CameraController = IECore.CameraController
CapturingRenderer = IECore.CapturingRenderer
ClippingPlane = IECore.ClippingPlane
CompressSmoothSkinningDataOp = IECore.CompressSmoothSkinningDataOp
ContrastSmoothSkinningWeightsOp = IECore.ContrastSmoothSkinningWeightsOp
CoordinateSystem = IECore.CoordinateSystem
CurveExtrudeOp = IECore.CurveExtrudeOp
CurveLineariser = IECore.CurveLineariser
CurveTangentsOp = IECore.CurveTangentsOp
CurvesAlgo = IECore.CurvesAlgo
CurvesMergeOp = IECore.CurvesMergeOp
CurvesPrimitive = IECore.CurvesPrimitive
CurvesPrimitiveEvaluator = IECore.CurvesPrimitiveEvaluator
CurvesPrimitiveOp = IECore.CurvesPrimitiveOp
CurvesPrimitiveParameter = IECore.CurvesPrimitiveParameter
DecompressSmoothSkinningDataOp = IECore.DecompressSmoothSkinningDataOp
DiskPrimitive = IECore.DiskPrimitive
Display = IECore.Display
EditBlock = IECore.EditBlock
ExternalProcedural = IECore.ExternalProcedural
FaceVaryingPromotionOp = IECore.FaceVaryingPromotionOp
Font = IECore.Font
Group = IECore.Group
GroupParameter = IECore.GroupParameter
IDXReader = IECore.IDXReader
Light = IECore.Light
LimitSmoothSkinningInfluencesOp = IECore.LimitSmoothSkinningInfluencesOp
LinkedScene = IECore.LinkedScene
MatrixMotionTransform = IECore.MatrixMotionTransform
MatrixMotionTransformParameter = IECore.MatrixMotionTransformParameter
MatrixTransform = IECore.MatrixTransform
MatrixTransformParameter = IECore.MatrixTransformParameter
MeshAlgo = IECore.MeshAlgo
MeshMergeOp = IECore.MeshMergeOp
MeshNormalsOp = IECore.MeshNormalsOp
MeshPrimitive = IECore.MeshPrimitive
MeshPrimitiveBuilder = IECore.MeshPrimitiveBuilder
MeshPrimitiveEvaluator = IECore.MeshPrimitiveEvaluator
MeshPrimitiveOp = IECore.MeshPrimitiveOp
MeshPrimitiveParameter = IECore.MeshPrimitiveParameter
MeshPrimitiveShrinkWrapOp = IECore.MeshPrimitiveShrinkWrapOp
MeshVertexReorderOp = IECore.MeshVertexReorderOp
MixSmoothSkinningWeightsOp = IECore.MixSmoothSkinningWeightsOp
MotionBlock = IECore.MotionBlock
MotionPrimitive = IECore.MotionPrimitive
MotionPrimitiveParameter = IECore.MotionPrimitiveParameter
NParticleReader = IECore.NParticleReader
NURBSPrimitive = IECore.NURBSPrimitive
NormalizeSmoothSkinningWeightsOp = IECore.NormalizeSmoothSkinningWeightsOp
OBJReader = IECore.OBJReader
Options = IECore.Options
PDCParticleReader = IECore.PDCParticleReader
PDCParticleWriter = IECore.PDCParticleWriter
ParameterisedProcedural = IECore.ParameterisedProcedural
ParticleReader = IECore.ParticleReader
ParticleWriter = IECore.ParticleWriter
PatchMeshPrimitive = IECore.PatchMeshPrimitive
PointBoundsOp = IECore.PointBoundsOp
PointDensitiesOp = IECore.PointDensitiesOp
PointNormalsOp = IECore.PointNormalsOp
PointSmoothSkinningOp = IECore.PointSmoothSkinningOp
PointVelocityDisplaceOp = IECore.PointVelocityDisplaceOp
PointsAlgo = IECore.PointsAlgo
PointsExpressionOp = IECore.PointsExpressionOp
PointsMotionOp = IECore.PointsMotionOp
PointsPrimitive = IECore.PointsPrimitive
PointsPrimitiveEvaluator = IECore.PointsPrimitiveEvaluator
PointsPrimitiveParameter = IECore.PointsPrimitiveParameter
PreWorldRenderable = IECore.PreWorldRenderable
Primitive = IECore.Primitive
PrimitiveEvaluator = IECore.PrimitiveEvaluator
PrimitiveOp = IECore.PrimitiveOp
PrimitiveParameter = IECore.PrimitiveParameter
PrimitiveVariable = IECore.PrimitiveVariable
ReadProcedural = IECore.ReadProcedural
RemovePrimitiveVariables = IECore.RemovePrimitiveVariables
RemoveSmoothSkinningInfluencesOp = IECore.RemoveSmoothSkinningInfluencesOp
RenamePrimitiveVariables = IECore.RenamePrimitiveVariables
Renderable = IECore.Renderable
RenderableParameter = IECore.RenderableParameter
Renderer = IECore.Renderer
ReorderSmoothSkinningInfluencesOp = IECore.ReorderSmoothSkinningInfluencesOp
SWAReader = IECore.SWAReader
SampledSceneInterface = IECore.SampledSceneInterface
SceneCache = IECore.SceneCache
SceneInterface = IECore.SceneInterface
Shader = IECore.Shader
ShaderParameter = IECore.ShaderParameter
SharedSceneInterfaces = IECore.SharedSceneInterfaces
SmoothSkinningData = IECore.SmoothSkinningData
SmoothSkinningDataParameter = IECore.SmoothSkinningDataParameter
SmoothSmoothSkinningWeightsOp = IECore.SmoothSmoothSkinningWeightsOp
SpherePrimitive = IECore.SpherePrimitive
SpherePrimitiveEvaluator = IECore.SpherePrimitiveEvaluator
StateRenderable = IECore.StateRenderable
StateRenderableParameter = IECore.StateRenderableParameter
TransferSmoothSkinningWeightsOp = IECore.TransferSmoothSkinningWeightsOp
Transform = IECore.Transform
TransformBlock = IECore.TransformBlock
TransformOp = IECore.TransformOp
TransformParameter = IECore.TransformParameter
TriangulateOp = IECore.TriangulateOp
TypeId = IECore.TypeId
V3fTriangulator = IECore.V3fTriangulator
VisibleRenderable = IECore.VisibleRenderable
VisibleRenderableParameter = IECore.VisibleRenderableParameter
VisualiserProcedural = IECore.VisualiserProcedural
WorldBlock = IECore.WorldBlock
