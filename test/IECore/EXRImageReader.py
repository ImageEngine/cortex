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
import sys
from IECore import *
import os

class TestEXRReader(unittest.TestCase):

	def testFactoryConstruction( self ) :

		r = Reader.create( "test/IECore/data/exrFiles/AllHalfValues.exr" )
		self.assertEqual( type( r ), EXRImageReader )

	def testCanReadAndIsComplete( self ) :
		
		self.assert_( EXRImageReader.canRead( "test/IECore/data/exrFiles/AllHalfValues.exr" ) )
		self.assert_( not EXRImageReader.canRead( "thisFileDoesntExist.exr" ) )
		
		r = EXRImageReader( "test/IECore/data/exrFiles/AllHalfValues.exr" )
		self.assert_( r.isComplete() )
		
		r = EXRImageReader( "test/IECore/data/exrFiles/incomplete.exr" )
		self.assert_( not r.isComplete() )
		
		r = EXRImageReader( "thisFileDoesntExist.exr" )
		self.assert_( not r.isComplete() )
		
	def testChannelNames( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/AllHalfValues.exr" )
		c = r.channelNames()
		self.assert_( c.staticTypeId()==StringVectorData.staticTypeId() )
		self.assert_( len( c ), 3 )
		self.assert_( "R" in c )
		self.assert_( "G" in c )
		self.assert_( "B" in c )
		
		r = EXRImageReader( "test/IECore/data/exrFiles/manyChannels.exr" )
		c = r.channelNames()
		self.assert_( c.staticTypeId()==StringVectorData.staticTypeId() )
		self.assert_( len( c ), 7 )
		self.assert_( "R" in c )
		self.assert_( "G" in c )
		self.assert_( "B" in c )
		self.assert_( "A" in c )
		self.assert_( "diffuse.red" in c )
		self.assert_( "diffuse.green" in c )
		self.assert_( "diffuse.blue" in c )
		
		r = EXRImageReader( "thisFileDoesntExist.exr" )
		self.assertRaises( Exception, r.channelNames )

	def testReadHeader( self ):

		r = EXRImageReader( "test/IECore/data/exrFiles/manyChannels.exr" )
		h = r.readHeader()

		c = h['channelNames']		
		self.assert_( c.staticTypeId()==StringVectorData.staticTypeId() )
		self.assert_( len( c ), 7 )
		self.assert_( "R" in c )
		self.assert_( "G" in c )
		self.assert_( "B" in c )
		self.assert_( "A" in c )
		self.assert_( "diffuse.red" in c )
		self.assert_( "diffuse.green" in c )
		self.assert_( "diffuse.blue" in c )

		self.assertEqual( h['displayWindow'], Box2iData( Box2i( V2i(0,0), V2i(255,255) ) ) )
		self.assertEqual( h['dataWindow'], Box2iData( Box2i( V2i(0,0), V2i(255,255) ) ) )
	
	def testDataAndDisplayWindows( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/AllHalfValues.exr" )
		self.assertEqual( r.dataWindow(), Box2i( V2i( 0 ), V2i( 255 ) ) )
		self.assertEqual( r.displayWindow(), Box2i( V2i( 0 ), V2i( 255 ) ) )
		
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMapWithDataWindow.100x100.exr" )
		self.assertEqual( r.dataWindow(), Box2i( V2i( 25 ), V2i( 49 ) ) )
		self.assertEqual( r.displayWindow(), Box2i( V2i( 0 ), V2i( 99 ) ) )
		
		r = EXRImageReader( "thisFileDoesntExist.exr" )
		self.assertRaises( Exception, r.dataWindow )
		self.assertRaises( Exception, r.displayWindow )
	
	def testReadImage( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMap.256x256.exr" )
		
		i = r.read()
		self.assertEqual( i.typeId(), ImagePrimitive.staticTypeId() )
				
		self.assertEqual( i.dataWindow, Box2i( V2i( 0 ), V2i( 255 ) ) )
		self.assertEqual( i.displayWindow, Box2i( V2i( 0 ), V2i( 255 ) ) )
		
		self.assert_( i.arePrimitiveVariablesValid() )
		
		self.assertEqual( len( i ), 3 )
		for c in ["R", "G", "B"] :
			self.assertEqual( i[c].data.typeId(), FloatVectorData.staticTypeId() )
			
		r = i["R"].data
		self.assertEqual( r[0], 0 )
		self.assertEqual( r[-1], 1 )
		g = i["G"].data
		self.assertEqual( r[0], 0 )
		self.assertEqual( r[-1], 1 )
		for b in i["B"].data :
			self.assertEqual( b, 0 )
			
	def testReadIndividualChannels( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMap.256x256.exr" )
		i = r.read()
		
		for c in ["R", "G", "B"] :
		
			cd = r.readChannel( c )
			self.assertEqual( i[c].data, cd )
	
	def testReadWithChangedDisplayWindow( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMap.256x256.exr" )
		i1 = r.read()
		
		r.parameters().displayWindow.setTypedValue( Box2i( V2i( -1000, -10 ), V2i( 1000, 10 ) ) ) 
		i2 = r.read()
		
		self.assertEqual( i2.displayWindow, Box2i( V2i( -1000, -10 ), V2i( 1000, 10 ) ) )
		self.assertEqual( i2.dataWindow, Box2i( V2i( 0 ), V2i( 255 ) ) )
		
		i2.displayWindow = i1.displayWindow
		self.assertEqual( i1, i2 )
	
	def testReadInvalidDataWindow( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMap.512x256.exr" )
		r.parameters().dataWindow.setTypedValue( Box2i( V2i( -1 ), V2i( 511, 255 ) ) )

		self.assertRaises( Exception, r.read )
		
	def testReadHorizontalSlices( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMap.512x256.exr" )
		iWhole = r.read()
		
		# read and test a horizontal slice starting at y==0
		r.parameters().dataWindow.setTypedValue( Box2i( V2i( 0, 0 ), V2i( 511, 100 ) ) )
		
		iSliced = r.read()
		self.assertEqual( iSliced.dataWindow, Box2i( V2i( 0, 0 ), V2i( 511, 100 ) ) )
		self.assertEqual( iSliced.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )
		
		self.assert_( iSliced.arePrimitiveVariablesValid() )
		
		for i in range( 0, len( iSliced["R"].data ) ) :
		
			self.assertEqual( iSliced["R"].data[i], iWhole["R"].data[i] )
			self.assertEqual( iSliced["G"].data[i], iWhole["G"].data[i] )
			self.assertEqual( iSliced["B"].data[i], iWhole["B"].data[i] )
		
		# read and test a horizontal slice ending at the end of the image
		r.parameters().dataWindow.setTypedValue( Box2i( V2i( 0, 200 ), V2i( 511, 255 ) ) )
		
		iSliced = r.read()
		self.assertEqual( iSliced.dataWindow, Box2i( V2i( 0, 200 ), V2i( 511, 255 ) ) )
		self.assertEqual( iSliced.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )
		
		for i in range( -1, -len( iSliced["R"].data ) ) :
			self.assertEqual( iSliced["R"].data[i], iWhole["R"].data[i] )
			self.assertEqual( iSliced["G"].data[i], iWhole["G"].data[i] )
			self.assertEqual( iSliced["B"].data[i], iWhole["B"].data[i] )
	
	def testReadArbitrarySlice( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMap.512x256.exr" )
		iWhole = r.read()
		
		r.parameters().dataWindow.setTypedValue( Box2i( V2i( 10, 10 ), V2i( 100, 150 ) ) )
		
		iSliced = r.read()
		self.assertEqual( iSliced.displayWindow, Box2i( V2i( 0, 0 ), V2i( 511, 255 ) ) )
		self.assertEqual( iSliced.dataWindow, Box2i( V2i( 10, 10 ), V2i( 100, 150 ) ) )

		self.assert_( iSliced.arePrimitiveVariablesValid() )
		
		wholeEvaluator = PrimitiveEvaluator.create( iWhole )
		slicedEvaluator = PrimitiveEvaluator.create( iSliced )
		wholeResult = wholeEvaluator.createResult()
		slicedResult = slicedEvaluator.createResult()
		wholeR = wholeEvaluator.R()
		wholeG = wholeEvaluator.G()
		wholeB = wholeEvaluator.B()
		slicedR = slicedEvaluator.R()
		slicedG = slicedEvaluator.G()
		slicedB = slicedEvaluator.B()
				
		for x in range( 10, 101 ) :
			for y in range( 10, 151 ) :
				
				wholeEvaluator.pointAtPixel( V2i( x, y ), wholeResult )
				slicedEvaluator.pointAtPixel( V2i( x, y ), slicedResult )
				
				self.assertEqual( wholeResult.floatPrimVar( wholeR ), slicedResult.floatPrimVar( slicedR ) )
				self.assertEqual( wholeResult.floatPrimVar( wholeG ), slicedResult.floatPrimVar( slicedG ) )
				self.assertEqual( wholeResult.floatPrimVar( wholeB ), slicedResult.floatPrimVar( slicedB ) )
	
	def testNonZeroDataWindowOrigin( self ) :
	
		r = EXRImageReader( "test/IECore/data/exrFiles/uvMapWithDataWindow.100x100.exr" )
		i = r.read()
		
		self.assertEqual( i.dataWindow, Box2i( V2i( 25 ), V2i( 49 ) ) )
		self.assertEqual( i.displayWindow, Box2i( V2i( 0 ), V2i( 99 ) ) )

		self.assert_( i.arePrimitiveVariablesValid() )
		
		r.parameters().dataWindow.setTypedValue( Box2i( V2i( 30 ), V2i( 40 ) ) )
		iSliced = r.read()
		
		self.assertEqual( iSliced.dataWindow, Box2i( V2i( 30 ), V2i( 40 ) ) )
		self.assertEqual( iSliced.displayWindow, Box2i( V2i( 0 ), V2i( 99 ) ) )
		
		wholeEvaluator = PrimitiveEvaluator.create( i )
		slicedEvaluator = PrimitiveEvaluator.create( iSliced )
		wholeResult = wholeEvaluator.createResult()
		slicedResult = slicedEvaluator.createResult()
		wholeR = wholeEvaluator.R()
		wholeG = wholeEvaluator.G()
		wholeB = wholeEvaluator.B()
		slicedR = slicedEvaluator.R()
		slicedG = slicedEvaluator.G()
		slicedB = slicedEvaluator.B()
				
		for x in range( 30, 41 ) :
			for y in range( 30, 41 ) :
				
				wholeEvaluator.pointAtPixel( V2i( x, y ), wholeResult )
				slicedEvaluator.pointAtPixel( V2i( x, y ), slicedResult )
				
				self.assertAlmostEqual( wholeResult.floatPrimVar( wholeR ), slicedResult.floatPrimVar( slicedR ), 4 )
				self.assertAlmostEqual( wholeResult.floatPrimVar( wholeG ), slicedResult.floatPrimVar( slicedG ), 4 )
				self.assertAlmostEqual( wholeResult.floatPrimVar( wholeB ), slicedResult.floatPrimVar( slicedB ), 4 )
				
	def testOrientation( self ) :
	
		img = Reader.create( "test/IECore/data/exrFiles/uvMap.512x256.exr" ).read()
		ipe = PrimitiveEvaluator.create( img )
		r = ipe.createResult()
		
		pointColors = {
			V2i(0, 0) : V3f( 0, 0, 0 ),
			V2i(511, 0) : V3f( 1, 0, 0 ),
			V2i(0, 255) : V3f( 0, 1, 0 ),
			V2i(511, 255) : V3f( 1, 1, 0 ),
		}
		
		for point, expectedColor in pointColors.items() :
		
			ipe.pointAtPixel( point, r )
			
			color = V3f( r.floatPrimVar( ipe.R() ), r.floatPrimVar( ipe.G() ), r.floatPrimVar( ipe.B() ) )
			
			self.assert_( ( color - expectedColor).length() < 1.e-6 )
			
	def testReadIncompleteImage( self ) :
	
		m = CapturingMessageHandler() 
		s = ScopedMessageHandler( m )
	
		r = EXRImageReader( "test/IECore/data/exrFiles/incomplete.exr" )
		i = r.read()
		
		# check image is valid even if not all data is present
		self.assertEqual( len( i ), 3 )
		self.assert_( "R" in i )
		self.assert_( "G" in i )
		self.assert_( "B" in i )
		
		self.assert_( i.arePrimitiveVariablesValid() )
		
		# check a warning message has been output for each channel
		self.assertEqual( len( m.messages ), 3 )
		self.assertEqual( m.messages[0].level, Msg.Level.Warning )
		self.assertEqual( m.messages[1].level, Msg.Level.Warning )
		self.assertEqual( m.messages[2].level, Msg.Level.Warning )
		

if __name__ == "__main__":
	unittest.main()   

