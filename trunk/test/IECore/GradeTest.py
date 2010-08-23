##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import os
import unittest
from IECore import *

class TestGradeColorTransformOp( unittest.TestCase ) :

	rampImgName = "test/IECore/data/exrFiles/ramp.exr"
	gradedRampImgName = "test/IECore/data/exrFiles/gradedRamp.exr"
	testImgName = "test/IECore/data/exrFiles/gradedRampTest.exr"

	def __verifyImageRGB( self, imgNew, imgOrig, msq = 0.05):

		self.assertEqual( type(imgNew), ImagePrimitive )

		op = ImageDiffOp()

		res = op(
			imageA = imgNew,
			imageB = imgOrig,
			maxError = msq,
			skipMissingChannels = True
		)

		self.failIf( res.value )

	# Apply a grade operation over the ramp.dpx file and compare results with an image
	# generated from Nuke with the following grade node parameters:
	# blackpoint {0.01 0.04 0.02 0}
	# whitepoint {0.8 0.4 0.66 1}
	# black {0.08 0.06 -0.28 0} (lift)
	# white {0.3 0.1 0.2 1} (gain)
	# multiply {0.6 1.2 0.2 1}
	# add {-0.2 0 -0.5 0}	(offset)
	# gamma {1.2 2 0.19 1}
	# black_clamp false
	def testGradeTransform( self ) :

		rampImg = Reader.create( self.rampImgName )()
		grade = Grade()
		grade['blackPoint'] = Color3f( 0.01, 0.04, 0.02 )
		grade['whitePoint'] = Color3f( 0.8, 0.4, 0.66 )
		grade['lift'] = Color3f( 0.08, 0.06, -0.28 )
		grade['gain'] = Color3f( 0.3, 0.1, 0.2 )
		grade['multiply'] = Color3f( 0.6, 1.2, 0.2 )
		grade['offset'] = Color3f( -0.2, 0, -0.5 )
		grade['gamma'] = Color3f( 1.2, 2, 0.19 )
		grade['blackClamp'] = False
		grade['whiteClamp'] = False
		imgNew = grade( input = rampImg )
		Writer.create( imgNew, self.testImgName )()
		self.__verifyImageRGB( Reader.create( self.testImgName )(), Reader.create( self.gradedRampImgName )(), msq = 0.1 )

	# Test if the identity operation works
	def testGradeTransformIdentity( self ) :

		rampImg = Reader.create( "test/IECore/data/exrFiles/ramp.exr" )()
		grade = Grade()
		grade['blackPoint'] = Color3f( 0, 0, 0 )
		grade['whitePoint'] = Color3f( 1, 1, 1 )
		grade['lift'] = Color3f( 0, 0, 0 )
		grade['gain'] = Color3f( 1, 1, 1 )
		grade['multiply'] = Color3f( 1, 1, 1 )
		grade['offset'] = Color3f( 0, 0, 0 )
		grade['gamma'] = Color3f( 1, 1, 1 )
		grade['blackClamp'] = False
		grade['whiteClamp'] = False
		imgNew = grade( input = rampImg )
		self.assertEqual( rampImg, imgNew )

	def tearDown( self ):
		if os.path.exists( self.testImgName ):
			os.remove( self.testImgName )

if __name__ == "__main__":
	unittest.main()

