##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

import random
import unittest
import imath
import IECore

class TestKDTree:
	treeSizes = [0, 1, 10, 100]
	radii = [0.01, 0.05, 0.1, 1]
	numNeighbours = [1, 4, 10, 20]

	def doNearestNeighbour(self, numPoints):

		self.makeTree(numPoints)

		for i in range(0, numPoints):
			pIdx = self.tree.nearestNeighbour( self.points[i] )
			self.assertTrue(pIdx >= 0)
			self.assertTrue(pIdx < numPoints)
			nearestPt = self.points[pIdx]

			self.assertEqual( pIdx, i )
			self.assertTrue( nearestPt.equalWithRelError(self.points[i], 0.00001) )

	def doNearestNeighbours(self, numPoints):

		self.makeTree(numPoints)

		for i in range(0, numPoints):
			for r in self.radii:
				pIdxArray = self.tree.nearestNeighbours( self.points[i], r )

				for pIdx in pIdxArray:
					self.assertTrue(pIdx >= 0)
					self.assertTrue(pIdx < numPoints)
					nearestPt = self.points[pIdx]
					distToNearest = (self.points[pIdx] - self.points[i]).length()

					self.assertTrue( distToNearest <= r )

	def doNearestNNeighbours(self, numPoints):

		self.makeTree( numPoints )

		for i in range(0, numPoints) :

			for n in self.numNeighbours :

				testPoint = self.points[i]

				pIdxArray = self.tree.nearestNNeighbours( testPoint, n )

				# check we've been given as many neighbours as we asked for,
				# or the whole tree if there aren't enough points
				self.assertEqual( len( pIdxArray ), min( n, numPoints ) )

				# check that all indices are in range
				for pIdx in pIdxArray :

					self.assertTrue(pIdx >= 0)
					self.assertTrue(pIdx < numPoints)

				# check that points are ordered by distance
				d = ( self.points[pIdxArray[0]] - testPoint ).length()
				for pIdx in pIdxArray[1:] :

					dd = ( self.points[pIdx] - testPoint ).length()
					self.assertTrue( dd >= d )
					d = dd

				# check that no points not in neighbours are closer than
				# the furthest neighbour
				furthestNeighbourDistance = (self.points[pIdxArray[0]] - testPoint).length()
				for i in range( 0, numPoints ) :

					if not i in pIdxArray :

						d = (self.points[i] - testPoint).length()
						self.assertTrue( d > furthestNeighbourDistance )

	def doEnclosedPoints( self, numPoints ) :

		self.makeTree( numPoints )

		for i in range( 0, 1000 ) :

			b = self.randomBox()

			p = self.tree.enclosedPoints( b )

			s = set( p )
			for i in range( self.points.size() ) :

				if b.intersects( self.points[i] ) :
					self.assertTrue( i in s )
				else :
					self.assertFalse( i in s )


class TestKDTreeV2f(unittest.TestCase, TestKDTree):

	def makeTree(self, numPoints):
		# Make tree creation repeatable, but different for every size
		random.seed(100 + 5 * numPoints)
		self.points = IECore.V2fVectorData()

		for i in range(0, numPoints):
			self.points.append( imath.V2f( random.random(), random.random() ) )

		self.tree = IECore.V2fTree( self.points )

	def randomBox( self ) :

		min = imath.V2f( random.random(), random.random() )
		max = min + imath.V2f( random.random(), random.random() )
		return imath.Box2f( min, max )

	def testConstructors(self):
		"""Test KDTreeV2f constructors"""

		for t in self.treeSizes:
			self.makeTree(t)

	def testNearestNeighbour(self):
		"""Test KDTreeV2f nearestNeighbour"""

		for t in self.treeSizes:
			self.doNearestNeighbour(t)

	def testNearestNeighbours(self):
		"""Test KDTreeV2f nearestNeighbours"""

		for t in self.treeSizes:
			self.doNearestNeighbours(t)

	def testNearestNNeighbours(self):
		"""Test KDTreeV2f nearestNNeighbours"""

		for t in self.treeSizes:
			self.doNearestNNeighbours(t)

	def testEnclosedPoints(self):
		"""Test KDTreeV2f enclosedPoints"""

		for t in self.treeSizes:
			self.doEnclosedPoints(t)

