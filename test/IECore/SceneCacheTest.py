##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

import gc
import sys
import math
import unittest

import IECore

class SceneCacheTest( unittest.TestCase ) :

	def testSupportedExtension( self ) :
		self.assertTrue( "scc" in IECore.SceneInterface.supportedExtensions() )
		self.assertTrue( "scc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Read ) )
		self.assertTrue( "scc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Write ) )
		self.assertTrue( "scc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Write + IECore.IndexedIO.OpenMode.Read ) )
		self.assertFalse( "scc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Append ) )

	def testFactoryFunction( self ) :
		# test Write factory function 
		m = IECore.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		self.assertTrue( isinstance( m, IECore.SceneCache ) )
		self.assertEqual( m.fileName(), "/tmp/test.scc" )
		self.assertRaises( RuntimeError, m.readBound, 0.0 )
		del m
		# test Read factory function
		m = IECore.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( isinstance( m, IECore.SceneCache ) )
		self.assertEqual( m.fileName(), "/tmp/test.scc" )
		m.readBound( 0.0 )

	def testAppendRaises( self ) :
		self.assertRaises( RuntimeError, IECore.SceneCache, "/tmp/test.scc", IECore.IndexedIO.OpenMode.Append )

	def testReadNonExistentRaises( self ) :
		self.assertRaises( RuntimeError, IECore.SceneCache, "iDontExist.scc", IECore.IndexedIO.OpenMode.Read )
		
	def testKnownStaticHierarchy( self ) :
	
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		self.assertEqual( m.path(), [] )
		self.assertEqual( m.pathAsString(), "/" )
		self.assertEqual( m.name(), "/" )
		self.assertEqual( m.hasObject(), False )
		
		m.writeAttribute( "w", IECore.BoolData( True ), 1.0 )
		
		
		t = m.createChild( "t" )
		self.assertEqual( t.path(), ["t"] )
		self.assertEqual( t.pathAsString(), "/t" )
		self.assertEqual( t.name(), "t" )
		self.assertEqual( t.hasObject(), False )
		self.assertEqual( m.childNames(), [ "t" ] )
		
		t.writeTransform( IECore.M44dData(IECore.M44d.createTranslated(IECore.V3d( 1, 0, 0 ))), 1.0 )
		self.assertEqual( t.hasObject(), False )
		
		t.writeAttribute( "wuh", IECore.BoolData( True ), 1.0 )
		
		s = t.createChild( "s" )
		self.assertEqual( s.path(), ["t","s"] )
		self.assertEqual( s.pathAsString(), "/t/s" )
		self.assertEqual( s.name(), "s" )
		self.assertEqual( s.hasObject(), False )
		self.assertEqual( t.childNames(), [ "s" ] )
		
		s.writeObject( IECore.SpherePrimitive( 1 ), 1.0 )
		self.assertEqual( s.hasObject(), True )
		
		s.writeAttribute( "glah", IECore.BoolData( True ), 1.0 )
		
		# need to delete all the SceneCache references to finalise the file
		del m, t, s

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		
		self.assertEqual( m.pathAsString(), "/" )
		self.assertEqual( m.name(), "/" )
		self.assertEqual( m.hasObject(), False )
		self.assertEqual( m.hasChild("a"), False )
		self.assertEqual( m.hasChild("t"), True )
		self.assertEqual( m.childNames(), [ "t" ] )
		self.assertEqual( m.numBoundSamples(), 1 )
		self.assertEqual( m.readBoundAtSample(0), IECore.Box3d( IECore.V3d( 0, -1, -1 ), IECore.V3d( 2, 1, 1 ) ) )
		self.assertEqual( m.readBound(0.0), IECore.Box3d( IECore.V3d( 0, -1, -1 ), IECore.V3d( 2, 1, 1 ) ) )
		self.assertEqual( m.numTransformSamples(), 1 )
		self.assertEqual( m.readTransformAtSample(0), IECore.M44dData(IECore.M44d()) )
		self.assertEqual( m.readTransform(0.0), IECore.M44dData(IECore.M44d()) )
		self.assertEqual( m.hasObject(), False )
		
		self.assertEqual( m.readAttribute( "w", 0 ), IECore.BoolData( True ) )
		
		t = m.child( "t" )
		
		self.assertEqual( t.pathAsString(), "/t" )
		self.assertEqual( t.name(), "t" )
		self.assertEqual( t.hasObject(), False )
		self.assertEqual( t.hasChild("t"), False )
		self.assertEqual( t.hasChild("s"), True )
		self.assertEqual( t.childNames(), [ "s" ] )
		self.assertEqual( t.readBound(0.0), IECore.Box3d( IECore.V3d( -1, -1, -1 ), IECore.V3d( 1, 1, 1 ) ) )
		self.assertEqual( t.readTransform(0.0), IECore.M44dData(IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) )) )
		self.assertEqual( t.hasObject(), False )

		self.assertEqual( t.readAttribute( "wuh", 0 ), IECore.BoolData( True ) )
		
		s = t.child( "s" )
		
		self.assertEqual( s.pathAsString(), "/t/s" )
		self.assertEqual( s.name(), "s" )
		self.assertEqual( s.hasObject(), True )
		self.assertEqual( s.childNames(), [] )
		self.assertEqual( s.readBound(0.0), IECore.Box3d( IECore.V3d( -1, -1, -1 ), IECore.V3d( 1, 1, 1 ) ) )
		self.assertEqual( s.readTransform(0.0), IECore.M44dData(IECore.M44d.createTranslated( IECore.V3d( 0, 0, 0 ) )) )
		self.assertEqual( s.hasObject(), True )
		self.assertEqual( s.readObject(0.0), IECore.SpherePrimitive( 1 ) )
	
		self.assertEqual( s.readAttribute( "glah", 0 ), IECore.BoolData( True ) )

	def testAnimatedAttributes( self ) :

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		m.writeAttribute( "w", IECore.BoolData( True ), 1.0 )
		m.writeAttribute( "w", IECore.BoolData( False ), 2.0 )
		del m
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.numAttributeSamples('w'), 2 )
		self.assertEqual( m.readAttributeAtSample( "w", 0 ), IECore.BoolData( True ) )
		self.assertEqual( m.readAttributeAtSample( "w", 1 ), IECore.BoolData( False ) )
		self.assertEqual( m.readAttribute( "w", 1 ), IECore.BoolData( True ) )
		self.assertEqual( m.readAttribute( "w", 2 ), IECore.BoolData( False ) )

	@staticmethod
	def compareBBox( box1, box2 ):
		errorTolerance = IECore.V3d(1e-5, 1e-5, 1e-5)
		boxTmp = IECore.Box3d( box1.min - errorTolerance, box1.max + errorTolerance )
		if not boxTmp.contains( box2 ):
			return False
		boxTmp = IECore.Box3d( box2.min - errorTolerance, box2.max + errorTolerance )
		if not boxTmp.contains( box1 ):
			return False
		return True
		
	def testRandomStaticHierarchy( self ) :
	
		r = IECore.Rand48()
				
		def writeWalk( m ) :
					
			if m.name() != "/" :
				if r.nextf( 0.0, 1.0 ) < 0.5 :
					m.writeObject( IECore.SpherePrimitive( r.nextf( 1.0, 4.0 ) ), 0.0 )
		
				if r.nextf( 0.0, 1.0 ) < 0.5 :
					m.writeTransform( IECore.M44dData(IECore.M44d.createTranslated( r.nextV3d() )), 0.0 )
		
			thisDepth = int( r.nextf( 1, 4 ) )
			if thisDepth > len(m.path()) :
				numChildren = int( r.nextf( 4, 50 ) )
				for i in range( 0, numChildren ) :
					mc = m.createChild( str( i ) )
					writeWalk( mc )
		
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		writeWalk( m )
		del m

		def readWalk( m, parentSpaceBound ) :
					
			localSpaceBound = IECore.Box3d()
			for childName in m.childNames() :
				readWalk( m.child( childName ), localSpaceBound )
				
			if m.hasObject() :
				o = m.readObject(0.0)
				ob = o.bound()
				ob = IECore.Box3d( IECore.V3d( *ob.min ), IECore.V3d( *ob.max ) )
				localSpaceBound.extendBy( ob ) 

			fileBound = m.readBound(0.0)

			# the two bounding boxes should be pretty tightly close!
			self.failUnless( SceneCacheTest.compareBBox( localSpaceBound, fileBound ) )
			
			transformedBound = localSpaceBound.transform( m.readTransformAsMatrix(0.0) )
			parentSpaceBound.extendBy( transformedBound )

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		readWalk( m, IECore.Box3d() )
								
	def testMissingReadableChild( self ) :
	
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		m.createChild( "a" )
		del m
		
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		m.child( "a", IECore.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, m.child, "b" )
		self.assertEqual( None, m.child( "b", IECore.SceneInterface.MissingBehaviour.NullIfMissing ) )
	
	def testMissingScene( self ) :
	
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		b = a.createChild( "b" )
		c = b.createChild( "c" )
		del m, a, b, c
		
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		a = m.scene( [ "a" ] )
		b = m.scene( [ "a", "b" ] )
		self.assertEqual( b.path(), a.child( "b" ).path() )
		c = m.scene( [ "a", "b", "c" ] )
		self.assertEqual( c.path(), b.child( "c" ).path() )
		
		self.assertRaises( RuntimeError, m.scene, [ "a", "d" ] )
		self.assertEqual( None, m.scene( [ "a", "d" ], IECore.SceneInterface.MissingBehaviour.NullIfMissing ) )
	
	def testExplicitBoundOverridesImplicitBound( self ) :
			
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		a.writeBound( IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 10 ) ), 0.0 )
		a.writeObject( IECore.SpherePrimitive( 0.1 ), 0.0 )
		
		b = a.createChild( "b" )
		b.writeObject( IECore.SpherePrimitive( 100 ), 0.0 )
		
		del m, a, b
		
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		
		a = m.child( "a" )
		self.assertEqual( a.readBound(0.0), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 10 ) ) )
		
		b = a.child( "b" )
		self.assertEqual( b.readBound(0.0), IECore.Box3d( IECore.V3d( -100 ), IECore.V3d( 100 ) ) )
	
	def testExplicitBoundPropagatesToImplicitBound( self ) :
			
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		
		a = m.createChild( "a" )
				
		b = a.createChild( "b" )
		b.writeBound( IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ), 0.0 )
		b.writeObject( IECore.SpherePrimitive( 100 ), 0.0 )
		
		# destroys reference to the write SceneCache handles to close the file
		del m, a, b
		
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.readBound(0.0), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		
		a = m.child( "a" )
		self.assertEqual( a.readBound(0.0), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		
		b = a.child( "b" )
		self.assertEqual( b.readBound(0.0), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
	
	def testWriteMultiObjects( self ) :
		
		sc = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		
		t = sc.createChild( "transform" )
		t.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ), 0.0 )
		
		s = t.createChild( "shape" )
		s.writeObject( IECore.SpherePrimitive( 10 ), 0.0 )
		
		c = t.createChild( "camera" )
		
		# looks like an early version crashes here:
		c.writeObject( IECore.Camera(), 0.0 )
	
	def testWriteNullPointers( self ) :

		sc = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		t = sc.createChild( "transform" )
		self.assertRaises( RuntimeError, t.writeAttribute, "a", None, 0 )
		self.assertRaises( RuntimeError, t.writeObject, None, 0 )
		self.assertRaises( RuntimeError, t.writeTransform, None, 0 )				

	def testWritingOnFlushedFiles( self ) :

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		b = a.createChild( "b" )
		b.writeObject( IECore.SpherePrimitive( 100 ), 0.0 )
		# removes root scene handle, which flushes samples to disk and computes bounding box.
		del m
		# after this, no modification on children should be allowed.
		self.assertRaises( RuntimeError, b.writeObject, IECore.SpherePrimitive( 100 ), 0.0 )
		self.assertRaises( RuntimeError, b.writeAttribute, "test", IECore.IntData( 100 ), 0.0 )
		self.assertRaises( RuntimeError, b.writeBound, IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ), 0.0 )
		self.assertRaises( RuntimeError, b.createChild, "c" )
		self.assertRaises( RuntimeError, b.child, "c", IECore.SceneInterface.MissingBehaviour.CreateIfMissing )

	def testStoredScene( self ):

		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.numBoundSamples(), 4 )
		self.assertEqual( m.boundSampleTime(0), 0.0 )
		self.assertEqual( m.boundSampleTime(1), 1.0 )
		self.assertEqual( m.boundSampleTime(2), 2.0 )
		self.assertEqual( m.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBound(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )
		self.assertEqual( m.boundSampleInterval(0), (0,0,0) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(1), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 3,3,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(2), IECore.Box3d( IECore.V3d( -2,-1,-2 ), IECore.V3d( 4,5,2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(3), IECore.Box3d( IECore.V3d( -3,-1,-3 ), IECore.V3d( 4,6,3 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBound(3), IECore.Box3d( IECore.V3d( -3,-1,-3 ), IECore.V3d( 4,6,3 ) ) ) )
		self.assertEqual( m.boundSampleInterval(3), (1.0,2,3) )
		self.assertEqual( m.boundSampleInterval(4), (0,3,3) )

		A = m.child("A")
		self.assertEqual( A.numBoundSamples(), 3 )
		self.assertEqual( A.boundSampleTime(0), 0.0 )
		self.assertEqual( A.boundSampleTime(1), 1.0 )
		self.assertEqual( A.boundSampleTime(2), 2.0 )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(2), IECore.Box3d(IECore.V3d( 0,-1,-1 ), IECore.V3d( 2,1,1 ) ) ) )
		a = A.child("a")
		self.assertEqual( a.numBoundSamples(), 1 )
		self.failUnless( SceneCacheTest.compareBBox( a.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1 ), IECore.V3d( 1 ) ) ) )
		B = m.child("B")
		self.assertEqual( B.numBoundSamples(), 4 )
		self.assertEqual( B.boundSampleTime(0), 0.0 )
		self.assertEqual( B.boundSampleTime(1), 1.0 )
		self.assertEqual( B.boundSampleTime(2), 2.0 )
		self.assertEqual( B.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(2), IECore.Box3d(IECore.V3d( -2,-1,-2 ), IECore.V3d( 2,3,2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(3), IECore.Box3d(IECore.V3d( -3,-2,-3 ), IECore.V3d( 3,4,3 ) ) ) )
		b = B.child("b")
		self.assertEqual( b.numBoundSamples(), 4 )
		self.assertEqual( b.boundSampleTime(0), 0.0 )
		self.assertEqual( b.boundSampleTime(1), 1.0 )
		self.assertEqual( b.boundSampleTime(2), 2.0 )
		self.assertEqual( b.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1 ), IECore.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1 ), IECore.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(2), IECore.Box3d(IECore.V3d( -2 ), IECore.V3d( 2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(3), IECore.Box3d(IECore.V3d( -3 ), IECore.V3d( 3 ) ) ) )

	def testUnionBoundsForAnimation( self ):

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		A = m.createChild( "A" )
		a = A.createChild( "a" )
		B = m.createChild( "B" )
		b = B.createChild( "b" )
		# time 0.0
		A.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ), 0.0 )
		a.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 0, 0, 0 ) ) ), 0.0 )
		a.writeObject( IECore.SpherePrimitive( 1 ), 0.0 )
		B.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 0, 1, 0 ) ) ), 0.0 )
		b.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 0, 0, 0 ) ) ), 0.0 )
		b.writeObject( IECore.SpherePrimitive( 1 ), 0.0 )
		# time 1.0
		A.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ), 1.0 )
		a.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 0, 0, 0 ) ) ), 1.0 )
		B.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 0, 2, 0 ) ) ), 1.0 )
		b.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 0, 0, 0 ) ) ), 1.0 )
		b.writeObject( IECore.SpherePrimitive( 1 ), 1.0 )
		# time 2.0
		a.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ), 2.0 )
		b.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 0, 1, 0 ) ) ), 2.0 )
		b.writeObject( IECore.SpherePrimitive( 2 ), 2.0 )
		# time 3.0
		b.writeObject( IECore.SpherePrimitive( 3 ), 3.0 )
		del m,A,a,B,b
		
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.numBoundSamples(), 4 )
		self.assertEqual( m.boundSampleTime(0), 0.0 )
		self.assertEqual( m.boundSampleTime(1), 1.0 )
		self.assertEqual( m.boundSampleTime(2), 2.0 )
		self.assertEqual( m.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBound(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )
		self.assertEqual( m.boundSampleInterval(0), (0,0,0) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(1), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 3,3,1 ) ) ) )
		self.assertEqual( m.readBoundAtSample(2), IECore.Box3d( IECore.V3d( -2,-1,-2 ), IECore.V3d( 4,5,2 ) ) )
		self.assertEqual( m.readBoundAtSample(3), IECore.Box3d( IECore.V3d( -3,-1,-3 ), IECore.V3d( 4,6,3 ) ) )
		self.assertEqual( m.readBound(3), IECore.Box3d( IECore.V3d( -3,-1,-3 ), IECore.V3d( 4,6,3 ) ) )
		self.assertEqual( m.boundSampleInterval(3), (1.0,2,3) )
		self.assertEqual( m.boundSampleInterval(4), (0,3,3) )

		A = m.child("A")
		self.assertEqual( A.numBoundSamples(), 3 )
		self.assertEqual( A.boundSampleTime(0), 0.0 )
		self.assertEqual( A.boundSampleTime(1), 1.0 )
		self.assertEqual( A.boundSampleTime(2), 2.0 )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(2), IECore.Box3d(IECore.V3d( 0,-1,-1 ), IECore.V3d( 2,1,1 ) ) ) )
		a = A.child("a")
		self.assertEqual( a.numBoundSamples(), 1 )
		self.assertEqual( a.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		B = m.child("B")
		self.assertEqual( B.numBoundSamples(), 4 )
		self.assertEqual( B.boundSampleTime(0), 0.0 )
		self.assertEqual( B.boundSampleTime(1), 1.0 )
		self.assertEqual( B.boundSampleTime(2), 2.0 )
		self.assertEqual( B.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(2), IECore.Box3d(IECore.V3d( -2,-1,-2 ), IECore.V3d( 2,3,2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(3), IECore.Box3d(IECore.V3d( -3,-2,-3 ), IECore.V3d( 3,4,3 ) ) ) )
		b = B.child("b")
		self.assertEqual( b.numBoundSamples(), 4 )
		self.assertEqual( b.boundSampleTime(0), 0.0 )
		self.assertEqual( b.boundSampleTime(1), 1.0 )
		self.assertEqual( b.boundSampleTime(2), 2.0 )
		self.assertEqual( b.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1 ), IECore.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1 ), IECore.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(2), IECore.Box3d(IECore.V3d( -2 ), IECore.V3d( 2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(3), IECore.Box3d(IECore.V3d( -3 ), IECore.V3d( 3 ) ) ) )

	def testExpandedBoundsForAnimation( self ):

		cube = IECore.Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" )()

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		a.writeObject( cube, 0.0 )

		a.writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d( 0, math.radians(0), 0 ) ) ), 0.0 )
		a.writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d( 0, math.radians(90), 0 ) ) ), 1.0 )
		a.writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d( 0, math.radians(180), 0 ) ) ), 2.0 )
		a.writeTransform( IECore.M44dData( IECore.M44d.createRotated( IECore.V3d( 0, math.radians(270), 0 ) ) ), 3.0 )

		del m,a

		cubeBound = IECore.Box3d( IECore.V3d( cube.bound().min ), IECore.V3d( cube.bound().max ) )
		errorTolerance = IECore.V3d(1e-5, 1e-5, 1e-5)
		
		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		a = m.child("a")
		self.assertEqual( a.numBoundSamples(), 1 )

		# the stored qube should have same bbox as the original qube.
		self.failUnless( SceneCacheTest.compareBBox( a.readBoundAtSample(0), cubeBound ) )

		self.assertEqual( m.numBoundSamples(), 4 )

		for t in xrange( 0, 30, 2 ):
			time = t / 10.0
			angle = time * math.radians(90)
			transformedBound = cubeBound.transform( IECore.M44d.createRotated( IECore.V3d( 0, angle, 0 ) ) )
			tmpBounds = m.readBound( time )
			tmpBounds.extendBy( IECore.Box3d( tmpBounds.min - errorTolerance, tmpBounds.max + errorTolerance ) )
			self.failUnless( tmpBounds.contains( transformedBound ) )	# interpolated bounding box must contain bounding box of interpolated rotation.
	
	def testAnimatedObjectAttributes( self ) :
		
		plane = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		box = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ) )
		box["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ IECore.Color3f( 1, 0, 0 ) ] * box.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) )
		box2 = box.copy()
		box2["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ IECore.Color3f( 0, 1, 0 ) ] * box.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) )
		
		s = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = s.createChild( "a" )
		b = a.createChild( "b" )
		c = a.createChild( "c" )
		d = a.createChild( "d" )
		
		# animated color
		b.writeObject( box, 0 )
		b.writeObject( box2, 1 )
		
		# static
		c.writeObject( box, 0 )
		c.writeObject( box, 1 )
		
		# animated topology
		d.writeObject( box, 0 )
		d.writeObject( plane, 1 )
		
		del s, a, b, c, d
		
		s = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		
		a = s.child( "a" )
		self.assertFalse( a.hasAttribute( "sceneInterface:animatedObjectTopology" ) )
		self.assertFalse( a.hasAttribute( "sceneInterface:animatedObjectPrimVars" ) )
		
		b = a.child( "b" )
		self.assertFalse( b.hasAttribute( "sceneInterface:animatedObjectTopology" ) )
		self.assertTrue( b.hasAttribute( "sceneInterface:animatedObjectPrimVars" ) )
		self.assertEqual( b.readAttribute( "sceneInterface:animatedObjectPrimVars", 0 ), IECore.InternedStringVectorData( [ "Cs" ] ) )
		
		c = a.child( "c" )
		self.assertFalse( c.hasAttribute( "sceneInterface:animatedObjectTopology" ) )
		self.assertTrue( c.hasAttribute( "sceneInterface:animatedObjectPrimVars" ) )
		self.assertEqual( c.readAttribute( "sceneInterface:animatedObjectPrimVars", 0 ), IECore.InternedStringVectorData() )
		
		d = a.child( "d" )
		self.assertTrue( d.hasAttribute( "sceneInterface:animatedObjectTopology" ) )
		self.assertEqual( d.readAttribute( "sceneInterface:animatedObjectTopology", 0 ), IECore.BoolData( True ) )
		self.assertFalse( d.hasAttribute( "sceneInterface:animatedObjectPrimVars" ) )

	def testObjectPrimitiveVariablesRead( self ) :
		
		box = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ) )
		box["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ IECore.Color3f( 1, 0, 0 ) ] * box.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) )
		box2 = box.copy()
		box2["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ IECore.Color3f( 0, 1, 0 ) ] * box.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) )

		s = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		b = s.createChild( "b" )
		b.writeObject( box, 0 )
		b.writeObject( box2, 1 )
		
		del s, b

		s = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		b = s.child( "b" )

		self.assertEqual( b.readObject(0)['P'], b.readObjectPrimitiveVariables(['P','Cs'], 0)['P'] )
		self.assertEqual( b.readObject(0)['Cs'], b.readObjectPrimitiveVariables(['P','Cs'], 0)['Cs'] )
		self.assertEqual( b.readObject(0.5)['P'], b.readObjectPrimitiveVariables(['P','Cs'], 0.5)['P'] )
		self.assertEqual( b.readObject(0.5)['Cs'], b.readObjectPrimitiveVariables(['P','Cs'], 0.5)['Cs'] )
		self.assertEqual( b.readObject(1)['P'], b.readObjectPrimitiveVariables(['P','Cs'], 1)['P'] )
		self.assertEqual( b.readObject(1)['Cs'], b.readObjectPrimitiveVariables(['P','Cs'], 1)['Cs'] )

	def testTags( self ) :

		sphere = IECore.SpherePrimitive( 1 )
		box = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ) )

		def testSet( values ):
			return set( map( lambda s: IECore.InternedString(s), values ) )

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		A = m.createChild( "A" )
		a = A.createChild( "a" )
		aa = a.createChild( "aa" )
		ab = a.createChild( "ab" )
		ab.writeObject( box, 0 )
		self.assertEqual( set( ab.readTags(includeChildren=False) ), testSet( [ "ObjectType:MeshPrimitive" ] ) )
		B = m.createChild( "B" )
		b = B.createChild( "b" )
		c = B.createChild( "c" )
		d = B.createChild( "d" )
		d.writeObject( sphere, 0 )
		self.assertEqual( set( d.readTags(includeChildren=False) ), testSet( [ "ObjectType:SpherePrimitive" ] ) )

		aa.writeTags( [ "t1" ] )
		self.assertEqual( set( aa.readTags(includeChildren=False) ), testSet( [ "t1" ] ) )
		self.assertRaises( RuntimeError, aa.readTags )
		aa.writeTags( [ "t1" ] )
		ab.writeTags( [ IECore.InternedString("t1") ] )
		ab.writeTags( [ IECore.InternedString("t2") ] )

		c.writeTags( [ "t3" ] )

		B.writeTags( [ "t4" ] )

		a.writeTags( [] )
		A.writeTags( [ "t1" ] )

		del m, A, a, aa, ab, B, b, c, d

		m = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		A = m.child("A")
		a = A.child("a")
		aa = a.child("aa")
		ab = a.child("ab")
		B = m.child("B")
		b = B.child("b")
		c = B.child("c")
		d = B.child("d")

		self.assertEqual( set( m.readTags() ), testSet( [ "t1", "t2", "t3", "t4", "ObjectType:MeshPrimitive", "ObjectType:SpherePrimitive" ] ) )
		self.assertEqual( set( m.readTags(includeChildren=False) ), testSet([]) )
		self.assertEqual( set( A.readTags() ), testSet( [ "t1", "t2", "ObjectType:MeshPrimitive" ] ) )
		self.assertEqual( set( A.readTags(includeChildren=False) ), testSet( [ "t1" ] ) )
		self.assertEqual( set( a.readTags() ), testSet( [ "t1", "t2", "ObjectType:MeshPrimitive" ] ) )
		self.assertEqual( set( aa.readTags() ), testSet( [ "t1" ] ) )
		self.assertEqual( set( aa.readTags(includeChildren=False) ), testSet(['t1']) )
		self.assertEqual( set( ab.readTags() ), testSet( [ "t1", "t2", "ObjectType:MeshPrimitive" ] ) )
		self.assertEqual( set( B.readTags() ), testSet( [ "t3", "t4", "ObjectType:SpherePrimitive" ] ) )
		self.assertEqual( set( B.readTags(includeChildren=False) ), testSet(['t4']) )
		self.assertEqual( set( b.readTags() ), testSet( [] ) )
		self.assertEqual( set( c.readTags() ), testSet( [ "t3" ] ) )
		self.assertEqual( set( d.readTags() ), testSet( [ "ObjectType:SpherePrimitive" ] ) )

		self.assertTrue( m.hasTag( "t1" ) )
		self.assertTrue( m.hasTag( "t4" ) )
		self.assertFalse( m.hasTag( "t1", includeChildren = False ) )
		self.assertFalse( m.hasTag( "t4", includeChildren = False ) )
		self.assertFalse( m.hasTag( "t5" ) )
		self.assertFalse( m.hasTag( "t5", includeChildren = False ) )
		self.assertTrue( ab.hasTag( "ObjectType:MeshPrimitive" ) )
		self.assertTrue( B.hasTag( "t4" ) )
		self.assertTrue( B.hasTag( "t4", includeChildren = False ) )
		self.assertFalse( B.hasTag( "t1" ) )
		self.assertTrue( B.hasTag( "t3" ) )
		self.assertTrue( B.hasTag( "ObjectType:SpherePrimitive" ) )
		self.assertTrue( d.hasTag( "ObjectType:SpherePrimitive" ) )

if __name__ == "__main__":
	unittest.main()

