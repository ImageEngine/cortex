##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
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
import unittest

import IECore
import IECoreAlembic

class AlembicSceneTest( unittest.TestCase ) :

	def testSupportedExtension( self ) :
		self.assertTrue( "abc" in IECore.SceneInterface.supportedExtensions() )
		self.assertTrue( "abc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Read ) )
		self.assertTrue( "abc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Write ) )
		self.assertFalse( "abc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Append ) )

	def testFactoryFunction( self ) :
		# test Write factory function 
		m = IECore.SceneInterface.create( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		self.assertTrue( isinstance( m, IECoreAlembic.AlembicScene ) )
		#self.assertEqual( m.fileName(), os.path.dirname( __file__ ) + "/data/cube.abc" )
		self.assertRaises( RuntimeError, m.readBound, 0.0 )
		del m
		# test Read factory function
		m = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( isinstance( m, IECoreAlembic.AlembicScene ) )
		m.readBound( 0.0 )
		#self.assertEqual( m.fileName(), os.path.dirname( __file__ ) + "/data/cube.abc" )

	def testAppendRaises( self ) :
		self.assertRaises( RuntimeError, IECoreAlembic.AlembicScene, "/tmp/test.scc", IECore.IndexedIO.OpenMode.Append )

	def testReadNonExistentRaises( self ) :
		self.assertRaises( RuntimeError, IECoreAlembic.AlembicScene, "iDontExist.scc", IECore.IndexedIO.OpenMode.Read )

	def testHierarchy( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( len( a.childNames() ), 1 )
		self.assertEqual( a.childNames(), [ "group1" ] )

		g1 = a.child( "group1" )

		self.assertEqual( g1.name(), "group1" )

		self.assertEqual( g1.path(), [ "group1" ] )

		self.assertEqual( g1.childNames(), ["pCube1"] )

		c = g1.child( "pCube1" )
		self.assertEqual( c.name(), "pCube1" )
		self.assertEqual( c.path(), ["group1","pCube1"] )

		# make sure mesh shape is hidden:
		self.assertEqual( c.childNames(), [] )
		self.assertRaises( Exception, c.child, "pCubeShape1" )
		self.assertRaises( Exception, c.child, "pCubeShape1", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )
		self.assertEqual( c.child( "pCubeShape1", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )

		# make sure bogus child name doesn't exist either:
		self.assertRaises( Exception, c.child, "iDontExist" )
		self.assertRaises( Exception, c.child, "iDontExist", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )
		self.assertEqual( c.child( "iDontExist", IECore.SceneInterface.MissingBehaviour.NullIfMissing ), None )


	def testHasObject( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertFalse( a.hasObject() )
		self.assertFalse( a.child("group1").hasObject() )
		self.assertTrue( a.child("group1").child("pCube1").hasObject() )

	def testConvertMesh( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )

		c = a.child( "group1" )
		self.assertEqual( c.hasObject(), False )
		self.assertEqual( c.readObjectAtSample( 0 ), IECore.NullObject.defaultNullObject() )

		c = c.child( "pCube1" )
		m = c.readObject( 0 )

		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )

	def testConvertTransform( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )

		g = a.child( "group1" )
		t = g.readTransformAtSample( 0 )
		self.assertEqual( t, IECore.M44dData( IECore.M44d.createScaled( IECore.V3d( 2 ) ) * IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )
		t = g.readTransform( 0 )
		self.assertEqual( t, IECore.M44dData( IECore.M44d.createScaled( IECore.V3d( 2 ) ) * IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )

		c = a.child( "group1" ).child( "pCube1" )
		t = c.readTransformAsMatrixAtSample( 0 )
		self.assertEqual( t, IECore.M44d.createTranslated( IECore.V3d( -1, 0, 0 ) ) )
		t = c.readTransformAsMatrix( 0 )
		self.assertEqual( t, IECore.M44d.createTranslated( IECore.V3d( -1, 0, 0 ) ) )

	def testBound( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -2 ), IECore.V3d( 2 ) ) )
		self.assertEqual( a.readBound( 0 ), IECore.Box3d( IECore.V3d( -2 ), IECore.V3d( 2 ) ) )

		cs = a.child( "group1" ).child( "pCube1" )
		self.assertEqual( cs.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		self.assertEqual( cs.readBound( 0 ), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )

	def testTransform( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readTransformAsMatrixAtSample( 0 ), IECore.M44d() )

		g = a.child( "group1" )
		self.assertEqual( g.readTransformAsMatrixAtSample( 0 ), IECore.M44d.createScaled( IECore.V3d( 2 ) ) * IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) )

		c = g.child( "pCube1" )
		self.assertEqual( c.readTransformAsMatrixAtSample( 0 ), IECore.M44d.createTranslated( IECore.V3d( -1, 0, 0 ) ) )

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
				IECore.Color4f( 0, 1, 0, 1 ),
				IECore.Color4f( 0, 0, 1, 1 ),
				IECore.Color4f( 0, 0, 0, 1 ),
				IECore.Color4f( 1, 0, 0, 1 ),												
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

		self.assertEqual( a.numTransformSamples(), 10 )
		for i in range( 0, a.numTransformSamples() ) :
			self.assertAlmostEqual( a.transformSampleTime( i ), (i + 1) / 24.0 )

		p = a.child( "persp" )
		self.assertEqual( p.numTransformSamples(), 1 )	
		self.assertEqual( p.transformSampleTime( 0 ), 1 / 24.0 )

		t = a.child( "pCube1" )
		self.assertEqual( t.numTransformSamples(), 10 )	
		for i in range( 0, t.numTransformSamples() ) :
			self.assertAlmostEqual( t.transformSampleTime( i ), (i + 1) / 24.0 )

		self.assertEqual( t.numObjectSamples(), 10 )
		for i in range( 0, t.numObjectSamples() ) :
			self.assertAlmostEqual( t.objectSampleTime( i ), (i + 1) / 24.0 )

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/noTopLevelStoredBounds.abc", IECore.IndexedIO.OpenMode.Read )

		# no time samples at the top level, so this should throw an exception:
		self.assertRaises( Exception, a.transformSampleTime, 0 )

		# should throw the RIGHT exceptions:
		try:
			a.transformSampleTime(0)
		except Exception, e:
			self.assertEqual( str(e), "Invalid Argument : Sample index out of range" )

		# should these throw exceptions?
		#a.readBoundAtSample(0)
		#a.readObjectAtSample(0)
		#a.readTransformAtSample(0)
		
	def testDifferentSampleTypes( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		b = a.createChild("blah")

		b.writeObject( IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) ), 0 )
		b.writeObject( IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-2), IECore.V3f(2) ) ), 1 )

		b.writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d(1,2,3) ) ), -1.0 )
		b.writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d(2,3,4) ) ), -0.75 )
		b.writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d(3,4,5) ) ), -0.5 )

		del a, b

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		b = a.child("blah")

		# test object samples:
		self.assertEqual( b.numObjectSamples(), 2 )
		self.assertEqual( b.objectSampleTime(0), 0.0 )
		self.assertEqual( b.objectSampleTime(1), 1.0 )

		# test transform samples:
		self.assertEqual( b.numTransformSamples(), 3 )
		self.assertEqual( b.transformSampleTime(0), -1.0 )
		self.assertEqual( b.transformSampleTime(1), -0.75 )
		self.assertEqual( b.transformSampleTime(2), -0.5 )
		
	def testOutOfRangeSamplesRaise( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertRaises( Exception, a.boundSampleTime, 10 )

	def testSampleInterval( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )

		# persp has only one sample, so should always be reading from that regardless the time
		p = a.child( "persp" )
		t = -1000
		while t < 1000 :
			t += .01
			self.assertEqual( p.boundSampleInterval( t ), ( 0, 0, 0 ) )

		# pCube1 has a sample per frame
		t = a.child( "pCube1" )
		for i in range( 0, t.numBoundSamples() ) :
			# reads on the frame should not need
			# interpolation.
			v = t.boundSampleInterval( t.boundSampleTime( i ) )
			self.assertEqual( v[0], 0 )
			self.assertEqual( v[1], i )
			self.assertEqual( v[1], i )
			# reads in between frames should need
			# interpolation
			if i < t.numBoundSamples() -1 :
				v = t.boundSampleInterval( t.boundSampleTime( i ) + 1 / 48.0 )
				self.assertAlmostEqual( v[0], 0.5 )
				self.assertEqual( v[1], i )
				self.assertEqual( v[2], i + 1 )

	def testTransformAtSample( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		t = a.child( "pCube1" )

		matrix = t.readTransformAsMatrixAtSample( 0 )
		self.assertEqual( matrix, IECore.M44d() )

		for i in range( 1, t.numTransformSamples() ) :
			matrix2 = t.readTransformAsMatrixAtSample( i )
			self.assertNotEqual( matrix, matrix2 )
			expectedMatrix = IECore.M44d.createTranslated( IECore.V3d( i / 9.0, 0, 0 ) )
			self.failUnless( matrix2.equalWithAbsError( expectedMatrix, 0.0000001 ) )

		self.assertEqual( t.readTransformAsMatrixAtSample( t.numTransformSamples() - 1 ), IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) )

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

		for i in range( 0, t.numTransformSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.readTransformAsMatrix( time )
			expectedMatrix = IECore.M44d.createTranslated( IECore.V3d( i / 18.0, 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )

	def testInterpolatedRotate( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/rotatingCube.abc", IECore.IndexedIO.OpenMode.Read )

		t = a.child( "pCube1" )	
		for i in range( 0, t.numTransformSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.readTransformAsMatrix( time )
			expectedMatrix = IECore.M44d.createRotated( IECore.V3d( IECore.degreesToRadians( 90 * i * 0.5 ), 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )

	def testBoundAtSample( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( a.readBoundAtSample( a.numBoundSamples()-1 ), IECore.Box3d( IECore.V3d( 0.5, -0.5, -0.5 ), IECore.V3d( 1.5, 2, 0.5 ) ) )

		t = a.child( "pCube1" )
		self.assertEqual( t.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( t.readBoundAtSample( t.numBoundSamples()-1 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 2, 0.5 ) ) )

	def testBoundAtTime( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		t = a.child( "pCube1" )

		startTime = a.boundSampleTime( 0 )
		endTime = a.boundSampleTime( a.numBoundSamples() - 1 )

		aStartBound = a.readBoundAtSample( 0 )
		aEndBound = a.readBoundAtSample( a.numBoundSamples() - 1 )

		mStartBound = t.readBoundAtSample( 0 )
		mEndBound = t.readBoundAtSample( t.numBoundSamples() - 1 )

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

			tBound = t.readBound( time )
			expectedMBound = lerpBox( mStartBound, mEndBound, lerpFactor )
			self.failUnless( tBound.min.equalWithAbsError( expectedMBound.min, 0.000001 ) )
			self.failUnless( tBound.max.equalWithAbsError( expectedMBound.max, 0.000001 ) )

	def testConvertNormals( self ) :

		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		m = a.child( "pCube1" )
		mesh = m.readObjectAtSample( 0 )

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

	def testAttributes( self ) :

		# attributes aren't supported right now...
		b = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( b.attributeNames(), [] )

	def testHashes( self ) :

		b = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/cube.abc", IECore.IndexedIO.OpenMode.Read )
		t = b.child( "group1" )

		# make sure all hashes are different:
		hashList = []
		hashList.append(  t.hash( IECore.SceneInterface.HashType.TransformHash, 0 ).toString() )
		hashList.append(  t.hash( IECore.SceneInterface.HashType.BoundHash, 0 ).toString() )
		hashList.append(  t.hash( IECore.SceneInterface.HashType.ObjectHash, 0 ).toString() )
		hashList.append(  t.hash( IECore.SceneInterface.HashType.ChildNamesHash, 0 ).toString() )
		hashList.append(  t.hash( IECore.SceneInterface.HashType.AttributesHash, 0 ).toString() )
		hashList.append(  t.hash( IECore.SceneInterface.HashType.HierarchyHash, 0 ).toString() )
		self.assertEqual( len( hashList ), len( set( hashList ) ) )

		# TransformHash:

		# shouldn't change on a non animated object:
		self.assertEqual( t.hash( IECore.SceneInterface.HashType.TransformHash, 0 ), t.hash( IECore.SceneInterface.HashType.TransformHash, 1 ) )

		# however, it SHOULD change on an animated object:
		a = IECore.SceneInterface.create( os.path.dirname( __file__ ) + "/data/animatedCube.abc", IECore.IndexedIO.OpenMode.Read )
		t = a.child( "pCube1" )	
		self.assertNotEqual( t.hash( IECore.SceneInterface.HashType.TransformHash, 1.0/24 ), t.hash( IECore.SceneInterface.HashType.TransformHash, 1.1/24 ) )

		# BoundHash:
		self.assertNotEqual( t.hash( IECore.SceneInterface.HashType.BoundHash, 0 ), t.hash( IECore.SceneInterface.HashType.BoundHash, 100 ) )

		# ObjectHash:
		self.assertNotEqual( t.hash( IECore.SceneInterface.HashType.ObjectHash, 0 ), t.hash( IECore.SceneInterface.HashType.ObjectHash, 100 ) )

		# ChildNamesHash:
		self.assertEqual( b.child( "group1" ).child("pCube1").hash( IECore.SceneInterface.HashType.ChildNamesHash, 0 ), a.child("pCube1").hash( IECore.SceneInterface.HashType.ChildNamesHash, 0 ) )
		self.assertEqual( a.hash( IECore.SceneInterface.HashType.ChildNamesHash, 0 ), a.hash( IECore.SceneInterface.HashType.ChildNamesHash, 1000 ) )
		self.assertNotEqual( b.hash( IECore.SceneInterface.HashType.ChildNamesHash, 0 ), a.hash( IECore.SceneInterface.HashType.ChildNamesHash, 0 ) )

	def testWriteConstructor( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )

		self.assertRaises( Exception, IECoreAlembic.AlembicScene, "/tmp/blah/blah/blah/blah/blah.abc", IECore.IndexedIO.OpenMode.Write )

	def testHierarchy( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a.createChild("blah")
		a.child("yeah", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )
		self.assertEqual( set( a.childNames() ), set([ "yeah", "blah" ]) )

		a.child("blah").createChild("whatever")
		self.assertEqual( set(a.child("blah").childNames()), set(["whatever"]) )

		self.assertEqual( set( a.childNames() ), set([ "yeah", "blah" ]) )
		self.assertEqual( set( a.child("blah").childNames() ), set(["whatever"]) )
		self.assertEqual( set( a.child("yeah").childNames() ), set() )
		self.assertEqual( set( a.child("blah").child("whatever").childNames() ), set() )

		self.assertEqual( a.child("blah").name(), "blah" )
		self.assertEqual( a.child("blah").child("whatever").name(), "whatever" )
		self.assertEqual( a.child("yeah").name(), "yeah" )

		self.assertEqual( a.path(), [] )
		self.assertEqual( a.child("blah").path(), ["blah"] )
		self.assertEqual( a.child("blah").child("whatever").path(), ["blah","whatever"] )
		self.assertEqual( a.child("yeah").path(), ["yeah"] )

		del a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( set( a.childNames() ), set([ "yeah", "blah" ]) )
		self.assertEqual( set( a.child("blah").childNames() ), set(["whatever"]) )
		self.assertEqual( set( a.child("yeah").childNames() ), set() )
		self.assertEqual( set( a.child("blah").child("whatever").childNames() ), set() )

	def testWriteStaticTransform( self ) :
		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )

		self.assertRaises( RuntimeError, a.writeTransform, IECore.M44dData( IECore.M44d.createRotated( IECore.V3d(0,2,0) ) ), 0 )
		a.createChild("blah").writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d(1,2,3) ) ), 0 )

		del a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.readTransformAsMatrix( 0 ), IECore.M44d() )
		self.assertEqual( a.child("blah").readTransformAsMatrix( 0 ), IECore.M44d.createRotated( IECore.V3d(1,2,3) ) )
		self.assertEqual( a.child("blah").readTransformAsMatrix( 10 ), IECore.M44d.createRotated( IECore.V3d(1,2,3) ) )

	def testWriteAnimatedTransform( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )

		a.createChild("blah").writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(1,2,3) ) ), -1 )
		a.child("blah").writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(4,5,6) ) ), 1 )
		a.child("blah").writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(7,8,9) ) ), 1.5 )

		self.assertRaises( RuntimeError, a.writeTransform, IECore.M44dData( IECore.M44d.createRotated( IECore.V3d(0,2,0) ) ), -2 )

		del a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( a.child("blah").readTransform( -2 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(1,2,3) ) ) )
		self.assertEqual( a.child("blah").readTransform( -1 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(1,2,3) ) ) )
		self.assertEqual( a.child("blah").readTransform( 0 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(2.5,3.5,4.5) ) ) )
		self.assertEqual( a.child("blah").readTransform( 1 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(4,5,6) ) ) )
		self.assertEqual( a.child("blah").readTransform( 1.25 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(5.5,6.5,7.5) ) ) )
		self.assertEqual( a.child("blah").readTransform( 1.5 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(7,8,9) ) ) )
		
		self.assertEqual( a.child("blah").readTransform( 2 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d(7,8,9) ) ) )

	def testWriteMesh( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a.createChild("blah")

		mesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) )
		self.assertRaises( RuntimeError, a.writeObject, mesh, 0 )
		self.assertRaises( RuntimeError, a.child("blah").writeObject, None, 0 )

		# write an actual mesh:
		a.child("blah").writeObject( mesh, 0 )

		# make sure new child name doesn't show up:
		self.assertEqual( len( a.child("blah").childNames() ), 0 )

		# write a different type of object:
		self.assertRaises( RuntimeError, a.child("blah").writeObject, IECore.BoolData(), 0 )
		
		# write the same object at an earlier time:
		self.assertRaises( RuntimeError, a.child("blah").writeObject, mesh, -1 )
		
		# try and write a subdiv mesh:
		subdivMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) )
		subdivMesh.interpolation = "catmullClark"
		self.assertRaises( RuntimeError, a.child("blah").writeObject, subdivMesh, 2 )

		# write a different mesh at a later time:
		a.child("blah").writeObject( IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-2), IECore.V3f(2) ) ), 1 )

		del a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )
		b = a.child("blah")

		self.assertEqual( set( b.childNames() ), set() )

		# test object samples:
		self.assertEqual( b.numObjectSamples(), 2 )
		self.assertEqual( b.objectSampleTime(0), 0.0 )
		self.assertEqual( b.objectSampleTime(1), 1.0 )
		self.assertEqual( b.readObjectAtSample( 0 ), IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) ) )
		self.assertEqual( b.readObjectAtSample( 1 ), IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-2), IECore.V3f(2) ) ) )

		# test interpolated objects:
		self.assertEqual( b.readObject( -1 ), IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) ) )
		self.assertEqual( b.readObject( 0 ), IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) ) )
		self.assertEqual( b.readObject( 0.5 ), IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1.5), IECore.V3f(1.5) ) ) )
		self.assertEqual( b.readObject( 1 ), IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-2), IECore.V3f(2) ) ) )
		self.assertEqual( b.readObject( 2 ), IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-2), IECore.V3f(2) ) ) )

	def testWriteSubdiv( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a.createChild("blah")

		subdivMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) )
		subdivMesh.interpolation = "catmullClark"
		self.assertRaises( RuntimeError, a.writeObject, subdivMesh, 0 )
		self.assertRaises( RuntimeError, a.child("blah").writeObject, None, 0 )

		# write an actual mesh:
		a.child("blah").writeObject( subdivMesh, 0 )

		# make sure new child name doesn't show up:
		self.assertEqual( len( a.child("blah").childNames() ), 0 )

		# write a different type of object:
		self.assertRaises( RuntimeError, a.child("blah").writeObject, IECore.BoolData(), 0 )

		# write the same object at an earlier time:
		self.assertRaises( RuntimeError, a.child("blah").writeObject, subdivMesh, -1 )

		# try and write a subdiv mesh:
		self.assertRaises( RuntimeError, a.child("blah").writeObject, IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) ), 2 )

		# write a different mesh at a later time:
		subdivMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-2), IECore.V3f(2) ) )
		subdivMesh.interpolation = "catmullClark"
		a.child("blah").writeObject( subdivMesh, 1 )

		del a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set( a.child("blah").childNames() ), set() )

		self.assertEqual( a.child("blah").readObject( 1 ), subdivMesh )
		self.assertEqual( a.child("blah").readObject( 2 ), subdivMesh )

		subdivMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1), IECore.V3f(1) ) )
		subdivMesh.interpolation = "catmullClark"

		self.assertEqual( a.child("blah").readObject( -1 ), subdivMesh )
		self.assertEqual( a.child("blah").readObject( 0 ), subdivMesh )

		subdivMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f(-1.5), IECore.V3f(1.5) ) )
		subdivMesh.interpolation = "catmullClark"

		self.assertEqual( a.child("blah").readObject( 0.5 ), subdivMesh )

	def testWriteCamera( self ) :

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Write )
		a.createChild("blah").writeObject( IECore.Camera(), 0 )

		# make sure new child name doesn't show up:
		self.assertEqual( len( a.child("blah").childNames() ), 0 )

		del a

		a = IECoreAlembic.AlembicScene( "/tmp/test.abc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set( a.child("blah").childNames() ), set() )
		self.assertTrue( isinstance( a.child("blah").readObject(0), IECore.Camera ) )

if __name__ == "__main__":
    unittest.main()