class TestKDTreeV2d(unittest.TestCase, TestKDTree):

	def makeTree(self, numPoints):
		# Make tree creation repeatable, but different for every size
		random.seed(100 + 5 * numPoints)
		self.points = IECore.V2dVectorData()

		for i in range(0, numPoints):
			self.points.append( imath.V2d( random.random(), random.random() ) )

		self.tree = IECore.V2dTree( self.points )

	def randomBox( self ) :

		min = imath.V2d( random.random(), random.random() )
		max = min + imath.V2d( random.random(), random.random() )
		return imath.Box2d( min, max )

	def testConstructors(self):
		"""Test KDTreeV2d constructors"""

		for t in self.treeSizes:
			self.makeTree(t)

	def testNearestNeighbour(self):
		"""Test KDTreeV2d nearestNeighbour"""

		for t in self.treeSizes:
			self.doNearestNeighbour(t)

	def testNearestNeighbours(self):
		"""Test KDTreeV2d nearestNeighbours"""

		for t in self.treeSizes:
			self.doNearestNeighbours(t)

	def testNearestNNeighbours(self):
		"""Test KDTreeV2d nearestNNeighbours"""

		for t in self.treeSizes:
			self.doNearestNNeighbours(t)

	def testEnclosedPoints(self):
		"""Test KDTreeV2d enclosedPoints"""

		for t in self.treeSizes:
			self.doEnclosedPoints(t)

class TestKDTreeV3f(unittest.TestCase, TestKDTree):

	def makeTree(self, numPoints):
		# Make tree creation repeatable, but different for every size
		random.seed(100 + 5 * numPoints)
		self.points = IECore.V3fVectorData()

		for i in range(0, numPoints):
			self.points.append( imath.V3f( random.random(), random.random(), random.random() ) )

		self.tree = IECore.V3fTree( self.points )

	def randomBox( self ) :

		min = imath.V3f( random.random(), random.random(), random.random() )
		max = min + imath.V3f( random.random(), random.random(), random.random() )
		return imath.Box3f( min, max )

	def testConstructors(self):
		"""Test KDTreeV3f constructors"""

		for t in self.treeSizes:
			self.makeTree(t)

	def testNearestNeighbour(self):
		"""Test KDTreeV3f nearestNeighbour"""

		for t in self.treeSizes:
			self.doNearestNeighbour(t)

	def testNearestNeighbours(self):
		"""Test KDTreeV3f nearestNeighbours"""

		for t in self.treeSizes:
			self.doNearestNeighbours(t)

	def testNearestNNeighbours(self):
		"""Test KDTreeV3f nearestNNeighbours"""

		for t in self.treeSizes:
			self.doNearestNNeighbours(t)

	def testEnclosedPoints(self):
		"""Test KDTreeV3f enclosedPoints"""

		for t in self.treeSizes:
			self.doEnclosedPoints(t)

class TestKDTreeV3d(unittest.TestCase, TestKDTree):

	def makeTree(self, numPoints):
		# Make tree creation repeatable, but different for every size
		random.seed(100 + 5 * numPoints)
		self.points = IECore.V3dVectorData()

		for i in range(0, numPoints):
			self.points.append( imath.V3d( random.random(), random.random(), random.random() ) )

		self.tree = IECore.V3dTree( self.points )

	def randomBox( self ) :

		min = imath.V3d( random.random(), random.random(), random.random() )
		max = min + imath.V3d( random.random(), random.random(), random.random() )
		return imath.Box3d( min, max )

	def testConstructors(self):
		"""Test KDTreeV3d constructors"""

		for t in self.treeSizes:
			self.makeTree(t)

	def testNearestNeighbour(self):
		"""Test KDTreeV3d nearestNeighbour"""

		for t in self.treeSizes:
			self.doNearestNeighbour(t)

	def testNearestNeighbours(self):
		"""Test KDTreeV3d nearestNeighbours"""

		for t in self.treeSizes:
			self.doNearestNeighbours(t)

	def testNearestNNeighbours(self):
		"""Test KDTreeV3d nearestNNeighbours"""

		for t in self.treeSizes:
			self.doNearestNNeighbours(t)

	def testEnclosedPoints(self):
		"""Test KDTreeV3d enclosedPoints"""

		for t in self.treeSizes:
			self.doEnclosedPoints(t)


if __name__ == "__main__":
	unittest.main()
