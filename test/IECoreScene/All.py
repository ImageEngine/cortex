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

from PDCReader import *
from PDCWriter import *
from PointsPrimitive import *
from MeshPrimitive import *
from Shader import *
from RemovePrimitiveVariables import *
from RenamePrimitiveVariables import *
from MotionPrimitive import *
from Transform import *
from Group import *
from Camera import *
from NURBS import *
from PointBoundsOp import *
from PrimitiveEvaluator import *
from MeshPrimitiveEvaluator import *
from TriangulateOp import *
from SpherePrimitiveEvaluator import *
from MeshPrimitiveShrinkWrapOp import *
from TransformOpTest import *
from CurvesPrimitiveTest import *
from TriangulatorTest import *
from MeshNormalsOpTest import *
from PrimitiveTest import *
from MeshMergeOpTest import *
from MeshVertexReorderOpTest import *
from AttributeStateTest import *
from CoordinateSystemTest import *
from OutputTest import OutputTest
from PatchMeshPrimitiveTest import *
from CurveExtrudeOp import *
from NParticleReader import *
from OBJReaderTest import TestOBJReader
from CurvesMergeOpTest import CurvesMergeOpTest
from CurvesPrimitiveEvaluatorTest import CurvesPrimitiveEvaluatorTest
from CurveLineariserTest import CurveLineariserTest
from IDXReaderTest import IDXReaderTest
from CurveTangentsOpTest import CurveTangentsOpTest
from SmoothSkinningDataTest import *
from PrimitiveVariableTest import PrimitiveVariableTest
from FaceVaryingPromotionOpTest import FaceVaryingPromotionOpTest
from PointVelocityDisplaceOp import *
from CompressAndDecompressSmoothSkinningDataOpsTest import CompressAndDecompressSmoothSkinningDataOpsTest
from ReorderSmoothSkinningInfluencesOpTest import ReorderSmoothSkinningInfluencesOpTest
from NormalizeSmoothSkinningWeightsOpTest import NormalizeSmoothSkinningWeightsOpTest
from LimitSmoothSkinningInfluencesOpTest import LimitSmoothSkinningInfluencesOpTest
from MixSmoothSkinningWeightsOpTest import MixSmoothSkinningWeightsOpTest
from SmoothSmoothSkinningWeightsOpTest import SmoothSmoothSkinningWeightsOpTest
from PointSmoothSkinningOpTest import PointSmoothSkinningOpTest
from AddAndRemoveSmoothSkinningInfluencesOpTest import AddAndRemoveSmoothSkinningInfluencesOpTest
from PointsPrimitiveEvaluatorTest import PointsPrimitiveEvaluatorTest
from PointsMotionOpTest import PointsMotionOpTest
from LightTest import LightTest
from ContrastSmoothSkinningWeightsOpTest import ContrastSmoothSkinningWeightsOpTest
from DiskPrimitiveTest import DiskPrimitiveTest
from SWAReaderTest import SWAReaderTest
from OptionsTest import OptionsTest
from SceneCacheTest import SceneCacheTest
from LinkedSceneTest import LinkedSceneTest
from ExternalProceduralTest import ExternalProceduralTest
from ClippingPlaneTest import ClippingPlaneTest
from MeshAlgoTest import *
from CurvesAlgoTest import *
from PointsAlgoTest import *
from ObjectInterpolationTest import ObjectInterpolationTest
from SceneAlgo import *

if IECore.withFreeType() :
	from FontTest import *

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
