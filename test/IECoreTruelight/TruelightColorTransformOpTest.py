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

import IECore

import unittest
from IECoreTruelight import *

class TruelightColorTransformOpTest( unittest.TestCase ) :

	def testDefaultParameterValues( self ) :

		o = TruelightColorTransformOp()
		self.assertEqual( o["profile"].getTypedValue(), "Kodak" )
		self.assertEqual( o["display"].getTypedValue(), "monitor" )
		self.assertEqual( o["inputSpace"].getCurrentPresetName(), "linear" )
		self.assertEqual( o["rawTruelightOutput"].getTypedValue(), True )

	def testCommands( self ) :

		o = TruelightColorTransformOp()

		c = o.commands()

		self.failUnless( "display{monitor}" in c )


	def testConversion( self ) :

		o = TruelightColorTransformOp()
		o["display"] = IECore.StringData( "SonyHD" ) # SonyHD is a default monitor profile that ships with Truelight

		w = IECore.Box2i(
			IECore.V2i( 0, 0 ),
			IECore.V2i( 0, 0 )
		)

		img = IECore.ImagePrimitive( w, w )
		rData = IECore.FloatVectorData()
		gData = IECore.FloatVectorData()
		bData = IECore.FloatVectorData()

		inCol = IECore.Color3f( 0.5, 0.5, 0.5 )
		rData.append( inCol.r )
		gData.append( inCol.g )
		bData.append( inCol.b )
		img["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, rData )
		img["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, gData )
		img["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, bData )

		# These values /should/ be correct, but may not necessarily be. They're here mainly to test
		# for any unexpected changes in behaviour.
		# \todo Validate the results here, and remove the above comment when done
		out = o( input = img )
		outCol1 = IECore.Color3f( out["R"].data[0], out["G"].data[0], out["B"].data[0] )
		self.assert_( outCol1.equalWithAbsError( IECore.Color3f( 0.693873, 0.702439, 0.702624 ), 0.01 ) )

		# Make sure that turning off rawTruelightOutput indeed does an SRGB->Linear conversion
		out = o( input = img, rawTruelightOutput = False )
		outCol2 = IECore.Color3f( out["R"].data[0], out["G"].data[0], out["B"].data[0] )
		self.assert_( outCol2.equalWithAbsError( IECore.Color3f( 0.439312, 0.451469, 0.451734 ), 0.01 ) )

		linearToSRGB = IECore.LinearToSRGBOp()
		outSRGB = linearToSRGB(
			input = out,
			channels = IECore.StringVectorData( [ "R", "G", "B" ] )
		)
		outColSRGB = IECore.Color3f( outSRGB["R"].data[0], outSRGB["G"].data[0], outSRGB["B"].data[0] )
		self.assert_( outColSRGB.equalWithAbsError( outCol1, 0.01 ) )

if __name__ == "__main__":
	unittest.main()

