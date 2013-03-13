##########################################################################
#
#  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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
import IECoreRI

class TestDTEXDeepImageWriter( unittest.TestCase ) :
	
	__dtex = "test/IECoreRI/data/dtex/coneAndSphere.dtex"
	__exr = "test/IECoreRI/data/dtex/coneAndSphere.exr"
	__output = "test/IECoreRI/data/dtex/written.dtex"
	
	def testConstructor( self ) :
	
		self.failUnless( "dtex" in IECore.DeepImageWriter.supportedExtensions() )
		self.failUnless( "dtex" in IECore.DeepImageWriter.supportedExtensions( IECore.TypeId.DeepImageWriter ) )
		
		writer = IECoreRI.DTEXDeepImageWriter()
		self.failUnless( isinstance( writer, IECoreRI.DTEXDeepImageWriter ) )
		self.assertEqual( writer.typeId(), IECoreRI.DTEXDeepImageWriter.staticTypeId() )
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		self.failUnless( isinstance( writer, IECoreRI.DTEXDeepImageWriter ) )
		self.assertEqual( writer.typeId(), IECoreRI.DTEXDeepImageWriter.staticTypeId() )
		
		writer = IECore.DeepImageWriter.create( TestDTEXDeepImageWriter.__output )
		self.failUnless( isinstance( writer, IECoreRI.DTEXDeepImageWriter ) )
		self.assertEqual( writer.typeId(), IECoreRI.DTEXDeepImageWriter.staticTypeId() )
	
	def testCanWrite( self ) :
		
		self.failUnless( not IECoreRI.DTEXDeepImageWriter.canWrite( "" ) )
		self.failUnless( IECoreRI.DTEXDeepImageWriter.canWrite( TestDTEXDeepImageWriter.__output ) )
	
	def testDefaultWriter( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter()
		self.assertEqual( writer.parameters()['fileName'].getTypedValue(), "" )
		self.assertEqual( writer.parameters()['channelNames'].getValue(), IECore.StringVectorData( list("RGBA") ) )
		self.assertEqual( writer.parameters()['resolution'].getTypedValue(), IECore.V2i( 2048, 1556 ) )
		self.assertEqual( writer.parameters()['tileSize'].getTypedValue(), IECore.V2i( 32, 32 ) )
		p = IECore.DeepPixel( "RGBA" )
		p.addSample( 1, [ 1, 0, 0, 1 ] )
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 0, p ) )
	
	def testParameters( self ) :
	
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		self.assertEqual( writer.parameters()['fileName'].getTypedValue(), TestDTEXDeepImageWriter.__output )
		self.assertEqual( writer.parameters()['channelNames'].getValue(), IECore.StringVectorData( list("RGBA") ) )
		self.assertEqual( writer.parameters()['resolution'].getTypedValue(), IECore.V2i( 2048, 1556 ) )
		self.assertEqual( writer.parameters()['tileSize'].getTypedValue(), IECore.V2i( 32, 32 ) )
		self.assertEqual( writer.parameters()['worldToCameraMatrix'].getTypedValue(), IECore.M44f() )
		self.assertEqual( writer.parameters()['worldToNDCMatrix'].getTypedValue(), IECore.M44f() )
	
	def testStrictChannels( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		
		p = IECore.DeepPixel( "RGBA" )
		p.addSample( 1, [ 1, 0, 0, 1 ] )
		writer.writePixel( 0, 0, p )
		
		p = IECore.DeepPixel( "RGB" )
		p.addSample( 1, [ 1, 0, 0 ] )
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 1, p ) )
		
		p = IECore.DeepPixel( "A" )
		p.addSample( 1, [ 1 ] )
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 2, p ) )
		
		p = IECore.DeepPixel( "RGBAST" )
		p.addSample( 1, [ 1, 0, 0, 1, 0, 1 ] )
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 3, p ) )
	
	def testTileSize( self ) :
		
		p = IECore.DeepPixel( "RGBA" )
		p.addSample( 1, [ 1, 0, 0, 1 ] )
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['resolution'].setTypedValue( IECore.V2i( 2, 2 ) )
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 0, p ) )
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['tileSize'].setTypedValue( IECore.V2i( 2, 2 ) )
		writer.writePixel( 0, 0, p )
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['resolution'].setTypedValue( IECore.V2i( 127, 127 ) )
		writer.writePixel( 0, 0, p )
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['resolution'].setTypedValue( IECore.V2i( 127, 127 ) )
		writer.parameters()['tileSize'].setTypedValue( IECore.V2i( 128, 128 ) )
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 0, p ) )
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['tileSize'].setTypedValue( IECore.V2i( 30, 30 ) )
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 0, p ) )
	
	def testWriteRGBAPixel( self ) :
	
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageWriter.__dtex )
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['resolution'].setTypedValue( reader.dataWindow().size() + IECore.V2i( 1 ) )
		self.assertEqual( writer.parameters()['resolution'].getTypedValue(), IECore.V2i( 384, 384 ) )
		self.assertEqual( writer.parameters()['channelNames'].getValue(), IECore.StringVectorData( list("RGBA") ) )
		
		writer.parameters()['resolution'].setTypedValue( IECore.V2i( 2, 2 ) )
		self.assertEqual( writer.parameters()['resolution'].getTypedValue(), IECore.V2i( 2, 2 ) )
		writer.parameters()['tileSize'].setTypedValue( IECore.V2i( 2, 2 ) )
		
		wToC = IECore.M44f( 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12, 13.13, 14.14, 15.15, 16.16 )
		cToS = IECore.M44f( 10.1, 20.2, 30.3, 40.4, 50.5, 60.6, 7.7, 80.8, 90.9, 100.1, 110.11, 120.12, 130.13, 140.14, 150.15, 160.16 )
		writer.parameters()['worldToCameraMatrix'].setTypedValue( wToC )
		writer.parameters()['worldToNDCMatrix'].setTypedValue( cToS )
		self.assertEqual( writer.parameters()['worldToCameraMatrix'].getTypedValue(), wToC )
		self.assertEqual( writer.parameters()['worldToNDCMatrix'].getTypedValue(), cToS )
		
		p = reader.readPixel( 192, 179 )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "A" ) )
		self.assertEqual( p.numSamples(), 1 )
		self.assertAlmostEqual( p.getDepth( 0 ), 5.297642, 6 )
		expected = ( 0.037882, 0.075764, 0.037882, 0.083333 )
		for i in range( 0, len(expected) ) :
			self.assertAlmostEqual( p[0][i], expected[i], 6 )
		
		p2 = reader.readPixel( 192, 183 )
		self.assertEqual( p2.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p2.numSamples(), 6 )
		self.assertAlmostEqual( p2.getDepth( 0 ), 4.893305, 6 )
		self.assertAlmostEqual( p2.getDepth( 1 ), 4.912039, 6 )
		self.assertAlmostEqual( p2.getDepth( 2 ), 4.926313, 6 )
		self.assertAlmostEqual( p2.getDepth( 3 ), 5.272715, 6 )
		self.assertAlmostEqual( p2.getDepth( 4 ), 5.284764, 6 )
		self.assertAlmostEqual( p2.getDepth( 5 ), 5.305941, 6 )
		expected = (
			( 0.019998, 0.019998, 0.026664, 0.133333 ),
			( 0.025801, 0.025801, 0.034401, 0.205128 ),
			( 0.016147, 0.016147, 0.021530, 0.129032 ),
			( 0.012917, 0.012917, 0.017223, 0.088889 ),
			( 0.029505, 0.029505, 0.039339, 0.195122 ),
			( 0.030522, 0.030522, 0.040697, 0.181818 )
		)
		for i in range( 0, len(expected) ) :
			for j in range( 0, 4 ) :
				self.assertAlmostEqual( p2[i][j], expected[i][j], 6 )
		
		p3 = reader.readPixel( 195, 225 )
		self.assertEqual( p3.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p3.numSamples(), 2 )
		self.assertAlmostEqual( p3.getDepth( 0 ), 4.296060, 6 )
		self.assertAlmostEqual( p3.getDepth( 1 ), 6.008639, 6 )
		expected = (
			( 0.248995, 0.248995, 0.331465, 0.400000 ),
			( 0.267683, 0.267683, 0.355995, 0.400000 )
		)
		for i in range( 0, len(expected) ) :
			for j in range( 0, 4 ) :
				self.assertAlmostEqual( p3[i][j], expected[i][j], 6 )
		
		writer.writePixel( 0, 0, p )
		writer.writePixel( 0, 1, p2 )
		writer.writePixel( 1, 1, p3 )
		del writer
		
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageWriter.__output )
		self.assertEqual( reader.dataWindow().size() + IECore.V2i( 1 ), IECore.V2i( 2, 2 ) )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( list("RGBA") ) )
		self.failUnless( reader.worldToCameraMatrix().equalWithAbsError( wToC, 1e-6 ) )
		self.failUnless( reader.worldToNDCMatrix().equalWithAbsError( cToS, 1e-6 ) )
		
		rp = reader.readPixel( 0, 0 )
		self.assertEqual( rp.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp.numSamples(), 1 )
		self.assertAlmostEqual( rp.getDepth( 0 ), 5.297642, 6 )
		expected = ( 0.037882, 0.075764, 0.037882, 0.083333 )
		for i in range( 0, len(expected) ) :
			self.assertAlmostEqual( rp[0][i], expected[i], 6 )
		
		rp2 = reader.readPixel( 0, 1 )
		self.assertEqual( rp2.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp2.numSamples(), 6 )
		self.assertAlmostEqual( rp2.getDepth( 0 ), 4.893305, 6 )
		self.assertAlmostEqual( rp2.getDepth( 1 ), 4.912039, 6 )
		self.assertAlmostEqual( rp2.getDepth( 2 ), 4.926313, 6 )
		self.assertAlmostEqual( rp2.getDepth( 3 ), 5.272715, 6 )
		self.assertAlmostEqual( rp2.getDepth( 4 ), 5.284764, 6 )
		self.assertAlmostEqual( rp2.getDepth( 5 ), 5.305941, 6 )
		expected = (
			( 0.019998, 0.019998, 0.026664, 0.133333 ),
			( 0.025801, 0.025801, 0.034401, 0.205128 ),
			( 0.016147, 0.016147, 0.021530, 0.129032 ),
			( 0.012917, 0.012917, 0.017223, 0.088889 ),
			( 0.029505, 0.029505, 0.039339, 0.195122 ),
			( 0.030522, 0.030522, 0.040697, 0.181818 )
		)
		for i in range( 0, len(expected) ) :
			for j in range( 0, 4 ) :
				self.assertAlmostEqual( rp2[i][j], expected[i][j], 6 )
		
		rp3 = reader.readPixel( 1, 1 )
		self.assertEqual( rp3.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp3.numSamples(), 2 )
		self.assertAlmostEqual( rp3.getDepth( 0 ), 4.296060, 6 )
		self.assertAlmostEqual( rp3.getDepth( 1 ), 6.008639, 6 )
		expected = (
			( 0.248995, 0.248995, 0.331465, 0.400000 ),
			( 0.267683, 0.267683, 0.355995, 0.400000 )
		)
		for i in range( 0, len(expected) ) :
			for j in range( 0, 4 ) :
				self.assertAlmostEqual( rp3[i][j], expected[i][j], 6 )
		
		self.failUnless( reader.readPixel( 1, 0 ) is None )
	
	def testWriteAlphaPixel( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['channelNames'].setValue( IECore.StringVectorData( list("A") ) )
		writer.parameters()['resolution'].setTypedValue( IECore.V2i( 2, 2 ) )
		writer.parameters()['tileSize'].setTypedValue( IECore.V2i( 2, 2 ) )
		
		p = IECore.DeepPixel( "A" )
		p.addSample( 1, ( 0.25, ) )
		
		p2 = IECore.DeepPixel( "A" )
		p2.addSample( 2, ( 0.5, ) )
		
		p3 = IECore.DeepPixel( "A" )
		p3.addSample( 1, ( 0.25, ) )
		p3.addSample( 2, ( 0.5, ) )
		
		writer.writePixel( 0, 0, p )
		writer.writePixel( 0, 1, p2 )
		writer.writePixel( 1, 1, p3 )
		del writer
		
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageWriter.__output )
		self.assertEqual( reader.dataWindow().size() + IECore.V2i( 1 ), IECore.V2i( 2, 2 ) )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( list("A") ) )
		
		rp = reader.readPixel( 0, 0 )
		self.assertEqual( rp.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp.numSamples(), 1 )
		self.assertEqual( rp.getDepth( 0 ), 1 )
		self.assertEqual( rp[0], ( 0.25, ) )
		
		rp2 = reader.readPixel( 0, 1 )
		self.assertEqual( rp2.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp2.numSamples(), 1 )
		self.assertEqual( rp2.getDepth( 0 ), 2 )
		self.assertEqual( rp2[0], ( 0.5, ) )
		
		rp3 = reader.readPixel( 1, 1 )
		self.assertEqual( rp3.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp3.numSamples(), 2 )
		self.assertEqual( rp3.getDepth( 0 ), 1 )
		self.assertEqual( rp3.getDepth( 1 ), 2 )
		self.assertEqual( rp3[0], ( 0.25, ) )
		self.assertEqual( rp3[1], ( 0.5, ) )
		self.assertEqual( rp3.composite(), [ 0.625 ] )
		
		self.failUnless( reader.readPixel( 1, 0 ) is None )
	
	def testEmptyPixel( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		p = IECore.DeepPixel()
		writer.writePixel( 0, 0, p )
		writer.writePixel( 0, 50, None )
		writer.writePixel( 0, 100, p )
		p.addSample( 1, [ 1, 0, 0, 1 ] )
		writer.writePixel( 0, 1, p )
		del writer
		
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageWriter.__output )
		self.failUnless( reader.readPixel( 0, 0 ) is None )
		self.failUnless( reader.readPixel( 0, 50 ) is None )
		self.failUnless( reader.readPixel( 0, 100 ) is None )
		rp = reader.readPixel( 0, 1 )
		self.failUnless( isinstance( rp, IECore.DeepPixel ) )
		self.assertEqual( rp[0], ( 1, 0, 0, 1 ) )
	
	def testFileConversion( self ) :
		
		reader = IECore.DeepImageReader.create( TestDTEXDeepImageWriter.__dtex )
		dataWindow = reader.dataWindow()
		
		writer = IECore.DeepImageWriter.create( TestDTEXDeepImageWriter.__output )
		writer.parameters()['channelNames'].setValue( reader.channelNames() )
		writer.parameters()['resolution'].setTypedValue( dataWindow.size() + IECore.V2i( 1 ) )
		
		for y in range( dataWindow.min.y, dataWindow.max.y ) :
			for x in range( dataWindow.min.x, dataWindow.max.x ) :
				writer.writePixel( x, y, reader.readPixel( x, y ) )
		
		del writer
		
		reader2 = IECore.DeepImageReader.create( TestDTEXDeepImageWriter.__output )
		self.assertEqual( reader2.channelNames(), reader.channelNames() )
		self.assertEqual( reader2.dataWindow(), reader.dataWindow() )
		self.assertEqual( reader2.read(), reader.read() )
		
		for y in range( dataWindow.min.y, dataWindow.max.y ) :
			for x in range( dataWindow.min.x, dataWindow.max.x ) :
				p = reader.readPixel( x, y )
				p2 = reader2.readPixel( x, y )
				if not p2 and not p :
					continue
				
				self.assertEqual( p2.channelNames(), p.channelNames() )
				self.assertEqual( p2.numSamples(), p.numSamples() )
				for i in range( 0, p.numSamples() ) :
					self.assertEqual( p2.getDepth( i ), p.getDepth( i ) )
					self.assertEqual( p2[i], p[i] )
	
	def testStrangeOrder( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['channelNames'].setValue( IECore.StringVectorData( list("ARGB") ) )
		
		p = IECore.DeepPixel( "ARGB", 2 )
		p.addSample( 1.0, [ 0.25, 0.5, 0.75, 0.5 ] )
		p.addSample( 1.5, [ 1.0, 0.25, 0.5, 0.75 ] )
		writer.writePixel( 0, 0, p )
		del writer
		
		reader = IECore.DeepImageReader.create( TestDTEXDeepImageWriter.__output )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( list("ARGB") ) )
		rp = reader.readPixel( 0, 0 )
		self.assertEqual( rp.channelNames(), ( "A", "R", "G", "B" ) )
		self.assertEqual( rp.numSamples(), 2 )
		self.assertEqual( rp.getDepth( 0 ), 1.0 )
		self.assertEqual( rp.getDepth( 1 ), 1.5 )
		self.assertEqual( rp[0], ( 0.25, 0.5, 0.75, 0.5 ) )
		self.assertEqual( rp[1], ( 1.0, 0.25, 0.5, 0.75 ) )
	
	def testNoAlpha( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['channelNames'].setValue( IECore.StringVectorData( list("RGB") ) )
		
		p = IECore.DeepPixel( "RGB", 2 )
		p.addSample( 1.0, [ 0.25, 0.5, 0.75 ] )
		p.addSample( 1.5, [ 0.75, 0.25, 0.5 ] )
		writer.writePixel( 0, 0, p )
		del writer
		
		reader = IECore.DeepImageReader.create( TestDTEXDeepImageWriter.__output )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( list("RGB") ) )
		rp = reader.readPixel( 0, 0 )
		self.assertEqual( rp.channelNames(), ( "R", "G", "B" ) )
		self.assertEqual( rp.numSamples(), 2 )
		self.assertEqual( rp.getDepth( 0 ), 1.0 )
		self.assertEqual( rp.getDepth( 1 ), 1.5 )
		self.assertEqual( rp[0], ( 0.25, 0.5, 0.75 ) )
		self.assertEqual( rp[1], ( 0.75, 0.25, 0.5 ) )
	
	def testArbitraryChannels( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['channelNames'].setValue( IECore.StringVectorData( [ "Testing", "Arbitrary", "Channel", "Names" ] ) )
		
		p = IECore.DeepPixel( [ "Testing", "Arbitrary", "Channel", "Names" ], 2 )
		p.addSample( 1.0, [ 0.25, 0.5, 0.75, 0.5 ] )
		p.addSample( 1.5, [ 1.0, 0.5, 0.25, 0.75 ] )
		
		self.assertRaises( RuntimeError, IECore.curry( writer.writePixel, 0, 0, p ) )
		
		writer.parameters()['channelNames'].setValue( IECore.StringVectorData( [ "T", "A", "C", "N" ] ) )
		
		p = IECore.DeepPixel( [ "T", "A", "C", "N" ], 2 )
		p.addSample( 1.0, [ 0.25, 0.5, 0.75, 0.5 ] )
		p.addSample( 1.5, [ 1.0, 0.5, 0.25, 0.75 ] )
		writer.writePixel( 0, 0, p )
		del writer
		
		reader = IECore.DeepImageReader.create( TestDTEXDeepImageWriter.__output )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( [ "T", "A", "C", "N" ] ) )
		rp = reader.readPixel( 0, 0 )
		self.assertEqual( rp.channelNames(), ( "T", "A", "C", "N" ) )
		self.assertEqual( rp.numSamples(), 2 )
		self.assertEqual( rp.getDepth( 0 ), 1.0 )
		self.assertEqual( rp.getDepth( 1 ), 1.5 )
		self.assertEqual( rp[0], ( 0.25, 0.5, 0.75, 0.5 ) )
		self.assertEqual( rp[1], ( 1.0, 0.5, 0.25, 0.75 ) )
		
	def testExtraChannels( self ) :
		
		writer = IECoreRI.DTEXDeepImageWriter( TestDTEXDeepImageWriter.__output )
		writer.parameters()['channelNames'].setValue( IECore.StringVectorData( list("RGBAST") ) )
		
		p = IECore.DeepPixel( "RGBAST", 2 )
		p.addSample( 1.0, [ 0.25, 0.5, 0.75, 0.5, 0.25, 0.5 ] )
		p.addSample( 1.5, [ 1.0, 0.5, 0.25, 0.75, 0.5, 0.75 ] )
		writer.writePixel( 0, 0, p )
		del writer
		
		reader = IECore.DeepImageReader.create( TestDTEXDeepImageWriter.__output )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( list("RGBAST") ) )
		rp = reader.readPixel( 0, 0 )
		self.assertEqual( rp.channelNames(), ( "R", "G", "B", "A", "S", "T" ) )
		self.assertEqual( rp.numSamples(), 2 )
		self.assertEqual( rp.getDepth( 0 ), 1.0 )
		self.assertEqual( rp.getDepth( 1 ), 1.5 )
		self.assertEqual( rp[0], ( 0.25, 0.5, 0.75, 0.5, 0.25, 0.5 ) )
		self.assertEqual( rp[1], ( 1.0, 0.5, 0.25, 0.75, 0.5, 0.75 ) )
	
	def tearDown( self ) :
		
		if os.path.isfile( TestDTEXDeepImageWriter.__output ) :
			os.remove( TestDTEXDeepImageWriter.__output )

if __name__ == "__main__":
    unittest.main()
