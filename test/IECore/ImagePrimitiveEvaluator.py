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

import math
import unittest
import random
from IECore import *

class TestImagePrimitiveEvaluator( unittest.TestCase ) :

	def testEmptyImage( self ) :
	
		""" Test ImagePrimitiveEvaluator with empty image"""
		
		w = Box2i( V2i( 0, 0 ), V2i( 99, 99 ) )
		img = ImagePrimitive( w, w )
		
		ipe = PrimitiveEvaluator.create( img )		
		self.assert_( ipe.isInstanceOf( "ImagePrimitiveEvaluator" ) )
		
		self.failIf( ipe.R() )
		self.failIf( ipe.G() )
		self.failIf( ipe.B() )
		self.failIf( ipe.A() )
		self.failIf( ipe.Y() )						
		
		self.assertEqual( ipe.surfaceArea(), 2 * 100 * 100 )
		self.assertEqual( ipe.volume(), 0.0 )
		self.assertEqual( ipe.centerOfGravity(), V3f( 0, 0, 0 ) )
		
		r = ipe.createResult()
		
		foundClosest = ipe.closestPoint( V3f( 0, 50, 0 ), r )
		
		self.assert_( foundClosest )
		self.assertEqual( r.pixel(), V2i( 49, 99 ) )
		self.assertEqual( r.point(), V3f( 0, 50, 0 ) )
		
		self.assert_( ( r.point() - V3f( 0, 50, 0 ) ).length() < 0.01 )
		self.assert_( ( r.uv() - V2f( 0.5, 1.0 ) ).length() < 0.001 )
		
		hit = ipe.intersectionPoint( V3f( 50, 50, 100 ), V3f( 0, 0, -1 ), r )
		self.assert_( hit )
		self.assert_( ( r.point() - V3f( 50, 50, 0 ) ).length() < 0.01 )
		self.assert_( ( r.uv() - V2f( 1, 1 ) ).length() < 0.001 )
		
		hits = ipe.intersectionPoints( V3f( 50, 50, 100 ), V3f( 0, 0, -1 ) )
		self.assertEqual( len(hits), 1 )
		
		hits = ipe.intersectionPoints( V3f( 150, 50, 100 ), V3f( 0, 0, -1 ) )
		self.assertEqual( len(hits), 0 )
		
		self.failIf( ipe.pointAtUV( V2f( 1.1, 1.0 ), r ) )
		self.failIf( ipe.pointAtUV( V2f( 1.0, 1.1 ), r ) )
		self.failIf( ipe.pointAtUV( V2f( 1.1, 1.1 ), r ) )
		self.failIf( ipe.pointAtUV( V2f( 1.0, -0.1 ), r ) )
		self.failIf( ipe.pointAtUV( V2f( -0.1, 1.0 ), r ) )
		self.failIf( ipe.pointAtUV( V2f( -0.1,  -0.1 ), r ) )
		
		self.failIf( ipe.pointAtPixel( V2i( -1, -1 ), r ) )		
		self.failIf( ipe.pointAtPixel( V2i( -1, 0 ), r ) )
		self.failIf( ipe.pointAtPixel( V2i( 0, -1 ), r ) )
		self.failIf( ipe.pointAtPixel( V2i( 0, 100 ), r ) )
		self.failIf( ipe.pointAtPixel( V2i( 100, 0 ), r ) )		
		self.failIf( ipe.pointAtPixel( V2i( 100, 100 ), r ) )						
		
		
	def testSimpleImage( self ) :	
		""" Test ImagePrimitiveEvaluator with simple gradient"""	
		
		random.seed( 1 )
	
		# Get an image which goes from black to red in the ascending X direction,
		# and from black to green in the ascending Y direction, and check that
		# its oriented correctly.
	
		reader = Reader.create( "test/IECore/data/exrFiles/uvMap.512x256.exr" )
		img = reader.read()
		
		ipe = PrimitiveEvaluator.create( img )		
		self.assert_( ipe.isInstanceOf( "ImagePrimitiveEvaluator" ) )
		
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf( ipe.A() )
		
		self.assertEqual( ipe.surfaceArea(), 2 * 512 * 256 )
		self.assertEqual( ipe.volume(), 0.0 )
		self.assertEqual( ipe.centerOfGravity(), V3f( 0, 0, 0 ) )
		
		r = ipe.createResult()
		
		foundClosest = ipe.pointAtPixel( V2i( 255, 127 ), r )
		self.assert_( foundClosest )
		self.assertEqual( r.point(), V3f( -0.5, -0.5, 0.0 ) )

		foundClosest = ipe.pointAtPixel( V2i( 256, 128 ), r )
		self.assert_( foundClosest )
		self.assertEqual( r.point(), V3f( 0.5, 0.5, 0.0 ) )
		
		foundClosest = ipe.closestPoint( V3f( 0, 0, 0 ), r )
		self.assert_( foundClosest )
		self.assertEqual( r.uv(), V2f( 0.5, 0.5 ) )
		self.assertEqual( r.normal(), V3f( 0.0, 0.0, 1.0 ) )
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )	
		self.assert_( ( V3f( colorR, colorG, colorB ) - V3f( 0.5, 0.5, 0.0 ) ).length() < 1.e-3 ) 
				
		foundClosest = ipe.closestPoint( V3f( -256, 127.5, 0 ), r )
		self.assert_( foundClosest )			
		self.assertEqual( r.uv(), V2f( 0.0, 1.0 - 0.5 / 256 ) )
		self.assertEqual( r.normal(), V3f( 0.0, 0.0, 1.0 ) )
		self.assertEqual( r.point(), V3f( -256, 127.5, 0 ) )
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )		
		colorB = r.floatPrimVar( ipe.B() )
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 0.0, 1.0, 0.0 ) ) 
		
		foundClosest = ipe.closestPoint( V3f( 256, -128, 0 ), r )		
		self.assert_( foundClosest )		
		self.assertEqual( r.uv(), V2f( 1.0, 0.0 ) )
		self.assertEqual( r.normal(), V3f( 0.0, 0.0, 1.0 ) )	
		self.assertEqual( r.point(), V3f( 256, -128, 0 ) )
		colorR = r.floatPrimVar( ipe.R() )		
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 1.0, 0.0, 0.0 ) ) 		
		
		foundClosest = ipe.closestPoint( V3f( 256, 128, 0 ), r )		
		self.assert_( foundClosest )	
		self.assertEqual( r.uv(), V2f( 1.0, 1.0 ) )
		self.assertEqual( r.normal(), V3f( 0.0, 0.0, 1.0 ) )				
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 1.0, 1.0, 0.0 ) )
		
		found = ipe.pointAtPixel( V2i( 511, 255 ), r )
		self.assert_( found )
		self.assertEqual( r.pixel(), V2i( 511, 255 ) )	
		self.assertEqual( r.uv(), V2f( 1.0 - 0.5 / 512, 1.0 - 0.5 / 256 ) )
		self.assertEqual( r.normal(), V3f( 0.0, 0.0, 1.0 ) )				
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )		
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 1.0, 1.0, 0.0 ) )
		
		foundClosest = ipe.pointAtUV( V2f( 0.5, 0.5 ), r )		
		self.assert_( foundClosest )
		self.assertEqual( r.point(), V3f( 0.0, 0.0, 0.0 ) )
		self.assertEqual( r.uv(), V2f( 0.5, 0.5 ) )
		self.assertEqual( r.normal(), V3f( 0.0, 0.0, 1.0 ) )				
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )		
		self.assert_( ( V3f( colorR, colorG, colorB ) - V3f( 0.5, 0.5, 0.0 ) ).length() < 1.e-3 ) 
		
		# Check that red ascends over X
		randomY = int( random.uniform ( 0, 255 ) )
		lastR = None
		for x in range( 0, 511 ):
		
			found = ipe.pointAtPixel( V2i( x, randomY ), r )
			self.assert_( found )
			
			if lastR :
				self.assert_( r.floatPrimVar( ipe.R() ) > lastR )
							
			lastR = r.floatPrimVar( ipe.R() )
				
		# Check that green ascends over Y
		randomX = int( random.uniform ( 0, 511 ) )
		lastG = None
		for y in range( 0, 255 ):
		
			found = ipe.pointAtPixel( V2i( randomX, y ), r )
			self.assert_( found )
			
			if lastG :
			
				self.assert_( r.floatPrimVar( ipe.G() ) > lastG )
				
			lastG = r.floatPrimVar( ipe.G() )
				
	def testDataWindow( self ) :		
		""" Test ImagePrimitiveEvaluator with a data window """	
		
		displayWindow = Box2i( V2i( 0, 0 ), V2i( 99, 99 ) )
		dataWindow = Box2i( V2i( 50, 50), V2i( 99, 99 ) )
		img = ImagePrimitive( dataWindow, displayWindow )

	
		dataWindowArea = 50 * 50
		R = FloatVectorData( dataWindowArea )
		G = FloatVectorData( dataWindowArea ) 
		B = FloatVectorData( dataWindowArea )
		
		colorMap = {}
		offset = 0
		for y in range( 0, 50 ):
			for x in range( 0, 50 ) :
				
				red = float(x+50) / 100.0
				green = float(y+50) / 100.0				
				
				R[offset] = red
				G[offset] = green
				B[offset] = 0.5
				
				colorMap[ (x+50,y+50) ] = V3f( red, green, 0.5 )
				
				offset = offset + 1
								
									
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )	
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )
		
		self.assert_( img.arePrimitiveVariablesValid() )
		
		ipe = PrimitiveEvaluator.create( img )		
		self.assert_( ipe.isInstanceOf( "ImagePrimitiveEvaluator" ) )
		
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf( ipe.A() )
		
		r = ipe.createResult()
		
		for y in range( 0, 100 ):
			for x in range( 0, 100 ) :			
				found = ipe.pointAtPixel( V2i( x, y ), r )
				self.assert_( found )
									
				self.assert_( ( r.point() - V3f( (x + 0.5 - 50) , (y + 0.5 - 50), 0 ) ).length() < 0.1 )
		
				c = V3f( r.floatPrimVar( ipe.R() ), r.floatPrimVar( ipe.G() ), r.floatPrimVar( ipe.B() ) )	
				
				expectedColor = V3f( 0, 0, 0 )
				
				if (x, y) in colorMap :
					
					expectedColor = colorMap[ (x, y) ]
			
				self.assert_( ( c - expectedColor ).length() < 0.001 )
				
	def testUpscale( self ):	
		""" Upscale a subregion of an image, testing interpolation of primvar evaluation """
	
		reader = Reader.create( "test/IECore/data/tiff/maya.tiff" )
		img = reader.read()
		
		ipe = PrimitiveEvaluator.create( img )		
		self.assert_( ipe.isInstanceOf( "ImagePrimitiveEvaluator" ) )
		
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		
		b = img.bound()
		
		newImage = ImagePrimitive( img.dataWindow, img.dataWindow )
		
		newWidth = img.displayWindow.max.x - img.displayWindow.min.x + 1
		newHeight = img.displayWindow.max.y - img.displayWindow.min.y + 1
		dataWindowArea = newWidth * newHeight
		R = FloatVectorData( dataWindowArea )
		G = FloatVectorData( dataWindowArea ) 
		B = FloatVectorData( dataWindowArea )
		
		newImage["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		newImage["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )	
		newImage["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )		

		r = ipe.createResult()
		
		offset = 0
		for y in range( -newHeight / 2, newHeight / 2 ) :

			for x in range( -newWidth / 2, newWidth / 2 ) :
			
				ipe.closestPoint( V3f( x / 2.0, y / 2.0, 0.0 ), r )
				
				R[offset] = r.floatPrimVar( ipe.R() )
				G[offset] = r.floatPrimVar( ipe.G() )
				B[offset] = r.floatPrimVar( ipe.B() )
							
				offset = offset + 1
		
		
		expectedImage = Reader.create( "test/IECore/data/expectedResults/imagePrimitiveEvaluatorUpscale.tiff" ).read()
		
		op = ImageDiffOp()
		
		res = op(
			imageA = newImage,
			imageB = expectedImage,
			skipMissingChannels = True,
			maxError = 0.0001
		)
		
		self.failIf( res.value )
		 
					
if __name__ == "__main__":
	unittest.main()
	
