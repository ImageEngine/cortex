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
import imath
import ctypes
import threading

import IECore
import IECoreScene
import IECoreAlembic

class AlembicSceneTest( unittest.TestCase ) :

	def testConstruction( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/cube.abc"

		s = IECoreScene.SceneInterface.create( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( s.fileName(), fileName )

		s = IECoreAlembic.AlembicScene( fileName, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( s.fileName(), fileName )

	def testNonexistentFile( self ) :

		self.assertRaisesRegexp( RuntimeError, "Unable to open file", IECoreScene.SceneInterface.create, "noExisty.abc", IECore.IndexedIO.OpenMode.Read )

	def testHierarchy( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )

		# root

		self.assertEqual( a.childNames(), [ "group1" ] )
		self.assertTrue( a.hasChild( "group1" ) )
		self.assertFalse( a.hasChild( "doesntExist" ) )

		self.assertEqual( a.child( "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, a.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, a.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		# group1

		g = a.child( "group1" )
		self.assertTrue( g )
		self.assertEqual( g.name(), "group1" )
		self.assertEqual( g.path(), [ "group1" ] )

		self.assertEqual( g.childNames(), [ "pCube1"] )
		self.assertTrue( g.hasChild( "pCube1" ) )
		self.assertFalse( g.hasChild( "doesntExist" ) )

		self.assertEqual( g.child( "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, g.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, g.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

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

		self.assertEqual( c.child( "pCubeShape1", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, c.child, "pCubeShape1", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, c.child, "pCubeShape1", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		self.assertEqual( c.child( "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ), None )
		self.assertRaises( RuntimeError, c.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, c.child, "doesntExist", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		# scene method

		c2 = a.scene( [ "group1", "pCube1" ] )
		self.assertEqual( c2.path(), [ "group1", "pCube1" ] )

		g2 = c.scene( [ "group1" ] )
		self.assertEqual( g2.path(), [ "group1" ] )

	def testStaticHashes( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
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

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )

		# Transform, object, bound and hierarchy hash should change with time
		for hashType in [ a.HashType.TransformHash, a.HashType.ObjectHash, a.HashType.BoundHash, a.HashType.HierarchyHash ] :
			self.assertNotEqual( m.hash( hashType, 0 ), m.hash( hashType, 1 ) )

	def testStaticAttributeHash( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		b = a.createChild( "b" )

		b.writeAttribute( "testFloat", IECore.FloatData( 0.0 ), 0.0 )

		del b, a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		attributesHash = IECoreScene.SceneInterface.HashType.AttributesHash
		b = a.child( "b" )
		h0 = b.hash( attributesHash, 0.0 )
		h1 = b.hash( attributesHash, 1.0 )

		self.assertEqual( h0, h1 )

	def testAttributeHash( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		b = a.createChild( "b" )
		b.writeAttribute( "testFloat", IECore.FloatData( 0.0 ), 0.0 )
		b.writeAttribute( "testFloat", IECore.FloatData( 1.0 ), 1.0 )

		c = a.createChild( "c" )

		del c, b, a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		b = a.child( "b" )
		c = a.child( "c" )

		attributesHash = IECoreScene.SceneInterface.HashType.AttributesHash
		h0 = b.hash( attributesHash, 0.0 )
		h1 = b.hash( attributesHash, 1.0 )

		self.assertNotEqual( h0, h1 )

		h0 = c.hash( attributesHash, 0.0 )
		h1 = c.hash( attributesHash, 1.0 )

		self.assertEqual( h0, h1 )

		# verify attribute hash of a location with no attributes from another file is the same
		other = IECoreAlembic.AlembicScene( "/tmp/otherFile.abc", IECore.IndexedIO.OpenMode.Write )
		b = other.createChild( "b" )
		del b, other

		other = IECoreAlembic.AlembicScene( "/tmp/otherFile.abc", IECore.IndexedIO.OpenMode.Read )
		hOther = other.child( "b" ).hash( attributesHash, 0.0 )

		self.assertEqual( hOther, h0 )

	def testHasObject( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( a.hasObject() )

		g = a.child( "group1" )
		self.assertFalse( g.hasObject() )

		c = g.child( "pCube1" )
		self.assertTrue( c.hasObject() )

	def testConvertMesh( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )

		c = a.child( "group1" ).child( "pCube1" )
		self.assertTrue( c.hasObject() )

		m = c.readObjectAtSample( 0 )
		self.failUnless( isinstance( m, IECoreScene.MeshPrimitive ) )

	def testBound( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readBoundAtSample( 0 ), imath.Box3d( imath.V3d( -2 ), imath.V3d( 2 ) ) )

		cs = a.child( "group1" ).child( "pCube1" )
		self.assertEqual( cs.readBoundAtSample( 0 ), imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) ) )

	def testTransform( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.numTransformSamples(), 0 )

		c = a.child( "pCube1" )
		self.assertEqual( c.numTransformSamples(), 10 )

		matrix = c.readTransformAsMatrixAtSample( 0 )
		self.assertEqual( matrix, imath.M44d() )

		for i in range( 1, c.numTransformSamples() ) :
			matrix2 = c.readTransformAsMatrixAtSample( i )
			self.assertNotEqual( matrix, matrix2 )
			expectedMatrix = imath.M44d().translate( imath.V3d( i / 9.0, 0, 0 ) )
			self.failUnless( matrix2.equalWithAbsError( expectedMatrix, 0.0000001 ) )

		self.assertEqual(
			c.readTransformAsMatrixAtSample( c.numTransformSamples() - 1 ),
			imath.M44d().translate( imath.V3d( 1, 0, 0 ) )
		)

	def testConvertSubD( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/subdPlane.abc", IECore.IndexedIO.OpenMode.Read )

		c = a.child( "pPlane1" )
		m = c.readObjectAtSample( 0 )
		self.failUnless( isinstance( m, IECoreScene.MeshPrimitive ) )
		self.assertEqual( m.interpolation, "catmullClark" )

	def testConvertArbGeomParams( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/coloredMesh.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pPlane1" ).readObjectAtSample( 0 )

		self.failUnless( m.arePrimitiveVariablesValid() )

		self.failUnless( "colorSet1" in m )
		self.assertEqual( m["colorSet1"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.failUnless( isinstance( m["colorSet1"].data, IECore.Color4fVectorData ) )
		self.assertEqual( len( m["colorSet1"].data ), 4 )
		self.assertEqual(
			m["colorSet1"].expandedData(),
			IECore.Color4fVectorData( [
				imath.Color4f( 1, 0, 0, 1 ),
				imath.Color4f( 0, 0, 0, 1 ),
				imath.Color4f( 0, 0, 1, 1 ),
				imath.Color4f( 0, 1, 0, 1 ),
			] )
		)

		self.failUnless( "ABC_int" in m )
		self.assertEqual( m["ABC_int"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m["ABC_int"].data, IECore.IntVectorData( [ 10 ] ) )

	def testConvertUVs( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/coloredMesh.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pPlane1" ).readObjectAtSample( 0 )

		self.failUnless( "uv" in m )

		self.assertEqual( m["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		self.failUnless( isinstance( m["uv"].data, IECore.V2fVectorData ) )
		self.assertEqual( m["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.failUnless( isinstance( m["uv"].indices, IECore.IntVectorData ) )

		self.assertEqual( len(m["uv"].data), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( len(m["uv"].indices), m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) )

		self.failUnless( m.isPrimitiveVariableValid( m["uv"] ) )

	def testSamples( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

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

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/noTopLevelStoredBounds.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( a.hasBound() )
		self.assertRaisesRegexp( IECore.Exception, "No stored bounds available", a.boundSampleTime, 0 )
		self.assertRaisesRegexp( IECore.Exception, "No stored bounds available", a.readBoundAtSample, 0 )

	def testSampleInterval( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

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

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )

		mesh = m.readObjectAtSample( 0 )
		self.failUnless( isinstance( mesh, IECoreScene.MeshPrimitive ) )

		for i in range( 1, m.numObjectSamples() ) :
			mesh2 = m.readObjectAtSample( i )
			self.failUnless( isinstance( mesh2, IECoreScene.MeshPrimitive ) )
			self.assertEqual( mesh.verticesPerFace, mesh2.verticesPerFace )
			self.assertNotEqual( mesh["P"], mesh2["P"] )

	def testConvertInterpolated( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )

		mesh0 = m.readObjectAtSample( 0 )
		mesh1 = m.readObjectAtSample( 1 )

		mesh = m.readObject( 1.5 / 24.0 )
		self.failUnless( isinstance( mesh, IECoreScene.MeshPrimitive ) )

		self.assertEqual( mesh, IECore.linearObjectInterpolation( mesh0, mesh1, 0.5 ) )

	def testRotatingTransformAtSample( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/rotatingCube.abc", IECore.IndexedIO.OpenMode.Read )

		t = a.child( "pCube1" )
		for i in range( 0, 24 ) :
			ti = t.readTransformAsMatrixAtSample( i )
			mi = imath.M44d().rotate( imath.V3d( IECore.degreesToRadians( 90 * i ), 0, 0 ) )
			self.failUnless( ti.equalWithAbsError( mi, 0.0000000000001 ) )

	def testInterpolatedTranslate( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		t = a.child( "pCube1" )
		self.assertEqual( t.numTransformSamples(), 10 )

		for i in range( 0, t.numTransformSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.readTransformAsMatrix( time )
			expectedMatrix = imath.M44d().translate( imath.V3d( i / 18.0, 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )

	def testInterpolatedRotate( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/rotatingCube.abc", IECore.IndexedIO.OpenMode.Read )
		t = a.child( "pCube1" )
		self.assertEqual( t.numTransformSamples(), 24 )

		for i in range( 0, t.numTransformSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.readTransformAsMatrix( time )
			expectedMatrix = imath.M44d().rotate( imath.V3d( IECore.degreesToRadians( 90 * i * 0.5 ), 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )

	def testHasBound( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( a.hasBound(), True )
		self.assertEqual( a.child( "persp" ).hasBound(), False )
		self.assertEqual( a.child( "pCube1" ).hasBound(), True )
		self.assertEqual( a.child( "front" ).hasBound(), False )
		self.assertEqual( a.child( "front" ).hasBound(), False )

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.child( "group1" ).hasBound(), False )

	def testBoundAtSample( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readBoundAtSample( 0 ), imath.Box3d( imath.V3d( -0.5 ), imath.V3d( 0.5 ) ) )
		self.assertEqual( a.readBoundAtSample( a.numBoundSamples()-1 ), imath.Box3d( imath.V3d( 0.5, -0.5, -0.5 ), imath.V3d( 1.5, 2, 0.5 ) ) )

		t = a.child( "pCube1" )
		self.assertEqual( t.readBoundAtSample( 0 ), imath.Box3d( imath.V3d( -0.5 ), imath.V3d( 0.5 ) ) )
		self.assertEqual( t.readBoundAtSample( t.numBoundSamples()-1 ), imath.Box3d( imath.V3d( -0.5, -0.5, -0.5 ), imath.V3d( 0.5, 2, 0.5 ) ) )

	def testBoundAtTime( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
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
			r.setMin( lerp( a.min(), b.min(), x ) )
			r.setMax( lerp( a.max(), b.max(), x ) )
			return r

		numSteps = 100
		for i in range( 0, numSteps ) :

			lerpFactor = ( float( i ) / (numSteps-1) )
			time = lerp( startTime, endTime, lerpFactor )

			aBound = a.readBound( time )
			expectedABound = lerpBox( aStartBound, aEndBound, lerpFactor )
			self.failUnless( aBound.min().equalWithAbsError( expectedABound.min(), 0.000001 ) )
			self.failUnless( aBound.max().equalWithAbsError( expectedABound.max(), 0.000001 ) )

			mBound = m.readBound( time )
			expectedMBound = lerpBox( mStartBound, mEndBound, lerpFactor )
			self.failUnless( mBound.min().equalWithAbsError( expectedMBound.min(), 0.000001 ) )
			self.failUnless( mBound.max().equalWithAbsError( expectedMBound.max(), 0.000001 ) )

	def testMeshVelocity( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/velocityCube.abc", IECore.IndexedIO.OpenMode.Read )
		c = a.child( "group1" ).child( "pCube1" )
		m = c.readObjectAtSample( 0 )

		self.assertEqual( m["velocity"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )

		self.assertEqual( len( m["velocity"].data ), 8 )
		for i in range( 0, 8 ) :
			self.assertEqual( m["velocity"].data[i], imath.V3f( 1, 0, 0 ) )

	def testConvertNormals( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		mesh = a.child( "pCube1" ).readObjectAtSample( 0 )

		self.failUnless( "N" in mesh )
		self.failUnless( isinstance( mesh["N"].data, IECore.V3fVectorData ) )
		self.assertEqual( mesh["N"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

	def testCamera( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		c = a.child( "persp" ).readObjectAtSample( 0 )
		self.failUnless( isinstance( c, IECoreScene.Camera ) )

		c = a.child( "persp" ).readObject( 0 )
		self.failUnless( isinstance( c, IECoreScene.Camera ) )

	def testLinearCurves( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/curves.abc", IECore.IndexedIO.OpenMode.Read )
		curves = a.child( "linearLine" ).readObjectAtSample( 0 )

		self.assertTrue( isinstance( curves, IECoreScene.CurvesPrimitive ) )
		self.assertEqual( curves.basis(), IECore.CubicBasisf.linear() )
		self.assertEqual( curves.verticesPerCurve(), IECore.IntVectorData( [ 2 ] ) )
		self.assertEqual( curves.periodic(), False )
		self.assertEqual(
			curves["P"].data,
			IECore.V3fVectorData(
				[ imath.V3f( 2, 0, 1 ), imath.V3f( 2, 0, -1 ) ],
				IECore.GeometricData.Interpretation.Point
			)
		)
		self.assertEqual(
			curves["velocity"].data,
			IECore.V3fVectorData(
				[ imath.V3f( 1, 0, 0 ), imath.V3f( 1, 0, 0 ) ],
				IECore.GeometricData.Interpretation.Vector
			)
		)
		self.assertTrue( curves.arePrimitiveVariablesValid() )

	def testCurves( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/curves.abc", IECore.IndexedIO.OpenMode.Read )
		curves = a.child( "curve" ).readObjectAtSample( 0 )

		self.assertTrue( isinstance( curves, IECoreScene.CurvesPrimitive ) )
		self.assertEqual( curves.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curves.verticesPerCurve(), IECore.IntVectorData( [ 4 ] ) )
		self.assertEqual( curves.periodic(), False )
		self.assertTrue( curves.arePrimitiveVariablesValid() )

	def testNURBSCircle( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/curves.abc", IECore.IndexedIO.OpenMode.Read )
		curves = a.child( "nurbsCircle" ).readObjectAtSample( 0 )

		self.assertTrue( isinstance( curves, IECoreScene.CurvesPrimitive ) )
		self.assertEqual( curves.basis(), IECore.CubicBasisf.bSpline() )
		self.assertEqual( curves.verticesPerCurve(), IECore.IntVectorData( [ 11 ] ) )
		self.assertEqual( curves.periodic(), True )
		self.assertTrue( curves.arePrimitiveVariablesValid() )

	def testPoints( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/points.abc", IECore.IndexedIO.OpenMode.Read )
		points = a.child( "particle1" ).readObjectAtSample( 9 )

		self.assertTrue( isinstance( points, IECoreScene.PointsPrimitive ) )
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

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/bool.abc", IECore.IndexedIO.OpenMode.Read )
		p = a.child( "pPlane1" )
		mesh = p.readObjectAtSample( 0 )

		for n in ( "abc_testBoolTrue", "abc_testBoolFalse" ) :
			self.assertIn( n, mesh )
			self.assertEqual( mesh[n].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )

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
		hdf5 = IECoreScene.SceneInterface.create( hdf5File, IECore.IndexedIO.OpenMode.Read )
		hdf5Copy = IECoreScene.SceneInterface.create( hdf5FileCopy, IECore.IndexedIO.OpenMode.Read )
		self.assertNotEqual(
			hdf5.child( "group1" ).child( "pCube1" ).hash( IECoreScene.SceneInterface.HashType.ObjectHash, 0 ),
			hdf5Copy.child( "group1" ).child( "pCube1" ).hash( IECoreScene.SceneInterface.HashType.ObjectHash, 0 ),
		)

		# Ogawa stores hashes, so we can use them to create hashes based purely on the
		# data. This means our hashes can be independent of the file the objects are stored
		# in, or the location they are stored at.
		ogawa = IECoreScene.SceneInterface.create( ogawaFile, IECore.IndexedIO.OpenMode.Read )
		ogawaCopy = IECoreScene.SceneInterface.create( ogawaFileCopy, IECore.IndexedIO.OpenMode.Read )
		self.assertEqual(
			ogawa.child( "particle1" ).hash( IECoreScene.SceneInterface.HashType.ObjectHash, 0 ),
			ogawaCopy.child( "particle1" ).hash( IECoreScene.SceneInterface.HashType.ObjectHash, 0 ),
		)

	def testSharedSceneInterfaces( self ) :

		fileName = os.path.dirname( __file__ ) + "/data/points.abc"
		s = IECoreScene.SharedSceneInterfaces.get( fileName )

		self.assertIsInstance( s, IECoreAlembic.AlembicScene )
		self.assertEqual( s.fileName(), fileName )

	def testWindingOrder( self ) :

		a = IECoreScene.SceneInterface.create( os.path.dirname( __file__ ) + "/data/subdPlane.abc", IECore.IndexedIO.OpenMode.Read )
		c = a.child( "pPlane1" )
		m = c.readObjectAtSample( 0 )

		IECoreScene.MeshNormalsOp()( input = m, copyInput = False )
		for n in m["N"].data :
			self.assertTrue( n.equalWithAbsError( imath.V3f( 0, 1, 0 ), 0.000001 ) )

	def testWriteConstruction( self ) :

		s = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		self.assertEqual( s.fileName(), "/tmp/test.abc" )

		self.assertRaises( RuntimeError, IECoreScene.SceneInterface.create, "/tmp/nonexistentDirectory/test.abc", IECore.IndexedIO.OpenMode.Write )

	def testWriteHierarchy( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a.createChild( "b" )
		c = a.child( "c", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )
		self.assertEqual( a.childNames(), [ "b", "c" ] )
		self.assertEqual( c.path(), [ "c" ] )
		self.assertEqual( c.name(), "c" )

		d = c.createChild( "d" )
		self.assertEqual( a.childNames(), [ "b", "c" ] )
		self.assertEqual( c.childNames(), [ "d" ] )
		self.assertEqual( d.childNames(), [] )

		del a, c, d

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.childNames(), [ "b", "c" ] )
		self.assertEqual( a.child( "b").childNames(), [] )
		self.assertEqual( a.child( "c" ).childNames(), [ "d" ] )
		self.assertEqual( a.child( "c" ).child( "d" ).childNames(), [] )

	def testWriteStaticAttributes( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test_keep.abc", IECore.IndexedIO.OpenMode.Write )
		b = a.createChild( "b" )
		c = b.createChild( "c" )

		a.writeAttribute( "testNoExceptionOnRoot", IECore.FloatData( 10.0 ), 0.0)

		b.writeAttribute( "testFloat", IECore.FloatData( 2.1 ), 0.0 )
		b.writeAttribute( "testInt", IECore.IntData( 3 ), 0.0 )
		b.writeAttribute( "testColor", IECore.Color3fData( imath.Color3f( 1.0, 2.0, 3.0 ) ), 0.0 )
		b.writeAttribute( "testColor4", IECore.Color4fData( imath.Color4f( 1.0, 2.0, 3.0, 4.0 ) ), 0.0 )
		b.writeAttribute( "testString", IECore.StringData( "helloWorld" ), 0.0 )

		del c, b, a

		a = IECoreAlembic.AlembicScene( "/tmp/test_keep.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( a.attributeNames(), [] )

		self.assertTrue( a.hasChild( "b" ) )
		b = a.child( "b" )

		self.assertTrue( b.hasChild( "c" ) )
		c = b.child( "c" )

		self.assertEqual( b.numAttributeSamples( "testFloat" ), 1 )
		self.assertEqual( b.numAttributeSamples( "testInt" ), 1 )
		self.assertEqual( b.numAttributeSamples( "testColor" ), 1 )
		self.assertEqual( b.numAttributeSamples( "testColor4" ), 1 )
		self.assertEqual( b.numAttributeSamples( "testString" ), 1 )
		self.assertEqual( b.numAttributeSamples( "doesntExist" ), 0 )

		self.assertFalse( a.hasAttribute( "DontExist" ) )
		self.assertEqual( a.attributeNames(), [] )

		self.assertTrue( b.hasAttribute( "testFloat" ) )
		self.assertTrue( b.hasAttribute( "testInt" ) )
		self.assertTrue( b.hasAttribute( "testColor" ) )
		self.assertTrue( b.hasAttribute( "testColor4" ) )
		self.assertTrue( b.hasAttribute( "testString" ) )

		self.assertFalse( b.hasAttribute( "DontExist" ) )

		self.assertEqual( b.attributeNames(), ["testFloat", "testInt", "testColor", "testColor4", "testString"] )

		self.assertEqual( b.readAttribute( "testFloat", 0.0 ), IECore.FloatData( 2.1 ) )
		self.assertEqual( b.readAttribute( "testInt", 0.0 ), IECore.IntData( 3 ) )
		self.assertEqual( b.readAttribute( "testColor", 0.0 ), IECore.Color3fData( imath.Color3f( 1.0, 2.0, 3.0 ) ) )
		self.assertEqual( b.readAttribute( "testColor4", 0.0 ), IECore.Color4fData( imath.Color4f( 1.0, 2.0, 3.0, 4.0 ) ) )
		self.assertEqual( b.readAttribute( "testString", 0.0 ), IECore.StringData( "helloWorld" ) )

		self.assertFalse( c.hasAttribute( "DontExist" ) )
		self.assertEqual( c.attributeNames(), [] )

	def testWriteAnimatedAttributes( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test_animated.abc", IECore.IndexedIO.OpenMode.Write )
		b = a.createChild( "b" )
		c = a.createChild( "c" )
		d = a.createChild( "d" )
		e = a.createChild( "e" )
		f = a.createChild( "f" )
		g = a.createChild( "g" )
		h = a.createChild( "h" )
		i = a.createChild( "i" )
		j = a.createChild( "j" )
		k = a.createChild( "k" )
		l = a.createChild( "l" )
		m = a.createChild( "m" )

		b.writeAttribute( "testBool", IECore.BoolData( False ), 0.5 )
		b.writeAttribute( "testBool", IECore.BoolData( True ), 1.0 )
		b.writeAttribute( "testBool", IECore.BoolData( False ), 2.0 )

		b.writeAttribute( "testU8", IECore.UCharData( 1 ), 0.0 )
		b.writeAttribute( "testU8", IECore.UCharData( 2 ), 0.1 )
		b.writeAttribute( "testU8", IECore.UCharData( 3 ), 0.2 )
		b.writeAttribute( "testU8", IECore.UCharData( 4 ), 0.3 )
		b.writeAttribute( "testU8", IECore.UCharData( 5 ), 0.4 )

		b.writeAttribute( "testS8", IECore.CharData( '1' ), 0.3 )
		b.writeAttribute( "testS8", IECore.CharData( '2' ), 0.6 )

		b.writeAttribute( "testU16", IECore.UShortData( 100 ), 10.0 )
		b.writeAttribute( "testU16", IECore.UShortData( 101 ), 11.0 )

		b.writeAttribute( "testS16", IECore.ShortData( 102 ), 12.0 )
		b.writeAttribute( "testS16", IECore.ShortData( 103 ), 13.0 )

		b.writeAttribute( "testU32", IECore.UIntData( 456789 ), 5.0 )
		b.writeAttribute( "testU32", IECore.UIntData( 987654 ), 6.0 )

		b.writeAttribute( "testS32", IECore.IntData( 123456 ), 0.0 )
		b.writeAttribute( "testS32", IECore.IntData( 654321 ), 10.0 )

		b.writeAttribute( "testU64", IECore.UInt64Data( 123456789 ), 5.0 )
		b.writeAttribute( "testU64", IECore.UInt64Data( 987654321 ), 6.0 )

		b.writeAttribute( "testS64", IECore.Int64Data( -123456789 ), 0.0 )
		b.writeAttribute( "testS64", IECore.Int64Data( -987654321 ), 10.0 )

		c.writeAttribute( "testF16", IECore.HalfData( 1.0 ), 1.0 )
		c.writeAttribute( "testF16", IECore.HalfData( 2.0 ), 2.0 )

		c.writeAttribute( "testF32", IECore.FloatData( 1.0 ), 0.0 )
		c.writeAttribute( "testF32", IECore.FloatData( 0.0 ), 2.0 )

		c.writeAttribute( "testF64", IECore.DoubleData( 12.0 ), 3.0 )
		c.writeAttribute( "testF64", IECore.DoubleData( 13.0 ), 4.0 )

		c.writeAttribute( "testString", IECore.StringData( "foo" ), 1.0 )
		c.writeAttribute( "testString", IECore.StringData( "bar" ), 5.0 )

		d.writeAttribute( "testV2i", IECore.V2iData( imath.V2i( 1, 2 ), IECore.GeometricData.Interpretation.Vector ), 0.0 )
		d.writeAttribute( "testV2f", IECore.V2fData( imath.V2f( 0.0, 1.0 ), IECore.GeometricData.Interpretation.Vector ), 0.0 )
		d.writeAttribute( "testV2d", IECore.V2dData( imath.V2d( 1.0, 2.0 ), IECore.GeometricData.Interpretation.Vector ), 0.0 )

		e.writeAttribute( "testV3i", IECore.V3iData( imath.V3i( 1, 2, 3 ), IECore.GeometricData.Interpretation.Vector ), 0.0 )
		e.writeAttribute( "testV3f", IECore.V3fData( imath.V3f( 0.0, 1.0, 2.0 ), IECore.GeometricData.Interpretation.Vector ), 0.0 )
		e.writeAttribute( "testV3d", IECore.V3dData( imath.V3d( 1.0, 2.0, 3.0 ), IECore.GeometricData.Interpretation.Vector ), 0.0 )

		f.writeAttribute( "testP2i", IECore.V2iData( imath.V2i( 1, 2 ), IECore.GeometricData.Interpretation.Point ), 0.0 )
		f.writeAttribute( "testP2f", IECore.V2fData( imath.V2f( 0.0, 1.0 ), IECore.GeometricData.Interpretation.Point ), 0.0 )
		f.writeAttribute( "testP2d", IECore.V2dData( imath.V2d( 1.0, 2.0 ), IECore.GeometricData.Interpretation.Point ), 0.0 )

		g.writeAttribute( "testP3i", IECore.V3iData( imath.V3i( 1, 2, 3 ), IECore.GeometricData.Interpretation.Point ), 0.0 )
		g.writeAttribute( "testP3f", IECore.V3fData( imath.V3f( 0.0, 1.0, 2.0 ), IECore.GeometricData.Interpretation.Point ), 0.0 )
		g.writeAttribute( "testP3d", IECore.V3dData( imath.V3d( 1.0, 2.0, 3.0 ), IECore.GeometricData.Interpretation.Point ), 0.0 )

		h.writeAttribute( "box2i", IECore.Box2iData( imath.Box2i( imath.V2i( 0, 1 ), imath.V2i( 2, 3 ) ) ), 0.0 )
		h.writeAttribute( "box2f", IECore.Box2fData( imath.Box2f( imath.V2f( 0, 1 ), imath.V2f( 2, 3 ) ) ), 0.0 )
		h.writeAttribute( "box2d", IECore.Box2dData( imath.Box2d( imath.V2d( 0, 1 ), imath.V2d( 2, 3 ) ) ), 0.0 )

		i.writeAttribute( "box3i", IECore.Box3iData( imath.Box3i( imath.V3i( 0, 1, 2 ), imath.V3i( 3, 4, 5 ) ) ), 0.0 )
		i.writeAttribute( "box3f", IECore.Box3fData( imath.Box3f( imath.V3f( 0, 1, 2 ), imath.V3f( 3, 4, 5 ) ) ), 0.0 )
		i.writeAttribute( "box3d", IECore.Box3dData( imath.Box3d( imath.V3d( 0, 1, 2 ), imath.V3d( 3, 4, 5 ) ) ), 0.0 )

		j.writeAttribute( "m33f", IECore.M33fData( imath.M33f( *range( 9 ) ) ), 0.0 )
		j.writeAttribute( "m33d", IECore.M33dData( imath.M33d( *range( 9 ) ) ), 0.0 )
		j.writeAttribute( "m44f", IECore.M44fData( imath.M44f( *range( 16 ) ) ), 0.0 )
		j.writeAttribute( "m44d", IECore.M44dData( imath.M44d( *range( 16 ) ) ), 0.0 )

		k.writeAttribute( "quatf", IECore.QuatfData( imath.Quatf( 4, imath.V3f( 1, 2, 3 ) ) ), 0.0 )
		k.writeAttribute( "quatd", IECore.QuatdData( imath.Quatd( 4, imath.V3d( 1, 2, 3 ) ) ), 0.0 )

		l.writeAttribute( "c3f", IECore.Color3fData( imath.Color3f( 0, 1, 2 ) ), 0.0 )
		l.writeAttribute( "c4f", IECore.Color4fData( imath.Color4f( 0, 1, 2, 3 ) ), 0.0 )

		m.writeAttribute( "n2f", IECore.V2fData( imath.V2f( 0, 1 ), IECore.GeometricData.Interpretation.Normal ), 0.0 )
		m.writeAttribute( "n2d", IECore.V2dData( imath.V2d( 1, 0 ), IECore.GeometricData.Interpretation.Normal ), 0.0 )

		m.writeAttribute( "n3f", IECore.V3fData( imath.V3f( 0, 1, 0 ), IECore.GeometricData.Interpretation.Normal ), 0.0 )
		m.writeAttribute( "n3d", IECore.V3dData( imath.V3d( 1, 0, 0 ), IECore.GeometricData.Interpretation.Normal ), 0.0 )

		del m, l, k, j, i, h, g, f, e, d, c, b, a

		a = IECoreAlembic.AlembicScene( "/tmp/test_animated.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertTrue( a.hasChild( "b" ) )
		b = a.child( "b" )

		self.assertTrue( a.hasChild( "c" ) )
		c = a.child( "c" )

		self.assertTrue( a.hasChild( "d" ) )
		d = a.child( "d" )

		self.assertTrue( a.hasChild( "e" ) )
		e = a.child( "e" )

		self.assertTrue( a.hasChild( "f" ) )
		f = a.child( "f" )

		self.assertTrue( a.hasChild( "g" ) )
		g = a.child( "g" )

		self.assertTrue( a.hasChild( "h" ) )
		h = a.child( "h" )

		self.assertTrue( a.hasChild( "i" ) )
		i = a.child( "i" )

		self.assertTrue( a.hasChild( "j" ) )
		j = a.child( "j" )

		self.assertTrue( a.hasChild( "k" ) )
		k = a.child( "k" )

		self.assertTrue( a.hasChild( "l" ) )
		l = a.child( "l" )

		self.assertTrue( a.hasChild( "m" ) )
		m = a.child( "m" )

		self.assertTrue( b.hasAttribute( "testBool" ) )
		self.assertTrue( b.hasAttribute( "testU8" ) )
		self.assertTrue( b.hasAttribute( "testS8" ) )
		self.assertTrue( b.hasAttribute( "testU16" ) )
		self.assertTrue( b.hasAttribute( "testS16" ) )
		self.assertTrue( b.hasAttribute( "testU32" ) )
		self.assertTrue( b.hasAttribute( "testS32" ) )

		self.assertFalse( b.hasAttribute( "DontExist" ) )

		self.assertEqual( b.attributeNames(), ["testBool", "testU8", "testS8", "testU16", "testS16", "testU32", "testS32", "testU64", "testS64"] )

		self.assertEqual( b.numAttributeSamples( "testBool" ), 3 )

		self.assertEqual( b.attributeSampleTime( "testBool", 0 ), 0.5 )
		self.assertEqual( b.attributeSampleTime( "testBool", 1 ), 1.0 )
		self.assertEqual( b.attributeSampleTime( "testBool", 2 ), 2.0 )

		self.assertEqual( b.attributeSampleInterval( "testBool", 0.5 ), (0.0, 0, 0,) )
		self.assertEqual( b.attributeSampleInterval( "testBool", 1.5 ), (0.5, 1, 2,) )
		self.assertEqual( b.attributeSampleInterval( "testBool", 2.75 ), (0.0, 2, 2,) )

		self.assertEqual( b.numAttributeSamples( "testU8" ), 5 )

		self.assertEqual( b.attributeSampleTime( "testU8", 0 ), 0.0 )
		self.assertEqual( b.attributeSampleTime( "testU8", 1 ), 0.1 )
		self.assertEqual( b.attributeSampleTime( "testU8", 2 ), 0.2 )
		self.assertEqual( b.attributeSampleTime( "testU8", 3 ), 0.3 )
		self.assertEqual( b.attributeSampleTime( "testU8", 4 ), 0.4 )

		self.assertEqual( b.readAttribute( "testBool", 0.5 ), IECore.BoolData( False ) )
		self.assertEqual( b.readAttribute( "testBool", 1.0 ), IECore.BoolData( True ) )
		self.assertEqual( b.readAttribute( "testBool", 2.0 ), IECore.BoolData( False ) )

		self.assertEqual( b.readAttribute( "testU8", 0.0 ), IECore.UCharData( 1 ) )
		self.assertEqual( b.readAttribute( "testU8", 0.1 ), IECore.UCharData( 2 ) )
		self.assertEqual( b.readAttribute( "testU8", 0.2 ), IECore.UCharData( 3 ) )
		self.assertEqual( b.readAttribute( "testU8", 0.3 ), IECore.UCharData( 4 ) )
		self.assertEqual( b.readAttribute( "testU8", 0.4 ), IECore.UCharData( 5 ) )

		self.assertEqual( b.readAttribute( "testS8", 0.3 ), IECore.CharData( '1' ) )
		self.assertEqual( b.readAttribute( "testS8", 0.6 ), IECore.CharData( '2' ) )

		self.assertEqual( b.readAttribute( "testU16", 10.0 ), IECore.UShortData( 100 ) )
		self.assertEqual( b.readAttribute( "testU16", 11.0 ), IECore.UShortData( 101 ) )

		self.assertEqual( b.readAttribute( "testS16", 12.0 ), IECore.ShortData( 102 ) )
		self.assertEqual( b.readAttribute( "testS16", 13.0 ), IECore.ShortData( 103 ) )

		self.assertEqual( b.readAttribute( "testU32", 5.0 ), IECore.UIntData( 456789 ) )
		self.assertEqual( b.readAttribute( "testU32", 6.0 ), IECore.UIntData( 987654 ) )

		self.assertEqual( b.readAttribute( "testS32", 0.0 ), IECore.IntData( 123456 ) )
		self.assertEqual( b.readAttribute( "testS32", 10.0 ), IECore.IntData( 654321 ) )

		self.assertEqual( b.readAttribute( "testU64", 5.0 ), IECore.UInt64Data( 123456789 ) )
		self.assertEqual( b.readAttribute( "testU64", 6.0 ), IECore.UInt64Data( 987654321 ) )

		self.assertEqual( b.readAttribute( "testS64", 0.0 ), IECore.Int64Data( -123456789 ) )
		self.assertEqual( b.readAttribute( "testS64", 10.0 ), IECore.Int64Data( -987654321 ) )

		self.assertTrue( c.hasAttribute( "testF16" ) )
		self.assertTrue( c.hasAttribute( "testF32" ) )
		self.assertTrue( c.hasAttribute( "testF64" ) )
		self.assertTrue( c.hasAttribute( "testString" ) )

		self.assertEqual( c.attributeNames(), ["testF16", "testF32", "testF64", "testString"] )

		self.assertEqual( c.readAttribute( "testF16", 1.0 ), IECore.HalfData( 1.0 ) )
		self.assertEqual( c.readAttribute( "testF16", 2.0 ), IECore.HalfData( 2.0 ) )

		self.assertEqual( c.readAttribute( "testF32", 0.0 ), IECore.FloatData( 1.0 ) )
		self.assertEqual( c.readAttribute( "testF32", 1.0 ), IECore.FloatData( 0.5 ) )
		self.assertEqual( c.readAttribute( "testF32", 2.0 ), IECore.FloatData( 0.0 ) )

		self.assertEqual( c.readAttribute( "testF64", 3.0 ), IECore.DoubleData( 12.0 ) )
		self.assertEqual( c.readAttribute( "testF64", 4.0 ), IECore.DoubleData( 13.0 ) )

		self.assertEqual( c.readAttribute( "testString", 0.0 ), IECore.StringData( "foo" ) )
		self.assertEqual( c.readAttribute( "testString", 1.0 ), IECore.StringData( "foo" ) )
		self.assertEqual( c.readAttribute( "testString", 2.0 ), IECore.StringData( "foo" ) )
		self.assertEqual( c.readAttribute( "testString", 3.0 ), IECore.StringData( "bar" ) )
		self.assertEqual( c.readAttribute( "testString", 5.0 ), IECore.StringData( "bar" ) )
		self.assertEqual( c.readAttribute( "testString", 5.1 ), IECore.StringData( "bar" ) )

		self.assertTrue( d.hasAttribute( "testV2i" ) )
		self.assertTrue( d.hasAttribute( "testV2f" ) )
		self.assertTrue( d.hasAttribute( "testV2d" ) )
		self.assertEqual( d.attributeNames(), ["testV2i", "testV2f", "testV2d"] )

		self.assertEqual( d.readAttribute( "testV2i", 0.0 ), IECore.V2iData( imath.V2i( 1, 2 ), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( d.readAttribute( "testV2f", 0.0 ), IECore.V2fData( imath.V2f( 0.0, 1.0 ), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( d.readAttribute( "testV2d", 0.0 ), IECore.V2dData( imath.V2d( 1.0, 2.0 ), IECore.GeometricData.Interpretation.Vector ) )

		self.assertTrue( e.hasAttribute( "testV3i" ) )
		self.assertTrue( e.hasAttribute( "testV3f" ) )
		self.assertTrue( e.hasAttribute( "testV3d" ) )
		self.assertEqual( e.attributeNames(), ["testV3i", "testV3f", "testV3d"] )

		self.assertEqual( e.readAttribute( "testV3i", 0.0 ), IECore.V3iData( imath.V3i( 1, 2, 3 ), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( e.readAttribute( "testV3f", 0.0 ), IECore.V3fData( imath.V3f( 0.0, 1.0, 2.0 ), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( e.readAttribute( "testV3d", 0.0 ), IECore.V3dData( imath.V3d( 1.0, 2.0, 3.0 ), IECore.GeometricData.Interpretation.Vector ) )

		self.assertTrue( f.hasAttribute( "testP2i" ) )
		self.assertTrue( f.hasAttribute( "testP2f" ) )
		self.assertTrue( f.hasAttribute( "testP2d" ) )
		self.assertEqual( f.attributeNames(), ["testP2i", "testP2f", "testP2d"] )

		self.assertEqual( f.readAttribute( "testP2i", 0.0 ), IECore.V2iData( imath.V2i( 1, 2 ), IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( f.readAttribute( "testP2f", 0.0 ), IECore.V2fData( imath.V2f( 0.0, 1.0 ), IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( f.readAttribute( "testP2d", 0.0 ), IECore.V2dData( imath.V2d( 1.0, 2.0 ), IECore.GeometricData.Interpretation.Point ) )

		self.assertTrue( g.hasAttribute( "testP3i" ) )
		self.assertTrue( g.hasAttribute( "testP3f" ) )
		self.assertTrue( g.hasAttribute( "testP3d" ) )
		self.assertEqual( g.attributeNames(), ["testP3i", "testP3f", "testP3d"] )

		self.assertEqual( g.readAttribute( "testP3i", 0.0 ), IECore.V3iData( imath.V3i( 1, 2, 3 ), IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( g.readAttribute( "testP3f", 0.0 ), IECore.V3fData( imath.V3f( 0.0, 1.0, 2.0 ), IECore.GeometricData.Interpretation.Point ) )
		self.assertEqual( g.readAttribute( "testP3d", 0.0 ), IECore.V3dData( imath.V3d( 1.0, 2.0, 3.0 ), IECore.GeometricData.Interpretation.Point ) )

		self.assertTrue( h.hasAttribute( "box2i" ) )
		self.assertTrue( h.hasAttribute( "box2f" ) )
		self.assertTrue( h.hasAttribute( "box2d" ) )
		self.assertEqual( h.attributeNames(), ["box2i", "box2f", "box2d"] )

		self.assertEqual( h.readAttribute( "box2i", 0.0 ), IECore.Box2iData( imath.Box2i( imath.V2i( 0, 1 ), imath.V2i( 2, 3 ) ) ) )
		self.assertEqual( h.readAttribute( "box2f", 0.0 ), IECore.Box2fData( imath.Box2f( imath.V2f( 0, 1 ), imath.V2f( 2, 3 ) ) ) )
		self.assertEqual( h.readAttribute( "box2d", 0.0 ), IECore.Box2dData( imath.Box2d( imath.V2d( 0, 1 ), imath.V2d( 2, 3 ) ) ) )

		self.assertTrue( i.hasAttribute( "box3i" ) )
		self.assertTrue( i.hasAttribute( "box3f" ) )
		self.assertTrue( i.hasAttribute( "box3d" ) )
		self.assertEqual( i.attributeNames(), ["box3i", "box3f", "box3d"] )

		self.assertEqual( i.readAttribute( "box3i", 0.0 ), IECore.Box3iData( imath.Box3i( imath.V3i( 0, 1, 2 ), imath.V3i( 3, 4, 5 ) ) ) )
		self.assertEqual( i.readAttribute( "box3f", 0.0 ), IECore.Box3fData( imath.Box3f( imath.V3f( 0, 1, 2 ), imath.V3f( 3, 4, 5 ) ) ) )
		self.assertEqual( i.readAttribute( "box3d", 0.0 ), IECore.Box3dData( imath.Box3d( imath.V3d( 0, 1, 2 ), imath.V3d( 3, 4, 5 ) ) ) )

		self.assertTrue( j.hasAttribute( "m33f" ) )
		self.assertTrue( j.hasAttribute( "m33d" ) )
		self.assertTrue( j.hasAttribute( "m44f" ) )
		self.assertTrue( j.hasAttribute( "m44d" ) )
		self.assertEqual( j.attributeNames(), ["m33f", "m33d", "m44f", "m44d"] )

		self.assertEqual( j.readAttribute( "m33f", 0.0 ), IECore.M33fData( imath.M33f( *range( 9 ) ) ) )
		self.assertEqual( j.readAttribute( "m33d", 0.0 ), IECore.M33dData( imath.M33d( *range( 9 ) ) ) )
		self.assertEqual( j.readAttribute( "m44f", 0.0 ), IECore.M44fData( imath.M44f( *range( 16 ) ) ) )
		self.assertEqual( j.readAttribute( "m44d", 0.0 ), IECore.M44dData( imath.M44d( *range( 16 ) ) ) )

		self.assertTrue( k.hasAttribute( "quatf" ) )
		self.assertTrue( k.hasAttribute( "quatd" ) )
		self.assertEqual( k.attributeNames(), ["quatf", "quatd"] )

		self.assertEqual( k.readAttribute( "quatf", 0.0 ), IECore.QuatfData( imath.Quatf( 4, imath.V3f( 1, 2, 3 ) ) ) )
		self.assertEqual( k.readAttribute( "quatd", 0.0 ), IECore.QuatdData( imath.Quatd( 4, imath.V3d( 1, 2, 3 ) ) ) )

		self.assertTrue( l.hasAttribute( "c3f" ) )
		self.assertTrue( l.hasAttribute( "c4f" ) )
		self.assertEqual( l.attributeNames(), ["c3f", "c4f"] )

		self.assertEqual( l.readAttribute( "c3f", 0.0 ), IECore.Color3fData( imath.Color3f( 0, 1, 2 ) ) )
		self.assertEqual( l.readAttribute( "c4f", 0.0 ), IECore.Color4fData( imath.Color4f( 0, 1, 2, 3 ) ) )

		self.assertTrue( m.hasAttribute( "n2f" ) )
		self.assertTrue( m.hasAttribute( "n2d" ) )
		self.assertTrue( m.hasAttribute( "n3f" ) )
		self.assertTrue( m.hasAttribute( "n3d" ) )
		self.assertEqual( m.attributeNames(), ["n2f", "n2d", "n3f", "n3d"] )

		self.assertEqual( m.readAttribute( "n2f", 0.0 ), IECore.V2fData( imath.V2f( 0, 1 ), IECore.GeometricData.Interpretation.Normal ) )
		self.assertEqual( m.readAttribute( "n2d", 0.0 ), IECore.V2dData( imath.V2d( 1, 0 ), IECore.GeometricData.Interpretation.Normal ) )
		self.assertEqual( m.readAttribute( "n3f", 0.0 ), IECore.V3fData( imath.V3f( 0, 1, 0 ), IECore.GeometricData.Interpretation.Normal ) )
		self.assertEqual( m.readAttribute( "n3d", 0.0 ), IECore.V3dData( imath.V3d( 1, 0, 0 ), IECore.GeometricData.Interpretation.Normal ) )

	def testCanWriteVectorWithNoInterpretation( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		b = a.createChild( "b" )

		b.writeAttribute( "noInterpretationV2i", IECore.V2iData( imath.V2i( 5, 6), IECore.GeometricData.Interpretation.None ), 0.0 )
		b.writeAttribute( "noInterpretationV2f", IECore.V2fData( imath.V2f( 2.0, 3.0), IECore.GeometricData.Interpretation.None ), 0.0 )
		b.writeAttribute( "noInterpretationV2d", IECore.V2dData( imath.V2d( 4.0, 5.0), IECore.GeometricData.Interpretation.None ), 0.0 )

		b.writeAttribute( "noInterpretationV3i", IECore.V3iData( imath.V3i( -1, -2, -3 ), IECore.GeometricData.Interpretation.None ), 0.0 )
		b.writeAttribute( "noInterpretationV3f", IECore.V3fData( imath.V3f( 1.0, 2.0, 3.0 ), IECore.GeometricData.Interpretation.None ), 0.0 )
		b.writeAttribute( "noInterpretationV3d", IECore.V3dData( imath.V3d( 7.0, 8.0, 9.0 ), IECore.GeometricData.Interpretation.None ), 0.0 )

		del b, a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertTrue( a.hasChild( "b" ) )
		b = a.child( "b" )

		self.assertTrue( b.hasAttribute( "noInterpretationV2i" ) )
		self.assertTrue( b.hasAttribute( "noInterpretationV2f" ) )
		self.assertTrue( b.hasAttribute( "noInterpretationV2d" ) )

		self.assertTrue( b.hasAttribute( "noInterpretationV3i" ) )
		self.assertTrue( b.hasAttribute( "noInterpretationV3f" ) )
		self.assertTrue( b.hasAttribute( "noInterpretationV3d" ) )


		self.assertEqual( b.readAttribute( "noInterpretationV2i", 0.0), IECore.V2iData( imath.V2i( 5, 6), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( b.readAttribute( "noInterpretationV2f", 0.0), IECore.V2fData( imath.V2f( 2.0, 3.0), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( b.readAttribute( "noInterpretationV2d", 0.0), IECore.V2dData( imath.V2d( 4.0, 5.0), IECore.GeometricData.Interpretation.Vector ) )

		self.assertEqual( b.readAttribute( "noInterpretationV3i", 0.0), IECore.V3iData( imath.V3i( -1, -2, -3), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( b.readAttribute( "noInterpretationV3f", 0.0), IECore.V3fData( imath.V3f( 1.0, 2.0, 3.0 ), IECore.GeometricData.Interpretation.Vector ) )
		self.assertEqual( b.readAttribute( "noInterpretationV3d", 0.0), IECore.V3dData( imath.V3d( 7.0, 8.0, 9.0 ), IECore.GeometricData.Interpretation.Vector ) )

	def testWriteStaticTransformUsingM44d( self ) :

		matrix = imath.M44d().translate( imath.V3d( 1 ) )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		self.assertRaises( RuntimeError, a.writeTransform, IECore.M44dData( matrix ), 0 )

		b = a.createChild( "b" )
		b.writeTransform( IECore.M44dData( matrix ), 0 )
		del a, b

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.numTransformSamples(), 0 )
		self.assertEqual( a.readTransformAsMatrix( 0 ), imath.M44d() )

		b = a.child( "b" )
		self.assertEqual( b.numTransformSamples(), 1 )
		self.assertEqual( b.readTransformAsMatrixAtSample( 0 ), matrix )
		self.assertEqual( b.readTransformAsMatrix( 0 ), matrix )
		self.assertEqual( b.readTransformAsMatrix( 0 ), matrix )

	def testWriteStaticTransformUsingTransformationMatrixd( self ) :

		matrix = IECore.TransformationMatrixd()
		matrix.translate = imath.V3d(1.0, 2.0, 3.0)

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		self.assertRaises( RuntimeError, a.writeTransform, IECore.TransformationMatrixdData( matrix ), 0 )

		b = a.createChild( "b" )
		b.writeTransform( IECore.TransformationMatrixdData( matrix ), 0 )
		del a, b

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.numTransformSamples(), 0 )
		self.assertEqual( a.readTransformAsMatrix( 0 ), imath.M44d() )

		b = a.child( "b" )
		self.assertEqual( b.numTransformSamples(), 1 )
		self.assertEqual( b.readTransformAsMatrixAtSample( 0 ), matrix.transform )
		self.assertEqual( b.readTransformAsMatrix( 0 ), matrix.transform )
		self.assertEqual( b.readTransformAsMatrix( 0 ), matrix.transform )

	def testWriteAnimatedTransform( self ) :

		matrix1 = imath.M44d().translate( imath.V3d( 1 ) )
		matrix2 = imath.M44d().translate( imath.V3d( 2 ) )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )

		b = a.createChild( "b" )
		b.writeTransform( IECore.M44dData( matrix1 ), 0 )
		b.writeTransform( IECore.M44dData( matrix2 ), 1 )
		del a, b

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		b = a.child( "b" )
		self.assertEqual( b.numTransformSamples(), 2 )
		self.assertEqual( b.readTransformAsMatrixAtSample( 0 ), matrix1 )
		self.assertEqual( b.readTransformAsMatrixAtSample( 1 ), matrix2 )
		self.assertEqual( b.readTransformAsMatrix( 0 ), matrix1 )
		self.assertEqual( b.readTransformAsMatrix( 1 ), matrix2 )
		self.assertEqual( b.readTransformAsMatrix( 0.5 ), imath.M44d().translate( imath.V3d( 1.5 ) ) )

	def testWriteStaticBounds( self ) :

		aBound = imath.Box3d( imath.V3d( -2 ), imath.V3d( 2 ) )
		bBound = imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a.writeBound( aBound, 0 )

		b = a.createChild( "b" )
		b.writeBound( bBound, 0 )

		del a, b

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.numBoundSamples(), 1 )
		self.assertEqual( a.readBoundAtSample( 0 ), aBound )
		self.assertEqual( a.readBound( 0 ), aBound )

		b = a.child( "b" )
		self.assertEqual( b.numBoundSamples(), 1 )
		self.assertEqual( b.readBoundAtSample( 0 ), bBound )
		self.assertEqual( b.readBound( 0 ), bBound )

	def testWriteAnimatedBounds( self ) :

		aBound1 = imath.Box3d( imath.V3d( -2 ), imath.V3d( 2 ) )
		aBound2 = imath.Box3d( imath.V3d( 0 ), imath.V3d( 4 ) )

		bBound1 = imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) )
		bBound2 = imath.Box3d( imath.V3d( 1 ), imath.V3d( 3 ) )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a.writeBound( aBound1, 0 )
		a.writeBound( aBound2, 1 )

		b = a.createChild( "b" )
		b.writeBound( bBound1, 0 )
		b.writeBound( bBound2, 1 )

		del a, b

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.numBoundSamples(), 2 )
		self.assertEqual( a.readBoundAtSample( 0 ), aBound1 )
		self.assertEqual( a.readBoundAtSample( 1 ), aBound2 )
		self.assertEqual( a.readBound( 0 ), aBound1 )
		self.assertEqual( a.readBound( 1 ), aBound2 )

		b = a.child( "b" )
		self.assertEqual( b.numBoundSamples(), 2 )
		self.assertEqual( b.readBoundAtSample( 0 ), bBound1 )
		self.assertEqual( b.readBoundAtSample( 1 ), bBound2 )
		self.assertEqual( b.readBound( 0 ), bBound1 )
		self.assertEqual( b.readBound( 1 ), bBound2 )

	def __testWriteObject( self, sourceFile, sourcePath ) :

		a1 = IECoreAlembic.AlembicScene( sourceFile, IECore.IndexedIO.OpenMode.Read )
		o1 = a1.scene( sourcePath ).readObject( 0 )

		a2 = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a2.createChild( "o" ).writeObject( o1, 0 )
		del a2

		a3 = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		o2 = a3.child( "o" ).readObject( 0 )

		self.assertEqual( o2, o1 )

	def testWriteMesh( self ) :

		self.__testWriteObject( os.path.dirname( __file__ ) + "/data/velocityCube.abc", [ "group1", "pCube1" ] )

	def testWriteMeshUVsAndArbGeomParams( self ) :

		self.__testWriteObject( os.path.dirname( __file__ ) + "/data/coloredMesh.abc", [ "pPlane1" ] )

	def testWriteCurves( self ) :

		self.__testWriteObject( os.path.dirname( __file__ ) + "/data/curves.abc", [ "curve" ] )

	def testWriteAnimatedObject( self ) :

		o1 = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 0 ) ] ) )
		o1["id"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.UInt64VectorData( [ 0 ] ) )
		o1["test"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [ 1 ] ) )

		o2 = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 1 ) ] ) )
		o2["id"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.UInt64VectorData( [ 0 ] ) )
		o2["test"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [ 2 ] ) )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		c = a.createChild( "o" )
		c.writeObject( o1, 0 )
		c.writeObject( o2, 1 )
		del a, c

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		c = a.child( "o" )

		self.assertEqual( c.numObjectSamples(), 2 )

		o1r = c.readObjectAtSample( 0 )

		self.assertEqual( c.readObjectAtSample( 0 ), o1 )
		self.assertEqual( c.readObjectAtSample( 1 ), o2 )

	def testWritePointsWithoutIDs( self ):

		# IDs a required by alembic and are  generated in the writer if they're not present
		# So we should expect them to be in the deserialized alembic file.
		numParticles = 1024 * 1024 * 16
		o1 = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 0 ) ] * numParticles ) )

		o1["a"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 0 ] * numParticles ) )
		o1["b"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * numParticles ) )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		c = a.createChild( "o" )
		c.writeObject( o1, 0 )
		del a, c

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		c = a.child( "o" )

		self.assertEqual( c.numObjectSamples(), 1 )

		ro1 = c.readObjectAtSample( 0 )
		self.assertTrue ("id" in ro1)
		self.assertEqual( ro1["id"].data, IECore.UInt64VectorData( range( numParticles ) ) )

		del ro1["id"]
		self.assertEqual( ro1, o1 )

	def testWriteGeometricTypedData( self ) :

		o = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 0 ) ] ) )
		o["id"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.UInt64VectorData( [ 0 ] ) )
		o["v3f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 1 ) ], IECore.GeometricData.Interpretation.Vector ) )
		o["n3f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 1 ) ], IECore.GeometricData.Interpretation.Normal ) )
		o["p3f"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 1 ) ], IECore.GeometricData.Interpretation.Point ) )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		c = a.createChild( "o" )
		c.writeObject( o, 0 )
		del a, c

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		c = a.child( "o" )
		self.assertEqual( c.readObjectAtSample( 0 ), o )

	def testReacquireChildDuringWriting( self ) :

		plane0 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		plane1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -2 ), imath.V2f( 2 ) ) )

		def writeHierarchy( a, plane, time ) :

			c = a.child( "c", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )
			d = c.child( "d", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

			d.writeObject( plane, time )

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )

		writeHierarchy( a, plane0, 0 )
		writeHierarchy( a, plane1, 1 )

		del a
		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		d = a.child( "c" ).child( "d" )
		self.assertEqual( d.numObjectSamples(), 2 )
		self.assertEqual( d.readObjectAtSample( 0 ), plane0 )
		self.assertEqual( d.readObjectAtSample( 1 ), plane1 )

	def testCanWeRoundTripIndexedPrimvars( self ) :

		plane = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		data = IECore.FloatVectorData( [1] )
		indices = IECore.IntVectorData( [0, 0, 0, 0] )

		primVar = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, data, indices )

		plane["test"] = primVar

		outputFilename = "/tmp/test.abc"

		root = IECoreAlembic.AlembicScene( outputFilename, IECore.IndexedIO.OpenMode.Write )

		location = root.child( "object", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

		location.writeObject( plane, 0 )

		del location
		del root

		root = IECoreAlembic.AlembicScene( outputFilename, IECore.IndexedIO.OpenMode.Read )

		c = root.child( "object" )

		object = c.readObjectAtSample( 0 )

		self.assertEqual( object["test"].data, IECore.FloatVectorData( [1] ) )
		self.assertEqual( object["test"].indices, IECore.IntVectorData( [0, 0, 0, 0] ) )

	def testSets( self ):
		# Based on IECoreScene/SceneCacheTest.py & IECoreUSD/USDSceneWriterTest.py
		# A
		#   B { 'don': ['/E'], 'john'; ['/F'] }
		#      E
		#      F
		#   C { 'don' : ['/O'] }
		#      O
		#   D { 'john' : ['/G] }
		#      G {'matti' : ['/'] }  this will not get written - added here so we ensure the other set information is writen inspite of
		# H
		#    I
		#       J
		#          K {'foo',['/L/M/N'] }
		#             L
		#                M
		#                   N

		writeRoot = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )

		A = writeRoot.createChild("A")
		B = A.createChild("B")
		C = A.createChild("C")
		D = A.createChild("D")
		E = B.createChild("E")
		F = B.createChild("F")
		G = D.createChild("G")

		H = writeRoot.createChild("H")
		I = H.createChild("I")
		J = I.createChild("J")
		K = J.createChild("K")
		L = K.createChild("L")
		M = L.createChild("M")
		N = M.createChild("N")

		O = C.createChild("O")

		B.writeSet( "don", IECore.PathMatcher( ['/E'] ) )
		B.writeSet( "john", IECore.PathMatcher( ['/F'] ) )
		C.writeSet( "don", IECore.PathMatcher( ['/O'] ) )
		D.writeSet( "john", IECore.PathMatcher( ['/G'] ) )
		K.writeSet( "foo", IECore.PathMatcher( ['/L/M/N'] ) )
		G.writeSet( "matti", IECore.PathMatcher( ['/'] ) )

		del O, N, M, L, K, J, I, H, G, F, E, D, C, B, A, writeRoot

		readRoot = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set(readRoot.childNames()), set (['A', 'H']) )

		A = readRoot.child('A')

		self.assertEqual( set( A.childNames() ), set( ['B', 'C', 'D'] ) )
		B = A.child('B')
		C = A.child('C')
		D = A.child('D')
		E = B.child('E')
		F = B.child('F')
		H = readRoot.child('H')

		self.assertEqual( set( B.childNames() ), set( ['E', 'F'] ) )
		self.assertEqual( D.childNames(), ['G'] )

		self.assertEqual( set(B.readSet("don").paths() ), set(['/E'] ) )
		self.assertEqual( set(B.readSet("john").paths() ), set(['/F'] ) )
		self.assertEqual( set(C.readSet("don").paths() ), set(['/O'] ) )
		self.assertEqual( set(D.readSet("john").paths() ), set(['/G'] ) )

		self.assertEqual( set(E.readSet("don").paths() ), set([] ) )

		# Check the setNames returns all the sets in it's subtree
		self.assertEqual( set( B.setNames() ), set( ['don', 'john'] ) )
		self.assertEqual( set( C.setNames() ), set( ['don'] ) )
		self.assertEqual( set( D.setNames() ), set( ['john', 'matti'] ) )
		self.assertEqual( set( E.setNames() ), set() )
		self.assertEqual( set( F.setNames() ), set() )

		self.assertEqual( len( A.setNames() ), 3)
		self.assertEqual( set( A.setNames() ), set( ['don', 'john', 'matti'] ) )
		self.assertEqual( set( A.readSet( "don" ).paths() ), set( ['/B/E', '/C/O'] ) )
		self.assertEqual( set( A.readSet( "john" ).paths() ), set( ['/B/F', '/D/G'] ) )

		self.assertEqual( set( H.readSet( "foo" ).paths() ), set( ['/I/J/K/L/M/N'] ) )

		self.assertEqual( len( A.setNames( includeDescendantSets = False ) ), 0)
		self.assertEqual( set( A.readSet( "don", includeDescendantSets = False ).paths() ), set( [] ) )
		self.assertEqual( set( A.readSet( "john", includeDescendantSets = False ).paths() ), set( [] ) )

		self.assertEqual( len( B.setNames( includeDescendantSets = False ) ), 2)
		self.assertEqual( set( B.setNames( includeDescendantSets = False ) ), set( ['don', 'john'] ))
		self.assertEqual( set( B.readSet( "don", includeDescendantSets = False ).paths() ), set( ['/E'] ) )
		self.assertEqual( set( B.readSet( "john", includeDescendantSets = False ).paths() ), set( ['/F'] ) )

	def testSetHashes( self ):

		# A
		#   B

		# Note we don't need to write out any sets to test the hashing a
		# as we only use scene graph location, filename & set name for the hash

		writeRoot = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )

		A = writeRoot.createChild("A")
		B = A.createChild("B")

		del A, B, writeRoot

		shutil.copyfile('/tmp/test.abc', '/tmp/testAnotherFile.abc')

		readRoot = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		readRoot2 = IECoreScene.SceneInterface.create( "/tmp/testAnotherFile.abc", IECore.IndexedIO.OpenMode.Read )

		readRoot3 = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		A = readRoot.child('A')
		Ap = readRoot.child('A')

		self.assertNotEqual( A.hashSet("dummySetA"), A.hashSet("dummySetB") )
		self.assertEqual( A.hashSet("dummySetA"), Ap.hashSet("dummySetA") )

		B = A.child("B")

		self.assertNotEqual( A.hashSet("dummySetA"), B.hashSet("dummySetA") )

		A2 = readRoot2.child('A')
		self.assertNotEqual( A.hashSet("dummySetA"), A2.hashSet("dummySetA") )

		A3 = readRoot3.child('A')
		self.assertEqual( A.hashSet("dummySetA"), A3.hashSet("dummySetA") )

	def testCanReadAndWriteQuatPrimvars( self ) :

		def _testTypedDataRoundTrip( data ):

			w = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
			c = w.createChild( "o" )

			boxMeshPrimitive = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )

			boxMeshPrimitive["qf"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, data )

			c.writeObject( boxMeshPrimitive, 0.0 )

			del c, w

			r = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
			c = r.child( "o" )

			o = c.readObject( 0.0 )

			self.assertTrue( "qf" in o.keys() )

			self.assertEqual( o["qf"].data, data )
			self.assertEqual( o["qf"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		_testTypedDataRoundTrip( IECore.QuatfVectorData( [imath.Quatf( i, 0, 0, 0 ) for i in range( 8 )] ) )
		_testTypedDataRoundTrip( IECore.QuatdVectorData( [imath.Quatd( i, 0, 0, 0 ) for i in range( 8 )] ) )

	def testConcurrentAccessToChildren( self ) :

		# Write file with 1000 children

		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		root = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		root.writeBound( imath.Box3d( mesh.bound() ), 0 )
		for i in range( 0, 1000 ) :
			child = root.createChild( str( i ) )
			child.writeObject( mesh, 0 )
			child.writeBound( imath.Box3d( mesh.bound() ), 0 )

		del root, child

		# Read file concurrently to expose crash bug in child access.
		# It is not sufficient to make only a single call to `parallelReadAll()`
		# because internally it calls `root->child()` in series at each level.
		# Instead we launch several threads each of which calls `parallelReadAll()`.

		root = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		threads = []
		for i in range( 0, 10 ) :
			thread = threading.Thread(
				target = IECoreScene.SceneAlgo.parallelReadAll,
				args = ( root, 0, 0, 1.0, IECoreScene.SceneAlgo.ProcessFlags.All )
			)
			threads.append( thread )
			thread.start()

		for thread in threads :
			thread.join()

	def testChildAccessPerformance( self ) :

		root = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		for i in range( 0, 10000 ) :
			child = root.createChild( str( i ) )

		del root, child

		root = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		t = IECore.Timer()
		IECoreScene.SceneAlgo.parallelReadAll( root, 0, 0, 1.0, IECoreScene.SceneAlgo.ProcessFlags.None )
		# print t.stop() # Uncomment for timing information

		# All times are the best of 4 runs, measured using local
		# SSD and 12 TBB threads (hostname ludo).
		#
		# Baseline (before bugfix)                      :   2.01s
		# With fix                                      :   2.06s
		# Using tbb::mutex instead of tbb::spin_mutex   :   2.21s

	def testConcurrentChildAccessPerformance( self ) :

		root = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		for i in range( 0, 10000 ) :
			child = root.createChild( str( i ) )

		del root, child

		root = IECoreScene.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		t = IECore.Timer()
		threads = []
		for i in range( 0, 10 ) :
			thread = threading.Thread(
				target = IECoreScene.SceneAlgo.parallelReadAll,
				args = ( root, 0, 0, 1.0, IECoreScene.SceneAlgo.ProcessFlags.None )
			)
			threads.append( thread )
			thread.start()

		for thread in threads :
			thread.join()

		# print t.stop() # Uncomment for timing information

		# All times are the best of 4 runs, measured using local
		# SSD and 12 TBB threads (hostname ludo).
		#
		# Baseline (before bugfix)                          :    Crashes
		# With fix                                          :    2.99s
		# Using tbb::mutex instead of tbb::spin_mutex       :    3.42s
		# Using concurrent_hash_map (finer grained locking) :    3.94s

	def testReadCreases( self ) :

		root = IECoreAlembic.AlembicScene( os.path.dirname( __file__ ) + "/data/creases.abc", IECore.IndexedIO.OpenMode.Read )
		cube = root.child( "CUBE" ).child( "C_cube_REN" ).readObjectAtSample( 0 )

		self.assertEqual( cube.creaseLengths(), IECore.IntVectorData( [ 2, 2, 2 ] ) )
		self.assertEqual( cube.creaseIds(), IECore.IntVectorData( [ 4, 5, 3, 5, 5, 7 ] ) )
		self.assertEqual( cube.creaseSharpnesses(), IECore.FloatVectorData( [ 1.28999900818, 1.28999900818, 1.28999900818 ] ) )

	def testRoundTripCornersAndCreases( self ) :

		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		del mesh["N"] # Reference caches created without normals
		mesh.setInterpolation( "catmullClark" )
		mesh.setCorners( IECore.IntVectorData( [ 3 ] ), IECore.FloatVectorData( [ 2 ] ) )
		mesh.setCreases( IECore.IntVectorData( [ 2 ] ), IECore.IntVectorData( [ 0, 1 ] ), IECore.FloatVectorData( [ 2.5 ] ) )

		root = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		child = root.createChild( "cube" )
		child.writeObject( mesh, 0 )
		del root, child

		root = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		child = root.child( "cube" )

		self.assertEqual( child.readObjectAtSample( 0 ), mesh )

if __name__ == "__main__":
    unittest.main()
