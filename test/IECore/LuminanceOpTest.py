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
from IECore import *

class LuminanceOpTest( unittest.TestCase ) :

	def testParameterDefaults( self ) :
	
		o = LuminanceOp()
		
		self.assertEqual( o["colorPrimVar"].getTypedValue(), "Cs" )
		self.assertEqual( o["redPrimVar"].getTypedValue(), "R" )
		self.assertEqual( o["greenPrimVar"].getTypedValue(), "G" )
		self.assertEqual( o["bluePrimVar"].getTypedValue(), "B" )
		self.assertEqual( o["luminancePrimVar"].getTypedValue(), "Y" )
		self.assertEqual( o["removeColorPrimVars"].getTypedValue(), True )

	def testSeparateRGB( self ) :
	
		p = PointsPrimitive( 3 )

		r = FloatVectorData( [ 1, 2, 3 ] )
		g = FloatVectorData( [ 4, 5, 6 ] )
		b = FloatVectorData( [ 7, 8, 9 ] )

		p["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, r )
		p["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, g )
		p["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, b )

		pp = LuminanceOp()( input=p, weights=Color3f( 1, 2, 3 ) )
		
		self.assert_( not "R" in pp )
		self.assert_( not "G" in pp )
		self.assert_( not "B" in pp )
		
		self.assert_( "Y" in pp )
		self.assertEqual( pp["Y"].interpolation, PrimitiveVariable.Interpolation.Vertex )
		
		self.assertAlmostEqual( pp["Y"].data[0], 30 )
		self.assertAlmostEqual( pp["Y"].data[1], 36 )
		self.assertAlmostEqual( pp["Y"].data[2], 42 )
		
	def testCs( self ) :
	
		p = PointsPrimitive( 2 )

		cs = Color3fVectorData( [ Color3f( 1, 2, 3 ), Color3f( 10, 11, 12 ) ] )
		p["Cs"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, cs )

		pp = LuminanceOp()( input=p, weights=Color3f( 1, 2, 3 ), removeColorPrimVars=False )

		self.assert_( "Cs" in pp )
		self.assert_( "Y" in pp )
		self.assertEqual( pp["Y"].interpolation, PrimitiveVariable.Interpolation.Vertex )
		
		self.assertAlmostEqual( pp["Y"].data[0], 14 )
		self.assertAlmostEqual( pp["Y"].data[1], 68 )

if __name__ == "__main__":
	unittest.main()
	
