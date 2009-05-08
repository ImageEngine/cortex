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

import unittest
from IECore import *

class TestMemoryUsage( unittest.TestCase ) :

	def testMultipleReferences( self ) :

		"""When an object has multiple references to the same child, that child
		should not be counted multiple times in the memory usage total."""

		c = CompoundObject()
		d = IntVectorData( 10000 )

		c["a"] = d

		m = c.memoryUsage()
		dm = d.memoryUsage()

		c["b"] = d
		self.assert_( c.memoryUsage() < m + dm )

	def testCopiedDataReferences( self ) :

		"""Copied data shouldn't use additional memory unless the copies have
		been modified by writing."""

		c = CompoundObject()
		d = IntVectorData( 10000 )

		c["a"] = d
		c["b"] = d.copy()

		c2 = CompoundObject()
		c2["a"] = d
		c2["b"] = d
		self.assert_( abs( c.memoryUsage() - c2.memoryUsage() ) < 10 )

		# writing to the copy should now increase the memory usage
		m = c.memoryUsage()
		c["b"][0] = 100
		self.assert_( c.memoryUsage()!=m )

if __name__ == "__main__":
    unittest.main()
