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

import hou
import IECore
import IECoreHoudini
import unittest

class TestRATDeepImageReader( IECoreHoudini.TestCase ) :
	
	__dcm = "test/IECoreHoudini/data/rat/planesDCM.rat"
	__dsmMono = "test/IECoreHoudini/data/rat/planesMonoDSM.rat"
	__dsmTriple = "test/IECoreHoudini/data/rat/planesTripleDSM.rat"
	__tif = "test/IECoreHoudini/data/rat/planes.tif"
	
	def testConstructor( self ) :
	
		self.failUnless( "rat" in IECore.Reader.supportedExtensions() )
		self.failUnless( "rat" in IECore.Reader.supportedExtensions( IECore.TypeId.DeepImageReader ) )
		
		reader = IECoreHoudini.RATDeepImageReader()
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )
		
		self.failUnless( IECoreHoudini.RATDeepImageReader.canRead( TestRATDeepImageReader.__dcm ) )

		reader = IECoreHoudini.RATDeepImageReader( TestRATDeepImageReader.__dcm )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )
				
		reader = IECore.DeepImageReader.create( TestRATDeepImageReader.__dcm )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )

		reader = IECore.Reader.create( TestRATDeepImageReader.__dcm )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )

		self.failUnless( IECoreHoudini.RATDeepImageReader.canRead( TestRATDeepImageReader.__dsmMono ) )

		reader = IECoreHoudini.RATDeepImageReader( TestRATDeepImageReader.__dsmMono )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )
		
		reader = IECore.DeepImageReader.create( TestRATDeepImageReader.__dsmMono )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )
	
		reader = IECore.Reader.create( TestRATDeepImageReader.__dsmMono )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )
		
		self.failUnless( IECoreHoudini.RATDeepImageReader.canRead( TestRATDeepImageReader.__dsmTriple ) )

		reader = IECoreHoudini.RATDeepImageReader( TestRATDeepImageReader.__dsmTriple )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )
		
		reader = IECore.DeepImageReader.create( TestRATDeepImageReader.__dsmTriple )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )
	
		reader = IECore.Reader.create( TestRATDeepImageReader.__dsmTriple )
		self.failUnless( isinstance( reader, IECoreHoudini.RATDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreHoudini.TypeId.RATDeepImageReader )

	def testDefaultReader( self ) :
		
		reader = IECoreHoudini.RATDeepImageReader()
		self.assertEqual( reader.parameters()['fileName'].getTypedValue(), "" )
		self.failUnless( not reader.isComplete() )
		self.assertRaises( RuntimeError, reader.channelNames )
		self.assertRaises( RuntimeError, reader.dataWindow )
		self.assertRaises( RuntimeError, reader.displayWindow )
		self.assertRaises( RuntimeError, reader.worldToCameraMatrix )
		self.assertRaises( RuntimeError, reader.worldToNDCMatrix )
	
	def testDCMBasics( self ) :
	
		reader = IECoreHoudini.RATDeepImageReader( TestRATDeepImageReader.__dcm )
		self.assertEqual( reader.parameters()['fileName'].getTypedValue(), TestRATDeepImageReader.__dcm )
		self.failUnless( reader.isComplete() )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( [ "R", "G", "B", "A" ] ) )
		self.assertEqual( reader.dataWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 639, 479 ) ) )
		self.assertEqual( reader.displayWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 639, 479 ) ) )
		self.failUnless( reader.worldToCameraMatrix().equalWithAbsError( IECore.M44f( -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 ), 1e-6 ) )
		self.failUnless( reader.worldToNDCMatrix().equalWithAbsError( IECore.M44f( -2.41421, 0, 0, 0, 0, 3.21895, 0, 0, 0, 0, 1, -1, 0, 0, 0, 0 ), 1e-4 ) )
		
	def testDSMBasics( self, dsm = __dsmMono ) :
		
		reader = IECoreHoudini.RATDeepImageReader( dsm )
		self.assertEqual( reader.parameters()['fileName'].getTypedValue(), dsm )
		self.failUnless( reader.isComplete() )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( [ "A" ] ) )
		self.assertEqual( reader.dataWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 639, 479 ) ) )
		self.assertEqual( reader.displayWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 639, 479 ) ) )
		self.failUnless( reader.worldToCameraMatrix().equalWithAbsError( IECore.M44f( -1, 0, -1.22465e-16, 0, 0, 1, -0, 0, -1.22465e-16, 0, 1, 0, 0, 0, -0, 1 ), 1e-6 ) )
		self.failUnless( reader.worldToNDCMatrix().equalWithAbsError( IECore.M44f( -2.41421, 0, 0, 0, 0, 3.21895, 0, 0, 0, 0, 1, -1, 0, 0, 0, 0 ), 1e-4 ) )
	
	def testDCMHeader( self ) :
	
		reader = IECoreHoudini.RATDeepImageReader( TestRATDeepImageReader.__dcm )
		header = reader.readHeader()
		self.failUnless( isinstance( header, IECore.CompoundObject ) )
		self.assertEqual( header["dataWindow"].value, reader.dataWindow() )
		self.assertEqual( header["displayWindow"].value, reader.displayWindow() )
		self.assertEqual( header["channelNames"], reader.channelNames() )
		self.failUnless( header["worldToCameraMatrix"].value.equalWithAbsError( reader.worldToCameraMatrix(), 1e-6 ) )
		self.failUnless( header["worldToNDCMatrix"].value.equalWithAbsError( reader.worldToNDCMatrix(), 1e-6 ) )
	
	def testDSMHeader( self, dsm = __dsmMono ) :
	
		reader = IECoreHoudini.RATDeepImageReader( dsm )
		header = reader.readHeader()
		self.failUnless( isinstance( header, IECore.CompoundObject ) )
		self.assertEqual( header["dataWindow"].value, reader.dataWindow() )
		self.assertEqual( header["displayWindow"].value, reader.displayWindow() )
		self.assertEqual( header["channelNames"], reader.channelNames() )
		self.failUnless( header["worldToCameraMatrix"].value.equalWithAbsError( reader.worldToCameraMatrix(), 1e-6 ) )
		self.failUnless( header["worldToNDCMatrix"].value.equalWithAbsError( reader.worldToNDCMatrix(), 1e-6 ) )
	
	def testDCMReadPixel( self ) :
	
		reader = IECoreHoudini.RATDeepImageReader( TestRATDeepImageReader.__dcm )

		self.failUnless( isinstance( reader.readPixel( 0, 0 ), IECore.DeepPixel ) )
		self.failUnless( reader.readPixel( 639, 479 ) is None )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 640, 479 ) )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 639, 480 ) )
		
		p = reader.readPixel( 319, 45 )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "A" ) )
		self.assertEqual( p.numSamples(), 1 )
		self.assertEqual( p.getDepth( 0 ), 1 )
		self.assertEqual( p[0], ( 0.25, 0.5, 0.75, 0.25 ) )
		
		p2 = reader.readPixel( 320, 47 )
		self.assertEqual( p2.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p2.numSamples(), 1 )
		self.assertEqual( p2.getDepth( 0 ), 2 )
		self.assertEqual( p2[0], ( 0.5, 0.25, 0.125, 0.5 ) )

		p3 = reader.readPixel( 319, 47 )
		self.assertEqual( p3.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p3.numSamples(), 2 )
		self.assertEqual( p3.getDepth( 0 ), 1 )
		self.assertEqual( p3.getDepth( 1 ), 2 )
		self.assertEqual( p3[0], ( 0.25, 0.5, 0.75, 0.25 ) )
		self.assertEqual( p3[1], ( 0.5, 0.25, 0.125, 0.5 ) )
		self.assertEqual( p3.composite(), [ 0.625, 0.6875, 0.84375, 0.625 ] )
		
		self.failUnless( reader.readPixel( 320, 45 ) is None )
		
		p2.merge( p )
		self.assertEqual( p2.numSamples(), p3.numSamples() )
		self.assertEqual( p2.getDepth( 0 ), p3.getDepth( 0 ) )
		self.assertEqual( p2.getDepth( 1 ), p3.getDepth( 1 ) )
		self.assertEqual( p2[0], p3[0] )
		self.assertEqual( p2[1], p3[1] )
		self.assertEqual( p2.composite(), p3.composite() )
	
	def testDSMReadPixel( self, dsm = __dsmMono ) :
	
		reader = IECoreHoudini.RATDeepImageReader( dsm )
		
		self.failUnless( isinstance( reader.readPixel( 0, 0 ), IECore.DeepPixel ) )
		self.failUnless( reader.readPixel( 639, 479 ) is None )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 640, 479 ) )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 639, 480 ) )
		
		p = reader.readPixel( 319, 45 )
		self.assertEqual( p.channelNames(), ( "A", ) )
		self.assertEqual( p.numSamples(), 1 )
		self.assertEqual( p.getDepth( 0 ), 1 )
		self.assertEqual( p[0], ( 0.25, ) )

		p2 = reader.readPixel( 320, 47 )
		self.assertEqual( p2.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p2.numSamples(), 1 )
		self.assertEqual( p2.getDepth( 0 ), 2 )
		self.assertEqual( p2[0], ( 0.5, ) )

		p3 = reader.readPixel( 319, 47 )
		self.assertEqual( p3.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p3.numSamples(), 2 )
		self.assertEqual( p3.getDepth( 0 ), 1 )
		self.assertEqual( p3.getDepth( 1 ), 2 )
		self.assertEqual( p3[0], ( 0.25, ) )
		self.assertEqual( p3[1], ( 0.5, ) )
		self.assertEqual( p3.composite(), [ 0.625 ] )
		
		self.failUnless( reader.readPixel( 320, 45 ) is None )
		
		p2.merge( p )
		self.assertEqual( p2.numSamples(), p3.numSamples() )
		self.assertEqual( p2.getDepth( 0 ), p3.getDepth( 0 ) )
		self.assertEqual( p2.getDepth( 1 ), p3.getDepth( 1 ) )
		self.assertEqual( p2[0], p3[0] )
		self.assertEqual( p2[1], p3[1] )
		self.assertEqual( p2.composite(), p3.composite() )
	
	def testDCMComposite( self ) :
	
		reader = IECoreHoudini.RATDeepImageReader( TestRATDeepImageReader.__dcm )
		
		image = reader.read()
		self.failUnless( isinstance( image, IECore.ImagePrimitive ) )
		self.assertEqual( image.dataWindow, reader.dataWindow() )
		self.assertEqual( image.displayWindow, reader.displayWindow() )
		self.failUnless( image.channelValid( "R" ) )
		self.failUnless( image.channelValid( "G" ) )
		self.failUnless( image.channelValid( "B" ) )
		self.failUnless( image.channelValid( "A" ) )
		
		( r, g, b, a ) = ( image["R"].data, image["G"].data, image["B"].data, image["A"].data )
		
		i = 45 * 640 + 319
		self.assertEqual( ( r[i], g[i], b[i], a[i] ), ( 0.25, 0.5, 0.75, 0.25 ) )
		
		i = 47 * 640 + 320
		self.assertEqual( ( r[i], g[i], b[i], a[i] ), ( 0.5, 0.25, 0.125, 0.5 ) )
		
		i = 47 * 640 + 319
		self.assertEqual( ( r[i], g[i], b[i], a[i] ), ( 0.625, 0.6875, 0.84375, 0.625 ) )
		
		i = 45 * 640 + 320
		self.assertEqual( ( r[i], g[i], b[i], a[i] ), ( 0, 0, 0, 0 ) )
		
		realImage = IECore.TIFFImageReader( TestRATDeepImageReader.__tif ).read()
		self.assertEqual( image, realImage )
		
	def testDSMComposite( self, dsm = __dsmMono ) :
		
		reader = IECoreHoudini.RATDeepImageReader( dsm )
		
		image = reader.read()
		self.failUnless( isinstance( image, IECore.ImagePrimitive ) )
		self.assertEqual( image.dataWindow, reader.dataWindow() )
		self.assertEqual( image.displayWindow, reader.displayWindow() )
		self.failUnless( image.channelValid( "A" ) )
		self.failUnless( not image.channelValid( "R" ) )
		
		a = image["A"].data
		
		self.assertEqual( a[45 * 640 + 319], 0.25 )
		self.assertEqual( a[47 * 640 + 320], 0.5 )
		self.assertEqual( a[47 * 640 + 319], 0.625 )
		self.assertEqual( a[45 * 640 + 320], 0 )
		
		realImage = IECore.TIFFImageReader( TestRATDeepImageReader.__tif ).read()
		del realImage["R"]
		del realImage["G"]
		del realImage["B"]
		self.assertEqual( image, realImage )
	
	def testDSMTriple( self ) :
	
		self.testDSMBasics( dsm = TestRATDeepImageReader.__dsmTriple )
		self.testDSMHeader( dsm = TestRATDeepImageReader.__dsmTriple )
		self.testDSMReadPixel( dsm = TestRATDeepImageReader.__dsmTriple )
		self.testDSMComposite( dsm = TestRATDeepImageReader.__dsmTriple )

if __name__ == "__main__":
    unittest.main()
