##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
#
#  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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
import IECore
import IECoreHoudini

from ProceduralHolder import *
from OpHolder import *
from CortexRmanInject import *
from ActiveTake import *
from NodeHandle import *
from FromHoudiniPointsConverter import *
from FromHoudiniPolygonsConverter import *
from ToHoudiniPointsConverter import *
from ToHoudiniPolygonsConverter import *
from AttributeRemap import *
from CortexConverterSop import *
from FromHoudiniCurvesConverter import *
from ToHoudiniCurvesConverter import *
from CobIOTranslator import *
from FromHoudiniGroupConverter import *
from TemporaryParameterValuesTest import * 
from ToHoudiniGroupConverter import *
from RATDeepImageReaderTest import *
from RATDeepImageWriterTest import *
from DeepImageConverter import *
from UpdateMode import *
from SceneCacheTest import *
from LiveSceneTest import *
from ToHoudiniCortexObjectConverter import *
from ToHoudiniCompoundObjectConverter import *

IECoreHoudini.TestProgram(
	testRunner = unittest.TextTestRunner(
		stream = IECore.CompoundStream(
			[
				sys.stderr,
				open( "test/IECoreHoudini/resultsPython.txt", "w" )
			]
		),
		verbosity = 2
	)
)
