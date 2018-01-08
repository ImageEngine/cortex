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

#include "tbb/tbb_thread.h"

#include "ParticleReaderBinding.h"
#include "PDCParticleReaderBinding.h"
#include "RenderableBinding.h"
#include "VisibleRenderableBinding.h"
#include "StateRenderableBinding.h"
#include "RendererBinding.h"
#include "ParticleWriterBinding.h"
#include "PDCParticleWriterBinding.h"
#include "PrimitiveBinding.h"
#include "PrimitiveVariableBinding.h"
#include "PointsPrimitiveBinding.h"
#include "ShaderBinding.h"
#include "PrimitiveOpBinding.h"
#include "MeshPrimitiveBinding.h"
#include "MotionPrimitiveBinding.h"
#include "TransformBinding.h"
#include "MatrixTransformBinding.h"
#include "GroupBinding.h"
#include "AttributeStateBinding.h"
#include "MatrixMotionTransformBinding.h"
#include "OBJReaderBinding.h"
#include "PointNormalsOpBinding.h"
#include "PointDensitiesOpBinding.h"
#include "PreWorldRenderableBinding.h"
#include "CameraBinding.h"
#include "NURBSPrimitiveBinding.h"
#include "PointBoundsOpBinding.h"
#include "MeshPrimitiveBuilderBinding.h"
#include "TypedPrimitiveOpBinding.h"
#include "PrimitiveEvaluatorBinding.h"
#include "MeshPrimitiveEvaluatorBinding.h"
#include "TriangulateOpBinding.h"
#include "SpherePrimitiveBinding.h"
#include "SpherePrimitiveEvaluatorBinding.h"
#include "MeshPrimitiveShrinkWrapOpBinding.h"
#include "TransformOpBinding.h"
#include "CurvesPrimitiveBinding.h"
#include "TriangulatorBinding.h"
#include "MeshNormalsOpBinding.h"
#include "MeshMergeOpBinding.h"
#include "FontBinding.h"
#include "MeshVertexReorderOpBinding.h"
#include "CoordinateSystemBinding.h"
#include "DisplayBinding.h"
#include "PatchMeshPrimitiveBinding.h"
#include "CurveExtrudeOpBinding.h"
#include "NParticleReaderBinding.h"
#include "CurvesMergeOpBinding.h"
#include "CurvesPrimitiveEvaluatorBinding.h"
#include "CurveLineariserBinding.h"
#include "CurveTangentsOpBinding.h"
#include "SmoothSkinningDataBinding.h"
#include "FaceVaryingPromotionOpBinding.h"
#include "PointVelocityDisplaceOpBinding.h"
#include "CompressSmoothSkinningDataOpBinding.h"
#include "DecompressSmoothSkinningDataOpBinding.h"
#include "ReorderSmoothSkinningInfluencesOpBinding.h"
#include "NormalizeSmoothSkinningWeightsOpBinding.h"
#include "LimitSmoothSkinningInfluencesOpBinding.h"
#include "MixSmoothSkinningWeightsOpBinding.h"
#include "SmoothSmoothSkinningWeightsOpBinding.h"
#include "PointSmoothSkinningOpBinding.h"
#include "AddSmoothSkinningInfluencesOpBinding.h"
#include "RemoveSmoothSkinningInfluencesOpBinding.h"
#include "TransferSmoothSkinningWeightsOpBinding.h"
#include "PointsPrimitiveEvaluatorBinding.h"
#include "PointsMotionOpBinding.h"
#include "LightBinding.h"
#include "ContrastSmoothSkinningWeightsOpBinding.h"
#include "DiskPrimitiveBinding.h"
#include "OptionsBinding.h"
#include "SceneInterfaceBinding.h"
#include "SharedSceneInterfacesBinding.h"
#include "SampledSceneInterfaceBinding.h"
#include "SceneCacheBinding.h"
#include "LinkedSceneBinding.h"
#include "ExternalProceduralBinding.h"
#include "ClippingPlaneBinding.h"
#include "MeshAlgoBinding.h"
#include "CurvesAlgoBinding.h"
#include "PointsAlgoBinding.h"
#include "TypedObjectParameterBinding.h"
#include "TypeIdBinding.h"

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
	bindDisplay();
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

}
