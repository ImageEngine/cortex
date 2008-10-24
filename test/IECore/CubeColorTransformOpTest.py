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
import math
from IECore import *

class CubeColorTransformOpTest( unittest.TestCase ) :

	@staticmethod
	def makeColorCube( dim, gamma = None ) :
		
		cubeData = Color3fVectorData()

		for x in range( 0, dim.x ) :
			for y in range( 0, dim.y ) :
				for z in range( 0, dim.z ) :

					color = Color3f( float(x) / ( dim.x - 1 ), float(y) / ( dim.y - 1 ), float(z) / ( dim.z - 1 ) )

					if gamma :

						color.r = math.pow( color.r, 1.0 / gamma )
						color.g = math.pow( color.g, 1.0 / gamma )
						color.b = math.pow( color.b, 1.0 / gamma )

					cubeData.append( color )	

		return CubeColorLookupf( dim, cubeData )

	def testIdentity( self ) :
	
		cubeLookup = CubeColorLookupf()
			
		op = CubeColorTransformOp()
		
		window = Box2i( V2i( 0, 0 ), V2i( 2, 2 ) )
		img = ImagePrimitive( window, window )
		
		rData = FloatVectorData()
		rData.append( 0.0 )
		rData.append( 0.0 )
		rData.append( 0.0 )
		rData.append( 0.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )		
		rData.append( 0.5 )						
		
		gData = FloatVectorData()
		gData.append( 0.0 )
		gData.append( 0.0 )
		gData.append( 1.0 )
		gData.append( 1.0 )
		gData.append( 0.0 )
		gData.append( 0.0 )
		gData.append( 1.0 )
		gData.append( 1.0 )
		gData.append( 0.5 )
		
		bData = FloatVectorData()
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.5 )			
		
		numPixels = len( gData )
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, rData )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, gData )
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, bData )				
		
		self.assert_( img.arePrimitiveVariablesValid() )
		
		result = op(
			input = img,
			cube = cubeLookup
		)
		
		for i in range( 0, numPixels ) :
			
			self.assertEqual( result["R"].data[i], rData[i] )
			self.assertEqual( result["G"].data[i], gData[i] )
			self.assertEqual( result["B"].data[i], bData[i] )
		
		self.assertEqual( result, img )
		
	def testInvertRed( self ) :
	
		testData = Color3fVectorData(
			[
				Color3f( 1, 0, 0 ),
				Color3f( 1, 0, 1 ),
				Color3f( 1, 1, 0 ),
				Color3f( 1, 1, 1 ),
				Color3f( 0, 0, 0 ),
				Color3f( 0, 0, 1 ),
				Color3f( 0, 1, 0 ),
				Color3f( 0, 1, 1 ),				
			]
		)
		
		cubeLookup = CubeColorLookupf( V3i( 2, 2, 2 ), testData )
			
		op = CubeColorTransformOp()
		
		window = Box2i( V2i( 0, 0 ), V2i( 2, 2 ) )
		img = ImagePrimitive( window, window )
		
		rData = FloatVectorData()
		rData.append( 0.0 )
		rData.append( 0.0 )
		rData.append( 0.0 )
		rData.append( 0.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )		
		rData.append( 0.5 )						
		
		gData = FloatVectorData()
		gData.append( 0.0 )
		gData.append( 0.0 )
		gData.append( 1.0 )
		gData.append( 1.0 )		
		gData.append( 0.0 )
		gData.append( 0.0 )
		gData.append( 1.0 )
		gData.append( 1.0 )
		gData.append( 0.5 )
		assert( len( rData ) == len( gData ) )
		
		bData = FloatVectorData()
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.0 )
		bData.append( 1.0 )
		bData.append( 0.5 )
		assert( len( bData ) == len( gData ) )					
		
		numPixels = len( gData )
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, rData )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, gData )
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, bData )				
		
		self.assert_( img.arePrimitiveVariablesValid() )
		
		result = op(
			input = img,
			cube = cubeLookup
		)
		
		self.assertNotEqual( img, result )
		
		for i in range( 0, numPixels ) :
			
			self.assertEqual( result["R"].data[i], 1.0 - rData[i] )
			self.assertEqual( result["G"].data[i], gData[i] )
			self.assertEqual( result["B"].data[i], bData[i] )
	
	
	def testDomain( self ) :
	
		testData = Color3fVectorData(
			[
				Color3f( 1, 1, 1 ),
				Color3f( 1, 1, 2 ),
				Color3f( 1, 2, 1 ),
				Color3f( 1, 2, 2 ),
				Color3f( 2, 1, 1 ),
				Color3f( 2, 1, 2 ),
				Color3f( 2, 2, 1 ),
				Color3f( 2, 2, 2 ),				
			]
		)
		
		cubeLookup = CubeColorLookupf( V3i( 2, 2, 2 ), testData, Box3f( V3f( 1, 1, 1 ), V3f( 2, 2, 2 ) ) )
			
		op = CubeColorTransformOp()
		
		window = Box2i( V2i( 0, 0 ), V2i( 2, 2 ) )
		img = ImagePrimitive( window, window )
		
		rData = FloatVectorData()
		rData.append( 1.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )
		rData.append( 1.0 )
		rData.append( 2.0 )
		rData.append( 2.0 )
		rData.append( 2.0 )
		rData.append( 2.0 )		
		rData.append( 1.5 )						
		
		gData = FloatVectorData()
		gData.append( 1.0 )
		gData.append( 1.0 )
		gData.append( 2.0 )
		gData.append( 2.0 )
		gData.append( 1.0 )
		gData.append( 1.0 )
		gData.append( 2.0 )
		gData.append( 2.0 )
		gData.append( 1.5 )
		assert( len( rData ) == len( gData ) )
		
		bData = FloatVectorData()
		bData.append( 1.0 )
		bData.append( 2.0 )
		bData.append( 1.0 )
		bData.append( 2.0 )
		bData.append( 1.0 )
		bData.append( 2.0 )
		bData.append( 1.0 )
		bData.append( 2.0 )
		bData.append( 1.5 )
		assert( len( bData ) == len( gData ) )					
		
		numPixels = len( gData )
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, rData )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, gData )
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, bData )				
		
		self.assert_( img.arePrimitiveVariablesValid() )
		
		result = op(
			input = img,
			cube = cubeLookup
		)
		
		self.assertEqual( img, result )
			
	def testLarge( self ) :
	
		dim = V3i( 10, 10, 10 )															
						
		cubeLookup = CubeColorTransformOpTest.makeColorCube( dim, gamma = 1.5 )								
	
		img = Reader.create( "test/IECore/data/jpg/exif.jpg").read()
		
		op = CubeColorTransformOp()
		
		result = op(
			input = img,
			cube = cubeLookup,			
		)
		
		Writer.create( result, "result.jpg").write()
		
			
		
if __name__ == "__main__":
	unittest.main()
	
