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
import time
import threading
import imath

import IECore
import IECoreScene

class PrimitiveTest( unittest.TestCase ) :

	def test( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3, 3 ] ), IECore.IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )

		self.assertEqual( m.inferInterpolation( 1 ), IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( 2 ), IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( m.inferInterpolation( 4 ), IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m.inferInterpolation( 6 ), IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m.inferInterpolation( 0 ), IECoreScene.PrimitiveVariable.Interpolation.Invalid )
		self.assertEqual( m.inferInterpolation( 10 ), IECoreScene.PrimitiveVariable.Interpolation.Invalid )

		self.assertEqual( m.inferInterpolation( IECore.FloatData( 1 ) ), IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( IECore.V3fVectorData( [ imath.V3f( 1 ) ] ) ), IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m.inferInterpolation( IECore.FloatVectorData( [ 2, 3 ] ) ), IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( m.inferInterpolation( IECore.IntVectorData( [ 1, 2, 3, 4 ] ) ), IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m.inferInterpolation( IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] ) ), IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m.inferInterpolation( IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6, 7 ] ) ), IECoreScene.PrimitiveVariable.Interpolation.Invalid )

	def testCopyFrom( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3, 3 ] ), IECore.IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		m["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( [ 1, 2 ] ), IECore.IntVectorData( [ 1, 0, 1, 0, 1, 0 ] ) )
		m2 = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 3, 2, 1 ] ) )
		m2["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ) )
		self.assertNotEqual( m, m2 )

		m2.copyFrom( m )
		self.assertEqual( m, m2 )

	def testLoad( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3, 3 ] ), IECore.IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		m["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f(1), imath.V3f(2), imath.V3f(3), imath.V3f(4) ] ) )
		m["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( [ 1, 2 ] ), IECore.IntVectorData( [ 1, 0, 1, 0, 1, 0 ] ) )
		self.assertTrue( m.arePrimitiveVariablesValid() )

		IECore.Writer.create( m, "/tmp/testPrimitiveLoad.cob" ).write()
		m2 = IECore.Reader.create( "/tmp/testPrimitiveLoad.cob" ).read()
		self.assertTrue( m2.arePrimitiveVariablesValid() )
		self.assertEqual( m, m2 )

	def testHash( self ) :

		hashes = []

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ) )
		hashes.append( m.hash() )

		m["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 1 ] ) )
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 1 ] ) )
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["a"].data[0] = 2
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 0 ] ) )
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

		m["b"].indices[0] = 1
		for h in hashes :
			self.assertNotEqual( h, m.hash() )
		hashes.append( m.hash() )

	def testPrimitiveVariableDataValidity( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ) )

		# only vector data
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 1 ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatData( 1 ) ) ) )

		# constant can be anything
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1, 2, 3 ] ) ) ) )
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ) ) ) )

		# data size matches interpolation
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3 ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3, 4 ] ) ) ) )

		# data size (not base size) matches interpolation
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f(1), imath.V3f(2), imath.V3f(3) ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f(1), imath.V3f(2), imath.V3f(3), imath.V3f(4) ] ) ) ) )

	def testPrimitiveVariableIndicesValidity( self ) :

		m = IECoreScene.MeshPrimitive( IECore.IntVectorData( [ 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ) )

		# only vector data
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 0 ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatData( 1 ), IECore.IntVectorData( [ 0 ] ) ) ) )

		# constant needs to be vector data if there are indices
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1, 2, 3 ] ), IECore.IntVectorData( [ 0 ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ), IECore.IntVectorData( [ 0 ] ) ) ) )
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1, 2, 3 ] ), IECore.IntVectorData( [ 0 ] ) ) ) )

		# indices must be in range
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 1 ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 1 ] ) ) ) )

		# indices size matches interpolation, regardless of data size
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 0, 0, 0 ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 0, 0, 0, 0 ] ) ) ) )
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ) ) ) )
		self.assertTrue( not m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1, 2, 3 ] ), IECore.IntVectorData( [ 0 ] ) ) ) )
		# except for constant which can have any number of indices
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 0 ] ) ) ) )
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1 ] ), IECore.IntVectorData( [ 0, 0 ] ) ) ) )
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1, 2, 3 ] ), IECore.IntVectorData( [ 0 ] ) ) ) )
		self.assertTrue( m.isPrimitiveVariableValid( IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 1, 2, 3 ] ), IECore.IntVectorData( [ 0, 1, 2 ] ) ) ) )

	def testVariableIndexedView( self ) :

		IECoreScene.testVariableIndexedView()

	def testCancelLoading( self ) :

		strip = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 100000, 1 ) ), imath.V2i( 1000000, 1 ) )
		testData = IECore.FloatVectorData( [0] * ( len( strip["P"].data ) ) )
		for i in range( 10 ):
			q = IECore.FloatVectorData( testData )
			q[0]  = i
			strip["var%i" % i] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, q )

		saveIO = IECore.MemoryIndexedIO( IECore.CharVectorData(), IECore.IndexedIO.OpenMode.Write )
		strip.save( saveIO, "test" )
		loadIO = IECore.MemoryIndexedIO( saveIO.buffer(), IECore.IndexedIO.OpenMode.Read )

		canceller = IECore.Canceller()
		cancelled = [False]

		def backgroundRun():
			try:
				IECore.Object.load( loadIO, "test", canceller )
			except IECore.Cancelled:
				cancelled[0] = True

		thread = threading.Thread(target=backgroundRun, args=())

		startTime = time.time()
		thread.start()

		time.sleep( 0.05 )
		canceller.cancel()
		thread.join()

		self.assertLess( time.time() - startTime, 0.1 )
		self.assertTrue( cancelled[0] )

if __name__ == "__main__":
    unittest.main()
