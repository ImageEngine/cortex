##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
import IECore
import sys

from ClassLoader import *
from AttributeCache import *
from HierarchicalCache import *
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
from InterpolatedCache import *
from TransformationMatrixData import *
from ReversedFrameList import *
from BinaryFrameList import *
from PointsExpressionOp import *
from FrameList import *
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
from ImplicitSurfaceFunction import *
from CachedImplicitSurfaceFunction import *
from MarchingCubes import *
from PointMeshOp import *
from CSGImplicitSurfaceFunction import *
from ParticleMeshOp import *
from PrimitiveEvaluator import *
from MeshPrimitiveEvaluator import *
from PrimitiveImplicitSurfaceFunction import *
from MeshPrimitiveImplicitSurfaceOp import *
from Interned import *
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
from FileExaminer import *
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
from UniformRandomPointDistributionOpTest import *
from UnicodeToStringTest import *
from MappedRandomPointDistributionOpTest import *
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

if IECore.withTIFF() :
	from TIFFImageReader import *
	from TIFFImageWriter import *

if IECore.withJPEG() :
	from JPEGImageReader import *
	from JPEGImageWriter import *
	
if IECore.withFreeType() :
	from FontTest import *

class SplitStream :

	def __init__( self ) :
	
		self.__f = open( "test/IECore/resultsPython.txt", 'w' )		

	def write( self, l ) :

		sys.stderr.write( l )
		self.__f.write( l )

unittest.TestProgram( testRunner = unittest.TextTestRunner( stream = SplitStream(), verbosity = 2 ) )		
