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

import unittest

import IECore
import IECoreRI

class TestDTEXDeepImageReader( unittest.TestCase ) :
	
	__dtex = "test/IECoreRI/data/dtex/coneAndSphere.dtex"
	__exr = "test/IECoreRI/data/dtex/coneAndSphere.exr"
	
	def testConstructor( self ) :
	
		self.failUnless( "dtex" in IECore.Reader.supportedExtensions() )
		self.failUnless( "dtex" in IECore.Reader.supportedExtensions( IECore.TypeId.DeepImageReader ) )
		
		reader = IECoreRI.DTEXDeepImageReader()
		self.failUnless( isinstance( reader, IECoreRI.DTEXDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.DTEXDeepImageReader.staticTypeId() )
		
		self.failUnless( IECoreRI.DTEXDeepImageReader.canRead( TestDTEXDeepImageReader.__dtex ) )

		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageReader.__dtex )
		self.failUnless( isinstance( reader, IECoreRI.DTEXDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.DTEXDeepImageReader.staticTypeId() )
				
		reader = IECore.DeepImageReader.create( TestDTEXDeepImageReader.__dtex )
		self.failUnless( isinstance( reader, IECoreRI.DTEXDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.DTEXDeepImageReader.staticTypeId() )

		reader = IECore.Reader.create( TestDTEXDeepImageReader.__dtex )
		self.failUnless( isinstance( reader, IECoreRI.DTEXDeepImageReader ) )
		self.assertEqual( reader.typeId(), IECoreRI.DTEXDeepImageReader.staticTypeId() )

		self.failUnless( not IECoreRI.DTEXDeepImageReader.canRead( TestDTEXDeepImageReader.__exr ) )

	def testDefaultReader( self ) :
		
		reader = IECoreRI.DTEXDeepImageReader()
		self.assertEqual( reader.parameters()['fileName'].getTypedValue(), "" )
		self.failUnless( not reader.isComplete() )
		self.assertRaises( RuntimeError, reader.channelNames )
		self.assertRaises( RuntimeError, reader.dataWindow )
		self.assertRaises( RuntimeError, reader.displayWindow )
		self.assertRaises( RuntimeError, reader.worldToCameraMatrix )
		self.assertRaises( RuntimeError, reader.worldToNDCMatrix )
	
	def testBasics( self ) :
	
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageReader.__dtex )
		self.assertEqual( reader.parameters()['fileName'].getTypedValue(), TestDTEXDeepImageReader.__dtex )
		self.failUnless( reader.isComplete() )
		self.assertEqual( reader.channelNames(), IECore.StringVectorData( [ "R", "G", "B", "A" ] ) )
		self.assertEqual( reader.dataWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 383, 383 ) ) )
		self.assertEqual( reader.displayWindow(), IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 383, 383 ) ) )
		self.failUnless( reader.worldToCameraMatrix().equalWithAbsError( IECore.M44f( 0.590606, 0.591501, 0.548919, 0, 0, 0.68023, -0.732998, 0, 0.80696, -0.432913, -0.401748, 0, 0.903436, 2.90732, 5.13921, 1 ), 1e-6 ) )
		self.failUnless( reader.worldToNDCMatrix().equalWithAbsError( IECore.M44f( 1.41078, 1.41291, 0.554464, 0.548919, 0, 1.62486, -0.740402, -0.732998, 1.92758, -1.0341, -0.405806, -0.401748, 2.15803, 6.94469, 5.09011, 5.13921 ), 1e-5 ) )
	
	def testHeader( self ) :
	
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageReader.__dtex )
		header = reader.readHeader()
		self.failUnless( isinstance( header, IECore.CompoundObject ) )
		self.assertEqual( header["dataWindow"].value, reader.dataWindow() )
		self.assertEqual( header["displayWindow"].value, reader.displayWindow() )
		self.assertEqual( header["channelNames"], reader.channelNames() )
		self.failUnless( header["worldToCameraMatrix"].value.equalWithAbsError( reader.worldToCameraMatrix(), 1e-6 ) )
		self.failUnless( header["worldToNDCMatrix"].value.equalWithAbsError( reader.worldToNDCMatrix(), 1e-6 ) )
	
	def testReadPixel( self ) :
	
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageReader.__dtex )

		self.failUnless( isinstance( reader.readPixel( 80, 50 ), IECore.DeepPixel ) )
		self.failUnless( reader.readPixel( 0, 0 ) is None )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 384, 383 ) )
		self.assertRaises( RuntimeError, IECore.curry( reader.readPixel, 383, 384 ) )
		
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
		
		self.failUnless( reader.readPixel( 193, 179 ) is None )
	
	def testComposite( self ) :
	
		reader = IECoreRI.DTEXDeepImageReader( TestDTEXDeepImageReader.__dtex )
		
		image = reader.read()
		self.failUnless( isinstance( image, IECore.ImagePrimitive ) )
		self.assertEqual( image.dataWindow, reader.dataWindow() )
		self.assertEqual( image.displayWindow, reader.displayWindow() )
		self.failUnless( image.channelValid( "R" ) )
		self.failUnless( image.channelValid( "G" ) )
		self.failUnless( image.channelValid( "B" ) )
		self.failUnless( image.channelValid( "A" ) )
		
		( r, g, b, a ) = ( image["R"].data, image["G"].data, image["B"].data, image["A"].data )
		
		i = 179 * 384 + 192
		self.assertAlmostEqual( r[i], 0.037882, 6 )
		self.assertAlmostEqual( g[i], 0.075764, 6 )
		self.assertAlmostEqual( b[i], 0.037882, 6 )
		self.assertAlmostEqual( a[i], 0.083333, 6 )
		
		i = 179 * 384 + 193
		self.assertEqual( ( r[i], g[i], b[i], a[i] ), ( 0, 0, 0, 0 ) )
		
		realImage = IECore.EXRImageReader( TestDTEXDeepImageReader.__exr ).read()
		realImage.blindData().clear()
		self.assertEqual( image.keys(), realImage.keys() )
		for k in image.keys() :
			imageData = image[k].data
			realImageData = realImage[k].data
			self.assertEqual( imageData.size(), realImageData.size() )
			for i in range( imageData.size() ) :
				self.assertAlmostEqual( imageData[i], realImageData[i], 6 )

if __name__ == "__main__":
    unittest.main()
