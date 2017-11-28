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
from SimpleTypedData import *
from TypedDataAsObject import *
from VectorData import *
from FileSequence import *
from PerlinNoise import *
from Turbulence import *
from SearchPath import *
from CachedReader import *
from Reader import *
from RunTimeTyped import *
from Op import *
from MemoryUsage import *
from FileSequenceParameter import *
from WrapperToPython import *
from WrapperGarbageCollection import *
from FormattedParameterHelp import *
from NamespacePollution import *
from OptionalCompoundParameter import *
from ObjectInterpolation import *
from TransformationMatrixData import *
from ReversedFrameList import *
from BinaryFrameList import *
from FrameList import *
from FrameListParameter import *
from Struct import *
from Enum import *
from HeaderGenerator import *
from Curry import *
from Menus import *
from DataCastOp import *
from DataPromoteOp import *
from MatrixMultiplyOp import *
from InternedStringTest import InternedStringTest
from Writer import *
from SearchReplaceOp import *
from InverseDistanceWeightedInterpolation import *
from CapturingMessageHandler import *
from Math import *
from FileSequenceVectorParameter import *
from TriangleAlgoTest import *
from LineSegmentTest import *
from CubicBasisTest import *
from BezierAlgoTest import *
from UnicodeToStringTest import *
from RadixSortTest import *
from ImathRootsTest import *
from AngleConversionTest import *
from RandomTest import *
from SplineTest import *
from SplineDataTest import *
from TypeIdTest import *
from LayeredDictTest import *
from SplineParameterTest import *
from CompoundVectorParameterTest import *
from ObjectVectorTest import *
from OversamplesCalculatorTest import *
from DateTimeDataTest import *
from DateTimeParameterTest import *
from SequenceLsOpTest import *
from TimeDurationDataTest import *
from TimePeriodDataTest import *
from LevenbergMarquardtTest import *
from TypedDataTest import *
from DataTraitsTest import *
from SubstitutedDictTest import SubstitutedDictTest
from PointDistributionTest import PointDistributionTest
from StringUtilTest import *
from ClassParameterTest import ClassParameterTest
from ClassVectorParameterTest import ClassVectorParameterTest
from IgnoredExceptionsTest import IgnoredExceptionsTest
from HexConversionTest import HexConversionTest
from BasicPreset import TestBasicPreset
from RelativePreset import TestRelativePreset
from LookupTest import LookupTest
from ParameterAlgoTest import ParameterAlgoTest
from CamelCaseTest import CamelCaseTest
from LRUCacheTest import LRUCacheTest
from DataInterleaveOpTest import DataInterleaveOpTest
from DataConvertOpTest import DataConvertOpTest
from ConfigLoaderTest import ConfigLoaderTest
from MurmurHashTest import MurmurHashTest
from BoolVectorData import BoolVectorDataTest
from CompoundParameterTest import CompoundParameterTest
from ImfTest import *
from TimeCodeDataTest import TimeCodeDataTest
from TimeCodeParameterTest import TimeCodeParameterTest
from NullObjectTest import NullObjectTest
from StandardRadialLensModelTest import StandardRadialLensModelTest
from ObjectPoolTest import ObjectPoolTest
from RefCountedTest import RefCountedTest
from DataAlgoTest import DataAlgoTest
from PolygonAlgoTest import PolygonAlgoTest

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
