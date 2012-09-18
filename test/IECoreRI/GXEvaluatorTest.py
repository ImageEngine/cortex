##########################################################################
#
#  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import unittest
import threading
import random
import math

import IECore
import IECoreRI

class GXEvaluatorTest( unittest.TestCase ) :

	def test( self ) :
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ) ) )
		e = IECoreRI.GXEvaluator( m )
		
		self.assertEqual( e.numFaces(), 1 )
		
		points = e.evaluate(
			IECore.IntVectorData( [ 0 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			[ "P", "s", "t" ],
		)
		
		self.failUnless( len( points ), 3 )
		self.failUnless( "P" in points )
		self.failUnless( "s" in points )
		self.failUnless( "t" in points )
		
		self.assertEqual( len( points["P"] ), 1 )
		self.assertEqual( len( points["s"] ), 1 )
		self.assertEqual( len( points["t"] ), 1 )
		
		self.assertEqual( points["P"][0], IECore.V3f( .5, .5, 0 ) )
		self.assertEqual( points["s"][0], .5 )
		self.assertEqual( points["t"][0], .5 )
		
	def testInputLengthValidation( self ) :
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ) ) )
		e = IECoreRI.GXEvaluator( m )

		self.assertRaises(
			RuntimeError,
			e.evaluate,
			IECore.IntVectorData( [ 0 ] ),
			IECore.FloatVectorData( [] ),
			IECore.FloatVectorData( [ .5 ] ),
			[ "P", "s", "t" ],
		)
		
		self.assertRaises(
			RuntimeError,
			e.evaluate,
			IECore.IntVectorData( [ 0 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			IECore.FloatVectorData( [] ),
			[ "P", "s", "t" ],
		)
		
		self.assertRaises(
			RuntimeError,
			e.evaluate,
			IECore.IntVectorData( [] ),
			IECore.FloatVectorData( [ .5 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			[ "P", "s", "t" ],
		)
		
	def testFaceIndexValidation( self ) :
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ) ) )
		e = IECoreRI.GXEvaluator( m )

		self.assertEqual( e.numFaces(), 1 )
		
		self.assertRaises(
			RuntimeError,
			e.evaluate,
			IECore.IntVectorData( [ -1 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			[ "P", "s", "t" ],
		)
		
		self.assertRaises(
			RuntimeError,
			e.evaluate,
			IECore.IntVectorData( [ 1 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			[ "P", "s", "t" ],
		)
		
		self.assertRaises(
			RuntimeError,
			e.evaluate,
			IECore.IntVectorData( [ 0, 1 ] ),
			IECore.FloatVectorData( [ .5, .5 ] ),
			IECore.FloatVectorData( [ .5, .5 ] ),
			[ "P", "s", "t" ],
		)
	
	def testPrimVarNameValidation( self ) :
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ) ) )
		e = IECoreRI.GXEvaluator( m )
		
		self.failIf( "unlikely" in m )
		
		self.assertRaises(
			RuntimeError,
			e.evaluate,
			IECore.IntVectorData( [ 0 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			IECore.FloatVectorData( [ .5 ] ),
			[ "unlikely" ],
		)
		 
	def testThreading( self ) :
	
		m = IECore.ObjectReader( "test/IECore/data/cobFiles/polySphereQuads.cob" ).read()
		e = IECoreRI.GXEvaluator( m )

		n = e.numFaces()
		f = IECore.IntVectorData()
		u = IECore.FloatVectorData()
		v = IECore.FloatVectorData()
		for i in range( 0, 100 ) :
			f.append( random.randint( 0, n - 1 ) )
			u.append( random.random() )
			v.append( random.random() )

		referencePoints = e.evaluate( f, u, v, [ "P", "s", "t" ] )

		points = []
		def ff( e, f, u, v ) :
			points.append( e.evaluate( f, u, v, [ "P", "s", "t" ] ) )
			
		threads = []
		for i in range( 0, 100 ) :
			t = threading.Thread( target = ff, args = ( e, f, u, v ) )
			t.start()
			threads.append( t )
			
		for t in threads :
			t.join()
			
		self.assertEqual( len( points ), len( threads ) )
		
		for p in points :
			self.assertEqual( p, referencePoints )
			
	def testSTEvaluation( self ) :
	
		m = IECore.ObjectReader( "test/IECoreRI/data/gxSubdivCube.cob" ).read()
		
		# use the cortex evaluator to get points to evaluate
		tm = IECore.TriangulateOp()( input=m )
		tme = IECore.MeshPrimitiveEvaluator( tm )
		tmer = tme.createResult()
		r = IECore.Rand32()
		
		s = IECore.FloatVectorData()
		t = IECore.FloatVectorData()
		for fi in range( 0, tm.numFaces() ) :
			for i in range( 100 ) :
				status = tme.barycentricPosition( fi, r.barycentricf(), tmer )
				self.assertEqual( status, True )
				s.append( tmer.uv()[0] )
				t.append( tmer.uv()[1] )
			
		# then use the GXEvaluator on those points
		e = IECoreRI.GXEvaluator( m )
	
		points = e.evaluate( s, t, [ "Ng", "P", "s", "t" ] )
		
		self.assertEqual( len( points["P"] ), len( s ) )
		self.assertEqual( len( points["s"] ), len( s ) )
		self.assertEqual( len( points["t"] ), len( s ) )
		self.assertEqual( len( points["Ng"] ), len( s ) )
		
		# check the points are ok
		for i in range( 0, len( points["P"] ) ) :
		
			self.failUnless( math.fabs( points["P"][i].length() - 0.425 ) < 0.01 )
			self.failUnless( points["P"][i].normalized().dot( points["Ng"][i].normalized() ) > 0.99 )
			self.failUnless( points["gxStatus"][i] )
			
	def testFailedSTEvaluation( self ) :
	
		m = IECore.ObjectReader( "test/IECoreRI/data/gxSubdivCube.cob" ).read()
		
		# the cube only covers a portion of 0-1 texture space. generate points in
		# this whole area, and then use a cortex evaluator to decide which ones
		# should succeed and which should fail.
		# use the cortex evaluator to get points to evaluate
		tm = IECore.TriangulateOp()( input=m )
		tme = IECore.MeshPrimitiveEvaluator( tm )
		tmer = tme.createResult()
		r = IECore.Rand32()
		
		s = IECore.FloatVectorData()
		t = IECore.FloatVectorData()
		expectedStatuses = IECore.BoolVectorData()
		for fi in range( 0, 100 ) :
			s.append( r.nextf( 0, 1 ) )
			t.append( r.nextf( 0, 1 ) )
			expectedStatuses.append( tme.pointAtUV( IECore.V2f( s[-1], t[-1] ), tmer ) )
					
		# then evaluate those points using the gx library
		e = IECoreRI.GXEvaluator( m )
		
		points = e.evaluate( s, t, [ "P" ] )
		
		# check we get the successes and failures we expect
		
		self.assertEqual( points["gxStatus"], expectedStatuses )
		
	def testZeroLengthInput( self ) :
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0, 0 ), IECore.V2f( 1, 1 ) ) )
		e = IECoreRI.GXEvaluator( m )
		
		self.assertEqual( e.numFaces(), 1 )
		
		points = e.evaluate(
			IECore.IntVectorData( [] ),
			IECore.FloatVectorData( [] ),
			IECore.FloatVectorData( [] ),
			[ "P", "s", "t" ],
		)
		
		self.failUnless( len( points ), 3 )
		self.failUnless( "P" in points )
		self.failUnless( "s" in points )
		self.failUnless( "t" in points )
		
		self.assertEqual( len( points["P"] ), 0 )
		self.assertEqual( len( points["s"] ), 0 )
		self.assertEqual( len( points["t"] ), 0 )
		
		points = e.evaluate(
			IECore.FloatVectorData( [] ),
			IECore.FloatVectorData( [] ),
			[ "P", "s", "t" ],
		)
		
		self.failUnless( len( points ), 3 )
		self.failUnless( "P" in points )
		self.failUnless( "s" in points )
		self.failUnless( "t" in points )
		
		self.assertEqual( len( points["P"] ), 0 )
		self.assertEqual( len( points["s"] ), 0 )
		self.assertEqual( len( points["t"] ), 0 )
		
	def testFailingQuads( self ) :
	
		mesh = IECore.ObjectReader( "test/IECoreRI/data/gxProblemMesh.cob" ).read()
		seeds = IECore.ObjectReader( "test/IECoreRI/data/gxProblemSeeds.cob" ).read()
				
		e = IECoreRI.GXEvaluator( mesh )
		
		points = e.evaluate( 
			seeds["s"].data,
			seeds["t"].data,
			[ "P", "s", "t" ],
		)
		
		pIn = seeds["P"].data
		pOut = points["P"]
		for i in range( 0, len( points["P"] ) ) :
		
			d = ( pIn[i] - pOut[i] ).length()
			self.assertAlmostEqual( d, 0, 5 )
		
if __name__ == "__main__":
    unittest.main()
