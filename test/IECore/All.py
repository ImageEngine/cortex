##########################################################################
#
#  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
#
#  Copyright (c) 2010, John Haddon. All rights reserved.
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

import unittest
import warnings
import sys

import IECore

warnings.simplefilter( "error", DeprecationWarning )

from ClassLoader import *
from BlindDataHolder import *
from CompoundData import *
from CompoundObject import *
from Imath import *
from ImathVectorData import *
from IndexedIO import *
from KDTree import *
from BoundedKDTree import *
from MessageHandler import *
from ObjectIO import *
from Object import *
from ObjectReader import *
from ObjectWriter import *
from ParameterParser import *
from Parameterised import *
from Parameters import *
from PDCReader import *
from PDCWriter import *
from SimpleTypedData import *
from TypedDataAsObject import *
from VectorData import *
from FileSequence import *
from EXRImageReader import *
from EXRImageWriter import *
from PointsPrimitive import *
from ImagePrimitive import *
from PerlinNoise import *
from Turbulence import *
from MeshPrimitive import *
from Shader import *
from SearchPath import *
from CachedReader import *
from Reader import *
from RunTimeTyped import *
from Op import *
from MemoryUsage import *
from FileSequenceParameter import *
from WrapperToPython import *
from RemovePrimitiveVariables import *
from RenamePrimitiveVariables import *
from WrapperGarbageCollection import *
from FormattedParameterHelp import *
from MotionPrimitive import *
from Transform import *
from Group import *
from NamespacePollution import *
from OptionalCompoundParameter import *
from ObjectInterpolation import *
from TransformationMatrixData import *
from ReversedFrameList import *
from BinaryFrameList import *
from PointsExpressionOp import *
from FrameList import *
from FrameListParameter import *
from Struct import *
from Enum import *
from HeaderGenerator import *
from Camera import *
from NURBS import *
from Curry import *
from Menus import *
from DataCastOp import *
from DataPromoteOp import *
from MatrixMultiplyOp import *
from PointBoundsOp import *
from PrimitiveEvaluator import *
from MeshPrimitiveEvaluator import *
from InternedStringTest import InternedStringTest
from Writer import *
from TriangulateOp import *
from SpherePrimitiveEvaluator import *
from SearchReplaceOp import *
from CINImageReader import *
from CINImageWriter import *
from DPXImageReader import *
from DPXImageWriter import *
from InverseDistanceWeightedInterpolation import *
from ImageCropOp import *
from MeshPrimitiveShrinkWrapOp import *
from ImagePrimitiveEvaluator import *
from CapturingMessageHandler import *
from Math import *
from FileSequenceVectorParameter import *
from TriangleAlgoTest import *
from ColorTransformOpTest import *
from TransformOpTest import *
from LineSegmentTest import *
from CubicBasisTest import *
from CurvesPrimitiveTest import *
from ImageDiffOp import *
from TriangulatorTest import *
from BezierAlgoTest import *
from MeshNormalsOpTest import *
from PrimitiveTest import *
from MeshMergeOpTest import *
from UnicodeToStringTest import *
from RadixSortTest import *
from ImathRootsTest import *
from AngleConversionTest import *
from LuminanceOpTest import *
from SummedAreaOpTest import *
from GradeTest import *
from MedianCutSamplerTest import *
from EnvMapSamplerTest import *
from RandomTest import *
from MeshVertexReorderOpTest import *
from SplineTest import *
from SplineDataTest import *
from TypeIdTest import *
from LayeredDictTest import *
from SplineParameterTest import *
from AttributeStateTest import *
from CoordinateSystemTest import *
from SplineToImageTest import *
from DisplayTest import *
from MeshTangentsOpTest import *
from CubeColorLookupTest import *
from CubeColorLookupDataTest import *
from CubeColorTransformOpTest import *
from CompoundVectorParameterTest import *
from UVDistortOpTest import *
from ObjectVectorTest import *
from ImagePremultiplyOpTest import *
from ImageUnpremultiplyOpTest import *
from ImageCompositeOpTest import *
from ImageSequenceCompositeOpTest import *
from YUVImageWriter import *
from OversamplesCalculatorTest import *
from DateTimeDataTest import *
from DateTimeParameterTest import *
from SequenceLsOpTest import *
from SGIImageReaderTest import *
from TimeDurationDataTest import *
from TimePeriodDataTest import *
from PatchMeshPrimitiveTest import *
from CurveExtrudeOp import *
from ParameterisedProceduralTest import *
from LevenbergMarquardtTest import *
from TypedDataTest import *
from DataTraitsTest import *
from ColorSpaceTransformOpTest import *
from TGAImageReaderTest import *
from TGAImageWriterTest import *
from NParticleReader import *
from OBJReaderTest import TestOBJReader
from FaceAreaOpTest import FaceAreaOpTest
from CurvesMergeOpTest import CurvesMergeOpTest
from CurvesPrimitiveEvaluatorTest import CurvesPrimitiveEvaluatorTest
from SubstitutedDictTest import SubstitutedDictTest
from PointDistributionTest import PointDistributionTest
from CurveTracerTest import CurveTracerTest
from ImageThinnerTest import ImageThinnerTest
from CurveLineariserTest import CurveLineariserTest
from IDXReaderTest import IDXReaderTest
from ThreadingTest import ThreadingTest
from StringUtilTest import *
from ClassParameterTest import ClassParameterTest
from ClassVectorParameterTest import ClassVectorParameterTest
from CurveTangentsOpTest import CurveTangentsOpTest
from SmoothSkinningDataTest import *
from IgnoredExceptionsTest import IgnoredExceptionsTest
from PrimitiveVariableTest import PrimitiveVariableTest
from FaceVaryingPromotionOpTest import FaceVaryingPromotionOpTest
from MeshDistortionsOpTest import TestMeshDistortionsOp
from PointVelocityDisplaceOp import *
from HexConversionTest import HexConversionTest
from CompressAndDecompressSmoothSkinningDataOpsTest import CompressAndDecompressSmoothSkinningDataOpsTest
from BasicPreset import TestBasicPreset
from RelativePreset import TestRelativePreset
from ReorderSmoothSkinningInfluencesOpTest import ReorderSmoothSkinningInfluencesOpTest
from NormalizeSmoothSkinningWeightsOpTest import NormalizeSmoothSkinningWeightsOpTest
from LimitSmoothSkinningInfluencesOpTest import LimitSmoothSkinningInfluencesOpTest
from MixSmoothSkinningWeightsOpTest import MixSmoothSkinningWeightsOpTest
from SmoothSmoothSkinningWeightsOpTest import SmoothSmoothSkinningWeightsOpTest
from PointSmoothSkinningOpTest import PointSmoothSkinningOpTest
from AddAndRemoveSmoothSkinningInfluencesOpTest import AddAndRemoveSmoothSkinningInfluencesOpTest
from LookupTest import LookupTest
from ParameterAlgoTest import ParameterAlgoTest
from PointsPrimitiveEvaluatorTest import PointsPrimitiveEvaluatorTest
from PointsMotionOpTest import PointsMotionOpTest
from CamelCaseTest import CamelCaseTest
from CapturingRendererTest import CapturingRendererTest
from LightTest import LightTest
from ContrastSmoothSkinningWeightsOpTest import ContrastSmoothSkinningWeightsOpTest
from CameraControllerTest import CameraControllerTest
from PointDistributionOpTest import PointDistributionOpTest
from LRUCacheTest import LRUCacheTest
from DataInterleaveOpTest import DataInterleaveOpTest
from DataConvertOpTest import DataConvertOpTest
from DeepPixelTest import DeepPixelTest
from ConfigLoaderTest import ConfigLoaderTest
from MurmurHashTest import MurmurHashTest
from BoolVectorData import BoolVectorDataTest
from CompoundParameterTest import CompoundParameterTest
from DiskPrimitiveTest import DiskPrimitiveTest
from ClampOpTest import ClampOpTest
from SWAReaderTest import SWAReaderTest
from ImfTest import *
from TimeCodeDataTest import TimeCodeDataTest
from TimeCodeParameterTest import TimeCodeParameterTest
from OptionsTest import OptionsTest
from NullObjectTest import NullObjectTest
from SceneCacheTest import SceneCacheTest
from LinkedSceneTest import LinkedSceneTest
from StandardRadialLensModelTest import StandardRadialLensModelTest
from LensDistortOpTest import LensDistortOpTest
from ObjectPoolTest import ObjectPoolTest
from RefCountedTest import RefCountedTest
from ExternalProceduralTest import ExternalProceduralTest
from ClippingPlaneTest import ClippingPlaneTest
from DataAlgoTest import DataAlgoTest
from DisplayDriverServerTest import DisplayDriverServerTest

if IECore.withDeepEXR() :
	from EXRDeepImageReaderTest import EXRDeepImageReaderTest
	from EXRDeepImageWriterTest import EXRDeepImageWriterTest

if IECore.withASIO() :
	from DisplayDriverTest import *

if IECore.withTIFF() :
	from TIFFImageReader import *
	from TIFFImageWriter import *

if IECore.withJPEG() :
	from JPEGImageReader import *
	from JPEGImageWriter import *

if IECore.withFreeType() :
	from FontTest import *

if IECore.withPNG() :
	from PNGImageReader import TestPNGReader

unittest.TestProgram(
	testRunner = unittest.TextTestRunner(
		stream = IECore.CompoundStream(
			[
				sys.stderr,
				open( "test/IECore/resultsPython.txt", "w" )
			]
		),
		verbosity = 2
	)
)
