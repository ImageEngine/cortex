##########################################################################
#
#  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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
import math

import IECore

class PointDistributionTest( unittest.TestCase ) :

	def testNoFunctors( self ) :
		
		pd = IECore.PointDistribution.defaultInstance()
		
		bound = IECore.Box2f( IECore.V2f( 0.25 ), IECore.V2f( 0.75 ) )
		points = pd( bound, 20000 )
	
		self.assert_( points.isInstanceOf( IECore.V2fVectorData.staticTypeId() ) )
		self.assert_( abs( len( points ) - 5000 ) < 50 )
		for p in points :
			self.assert_( bound.intersects( p ) )

	def testDensityOnly( self ) :
	
		pd = IECore.PointDistribution.defaultInstance()
		
		bound = IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) )
		
		def density( p ) :
		
			if (p - IECore.V2f( 0.5 )).length() < 0.5 :
				return 1
			else :
				return 0
		
		points = pd( bound, 20000, density )
	
		self.assert_( points.isInstanceOf( IECore.V2fVectorData.staticTypeId() ) )
		self.assert_( abs( len( points ) - math.pi * .5 * .5 * 20000 ) < 50 )
		for p in points :
			self.assert_( bound.intersects( p ) )
			self.assert_( (p - IECore.V2f( 0.5 )).length() < 0.5 )
		
	def testEmitterOnly( self ) :
	
		pd = IECore.PointDistribution.defaultInstance()
		
		bound = IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) )
		
		points = []
		def emit( p ) :
			points.append( p )
			
		result = pd( bound, 1000, None, emit )
		self.assert_( result is None )
		self.assert_( abs( len( points ) - 1000 ) < 50 )

	def testEmitterAndDensity( self ) :
	
		pd = IECore.PointDistribution.defaultInstance()
		
		bound = IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) )
		
		def density( p ) :
		
			if (p - IECore.V2f( 0.5 )).length() < 0.5 :
				return 1
			else :
				return 0
		
		points = []
		def emit( p ) :
			points.append( p )
			
		result = pd( bound, 20000, density, emit )
		
		self.assert_( result is None )
		self.assert_( abs( len( points ) - math.pi * .5 * .5 * 20000 ) < 50 )
		for p in points :
			self.assert_( bound.intersects( p ) )
			self.assert_( (p - IECore.V2f( 0.5 )).length() < 0.5 )
	
	def testDistanceBetweenPoints( self ) :
		
		pd = IECore.PointDistribution.defaultInstance()
		
		bound = IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) )
		
		def density( p ) :
		
			if (p - IECore.V2f( 0.5 )).length() < 0.5 :
				return 1
			else :
				return 0
		
		positions = pd( bound, 20000, density )
		
		tree = IECore.V2fTree( positions )
		
		for i in range( 0, positions.size() ) :
			neighbours = list(tree.nearestNNeighbours( positions[i], 6 ))
			self.failUnless( i in neighbours )
			neighbours.remove( i )
			for n in neighbours :
				self.failUnless( ( positions[i] - positions[n] ).length() > 0.004 )

	def setUp( self ) :
	
		os.environ["CORTEX_POINTDISTRIBUTION_TILESET"] = "test/IECore/data/pointDistributions/pointDistributionTileSet2048.dat"
		
if __name__ == "__main__":
	unittest.main()

