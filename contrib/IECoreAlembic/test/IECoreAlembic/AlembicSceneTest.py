##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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
import shutil
import unittest

import IECore
import IECoreAlembic

class AlembicSceneTest( unittest.TestCase ) :

	def testConstruction( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/cube.abc"

		s = IECore.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( s.fileName(), fileName )

		s = IECoreAlembic.AlembicScene( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( s.fileName(), fileName )

	def testNonexistentFile( self ) :

		self.assertRaisesRegexp( RuntimeError, "Unable to open file", IECore.SceneInterface.create, "noExisty.abc", IECore.IndexedIO.OpenMode.Read )

	def testHierarchy( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )

		# root

		self.assertEqual( a.childNames(), [ "group1" ] )
		self.assertTrue( a.hasChild( "group1" ) )
		self.assertFalse( a.hasChild( "doesntExist" ) )

		self.assertEqual( a.child( "doesntExist", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, a.child, "doesntExist", IECore.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, a.child, "doesntExist", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )

		# group1

		g = a.child( "group1" )
		self.assertTrue( g )
		self.assertEqual( g.name(), "group1" )
		self.assertEqual( g.path(), [ "group1" ] )

		self.assertEqual( g.childNames(), [ "pCube1"] )
		self.assertTrue( g.hasChild( "pCube1" ) )
		self.assertFalse( g.hasChild( "doesntExist" ) )

		self.assertEqual( g.child( "doesntExist", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, g.child, "doesntExist", IECore.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, g.child, "doesntExist", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )

		# pCube1

		c = g.child( "pCube1" )
		self.assertTrue( c )
		self.assertEqual( c.name(), "pCube1" )
		self.assertEqual( c.path(), [ "group1", "pCube1" ] )

		self.assertEqual( c.childNames(), [] )
		# Alembic does have a child of this name, but we only treat IXforms
		# as children, and treat shapes as objects at the parent location.
		self.assertFalse( c.hasChild( "pCubeShape1" ) )
		self.assertFalse( c.hasChild( "doesntExist" ) )

		self.assertEqual( c.child( "pCubeShape1", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, c.child, "pCubeShape1", IECore.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, c.child, "pCubeShape1", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )

		self.assertEqual( c.child( "doesntExist", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, c.child, "doesntExist", IECore.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, c.child, "doesntExist", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )

		# scene method

		c2 = a.scene( [ "group1", "pCube1" ] )
		self.assertEqual( c2.path(), [ "group1", "pCube1" ] )

		g2 = c.scene( [ "group1" ] )
		self.assertEqual( g2.path(), [ "group1" ] )

	def testStaticHashes( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		g = a.child( "group1" )
		m = g.child( "pCube1" )

		# Make sure all hashes are different
		hashes = [ m.hash( hashType, 0 ) for hashType in a.HashType.values.values() ]
		self.assertEqual( len( hashes ), len( set( hashes ) ) )

		# Make sure none of the hashes are time-sensitive, since
		# this cache is not animated.
		for i, hashType in enumerate( a.HashType.values.values() ) :
			self.assertEqual( m.hash( hashType, 1 ), hashes[i] )

	def testAnimatedHashes( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )

		# Transform, object, bound and hierarchy hash should change with time
		for hashType in [ a.HashType.TransformHash, a.HashType.ObjectHash, a.HashType.BoundHash, a.HashType.HierarchyHash ] :
			self.assertNotEqual( m.hash( hashType, 0 ), m.hash( hashType, 1 ) )

		# Childnames and attributes should be static still
		for hashType in [ a.HashType.ChildNamesHash, a.AttributesHash ] :
			self.assertEqual( m.hash( hashType, 0 ), m.hash( hashType, 1 ) )

	def testHasObject( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( a.hasObject() )

		g = a.child( "group1" )
		self.assertFalse( g.hasObject() )

		c = g.child( "pCube1" )
		self.assertTrue( c.hasObject() )

	def testConvertMesh( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )

		c = a.child( "group1" ).child( "pCube1" )
		self.assertTrue( c.hasObject() )

		m = c.readObjectAtSample( 0 )
		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )

	def testBound( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -2 ), IECore.V3d( 2 ) ) )

		cs = a.child( "group1" ).child( "pCube1" )
		self.assertEqual( cs.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )

	def testTransform( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.numTransformSamples(), 0 )

		c = a.child( "pCube1" )
		self.assertEqual( c.numTransformSamples(), 10 )

		matrix = c.readTransformAsMatrixAtSample( 0 )
		self.assertEqual( matrix, IECore.M44d() )

		for i in range( 1, c.numTransformSamples() ) :
			matrix2 = c.readTransformAsMatrixAtSample( i )
			self.assertNotEqual( matrix, matrix2 )
			expectedMatrix = IECore.M44d.createTranslated( IECore.V3d( i / 9.0, 0, 0 ) )
			self.failUnless( matrix2.equalWithAbsError( expectedMatrix, 0.0000001 ) )

		self.assertEqual(
			c.readTransformAsMatrixAtSample( c.numTransformSamples() - 1 ),
			IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )
		)

	def testConvertSubD( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/subdPlane.abc", IECore.IndexedIO.OpenMode.Read )

		c = a.child( "pPlane1" )
		m = c.readObjectAtSample( 0 )
		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )
		self.assertEqual( m.interpolation, "catmullClark" )

	def testConvertArbGeomParams( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/coloredMesh.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pPlane1" ).readObjectAtSample( 0 )

		self.failUnless( m.arePrimitiveVariablesValid() )

		self.failUnless( "colorSet1" in m )
		self.assertEqual( m["colorSet1"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.failUnless( isinstance( m["colorSet1"].data, IECore.Color4fVectorData ) )
		self.assertEqual( len( m["colorSet1"].data ), 4 )
		self.assertEqual(
			m["colorSet1"].data,
			IECore.Color4fVectorData( [
				IECore.Color4f( 1, 0, 0, 1 ),
				IECore.Color4f( 0, 0, 0, 1 ),
				IECore.Color4f( 0, 0, 1, 1 ),
				IECore.Color4f( 0, 1, 0, 1 ),
			] )
		)

		self.failUnless( "ABC_int" in m )
		self.assertEqual( m["ABC_int"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m["ABC_int"].data, IECore.IntVectorData( [ 10 ] ) )

	def testConvertUVs( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/coloredMesh.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pPlane1" ).readObjectAtSample( 0 )

		self.failUnless( "s" in m )
		self.failUnless( "t" in m )

		self.assertEqual( m["s"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m["t"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.failUnless( isinstance( m["s"].data, IECore.FloatVectorData ) )
		self.failUnless( isinstance( m["t"].data, IECore.FloatVectorData ) )

	def testSamples( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( a.numBoundSamples(), 10 )
		for i in range( 0, a.numBoundSamples() ) :
			self.assertAlmostEqual( a.boundSampleTime( i ), (i + 1) / 24.0 )

		p = a.child( "persp" )
		self.assertEqual( p.numTransformSamples(), 1 )
		self.assertEqual( p.transformSampleTime( 0 ), 1 / 24.0 )
		self.assertEqual( p.numObjectSamples(), 1 )
		self.assertEqual( p.objectSampleTime( 0 ), 1 / 24.0 )

		t = a.child( "pCube1" )
		self.assertEqual( t.numTransformSamples(), 10 )
		self.assertEqual( t.numObjectSamples(), 10 )
		self.assertEqual( t.numBoundSamples(), 10 )
		for i in range( 0, t.numTransformSamples() ) :
			self.assertAlmostEqual( t.transformSampleTime( i ), (i + 1) / 24.0 )
			self.assertAlmostEqual( t.objectSampleTime( i ), (i + 1) / 24.0 )
			self.assertAlmostEqual( t.boundSampleTime( i ), (i + 1) / 24.0 )

	def testMissingArchiveBounds( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/noTopLevelStoredBounds.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( a.hasBound() )
		self.assertRaisesRegexp( Exception, "Exception : No stored bounds available", a.boundSampleTime, 0 )
		self.assertRaisesRegexp( Exception, "Exception : No stored bounds available", a.readBoundAtSample, 0 )

	def testSampleInterval( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		# persp has only one sample, so should always be reading from that regardless the time
		p = a.child( "persp" )
		t = -1000
		while t < 1000 :
			t += .01
			self.assertEqual( p.transformSampleInterval( t ), ( 0, 0, 0 ) )

		# pCube1 has a sample per frame
		m = a.child( "pCube1" )
		self.assertEqual( m.numTransformSamples(), 10 )
		for i in range( 0, m.numTransformSamples() ) :
			# Reads on the frame should not need
			# interpolation.
			interval = m.transformSampleInterval( m.transformSampleTime( i ) )
			self.assertEqual( interval, ( 0.0, i, i ) )
			# Reads in between frames should need
			# interpolation.
			if i < m.numTransformSamples() - 1 :
				interval = m.transformSampleInterval( m.transformSampleTime( i ) + 1 / 48.0 )
				self.assertAlmostEqual( interval[0], 0.5 )
				self.assertEqual( interval[1], i )
				self.assertEqual( interval[2], i + 1 )

	def testObjectAtSample( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )

		mesh = m.readObjectAtSample( 0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )

		for i in range( 1, m.numObjectSamples() ) :
			mesh2 = m.readObjectAtSample( i )
			self.failUnless( isinstance( mesh2, IECore.MeshPrimitive ) )
			self.assertEqual( mesh.verticesPerFace, mesh2.verticesPerFace )
			self.assertNotEqual( mesh["P"], mesh2["P"] )

	def testConvertInterpolated( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )

		mesh0 = m.readObjectAtSample( 0 )
		mesh1 = m.readObjectAtSample( 1 )

		mesh = m.readObject( 1.5 / 24.0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )

		self.assertEqual( mesh, IECore.linearObjectInterpolation( mesh0, mesh1, 0.5 ) )

	def testRotatingTransformAtSample( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/rotatingCube.abc", IECore.IndexedIO.OpenMode.Read )

		t = a.child( "pCube1" )
		for i in range( 0, 24 ) :
			ti = t.readTransformAsMatrixAtSample( i )
			mi = IECore.M44d.createRotated( IECore.V3d( IECore.degreesToRadians( 90 * i ), 0, 0 ) )
			self.failUnless( ti.equalWithAbsError( mi, 0.0000000000001 ) )

	def testInterpolatedTranslate( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		t = a.child( "pCube1" )
		self.assertEqual( t.numTransformSamples(), 10 )

		for i in range( 0, t.numTransformSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.readTransformAsMatrix( time )
			expectedMatrix = IECore.M44d.createTranslated( IECore.V3d( i / 18.0, 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )

	def testInterpolatedRotate( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/rotatingCube.abc", IECore.IndexedIO.OpenMode.Read )
		t = a.child( "pCube1" )
		self.assertEqual( t.numTransformSamples(), 24 )

		for i in range( 0, t.numTransformSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.readTransformAsMatrix( time )
			expectedMatrix = IECore.M44d.createRotated( IECore.V3d( IECore.degreesToRadians( 90 * i * 0.5 ), 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )

	def testHasBound( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( a.hasBound(), True )
		self.assertEqual( a.child( "persp" ).hasBound(), False )
		self.assertEqual( a.child( "pCube1" ).hasBound(), True )
		self.assertEqual( a.child( "front" ).hasBound(), False )
		self.assertEqual( a.child( "front" ).hasBound(), False )

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.child( "group1" ).hasBound(), False )

	def testBoundAtSample( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( a.readBoundAtSample( a.numBoundSamples()-1 ), IECore.Box3d( IECore.V3d( 0.5, -0.5, -0.5 ), IECore.V3d( 1.5, 2, 0.5 ) ) )

		t = a.child( "pCube1" )
		self.assertEqual( t.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( t.readBoundAtSample( t.numBoundSamples()-1 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 2, 0.5 ) ) )

	def testBoundAtTime( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )

		startTime = a.boundSampleTime( 0 )
		endTime = a.boundSampleTime( a.numBoundSamples() - 1 )

		aStartBound = a.readBoundAtSample( 0 )
		aEndBound = a.readBoundAtSample( a.numBoundSamples() - 1 )

		mStartBound = m.readBoundAtSample( 0 )
		mEndBound = m.readBoundAtSample( m.numBoundSamples() - 1 )

		def lerp( a, b, x ) :

			return a + ( b - a ) * x

		def lerpBox( a, b, x ) :

			r = a.__class__()
			r.min = lerp( a.min, b.min, x )
			r.max = lerp( a.max, b.max, x )
			return r

		numSteps = 100
		for i in range( 0, numSteps ) :

			lerpFactor = ( float( i ) / (numSteps-1) )
			time = lerp( startTime, endTime, lerpFactor )

			aBound = a.readBound( time )
			expectedABound = lerpBox( aStartBound, aEndBound, lerpFactor )
			self.failUnless( aBound.min.equalWithAbsError( expectedABound.min, 0.000001 ) )
			self.failUnless( aBound.max.equalWithAbsError( expectedABound.max, 0.000001 ) )

			mBound = m.readBound( time )
			expectedMBound = lerpBox( mStartBound, mEndBound, lerpFactor )
			self.failUnless( mBound.min.equalWithAbsError( expectedMBound.min, 0.000001 ) )
			self.failUnless( mBound.max.equalWithAbsError( expectedMBound.max, 0.000001 ) )

	def testConvertNormals( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		mesh = a.child( "pCube1" ).readObjectAtSample( 0 )

		self.failUnless( "N" in mesh )
		self.failUnless( isinstance( mesh["N"].data, IECore.V3fVectorData ) )
		self.assertEqual( mesh["N"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

	def testCamera( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		c = a.child( "persp" ).readObjectAtSample( 0 )
		self.failUnless( isinstance( c, IECore.Camera ) )

		c = a.child( "persp" ).readObject( 0 )
		self.failUnless( isinstance( c, IECore.Camera ) )

	def testLinearCurves( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/curves.abc", IECore.IndexedIO.OpenMode.Read )
		curves = a.child( "linearLine" ).readObjectAtSample( 0 )

		self.assertTrue( isinstance( curves, IECore.CurvesPrimitive ) )
		self.assertEqual( curves.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( curves.verticesPerCurve(), IECore.IntVectorData( [ 2 ] ) )
		self.assertEqual( curves.periodic(), False )
		self.assertEqual(
			curves["P"].data,
			IECore.V3fVectorData(
				[ IECore.V3f( 2, 0, 1 ), IECore.V3f( 2, 0, -1 ) ],
				IECore.GeometricData.Interpretation.Point
			)
		)
		self.assertTrue( curves.arePrimitiveVariablesValid() )

	def testCurves( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/curves.abc", IECore.IndexedIO.OpenMode.Read )
		curves = a.child( "curve" ).readObjectAtSample( 0 )

		self.assertTrue( isinstance( curves, IECore.CurvesPrimitive ) )
		self.assertEqual( curves.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curves.verticesPerCurve(), IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( curves.periodic(), False )
		self.assertTrue( curves.arePrimitiveVariablesValid() )

	def testNURBSCircle( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/curves.abc", IECore.IndexedIO.OpenMode.Read )
		curves = a.child( "nurbsCircle" ).readObjectAtSample( 0 )

		self.assertTrue( isinstance( curves, IECore.CurvesPrimitive ) )
		self.assertEqual( curves.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curves.verticesPerCurve(), IECore.IntVectorData( [ 11 ] ) )
		self.assertEqual( curves.periodic(), True )
		self.assertTrue( curves.arePrimitiveVariablesValid() )

	def testPoints( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/points.abc", IECore.IndexedIO.OpenMode.Read )
		points = a.child( "particle1" ).readObjectAtSample( 9 )

		self.assertTrue( isinstance( points, IECore.PointsPrimitive ) )
		self.assertEqual( points.numPoints, 9 )

		self.assertTrue( points.arePrimitiveVariablesValid() )

		self.assertTrue( isinstance( points["P"].data, IECore.V3fVectorData ) )
		self.assertEqual( points["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( points["velocity"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )

		self.assertTrue( isinstance( points["id"].data, IECore.UInt64VectorData) )

		for i in range( 0, 9 ) :

			self.assertTrue(
				points["P"].data[i].normalized().equalWithAbsError(
					points["velocity"].data[i].normalized(),
					0.00001
				),
			)
			self.assertEqual( points["id"].data[i], i )

	def testBoolGeomParam( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/bool.abc", IECore.IndexedIO.OpenMode.Read )
		p = a.child( "pPlane1" )
		mesh = p.readObjectAtSample( 0 )

		for n in ( "abc_testBoolTrue", "abc_testBoolFalse" ) :
			self.assertIn( n, mesh )
			self.assertEqual( mesh[n].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( mesh["abc_testBoolTrue"].data, IECore.BoolVectorData( [ True ] ) )
		self.assertEqual( mesh["abc_testBoolFalse"].data, IECore.BoolVectorData( [ False ] ) )

	def testOgawaHashes( self ) :

		hdf5File = os.path.dirname( __file__ ) + "/data/cube.abc"
		hdf5FileCopy = "/tmp/hdf5Copy.abc"
		ogawaFile = os.path.dirname( __file__ ) + "/data/points.abc"
		ogawaFileCopy = "/tmp/ogawaCopy.abc"

		shutil.copy( hdf5File, hdf5FileCopy )
		shutil.copy( ogawaFile, ogawaFileCopy )

		# HDF5 doesn't store any hashes, so our hash must include the filename
		hdf5 = IECore.SceneInterface.create( hdf5File, IECore.IndexedIO.OpenMode.Read )
		hdf5Copy = IECore.SceneInterface.create( hdf5FileCopy, IECore.IndexedIO.OpenMode.Read )
		self.assertNotEqual(
			hdf5.child( "group1" ).child( "pCube1" ).hash( IECore.SceneInterface.HashType.ObjectHash, 0 ),
			hdf5Copy.child( "group1" ).child( "pCube1" ).hash( IECore.SceneInterface.HashType.ObjectHash, 0 ),
		)

		# Ogawa stores hashes, so we can use them to create hashes based purely on the
		# data. This means our hashes can be independent of the file the objects are stored
		# in, or the location they are stored at.
		ogawa = IECore.SceneInterface.create( ogawaFile, IECore.IndexedIO.OpenMode.Read )
		ogawaCopy = IECore.SceneInterface.create( ogawaFileCopy, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual(
			ogawa.child( "particle1" ).hash( IECore.SceneInterface.HashType.ObjectHash, 0 ),
			ogawaCopy.child( "particle1" ).hash( IECore.SceneInterface.HashType.ObjectHash, 0 ),
		)

	def testSharedSceneInterfaces( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/points.abc"
		s = IECore.SharedSceneInterfaces.get( fileName )

		self.assertIsInstance( s, IECoreAlembic.AlembicScene )
		self.assertEqual( s.fileName(), fileName )

	def testWindingOrder( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/subdPlane.abc", IECore.IndexedIO.OpenMode.Read )
		c = a.child( "pPlane1" )
		m = c.readObjectAtSample( 0 )

		IECore.MeshNormalsOp()( input = m, copyInput = False )
		for n in m["N"].data :
			self.assertTrue( n.equalWithAbsError( IECore.V3f( 0, 1, 0 ), 0.000001 ) )

if __name__ == "__main__":
    unittest.main()
