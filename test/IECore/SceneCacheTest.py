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
		self.assertRaises( RuntimeError, m.readBound, 0.0 )
		del m
		# test Read factory function
		m = IECore.SceneInterface.create( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( isinstance( m, IECore.SceneCache ) )
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
		self.assertEqual( m.readBound(0.0), IECore.Box3d( IECore.V3d( 0, -1, -1 ), IECore.V3d( 2, 1, 1 ) ) )
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

		errorTolerance = IECore.V3d(1e-5, 1e-5, 1e-5)
		
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
			fileBound.extendBy( IECore.Box3d( fileBound.min - errorTolerance, fileBound.max + errorTolerance ) )
			self.failUnless( fileBound.contains( localSpaceBound ) )
			
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

if __name__ == "__main__":
	unittest.main()

