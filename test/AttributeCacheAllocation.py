import os
import math
import random
import VersionControl
VersionControl.setVersion("IECore", "2")
from IECore import *

class TestAttributeCache:

	cachedObjectNames = [ "pSphere1", "pSphere2", "pSphere3", "pCube1" ]
	cachedHeaderNames = [ "header1", "header2" ]
	uncachedObjectNames = [ "pPlane14", "nurbsCurve12" ]

	def testSingleWrite(self):
		"""Test AttributeCache read/write"""
		
		cache = AttributeCache("./AttributeCache.fio", IndexedIOOpenMode.Write)
		
		for obj in self.cachedObjectNames:
			# Make some random vertex data
			
			dataWritten = V3fVectorData()
			
			numPts = int(random.random())
			numPts = numPts * numPts * 500
			
			for i in range(0, numPts):
				dataWritten.append( V3f( random.random(), random.random(), random.random() ) )
			
			cache.write(obj, "P", dataWritten)
		
	def testMultipleWrite(self):
		
		for i in xrange(0,200):
			self.testSingleWrite()

	def printTop( self ):

		os.system( 'top -b -n 1 -p %d' % os.getpid() )
		

print "before single tests"
a = TestAttributeCache()
a.printTop()
a.testSingleWrite()
a.testSingleWrite()
a.testSingleWrite()
print "before multiple"
a.printTop()
a.testMultipleWrite()
print "after multiple 1"
a.printTop()
a.testMultipleWrite()
print "after multiple 2"
a.printTop()
a.testMultipleWrite()
print "after multiple 3"
a.printTop()
