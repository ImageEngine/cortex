//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECorePython/RefCountedBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/ExceptionBinding.h"
#include "IECorePython/ImathBinding.h"
#include "IECorePython/KDTreeBinding.h"
#include "IECorePython/IndexedIOBinding.h"
#include "IECorePython/DataBinding.h"
#include "IECorePython/GeometricTypedDataBinding.h"
#include "IECorePython/SimpleTypedDataBinding.h"
#include "IECorePython/VectorTypedDataBinding.h"
#include "IECorePython/ObjectBinding.h"
#include "IECorePython/TypeIdBinding.h"
#include "IECorePython/CompoundDataBinding.h"
#include "IECorePython/MessageHandlerBinding.h"
#include "IECorePython/ReaderBinding.h"
#include "IECorePython/ParticleReaderBinding.h"
#include "IECorePython/PDCParticleReaderBinding.h"
#include "IECorePython/BlindDataHolderBinding.h"
#include "IECorePython/RenderableBinding.h"
#include "IECorePython/VisibleRenderableBinding.h"
#include "IECorePython/StateRenderableBinding.h"
#include "IECorePython/RendererBinding.h"
#include "IECorePython/WriterBinding.h"
#include "IECorePython/ParticleWriterBinding.h"
#include "IECorePython/PDCParticleWriterBinding.h"
#include "IECorePython/ParameterBinding.h"
#include "IECorePython/NumericParameterBinding.h"
#include "IECorePython/SimpleTypedParameterBinding.h"
#include "IECorePython/VectorTypedParameterBinding.h"
#include "IECorePython/SplineParameterBinding.h"
#include "IECorePython/DateTimeParameterBinding.h"
#include "IECorePython/CubeColorLookupParameterBinding.h"
#include "IECorePython/TimePeriodParameterBinding.h"
#include "IECorePython/TimeDurationParameterBinding.h"
#include "IECorePython/CompoundParameterBinding.h"
#include "IECorePython/ValidatedStringParameterBinding.h"
#include "IECorePython/PathParameterBinding.h"
#include "IECorePython/FileNameParameterBinding.h"
#include "IECorePython/DirNameParameterBinding.h"
#include "IECorePython/CompoundObjectBinding.h"
#include "IECorePython/ObjectReaderBinding.h"
#include "IECorePython/ObjectWriterBinding.h"
#include "IECorePython/PrimitiveBinding.h"
#include "IECorePython/PrimitiveVariableBinding.h"
#include "IECorePython/PointsPrimitiveBinding.h"
#include "IECorePython/TimerBinding.h"
#include "IECorePython/TurbulenceBinding.h"
#include "IECorePython/ShaderBinding.h"
#include "IECorePython/SearchPathBinding.h"
#include "IECorePython/CachedReaderBinding.h"
#include "IECorePython/ParameterisedBinding.h"
#include "IECorePython/OpBinding.h"
#include "IECorePython/ObjectParameterBinding.h"
#include "IECorePython/ModifyOpBinding.h"
#include "IECorePython/PrimitiveOpBinding.h"
#include "IECorePython/ImagePrimitiveBinding.h"
#include "IECorePython/ImageReaderBinding.h"
#include "IECorePython/ImageWriterBinding.h"
#include "IECorePython/PerlinNoiseBinding.h"
#include "IECorePython/EXRImageReaderBinding.h"
#include "IECorePython/EXRImageWriterBinding.h"
#include "IECorePython/HalfBinding.h"
#include "IECorePython/TIFFImageReaderBinding.h"
#include "IECorePython/TIFFImageWriterBinding.h"
#include "IECorePython/CINImageReaderBinding.h"
#include "IECorePython/CINImageWriterBinding.h"
#include "IECorePython/DPXImageReaderBinding.h"
#include "IECorePython/DPXImageWriterBinding.h"
#include "IECorePython/JPEGImageReaderBinding.h"
#include "IECorePython/JPEGImageWriterBinding.h"
#include "IECorePython/TGAImageReaderBinding.h"
#include "IECorePython/TGAImageWriterBinding.h"
#include "IECorePython/MeshPrimitiveBinding.h"
#include "IECorePython/MotionPrimitiveBinding.h"
#include "IECorePython/TransformBinding.h"
#include "IECorePython/MatrixTransformBinding.h"
#include "IECorePython/GroupBinding.h"
#include "IECorePython/AttributeStateBinding.h"
#include "IECorePython/MatrixMotionTransformBinding.h"
#include "IECorePython/OBJReaderBinding.h"
#include "IECorePython/NullObjectBinding.h"
#include "IECorePython/ObjectInterpolatorBinding.h"
#include "IECorePython/PointNormalsOpBinding.h"
#include "IECorePython/PointDensitiesOpBinding.h"
#include "IECorePython/TransformationMatrixBinding.h"
#include "IECorePython/TransformationMatrixDataBinding.h"
#include "IECorePython/BoundedKDTreeBinding.h"
#include "IECorePython/VectorDataFilterOpBinding.h"
#include "IECorePython/TypedObjectParameterBinding.h"
#include "IECorePython/TypedPrimitiveParameterBinding.h"
#include "IECorePython/HeaderGeneratorBinding.h"
#include "IECorePython/PreWorldRenderableBinding.h"
#include "IECorePython/CameraBinding.h"
#include "IECorePython/NURBSPrimitiveBinding.h"
#include "IECorePython/DataCastOpBinding.h"
#include "IECorePython/DataPromoteOpBinding.h"
#include "IECorePython/MatrixMultiplyOpBinding.h"
#include "IECorePython/PointBoundsOpBinding.h"
#include "IECorePython/ImathRandomBinding.h"
#include "IECorePython/RandomRotationOpBinding.h"
#include "IECorePython/MeshPrimitiveBuilderBinding.h"
#include "IECorePython/TypedPrimitiveOpBinding.h"
#include "IECorePython/PrimitiveEvaluatorBinding.h"
#include "IECorePython/MeshPrimitiveEvaluatorBinding.h"
#include "IECorePython/TriangulateOpBinding.h"
#include "IECorePython/InternedStringBinding.h"
#include "IECorePython/SpherePrimitiveBinding.h"
#include "IECorePython/SpherePrimitiveEvaluatorBinding.h"
#include "IECorePython/InverseDistanceWeightedInterpolationBinding.h"
#include "IECorePython/ImageCropOpBinding.h"
#include "IECorePython/MeshPrimitiveShrinkWrapOpBinding.h"
#include "IECorePython/ImagePrimitiveEvaluatorBinding.h"
#include "IECorePython/MathBinding.h"
#include "IECorePython/CameraControllerBinding.h"
#include "IECorePython/PathVectorParameterBinding.h"
#include "IECorePython/TriangleAlgoBinding.h"
#include "IECorePython/ColorTransformOpBinding.h"
#include "IECorePython/ConverterBinding.h"
#include "IECorePython/FromCoreConverterBinding.h"
#include "IECorePython/TransformOpBinding.h"
#include "IECorePython/LineSegmentBinding.h"
#include "IECorePython/CubicBasisBinding.h"
#include "IECorePython/CurvesPrimitiveBinding.h"
#include "IECorePython/ImageDiffOpBinding.h"
#include "IECorePython/TriangulatorBinding.h"
#include "IECorePython/BezierAlgoBinding.h"
#include "IECorePython/ToCoreConverterBinding.h"
#include "IECorePython/MeshNormalsOpBinding.h"
#include "IECorePython/PolygonAlgoBinding.h"
#include "IECorePython/MeshMergeOpBinding.h"
#include "IECorePython/FontBinding.h"
#include "IECorePython/UnicodeToStringBinding.h"
#include "IECorePython/RadixSortBinding.h"
#include "IECorePython/PointRepulsionOpBinding.h"
#include "IECorePython/AngleConversionBinding.h"
#include "IECorePython/LuminanceOpBinding.h"
#include "IECorePython/ChannelOpBinding.h"
#include "IECorePython/SummedAreaOpBinding.h"
#include "IECorePython/GradeBinding.h"
#include "IECorePython/MedianCutSamplerBinding.h"
#include "IECorePython/EnvMapSamplerBinding.h"
#include "IECorePython/MeshVertexReorderOpBinding.h"
#include "IECorePython/SplineBinding.h"
#include "IECorePython/SplineDataBinding.h"
#include "IECorePython/DisplayDriverBinding.h"
#include "IECorePython/ImageDisplayDriverBinding.h"
#include "IECorePython/CoordinateSystemBinding.h"
#include "IECorePython/ClientDisplayDriverBinding.h"
#include "IECorePython/DisplayDriverServerBinding.h"
#include "IECorePython/SplineToImageBinding.h"
#include "IECorePython/DisplayBinding.h"
#include "IECorePython/MeshTangentsOpBinding.h"
#include "IECorePython/WarpOpBinding.h"
#include "IECorePython/UVDistortOpBinding.h"
#include "IECorePython/LinearToSRGBOpBinding.h"
#include "IECorePython/SRGBToLinearOpBinding.h"
#include "IECorePython/LinearToCineonOpBinding.h"
#include "IECorePython/CineonToLinearOpBinding.h"
#include "IECorePython/LinearToAlexaLogcOpBinding.h"
#include "IECorePython/AlexaLogcToLinearOpBinding.h"
#include "IECorePython/CubeColorLookupBinding.h"
#include "IECorePython/CubeColorLookupDataBinding.h"
#include "IECorePython/CubeColorTransformOpBinding.h"
#include "IECorePython/LinearToRec709OpBinding.h"
#include "IECorePython/Rec709ToLinearOpBinding.h"
#include "IECorePython/ObjectVectorBinding.h"
#include "IECorePython/HenyeyGreensteinBinding.h"
#include "IECorePython/YUVImageWriterBinding.h"
#include "IECorePython/ImageCompositeOpBinding.h"
#include "IECorePython/ImagePremultiplyOpBinding.h"
#include "IECorePython/ImageUnpremultiplyOpBinding.h"
#include "IECorePython/OversamplesCalculatorBinding.h"
#include "IECorePython/DateTimeDataBinding.h"
#include "IECorePython/SGIImageReaderBinding.h"
#include "IECorePython/TimeDurationDataBinding.h"
#include "IECorePython/TimePeriodBinding.h"
#include "IECorePython/TimePeriodDataBinding.h"
#include "IECorePython/PatchMeshPrimitiveBinding.h"
#include "IECorePython/CurveExtrudeOpBinding.h"
#include "IECorePython/FrameListBinding.h"
#include "IECorePython/EmptyFrameListBinding.h"
#include "IECorePython/FrameRangeBinding.h"
#include "IECorePython/CompoundFrameListBinding.h"
#include "IECorePython/ReorderedFrameListBinding.h"
#include "IECorePython/BinaryFrameListBinding.h"
#include "IECorePython/ReversedFrameListBinding.h"
#include "IECorePython/ExclusionFrameListBinding.h"
#include "IECorePython/FrameListParameterBinding.h"
#include "IECorePython/FileSequenceBinding.h"
#include "IECorePython/FileSequenceFunctionsBinding.h"
#include "IECorePython/FileSequenceParameterBinding.h"
#include "IECorePython/FileSequenceVectorParameterBinding.h"
#include "IECorePython/ParameterisedProceduralBinding.h"
#include "IECorePython/LevenbergMarquardtBinding.h"
#include "IECorePython/ColorSpaceTransformOpBinding.h"
#include "IECorePython/NParticleReaderBinding.h"
#include "IECorePython/FaceAreaOpBinding.h"
#include "IECorePython/CurvesMergeOpBinding.h"
#include "IECorePython/CurvesPrimitiveEvaluatorBinding.h"
#include "IECorePython/HdrMergeOpBinding.h"
#include "IECorePython/PointDistributionBinding.h"
#include "IECorePython/CurveTracerBinding.h"
#include "IECorePython/ImageThinnerBinding.h"
#include "IECorePython/CurveLineariserBinding.h"
#include "IECorePython/CurveTangentsOpBinding.h"
#include "IECorePython/SmoothSkinningDataBinding.h"
#include "IECorePython/FaceVaryingPromotionOpBinding.h"
#include "IECorePython/MeshDistortionsOpBinding.h"
#include "IECorePython/PointVelocityDisplaceOpBinding.h"
#include "IECorePython/HexConversionBinding.h"
#include "IECorePython/CompressSmoothSkinningDataOpBinding.h"
#include "IECorePython/DecompressSmoothSkinningDataOpBinding.h"
#include "IECorePython/ReorderSmoothSkinningInfluencesOpBinding.h"
#include "IECorePython/NormalizeSmoothSkinningWeightsOpBinding.h"
#include "IECorePython/LimitSmoothSkinningInfluencesOpBinding.h"
#include "IECorePython/MixSmoothSkinningWeightsOpBinding.h"
#include "IECorePython/SmoothSmoothSkinningWeightsOpBinding.h"
#include "IECorePython/PointSmoothSkinningOpBinding.h"
#include "IECorePython/AddSmoothSkinningInfluencesOpBinding.h"
#include "IECorePython/RemoveSmoothSkinningInfluencesOpBinding.h"
#include "IECorePython/TransferSmoothSkinningWeightsOpBinding.h"
#include "IECorePython/LookupBinding.h"
#include "IECorePython/PointsPrimitiveEvaluatorBinding.h"
#include "IECorePython/PointsMotionOpBinding.h"
#include "IECorePython/CapturingRendererBinding.h"
#include "IECorePython/LightBinding.h"
#include "IECorePython/ContrastSmoothSkinningWeightsOpBinding.h"
#include "IECorePython/CamelCaseBinding.h"
#include "IECorePython/PointDistributionOpBinding.h"
#include "IECorePython/LRUCacheBinding.h"
#include "IECorePython/DataInterleaveOpBinding.h"
#include "IECorePython/DataConvertOpBinding.h"
#include "IECorePython/PNGImageReaderBinding.h"
#include "IECorePython/DeepPixelBinding.h"
#include "IECorePython/DeepImageReaderBinding.h"
#include "IECorePython/DeepImageWriterBinding.h"
#include "IECorePython/DeepImageConverterBinding.h"
#include "IECorePython/MurmurHashBinding.h"
#include "IECorePython/DiskPrimitiveBinding.h"
#include "IECorePython/ClampOpBinding.h"
#include "IECorePython/MeshFaceFilterOpBinding.h"
#include "IECorePython/ImfBinding.h"
#include "IECorePython/TimeCodeDataBinding.h"
#include "IECorePython/TimeCodeParameterBinding.h"
#include "IECorePython/OptionsBinding.h"
#include "IECorePython/MPlayDisplayDriverBinding.h"
#include "IECorePython/SceneInterfaceBinding.h"
#include "IECorePython/SharedSceneInterfacesBinding.h"
#include "IECorePython/SampledSceneInterfaceBinding.h"
#include "IECorePython/SceneCacheBinding.h"
#include "IECorePython/LinkedSceneBinding.h"
#include "IECorePython/LensModelBinding.h"
#include "IECorePython/StandardRadialLensModelBinding.h"
#include "IECorePython/LensDistortOpBinding.h"
#include "IECorePython/ObjectPoolBinding.h"
#include "IECorePython/EXRDeepImageReaderBinding.h"
#include "IECorePython/EXRDeepImageWriterBinding.h"
#include "IECorePython/ExternalProceduralBinding.h"
#include "IECorePython/ClippingPlaneBinding.h"
#include "IECorePython/DataAlgoBinding.h"
#include "IECorePython/MeshAlgoBinding.h"
#include "IECore/IECore.h"

