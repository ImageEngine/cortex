##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

class MarschnerLookupTableOpTest( unittest.TestCase ) :
	
	def testMarschnerLookupTable( self ) :
		
		m = IECore.MarschnerParameter( "marschner", "model parameters", True )
		
		# Make sure that we don't fail just because the defaults have changed.
		m["refraction"] = 1.55
		m["color"] =  IECore.Color3f( 0.7, 0.5, 0.1 )
		m["eccentricity"] = 1.0
		m["shiftR"] = -8.0
		m["shiftTT"] = 4.0
		m["shiftTRT"] = 12.0
		m["widthR"] = 10.0
		m["widthTT"] = 5.0
		m["widthTRT"] = 20.0
		m["glint"] = 1.0
		m["causticWidth"] = 20.0
		m["causticFade"] = 0.2
		m["causticLimit"] = 0.5
		
		result = IECore.MarschnerLookupTableOp()( model = m.getValue(), resolution = 128 )
		
		expectedResult = IECore.Reader.create( "test/IECore/data/expectedResults/marschnerLookup.exr" ).read()

		diffOp = IECore.ImageDiffOp()
		diff = diffOp( imageA = result, imageB = expectedResult ).value
		
		self.failIf( diff )
		
	def testInit( self ) :
		
		op = IECore.MarschnerLookupTableOp()
		self.failUnless( "color" in op.parameters()["model"] )
		
		op = IECore.MarschnerLookupTableOp( False )
		self.failUnless( "absorption" in op.parameters()["model"] )
		
		
if __name__ == "__main__":
	unittest.main()

