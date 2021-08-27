##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

import os
import unittest
import imath

import IECore
import IECoreImage


class ImagePrimitiveTest( unittest.TestCase ) :
	def testConstructor( self ) :
		""" Test IECoreImage.ImagePrimitive constructor """

		windowMin = imath.V2i( 0, 0 )
		windowMax = imath.V2i( 100, 100 )
		w = imath.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		self.assertEqual( i.dataWindow, w )
		self.assertEqual( i.displayWindow, w )

		i.dataWindow = imath.Box2i( windowMin, imath.V2i( 10, 10 ) )
		self.assertEqual( i.dataWindow, imath.Box2i( windowMin, imath.V2i( 10, 10 ) ) )
		self.assertEqual( i.displayWindow, w )

		i.displayWindow = imath.Box2i( windowMin, imath.V2i( 10, 10 ) )
		self.assertEqual( i.displayWindow, imath.Box2i( windowMin, imath.V2i( 10, 10 ) ) )

	def testDataWindow( self ) :

		displayWindow = imath.Box2i( imath.V2i( 0, 0 ), imath.V2i( 99, 99 ) )
		dataWindow = imath.Box2i( imath.V2i( 50, 50 ), imath.V2i( 99, 99 ) )
		img = IECoreImage.ImagePrimitive( dataWindow, displayWindow )

	def testDataWindow( self ) :
		""" Test IECoreImage.ImagePrimitive data window """

		displayWindow = imath.Box2i( imath.V2i( 0, 0 ), imath.V2i( 99, 99 ) )
		dataWindow = imath.Box2i( imath.V2i( 50, 50 ), imath.V2i( 99, 99 ) )
		img = IECoreImage.ImagePrimitive( dataWindow, displayWindow )

		dataWindowArea = 50 * 50

		img["R"] = IECore.FloatVectorData( dataWindowArea )
		img["G"] = IECore.FloatVectorData( dataWindowArea )
		img["B"] = IECore.FloatVectorData( dataWindowArea )

		self.assertTrue( img.channelsValid() )

	# \todo Verify behaviour when dataWindow and displayWindow are contradictory or inconsistent

	def testLoadSave( self ) :
		""" Test IECoreImage.ImagePrimitive load/save """

		windowMin = imath.V2i( 0, 0 )
		windowMax = imath.V2i( 100, 100 )
		w = imath.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )
		i["R"] = IECore.FloatVectorData( 101 * 101 )
		i["G"] = IECore.FloatVectorData( 101 * 101 )
		i["B"] = IECore.FloatVectorData( 101 * 101 )
		self.assertTrue( i.channelsValid() )

		IECore.Writer.create( i, os.path.join( "test", "IECore", "data", "output.cob" ) ).write()

		i2 = IECore.Reader.create( os.path.join( "test", "IECore", "data", "output.cob" ) ).read()
		self.assertEqual( type( i2 ), IECoreImage.ImagePrimitive )

		self.assertEqual( i.displayWindow, i2.displayWindow )
		self.assertEqual( i.dataWindow, i2.dataWindow )
		self.assertEqual( i.channelNames(), i2.channelNames() )
		self.assertTrue( i2.channelsValid() )

	def testChannelNames( self ) :

		""" Test IECoreImage.ImagePrimitive channel names """

		windowMin = imath.V2i( 0, 0 )
		windowMax = imath.V2i( 99, 99 )
		w = imath.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		r = IECore.FloatVectorData()
		r.resize( 100 * 100 )

		i["R"] = r

		self.assertTrue( "R" in i.channelNames() )
		self.assertTrue( i.channelsValid() )

		b = IECore.FloatData()

		i["B"] = b

		self.assertFalse( "B" in i.channelNames() )
		self.assertFalse( i.channelsValid() )

		# Deliberately make a primvar too small!
		g = IECore.FloatVectorData()
		g.resize( 50 * 100 )

		i["G"] = g

		self.assertFalse( "G" in i.channelNames() )
		self.assertFalse( i.channelsValid() )

		i["B"] = i["R"]
		i["G"].resize( 100 * 100 )
		self.assertTrue( "R" in i.channelNames() )
		self.assertTrue( "G" in i.channelNames() )
		self.assertTrue( "B" in i.channelNames() )
		self.assertTrue( i.channelsValid() )

	def testCreateChannel( self ) :

		windowMin = imath.V2i( 0, 0 )
		windowMax = imath.V2i( 99, 99 )
		w = imath.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		i.createFloatChannel( "R" )
		i.createHalfChannel( "G" )
		i.createUIntChannel( "B" )

		self.assertTrue( "R" in i )
		self.assertTrue( "G" in i )
		self.assertTrue( "B" in i )

	def testErrors( self ) :

		windowMin = imath.V2i( 0, 0 )
		windowMax = imath.V2i( 99, 99 )
		w = imath.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		empty = imath.Box2i()

		self.assertRaises( RuntimeError, setattr, i, "displayWindow", empty )

		self.assertRaises( RuntimeError, IECoreImage.ImagePrimitive, empty, empty )

	def testChannelValid( self ) :

		b = imath.Box2i( imath.V2i( 0 ), imath.V2i( 9 ) )
		i = IECoreImage.ImagePrimitive( b, b )

		d = IECore.FloatVectorData( [1] )

		i["Y"] = d

		self.assertEqual( i.channelValid( d ), False )
		self.assertEqual( i.channelValid( "Y" ), False )
		self.assertEqual( i.getChannel( "Y" ), None )

		t = i.channelValid( d, True )
		self.assertTrue( isinstance( t, tuple ) )
		self.assertEqual( t[0], False )
		self.assertTrue( isinstance( t[1], str ) )

		d.resize( 100 )

		self.assertEqual( i.channelValid( d ), True )
		self.assertEqual( i.channelValid( "Y" ), True )
		self.assertTrue( d.isSame( i.getChannel( "Y" ) ) )

		dd = IECore.FloatData( 1 )
		self.assertEqual( i.channelValid( dd ), False )
		i["PP"] = dd
		self.assertEqual( i.channelValid( "PP" ), False )
		self.assertEqual( i.getChannel( "PP" ), None )

	def testConvenienceConstructors( self ) :

		""" Test IECoreImage.ImagePrimitive convenience constructors """

		window1Min = imath.V2i( 0, 0 )
		window1Max = imath.V2i( 15, 15 )
		w1 = imath.Box2i( window1Min, window1Max )

		window2Min = imath.V2i( 4, 4 )
		window2Max = imath.V2i( 11, 11 )
		w2 = imath.Box2i( window2Min, window2Max )

		fill = imath.Color3f( 0.49, 0.50, 0.51 )
		i = IECoreImage.ImagePrimitive.createRGBFloat( fill, w1, w2 )

		self.assertTrue( i.isInstanceOf( IECoreImage.ImagePrimitive.staticTypeId() ) )

		self.assertTrue( "R" in i )
		self.assertTrue( "G" in i )
		self.assertTrue( "B" in i )
		self.assertTrue( "Y" not in i )

		self.assertEqual( i.dataWindow, w1 )
		self.assertEqual( i.displayWindow, w2 )

		self.assertTrue( i["R"].isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )
		self.assertTrue( i["G"].isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )
		self.assertTrue( i["B"].isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )

		self.assertEqual( i["R"].size(), 256 )
		self.assertEqual( i["G"].size(), 256 )
		self.assertEqual( i["B"].size(), 256 )

		for p in (0, 63, 127, 255) :
			self.assertEqual( i["R"][p], fill[0] )
			self.assertEqual( i["G"][p], fill[1] )
			self.assertEqual( i["B"][p], fill[2] )

		fill = 0.5
		i = IECoreImage.ImagePrimitive.createGreyscaleFloat( fill, w1, w2 )

		self.assertTrue( i.isInstanceOf( IECoreImage.ImagePrimitive.staticTypeId() ) )

		self.assertTrue( "R" not in i )
		self.assertTrue( "G" not in i )
		self.assertTrue( "B" not in i )
		self.assertTrue( "Y" in i )

		self.assertEqual( i.dataWindow, w1 )
		self.assertEqual( i.displayWindow, w2 )

		self.assertTrue( i["Y"].isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )
		self.assertEqual( i["Y"].size(), 256 )

		for p in (0, 63, 127, 255) :
			self.assertEqual( i["Y"][p], fill )

	def testSpaces( self ) :

		# one pixel image 0,0 -> 0,0

		onePixelWindow = imath.Box2i( imath.V2i( 0 ), imath.V2i( 0 ) )
		i = IECoreImage.ImagePrimitive( onePixelWindow, onePixelWindow )

		m = i.pixelToObjectMatrix()
		self.assertEqual( imath.V2f( 0 ) * m, imath.V2f( 0 ) )
		m2 = i.objectToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.pixelToUVMatrix()
		self.assertEqual( imath.V2f( 0 ) * m, imath.V2f( 0.5 ) )
		m2 = i.uvToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.objectToUVMatrix()
		self.assertEqual( imath.V2f( -0.5 ) * m, imath.V2f( 0, 1 ) )
		self.assertEqual( imath.V2f( 0.5 ) * m, imath.V2f( 1, 0 ) )
		m2 = i.uvToObjectMatrix()
		self.assertEqual( m2, m.inverse() )

		self.assertTrue( (i.objectToUVMatrix() * i.uvToPixelMatrix()).equalWithAbsError( i.objectToPixelMatrix(), 0.00001 ) )
		self.assertTrue( (i.pixelToUVMatrix() * i.uvToObjectMatrix()).equalWithAbsError( i.pixelToObjectMatrix(), 0.00001 ) )

		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Pixel ), i.uvToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Object ), i.uvToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.UV ), i.pixelToUVMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.Object ), i.pixelToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.Pixel ), i.objectToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.UV ), i.objectToUVMatrix() )

		# two pixel image 0,0 -> 1,1

		twoPixelWindow = imath.Box2i( imath.V2i( 0 ), imath.V2i( 1 ) )
		i = IECoreImage.ImagePrimitive( twoPixelWindow, twoPixelWindow )

		m = i.pixelToObjectMatrix()
		self.assertEqual( imath.V2f( 0 ) * m, imath.V2f( -0.5, 0.5 ) )
		self.assertEqual( imath.V2f( 1 ) * m, imath.V2f( 0.5, -0.5 ) )
		m2 = i.objectToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.pixelToUVMatrix()
		self.assertEqual( imath.V2f( 0 ) * m, imath.V2f( 0.25 ) )
		self.assertEqual( imath.V2f( 1 ) * m, imath.V2f( 0.75 ) )
		m2 = i.uvToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.objectToUVMatrix()
		self.assertEqual( imath.V2f( -1 ) * m, imath.V2f( 0, 1 ) )
		self.assertEqual( imath.V2f( 1 ) * m, imath.V2f( 1, 0 ) )
		m2 = i.uvToObjectMatrix()
		self.assertEqual( m2, m.inverse() )

		self.assertTrue( (i.objectToUVMatrix() * i.uvToPixelMatrix()).equalWithAbsError( i.objectToPixelMatrix(), 0.00001 ) )
		self.assertTrue( (i.pixelToUVMatrix() * i.uvToObjectMatrix()).equalWithAbsError( i.pixelToObjectMatrix(), 0.00001 ) )

		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Pixel ), i.uvToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Object ), i.uvToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.UV ), i.pixelToUVMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.Object ), i.pixelToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.Pixel ), i.objectToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.UV ), i.objectToUVMatrix() )

		# three by two pixel image 10,20 -> 12,21

		threeTwoPixelWindowOffset = imath.Box2i( imath.V2i( 10, 20 ), imath.V2i( 12, 21 ) )
		i = IECoreImage.ImagePrimitive( threeTwoPixelWindowOffset, threeTwoPixelWindowOffset )

		m = i.pixelToObjectMatrix()
		self.assertEqual( imath.V2f( 10, 20 ) * m, imath.V2f( -1, 0.5 ) )
		self.assertEqual( imath.V2f( 12, 21 ) * m, imath.V2f( 1, -0.5 ) )
		m2 = i.objectToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.pixelToUVMatrix()
		self.assertTrue( (imath.V2f( 10, 20 ) * m).equalWithAbsError( imath.V2f( 1 / 6.0, 1 / 4.0 ), 0.00001 ) )
		self.assertTrue( (imath.V2f( 12, 21 ) * m).equalWithAbsError( imath.V2f( 5 / 6.0, 3 / 4.0 ), 0.00001 ) )
		m2 = i.uvToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.objectToUVMatrix()
		self.assertEqual( imath.V2f( -1.5, -1 ) * m, imath.V2f( 0, 1 ) )
		self.assertEqual( imath.V2f( 1.5, 1 ) * m, imath.V2f( 1, 0 ) )
		m2 = i.uvToObjectMatrix()
		self.assertEqual( m2, m.inverse() )

		self.assertTrue( (i.objectToUVMatrix() * i.uvToPixelMatrix()).equalWithAbsError( i.objectToPixelMatrix(), 0.00001 ) )
		self.assertTrue( (i.pixelToUVMatrix() * i.uvToObjectMatrix()).equalWithAbsError( i.pixelToObjectMatrix(), 0.00001 ) )

		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Pixel ), i.uvToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Object ), i.uvToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.UV ), i.pixelToUVMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.Object ), i.pixelToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.Pixel ), i.objectToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.UV ), i.objectToUVMatrix() )

	def testHash( self ) :

		w = imath.Box2i( imath.V2i( 0 ), imath.V2i( 10 ) )
		i = IECoreImage.ImagePrimitive( w, w )
		h = i.hash()

		i.displayWindow = imath.Box2i( imath.V2i( 10 ), imath.V2i( 20 ) )
		self.assertNotEqual( i.hash(), h )
		h = i.hash()

		i.dataWindow = imath.Box2i( imath.V2i( 10 ), imath.V2i( 20 ) )
		self.assertNotEqual( i.hash(), h )
		h = i.hash()

		i["R"] = IECore.IntData( 10 )
		self.assertNotEqual( i.hash(), h )
		h = i.hash()

		i["R"] = IECore.FloatData( 10 )
		self.assertNotEqual( i.hash(), h )
		h = i.hash()

		i["G"] = IECore.IntData( 10 )
		self.assertNotEqual( i.hash(), h )
		h = i.hash()

	def tearDown( self ) :

		if os.path.exists( os.path.join( "test", "IECore", "data", "output.cob" ) ) :

			os.remove( os.path.join( "test", "IECore", "data", "output.cob" ) )


if __name__ == "__main__" :
	unittest.main()