using namespace IECorePython;
using namespace boost::python;

// Module declaration

BOOST_PYTHON_MODULE(_IECore)
{
	bindRefCounted();
	bindRunTimeTyped();
	bindException();
	bindImath();
	bindKDTree();
	bindObject();
	bindCompoundObject();
	bindTypeId();
	bindData();
	bindGeometricTypedData();
	bindAllSimpleTypedData();
	bindAllVectorTypedData();
	bindCompoundData();
	bindIndexedIO();
	bindMessageHandler();
	bindParameterised();
	bindOp();
	bindReader();
	bindParticleReader();
	bindPDCParticleReader();
	bindBlindDataHolder();
	bindRenderable();
	bindStateRenderable();
	bindVisibleRenderable();
	bindRenderer();
	bindWriter();
	bindParticleWriter();
	bindPDCParticleWriter();
	bindObjectReader();
	bindObjectWriter();
	bindParameter();
	bindNumericParameter();
	bindSimpleTypedParameter();
	bindVectorTypedParameter();
	bindSplineParameter();
	bindCubeColorLookupParameter();
	bindDateTimeParameter();
	bindTimePeriodParameter();
	bindTimeDurationParameter();
	bindCompoundParameter();
	bindValidatedStringParameter();
	bindPathParameter();
	bindFileNameParameter();
	bindDirNameParameter();
	bindPrimitive();
	bindPrimitiveVariable();
	bindPointsPrimitive();
	bindPerlinNoise();
	bindHalf();
	bindTimer();
	bindTurbulence();
	bindShader();
	bindSearchPath();
	bindCachedReader();
	bindObjectParameter();
	bindModifyOp();
	bindPrimitiveOp();
	bindImagePrimitive();
	bindImageReader();
	bindImageWriter();
	bindEXRImageReader();
	bindEXRImageWriter();

#ifdef IECORE_WITH_TIFF
	bindTIFFImageReader();
	bindTIFFImageWriter();
#endif

	bindCINImageReader();
	bindCINImageWriter();
	bindDPXImageReader();
	bindDPXImageWriter();

#ifdef IECORE_WITH_JPEG
	bindJPEGImageReader();
	bindJPEGImageWriter();
#endif

	bindTGAImageReader();
	bindTGAImageWriter();

	bindMeshPrimitive();
	bindMotionPrimitive();
	bindTransform();
	bindMatrixTransform();
	bindMatrixMotionTransform();
	bindGroup();
	bindAttributeState();
	bindNullObject();
	bindObjectInterpolator();
	bindPointNormalsOp();
	bindPointDensitiesOp();
	bindOversamplesCalculator();
	bindTransformationMatrix();
	bindTransformationMatrixData();
	bindBoundedKDTree();
	bindVectorDataFilterOp();
	bindTypedObjectParameter();
	bindTypedPrimitiveParameter();
	bindHeaderGenerator();
	bindPreWorldRenderable();
	bindCamera();
	bindNURBSPrimitive();
	bindDataCastOp();
	bindDataPromoteOp();
	bindMatrixMultiplyOp();
	bindPointBoundsOp();
	bindImathRandom();
	bindRandomRotationOp();
	bindMeshPrimitiveBuilder();
	bindTypedPrimitiveOp();
	bindPrimitiveEvaluator();
	bindMeshPrimitiveEvaluator();
	bindTriangulateOp();
	bindInternedString();
	bindSpherePrimitive();
	bindSpherePrimitiveEvaluator();
	bindInverseDistanceWeightedInterpolation();
	bindImageCropOp();
	bindMeshPrimitiveShrinkWrapOp();
	bindImagePrimitiveEvaluator();
	bindMath();
	bindCameraController();
	bindPathVectorParameter();
	bindTriangleAlgo();
	bindColorTransformOp();
	bindConverter();
	bindFromCoreConverter();
	bindTransformOp();
	bindLineSegment();
	bindCubicBasis();
	bindCurvesPrimitive();
	bindImageDiffOp();
	bindTriangulator();
	bindBezierAlgo();
	bindToCoreConverter();
	bindMeshNormalsOp();
	bindPolygonAlgo();
	bindMeshMergeOp();

#ifdef IECORE_WITH_FREETYPE

	bindFont();

#endif

	bindUnicodeToString();
	bindRadixSort();
	bindPointRepulsionOp();
	bindAngleConversion();
	bindLuminanceOp();
	bindChannelOp();
	bindSummedAreaOp();
	bindGrade();
	bindMedianCutSampler();
	bindEnvMapSampler();
	bindMeshVertexReorderOp();
	bindSpline();
	bindSplineData();
	bindCoordinateSystem();

#ifdef IECORE_WITH_ASIO

	bindDisplayDriver();
	bindImageDisplayDriver();
	bindClientDisplayDriver();
	bindDisplayDriverServer();
	// see note in Sconstruct re IECORE_WITH_ASIO and OBJReader
	bindOBJReader();

#endif

	bindSplineToImage();
	bindDisplay();
	bindMeshTangentsOp();
	bindWarpOp();
	bindUVDistortOp();
	bindSRGBToLinearOp();
	bindLinearToSRGBOp();
	bindCineonToLinearOp();
	bindLinearToCineonOp();
	bindAlexaLogcToLinearOp();
	bindLinearToAlexaLogcOp();
	bindCubeColorLookup();
	bindCubeColorLookupData();
	bindCubeColorTransformOp();
	bindLinearToRec709Op();
	bindRec709ToLinearOp();
	bindObjectVector();
	bindHenyeyGreenstein();
	bindYUVImageWriter();
	bindImagePremultiplyOp();
	bindImageUnpremultiplyOp();
	bindImageCompositeOp();
	bindDateTimeData();
	bindSGIImageReader();
	bindTimeDurationData();
	bindTimePeriod();
	bindTimePeriodData();
	bindPatchMeshPrimitive();
	bindCurveExtrudeOp();
	bindFrameList();
	bindEmptyFrameList();
	bindFrameRange();
	bindCompoundFrameList();
	bindReorderedFrameList();
	bindBinaryFrameList();
	bindReversedFrameList();
	bindExclusionFrameList();
	bindFrameListParameter();
	bindFileSequence();
	bindFileSequenceFunctions();
	bindFileSequenceParameter();
	bindFileSequenceVectorParameter();
	bindParameterisedProcedural();
	bindLevenbergMarquardt();
	bindColorSpaceTransformOp();
	bindNParticleReader();
	bindFaceAreaOp();
	bindCurvesMergeOp();
	bindCurvesPrimitiveEvaluator();
	bindHdrMergeOp();
	bindPointDistribution();
	bindCurveTracer();
	bindImageThinner();
	bindCurveLineariser();
	bindCurveTangentsOp();
	bindSmoothSkinningData();
	bindFaceVaryingPromotionOp();
	bindMeshDistortionsOp();
	bindPointVelocityDisplaceOp();
	bindHexConversion();
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
	bindLookup();
	bindPointsPrimitiveEvaluator();
	bindPointsMotionOp();
	bindCapturingRenderer();
	bindLight();
	bindContrastSmoothSkinningWeightsOp();
	bindCamelCase();
	bindPointDistributionOp();
	bindLRUCache();
	bindDataInterleaveOp();
	bindDataConvertOp();
	
#ifdef IECORE_WITH_PNG

	bindPNGImageReader();
	
#endif
	
	bindDeepPixel();
	bindDeepImageReader();
	bindDeepImageWriter();
	bindDeepImageConverter();
	bindMurmurHash();
	bindDiskPrimitive();
	bindClampOp();
	bindMeshFaceFilterOp();
	bindImf();
	bindTimeCodeData();
	bindTimeCodeParameter();
	bindOptions();
	bindMPlayDisplayDriver();
	bindSceneInterface();
	bindSharedSceneInterfaces();
	bindSampledSceneInterface();
	bindSceneCache();
	bindLinkedScene();
	bindLensModel();
	bindStandardRadialLensModel();
	bindLensDistortOp();
	bindObjectPool();
	bindExternalProcedural();
	bindClippingPlane();
	bindDataAlgo();
	bindMeshAlgo();

#ifdef IECORE_WITH_DEEPEXR

	bindEXRDeepImageReader();
	bindEXRDeepImageWriter();

#endif

	def( "majorVersion", &IECore::majorVersion );
	def( "minorVersion", &IECore::minorVersion );
	def( "patchVersion", &IECore::patchVersion );
	def( "versionString", &IECore::versionString, return_value_policy<copy_const_reference>() );
	def( "withASIO", &IECore::withASIO );
	def( "withSignals", &IECore::withSignals );
	def( "withTIFF", &IECore::withTIFF );
	def( "withJPEG", &IECore::withJPEG );
	def( "withDeepEXR", &IECore::withDeepEXR );
	def( "withFreeType", &IECore::withFreeType );
	def( "withPNG", &IECore::withPNG );
	def( "initThreads", &PyEval_InitThreads );
	def( "hardwareConcurrency", &tbb::tbb_thread::hardware_concurrency );

}

