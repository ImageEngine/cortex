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

import IECore
import IECoreImage


class ImagePrimitiveTest( unittest.TestCase ) :
	def testConstructor( self ) :
		""" Test IECoreImage.ImagePrimitive constructor """

		windowMin = IECore.V2i( 0, 0 )
		windowMax = IECore.V2i( 100, 100 )
		w = IECore.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		self.assertEqual( i.dataWindow, w )
		self.assertEqual( i.displayWindow, w )

		i.dataWindow = IECore.Box2i( windowMin, IECore.V2i( 10, 10 ) )
		self.assertEqual( i.dataWindow, IECore.Box2i( windowMin, IECore.V2i( 10, 10 ) ) )
		self.assertEqual( i.displayWindow, w )

		i.displayWindow = IECore.Box2i( windowMin, IECore.V2i( 10, 10 ) )
		self.assertEqual( i.displayWindow, IECore.Box2i( windowMin, IECore.V2i( 10, 10 ) ) )

	def testDataWindow( self ) :

		displayWindow = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 99, 99 ) )
		dataWindow = IECore.Box2i( IECore.V2i( 50, 50 ), IECore.V2i( 99, 99 ) )
		img = IECoreImage.ImagePrimitive( dataWindow, displayWindow )

	def testBound( self ) :
		""" Test IECoreImage.ImagePrimitive bound """

		windowMin = IECore.V2i( 0, 0 )
		windowMax = IECore.V2i( 99, 99 )
		w = IECore.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		self.assertEqual( i.bound(), IECore.Box3f( IECore.V3f( -50, -50, 0 ), IECore.V3f( 50, 50, 0 ) ) )

		windowMin = IECore.V2i( 50, 50 )
		windowMax = IECore.V2i( 99, 99 )
		w = IECore.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		self.assertEqual( i.bound(), IECore.Box3f( IECore.V3f( -25, -25, 0 ), IECore.V3f( 25, 25, 0 ) ) )

	def testDataWindow( self ) :
		""" Test IECoreImage.ImagePrimitive data window """

		displayWindow = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 99, 99 ) )
		dataWindow = IECore.Box2i( IECore.V2i( 50, 50 ), IECore.V2i( 99, 99 ) )
		img = IECoreImage.ImagePrimitive( dataWindow, displayWindow )

		dataWindowArea = 50 * 50
		R = IECore.FloatVectorData( dataWindowArea )
		G = IECore.FloatVectorData( dataWindowArea )
		B = IECore.FloatVectorData( dataWindowArea )

		img["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, R )
		img["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, G )
		img["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, B )

		self.assert_( img.arePrimitiveVariablesValid() )

	# \todo Verify behaviour when dataWindow and displayWindow are contradictory or inconsistent

	def testLoadSave( self ) :
		""" Test IECoreImage.ImagePrimitive load/save """

		windowMin = IECore.V2i( 0, 0 )
		windowMax = IECore.V2i( 100, 100 )
		w = IECore.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		IECore.Writer.create( i, "test/IECore/data/output.cob" ).write()

		i2 = IECore.Reader.create( "test/IECore/data/output.cob" ).read()
		self.assertEqual( type( i2 ), IECoreImage.ImagePrimitive )

		self.assertEqual( i.displayWindow, i2.displayWindow )
		self.assertEqual( i.dataWindow, i2.dataWindow )

	def testChannelNames( self ) :

		""" Test IECoreImage.ImagePrimitive channel names """

		windowMin = IECore.V2i( 0, 0 )
		windowMax = IECore.V2i( 99, 99 )
		w = IECore.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		r = IECore.FloatVectorData()
		r.resize( 100 * 100 )

		i["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, r )

		self.assert_( "R" in i.channelNames() )

		b = IECore.FloatData()

		i["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, b )

		self.failIf( "B" in i.channelNames() )

		self.assert_( i.arePrimitiveVariablesValid() )

		# Deliberately make a primvar too small!
		g = IECore.FloatVectorData()
		g.resize( 50 * 100 )

		i["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, g )

		self.failIf( "G" in i.channelNames() )
		self.failIf( i.arePrimitiveVariablesValid() )

	def testCreateChannel( self ) :

		windowMin = IECore.V2i( 0, 0 )
		windowMax = IECore.V2i( 99, 99 )
		w = IECore.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		i.createFloatChannel( "R" )
		i.createHalfChannel( "G" )
		i.createUIntChannel( "B" )

		self.assert_( "R" in i )
		self.assert_( "G" in i )
		self.assert_( "B" in i )

	def testErrors( self ) :

		windowMin = IECore.V2i( 0, 0 )
		windowMax = IECore.V2i( 99, 99 )
		w = IECore.Box2i( windowMin, windowMax )
		i = IECoreImage.ImagePrimitive( w, w )

		empty = IECore.Box2i()

		self.assertRaises( RuntimeError, setattr, i, "displayWindow", empty )

		self.assertRaises( RuntimeError, IECoreImage.ImagePrimitive, empty, empty )

	def testChannelValid( self ) :

		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 9 ) )
		i = IECoreImage.ImagePrimitive( b, b )

		d = IECore.FloatVectorData( [1] )

		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, d )
		i["Y"] = p

		self.assertEqual( i.channelValid( p ), False )
		self.assertEqual( i.channelValid( "Y" ), False )
		self.assertEqual( i.getChannel( "Y" ), None )

		t = i.channelValid( p, True )
		self.assert_( isinstance( t, tuple ) )
		self.assertEqual( t[0], False )
		self.assert_( isinstance( t[1], str ) )

		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, d )
		i["Y"] = p

		self.assertEqual( i.channelValid( p ), False )
		self.assertEqual( i.channelValid( "Y" ), False )
		self.assertEqual( i.getChannel( "Y" ), None )

		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, d )
		i["Y"] = p

		self.assertEqual( i.channelValid( p ), False )
		self.assertEqual( i.channelValid( "Y" ), False )
		self.assertEqual( i.getChannel( "Y" ), None )

		d.resize( 100 )

		self.assertEqual( i.channelValid( p ), True )
		self.assertEqual( i.channelValid( "Y" ), True )
		self.assert_( d.isSame( i.getChannel( "Y" ) ) )

		pp = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatData( 1 ) )
		self.assertEqual( i.channelValid( pp ), False )
		i["PP"] = pp
		self.assertEqual( i.channelValid( "PP" ), False )
		self.assertEqual( i.getChannel( "PP" ), None )

	def testConvenienceConstructors( self ) :

		""" Test IECoreImage.ImagePrimitive convenience constructors """

		window1Min = IECore.V2i( 0, 0 )
		window1Max = IECore.V2i( 15, 15 )
		w1 = IECore.Box2i( window1Min, window1Max )

		window2Min = IECore.V2i( 4, 4 )
		window2Max = IECore.V2i( 11, 11 )
		w2 = IECore.Box2i( window2Min, window2Max )

		fill = IECore.Color3f( 0.49, 0.50, 0.51 )
		i = IECoreImage.ImagePrimitive.createRGBFloat( fill, w1, w2 )

		self.assert_( i.isInstanceOf( IECoreImage.ImagePrimitive.staticTypeId() ) )

		self.assert_( "R" in i )
		self.assert_( "G" in i )
		self.assert_( "B" in i )
		self.assert_( "Y" not in i )

		self.assertEqual( i.dataWindow, w1 )
		self.assertEqual( i.displayWindow, w2 )

		self.assert_( i["R"].data.isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )
		self.assert_( i["G"].data.isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )
		self.assert_( i["B"].data.isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )

		self.assertEqual( i["R"].data.size(), 256 )
		self.assertEqual( i["G"].data.size(), 256 )
		self.assertEqual( i["B"].data.size(), 256 )

		for p in (0, 63, 127, 255) :
			self.assertEqual( i["R"].data[p], fill[0] )
			self.assertEqual( i["G"].data[p], fill[1] )
			self.assertEqual( i["B"].data[p], fill[2] )

		fill = 0.5
		i = IECoreImage.ImagePrimitive.createGreyscaleFloat( fill, w1, w2 )

		self.assert_( i.isInstanceOf( IECoreImage.ImagePrimitive.staticTypeId() ) )

		self.assert_( "R" not in i )
		self.assert_( "G" not in i )
		self.assert_( "B" not in i )
		self.assert_( "Y" in i )

		self.assertEqual( i.dataWindow, w1 )
		self.assertEqual( i.displayWindow, w2 )

		self.assert_( i["Y"].data.isInstanceOf( IECore.FloatVectorData.staticTypeId() ) )
		self.assertEqual( i["Y"].data.size(), 256 )

		for p in (0, 63, 127, 255) :
			self.assertEqual( i["Y"].data[p], fill )

	def testSpaces( self ) :

		# one pixel image 0,0 -> 0,0

		onePixelWindow = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 0 ) )
		i = IECoreImage.ImagePrimitive( onePixelWindow, onePixelWindow )

		m = i.pixelToObjectMatrix()
		self.assertEqual( IECore.V2f( 0 ) * m, IECore.V2f( 0 ) )
		m2 = i.objectToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.pixelToUVMatrix()
		self.assertEqual( IECore.V2f( 0 ) * m, IECore.V2f( 0.5 ) )
		m2 = i.uvToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.objectToUVMatrix()
		self.assertEqual( IECore.V2f( -0.5 ) * m, IECore.V2f( 0, 1 ) )
		self.assertEqual( IECore.V2f( 0.5 ) * m, IECore.V2f( 1, 0 ) )
		m2 = i.uvToObjectMatrix()
		self.assertEqual( m2, m.inverse() )

		self.failUnless( (i.objectToUVMatrix() * i.uvToPixelMatrix()).equalWithAbsError( i.objectToPixelMatrix(), 0.00001 ) )
		self.failUnless( (i.pixelToUVMatrix() * i.uvToObjectMatrix()).equalWithAbsError( i.pixelToObjectMatrix(), 0.00001 ) )

		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Pixel ), i.uvToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Object ), i.uvToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.UV ), i.pixelToUVMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.Object ), i.pixelToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.Pixel ), i.objectToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.UV ), i.objectToUVMatrix() )

		# two pixel image 0,0 -> 1,1

		twoPixelWindow = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 1 ) )
		i = IECoreImage.ImagePrimitive( twoPixelWindow, twoPixelWindow )

		m = i.pixelToObjectMatrix()
		self.assertEqual( IECore.V2f( 0 ) * m, IECore.V2f( -0.5, 0.5 ) )
		self.assertEqual( IECore.V2f( 1 ) * m, IECore.V2f( 0.5, -0.5 ) )
		m2 = i.objectToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.pixelToUVMatrix()
		self.assertEqual( IECore.V2f( 0 ) * m, IECore.V2f( 0.25 ) )
		self.assertEqual( IECore.V2f( 1 ) * m, IECore.V2f( 0.75 ) )
		m2 = i.uvToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.objectToUVMatrix()
		self.assertEqual( IECore.V2f( -1 ) * m, IECore.V2f( 0, 1 ) )
		self.assertEqual( IECore.V2f( 1 ) * m, IECore.V2f( 1, 0 ) )
		m2 = i.uvToObjectMatrix()
		self.assertEqual( m2, m.inverse() )

		self.failUnless( (i.objectToUVMatrix() * i.uvToPixelMatrix()).equalWithAbsError( i.objectToPixelMatrix(), 0.00001 ) )
		self.failUnless( (i.pixelToUVMatrix() * i.uvToObjectMatrix()).equalWithAbsError( i.pixelToObjectMatrix(), 0.00001 ) )

		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Pixel ), i.uvToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Object ), i.uvToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.UV ), i.pixelToUVMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.Object ), i.pixelToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.Pixel ), i.objectToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.UV ), i.objectToUVMatrix() )

		# three by two pixel image 10,20 -> 12,21

		threeTwoPixelWindowOffset = IECore.Box2i( IECore.V2i( 10, 20 ), IECore.V2i( 12, 21 ) )
		i = IECoreImage.ImagePrimitive( threeTwoPixelWindowOffset, threeTwoPixelWindowOffset )

		m = i.pixelToObjectMatrix()
		self.assertEqual( IECore.V2f( 10, 20 ) * m, IECore.V2f( -1, 0.5 ) )
		self.assertEqual( IECore.V2f( 12, 21 ) * m, IECore.V2f( 1, -0.5 ) )
		m2 = i.objectToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.pixelToUVMatrix()
		self.failUnless( (IECore.V2f( 10, 20 ) * m).equalWithAbsError( IECore.V2f( 1 / 6.0, 1 / 4.0 ), 0.00001 ) )
		self.failUnless( (IECore.V2f( 12, 21 ) * m).equalWithAbsError( IECore.V2f( 5 / 6.0, 3 / 4.0 ), 0.00001 ) )
		m2 = i.uvToPixelMatrix()
		self.assertEqual( m2, m.inverse() )

		m = i.objectToUVMatrix()
		self.assertEqual( IECore.V2f( -1.5, -1 ) * m, IECore.V2f( 0, 1 ) )
		self.assertEqual( IECore.V2f( 1.5, 1 ) * m, IECore.V2f( 1, 0 ) )
		m2 = i.uvToObjectMatrix()
		self.assertEqual( m2, m.inverse() )

		self.failUnless( (i.objectToUVMatrix() * i.uvToPixelMatrix()).equalWithAbsError( i.objectToPixelMatrix(), 0.00001 ) )
		self.failUnless( (i.pixelToUVMatrix() * i.uvToObjectMatrix()).equalWithAbsError( i.pixelToObjectMatrix(), 0.00001 ) )

		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Pixel ), i.uvToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.UV, IECoreImage.ImagePrimitive.Space.Object ), i.uvToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.UV ), i.pixelToUVMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Pixel, IECoreImage.ImagePrimitive.Space.Object ), i.pixelToObjectMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.Pixel ), i.objectToPixelMatrix() )
		self.assertEqual( i.matrix( IECoreImage.ImagePrimitive.Space.Object, IECoreImage.ImagePrimitive.Space.UV ), i.objectToUVMatrix() )

	def testHash( self ) :

		w = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 10 ) )
		i = IECoreImage.ImagePrimitive( w, w )
		h = i.hash()
		t = i.topologyHash()

		i.displayWindow = IECore.Box2i( IECore.V2i( 10 ), IECore.V2i( 20 ) )
		self.assertNotEqual( i.hash(), h )
		self.assertNotEqual( i.topologyHash(), h )
		h = i.hash()
		t = i.topologyHash()

		i.dataWindow = IECore.Box2i( IECore.V2i( 10 ), IECore.V2i( 20 ) )
		self.assertNotEqual( i.hash(), h )
		self.assertNotEqual( i.topologyHash(), h )
		h = i.hash()
		t = i.topologyHash()

		i["primVar"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 10 ) )
		self.assertNotEqual( i.hash(), h )
		self.assertEqual( i.topologyHash(), t )

	def tearDown( self ) :

		if os.path.exists( "test/IECore/data/output.cob" ) :

			os.remove( "test/IECore/data/output.cob" )


if __name__ == "__main__" :
	unittest.main()
