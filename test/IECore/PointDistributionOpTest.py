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

import os
import unittest
from IECore import *

class PointDistributionOpTest( unittest.TestCase ) :

	def pointTest( self, mesh, points, density, error=0.05 ) :
		
		self.failUnless( "P" in points )
		self.assertEqual( points.numPoints, points['P'].data.size() )
		self.failUnless( points.arePrimitiveVariablesValid() )
		
		mesh = TriangulateOp()( input = mesh )
		meshEvaluator = MeshPrimitiveEvaluator( mesh )
		result = meshEvaluator.createResult()
		pointsPerFace = [ 0 ] * mesh.verticesPerFace.size()
		positions = points["P"].data
		
		## test that the points are on the mesh
		for p in positions :
			self.assertAlmostEqual( meshEvaluator.signedDistance( p ), 0.0, 3 )
			meshEvaluator.closestPoint( p, result )
			pointsPerFace[result.triangleIndex()] += 1
		
		## test that we have roughly the expected density per face
		origDensity = density
		mesh = FaceAreaOp()( input = mesh )
		for f in range( 0, mesh.verticesPerFace.size() ) :
			if "density" in mesh :
				density = mesh["density"].data[f] * origDensity
			self.failUnless( abs(pointsPerFace[f] - density * mesh['faceArea'].data[f]) <= density * error )
	
	def testSimple( self ) :
		
		m = Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()
		p = PointDistributionOp()( mesh = m, density = 100 )
		self.pointTest( m, p, 100 )
	
	def testHighDensity( self ) :
		
		m = Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()
		p = PointDistributionOp()( mesh = m, density = 50000 )
		self.pointTest( m, p, 50000 )
	
	def testDensityMaskPrimVar( self ) :
		
		m = Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()
		m = TriangulateOp()( input = m )
		numFaces = m.variableSize( PrimitiveVariable.Interpolation.Uniform )
		m['density'] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( [ float(x)/numFaces for x in range( 0, numFaces ) ] ) )
		p = PointDistributionOp()( mesh = m, density = 100 )
		self.pointTest( m, p, 100, error=0.1 )
		p = PointDistributionOp()( mesh = m, density = 1000 )
		self.pointTest( m, p, 1000, error=0.1 )
	
	def testOffsetParameter( self ) :
		
		m = Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()
		density = 500
		p = PointDistributionOp()( mesh = m, density = density, offset = V2f( 0, 0 ) )
		pOffset = PointDistributionOp()( mesh = m, density = density, offset = V2f( 0.5, 0.75 ) )
		self.pointTest( m, p, density )
		self.pointTest( m, pOffset, density )
		self.assertNotEqual( p.numPoints, pOffset.numPoints )
		
		pos = p["P"].data
		posOffset = pOffset["P"].data
		for i in range( 0, min(p.numPoints, pOffset.numPoints) ) :
			self.assertNotEqual( pos[i], posOffset[i] )
	
	def testDistanceBetweenPoints( self ) :
		
		m = Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()
		
		density = 300
		points = PointDistributionOp()( mesh = m, density = density )
		positions = points["P"].data
		
		tree = V3fTree( points["P"].data )
		for i in range( 0, positions.size() ) :
			neighbours = list(tree.nearestNNeighbours( positions[i], 6 ))
			self.failUnless( i in neighbours )
			neighbours.remove( i )
			for n in neighbours :
				self.assert_( ( positions[i] - positions[n] ).length() > 1.0 / density )
	
	def testPointOrder( self ) :
		
		m = Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()
		m2 = m.copy()
		m2['P'].data += V3f( 0, 5, 0 )
		pos = m["P"].data
		pos2 = m2["P"].data
		for i in range( 0, pos.size() ) :
			self.assertNotEqual( pos[i], pos2[i] )

		density = 500
		p = PointDistributionOp()( mesh = m, density = density )
		p2 = PointDistributionOp()( mesh = m2, density = density )
		self.pointTest( m, p, density )
		self.pointTest( m2, p2, density )
		self.assertEqual( p.numPoints, p2.numPoints )
		
		pos = p["P"].data
		pos2 = p2["P"].data
		for i in range( 0, p.numPoints ) :
			self.failUnless( pos2[i].equalWithRelError( pos[i] + V3f( 0, 5, 0 ), 1e-6 ) )
	
	def testDensityRange( self ) :
	
		p = PointDistributionOp()
		p["density"].setNumericValue( -1 )
		
		self.assertRaises( RuntimeError, p["density"].validate )
	
	def setUp( self ) :
	
		os.environ["CORTEX_POINTDISTRIBUTION_TILESET"] = "test/IECore/data/pointDistributions/pointDistributionTileSet2048.dat"

if __name__ == "__main__":
	unittest.main()

