##########################################################################
#
#  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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
import shutil

import IECore
import IECoreScene

import imath

class SceneCacheTest( unittest.TestCase ) :

	def testSupportedExtension( self ) :
		self.assertTrue( "scc" in IECoreScene.SceneInterface.supportedExtensions() )
		self.assertTrue( "scc" in IECoreScene.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Read ) )
		self.assertTrue( "scc" in IECoreScene.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Write ) )
		self.assertTrue( "scc" in IECoreScene.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Write + IECore.IndexedIO.OpenMode.Read ) )
		self.assertFalse( "scc" in IECoreScene.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Append ) )

	def testFactoryFunction( self ) :
		# test Write factory function
		m = IECoreScene.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		self.assertTrue( isinstance( m, IECoreScene.SceneCache ) )
		self.assertEqual( m.fileName(), "/tmp/test.scc" )
		self.assertRaises( RuntimeError, m.readBound, 0.0 )
		del m
		# test Read factory function
		m = IECoreScene.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( isinstance( m, IECoreScene.SceneCache ) )
		self.assertEqual( m.fileName(), "/tmp/test.scc" )
		m.readBound( 0.0 )

	def testAppendRaises( self ) :
		self.assertRaises( RuntimeError, IECoreScene.SceneCache, "/tmp/test.scc", IECore.IndexedIO.OpenMode.Append )

	def testReadNonExistentRaises( self ) :
		self.assertRaises( RuntimeError, IECoreScene.SceneCache, "iDontExist.scc", IECore.IndexedIO.OpenMode.Read )

	def testKnownStaticHierarchy( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
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

		t.writeTransform( IECore.M44dData(imath.M44d().translate(imath.V3d( 1, 0, 0 ))), 1.0 )
		self.assertEqual( t.hasObject(), False )

		t.writeAttribute( "wuh", IECore.BoolData( True ), 1.0 )

		s = t.createChild( "s" )
		self.assertEqual( s.path(), ["t","s"] )
		self.assertEqual( s.pathAsString(), "/t/s" )
		self.assertEqual( s.name(), "s" )
		self.assertEqual( s.hasObject(), False )
		self.assertEqual( t.childNames(), [ "s" ] )

		s.writeObject( IECoreScene.SpherePrimitive( 1 ), 1.0 )
		self.assertEqual( s.hasObject(), True )

		s.writeAttribute( "glah", IECore.BoolData( True ), 1.0 )

		# need to delete all the SceneCache references to finalise the file
		del m, t, s

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( m.pathAsString(), "/" )
		self.assertEqual( m.name(), "/" )
		self.assertEqual( m.hasObject(), False )
		self.assertEqual( m.hasChild("a"), False )
		self.assertEqual( m.hasChild("t"), True )
		self.assertEqual( m.childNames(), [ "t" ] )
		self.assertEqual( m.numBoundSamples(), 1 )
		self.assertEqual( m.readBoundAtSample(0), imath.Box3d( imath.V3d( 0, -1, -1 ), imath.V3d( 2, 1, 1 ) ) )
		self.assertEqual( m.readBound(0.0), imath.Box3d( imath.V3d( 0, -1, -1 ), imath.V3d( 2, 1, 1 ) ) )
		self.assertEqual( m.numTransformSamples(), 1 )
		self.assertEqual( m.readTransformAtSample(0), IECore.M44dData(imath.M44d()) )
		self.assertEqual( m.readTransform(0.0), IECore.M44dData(imath.M44d()) )
		self.assertEqual( m.hasObject(), False )

		self.assertEqual( m.readAttribute( "w", 0 ), IECore.BoolData( True ) )

		t = m.child( "t" )

		self.assertEqual( t.pathAsString(), "/t" )
		self.assertEqual( t.name(), "t" )
		self.assertEqual( t.hasObject(), False )
		self.assertEqual( t.hasChild("t"), False )
		self.assertEqual( t.hasChild("s"), True )
		self.assertEqual( t.childNames(), [ "s" ] )
		self.assertEqual( t.readBound(0.0), imath.Box3d( imath.V3d( -1, -1, -1 ), imath.V3d( 1, 1, 1 ) ) )
		self.assertEqual( t.readTransform(0.0), IECore.M44dData(imath.M44d().translate( imath.V3d( 1, 0, 0 ) )) )
		self.assertEqual( t.hasObject(), False )

		self.assertEqual( t.readAttribute( "wuh", 0 ), IECore.BoolData( True ) )

		s = t.child( "s" )

		self.assertEqual( s.pathAsString(), "/t/s" )
		self.assertEqual( s.name(), "s" )
		self.assertEqual( s.hasObject(), True )
		self.assertEqual( s.childNames(), [] )
		self.assertEqual( s.readBound(0.0), imath.Box3d( imath.V3d( -1, -1, -1 ), imath.V3d( 1, 1, 1 ) ) )
		self.assertEqual( s.readTransform(0.0), IECore.M44dData(imath.M44d().translate( imath.V3d( 0, 0, 0 ) )) )
		self.assertEqual( s.hasObject(), True )
		self.assertEqual( s.readObject(0.0), IECoreScene.SpherePrimitive( 1 ) )

		self.assertEqual( s.readAttribute( "glah", 0 ), IECore.BoolData( True ) )

	def testAnimatedAttributes( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		m.writeAttribute( "w", IECore.BoolData( True ), 1.0 )
		m.writeAttribute( "w", IECore.BoolData( False ), 2.0 )
		del m
		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.numAttributeSamples('w'), 2 )
		self.assertEqual( m.readAttributeAtSample( "w", 0 ), IECore.BoolData( True ) )
		self.assertEqual( m.readAttributeAtSample( "w", 1 ), IECore.BoolData( False ) )
		self.assertEqual( m.readAttribute( "w", 1 ), IECore.BoolData( True ) )
		self.assertEqual( m.readAttribute( "w", 2 ), IECore.BoolData( False ) )

	@staticmethod
	def compareBBox( box1, box2 ):
		errorTolerance = imath.V3d(1e-5, 1e-5, 1e-5)
		boxTmp = imath.Box3d( box1.min() - errorTolerance, box1.max() + errorTolerance )
		if not IECore.BoxAlgo.contains( boxTmp, box2 ):
			return False
		boxTmp = imath.Box3d( box2.min() - errorTolerance, box2.max() + errorTolerance )
		if not IECore.BoxAlgo.contains( boxTmp, box1 ):
			return False
		return True

	def testRandomStaticHierarchy( self ) :

		r = imath.Rand48()

		def writeWalk( m ) :

			if m.name() != "/" :
				if r.nextf( 0.0, 1.0 ) < 0.5 :
					m.writeObject( IECoreScene.SpherePrimitive( r.nextf( 1.0, 4.0 ) ), 0.0 )

				if r.nextf( 0.0, 1.0 ) < 0.5 :
					t = imath.V3d( r.nextf(), r.nextf(), r.nextf() )
					m.writeTransform( IECore.M44dData( imath.M44d().translate( t ) ), 0.0 )

			thisDepth = int( r.nextf( 1, 4 ) )
			if thisDepth > len(m.path()) :
				numChildren = int( r.nextf( 4, 50 ) )
				for i in range( 0, numChildren ) :
					mc = m.createChild( str( i ) )
					writeWalk( mc )

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		writeWalk( m )
		del m

		def readWalk( m, parentSpaceBound ) :

			localSpaceBound = imath.Box3d()
			for childName in m.childNames() :
				readWalk( m.child( childName ), localSpaceBound )

			if m.hasObject() :
				o = m.readObject(0.0)
				ob = o.bound()
				ob = imath.Box3d( imath.V3d( *ob.min() ), imath.V3d( *ob.max() ) )
				localSpaceBound.extendBy( ob )

			fileBound = m.readBound(0.0)

			# the two bounding boxes should be pretty tightly close!
			self.failUnless( SceneCacheTest.compareBBox( localSpaceBound, fileBound ) )

			transformedBound = localSpaceBound * m.readTransformAsMatrix(0.0)
			parentSpaceBound.extendBy( transformedBound )

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		readWalk( m, imath.Box3d() )

	def testMissingReadableChild( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		m.createChild( "a" )
		del m

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		m.child( "a", IECoreScene.SceneInterface.MissingBehaviour.ThrowIfMissing )
		self.assertRaises( RuntimeError, m.child, "b" )
		self.assertEqual( None, m.child( "b", IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ) )

	def testMissingScene( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		b = a.createChild( "b" )
		c = b.createChild( "c" )
		del m, a, b, c

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		a = m.scene( [ "a" ] )
		b = m.scene( [ "a", "b" ] )
		self.assertEqual( b.path(), a.child( "b" ).path() )
		c = m.scene( [ "a", "b", "c" ] )
		self.assertEqual( c.path(), b.child( "c" ).path() )

		self.assertRaises( RuntimeError, m.scene, [ "a", "d" ] )
		self.assertEqual( None, m.scene( [ "a", "d" ], IECoreScene.SceneInterface.MissingBehaviour.NullIfMissing ) )

	def testExplicitBoundDilatesImplicitBound( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		a.writeBound( imath.Box3d( imath.V3d( -200 ), imath.V3d( 10 ) ), 0.0 )
		a.writeBound( imath.Box3d( imath.V3d( -300 ), imath.V3d( 10 ) ), 1.0 )
		a.writeObject( IECoreScene.SpherePrimitive( 0.1 ), 0.0 )

		b = a.createChild( "b" )
		b.writeObject( IECoreScene.SpherePrimitive( 100 ), 0.0 )
		b.writeObject( IECoreScene.SpherePrimitive( 50 ), 0.5 )

		del m, a, b

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )

		a = m.child( "a" )
		self.assertEqual( a.readBound(0.0), imath.Box3d( imath.V3d( -200 ), imath.V3d( 100 ) ) )

		# this one should be the union of ( -250, -250, -250 ) ( 10, 10, 10 ) and (-50 -50 -50) (50 50 50)
		self.assertEqual( a.readBound(0.5), imath.Box3d( imath.V3d( -250 ), imath.V3d( 50 ) ) )

		self.assertEqual( a.readBound(1.0), imath.Box3d( imath.V3d( -300 ), imath.V3d( 50 ) ) )

		b = a.child( "b" )
		self.assertEqual( b.readBound(0.0), imath.Box3d( imath.V3d( -100 ), imath.V3d( 100 ) ) )
		self.assertEqual( b.readBound(0.5), imath.Box3d( imath.V3d( -50 ), imath.V3d( 50 ) ) )


	def testAnimatedBoundPropagation( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )

		# write some interleaved animated samples:
		p1 = m.createChild( "p1" )

		a = p1.createChild( "a" )
		a.writeBound( imath.Box3d( imath.V3d( -4 ), imath.V3d( 0 ) ), -2.5 )
		a.writeBound( imath.Box3d( imath.V3d( -3 ), imath.V3d( 0 ) ), -1.5 )
		a.writeBound( imath.Box3d( imath.V3d( -2 ), imath.V3d( 0 ) ), -0.5 )
		a.writeBound( imath.Box3d( imath.V3d( -1 ), imath.V3d( 0 ) ), 0.5 )

		b = a.createChild( "b" )
		b.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 1 ) ), -1.0 )
		b.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 2 ) ), 0.0 )
		b.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 3 ) ), 1.0 )
		b.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 4 ) ), 2.0 )

		p2 = m.createChild( "p2" )

		a = p2.createChild( "a" )
		a.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 1 ) ), -1.0 )
		a.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 2 ) ), 0.0 )
		a.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 3 ) ), 1.0 )
		a.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 4 ) ), 2.0 )

		b = a.createChild( "b" )
		b.writeBound( imath.Box3d( imath.V3d( -4 ), imath.V3d( 0 ) ), -2.5 )
		b.writeBound( imath.Box3d( imath.V3d( -3 ), imath.V3d( 0 ) ), -1.5 )
		b.writeBound( imath.Box3d( imath.V3d( -2 ), imath.V3d( 0 ) ), -0.5 )
		b.writeBound( imath.Box3d( imath.V3d( -1 ), imath.V3d( 0 ) ), 0.5 )


		# non interleaved with a coincident sample:
		p3 = m.createChild( "p3" )
		a = p3.createChild( "a" )
		a.writeBound( imath.Box3d( imath.V3d( -4 ), imath.V3d( 0 ) ), -2.5 )
		a.writeBound( imath.Box3d( imath.V3d( -3 ), imath.V3d( 0 ) ), -1.5 )
		a.writeBound( imath.Box3d( imath.V3d( -2 ), imath.V3d( 0 ) ), 0 )

		b = p3.createChild( "b" )
		b.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 1 ) ), 0 )
		b.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 2 ) ), 1.5 )
		b.writeBound( imath.Box3d( imath.V3d( 0 ), imath.V3d( 3 ) ), 2.5 )

		del m, a, b, p1, p2, p3

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		p1 = m.child( "p1" )
		p2 = m.child( "p2" )
		p3 = m.child( "p3" )

		for t in [-2.5, -1.5, -1, -0.5, 0, 0.5, 1, 2]:
			self.assertEqual( p1.readBound( t ), p2.readBound( t ) )

		self.assertEqual( p1.readBound( -2.5 ), imath.Box3d( imath.V3d( -4 ), imath.V3d( 1 ) ) )
		self.assertEqual( p1.readBound( -1.5 ), imath.Box3d( imath.V3d( -3 ), imath.V3d( 1 ) ) )
		self.assertEqual( p1.readBound( -1.0 ), imath.Box3d( imath.V3d( -2.5 ), imath.V3d( 1 ) ) )
		self.assertEqual( p1.readBound( -0.5 ), imath.Box3d( imath.V3d( -2 ), imath.V3d( 1.5 ) ) )
		self.assertEqual( p1.readBound(  0.0 ), imath.Box3d( imath.V3d( -1.5 ), imath.V3d( 2 ) ) )
		self.assertEqual( p1.readBound(  0.5 ), imath.Box3d( imath.V3d( -1 ), imath.V3d( 2.5 ) ) )
		self.assertEqual( p1.readBound(  1.0 ), imath.Box3d( imath.V3d( -1 ), imath.V3d( 3 ) ) )
		self.assertEqual( p1.readBound(  2.0 ), imath.Box3d( imath.V3d( -1 ), imath.V3d( 4 ) ) )


		self.assertEqual( p3.readBound( -2.5 ), imath.Box3d( imath.V3d( -4 ), imath.V3d( 1 ) ) )
		self.assertEqual( p3.readBound( -1.5 ), imath.Box3d( imath.V3d( -3 ), imath.V3d( 1 ) ) )
		self.assertEqual( p3.readBound(    0 ), imath.Box3d( imath.V3d( -2 ), imath.V3d( 1 ) ) )
		self.assertEqual( p3.readBound(  1.5 ), imath.Box3d( imath.V3d( -2 ), imath.V3d( 2 ) ) )
		self.assertEqual( p3.readBound(  2.5 ), imath.Box3d( imath.V3d( -2 ), imath.V3d( 3 ) ) )


	def testExplicitBoundPropagatesToImplicitBound( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )

		# This hierarchy has a leaf with a small primitive and a large explicit bound.
		# The explicit bound should win as it's bigger:
		a = m.createChild( "a" )

		b = a.createChild( "b" )
		b.writeBound( imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) ), 0.0 )
		b.writeObject( IECoreScene.SpherePrimitive( 0.1 ), 0.0 )


		# This hierarchy has a leaf with a large primitive and a smaller explicit bound.
		# The primitive's bound should win in this case :
		c = m.createChild( "c" )
		c.writeBound( imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) ), 0.0 )

		d = c.createChild( "d" )
		d.writeBound( imath.Box3d( imath.V3d( -2 ), imath.V3d( 2 ) ), 0.0 )
		d.writeObject( IECoreScene.SpherePrimitive( 100 ), 0.0 )

		# destroys reference to the write SceneCache handles to close the file
		del m, a, b, c, d

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.readBound(0.0), imath.Box3d( imath.V3d( -100 ), imath.V3d( 100 ) ) )

		a = m.child( "a" )
		self.assertEqual( a.readBound(0.0), imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) ) )

		b = a.child( "b" )
		self.assertEqual( b.readBound(0.0), imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) ) )

		c = m.child( "c" )
		self.assertEqual( c.readBound(0.0), imath.Box3d( imath.V3d( -100 ), imath.V3d( 100 ) ) )

		d = c.child( "d" )
		self.assertEqual( d.readBound(0.0), imath.Box3d( imath.V3d( -100 ), imath.V3d( 100 ) ) )



	def testWriteMultiObjects( self ) :

		sc = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )

		t = sc.createChild( "transform" )
		t.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 1, 0, 0 ) ) ), 0.0 )

		s = t.createChild( "shape" )
		s.writeObject( IECoreScene.SpherePrimitive( 10 ), 0.0 )

		c = t.createChild( "camera" )

		# looks like an early version crashes here:
		c.writeObject( IECoreScene.Camera(), 0.0 )

	def testWriteNullPointers( self ) :

		sc = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		t = sc.createChild( "transform" )
		self.assertRaises( RuntimeError, t.writeAttribute, "a", None, 0 )
		self.assertRaises( RuntimeError, t.writeObject, None, 0 )
		self.assertRaises( RuntimeError, t.writeTransform, None, 0 )

	def testWritingOnFlushedFiles( self ) :

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		b = a.createChild( "b" )
		b.writeObject( IECoreScene.SpherePrimitive( 100 ), 0.0 )
		# removes root scene handle, which flushes samples to disk and computes bounding box.
		del m
		# after this, no modification on children should be allowed.
		self.assertRaises( RuntimeError, b.writeObject, IECoreScene.SpherePrimitive( 100 ), 0.0 )
		self.assertRaises( RuntimeError, b.writeAttribute, "test", IECore.IntData( 100 ), 0.0 )
		self.assertRaises( RuntimeError, b.writeBound, imath.Box3d( imath.V3d( -1 ), imath.V3d( 1 ) ), 0.0 )
		self.assertRaises( RuntimeError, b.createChild, "c" )
		self.assertRaises( RuntimeError, b.child, "c", IECoreScene.SceneInterface.MissingBehaviour.CreateIfMissing )

	def testStoredScene( self ):

		m = IECoreScene.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.numBoundSamples(), 4 )
		self.assertEqual( m.boundSampleTime(0), 0.0 )
		self.assertEqual( m.boundSampleTime(1), 1.0 )
		self.assertEqual( m.boundSampleTime(2), 2.0 )
		self.assertEqual( m.boundSampleTime(3), 3.0 )
		self.assertTrue( m.hasBound() )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(0), imath.Box3d( imath.V3d( -1,-1,-1 ), imath.V3d( 2,2,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBound(0), imath.Box3d( imath.V3d( -1,-1,-1 ), imath.V3d( 2,2,1 ) ) ) )
		self.assertEqual( m.boundSampleInterval(0), (0,0,0) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(1), imath.Box3d( imath.V3d( -1,-1,-1 ), imath.V3d( 3,3,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(2), imath.Box3d( imath.V3d( -2,-1,-2 ), imath.V3d( 4,5,2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(3), imath.Box3d( imath.V3d( -3,-1,-3 ), imath.V3d( 4,6,3 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBound(3), imath.Box3d( imath.V3d( -3,-1,-3 ), imath.V3d( 4,6,3 ) ) ) )
		self.assertEqual( m.boundSampleInterval(3), (1.0,2,3) )
		self.assertEqual( m.boundSampleInterval(4), (0,3,3) )

		A = m.child("A")
		self.assertTrue( A.hasBound() )
		self.assertEqual( A.numBoundSamples(), 3 )
		self.assertEqual( A.boundSampleTime(0), 0.0 )
		self.assertEqual( A.boundSampleTime(1), 1.0 )
		self.assertEqual( A.boundSampleTime(2), 2.0 )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(0), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(1), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(2), imath.Box3d(imath.V3d( 0,-1,-1 ), imath.V3d( 2,1,1 ) ) ) )
		a = A.child("a")
		self.assertEqual( a.numBoundSamples(), 1 )
		self.failUnless( SceneCacheTest.compareBBox( a.readBoundAtSample(0), imath.Box3d(imath.V3d( -1 ), imath.V3d( 1 ) ) ) )
		B = m.child("B")
		self.assertTrue( B.hasBound() )
		self.assertEqual( B.numBoundSamples(), 4 )
		self.assertEqual( B.boundSampleTime(0), 0.0 )
		self.assertEqual( B.boundSampleTime(1), 1.0 )
		self.assertEqual( B.boundSampleTime(2), 2.0 )
		self.assertEqual( B.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(0), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(1), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(2), imath.Box3d(imath.V3d( -2,-1,-2 ), imath.V3d( 2,3,2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(3), imath.Box3d(imath.V3d( -3,-2,-3 ), imath.V3d( 3,4,3 ) ) ) )
		b = B.child("b")
		self.assertTrue( b.hasBound() )
		self.assertEqual( b.numBoundSamples(), 4 )
		self.assertEqual( b.boundSampleTime(0), 0.0 )
		self.assertEqual( b.boundSampleTime(1), 1.0 )
		self.assertEqual( b.boundSampleTime(2), 2.0 )
		self.assertEqual( b.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(0), imath.Box3d(imath.V3d( -1 ), imath.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(1), imath.Box3d(imath.V3d( -1 ), imath.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(2), imath.Box3d(imath.V3d( -2 ), imath.V3d( 2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(3), imath.Box3d(imath.V3d( -3 ), imath.V3d( 3 ) ) ) )

	def testUnionBoundsForAnimation( self ):

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		A = m.createChild( "A" )
		a = A.createChild( "a" )
		B = m.createChild( "B" )
		b = B.createChild( "b" )
		# time 0.0
		A.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 1, 0, 0 ) ) ), 0.0 )
		a.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 0, 0, 0 ) ) ), 0.0 )
		a.writeObject( IECoreScene.SpherePrimitive( 1 ), 0.0 )
		B.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 0, 1, 0 ) ) ), 0.0 )
		b.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 0, 0, 0 ) ) ), 0.0 )
		b.writeObject( IECoreScene.SpherePrimitive( 1 ), 0.0 )
		# time 1.0
		A.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 2, 0, 0 ) ) ), 1.0 )
		a.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 0, 0, 0 ) ) ), 1.0 )
		B.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 0, 2, 0 ) ) ), 1.0 )
		b.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 0, 0, 0 ) ) ), 1.0 )
		b.writeObject( IECoreScene.SpherePrimitive( 1 ), 1.0 )
		# time 2.0
		a.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 1, 0, 0 ) ) ), 2.0 )
		b.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 0, 1, 0 ) ) ), 2.0 )
		b.writeObject( IECoreScene.SpherePrimitive( 2 ), 2.0 )
		# time 3.0
		b.writeObject( IECoreScene.SpherePrimitive( 3 ), 3.0 )
		del m,A,a,B,b

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.numBoundSamples(), 4 )
		self.assertEqual( m.boundSampleTime(0), 0.0 )
		self.assertEqual( m.boundSampleTime(1), 1.0 )
		self.assertEqual( m.boundSampleTime(2), 2.0 )
		self.assertEqual( m.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(0), imath.Box3d( imath.V3d( -1,-1,-1 ), imath.V3d( 2,2,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBound(0), imath.Box3d( imath.V3d( -1,-1,-1 ), imath.V3d( 2,2,1 ) ) ) )
		self.assertEqual( m.boundSampleInterval(0), (0,0,0) )
		self.failUnless( SceneCacheTest.compareBBox( m.readBoundAtSample(1), imath.Box3d( imath.V3d( -1,-1,-1 ), imath.V3d( 3,3,1 ) ) ) )
		self.assertEqual( m.readBoundAtSample(2), imath.Box3d( imath.V3d( -2,-1,-2 ), imath.V3d( 4,5,2 ) ) )
		self.assertEqual( m.readBoundAtSample(3), imath.Box3d( imath.V3d( -3,-1,-3 ), imath.V3d( 4,6,3 ) ) )
		self.assertEqual( m.readBound(3), imath.Box3d( imath.V3d( -3,-1,-3 ), imath.V3d( 4,6,3 ) ) )
		self.assertEqual( m.boundSampleInterval(3), (1.0,2,3) )
		self.assertEqual( m.boundSampleInterval(4), (0,3,3) )

		A = m.child("A")
		self.assertEqual( A.numBoundSamples(), 3 )
		self.assertEqual( A.boundSampleTime(0), 0.0 )
		self.assertEqual( A.boundSampleTime(1), 1.0 )
		self.assertEqual( A.boundSampleTime(2), 2.0 )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(0), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(1), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( A.readBoundAtSample(2), imath.Box3d(imath.V3d( 0,-1,-1 ), imath.V3d( 2,1,1 ) ) ) )
		a = A.child("a")
		self.assertEqual( a.numBoundSamples(), 1 )
		self.assertEqual( a.readBoundAtSample(0), imath.Box3d(imath.V3d( -1 ), imath.V3d( 1 ) ) )
		B = m.child("B")
		self.assertEqual( B.numBoundSamples(), 4 )
		self.assertEqual( B.boundSampleTime(0), 0.0 )
		self.assertEqual( B.boundSampleTime(1), 1.0 )
		self.assertEqual( B.boundSampleTime(2), 2.0 )
		self.assertEqual( B.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(0), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(1), imath.Box3d(imath.V3d( -1,-1,-1 ), imath.V3d( 1,1,1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(2), imath.Box3d(imath.V3d( -2,-1,-2 ), imath.V3d( 2,3,2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( B.readBoundAtSample(3), imath.Box3d(imath.V3d( -3,-2,-3 ), imath.V3d( 3,4,3 ) ) ) )
		b = B.child("b")
		self.assertEqual( b.numBoundSamples(), 4 )
		self.assertEqual( b.boundSampleTime(0), 0.0 )
		self.assertEqual( b.boundSampleTime(1), 1.0 )
		self.assertEqual( b.boundSampleTime(2), 2.0 )
		self.assertEqual( b.boundSampleTime(3), 3.0 )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(0), imath.Box3d(imath.V3d( -1 ), imath.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(1), imath.Box3d(imath.V3d( -1 ), imath.V3d( 1 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(2), imath.Box3d(imath.V3d( -2 ), imath.V3d( 2 ) ) ) )
		self.failUnless( SceneCacheTest.compareBBox( b.readBoundAtSample(3), imath.Box3d(imath.V3d( -3 ), imath.V3d( 3 ) ) ) )

	def testExpandedBoundsForAnimation( self ):

		cube = IECore.Reader.create( "test/IECore/data/cobFiles/pCubeShape1.cob" )()

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		a = m.createChild( "a" )
		a.writeObject( cube, 0.0 )

		a.writeTransform( IECore.M44dData( imath.M44d().rotate( imath.V3d( 0, math.radians(0), 0 ) ) ), 0.0 )
		a.writeTransform( IECore.M44dData( imath.M44d().rotate( imath.V3d( 0, math.radians(90), 0 ) ) ), 1.0 )
		a.writeTransform( IECore.M44dData( imath.M44d().rotate( imath.V3d( 0, math.radians(180), 0 ) ) ), 2.0 )
		a.writeTransform( IECore.M44dData( imath.M44d().rotate( imath.V3d( 0, math.radians(270), 0 ) ) ), 3.0 )

		del m,a

		cubeBound = imath.Box3d( imath.V3d( cube.bound().min() ), imath.V3d( cube.bound().max() ) )
		errorTolerance = imath.V3d(1e-5, 1e-5, 1e-5)

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		a = m.child("a")
		self.assertEqual( a.numBoundSamples(), 1 )

		# the stored qube should have same bbox as the original qube.
		self.failUnless( SceneCacheTest.compareBBox( a.readBoundAtSample(0), cubeBound ) )

		self.assertEqual( m.numBoundSamples(), 4 )

		for t in range( 0, 30, 2 ):
			time = t / 10.0
			angle = time * math.radians(90)
			transformedBound = cubeBound * imath.M44d().rotate( imath.V3d( 0, angle, 0 ) )
			tmpBounds = m.readBound( time )
			tmpBounds.extendBy( imath.Box3d( tmpBounds.min() - errorTolerance, tmpBounds.max() + errorTolerance ) )
			self.failUnless( IECore.BoxAlgo.contains( tmpBounds, transformedBound ) ) # interpolated bounding box must contain bounding box of interpolated rotation.

	def testAnimatedObjectAttributes( self ) :

		plane = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		box = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		box["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ imath.Color3f( 1, 0, 0 ) ] * box.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) ) )
		box2 = box.copy()
		box2["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ imath.Color3f( 0, 1, 0 ) ] * box.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) ) )

		s = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
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
		d.writeObject( box, 2 )

		del s, a, b, c, d

		s = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )

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

		box = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		box["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ imath.Color3f( 1, 0, 0 ) ] * box.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) ) )
		box2 = box.copy()
		box2["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.Color3fVectorData( [ imath.Color3f( 0, 1, 0 ) ] * box.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) ) )

		s = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		b = s.createChild( "b" )
		b.writeObject( box, 0 )
		b.writeObject( box2, 1 )

		del s, b

		s = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		b = s.child( "b" )

		self.assertEqual( b.readObject(0)['P'], b.readObjectPrimitiveVariables(['P','Cs'], 0)['P'] )
		self.assertEqual( b.readObject(0)['Cs'], b.readObjectPrimitiveVariables(['P','Cs'], 0)['Cs'] )
		self.assertEqual( b.readObject(0.5)['P'], b.readObjectPrimitiveVariables(['P','Cs'], 0.5)['P'] )
		self.assertEqual( b.readObject(0.5)['Cs'], b.readObjectPrimitiveVariables(['P','Cs'], 0.5)['Cs'] )
		self.assertEqual( b.readObject(1)['P'], b.readObjectPrimitiveVariables(['P','Cs'], 1)['P'] )
		self.assertEqual( b.readObject(1)['Cs'], b.readObjectPrimitiveVariables(['P','Cs'], 1)['Cs'] )

	def testTags( self ) :

		sphere = IECoreScene.SpherePrimitive( 1 )
		box = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )

		def testSet( values ):
			return set( map( lambda s: IECore.InternedString(s), values ) )

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		A = m.createChild( "A" )
		a = A.createChild( "a" )
		aa = a.createChild( "aa" )
		ab = a.createChild( "ab" )
		ab.writeObject( box, 0 )
		abc = ab.createChild( "abc" )
		abcd = abc.createChild( "abcd" )
		self.assertEqual( set( ab.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet( [ "ObjectType:MeshPrimitive" ] ) )
		B = m.createChild( "B" )
		b = B.createChild( "b" )
		c = B.createChild( "c" )
		d = B.createChild( "d" )
		d.writeObject( sphere, 0 )
		self.assertEqual( set( d.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet( [ "ObjectType:SpherePrimitive" ] ) )

		aa.writeTags( [ "t1" ] )
		self.assertEqual( set( aa.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet( [ "t1" ] ) )
		self.assertRaises( RuntimeError, aa.readTags, IECoreScene.SceneInterface.TagFilter.EveryTag )
		aa.writeTags( [ "t1" ] )
		ab.writeTags( [ IECore.InternedString("t1") ] )
		ab.writeTags( [ IECore.InternedString("t2") ] )

		c.writeTags( [ "t3" ] )

		B.writeTags( [ "t4" ] )

		a.writeTags( [] )
		A.writeTags( [ "t1" ] )

		del m, A, a, aa, ab, B, b, c, d, abc, abcd

		m = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		A = m.child("A")
		a = A.child("a")
		aa = a.child("aa")
		ab = a.child("ab")
		abc = ab.child("abc")
		abcd = abc.child("abcd")
		B = m.child("B")
		b = B.child("b")
		c = B.child("c")
		d = B.child("d")

		self.assertEqual( set( m.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t1", "t2", "t3", "t4", "ObjectType:MeshPrimitive", "ObjectType:SpherePrimitive" ] ) )
		self.assertEqual( set( m.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet([]) )
		self.assertEqual( set( A.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t1", "t2", "ObjectType:MeshPrimitive" ] ) )
		self.assertEqual( set( A.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet( [ "t1" ] ) )
		self.assertEqual( set( a.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t1", "t2", "ObjectType:MeshPrimitive" ] ) )
		self.assertEqual( set( aa.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t1" ] ) )
		self.assertEqual( set( aa.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet(['t1']) )
		self.assertEqual( set( ab.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t1", "t2", "ObjectType:MeshPrimitive" ] ) )
		self.assertEqual( set( abcd.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t1", "t2", "ObjectType:MeshPrimitive" ] ) )
		self.assertEqual( set( B.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t3", "t4", "ObjectType:SpherePrimitive" ] ) )
		self.assertEqual( set( B.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet(['t4']) )
		self.assertEqual( set( b.readTags(IECoreScene.SceneInterface.TagFilter.AncestorTag) ), testSet( ['t4'] ) )
		self.assertEqual( set( b.readTags(IECoreScene.SceneInterface.TagFilter.DescendantTag|IECoreScene.SceneInterface.TagFilter.LocalTag) ), set() )
		self.assertEqual( set( c.readTags(IECoreScene.SceneInterface.TagFilter.AncestorTag) ), testSet( ['t4'] ) )
		self.assertEqual( set( c.readTags(IECoreScene.SceneInterface.TagFilter.EveryTag) ), testSet( [ "t4", "t3" ] ) )
		self.assertEqual( set( d.readTags(IECoreScene.SceneInterface.TagFilter.LocalTag) ), testSet( [ "ObjectType:SpherePrimitive" ] ) )

		self.assertTrue( m.hasTag( "t1", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( m.hasTag( "t4", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertFalse( m.hasTag( "t1", IECoreScene.SceneInterface.TagFilter.LocalTag ) )
		self.assertFalse( m.hasTag( "t4", IECoreScene.SceneInterface.TagFilter.LocalTag ) )
		self.assertFalse( m.hasTag( "t5", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertFalse( m.hasTag( "t5", IECoreScene.SceneInterface.TagFilter.LocalTag ) )
		self.assertTrue( ab.hasTag( "ObjectType:MeshPrimitive", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( B.hasTag( "t4", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( B.hasTag( "t4", IECoreScene.SceneInterface.TagFilter.LocalTag ) )
		self.assertFalse( B.hasTag( "t1", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( B.hasTag( "t3", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( B.hasTag( "ObjectType:SpherePrimitive", IECoreScene.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( d.hasTag( "ObjectType:SpherePrimitive", IECoreScene.SceneInterface.TagFilter.EveryTag ) )

	def testSets( self ):

		# A
		#   B { 'don': ['/E'], 'john'; ['/F'] }
		#      E
		#      F
		#   C { 'don' : ['/'] }
		#   D { 'john' : ['/G] }
		#      G
		# H
		#    I
		#       J
		#          K {'foo',['/L/M/N'] }
		#             L
		#                M
		#                   N

		writeRoot = IECoreScene.SceneCache( "/tmp/testset.scc", IECore.IndexedIO.OpenMode.Write )

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

		B.writeSet( "don", IECore.PathMatcher( ['/E'] ) )
		B.writeSet( "john", IECore.PathMatcher( ['/F'] ) )
		C.writeSet( "don", IECore.PathMatcher( ['/'] ) )
		D.writeSet( "john", IECore.PathMatcher( ['/G'] ) )
		K.writeSet( "foo", IECore.PathMatcher( ['/L/M/N'] ) )

		del N, M, L, K, J, I, H, G, F, E, D, C, B, A, writeRoot

		readRoot = IECoreScene.SceneCache( "/tmp/testset.scc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set(readRoot.childNames()), set (['A', 'H']) )

		A = readRoot.child('A')

		self.assertEqual( set( A.childNames() ), set( ['B', 'C', 'D'] ) ) # default behaviour is to look for sets in descendants.
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
		self.assertEqual( set(C.readSet("don").paths() ), set(['/'] ) )
		self.assertEqual( set(D.readSet("john").paths() ), set(['/G'] ) )

		self.assertEqual( set(E.readSet("don").paths() ), set() )

		# Check the setNames returns all the sets in it's subtree
		self.assertEqual( set( B.setNames() ), set( ['don', 'john'] ) )
		self.assertEqual( set( C.setNames() ), set( ['don'] ) )
		self.assertEqual( set( D.setNames() ), set( ['john'] ) )
		self.assertEqual( set( E.setNames() ), set() )
		self.assertEqual( set( F.setNames() ), set() )

		self.assertEqual( len( A.setNames() ), 2)
		self.assertEqual( set( A.setNames() ), set( ['don', 'john'] ) )
		self.assertEqual( set( A.readSet( "don" ).paths() ), set( ['/B/E', '/C'] ) )
		self.assertEqual( set( A.readSet( "john" ).paths() ), set( ['/B/F', '/D/G'] ) )

		self.assertEqual( set( H.readSet( "foo" ).paths() ), set( ['/I/J/K/L/M/N'] ) )

		self.assertEqual( set( A.setNames( includeDescendantSets = False ) ), set() )  # no set is defined on the top level /A
		self.assertEqual( set( A.readSet( "don", includeDescendantSets = False ).paths() ), set() )
		self.assertEqual( set( B.setNames( includeDescendantSets = False ) ), set( ['don', 'john'] ) )  # no set is defined on the top level /A
		self.assertEqual( set( B.readSet( "don", includeDescendantSets = False ).paths() ), set( ['/E'] ) )
		self.assertEqual( set( B.readSet( "john", includeDescendantSets = False ).paths() ), set( ['/F'] ) )

	def testSetHashes( self ):

		# A
		#   B


		# Note we don't need to write out any sets to test the hashing a
		# as we only use scene graph location, filename & set name for the hash

		writeRoot = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )

		A = writeRoot.createChild("A")
		B = A.createChild("B")

		del A, B, writeRoot

		shutil.copyfile('/tmp/test.scc', '/tmp/testAnotherFile.scc')

		readRoot = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		readRoot2 = IECoreScene.SceneCache( "/tmp/testAnotherFile.scc", IECore.IndexedIO.OpenMode.Read )

		readRoot3 = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )

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

	def testTagsConvertedToSets( self ) :

		# A
		#   B
		#      E ['don']
		#      F ['john']
		#   C  ['don']
		#   D
		#      G ['john']
		# H
		#    I
		#       J
		#          K
		#             L
		#                M
		#                   N ['foo']

		writeRoot = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )

		A = writeRoot.createChild( "A" )
		B = A.createChild( "B" )
		C = A.createChild( "C" )
		D = A.createChild( "D" )
		E = B.createChild( "E" )
		F = B.createChild( "F" )
		G = D.createChild( "G" )

		H = writeRoot.createChild( "H" )
		I = H.createChild( "I" )
		J = I.createChild( "J" )
		K = J.createChild( "K" )
		L = K.createChild( "L" )
		M = L.createChild( "M" )
		N = M.createChild( "N" )

		E.writeTags( ['don'] )
		C.writeTags( ['don'] )
		F.writeTags( ['john'] )
		G.writeTags( ['john'] )
		N.writeTags( ['foo'] )

		del N, M, L, K, J, I, H, G, F, E, D, C, B, A, writeRoot

		readRoot = IECoreScene.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( set( readRoot.childNames() ), set( ['A', 'H'] ) )

		A = readRoot.child( 'A' )

		self.assertEqual( set( A.childNames() ), set( ['B', 'C', 'D'] ) )
		B = A.child( 'B' )
		C = A.child( 'C' )
		D = A.child( 'D' )
		E = B.child( 'E' )
		F = B.child( 'F' )
		H = readRoot.child( 'H' )

		self.assertEqual( set( B.childNames() ), set( ['E', 'F'] ) )
		self.assertEqual( D.childNames(), ['G'] )

		self.assertEqual( set( B.readSet( "don" ).paths() ), set( ['/E'] ) )
		self.assertEqual( set( B.readSet( "john" ).paths() ), set( ['/F'] ) )
		self.assertEqual( set( C.readSet( "don" ).paths() ), set( ['/'] ) )
		self.assertEqual( set( D.readSet( "john" ).paths() ), set( ['/G'] ) )

		self.assertEqual( set( E.readSet( "don" ).paths() ), set( ['/'] ) )

		# Check the setNames returns all the sets in it's subtree
		self.assertEqual( set( B.setNames() ), set( ['don', 'john'] ) )
		self.assertEqual( set( C.setNames() ), set( ['don'] ) )
		self.assertEqual( set( D.setNames() ), set( ['john'] ) )
		self.assertEqual( set( E.setNames() ), set( ['don'] ) )
		self.assertEqual( set( F.setNames() ), set( ['john'] ) )

		self.assertEqual( len( A.setNames() ), 2 )
		self.assertEqual( set( A.setNames() ), set( ['don', 'john'] ) )
		self.assertEqual( set( A.readSet( "don" ).paths() ), set( ['/B/E', '/C'] ) )
		self.assertEqual( set( A.readSet( "john" ).paths() ), set( ['/B/F', '/D/G'] ) )

		self.assertEqual( set( H.readSet( "foo" ).paths() ), set( ['/I/J/K/L/M/N'] ) )

	def testSampleTimeOrder( self ):

		m = IECoreScene.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )

		t = m.createChild( "t" )
		t.writeObject( IECoreScene.SpherePrimitive( 1 ), 1.0 )

		s = m.createChild( "s" )
		s.writeObject( IECoreScene.SpherePrimitive( 1 ), 10.0 )

		del m, t, s

		m = IECoreScene.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( m.boundSampleTime(0) < m.boundSampleTime(1) )

	def testMemoryIndexedIOReadWrite( self ) :

		# create inital file structure in memory:
		mio = IECore.MemoryIndexedIO( IECore.CharVectorData(), IECore.IndexedIO.OpenMode.Write )

		# write to the actual linkedscene:
		scc = IECoreScene.SceneCache( mio )

		c0 = scc.createChild("child0")
		c1 = scc.createChild("child1")

		c0.writeAttribute( "testAttr", IECore.StringData("test0"), 0 )
		c1.writeAttribute( "testAttr", IECore.StringData("test1"), 0 )

		# write the "file" to memory
		del scc, c0, c1

		# can we read it back again?
		mioData = mio.buffer()
		mio = IECore.MemoryIndexedIO( mioData, IECore.IndexedIO.OpenMode.Read )

		scc = IECoreScene.SceneCache( mio )

		self.assertEqual( set( scc.childNames() ), set( ["child0", "child1"] ) )

		# no write access!
		self.assertRaises( RuntimeError, scc.createChild, "child2" )

		c0 = scc.child("child0")
		c1 = scc.child("child1")

		self.assertEqual( c0.readAttribute( "testAttr", 0 ), IECore.StringData( "test0" ) )
		self.assertEqual( c1.readAttribute( "testAttr", 0 ), IECore.StringData( "test1" ) )

	def testTransformInterpolation( self ):

		s = IECoreScene.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )

		t = s.createChild( "t" )

		m = imath.Eulerd( 0, math.pi/2, 0 ).toMatrix44()
		m[3][0] = 10

		t.writeTransform( IECore.M44dData( imath.M44d().translate(imath.V3d( 5, 0, 0 ) ) ), 0.0 )
		t.writeTransform( IECore.M44dData(m), 1.0 )

		del m, t, s

		s = IECoreScene.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		tchild = s.child("t")

		for i in range(0,11):
			interpolatedTransform = tchild.readTransformAsMatrix(float(i)/10)
			s = imath.V3d()
			h = imath.V3d()
			r = imath.V3d()
			t = imath.V3d()
			interpolatedTransform.extractSHRT( s, h, r, t )
			self.assertAlmostEqual( r[1], 0.1 * i * math.pi * 0.5, 9 )
			self.assertAlmostEqual( t[0], 5 + 0.5 * i, 9 )

	def testHashes( self ):

		m = IECoreScene.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )

		def collectHashes( scene, hashType, time, hashResults ) :
			counter = 1
			h = scene.hash( hashType, time ).toString()
			hashResults.add( h )
			for n in scene.childNames() :
				counter += collectHashes( scene.child(n), hashType, time, hashResults )
			return counter

		hashTypes = IECoreScene.SceneInterface.HashType.values.values()

		def checkHash( hashType, scene, currTime, duplicates = 0 ):
			hh = set()
			cc = collectHashes( scene, hashType, currTime, hh )
			self.assertEqual( cc - duplicates, len(hh) )
			return ( cc, hh )

		t0 = checkHash( IECoreScene.SceneInterface.HashType.TransformHash, m, 0 )
		t1 = checkHash( IECoreScene.SceneInterface.HashType.TransformHash, m, 1 )
		self.assertEqual( t0[0]+t1[0]-1, len(t0[1].union(t1[1])) )	# all transforms should be animated except the root

		t05 = checkHash( IECoreScene.SceneInterface.HashType.TransformHash, m, 0.5 )
		self.assertEqual( t0[0]+t05[0]+t1[0]-2, len(t0[1].union(t05[1].union(t1[1]))) )	# all transforms should be animated except the root

		tn1 = checkHash( IECoreScene.SceneInterface.HashType.TransformHash, m, -1 )
		self.assertEqual( t0[0], len(t0[1].union(tn1[1])) )	# time 0 should match time -1's hashes

		duplicatedAttributes = 2
		t0 = checkHash( IECoreScene.SceneInterface.HashType.AttributesHash, m, 0, duplicatedAttributes )
		t1 = checkHash( IECoreScene.SceneInterface.HashType.AttributesHash, m, 1, duplicatedAttributes )
		self.assertEqual( t0[0] - duplicatedAttributes, len(t0[1].union(t1[1])) )		# attributes are not animated in the example scene

		t0 = checkHash( IECoreScene.SceneInterface.HashType.BoundHash, m, 0 )
		t1 = checkHash( IECoreScene.SceneInterface.HashType.BoundHash, m, 1 )
		self.assertEqual( t0[0]+t1[0]-1, len(t0[1].union(t1[1])) )		# only object at /A/a is constant in time and not vary it's bounds everything else differs

		noObjects = 2
		t0 = checkHash( IECoreScene.SceneInterface.HashType.ObjectHash, m, 0, noObjects )
		t1 = checkHash( IECoreScene.SceneInterface.HashType.ObjectHash, m, 1, noObjects )
		self.assertEqual( t0[0] - noObjects + 1, len(t0[1].union(t1[1])) )	# only object at /B/b vary in time everything else should match

		t0 = checkHash( IECoreScene.SceneInterface.HashType.ChildNamesHash, m, 0 )
		t1 = checkHash( IECoreScene.SceneInterface.HashType.ChildNamesHash, m, 1 )
		self.assertEqual( t0[0], len(t0[1].union(t1[1])) )	# child names does not change over time

		t0 = checkHash( IECoreScene.SceneInterface.HashType.HierarchyHash, m, 0 )
		t1 = checkHash( IECoreScene.SceneInterface.HashType.HierarchyHash, m, 1 )
		self.assertEqual( t0[0] + t1[0], len(t0[1].union(t1[1])) )		# all locations differ

	def testHashStability( self ) :

		def collectHashesWalk( scene, hashType, time ) :

			result = {}
			result[scene.pathToString(scene.path())] = scene.hash( hashType, time )
			for name in scene.childNames() :
				result.update( collectHashesWalk( scene.child( name ), hashType, time ) )

			return result

		for hashType in IECoreScene.SceneInterface.HashType.values.values() :

			m1 = IECoreScene.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
			h1 = collectHashesWalk( m1, hashType, 0 )
			del m1

			m2 = IECoreScene.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
			h2 = collectHashesWalk( m2, hashType, 0 )
			del m2

			self.assertEqual( h1, h2 )

	def testParallelAttributeRead( self ) :

		IECoreScene.testSceneCacheParallelAttributeRead()

	def testParallelFakeAttributeRead( self ) :

		IECoreScene.testSceneCacheParallelFakeAttributeRead()

	def testCanReadV6SceneCache( self ):

		r = IECore.IndexedIO.create("test/IECore/data/sccFiles/cube_v6.scc", IECore.IndexedIO.OpenMode.Read)
		metadata = r.metadata()
		self.assertEqual( metadata["version"].value, 6)

		m = IECoreScene.SceneCache( "test/IECore/data/sccFiles/cube_v6.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( ['cube'], m.childNames())

		c = m.child("cube")
		o = c.readObject(0.0)
		m = c.readTransformAsMatrix(1.0)

		self.assertEqual( o.numFaces(), 6 )
		self.assertEqual( o.keys(), ['P'] )

		self.assertEqual( m[0][0], 1.0 )
		self.assertAlmostEqual( m[1][1], 0.74005603790283203 )

	def testObjectVectorShaderCompatibility( self ) :

		# Write a file using ObjectVectors to represent shaders.

		shaderAttributes = [
			"surface", "displacement", "light",
			"ai:surface", "ai:displacement", "ai:light",
			"as:surface", "as:displacement", "as:light",
		]

		nonShaderAttributes = [
			"user:bar", "foo",
		]

		objectVector = IECore.ObjectVector( [
			IECoreScene.Shader( "noise", parameters = { "__handle" : "textureHandle" } ),
			IECoreScene.Shader( "standard_surface", parameters = { "base" : "link:textureHandle.r", "base_color" : "link:textureHandle" } ),
		] )

		io = IECore.MemoryIndexedIO( IECore.CharVectorData(), IECore.IndexedIO.OpenMode.Write )
		scc = IECoreScene.SceneCache( io )
		c = scc.createChild( "c" )

		for a in shaderAttributes :
			c.writeAttribute( a, objectVector, 0 )
		for a in nonShaderAttributes :
			c.writeAttribute( a, objectVector, 0 )

		del c, scc

		# Check that the ObjectVectors are converted to
		# ShaderNetworks during loading.

		shaderNetwork = IECoreScene.ShaderNetwork(
			shaders = {
				"textureHandle" : IECoreScene.Shader( "noise" ),
				"shader" : IECoreScene.Shader( "standard_surface" ),
			},
			connections = [
				( ( "textureHandle", "r" ), ( "shader", "base" ) ),
				( ( "textureHandle" ), ( "shader", "base_color" ) ),
			],
			output = "shader",
		)

		io = IECore.MemoryIndexedIO( io.buffer(), IECore.IndexedIO.OpenMode.Read )
		scc = IECoreScene.SceneCache( io )
		c = scc.child( "c" )

		for a in shaderAttributes :
			self.assertEqual( c.readAttribute( a, 0 ), shaderNetwork )
		for a in nonShaderAttributes :
			self.assertEqual( c.readAttribute( a, 0 ), objectVector )

if __name__ == "__main__":
	unittest.main()

