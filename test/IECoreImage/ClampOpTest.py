##########################################################################
#
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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
import os

import IECore
import IECoreImage

class ClampOpTest( unittest.TestCase ) :

	def test( self ) :

		image = IECore.Reader.create( os.path.join( "test", "IECoreImage", "data", "exr", "ramp.exr" ) ).read()
		minInputValue = min( image["R"] )
		maxInputValue = max( image["R"] )

		self.assertTrue( minInputValue < 0.25 )
		self.assertTrue( maxInputValue > 0.5 )

		image2 = IECoreImage.ClampOp()( input=image, min=0.25, max=0.5 )

		minOutputValue = min( image2["R"] )
		maxOutputValue = max( image2["R"] )

		self.assertEqual( minOutputValue, 0.25 )
		self.assertEqual( maxOutputValue, 0.5 )

	def testMinTo( self ) :

		image = IECore.Reader.create( os.path.join( "test", "IECoreImage", "data", "exr", "ramp.exr" ) ).read()
		minInputValue = min( image["R"] )
		maxInputValue = max( image["R"] )

		self.assertTrue( minInputValue < 0.125 )
		self.assertTrue( maxInputValue > 0.5 )

		image2 = IECoreImage.ClampOp()( input=image, min=0.25, max=0.5, enableMinTo=True, minTo=0.125 )

		minOutputValue = min( image2["R"] )
		maxOutputValue = max( image2["R"] )

		self.assertEqual( minOutputValue, 0.125 )
		self.assertEqual( maxOutputValue, 0.5 )

		intermediateValues = False
		for v in image2["R"] :
			if v > 0.125 and v < 0.25 :
				intermediateValues = True

		self.assertFalse( intermediateValues )

	def testMaxTo( self ) :

		image = IECore.Reader.create( os.path.join( "test", "IECoreImage", "data", "exr", "ramp.exr" ) ).read()
		minInputValue = min( image["R"] )
		maxInputValue = max( image["R"] )

		self.assertTrue( minInputValue < 0.125 )
		self.assertTrue( maxInputValue > 0.5 )

		image2 = IECoreImage.ClampOp()( input=image, min=0.25, max=0.5, enableMaxTo=True, maxTo=0.75 )

		minOutputValue = min( image2["R"] )
		maxOutputValue = max( image2["R"] )

		self.assertEqual( minOutputValue, 0.25 )
		self.assertEqual( maxOutputValue, 0.75 )

		intermediateValues = False
		for v in image2["R"] :
			if v > 0.5 and v < 0.75 :
				intermediateValues = True

		self.assertFalse( intermediateValues )

	def testMinToAndMaxTo( self ) :

		image = IECore.Reader.create( os.path.join( "test", "IECoreImage", "data", "exr", "ramp.exr" ) ).read()
		minInputValue = min( image["R"] )
		maxInputValue = max( image["R"] )

		self.assertTrue( minInputValue < 0.125 )
		self.assertTrue( maxInputValue > 0.5 )

		image2 = IECoreImage.ClampOp()( input=image, min=0.25, max=0.5, enableMinTo=True, minTo=0.125, enableMaxTo=True, maxTo=0.75 )

		minOutputValue = min( image2["R"] )
		maxOutputValue = max( image2["R"] )

		self.assertEqual( minOutputValue, 0.125 )
		self.assertEqual( maxOutputValue, 0.75 )

		intermediateValues = False
		for v in image2["R"] :
			if v > 0.5 and v < 0.75 :
				intermediateValues = True
			elif v > 0.5 and v < 0.75 :
				intermediateValues = True

		self.assertFalse( intermediateValues )

if __name__ == "__main__":
	unittest.main()

