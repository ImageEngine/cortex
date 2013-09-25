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

class AlembicInputTest( unittest.TestCase ) :

	def testConstructor( self ) :

		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		self.assertRaises( Exception, IECoreAlembic.AlembicInput, "iDontExist" )
	
	def testHierarchy( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		self.assertEqual( a.numChildren(), 1 )
		self.assertEqual( a.childNames(), IECore.StringVectorData( [ "group1" ] ) )
				
		g1 = a.child( 0 )
		g2 = a.child( "group1" )
		
		self.assertEqual( g1.name(), g2.name() )
		self.assertEqual( g1.name(), "group1" )
		
		self.assertEqual( g1.fullName(), g2.fullName() )
		self.assertEqual( g1.fullName(), "/group1" )
		
		self.assertEqual( g1.numChildren(), 1 )
		self.assertEqual( g1.childNames(), IECore.StringVectorData( [ "pCube1" ] ) )
		
		c = g1.child( 0 )
		self.assertEqual( c.name(), "pCube1" )
		self.assertEqual( c.fullName(), "/group1/pCube1" )
		
		self.assertEqual( c.numChildren(), 1 )
		self.assertEqual( c.childNames(), IECore.StringVectorData( [ "pCubeShape1" ] ) )
		
		cs = c.child( 0 )
		self.assertEqual( cs.numChildren(), 0 )
		self.assertEqual( cs.childNames(), IECore.StringVectorData() )
		
		self.assertRaises( Exception, cs.child, 0 )
		self.assertRaises( Exception, cs.child, "iDontExist" )
						
	def testConvertMesh( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		c = a.child( "group1" ).child( "pCube1" )
		self.assertEqual( c.objectAtSample( 0, IECore.MeshPrimitive.staticTypeId() ), None )
		
		cs = c.child( "pCubeShape1" )
		m = cs.objectAtSample( 0, IECore.MeshPrimitive.staticTypeId() )
		
		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )
		
	def testConvertTransform( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		g = a.child( "group1" )
		t = g.objectAtSample( 0, IECore.M44fData.staticTypeId() )
		self.assertEqual( t, IECore.M44fData( IECore.M44f.createScaled( IECore.V3f( 2 ) ) * IECore.M44f.createTranslated( IECore.V3f( 2, 0, 0 ) ) ) )
		
		c = a.child( "group1" ).child( "pCube1" )
		t = c.objectAtSample( 0, IECore.M44fData.staticTypeId() )
		self.assertEqual( t, IECore.M44fData( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) ) )
		
		cs = c.child( "pCubeShape1" )
		t = cs.objectAtSample( 0, IECore.M44fData.staticTypeId() )
		self.assertEqual( t, None )
		
	def testMetaData( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		
		m = a.metaData()
		self.failUnless( isinstance( m , IECore.CompoundData ) )
		
	def testBound( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		self.assertEqual( a.boundAtSample(), IECore.Box3d( IECore.V3d( -2 ), IECore.V3d( 2 ) ) )
		
		cs = a.child( "group1" ).child( "pCube1" ).child( "pCubeShape1" )
		self.assertEqual( cs.boundAtSample(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )		
	
	def testTransform( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/cube.abc" )
		self.assertEqual( a.transformAtSample(), IECore.M44d() )
				
		g = a.child( "group1" )
		self.assertEqual( g.transformAtSample(), IECore.M44d.createScaled( IECore.V3d( 2 ) ) * IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) )
		
		c = g.child( "pCube1" )
		self.assertEqual( c.transformAtSample(), IECore.M44d.createTranslated( IECore.V3d( -1, 0, 0 ) ) )
		
		cs = c.child( "pCubeShape1" )
		self.assertEqual( cs.transformAtSample(), IECore.M44d() )
		
	def testConvertSubD( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/subdPlane.abc" )
		
		c = a.child( "pPlane1" )
		self.assertEqual( c.objectAtSample( 0, IECore.MeshPrimitive.staticTypeId() ), None )
		
		cs = c.child( "pPlaneShape1" )
		m = cs.objectAtSample( 0, IECore.MeshPrimitive.staticTypeId() )
		
		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )
		self.assertEqual( m.interpolation, "catmullClark" )
		
	def testConvertArbGeomParams( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/coloredMesh.abc" )
		
		m = a.child( "pPlane1" ).child( "pPlaneShape1" ).objectAtSample( 0, IECore.MeshPrimitive.staticTypeId() )
				
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
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/coloredMesh.abc" )
		m = a.child( "pPlane1" ).child( "pPlaneShape1" ).objectAtSample( 0, IECore.MeshPrimitive.staticTypeId() )
		
		self.failUnless( "s" in m )
		self.failUnless( "t" in m )				
	
		self.assertEqual( m["s"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m["t"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		
		self.failUnless( isinstance( m["s"].data, IECore.FloatVectorData ) )
		self.failUnless( isinstance( m["t"].data, IECore.FloatVectorData ) )
		
	def testSamples( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		
		self.assertEqual( a.numSamples(), 10 )
		for i in range( 0, a.numSamples() ) :
			self.assertAlmostEqual( a.timeAtSample( i ), (i + 1) / 24.0 )
		
		p = a.child( "persp" )
		self.assertEqual( p.numSamples(), 1 )	
		self.assertEqual( p.timeAtSample( 0 ), 1 / 24.0 )
		
		t = a.child( "pCube1" )
		self.assertEqual( t.numSamples(), 10 )	
		for i in range( 0, t.numSamples() ) :
			self.assertAlmostEqual( t.timeAtSample( i ), (i + 1) / 24.0 )
	
		m = t.child( "pCubeShape1" )
		self.assertEqual( m.numSamples(), 10 )	
		for i in range( 0, m.numSamples() ) :
			self.assertAlmostEqual( m.timeAtSample( i ), (i + 1) / 24.0 )
		
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/noTopLevelStoredBounds.abc" )
		
		self.assertEqual( a.numSamples(), 0 )
		
		# no time samples at the top level, so this should throw an exception:
		self.assertRaises( Exception, a.timeAtSample, 0 )
		
		# should throw the RIGHT exceptions:
		try:
			a.timeAtSample(0)
		except Exception, e:
			self.assertEqual( str(e), "Invalid Argument : Sample index out of range" )
		
		# should these throw exceptions?
		a.boundAtSample(0)
		a.objectAtSample(0)
		a.transformAtSample(0)
		
		
	
	def testOutOfRangeSamplesRaise( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )	
	
		self.assertRaises( Exception, a.timeAtSample, 10 )
		
	def testSampleInterval( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		
		# persp has only one sample, so should always be reading from that regardless the time
		p = a.child( "persp" )
		t = -1000
		while t < 1000 :
			t += .01
			self.assertEqual( p.sampleIntervalAtTime( t ), ( 0, 0, 0 ) )
	
		# pCube1 has a sample per frame
		t = a.child( "pCube1" )
		for i in range( 0, t.numSamples() ) :
			# reads on the frame should not need
			# interpolation.
			v = t.sampleIntervalAtTime( t.timeAtSample( i ) )
			self.assertEqual( v[0], 0 )
			self.assertEqual( v[1], i )
			self.assertEqual( v[1], i )
			# reads in between frames should need
			# interpolation
			if i < t.numSamples() -1 :
				v = t.sampleIntervalAtTime( t.timeAtSample( i ) + 1 / 48.0 )
				self.assertAlmostEqual( v[0], 0.5 )
				self.assertEqual( v[1], i )	
				self.assertEqual( v[2], i + 1 )			
	
	def testConverterAccess( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		m = a.child( "pCube1" ).child( "pCubeShape1" )
		c = m.converter()
		
		mesh = c.convert()
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		
	def testConvertAtIndices( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		m = a.child( "pCube1" ).child( "pCubeShape1" )
		c = m.converter()
		
		mesh = c.convert()
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		
		for i in range( 1, m.numSamples() ) :
			c["sampleIndex"].setNumericValue( i )
			mesh2 = c.convert()
			self.failUnless( isinstance( mesh2, IECore.MeshPrimitive ) )
			self.assertEqual( mesh.verticesPerFace, mesh2.verticesPerFace )
			self.assertNotEqual( mesh["P"], mesh2["P"] )
	
	def testTransformAtSample( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		t = a.child( "pCube1" )
		
		matrix = t.transformAtSample()
		self.assertEqual( matrix, IECore.M44d() )
		self.assertEqual( matrix, t.transformAtSample( 0 ) )
		
		for i in range( 1, t.numSamples() ) :
			matrix2 = t.transformAtSample( i )
			self.assertNotEqual( matrix, matrix2 )
			expectedMatrix = IECore.M44d.createTranslated( IECore.V3d( i / 9.0, 0, 0 ) )
			self.failUnless( matrix2.equalWithAbsError( expectedMatrix, 0.0000001 ) )
			
		self.assertEqual( t.transformAtSample( t.numSamples() - 1 ), IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) )
	
	def testConvertInterpolated( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		m = a.child( "pCube1" ).child( "pCubeShape1" )
		
		mesh0 = m.objectAtSample( 0 )
		mesh1 = m.objectAtSample( 1 )
		
		mesh = m.objectAtTime( 1.5 / 24.0 )
		self.failUnless( isinstance( mesh, IECore.MeshPrimitive ) )
		
		self.assertEqual( mesh, IECore.linearObjectInterpolation( mesh0, mesh1, 0.5 ) )
		
	def testRotatingTransformAtSample( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/rotatingCube.abc" )		

		t = a.child( "pCube1" )	
		for i in range( 0, 24 ) :
			ti = t.transformAtSample( i )
			mi = IECore.M44d.createRotated( IECore.V3d( IECore.degreesToRadians( 90 * i ), 0, 0 ) )
			self.failUnless( ti.equalWithAbsError( mi, 0.0000000000001 ) )
	
	def testInterpolatedTranslate( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		t = a.child( "pCube1" )
		
		for i in range( 0, t.numSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.transformAtTime( time )
			expectedMatrix = IECore.M44d.createTranslated( IECore.V3d( i / 18.0, 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )
				
	def testInterpolatedRotate( self ) :
			
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/rotatingCube.abc" )		

		t = a.child( "pCube1" )	
		for i in range( 0, t.numSamples() * 2 - 1 ) :
			frame = i / 2.0 + 1
			time = frame / 24.0
			matrix = t.transformAtTime( time )
			expectedMatrix = IECore.M44d.createRotated( IECore.V3d( IECore.degreesToRadians( 90 * i * 0.5 ), 0, 0 ) )
			self.failUnless( matrix.equalWithAbsError( expectedMatrix, 0.0000001 ) )
	
	def testHasStoredBound( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		
		self.assertEqual( a.hasStoredBound(), True )
		self.assertEqual( a.child( "persp" ).hasStoredBound(), False )
		self.assertEqual( a.child( "persp" ).child( "perspShape" ).hasStoredBound(), False )
		self.assertEqual( a.child( "pCube1" ).hasStoredBound(), False )
		self.assertEqual( a.child( "pCube1" ).child( "pCubeShape1" ).hasStoredBound(), True )		
		self.assertEqual( a.child( "front" ).hasStoredBound(), False )
		self.assertEqual( a.child( "front" ).child( "frontShape" ).hasStoredBound(), False )
		
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/noTopLevelStoredBounds.abc" )
		self.assertEqual( a.hasStoredBound(), False )
	
	def testBoundAtSample( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		self.assertEqual( a.boundAtSample( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( a.boundAtSample( a.numSamples()-1 ), IECore.Box3d( IECore.V3d( 0.5, -0.5, -0.5 ), IECore.V3d( 1.5, 2, 0.5 ) ) )
	
		t = a.child( "pCube1" )
		self.assertRaises( Exception, t.boundAtSample, 0 )
		self.assertRaises( Exception, t.boundAtSample, t.numSamples() - 1 )		
		
		m = t.child( "pCubeShape1" )
		self.assertEqual( m.boundAtSample( 0 ), IECore.Box3d( IECore.V3d( -0.5 ), IECore.V3d( 0.5 ) ) )
		self.assertEqual( m.boundAtSample( m.numSamples()-1 ), IECore.Box3d( IECore.V3d( -0.5, -0.5, -0.5 ), IECore.V3d( 0.5, 2, 0.5 ) ) )
		
	def testBoundAtTime( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		t = a.child( "pCube1" )
		m = t.child( "pCubeShape1" )
		
		startTime = a.timeAtSample( 0 )
		endTime = a.timeAtSample( a.numSamples() - 1 )
		
		aStartBound = a.boundAtSample( 0 )
		aEndBound = a.boundAtSample( a.numSamples() - 1 )
	
		mStartBound = m.boundAtSample( 0 )
		mEndBound = m.boundAtSample( m.numSamples() - 1 )
		
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
			
			aBound = a.boundAtTime( time )
			expectedABound = lerpBox( aStartBound, aEndBound, lerpFactor )
			self.failUnless( aBound.min.equalWithAbsError( expectedABound.min, 0.000001 ) )
			self.failUnless( aBound.max.equalWithAbsError( expectedABound.max, 0.000001 ) )
				
			mBound = m.boundAtTime( time )
			expectedMBound = lerpBox( mStartBound, mEndBound, lerpFactor )
			self.failUnless( mBound.min.equalWithAbsError( expectedMBound.min, 0.000001 ) )
			self.failUnless( mBound.max.equalWithAbsError( expectedMBound.max, 0.000001 ) )
			
			tBound = t.boundAtTime( time )
			self.failUnless( tBound.min.equalWithAbsError( expectedMBound.min, 0.000001 ) )
			self.failUnless( tBound.max.equalWithAbsError( expectedMBound.max, 0.000001 ) )
			
	def testConvertNormals( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		m = a.child( "pCube1" ).child( "pCubeShape1" )
		mesh = m.objectAtSample( 0 )
		
		self.failUnless( "N" in mesh )
		self.failUnless( isinstance( mesh["N"].data, IECore.V3fVectorData ) )
		self.assertEqual( mesh["N"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
	
	def testCamera( self ) :
	
		a = IECoreAlembic.AlembicInput( os.path.dirname( __file__ ) + "/data/animatedCube.abc" )
		
		c = a.child( "persp" ).child( "perspShape" ).objectAtSample( 0 )
		self.failUnless( isinstance( c, IECore.Camera ) )		
		
		c = a.child( "persp" ).child( "perspShape" ).objectAtTime( 0 )
		self.failUnless( isinstance( c, IECore.Camera ) )		
		
if __name__ == "__main__":
    unittest.main()
