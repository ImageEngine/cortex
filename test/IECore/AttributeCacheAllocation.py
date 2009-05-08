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
