##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

import sys
import unittest
import warnings

import IECore
import IECoreMaya

warnings.simplefilter( "error", DeprecationWarning )

from ConverterHolder import *
from PlaybackFrameList import *
from ParameterisedHolder import *
from FromMayaCurveConverterTest import *
from PluginLoadUnload import *
from NamespacePollution import *
from FromMayaMeshConverterTest import *
from FromMayaParticleConverterTest import *
from FromMayaPlugConverterTest import *
from FromMayaUnitPlugConverterTest import *
from FromMayaGroupConverterTest import *
from FromMayaCameraConverterTest import *
from FromMayaConverterTest import *
from FromMayaObjectConverterTest import *
from FnParameterisedHolderTest import *
from ToMayaPlugConverterTest import *
from ToMayaMeshConverterTest import *
from ToMayaCurveConverterTest import *
from MayaTypeIdTest import *
from FromMayaTransformConverterTest import *
from CallbackIdTest import *
from TemporaryAttributeValuesTest import *
from SplineParameterHandlerTest import *
from DAGPathParametersTest import *
from FnProceduralHolderTest import FnProceduralHolderTest
from GeometryCombinerTest import GeometryCombinerTest
from FromMayaSkinClusterConverterTest import *
from ToMayaSkinClusterConverterTest import *
from ToMayaGroupConverterTest import ToMayaGroupConverterTest
from RunTimeTypedTest import RunTimeTypedTest
from ToMayaParticleConverterTest import ToMayaParticleConverterTest
from ImageConverterTest import ImageConverterTest
from ObjectDataTest import ObjectDataTest
from ToMayaCameraConverterTest import ToMayaCameraConverterTest
from MayaSceneTest import *
from SceneShapeTest import SceneShapeTest
from FnSceneShapeTest import FnSceneShapeTest
from FromMayaLocatorConverterTest import FromMayaLocatorConverterTest
from ToMayaLocatorConverterTest import ToMayaLocatorConverterTest

IECoreMaya.TestProgram(

	testRunner = unittest.TextTestRunner(
		stream = IECore.CompoundStream(
			[
				sys.stderr,
				open( "test/IECoreMaya/resultsPython.txt", "w" )
			]
		),
		verbosity = 2
	),

	plugins = [ "ieCore" ],

)
