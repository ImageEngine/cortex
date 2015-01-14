##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

class InterpolatedCacheIssue( unittest.TestCase ) :

	# This test exposes the fact that continuity of Euler angles isn't preserved by the fact that we're doing
	# quaternion interpolation in the cache interpolation code. See the discussion and todo in include/IECore/TransformationMatrixInterpolator.inl
	# for more details.
	def testNewTransformationMatrixData( self ):

		# Code that created the cache files for this test:
		#cache1 = AttributeCache( "test/IECore/data/attributeCaches/transform.new.0250.fio", IndexedIOOpenMode.Write )
		#cache2 = AttributeCache( "test/IECore/data/attributeCaches/transform.new.0500.fio", IndexedIOOpenMode.Write )
		#for order in [ Eulerd.Order.XYZ, Eulerd.Order.XZY, Eulerd.Order.YZX, Eulerd.Order.YXZ, Eulerd.Order.ZXY, Eulerd.Order.ZYX ]:
		#	t = TransformationMatrixd()
		#	t.rotate = Eulerd( 1000, -10, 100, order )
		#	transform = TransformationMatrixdData( t )
		#	cache1.write( "test%d" % order, "transform", transform )
		#	t = TransformationMatrixd()
		#	t.rotate = Eulerd( 1002, -12, 102, order )
		#	transform = TransformationMatrixdData( t )
		#	cache2.write( "test%d" % order, "transform", transform )
		#cache1 = None
		#cache2 = None

		cache = InterpolatedCache( "test/IECore/data/attributeCaches/transform.new.####.fio", frame = 1.5, interpolation = InterpolatedCache.Interpolation.Linear )
		for order in [ Eulerd.Order.XYZ, Eulerd.Order.XZY, Eulerd.Order.YZX, Eulerd.Order.YXZ, Eulerd.Order.ZXY, Eulerd.Order.ZYX ]:
			self.assertEqual( cache.read( "test%d" % order, "transform" ).value.rotate, Eulerd( 1001, -11, 101, order ) )
		cache = None

if __name__ == "__main__":
	unittest.main()

