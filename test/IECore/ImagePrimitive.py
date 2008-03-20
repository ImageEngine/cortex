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
import os

from IECore import *

class TestImagePrimitive( unittest.TestCase ) :

	def testConstructor( self ) :
		""" Test ImagePrimitive constructor """
	
		windowMin = V2i( 0, 0 )
		windowMax = V2i( 100, 100 )
		w = Box2i( windowMin, windowMax )
		i = ImagePrimitive( w, w )
		
		self.assertEqual( i.dataWindow, w )
		self.assertEqual( i.displayWindow, w )
						
		i.dataWindow = Box2i( windowMin, V2i( 10, 10 ) )
		self.assertEqual( i.dataWindow, Box2i( windowMin, V2i( 10, 10 ) ) )
		self.assertEqual( i.displayWindow, w )		
		
		i.displayWindow = Box2i( windowMin, V2i( 10, 10 ) )
		self.assertEqual( i.displayWindow, Box2i( windowMin, V2i( 10, 10 ) ) )
					
	def testDataWindow( self ) :			
	
		displayWindow = Box2i( V2i( 0, 0 ), V2i( 99, 99 ) )
		dataWindow = Box2i( V2i( 50, 50), V2i( 99, 99 ) )
		img = ImagePrimitive( dataWindow, displayWindow )
		
	def testBound( self ) :
		""" Test ImagePrimitive bound """
		
		windowMin = V2i( 0, 0 )
		windowMax = V2i( 99, 99 )
		w = Box2i( windowMin, windowMax )
		i = ImagePrimitive( w, w )
		
		self.assertEqual( i.bound(), Box3f( V3f( -50, -50, 0 ), V3f( 50, 50, 0 ) ) )
		
		windowMin = V2i( 50, 50 )
		windowMax = V2i( 99, 99 )
		w = Box2i( windowMin, windowMax )
		i = ImagePrimitive( w, w )
		
		self.assertEqual( i.bound(), Box3f( V3f( -25, -25, 0 ), V3f( 25, 25, 0 ) ) )	
					
	def testDataWindow( self ) :
		""" Test ImagePrimitive data window """			
	
		displayWindow = Box2i( V2i( 0, 0 ), V2i( 99, 99 ) )
		dataWindow = Box2i( V2i( 50, 50), V2i( 99, 99 ) )
		img = ImagePrimitive( dataWindow, displayWindow )
		
		dataWindowArea = 50 * 50
		R = FloatVectorData( dataWindowArea )
		G = FloatVectorData( dataWindowArea ) 
		B = FloatVectorData( dataWindowArea )
				
		img["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, G )	
		img["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, B )
		
		self.assert_( img.arePrimitiveVariablesValid() )
		
		# \todo Verify behaviour when dataWindow and displayWindow are contradictory or inconsistent
		
	def testLoadSave( self ):
		""" Test ImagePrimitive load/save """
	
		windowMin = V2i( 0, 0 )
		windowMax = V2i( 100, 100 )
		w = Box2i( windowMin, windowMax )
		i = ImagePrimitive( w, w )	
		
		Writer.create( i, "test/IECore/data/output.cob" ).write()
		
		i2 = Reader.create( "test/IECore/data/output.cob" ).read()
		self.assertEqual( type(i2), ImagePrimitive )
		
		self.assertEqual( i.displayWindow, i2.displayWindow )
		self.assertEqual( i.dataWindow, i2.dataWindow )		
		
	def testChannelNames( self ) :	
	
		""" Test ImagePrimitive channel names """
	
		windowMin = V2i( 0, 0 )
		windowMax = V2i( 99, 99 )
		w = Box2i( windowMin, windowMax )
		i = ImagePrimitive( w, w )
		
		r = FloatVectorData()
		r.resize( 100 * 100 )
		
		i["R"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, r )
		
		self.assert_( "R" in i.channelNames() )
		
		b = FloatData()
		
		i["B"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, b )
		
		self.assert_( "B" in i.channelNames() )
		
		self.assert_( i.arePrimitiveVariablesValid() )
		
		# Deliberately make a primvar too small!
		g = FloatVectorData()
		g.resize( 50 * 100 )

		i["G"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, g )
		
		self.failIf( "G" in i.channelNames() )
		
		self.failIf( i.arePrimitiveVariablesValid() )
		
	def testCreateChannel( self ):
	
		windowMin = V2i( 0, 0 )
		windowMax = V2i( 99, 99 )
		w = Box2i( windowMin, windowMax )
		i = ImagePrimitive( w, w )
		
		i.createFloatChannel( "R" )
		i.createHalfChannel( "G" )
		i.createUIntChannel( "B" )
		
		self.assert_( "R" in i )
		self.assert_( "G" in i )
		self.assert_( "B" in i )				
		
	def testErrors( self ):
	
		windowMin = V2i( 0, 0 )
		windowMax = V2i( 99, 99 )
		w = Box2i( windowMin, windowMax )
		i = ImagePrimitive( w, w )
		
		empty = Box2i()
		
		self.assertRaises( RuntimeError, setattr, i, "displayWindow", empty )
		
		self.assertRaises( RuntimeError, ImagePrimitive, empty, empty )	
		
		
	def tearDown( self ) :
	
		if os.path.exists( "test/IECore/data/output.cob" ) :
		
			os.remove( "test/IECore/data/output.cob" )	
			
		
if __name__ == "__main__":
    unittest.main()   
