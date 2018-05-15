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

#include "AddSmoothSkinningInfluencesOpBinding.h"
#include "AttributeStateBinding.h"
#include "CameraBinding.h"
#include "ClippingPlaneBinding.h"
#include "CompressSmoothSkinningDataOpBinding.h"
#include "ContrastSmoothSkinningWeightsOpBinding.h"
#include "CoordinateSystemBinding.h"
#include "CurveExtrudeOpBinding.h"
#include "CurveLineariserBinding.h"
#include "CurveTangentsOpBinding.h"
#include "CurvesAlgoBinding.h"
#include "CurvesMergeOpBinding.h"
#include "CurvesPrimitiveBinding.h"
#include "CurvesPrimitiveEvaluatorBinding.h"
#include "DecompressSmoothSkinningDataOpBinding.h"
#include "DiskPrimitiveBinding.h"
#include "ExternalProceduralBinding.h"
#include "FaceVaryingPromotionOpBinding.h"
#include "FontBinding.h"
#include "GroupBinding.h"
#include "LightBinding.h"
#include "LimitSmoothSkinningInfluencesOpBinding.h"
#include "LinkedSceneBinding.h"
#include "MatrixMotionTransformBinding.h"
#include "MatrixTransformBinding.h"
#include "MeshAlgoBinding.h"
#include "MeshMergeOpBinding.h"
#include "MeshNormalsOpBinding.h"
#include "MeshPrimitiveBinding.h"
#include "MeshPrimitiveBuilderBinding.h"
#include "MeshPrimitiveEvaluatorBinding.h"
#include "MeshPrimitiveShrinkWrapOpBinding.h"
#include "MeshVertexReorderOpBinding.h"
#include "MixSmoothSkinningWeightsOpBinding.h"
#include "MotionPrimitiveBinding.h"
#include "NParticleReaderBinding.h"
#include "NURBSPrimitiveBinding.h"
#include "NormalizeSmoothSkinningWeightsOpBinding.h"
#include "OBJReaderBinding.h"
#include "OptionsBinding.h"
#include "OutputBinding.h"
#include "PDCParticleReaderBinding.h"
#include "PDCParticleWriterBinding.h"
#include "ParticleReaderBinding.h"
#include "ParticleWriterBinding.h"
#include "PatchMeshPrimitiveBinding.h"
#include "PointBoundsOpBinding.h"
#include "PointDensitiesOpBinding.h"
#include "PointNormalsOpBinding.h"
#include "PointSmoothSkinningOpBinding.h"
#include "PointVelocityDisplaceOpBinding.h"
#include "PointsAlgoBinding.h"
#include "PointsMotionOpBinding.h"
#include "PointsPrimitiveBinding.h"
#include "PointsPrimitiveEvaluatorBinding.h"
#include "PreWorldRenderableBinding.h"
#include "PrimitiveBinding.h"
#include "PrimitiveEvaluatorBinding.h"
#include "PrimitiveOpBinding.h"
#include "PrimitiveVariableBinding.h"
#include "RemoveSmoothSkinningInfluencesOpBinding.h"
#include "RenderableBinding.h"
#include "RendererBinding.h"
#include "ReorderSmoothSkinningInfluencesOpBinding.h"
#include "SampledSceneInterfaceBinding.h"
#include "SceneCacheBinding.h"
#include "SceneInterfaceBinding.h"
#include "ShaderBinding.h"
#include "SharedSceneInterfacesBinding.h"
#include "SmoothSkinningDataBinding.h"
#include "SmoothSmoothSkinningWeightsOpBinding.h"
#include "SpherePrimitiveBinding.h"
#include "SpherePrimitiveEvaluatorBinding.h"
#include "StateRenderableBinding.h"
#include "TransferSmoothSkinningWeightsOpBinding.h"
#include "TransformBinding.h"
#include "TransformOpBinding.h"
#include "TriangulateOpBinding.h"
#include "TriangulatorBinding.h"
#include "TypeIdBinding.h"
#include "TypedObjectParameterBinding.h"
#include "TypedPrimitiveOpBinding.h"
#include "VisibleRenderableBinding.h"
#include "SceneAlgoBinding.h"

#include "tbb/tbb_thread.h"

using namespace IECoreSceneModule;
using namespace boost::python;

// Module declaration

BOOST_PYTHON_MODULE(_IECoreScene)
{
	bindParticleReader();
	bindPDCParticleReader();
	bindRenderable();
	bindStateRenderable();
	bindVisibleRenderable();
	bindRenderer();
	bindParticleWriter();
	bindPDCParticleWriter();
	bindPrimitive();
	bindPrimitiveVariable();
	bindPointsPrimitive();
	bindShader();
	bindPrimitiveOp();
	bindMeshPrimitive();
	bindMotionPrimitive();
	bindTransform();
	bindMatrixTransform();
	bindMatrixMotionTransform();
	bindGroup();
	bindAttributeState();
	bindPointNormalsOp();
	bindPointDensitiesOp();
	bindPreWorldRenderable();
	bindCamera();
	bindNURBSPrimitive();
	bindPointBoundsOp();
	bindMeshPrimitiveBuilder();
	bindTypedPrimitiveOp();
	bindPrimitiveEvaluator();
	bindMeshPrimitiveEvaluator();
	bindTriangulateOp();
	bindSpherePrimitive();
	bindSpherePrimitiveEvaluator();
	bindMeshPrimitiveShrinkWrapOp();
	bindTransformOp();
	bindCurvesPrimitive();
	bindTriangulator();
	bindMeshNormalsOp();
	bindMeshMergeOp();

#ifdef IECORE_WITH_FREETYPE

	bindFont();

#endif

	bindMeshVertexReorderOp();
	bindCoordinateSystem();
	bindOBJReader();
	bindOutput();
	bindPatchMeshPrimitive();
	bindCurveExtrudeOp();
	bindNParticleReader();
	bindCurvesMergeOp();
	bindCurvesPrimitiveEvaluator();
	bindCurveLineariser();
	bindCurveTangentsOp();
	bindSmoothSkinningData();
	bindFaceVaryingPromotionOp();
	bindPointVelocityDisplaceOp();
	bindCompressSmoothSkinningDataOp();
	bindDecompressSmoothSkinningDataOp();
	bindReorderSmoothSkinningInfluencesOp();
	bindNormalizeSmoothSkinningWeightsOp();
	bindLimitSmoothSkinningInfluencesOp();
	bindMixSmoothSkinningWeightsOp();
	bindSmoothSmoothSkinningWeightsOp();
	bindPointSmoothSkinningOp();
	bindAddSmoothSkinningInfluencesOp();
	bindRemoveSmoothSkinningInfluencesOp();
	bindTransferSmoothSkinningWeightsOp();
	bindPointsPrimitiveEvaluator();
	bindPointsMotionOp();
	bindLight();
	bindContrastSmoothSkinningWeightsOp();
	bindDiskPrimitive();
	bindOptions();
	bindSceneInterface();
	bindSharedSceneInterfaces();
	bindSampledSceneInterface();
	bindSceneCache();
	bindLinkedScene();
	bindExternalProcedural();
	bindClippingPlane();
	bindMeshAlgo();
	bindCurvesAlgo();
	bindPointsAlgo();
	bindTypedObjectParameter();
	bindTypeId();
	bindSceneAlgo();

}
