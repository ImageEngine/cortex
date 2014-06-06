##########################################################################
#
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreRI

class TestSHWDeepImageReader( unittest.TestCase ) :
	
	__shw = "test/IECoreRI/data/shw/translucentBoxes.shw"
	__shwConstantPlane = "test/IECoreRI/data/shw/constantPlane.shw"
	__shwConstantPlane2 = "test/IECoreRI/data/shw/constantPlane2.shw"
	__shwOrtho = "test/IECoreRI/data/shw/constantPlaneOrtho.shw"

	__exr = "test/IECoreRI/data/shw/groundPlane.exr"
	
	def testConstructor( self ) :
	
		self.failUnless( "shw" in IECore.Reader.supportedExtensions() )
		self.failUnless( "shw" in IECore.Reader.supportedExtensions( IECore.TypeId.DeepImageReader ) )
		
		reader = IECoreRI.SHWDeepImageReader()
		self.failUnless( isinstance( reader, IECoreRI.SHWDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.SHWDeepImageReader.staticTypeId() )
		
		self.failUnless( IECoreRI.SHWDeepImageReader.canRead( TestSHWDeepImageReader.__shw ) )

		reader = IECoreRI.SHWDeepImageReader( TestSHWDeepImageReader.__shw )
		self.failUnless( isinstance( reader, IECoreRI.SHWDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.SHWDeepImageReader.staticTypeId() )
				
		reader = IECore.DeepImageReader.create( TestSHWDeepImageReader.__shw )
		self.failUnless( isinstance( reader, IECoreRI.SHWDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.SHWDeepImageReader.staticTypeId() )

		reader = IECore.Reader.create( TestSHWDeepImageReader.__shw )
		self.failUnless( isinstance( reader, IECoreRI.SHWDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.SHWDeepImageReader.staticTypeId() )

		self.failUnless( not IECoreRI.SHWDeepImageReader.canRead( TestSHWDeepImageReader.__exr ) )

	def testDefaultReader( self ) :
		
		reader = IECoreRI.SHWDeepImageReader()
		self.assertEqual( reader.parameters()['fileName'].getTypedValue(), "" )
		self.failUnless( not reader.isComplete() )
		self.assertRaises( RuntimeError, reader.channelNames )
		self.assertRaises( RuntimeError, reader.dataWindow )
		self.assertRaises( RuntimeError, reader.displayWindow )
		self.assertRaises( RuntimeError, reader.worldToCameraMatrix )
		self.assertRaises( RuntimeError, reader.worldToNDCMatrix )
	
	def testBasics( self ) :
	
		reader = IECoreRI.SHWDeepImageReader( TestSHWDeepImageReader.__shw )
		self.assertEqual( reader.parameters()['fileName'].getTypedValue(), TestSHWDeepImageReader.__shw )
		self.failUnless( reader.isComplete() )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( [ "A" ] ) )
		self.assertEqual( reader.dataWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 511, 511 ) ) )
		self.assertEqual( reader.displayWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 511, 511 ) ) )
		self.failUnless( reader.worldToCameraMatrix().equalWithAbsError( IECore.M44f( -0.422618, -0.694272, -0.582563, 0, -2.42861e-17, 0.642788, -0.766044, 0, -0.906308, 0.323744, 0.271654, 0, 3.00476, -0.592705, 84.3541, 1 ), 1e-4 ) )
		self.failUnless( reader.worldToNDCMatrix().equalWithAbsError( IECore.M44f( -1.02029, -1.67612, -0.582578, -0.582563, 0, 1.55183, -0.766064, -0.766044, -2.18802, 0.781588, 0.271661, 0.271654, 7.25413, -1.43092, 83.5416, 84.3541 ), 1e-4 ) )
	
	def testHeader( self ) :
	
		reader = IECoreRI.SHWDeepImageReader( TestSHWDeepImageReader.__shw )
		header = reader.readHeader()
		self.failUnless( isinstance( header, IECore.CompoundObject ) )
		self.assertEqual( header["dataWindow"].value, reader.dataWindow() )
		self.assertEqual( header["displayWindow"].value, reader.displayWindow() )
		self.assertEqual( header["channelNames"], reader.channelNames() )
		self.failUnless( header["worldToCameraMatrix"].value.equalWithAbsError( reader.worldToCameraMatrix(), 1e-6 ) )
		self.failUnless( header["worldToNDCMatrix"].value.equalWithAbsError( reader.worldToNDCMatrix(), 1e-6 ) )
	
	def testReadPixel( self ) :
	
		reader = IECoreRI.SHWDeepImageReader( TestSHWDeepImageReader.__shw )

		self.failUnless( isinstance( reader.readPixel( 80, 112 ), IECore.DeepPixel ) )
		self.failUnless( reader.readPixel( 0, 0 ) is None )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 512, 511 ) )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 511, 512 ) )
		
		# hits ground plane only
		p = reader.readPixel( 100, 100 )
		self.assertEqual( p.channelNames(), ( "A", ) )
		self.assertEqual( p.numSamples(), 1 )
		self.assertAlmostEqual( p.getDepth( 0 ), 102.17636108, 6 )
		self.assertAlmostEqual( p[0][0], 1.0, 6 )
		
		# hits one box then ground plane
		p2 = reader.readPixel( 256, 256 )
		self.assertEqual( p2.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p2.numSamples(), 3 )
		self.assertAlmostEqual( p2.getDepth( 0 ), 72.6087493, 6 )
		self.assertAlmostEqual( p2.getDepth( 1 ), 77.7387313, 6 )
		self.assertAlmostEqual( p2.getDepth( 2 ), 85.6622314, 6 )
		
		expected = ( 0.5, 0.5, 1.0 )
		for i in range( 0, len(expected) ) :
			self.assertEqual( p2[i][0], expected[i] )
		
		# hits 2 boxes then ground plane
		p3 = reader.readPixel( 195, 225 )
		self.assertEqual( p3.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( p3.numSamples(), 5 )
		self.assertAlmostEqual( p3.getDepth( 0 ), 68.6177368, 6 )
		self.assertAlmostEqual( p3.getDepth( 1 ), 75.3023605, 6 )
		self.assertAlmostEqual( p3.getDepth( 2 ), 77.4083328, 6 )
		self.assertAlmostEqual( p3.getDepth( 3 ), 80.0680771, 6 )
		self.assertAlmostEqual( p3.getDepth( 4 ), 88.8455811, 6 )
		
		expected = ( 0.5, 0.75, 0.5, 0.5, 1.0 )
		for i in range( 0, len(expected) ) :
			self.assertEqual( p3[i][0], expected[i] )
		
		self.failUnless( reader.readPixel( 440, 30 ) is None )
	
	def testComposite( self ) :
	
		reader = IECoreRI.SHWDeepImageReader( TestSHWDeepImageReader.__shw )
		
		image = reader.read()
		self.failUnless( isinstance( image, IECore.ImagePrimitive ) )
		self.assertEqual( image.dataWindow, reader.dataWindow() )
		self.assertEqual( image.displayWindow, reader.displayWindow() )
		self.failUnless( image.channelValid( "A" ) )
		
		a = image["A"].data
		
		i = 11 * 512 + 153
		self.assertEqual( a[i], 1.0 )
		
		i = 11 * 512 + 154
		self.assertEqual( a[i], 0 )
		
		realImage = IECore.EXRImageReader( TestSHWDeepImageReader.__exr ).read()
		for k in image.keys() :
			imageData = image[k].data
			realImageData = realImage[k].data
			self.assertEqual( imageData.size(), realImageData.size() )
			for i in range( imageData.size() ) :
				self.assertAlmostEqual( imageData[i], realImageData[i], 6 )

	def __testDepthConversionWithFile( self, filename, tolerance ) :

		reader = IECore.DeepImageReader.create( filename )
		dataWindow = reader.dataWindow()

		for y in range( dataWindow.min.y, dataWindow.max.y + 1 ) :
			for x in range( dataWindow.min.x, dataWindow.max.x + 1 ) :
				p = reader.readPixel( x, y )
				s = p.numSamples()
				avgDepth = 0
				for i in range( s ):
					avgDepth += p.getDepth( i ) / s
				self.assertAlmostEqual( avgDepth, 10, tolerance )

	def testDepthConversion( self ) :

		# The first constant plane test is rendered with a single sample, and no jittering, so the depth should match to 6 significant digits
		self.__testDepthConversionWithFile( TestSHWDeepImageReader.__shwConstantPlane, 4 )

		# The second constant plane was rendered with a small number of jittered samples, and because of the low resolution, there is quite a bit
		# of variation in the correction factor across the pixel, so it won't be a perfect match, but it should still be close
		self.__testDepthConversionWithFile( TestSHWDeepImageReader.__shwConstantPlane2, 2 )

		# Test out an ortho render as well
		self.__testDepthConversionWithFile( TestSHWDeepImageReader.__shwOrtho, 2 )

if __name__ == "__main__":
    unittest.main()
