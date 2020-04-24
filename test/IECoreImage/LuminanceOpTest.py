##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore
import IECoreImage

class LuminanceOpTest( unittest.TestCase ) :

	def testParameterDefaults( self ) :

		o = IECoreImage.LuminanceOp()

		self.assertEqual( o["colorChannel"].getTypedValue(), "Cs" )
		self.assertEqual( o["redChannel"].getTypedValue(), "R" )
		self.assertEqual( o["greenChannel"].getTypedValue(), "G" )
		self.assertEqual( o["blueChannel"].getTypedValue(), "B" )
		self.assertEqual( o["luminanceChannel"].getTypedValue(), "Y" )
		self.assertEqual( o["removeColorChannels"].getTypedValue(), True )

	def testSeparateRGB( self ) :

		w = imath.Box2i( imath.V2i( 0 ), imath.V2i( 2, 0 ) )
		i = IECoreImage.ImagePrimitive( w, w )
		i["R"] = IECore.FloatVectorData( [ 1, 2, 3 ] )
		i["G"] = IECore.FloatVectorData( [ 4, 5, 6 ] )
		i["B"] = IECore.FloatVectorData( [ 7, 8, 9 ] )

		ii = IECoreImage.LuminanceOp()( input=i, weights=imath.Color3f( 1, 2, 3 ) )

		self.assertTrue( not "R" in ii )
		self.assertTrue( not "G" in ii )
		self.assertTrue( not "B" in ii )

		self.assertTrue( "Y" in ii )

		self.assertAlmostEqual( ii["Y"][0], 30 )
		self.assertAlmostEqual( ii["Y"][1], 36 )
		self.assertAlmostEqual( ii["Y"][2], 42 )

	def testCs( self ) :

		i = IECoreImage.ImagePrimitive()
		i["Cs"] = IECore.Color3fVectorData( [ imath.Color3f( 1, 2, 3 ), imath.Color3f( 10, 11, 12 ) ] )

		ii = IECoreImage.LuminanceOp()( input=i, weights=imath.Color3f( 1, 2, 3 ), removeColorChannels=False )

		self.assertTrue( "Cs" in ii )
		self.assertTrue( "Y" in ii )

		self.assertAlmostEqual( ii["Y"][0], 14 )
		self.assertAlmostEqual( ii["Y"][1], 68 )

if __name__ == "__main__":
	unittest.main()

