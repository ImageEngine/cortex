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
		
		self.assertEqual( ipe.surfaceArea(), 2 * 100 * 100 )
		self.assertEqual( ipe.volume(), 0.0 )
		self.assertEqual( ipe.centerOfGravity(), V3f( 50, 50, 0 ) )
		
		r = ipe.createResult()
		
		foundClosest = ipe.closestPoint( V3f( 0, 50, 0 ), r )
		
		self.assert_( foundClosest )
		self.assertEqual( r.pixel(), V2i( 0, 49 ) )
		
		self.assert_( ( r.point() - V3f( 0, 50, 0 ) ).length() < 0.01 )
		self.assert_( ( r.uv() - V2f( 0.0, 0.5 ) ).length() < 0.001 )
		
		hit = ipe.intersectionPoint( V3f( 50, 50, 100 ), V3f( 0, 0, -1 ), r )
		self.assert_( hit )
		self.assert_( ( r.point() - V3f( 50, 50, 0 ) ).length() < 0.01 )
		self.assert_( ( r.uv() - V2f( 0.5, 0.5 ) ).length() < 0.001 )
		
		hits = ipe.intersectionPoints( V3f( 50, 50, 100 ), V3f( 0, 0, -1 ) )
		self.assertEqual( len(hits), 1 )
		
		hits = ipe.intersectionPoints( V3f( 150, 50, 100 ), V3f( 0, 0, -1 ) )
		self.assertEqual( len(hits), 0 )
		
		
	def testSimpleImage( self ) :	
	
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
		self.assertEqual( ipe.centerOfGravity(), V3f( 256, 128, 0 ) )
		
		r = ipe.createResult()
		
		foundClosest = ipe.closestPoint( V3f( 0, 0, 0 ), r )
		self.assert_( foundClosest )
		self.assertEqual( r.uv(), V2f( 0.0, 0.0 ) )
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )		
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 0.0, 0.0, 0.0 ) ) 
				
		foundClosest = ipe.closestPoint( V3f( 0, 256, 0 ), r )		
		self.assert_( foundClosest )			
		self.assertEqual( r.uv(), V2f( 0.0, 1.0 ) )
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )		
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 0.0, 1.0, 0.0 ) ) 
		
		foundClosest = ipe.closestPoint( V3f( 512, 0, 0 ), r )		
		self.assert_( foundClosest )		
		self.assertEqual( r.uv(), V2f( 1.0, 0.0 ) )
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )		
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 1.0, 0.0, 0.0 ) ) 		
		
		foundClosest = ipe.closestPoint( V3f( 512, 256, 0 ), r )		
		self.assert_( foundClosest )	
		self.assertEqual( r.uv(), V2f( 1.0, 1.0 ) )		
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )		
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 1.0, 1.0, 0.0 ) )
		
		found = ipe.pointAtPixel( V2i( 511, 255 ), r )
		self.assert_( found )
		self.assertEqual( r.pixel(), V2i( 511, 255 ) )	
		self.assertEqual( r.uv(), V2f( 1.0, 1.0 ) )		
		colorR = r.floatPrimVar( ipe.R() )
		colorG = r.floatPrimVar( ipe.G() )
		colorB = r.floatPrimVar( ipe.B() )		
		self.assertEqual( Color3f( colorR, colorG, colorB ), Color3f( 1.0, 1.0, 0.0 ) )
		
		# Check that red ascends over X
		randomY = int( random.uniform ( 0, 255 ) )
		lastR = None
		for x in range( 0, 512 ):
		
			found = ipe.pointAtPixel( V2i( x, randomY ), r )
			self.assert_( found )
			
			if lastR :
			
				self.assert_( r.floatPrimVar( ipe.R() ) > lastR )
				
			else :
				lastR = r.floatPrimVar( ipe.R() )
				
		# Check that green ascends over Y
		randomX = int( random.uniform ( 0, 511 ) )
		lastG = None
		for y in range( 0, 256 ):
		
			found = ipe.pointAtPixel( V2i( randomX, y ), r )
			self.assert_( found )
			
			if lastG :
			
				self.assert_( r.floatPrimVar( ipe.G() ) > lastG )
				
			else :
				lastG = r.floatPrimVar( ipe.G() )
		
			
		 
					
if __name__ == "__main__":
	unittest.main()
	
