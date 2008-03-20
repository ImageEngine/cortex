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
import glob
import sys
import os
from IECore import *

class TestJPEGImageWriter(unittest.TestCase):

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
		
	def __makeGreyscaleImage( self, dataWindow, displayWindow ) :
	
		img = ImagePrimitive( dataWindow, displayWindow )
		
		w = dataWindow.max.x - dataWindow.min.x + 1
		h = dataWindow.max.y - dataWindow.min.y + 1
		
		area = w * h
		Y = FloatVectorData( area )
		
		offset = 0
		for y in range( 0, h ) :
			for x in range( 0, w ) :
			
				Y[offset] = float(x) / (w - 1)	* float(y) / (h - 1)			
				
				offset = offset + 1				
		
		img["Y"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, Y )
		
		return img	

	def testConstruction( self ):
		
		img = ImagePrimitive()	
		self.assert_( JPEGImageWriter.canWrite(  img, "test/IECore/data/jpg/output.jpg" ) )		
		w = Writer.create( img, "test/IECore/data/jpg/output.jpg" )
		self.assertEqual( type(w), JPEGImageWriter )
		
	def testQuality ( self ) :
		
		w = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99)
		)	
	
		img = self.__makeImage( w, w )				
		w = Writer.create( img, "test/IECore/data/jpg/output.jpg" )
		self.assertEqual( type(w), JPEGImageWriter )
		
		qualitySizeMap = {}
		lastSize = None
		for q in [ 0, 10, 50, 80, 100 ]:
		
			w.parameters().quality = q
			
			if os.path.exists( "test/IECore/data/jpg/output.jpg" ) :
			
				os.remove( "test/IECore/data/jpg/output.jpg" )
				
			w.write()
			self.assert_( os.path.exists( "test/IECore/data/jpg/output.jpg" ) )
			
			size = os.path.getsize( "test/IECore/data/jpg/output.jpg" )
			qualitySizeMap[q] = size
						
			if lastSize :
			
				self.assert_( size >= lastSize )
				
			lastSize = size
			
		self.assert_( qualitySizeMap[100] > qualitySizeMap[0] )	
			
		
	def testWrite( self ) :	
		
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)
		
		dataWindow = displayWindow
		
		img = self.__makeImage( dataWindow, displayWindow )
		
		w = Writer.create( img, "test/IECore/data/jpg/output.jpg" )
		self.assertEqual( type(w), JPEGImageWriter )
		
		w.write()
		
		self.assert_( os.path.exists( "test/IECore/data/jpg/output.jpg" ) )
		self.assertEqual( os.path.getsize( "test/IECore/data/jpg/output.jpg" ), 4559 )
		
	def testGreyscaleWrite( self ) :
	
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 199, 99 )
		)
		
		dataWindow = displayWindow	
		
		img = self.__makeGreyscaleImage( dataWindow, displayWindow )
		
		w = Writer.create( img, "test/IECore/data/jpg/output.jpg" )
		self.assertEqual( type(w), JPEGImageWriter )
		
		w.write()
		
		self.assert_( os.path.exists( "test/IECore/data/jpg/output.jpg" ) )
		
		r = Reader.create( "test/IECore/data/jpg/output.jpg" )
		img2 = r.read()
		
		channelNames = r.channelNames()
		self.assertEqual( len(channelNames), 1 )
		
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
		
		w = Writer.create( img, "test/IECore/data/jpg/output.jpg" )
		self.assertEqual( type(w), JPEGImageWriter )
		
		self.assertRaises( RuntimeError, w.write )				
		self.failIf( os.path.exists( "test/IECore/data/jpg/output.jpg" ) )
		
	def testErrors( self ) :
		
		displayWindow = Box2i(
			V2i( 0, 0 ),
			V2i( 99, 99 )
		)
		
		dataWindow = displayWindow
		
		
		# Try and write an image with the "R" channel of an unsupported type
		img = self.__makeImage( dataWindow, displayWindow )	
		img[ "R" ] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, StringData( "hello") )
		
		w = Writer.create( img, "test/IECore/data/jpg/output.jpg" )
		self.assertEqual( type(w), JPEGImageWriter )
		
		self.assertRaises( RuntimeError, w.write )
		
		
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
		
		w = Writer.create( img, "test/IECore/data/jpg/output.jpg" )
		self.assertEqual( type(w), JPEGImageWriter )		
		w.write()
		
		self.assert_( os.path.exists( "test/IECore/data/jpg/output.jpg" ) )
				
		r = Reader.create( "test/IECore/data/jpg/output.jpg" )
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
		expectedColor = V3f( 0, 0, 0 )
		self.assert_( ( color - expectedColor).length() < 1.e-3 )
		
		found = ipe.pointAtPixel( V2i( 110, 110 ), result )
		self.assert_( found )		
		color = V3f(
				result.halfPrimVar( ipe.R() ),
				result.halfPrimVar( ipe.G() ), 
				result.halfPrimVar( ipe.B() )
			)		
		expectedColor = V3f( 0.913574, 0.909668, 0 )
		self.assert_( ( color - expectedColor).length() < 1.e-3 )	
		
		
	def setUp( self ) :

		if os.path.exists( "test/IECore/data/jpg/output.jpg" ) :
			
			os.remove( "test/IECore/data/jpg/output.jpg" )					
		
		
	def tearDown( self ) :

		if os.path.exists( "test/IECore/data/jpg/output.jpg" ) :
			
			os.remove( "test/IECore/data/jpg/output.jpg" )
			
				
		
	
		
if __name__ == "__main__":
	unittest.main()   
	
