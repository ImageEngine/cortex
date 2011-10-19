##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

class DeepPixelTest( unittest.TestCase ) :

	def testConstructors( self ) :

		p = IECore.DeepPixel()
		self.assert_( isinstance( p, IECore.DeepPixel ) )
		self.assertEqual( p.numSamples(), 0 )
		self.assertEqual( len(p), 0 )
		self.assertEqual( p.range(), ( 0, 0 ) )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "A" ) )
		self.assertRaises( IndexError, IECore.curry( p.getDepth, 0 ) )
		
		p = IECore.DeepPixel( 5 )
		self.assert_( isinstance( p, IECore.DeepPixel ) )
		self.assertEqual( p.numSamples(), 0 )
		self.assertEqual( len(p), 0 )
		self.assertEqual( p.range(), ( 0, 0 ) )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "A" ) )
		self.assertRaises( IndexError, IECore.curry( p.getDepth, 0 ) )
		self.assertRaises( IndexError, IECore.curry( p.channelData, 0 ) )
		
		p2 = IECore.DeepPixel( p )
		self.assert_( isinstance( p2, IECore.DeepPixel ) )
		self.assertEqual( p2.numSamples(), p.numSamples() )
		self.assertEqual( len(p2), len(p) )
		self.assertEqual( p2.range(), p.range() )
		self.assertEqual( p2.channelNames(), p.channelNames() )
		self.assertRaises( IndexError, IECore.curry( p2.getDepth, 0 ) )
		self.assertRaises( IndexError, IECore.curry( p2.channelData, 0 ) )
		p2.addSample( 2.5, [ 1, 0, 0, 1 ] )
		self.assertNotEqual( p2.numSamples(), p.numSamples() )
		self.assertNotEqual( len(p2), len(p) )
		self.assertNotEqual( p2.range(), p.range() )
		self.assertEqual( p2.getDepth( 0 ), 2.5 )
		self.assertEqual( p2.channelData( 0 ), ( 1, 0, 0, 1 ) )
		
		p = IECore.DeepPixel( "RGBPD" )
		self.assert_( isinstance( p, IECore.DeepPixel ) )
		self.assertEqual( p.numSamples(), 0 )
		self.assertEqual( len(p), 0 )
		self.assertEqual( p.range(), ( 0, 0 ) )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "P", "D" ) )
		self.assertRaises( IndexError, IECore.curry( p.getDepth, 0 ) )
		
		p = IECore.DeepPixel( list("RGBPD"), 2 )
		self.assert_( isinstance( p, IECore.DeepPixel ) )
		self.assertEqual( p.numSamples(), 0 )
		self.assertEqual( len(p), 0 )
		self.assertEqual( p.range(), ( 0, 0 ) )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "P", "D" ) )
		self.assertRaises( IndexError, IECore.curry( p.getDepth, 0 ) )
		self.assertRaises( IndexError, IECore.curry( p.channelData, 0 ) )

	def pixel( self ) :
		
		p = IECore.DeepPixel()
		p.addSample( 5.4, [ 0, 0, 1, 1 ] ) # 5
		p.addSample( 2.6, [ 1, 0, 0, 1 ] ) # 3
		p.addSample( 4.3, [ 0, 1, 0, 1 ] ) # 4
		p.addSample( 6.2, [ 0.5, 0.5, 0, 1 ] ) # 6
		p.addSample( 6.5, [ 0, 0.5, 0.5, 1 ] ) # 7
		p.addSample( 2.1, [ 0, 0, 1, 0.25 ] ) # 2
		p.addSample( 0.75, [ 1, 0, 0, 0.75 ] ) # 0
		p.addSample( 1.5, [ 0, 1, 0, 0.5 ] ) # 1
		
		return p
	
	def testRange( self ) :
		
		p = self.pixel()
		self.assertEqual( p.numSamples(), 8 )
		self.assertEqual( len(p), 8 )
		self.assertEqual( p.range(), ( 0.75, 6.5 ) )
		self.assertEqual( p.min(), p.range()[0] )
		self.assertEqual( p.max(), p.range()[1] )
		
	def testDepths( self ) :
		
		p = self.pixel()
		self.assertAlmostEqual( p.getDepth( 3 ), 2.6, 6 )
		self.assertAlmostEqual( p.getDepth( 5 ), 5.4, 6 )
		self.assertAlmostEqual( p.getDepth( 6 ), 6.2, 6 )
		
		p.setDepth( 3, 5.54321 )
		self.assertAlmostEqual( p.getDepth( 3 ), 4.3, 6 )
		self.assertAlmostEqual( p.getDepth( 4 ), 5.4, 6 )
		self.assertAlmostEqual( p.getDepth( 5 ), 5.54321, 6 )
		self.assertAlmostEqual( p.getDepth( 6 ), 6.2, 6 )
		
		self.assertRaises( IndexError, IECore.curry( p.getDepth, 10 ) )
		self.assertRaises( IndexError, IECore.curry( p.setDepth, 10, 3.0 ) )
	
	def testNegativeDepths( self ) :
		
		p = self.pixel()
		p.addSample( -1, [ 0.25, 0.5, 0.75, 1 ] )
		p.addSample( -2.5, [ 0.25, 0.5, 0.5, 1 ] )
		p.addSample( -0.25, [ 0.5, 0.25, 0.5, 1 ] )
		self.assertEqual( p.numSamples(), 11 )
		self.assertEqual( len(p), 11 )
		self.assertEqual( p.getDepth( 0 ), -2.5 )
		self.assertEqual( p.getDepth( 1 ), -1 )
		self.assertEqual( p.getDepth( 2 ), -0.25 )
		self.assertEqual( p.getDepth( 3 ), 0.75 )
		self.assertEqual( p.channelData( 0 ), ( 0.25, 0.5, 0.5, 1 ) )
		self.assertEqual( p.channelData( 1 ), ( 0.25, 0.5, 0.75, 1 ) )
		self.assertEqual( p.channelData( 2 ), ( 0.5, 0.25, 0.5, 1 ) )
		self.assertEqual( p.channelData( 3 ), ( 1, 0, 0, 0.75 ) )
	
	def testChannelData( self ) :
		
		p = self.pixel()
		self.assertEqual( p.channelData( 1 ), ( 0, 1, 0, 0.5 ) )
		self.assertEqual( p.channelData( 3 ), ( 1, 0, 0, 1 ) )
		self.assertEqual( p.channelData( 6 ), ( 0.5, 0.5, 0, 1 ) )
		self.assertEqual( p[1], p.channelData( 1 ) )
		self.assertEqual( p[3], p.channelData( 3 ) )
		self.assertEqual( p[6], p.channelData( 6 ) )
		
		p[3] = [ 0.25, 0.5, 0.75, 0.5 ]
		self.assertEqual( p.channelData( 3 ), ( 0.25, 0.5, 0.75, 0.5 ) )
		self.assertEqual( p[3], p.channelData( 3 ) )
		
		p[4] = ( 0.75, 0.5, 0.25, 1 )
		self.assertEqual( p.channelData( 4 ), ( 0.75, 0.5, 0.25, 1 ) )
		self.assertEqual( p[4], p.channelData( 4 ) )
		
		self.assertRaises( IndexError, IECore.curry( p.channelData, 8 ) )
		self.assertRaises( IndexError, IECore.curry( p.__getitem__, 8 ) )
		self.assertRaises( IndexError, IECore.curry( p.__setitem__, 8, ( 1, 1, 1, 1 ) ) )
	
	def testNegativeIndexingChannelData( self ) :
		
		p = self.pixel()
		self.assertEqual( p.channelData( 7 ), ( 0, 0.5, 0.5, 1 ) )
		self.assertEqual( p.channelData( 6 ), ( 0.5, 0.5, 0, 1 ) )
		self.assertEqual( p.channelData( 0 ), ( 1, 0, 0, 0.75 ) )
		self.assertEqual( p[-1], p.channelData( -1 ) )
		self.assertEqual( p[-2], p.channelData( -2 ) )
		self.assertEqual( p[-8], p.channelData( -8 ) )
		self.assertEqual( p[-1], p.channelData( 7 ) )
		self.assertEqual( p[-2], p.channelData( 6 ) )
		self.assertEqual( p[-8], p.channelData( 0 ) )
		self.assertRaises( IndexError, IECore.curry( p.channelData, -9 ) )
		self.assertRaises( IndexError, IECore.curry( p.__getitem__, -9 ) )
		
		p[-1] = [ 0.75, 0.5, 0.25, 1 ]
		self.assertEqual( p.channelData( 7 ), ( 0.75, 0.5, 0.25, 1 ) )
		self.assertEqual( p[-1], p.channelData( -1 ) )
		self.assertEqual( p[-1], p.channelData( 7 ) )
		self.assertRaises( IndexError, IECore.curry( p.__setitem__, -9, [ 1, 1, 1, 1 ] ) )
		
		self.assertRaises( IndexError, IECore.curry( p.removeSample, -9 ) )
		self.assertRaises( IndexError, IECore.curry( p.__delitem__, -9 ) )
		
		p.removeSample( -2 )
		self.assertEqual( len(p), 7 )
		self.assertRaises( IndexError, IECore.curry( p.channelData, -8 ) )
		self.assertRaises( IndexError, IECore.curry( p.__getitem__, -8 ) )
		self.assertRaises( IndexError, IECore.curry( p.removeSample, -8 ) )
		self.assertRaises( IndexError, IECore.curry( p.__delitem__, -8 ) )
		self.assertEqual( p.channelData( 5 ), ( 0, 0, 1, 1 ) )
		self.assertEqual( p.channelData( 6 ), ( 0.75, 0.5, 0.25, 1 ) )
		self.assertEqual( p[-1], p.channelData( -1 ) )
		self.assertEqual( p[-2], p.channelData( -2 ) )
		self.assertEqual( p[-1], p.channelData( 6 ) )
		self.assertEqual( p[-2], p.channelData( 5 ) )
		
		del p[-2]
		self.assertEqual( len(p), 6 )
		self.assertRaises( IndexError, IECore.curry( p.channelData, -7 ) )
		self.assertRaises( IndexError, IECore.curry( p.__getitem__, -7 ) )
		self.assertRaises( IndexError, IECore.curry( p.removeSample, -7 ) )
		self.assertRaises( IndexError, IECore.curry( p.__delitem__, -7 ) )
		self.assertEqual( p.channelData( 4 ), ( 0, 1, 0, 1 ) )
		self.assertEqual( p.channelData( 5 ), ( 0.75, 0.5, 0.25, 1 ) )
		self.assertEqual( p[-1], p.channelData( -1 ) )
		self.assertEqual( p[-2], p.channelData( -2 ) )
		self.assertEqual( p[-1], p.channelData( 5 ) )
		self.assertEqual( p[-2], p.channelData( 4 ) )
		
	def testNegativeIndexingDepth( self ) :
		
		p = self.pixel()
		self.assertAlmostEqual( p.getDepth( -1 ), 6.5, 6 )
		self.assertAlmostEqual( p.getDepth( -2 ), 6.2, 6 )
		self.assertAlmostEqual( p.getDepth( -8 ), 0.75, 6 )
		self.assertRaises( IndexError, IECore.curry( p.getDepth, -9 ) )
		
		p.setDepth( -1, 6.9 )
		self.assertAlmostEqual( p.getDepth( -1 ), 6.9, 6 )
		p.setDepth( -2, 7.3 )
		self.assertAlmostEqual( p.getDepth( -1 ), 7.3, 6 )
		self.assertAlmostEqual( p.getDepth( -2 ), 6.9, 6 )
		p.setDepth( -8, 3.4 )
		self.assertAlmostEqual( p.getDepth( -8 ), 1.5, 6 )
		self.assertAlmostEqual( p.getDepth( 3 ), 3.4, 6 )
		
		self.assertRaises( IndexError, IECore.curry( p.setDepth, -9, 3.0 ) )
	
	def testBadChannelData( self ) :
		
		p = self.pixel()
		self.assertRaises( TypeError, IECore.curry( p.addSample, 2, [ 1, 0, "B", 1 ] ) )
		self.assertRaises( TypeError, IECore.curry( p.addSample, 2, [ 1, [ 0, 1 ], 1 ] ) )
		self.assertRaises( TypeError, IECore.curry( p.addSample, 2, [ 1, 0, 1 ] ) )
		self.assertRaises( TypeError, IECore.curry( p.addSample, 2, [ 1, 0, 1, 1, 1 ] ) )
		
		self.assertRaises( TypeError, IECore.curry( p.__setitem__, 2, [ 1, 0, "B", 1 ] ) )
		self.assertRaises( TypeError, IECore.curry( p.__setitem__, 2, [ 1, [ 0, 1 ], 1 ] ) )
		self.assertRaises( TypeError, IECore.curry( p.__setitem__, 2, [ 1, 0, 1 ] ) )
		self.assertRaises( TypeError, IECore.curry( p.__setitem__, 2, [ 1, 0, 1, 1, 1 ] ) )
	
	def testInterpolatedChannelData( self ) :
		
		p = self.pixel()
		
		expected = ( 0.375, 0.375, 0.25, 1.0 )
		result = p.interpolatedChannelData( 6 )
		for i in range( 0, 4 ) :
			self.assertAlmostEqual( result[i], expected[i], 6 )
		
		expected = ( 0.0, 0.583333, 0.416666, 0.3958333 )
		result = p.interpolatedChannelData( 1.75 )
		for i in range( 0, 4 ) :
			self.assertAlmostEqual( result[i], expected[i], 5 )
		
		self.assertEqual( p.interpolatedChannelData( 0.15 ), ( 1, 0, 0, 0.75 ) )
		self.assertEqual( p.interpolatedChannelData( 7.5 ), ( 0, 0.5, 0.5, 1 ) )

	def testRemoveSamples( self ) :
		
		p = self.pixel()
		self.assertEqual( p.numSamples(), 8 )
		self.assertEqual( len(p), 8 )
		self.assertAlmostEqual( p.getDepth( 3 ), 2.6, 6 )
		self.assertAlmostEqual( p.getDepth( 4 ), 4.3, 6 )
		self.assertAlmostEqual( p.getDepth( 5 ), 5.4, 6 )
		self.assertAlmostEqual( p.getDepth( 6 ), 6.2, 6 )
		self.assertEqual( p.channelData( 3 ), ( 1, 0, 0, 1 ) )
		self.assertEqual( p.channelData( 4 ), ( 0, 1, 0, 1 ) )
		self.assertEqual( p.channelData( 5 ), ( 0, 0, 1, 1 ) )
		self.assertEqual( p.channelData( 6 ), ( 0.5, 0.5, 0, 1 ) )
		
		p.removeSample( 4 )
		self.assertEqual( len(p), 7 )
		self.assertEqual( p.numSamples(), 7 )
		self.assertAlmostEqual( p.getDepth( 3 ), 2.6, 6 )
		self.assertAlmostEqual( p.getDepth( 4 ), 5.4, 6 )
		self.assertAlmostEqual( p.getDepth( 5 ), 6.2, 6 )
		self.assertAlmostEqual( p.getDepth( 6 ), 6.5, 6 )
		self.assertEqual( p.channelData( 3 ), ( 1, 0, 0, 1 ) )
		self.assertEqual( p.channelData( 4 ), ( 0, 0, 1, 1 ) )
		self.assertEqual( p.channelData( 5 ), ( 0.5, 0.5, 0, 1 ) )
		self.assertEqual( p.channelData( 6 ), ( 0, 0.5, 0.5, 1 ) )
		self.assertRaises( IndexError, IECore.curry( p.channelData, 7 ) )
		self.assertRaises( IndexError, IECore.curry( p.__getitem__, 7 ) )
		self.assertRaises( IndexError, IECore.curry( p.removeSample, 7 ) )
		self.assertRaises( IndexError, IECore.curry( p.__delitem__, 7 ) )
		
		del p[4]
		self.assertEqual( len(p), 6 )
		self.assertEqual( p.numSamples(), 6 )
		self.assertAlmostEqual( p.getDepth( 3 ), 2.6, 6 )
		self.assertAlmostEqual( p.getDepth( 4 ), 6.2, 6 )
		self.assertAlmostEqual( p.getDepth( 5 ), 6.5, 6 )
		self.assertRaises( IndexError, IECore.curry( p.getDepth, 6 ) )
		self.assertEqual( p.channelData( 3 ), ( 1, 0, 0, 1 ) )
		self.assertEqual( p.channelData( 4 ), ( 0.5, 0.5, 0, 1 ) )
		self.assertEqual( p.channelData( 5 ), ( 0, 0.5, 0.5, 1 ) )
		self.assertRaises( IndexError, IECore.curry( p.channelData, 6 ) )
		self.assertRaises( IndexError, IECore.curry( p.__getitem__, 6 ) )
		self.assertRaises( IndexError, IECore.curry( p.removeSample, 6 ) )
		self.assertRaises( IndexError, IECore.curry( p.__delitem__, 6 ) )
	
	def testChannels( self ) :
		
		p = IECore.DeepPixel()
		self.assertEqual( p.numChannels(), 4 )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "A" ) )
		self.assertEqual( p.channelIndex( "R" ), 0 )
		self.assertEqual( p.channelIndex( "G" ), 1 )
		self.assertEqual( p.channelIndex( "B" ), 2 )
		self.assertEqual( p.channelIndex( "A" ), 3 )
		
		p = IECore.DeepPixel( "RGBPD", 2 )
		self.assertEqual( p.numChannels(), 5 )
		self.assertEqual( p.channelNames(), ( "R", "G", "B", "P", "D" ) )
		self.assertEqual( p.channelIndex( "R" ), 0 )
		self.assertEqual( p.channelIndex( "G" ), 1 )
		self.assertEqual( p.channelIndex( "B" ), 2 )
		self.assertEqual( p.channelIndex( "P" ), 3 )
		self.assertEqual( p.channelIndex( "D" ), 4 )
		
		p = IECore.DeepPixel( [ "Testing", "Arbitrary", "Channel", "Names" ] )
		self.assertEqual( p.numChannels(), 4 )
		self.assertEqual( p.channelNames(), ( "Testing", "Arbitrary", "Channel", "Names" ) )
		self.assertEqual( p.channelIndex( "Testing" ), 0 )
		self.assertEqual( p.channelIndex( "Arbitrary" ), 1 )
		self.assertEqual( p.channelIndex( "Channel" ), 2 )
		self.assertEqual( p.channelIndex( "Names" ), 3 )
		
		self.assertRaises( TypeError, IECore.curry( IECore.DeepPixel, [ "R", "G", 1, "A" ] ) )
		self.assertRaises( TypeError, IECore.curry( IECore.DeepPixel, [ "R", ["G", "B"], "A" ] ) )

	def testMerge( self ) :
		
		p = self.pixel()
		p2 = IECore.DeepPixel()
		p2.merge( p )
		self.assertEqual( p2.numChannels(), p.numChannels() )
		self.assertEqual( p2.channelNames(), p.channelNames() )
		self.assertEqual( len(p2), len(p) )
		for i in range( 0, len(p) ) :
			self.assertEqual( p2.getDepth( i ), p.getDepth( i ) )
			self.assertEqual( p2[i], p[i] )
		
		p2.merge( p )
		self.assertEqual( p2.numChannels(), p.numChannels() )
		self.assertEqual( p2.channelNames(), p.channelNames() )
		self.assertEqual( len(p2), len(p) * 2 )
		j = 0
		for i in range( 0, len(p) ) :
			self.assertEqual( p2.getDepth( j ), p.getDepth( i ) )
			self.assertEqual( p2.getDepth( j+1 ), p.getDepth( i ) )
			self.assertEqual( p2[j], p[i] )
			self.assertEqual( p2[j+1], p[i] )
			j += 2
		
		p3 = IECore.DeepPixel()
		p3.addSample( 3.4, [ 0.1, 0.2, 0.8, 1 ] ) # 2
		p3.addSample( 1.2, [ 0.9, 0.6, 0.3, 0.25 ] ) # 1
		p3.addSample( 4.5, [ 0.25, 0.25, 0.5, 0.75 ] ) # 3
		p3.addSample( -2.1, [ 0.5, 0, 0.5, 0.25 ] ) # 0
		
		p4 = self.pixel()
		p.merge( p3 )
		p3.merge( p4 )
		self.assertEqual( p3.numChannels(), p.numChannels() )
		self.assertEqual( p3.channelNames(), p.channelNames() )
		self.assertAlmostEqual( p3.getDepth( 0 ), -2.1, 6 )
		self.assertAlmostEqual( p3.getDepth( 4 ), 2.1, 6 )
		self.assertEqual( p3.channelData( 0 ), ( 0.5, 0, 0.5, 0.25 ) )
		self.assertEqual( p3.channelData( 4 ), ( 0, 0, 1, 0.25 ) )
		self.assertEqual( len(p3), len(p) )
		for i in range( 0, len(p) ) :
			self.assertEqual( p3.getDepth( i ), p.getDepth( i ) )
			self.assertEqual( p3[i], p[i] )

	def testComposite( self ) :
		
		p = IECore.DeepPixel()
		p.addSample( 2, [ 0.5, 0.5, 0.5, 0.5 ] )
		p.addSample( 1, [ 0.25, 0.5, 0.75, 0.25 ] )
		data = p.composite()
		self.assertEqual( data, [ 0.625, 0.875, 1.125, 0.625 ] )
		
		p.addSample( 3, [ 0.75, 0.5, 0.25, 1 ] )
		data = p.composite()
		self.assertEqual( data, [ 0.90625, 1.06250, 1.21875, 1 ] )

		# opaque
		sample = list(p[0])
		sample[-1] = 1
		p[0] = sample
		data = p.composite()
		self.assertEqual( data, [ 0.25, 0.5, 0.75, 1 ] )
		
		# no alpha just returns first sample
		p2 = IECore.DeepPixel( "RGBPD" )
		p2.addSample( 2, [ 0.5, 0.5, 0.5, 0.5, 0.5 ] )
		p2.addSample( 1, [ 0.25, 0.5, 0.75, 0.75, 0.25 ] )
		p2.addSample( 3, [ 0.75, 0.5, 0.25, 0.75, 0.35 ] )
		data = p2.composite()
		self.assertEqual( data, [ 0.25, 0.5, 0.75, 0.75, 0.25 ] )
		
		p3 = self.pixel()
		data = p3.composite()
		self.assertEqual( data, [ 1.09375, 0.25, 0.125, 1 ] )

	def testAverage( self ) :
		
		original = self.pixel()
		simple = IECore.DeepPixel()
		simple.addSample( 2, [ 0.5, 0.5, 0.5, 0.5 ] )
		simple.addSample( 1, [ 0.25, 0.5, 0.75, 0.25 ] )
		simple.addSample( 3, [ 0.75, 0.5, 0.25, 1 ] )
		
		p = IECore.DeepPixel.average( [ original ], [ 0.25 ] )
		self.assertEqual( p.numChannels(), original.numChannels() )
		self.assertEqual( p.channelNames(), original.channelNames() )
		self.assertEqual( len(p), len(original) )
		for i in range( 0, len(p) ) :
			self.assertEqual( p.getDepth( i ), original.getDepth( i ) )
			self.assertEqual( p[i], tuple( [ x * 0.25 for x in original[i] ] ) )
		
		expected = [
			( 0.625, 0.25, 0.375, 0.5 ),
			( 0.1875, 0.75, 0.3125, 0.4375 ),
			( 0.262499, 0.25, 0.73750, 0.399999 ),
			( 0.824999, 0.25, 0.17500, 0.899999 ),
			( 0.375, 0.75, 0.125, 1.0 ),
			( 0.375, 0.25, 0.625, 1.0 ),
			( 0.625, 0.5, 0.125, 1.0 ),
			( 0.375, 0.5, 0.375, 1.0 ),
		]
		
		p = IECore.DeepPixel.average( [ original, simple ], [ 0.5, 0.5 ] )
		self.assertEqual( p.numChannels(), original.numChannels() )
		self.assertEqual( p.channelNames(), original.channelNames() )
		self.assertEqual( len(p), len(original) )
		for i in range( 0, len(p) ) :
			self.assertEqual( p.getDepth( i ), original.getDepth( i ) )
			for j in range( 0, 4 ) :
				self.assertAlmostEqual( p[i][j], expected[i][j], 5 )
		
		p2 = IECore.DeepPixel( p )
		p3 = IECore.DeepPixel( simple )
		p3[1] = [ 0.375, 0.25, 0.625, 1.0 ]
		p3.addSample( 5.5, [ 0.1875, 0.75, 0.3125, 0.4375 ] )
		p4 = IECore.DeepPixel( original )
		p4[5] = [ 0.375, 0.25, 0.625, 1.0 ]
		p4[6] = [ 0.625, 0.5, 0.125, 1.0 ]
		p4.removeSample( 3 )
		p4.removeSample( 4 )
		
		expected = [
			( 0.775, 0.149999, 0.225, 0.599999 ),
			( 0.109375, 0.843750, 0.190624, 0.475 ),
			( 0.151874, 0.138749, 0.848125, 0.362499 ),
			( 0.742499, 0.179090, 0.223409, 0.863068 ),
			( 0.210374, 0.856499, 0.076624, 0.985374 ),
			( 0.252276, 0.268578, 0.638855, 0.972999 ),
			( 0.565625, 0.512499, 0.096875, 0.971874 ),
			( 0.196875, 0.512499, 0.428125, 0.971874 ),
		]
		
		p = IECore.DeepPixel.average( [ original, simple, p2, p3, p4 ], [ 0.5, 0.2, 0.1, 0.05, 0.15 ] )
		self.assertEqual( p.numChannels(), original.numChannels() )
		self.assertEqual( p.channelNames(), original.channelNames() )
		self.assertEqual( len(p), len(original) )
		for i in range( 0, len(p) ) :
			self.assertEqual( p.getDepth( i ), original.getDepth( i ) )
			for j in range( 0, 4 ) :
				self.assertAlmostEqual( p[i][j], expected[i][j], 5 )
		
		self.assertRaises( RuntimeError, IECore.curry( IECore.DeepPixel.average, [], [] ) )
		self.assertRaises( RuntimeError, IECore.curry( IECore.DeepPixel.average, [ original, simple ], [ 0.5 ] ) )
		self.assertRaises( RuntimeError, IECore.curry( IECore.DeepPixel.average, [ original, simple ], [ 0.5, 0.5, 0.5 ] ) )
		self.assertRaises( RuntimeError, IECore.curry( IECore.DeepPixel.average, [ IECore.DeepPixel( "RGB" ), simple ], [ 0.5, 0.5 ] ) )
		
		self.assertRaises( TypeError, IECore.curry( IECore.DeepPixel.average, [ original, "thatsNoPixel" ], [ 0.5, 0.5 ] ) )
		self.assertRaises( TypeError, IECore.curry( IECore.DeepPixel.average, [ original, simple ], [ 0.5, "thatsNoFloat" ] ) )

if __name__ == "__main__":
    unittest.main()
