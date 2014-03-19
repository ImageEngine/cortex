##########################################################################
#
#  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

from IECore import *
import sys
import unittest

class EXRDeepImageReaderTest(unittest.TestCase):

	def testCreator( self ) :

		reader = DeepImageReader.create( "test/IECoreRI/data/exr/primitives.exr" )
		self.assertEqual( reader.typeId(), TypeId.EXRDeepImageReader )
	
	def testCanRead( self ) :
		
		self.assertFalse( EXRDeepImageReader.canRead( "test/IECore/data/exrFiles/AllHalfValues.exr" ) ) # Regular EXR image.
		self.assertTrue( EXRDeepImageReader.canRead( "test/IECoreRI/data/exr/primitives.exr" ) ) # Deep EXR image.

	def testNDCMatrix( self ) :

		reader = DeepImageReader.create( "test/IECoreRI/data/exr/primitives.exr" )
		expectedMatrix = M44f( 1.95253, 2.02384e-16, 0.450882, 0.450878, 0, 2.91667, 0, 0, 0.986295, 8.09538e-17, -0.892595, -0.892586, -3.17955, -4.56305, 9.30162, 9.40153 )
		self.assertTrue( expectedMatrix.equalWithAbsError( reader.worldToNDCMatrix(), 10e-6 ) )
	
	def testWorldToCameraMatrix( self ) :

		reader = DeepImageReader.create( "test/IECoreRI/data/exr/primitives.exr" )
		expectedMatrix = M44f( 0.892586, 6.93889e-17, 0.450878, 0, 0, 1, 0, 0, 0.450878, 2.77556e-17, -0.892586, 0, -1.45351, -1.56447, 9.40153, 1 )
		self.assertTrue( expectedMatrix.equalWithAbsError( reader.worldToCameraMatrix(), 10e-6 ) )
	
	def testChannelNames( self ) :

		reader = DeepImageReader.create( "test/IECoreRI/data/exr/primitives.exr" )
		self.assertEqual( StringVectorData( [ "A", "B", "G", "R", "ZBack" ] ), reader.channelNames() )

	def testDataWindow( self ) :

		reader = DeepImageReader.create( "test/IECoreRI/data/exr/primitives.exr" )
		self.assertEqual( reader.dataWindow(), Box2i( V2i( 0, 0 ), V2i( 511, 511 ) ) )

	def testDisplayWindow( self ) :

		reader = DeepImageReader.create( "test/IECoreRI/data/exr/primitives.exr" )
		self.assertEqual( reader.displayWindow(), Box2i( V2i( 0, 0 ), V2i( 511, 511 ) ) )

	def testReadPixel( self ) :

		reader = DeepImageReader.create( "test/IECoreRI/data/exr/primitives.exr" )
		d = reader.readPixel( 154, 285 )
		self.assertEqual( set( [ 'A', 'B', 'G', 'R', 'ZBack' ] ), set( d.channelNames() ) )
		
		expectedResults = [
			{ "A" : 0.11111116409301758, "B" : 0.105224609375, "G" : 0.105224609375, "R" : 0.105224609375, "ZBack" : 9.7488307952880859 },
			{ "A" : 0.12499994039535522, "B" : 0.11834716796875, "G" : 0.11834716796875, "R" : 0.11834716796875, "ZBack" : 9.7491855621337891 },
			{ "A" : 0.14285707473754883, "B" : 0.135009765625, "G" : 0.135009765625, "R" : 0.135009765625, "ZBack" : 9.7497596740722656 }, 
			{ "A" : 0.16666662693023682, "B" : 0.157470703125, "G" : 0.157470703125, "R" : 0.157470703125, "ZBack" : 9.7503156661987305 }, 
			{ "A" : 0.20000016689300537, "B" : 0.1888427734375, "G" : 0.1888427734375, "R" : 0.1888427734375, "ZBack" : 9.750544548034668 },
			{ "A" : 0.24999994039535522, "B" : 0.2357177734375, "G" : 0.2357177734375, "R" : 0.2357177734375, "ZBack" : 9.7510242462158203 },
			{ "A" : 0.33333331346511841, "B" : 0.314208984375, "G" : 0.314208984375, "R" : 0.314208984375, "ZBack" : 9.751317024230957 }, 
			{ "A" : 0.5, "B" : 0.470703125, "G" : 0.470703125, "R" : 0.470703125, "ZBack" : 9.7521572113037109 }, 
			{ "A" : 1.0, "B" : 0.94091796875, "G" : 0.94091796875, "R" : 0.94091796875, "ZBack" : 9.7521839141845703 },
		]
		
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(0) ) ), expectedResults[0] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(1) ) ), expectedResults[1] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(2) ) ), expectedResults[2] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(3) ) ), expectedResults[3] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(4) ) ), expectedResults[4] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(5) ) ), expectedResults[5] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(6) ) ), expectedResults[6] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(7) ) ), expectedResults[7] )	
		self.assertEqual( dict( zip( d.channelNames(), d.channelData(8) ) ), expectedResults[8] )	
		
		self.assertEqual( d.getDepth(0), 9.7488307952880859 )
		self.assertEqual( d.getDepth(1), 9.7488307952880859 )
		self.assertEqual( d.getDepth(2), 9.7491855621337891 )
		self.assertEqual( d.getDepth(3), 9.7497596740722656 )
		self.assertEqual( d.getDepth(4), 9.7503156661987305 )
		self.assertEqual( d.getDepth(5), 9.750544548034668 )
		self.assertEqual( d.getDepth(6), 9.7510242462158203 )
		self.assertEqual( d.getDepth(7), 9.751317024230957 )
		self.assertEqual( d.getDepth(8), 9.7521572113037109 )

if __name__ == "__main__":
	unittest.main()

