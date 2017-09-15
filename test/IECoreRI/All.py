##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
import IECoreRI

from SLOReader import *
from Renderer import *
from Instancing import *
from PTCParticleReader import *
from PTCParticleWriter import *
from ArchiveRecord import *
from DoubleSided import *
from Orientation import *
from MultipleContextsTest import *
from Camera import *
from CurvesTest import *
from TextureOrientationTest import *
from ArrayPrimVarTest import *
from CoordinateSystemTest import *
from IlluminateTest import *
from SubsurfaceTest import *
from PatchMeshTest import *
from RIBWriterTest import *
from ParameterisedProcedural import *
from MotionTest import MotionTest
from PythonProceduralTest import PythonProceduralTest
from DetailTest import DetailTest
from ProceduralThreadingTest import ProceduralThreadingTest
from StringArrayParameterTest import StringArrayParameterTest
from CoshaderTest import CoshaderTest
from GroupTest import GroupTest
from DspyTest import DspyTest
from RerenderingTest import RerenderingTest
from PrimVarInterpretationTest import PrimVarInterpretationTest

if hasattr( IECoreRI, "SXRenderer" ) :
	from SXRendererTest import SXRendererTest

if IECore.withFreeType() :

	from TextTest import *

unittest.TestProgram(
	testRunner = unittest.TextTestRunner(
		stream = IECore.CompoundStream(
			[
				sys.stderr,
				open( "test/IECoreRI/resultsPython.txt", "w" )
			]
		),
		verbosity = 2
	)
)
