##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

import math
import random
import unittest
from IECore import *

class TestBoundedKDTree:

	treeSizes = [0, 1, 10, 50, 5000]
	
	def doIntersect(self, numBounds):
		
		self.makeTree(numBounds)
		
		numTests = 25
		if numBounds:
			numTests = (int)( math.log10( numBounds ) + 1 ) * 25
		
		for j in range(0, numTests):
		
			bound = self.makeBound()			
			bIdxArray = self.tree.intersect( bound )
				
			for bIdx in bIdxArray:
				self.assert_(bIdx >= 0)
				self.assert_(bIdx < numBounds)
				nearestBound = self.bounds[bIdx]
		
				self.assert_( nearestBound.intersects( bound ) )

	
class TestBoundedKDTreeBox3f(unittest.TestCase, TestBoundedKDTree):

	def makeBound( self ) :
		b1 = V3f( random.random(), random.random(), random.random() )
		b2 = V3f( random.random(), random.random(), random.random() )
		bound = Box3f( b1 )
		bound.extendBy( b2 )
		
		return bound
		

	def makeTree(self, numBounds):
		# Make tree creation repeatable, but different for every size
		random.seed(100 + 5 * numBounds)
		self.bounds = Box3fVectorData()
		
		for i in range(0, numBounds):
			self.bounds.append( self.makeBound() )
				
		self.tree = Box3fTree( self.bounds )
	
	def testConstructors(self):
		"""Test BoundedKDTreeBox3f constructors"""
		
		for t in self.treeSizes:
			self.makeTree(t)
			
	def testIntersect(self):
		"""Test BoundedKDTreeBox3f intersect"""
		
		for t in self.treeSizes:
			self.doIntersect(t)			
	

class TestBoundedKDTreeBox3d(unittest.TestCase, TestBoundedKDTree):

	def makeBound( self ) :
		b1 = V3d( random.random(), random.random(), random.random() )
		b2 = V3d( random.random(), random.random(), random.random() )
		bound = Box3d( b1 )
		bound.extendBy( b2 )
		
		return bound
		

	def makeTree(self, numBounds):
		# Make tree creation repeatable, but different for every size
		random.seed(100 + 5 * numBounds)
		self.bounds = Box3dVectorData()
		
		for i in range(0, numBounds):
			self.bounds.append( self.makeBound() )
				
		self.tree = Box3dTree( self.bounds )
	
	def testConstructors(self):
		"""Test BoundedKDTreeBox3d constructors"""
		
		for t in self.treeSizes:
			self.makeTree(t)
			
	def testIntersect(self):
		"""Test BoundedKDTreeBox3d intersect"""
		
		for t in self.treeSizes:
			self.doIntersect(t)
	
class TestBoundedKDTreeBox2f(unittest.TestCase, TestBoundedKDTree):

	def makeBound( self ) :
		b1 = V2f( random.random(), random.random() )
		b2 = V2f( random.random(), random.random() )
		bound = Box2f( b1 )
		bound.extendBy( b2 )
		
		return bound


	def makeTree(self, numBounds):
		# Make tree creation repeatable, but different for every size
		random.seed(100 + 5 * numBounds)
		self.bounds = Box2fVectorData()
		
		for i in range(0, numBounds):
			self.bounds.append( self.makeBound() )
				
		self.tree = Box2fTree( self.bounds )
	
	def testConstructors(self):
		"""Test BoundedKDTreeBox2f constructors"""
		
		for t in self.treeSizes:
			self.makeTree(t)
			
	def testIntersect(self):
		"""Test BoundedKDTreeBox2f intersect"""
		
		for t in self.treeSizes:
			self.doIntersect(t)	
			


	
if __name__ == "__main__":
	unittest.main()
