##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
import sys, os
from IECore import *

from math import pow

class TestDPXWriter(unittest.TestCase):
	
	def __verifyImageRGB( self, img ):
	
		self.assertEqual( type(img), ImagePrimitive )	
	
		topLeft =  img.dataWindow.min - img.displayWindow.min
		bottomRight = img.dataWindow.max - img.displayWindow.min
		topRight = V2i( img.dataWindow.max.x, img.dataWindow.min.y) - img.displayWindow.min
		bottomLeft = V2i( img.dataWindow.min.x, img.dataWindow.max.y) - img.displayWindow.min
	
		pixelColorMap = {
			topLeft : V3f( 0, 0, 0 ),
			bottomRight : V3f( 1, 1, 0 ),
			topRight: V3f( 1, 0, 0 ),
			bottomLeft: V3f( 0, 1, 0 ),			
		}
	
		ipe = PrimitiveEvaluator.create( img )
		result = ipe.createResult()	
		
		for pixelColor in pixelColorMap.items() :
		
			found = ipe.pointAtPixel( pixelColor[0], result )
			self.assert_( found )		
			color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)	
								
			self.assert_( ( color - pixelColor[1]).length() < 1.e-3 )
			
	def __makeImage( self, dataWindow, displayWindow ) :
	
		img = ImagePrimitive( dataWindow, displayWindow )
		
		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1
		
		area = w * h
		R = FloatVectorData( area )
		G = FloatVectorData( area )		
		B = FloatVectorData( area )
		
		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :
			
				R[offset] = float(x) / (w - 1)				
				G[offset] = float(y) / (h - 1)
				B[offset] = 0.0
				
				offset = offset + 1				
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )		
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )
		
		return img
				
	
	def __makeFloatImage( self, dataWindow, displayWindow, withAlpha = False, dataType = FloatVectorData ) :
	
		img = ImagePrimitive( dataWindow, displayWindow )
		
		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1
		
		area = w * h
		R = dataType( area )
		G = dataType( area )		
		B = dataType( area )
		
		if withAlpha:
			A = dataType( area )
		
		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :
			
				R[offset] = float(x) / (w - 1)				
				G[offset] = float(y) / (h - 1)
				B[offset] = 0.0
				if withAlpha:
					A[offset] = 0.5
				
				offset = offset + 1				
		
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )		
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )
		
		if withAlpha:
			img["A"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, A )
		
		return img
		
	def testWrite( self ) :	
		
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)
		
		dataWindow = displayWindow
		
		for dataType in [ FloatVectorData, HalfVectorData, DoubleVectorData ] :
		
			self.setUp()
		
			img = self.__makeFloatImage( dataWindow, displayWindow, dataType = dataType )
			w = Writer.create( img, "test/IECore/data/dpx/output.dpx" )
			self.assertEqual( type(w), DPXImageWriter )
			w.write()
		
			self.assert_( os.path.exists( "test/IECore/data/dpx/output.dpx" ) )
			
			# Now we've written the image, verify the rgb
			
			img2 = Reader.create( "test/IECore/data/dpx/output.dpx" ).read()
			self.__verifyImageRGB( img2 )
			
			self.tearDown()	

	def testColorConversion(self):

		r = Reader.create( "test/IECore/data/dpx/ramp.dpx" )
		img = r.read()
		self.assertEqual( type(img), ImagePrimitive )
		w = Writer.create( img, "test/IECore/data/dpx/output.dpx" )
		self.assertEqual( type(w), DPXImageWriter )
		w.write()
		w = None
		r = Reader.create( "test/IECore/data/dpx/output.dpx" )
		img2 = r.read()
		self.assertEqual( type(img2), ImagePrimitive )
		self.assertEqual( img, img2 )
		
	def testWriteIncomplete( self ) :
	
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)
		
		dataWindow = displayWindow
		
		img = self.__makeImage( dataWindow, displayWindow )
		
		# We don't have enough data to fill this dataWindow
		img.dataWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 199, 199 )
		)
		
		self.failIf( img.arePrimitiveVariablesValid() )
		
		w = Writer.create( img, "test/IECore/data/dpx/output.dpx" )
		self.assertEqual( type(w), DPXImageWriter )
		
		self.assertRaises( RuntimeError, w.write )				
		self.failIf( os.path.exists( "test/IECore/data/dpx/output.dpx" ) )		
		
	def testWindowWrite( self ) :	
	
		dataWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)

		img = self.__makeImage( dataWindow, dataWindow )
		
		img.displayWindow = Box2i(
			V2i( -20, -20 ),
			V2i( 199, 199 )
		)
		
		w = Writer.create( img, "test/IECore/data/dpx/output.dpx" )
		self.assertEqual( type(w), DPXImageWriter )		
		w.write()
		
		w = Writer.create( img, "test/IECore/data/dpx/output2.dpx" )
		self.assertEqual( type(w), DPXImageWriter )		
		w.write()
		
		self.assert_( os.path.exists( "test/IECore/data/dpx/output.dpx" ) )
				
		r = Reader.create( "test/IECore/data/dpx/output.dpx" )
		img2 = r.read()
		
		self.assertEqual( img2.displayWindow.min, V2i( 0, 0 ) )			
		self.assertEqual( img2.displayWindow.max, V2i( 219, 219 ) )
		
		ipe = PrimitiveEvaluator.create( img2 )
		self.assert_( ipe.R() )
		self.assert_( ipe.G() )
		self.assert_( ipe.B() )
		self.failIf ( ipe.A() )
		
		result = ipe.createResult()
				
		# Check that image has been written correctly, accounting for the change in display window
		found = ipe.pointAtPixel( V2i( 19, 19 ), result )
		self.assert_( found )		
		color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)
					
		# \todo Check	
		expectedColor = V3f( -0.0056, -0.0056, -0.0056 )
		self.assert_( ( color - expectedColor).length() < 1.e-3 )
		
		found = ipe.pointAtPixel( V2i( 110, 110 ), result )
		self.assert_( found )		
		color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)	
				
		# \todo Check		
		expectedColor = V3f( 0.911133, 0.911133, 0 )
		self.assert_( ( color - expectedColor).length() < 1.e-3 )
		
	def setUp( self ) :
	
		if os.path.isfile( "test/IECore/data/dpx/output.dpx") :
			os.remove( "test/IECore/data/dpx/output.dpx" )				

	def tearDown( self ) :
	
		if os.path.isfile( "test/IECore/data/dpx/output.dpx") :
			os.remove( "test/IECore/data/dpx/output.dpx" )
		
       			
if __name__ == "__main__":
	unittest.main()   
	
