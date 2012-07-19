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
import os

## \todo: test writing formats other than RAT once the appropriate readers/writers exist...
## \todo: add similar DeepImageConverter tests to IECore once any readers/writers are registered there...
class TestDeepImageConverter( IECoreHoudini.TestCase ) :
	
	__inRAT = "test/IECoreHoudini/data/rat/planesDCM.rat"
	__outRAT = "test/IECoreHoudini/data/rat/written.rat"
	
	def testConstructor( self ) :
	
		converter = IECore.DeepImageConverter()
		self.failUnless( isinstance( converter, IECore.DeepImageConverter ) )
		self.assertEqual( converter.typeId(), IECore.TypeId.DeepImageConverter )
		self.failUnless( "rat" in converter.parameters()['inputFile'].extensions )
		self.failUnless( "rat" in converter.parameters()['outputFile'].extensions )
	
	def testOperation( self ) :
		
		result = IECore.DeepImageConverter()( 
			inputFile = TestDeepImageConverter.__inRAT,
			outputFile = TestDeepImageConverter.__outRAT
		)
		
		self.assertEqual( result.value, TestDeepImageConverter.__outRAT )
		
		reader = IECore.DeepImageReader.create( TestDeepImageConverter.__inRAT )
		reader2 = IECore.DeepImageReader.create( TestDeepImageConverter.__outRAT )
		self.assertEqual( reader2.channelNames(), reader.channelNames() )
		self.assertEqual( reader2.dataWindow(), reader.dataWindow() )
		self.assertEqual( reader2.read(), reader.read() )
		
		dataWindow = reader.dataWindow()
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
	
	def testErrors( self ) :
		
		converter = IECore.DeepImageConverter()
		converter.parameters()['inputFile'].setTypedValue( "test/IECore/data/tiff/rgbCheckerboard.tiff" )
		converter.parameters()['outputFile'].setTypedValue( "out.rat" )
		self.assertRaises( RuntimeError, converter.operate )
		converter.parameters()['inputFile'].setTypedValue( "in.rat" )
		self.assertRaises( RuntimeError, converter.operate )
		converter.parameters()['inputFile'].setTypedValue( TestDeepImageConverter.__inRAT )
		converter.parameters()['outputFile'].setTypedValue( "out.tif" )
		self.assertRaises( RuntimeError, converter.operate )
	
	def tearDown( self ) :
		
		if os.path.isfile( TestDeepImageConverter.__outRAT ) :
			os.remove( TestDeepImageConverter.__outRAT )

if __name__ == "__main__":
	unittest.main()
