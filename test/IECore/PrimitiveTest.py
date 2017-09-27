##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

class PrimitiveTest( unittest.TestCase ) :

	def test( self ) :

		m = MeshPrimitive( IntVectorData( [ 3, 3 ] ), IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )

		self.assertEqual( m.inferInterpolation( 1 ), PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( 2 ), PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( m.inferInterpolation( 4 ), PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m.inferInterpolation( 6 ), PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m.inferInterpolation( 0 ), PrimitiveVariable.Interpolation.Invalid )
		self.assertEqual( m.inferInterpolation( 10 ), PrimitiveVariable.Interpolation.Invalid )

		self.assertEqual( m.inferInterpolation( FloatData( 1 ) ), PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( V3fVectorData( [ V3f( 1 ) ] ) ), PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( FloatVectorData( [ 2, 3 ] ) ), PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( m.inferInterpolation( IntVectorData( [ 1, 2, 3, 4 ] ) ), PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m.inferInterpolation( IntVectorData( [ 1, 2, 3, 4, 5, 6 ] ) ), PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m.inferInterpolation( IntVectorData( [ 1, 2, 3, 4, 5, 6, 7 ] ) ), PrimitiveVariable.Interpolation.Invalid )

	def testCopyFrom( self ) :

		m = MeshPrimitive( IntVectorData( [ 3, 3 ] ), IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		m["a"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, FloatVectorData( [ 1, 2 ] ), IntVectorData( [ 1, 0, 1, 0, 1, 0 ] ) )
		m2 = MeshPrimitive( IntVectorData( [ 3 ] ), IntVectorData( [ 3, 2, 1 ] ) )
		m2["a"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 1 ) )
		self.assertNotEqual( m, m2 )

		m2.copyFrom( m )
		self.assertEqual( m, m2 )

	def testLoad( self ) :

		m = MeshPrimitive( IntVectorData( [ 3, 3 ] ), IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		m["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData( [ V3f(1), V3f(2), V3f(3), V3f(4) ] ) )
		m["a"] = PrimitiveVariable( PrimitiveVariable.Interpolation.FaceVarying, FloatVectorData( [ 1, 2 ] ), IntVectorData( [ 1, 0, 1, 0, 1, 0 ] ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )

		Writer.create( m, "/tmp/testPrimitiveLoad.cob" ).write()
		m2 = Reader.create( "/tmp/testPrimitiveLoad.cob" ).read()
		self.assertTrue( m2.arePrimitiveVariablesValid() )
		self.assertEqual( m, m2 )

	def testHash( self ) :

		hashes = []

		m = MeshPrimitive( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 1, 2 ] ) )
		hashes.append( m.hash() )

		m["a"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( [ 1 ] ) )
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["b"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( [ 1 ] ) )
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["a"].data[0] = 2
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["b"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( [ 1 ] ), IntVectorData( [ 0 ] ) )
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["b"].indices[0] = 1
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

	def testPrimitiveVariableDataValidity( self ) :

		m = MeshPrimitive( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 1, 2 ] ) )

		# only vector data
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( [ 1 ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatData( 1 ) ) ) )

		# constant can be anything
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1, 2, 3 ] ) ) ) )
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 1 ) ) ) )

		# data size matches interpolation
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( [ 1, 2, 3 ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( [ 1, 2, 3, 4 ] ) ) ) )

		# data size (not base size) matches interpolation
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData( [ V3f(1), V3f(2), V3f(3) ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData( [ V3f(1), V3f(2), V3f(3), V3f(4) ] ) ) ) )

	def testPrimitiveVariableIndicesValidity( self ) :

		m = MeshPrimitive( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 1, 2 ] ) )

		# only vector data
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( [ 1 ] ), IntVectorData( [ 0 ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatData( 1 ), IntVectorData( [ 0 ] ) ) ) )

		# constant needs to be vector data if there are indices
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1, 2, 3 ] ), IntVectorData( [ 0 ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 1 ), IntVectorData( [ 0 ] ) ) ) )
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1, 2, 3 ] ), IntVectorData( [ 0 ] ) ) ) )

		# indices must be in range
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Uniform, FloatVectorData( [ 1 ] ), IntVectorData( [ 1 ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1 ] ), IntVectorData( [ 1 ] ) ) ) )

		# indices size matches interpolation, regardless of data size
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( [ 1 ] ), IntVectorData( [ 0, 0, 0 ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( [ 1 ] ), IntVectorData( [ 0, 0, 0, 0 ] ) ) ) )
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( [ 1, 2, 3 ] ), IntVectorData( [ 0, 1, 2 ] ) ) ) )
		self.assert_( not m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, FloatVectorData( [ 1, 2, 3 ] ), IntVectorData( [ 0 ] ) ) ) )
		# except for constant which can have any number of indices
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1 ] ), IntVectorData( [ 0 ] ) ) ) )
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1 ] ), IntVectorData( [ 0, 0 ] ) ) ) )
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1, 2, 3 ] ), IntVectorData( [ 0 ] ) ) ) )
		self.assert_( m.isPrimitiveVariableValid( PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatVectorData( [ 1, 2, 3 ] ), IntVectorData( [ 0, 1, 2 ] ) ) ) )

if __name__ == "__main__":
    unittest.main()
