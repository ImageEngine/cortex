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

import unittest
from IECore import *

class AddOp( ColorTransformOp ) :

	def __init__( self ) :
	
		ColorTransformOp.__init__( self, "add", "adds" )

		self.numBegins = 0
		self.numTransforms = 0
		self.numEnds = 0
		self.add = Color3f( 1, 2, 3 )
		self.raiseException = False

	def begin( self, operands ) :
	
		self.numBegins += 1

	def transform( self, color ) :			
	
		# we expect the python implementations to
		# return a new color as below
		result = color + self.add
	
		# we don't intend the python implementations to
		# modify in place so this should have no effect
		color[0]=10000
		color[1]=10000
		color[2]=10000
		
		self.numTransforms += 1
		
		if self.raiseException :
			
			raise RuntimeError( "Error in transform!" )
		
		return result

	def end( self ) :
	
		self.numEnds += 1

class TestPythonOp( unittest.TestCase ) :

	def testParameterDefaults( self ) :
	
		o = AddOp()
		
		self.assertEqual( o.colorPrimVar.getTypedValue(), "Cs" )
		self.assertEqual( o.redPrimVar.getTypedValue(), "R" )
		self.assertEqual( o.greenPrimVar.getTypedValue(), "G" )
		self.assertEqual( o.bluePrimVar.getTypedValue(), "B" )
		self.assertEqual( o.alphaPrimVar.getTypedValue(), "A" )
		self.assertEqual( o.premultiplied.getTypedValue(), True )

	def testSeparateRGB( self ) :
	
		p = PointsPrimitive( 3 )

		r = FloatVectorData( [ 1, 2, 3 ] )
		g = FloatVectorData( [ 4, 5, 6 ] )
		b = FloatVectorData( [ 7, 8, 9 ] )

		p["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, r )
		p["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, g )
		p["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, b )
	
		o = AddOp()
		
		pp = o( input=p )
		
		self.assertEqual( o.numBegins, 1 )
		self.assertEqual( o.numTransforms, 3 )
		self.assertEqual( o.numEnds, 1 )
		
		self.assertEqual( pp["R"].data, FloatVectorData( [ x + 1 for x in r ] ) ) 
		self.assertEqual( pp["G"].data, FloatVectorData( [ x + 2 for x in g ] ) ) 
		self.assertEqual( pp["B"].data, FloatVectorData( [ x + 3 for x in b ] ) ) 
		
		# repeat with alpha
		
		a = FloatVectorData( [ 0.5, 0.5, 0.5 ] )
		p["A"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, a )
		
		pp = o( input=p )

		self.assertEqual( o.numBegins, 2 )
		self.assertEqual( o.numTransforms, 6 )
		self.assertEqual( o.numEnds, 2 )
		
		self.assertEqual( pp["R"].data, FloatVectorData( [ x + 0.5 for x in r ] ) ) 
		self.assertEqual( pp["G"].data, FloatVectorData( [ x + 1 for x in g ] ) ) 
		self.assertEqual( pp["B"].data, FloatVectorData( [ x + 1.5 for x in b ] ) ) 
		
		# repeat with alpha and premultiplication off
		
		pp = o( input=p, premultiplied=False )
		
		self.assertEqual( o.numBegins, 3 )
		self.assertEqual( o.numTransforms, 9 )
		self.assertEqual( o.numEnds, 3 )
		
		self.assertEqual( pp["R"].data, FloatVectorData( [ x + 1 for x in r ] ) ) 
		self.assertEqual( pp["G"].data, FloatVectorData( [ x + 2 for x in g ] ) ) 
		self.assertEqual( pp["B"].data, FloatVectorData( [ x + 3 for x in b ] ) ) 

	def testCs( self ) :
	
		p = PointsPrimitive( 2 )
		
		cs = Color3fVectorData( [ Color3f( 1, 2, 3 ), Color3f( 10, 11, 12 ) ] )
		p["Cs"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, cs )
		
		o = AddOp()
		
		pp = o( input=p )
		
		self.assertEqual( pp["Cs"].data, Color3fVectorData( [ x + Color3f( 1, 2, 3 ) for x in cs ] ) ) 
		
		self.assertEqual( o.numBegins, 1 )
		self.assertEqual( o.numTransforms, 2 )
		self.assertEqual( o.numEnds, 1 )
		
		# repeat with alpha
		
		a = FloatVectorData( [ 0.5, 0.5 ] )
		p["A"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, a )
		
		pp = o( input=p )

		self.assertEqual( o.numBegins, 2 )
		self.assertEqual( o.numTransforms, 4 )
		self.assertEqual( o.numEnds, 2 )

		self.assertEqual( pp["Cs"].data, Color3fVectorData( [ x + Color3f( 0.5, 1, 1.5 ) for x in cs ] ) ) 
		
		
		# repeat with alpha and premultiplication off
				
		pp = o( input=p, premultiplied=False )

		self.assertEqual( o.numBegins, 3 )
		self.assertEqual( o.numTransforms, 6 )
		self.assertEqual( o.numEnds, 3 )

		self.assertEqual( pp["Cs"].data, Color3fVectorData( [ x + Color3f( 1, 2, 3 ) for x in cs ] ) ) 
		
	def testExceptions( self ) :
	
		p = PointsPrimitive( 2 )
		
		cs = Color3fVectorData( [ Color3f( 1, 2, 3 ) ] )
		p["Cs"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, cs )
		
		o = AddOp()
		o.raiseException = True
				
		self.assertRaises( RuntimeError, curry( o, input = p ) )
		self.assertEqual( o.numBegins, 1 )
		self.assertEqual( o.numTransforms, 1 )
		self.assertEqual( o.numEnds, 1 )	
		
if __name__ == "__main__":
	unittest.main()
	
