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
import os
import math
import unittest

import IECore

class LinkedSceneTest( unittest.TestCase ) :

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
	
	def testSupportedExtension( self ) :
		self.assertTrue( "lscc" in IECore.SceneInterface.supportedExtensions() )
		self.assertTrue( "lscc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Read ) )
		self.assertTrue( "lscc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Write ) )
		self.assertTrue( "lscc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Write + IECore.IndexedIO.OpenMode.Read ) )
		self.assertFalse( "lscc" in IECore.SceneInterface.supportedExtensions( IECore.IndexedIO.OpenMode.Append ) )

	def testFactoryFunction( self ):
		# test Write factory function 
		m = IECore.SceneInterface.create( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
		self.assertTrue( isinstance( m, IECore.LinkedScene ) )
		self.assertEqual( m.fileName(), "/tmp/test.lscc" )
		self.assertRaises( RuntimeError, m.readBound, 0.0 )
		del m
		# test Read factory function
		m = IECore.SceneInterface.create( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( isinstance( m, IECore.LinkedScene ) )
		self.assertEqual( m.fileName(), "/tmp/test.lscc" )
		m.readBound( 0.0 )

	def testConstructors( self ):

		# test Read from a previously opened scene.
		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		l = IECore.LinkedScene( m )
		# test Write mode
		m = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
		self.assertTrue( isinstance( m, IECore.LinkedScene ) )
		self.assertEqual( m.fileName(), "/tmp/test.lscc" )
		self.assertRaises( RuntimeError, m.readBound, 0.0 )
		del m
		# test Read mode
		m = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		self.assertTrue( isinstance( m, IECore.LinkedScene ) )
		self.assertEqual( m.fileName(), "/tmp/test.lscc" )
		m.readBound( 0.0 )

	def testAppendRaises( self ) :
		self.assertRaises( RuntimeError, IECore.SceneInterface.create, "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Append )
		self.assertRaises( RuntimeError, IECore.LinkedScene, "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Append )

	def testReadNonExistentRaises( self ) :
		self.assertRaises( RuntimeError, IECore.LinkedScene, "iDontExist.lscc", IECore.IndexedIO.OpenMode.Read )

	def testLinkAttribute( self ):

		self.assertEqual( IECore.LinkedScene.linkAttribute, "sceneInterface:link" )

		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		attr = IECore.LinkedScene.linkAttributeData( m )
		expectedAttr = IECore.CompoundData( 
			{
				"fileName": IECore.StringData("test/IECore/data/sccFiles/animatedSpheres.scc"), 
				"root": IECore.InternedStringVectorData( [] )
			}
		)
		self.assertEqual( attr, expectedAttr )

		A = m.child("A")
		attr = IECore.LinkedScene.linkAttributeData( A )
		expectedAttr = IECore.CompoundData( 
			{
				"fileName": IECore.StringData("test/IECore/data/sccFiles/animatedSpheres.scc"), 
				"root": IECore.InternedStringVectorData( [ 'A' ] )
			}
		)
		self.assertEqual( attr, expectedAttr )

		A = m.child("A")
		attr = IECore.LinkedScene.linkAttributeData( A, 10.0 )
		expectedAttr['time'] = IECore.DoubleData(10.0)
		self.assertEqual( attr, expectedAttr )
		
	def testWriting( self ):

		generateTestFiles = False	# change this to True to recreate the LinkedScene files for other tests.
		testFilesSuffix = "_newTags"
		if generateTestFiles :
			outputPath = "test/IECore/data/sccFiles"
		else :
			outputPath = "/tmp"

		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		A = m.child("A")

		l = IECore.LinkedScene( os.path.join(outputPath,"instancedSpheres%s.lscc"%testFilesSuffix), IECore.IndexedIO.OpenMode.Write )
		i0 = l.createChild("instance0")
		i0.writeLink( m )
		i1 = l.createChild("instance1")
		i1.writeLink( m )
		i1.writeAttribute( "testAttr", IECore.StringData("test"), 0 )
		i1.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ), 0.0 )
		i2 = l.createChild("instance2")
		i2.writeLink( A )
		i2.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ), 0.0 )
		self.assertRaises( RuntimeError, i2.createChild, "cannotHaveChildrenAtLinks" )
		i2.writeTags( ["canHaveTagsAtLinks"] )
		self.assertRaises( RuntimeError, i2.writeObject, IECore.SpherePrimitive( 1 ), 0.0 )  # cannot save objects at link locations.
		b1 = l.createChild("branch1")
		b1.writeObject( IECore.SpherePrimitive( 1 ), 0.0 )
		self.assertRaises( RuntimeError, b1.writeLink, A )
		b2 = l.createChild("branch2")
		c2 = b2.createChild("child2")
		self.assertRaises( RuntimeError, b2.writeLink, A )
		del i0, i1, i2, l, b1, b2, c2

		l = IECore.LinkedScene( os.path.join(outputPath,"instancedSpheres%s.lscc"%testFilesSuffix), IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( l.numBoundSamples(), 4 )
		self.assertEqual( set(l.childNames()), set(['instance0','instance1','instance2','branch1','branch2']) )
		i0 = l.child("instance0")
		self.assertEqual( i0.numBoundSamples(), 4 )
		self.failUnless( LinkedSceneTest.compareBBox( i0.readBoundAtSample(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i0.readBoundAtSample(1), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 3,3,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i0.readBoundAtSample(2), IECore.Box3d( IECore.V3d( -2,-1,-2 ), IECore.V3d( 4,5,2 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i0.readBoundAtSample(3), IECore.Box3d( IECore.V3d( -3,-1,-3 ), IECore.V3d( 4,6,3 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i0.readBound(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )

		A = i0.child("A")
		self.failUnless( LinkedSceneTest.compareBBox( A.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( A.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( A.readBoundAtSample(2), IECore.Box3d(IECore.V3d( 0,-1,-1 ), IECore.V3d( 2,1,1 ) ) ) )
		self.assertEqual( i0.readTransform( 0 ), IECore.M44dData( IECore.M44d() ) )

		i1 = l.child("instance1")
		self.assertEqual( i1.numBoundSamples(), 4 )
		self.failUnless( LinkedSceneTest.compareBBox( i1.readBoundAtSample(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i1.readBoundAtSample(2), IECore.Box3d( IECore.V3d( -2,-1,-2 ), IECore.V3d( 4,5,2 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i1.readBoundAtSample(3), IECore.Box3d( IECore.V3d( -3,-1,-3 ), IECore.V3d( 4,6,3 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i1.readBound(0), IECore.Box3d( IECore.V3d( -1,-1,-1 ), IECore.V3d( 2,2,1 ) ) ) )
		self.assertEqual( i1.readTransform( 0 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ) )
		self.assertEqual( i1.readAttribute( "testAttr", 0 ), IECore.StringData("test") )
		
		i2 = l.child("instance2")
		self.assertEqual( i2.numBoundSamples(), 3 )
		self.failUnless( LinkedSceneTest.compareBBox( i2.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i2.readBoundAtSample(1), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( i2.readBoundAtSample(2), IECore.Box3d(IECore.V3d( 0,-1,-1 ), IECore.V3d( 2,1,1 ) ) ) )
		self.assertEqual( i2.readTransform( 0 ), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )
		self.assertTrue( i2.hasTag( "canHaveTagsAtLinks", IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( l.hasTag( "canHaveTagsAtLinks", IECore.SceneInterface.TagFilter.EveryTag ) )	# tags propagate up
		self.assertTrue( i2.child("a").hasTag( "canHaveTagsAtLinks", IECore.SceneInterface.TagFilter.EveryTag ) )		# tags at link locations propagate down as well

		self.assertEqual( l.scene( [ 'instance0' ] ).path(), [ 'instance0' ] )
		self.assertEqual( l.scene( [ 'instance0', 'A' ] ).path(), [ 'instance0', 'A' ] )
		self.assertEqual( i0.path(), [ 'instance0' ] )

		# test saving a two level LinkedScene
		l2 = IECore.LinkedScene( os.path.join(outputPath,"environment%s.lscc"%testFilesSuffix), IECore.IndexedIO.OpenMode.Write )
		base = l2.createChild("base")
		t1 = base.createChild("test1")
		t1.writeLink( l )
		t2 = base.createChild("test2")
		t2.writeLink( i0 )
		t3 = base.createChild("test3")
		t3.writeLink( i1 )
		t4 = base.createChild("test4")
		t4.writeLink( i2 )
		t5 = base.createChild("test5")
		t5.writeLink( A )
		del l2, t1, t2, t3, t4, t5

	def testWriteLinkAnimatedTransform( self ):

		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :
		
			m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )

			l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
			i0 = l.createChild("instance0")
			i0.writeLink( m )

			# this was causing a problem upon deleting l, as the first transform sample doesn't coincide with the
			# first bound sample in the link
			i0.writeTransform( IECore.M44dData( IECore.M44d() ), 5.0 )
			i0.writeTransform( IECore.M44dData( IECore.M44d() ), 6.0 )

			del i0, l, m
		
		for messageInfo in messageHandler.messages:
			if not messageInfo.message.startswith( "Detected ancestor tags" ) :
				self.fail( messageInfo.message )
	
	def testTimeRemapping( self ):

		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )

		l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
		# save animated spheres with double the speed and with offset, using less samples (time remapping)
		i0 = l.createChild("instance0")
		i0.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 0.0 ), 1.0 )
		i0.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 3.0 ), 2.0 )
		# save animated spheres with same speed and with offset, same samples (time remapping is identity)
		i1 = l.createChild("instance1")
		i1.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 0.0 ), 1.0 )
		i1.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 1.0 ), 2.0 )
		i1.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 2.0 ), 3.0 )
		i1.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 3.0 ), 4.0 )
		# save animated spheres with half the speed, adding more samples to a range of the original (time remapping)
		i2 = l.createChild("instance2")
		i2.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 0.0 ), 0.0 )
		i2.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 0.5 ), 1.0 )
		i2.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 1.0 ), 2.0 )

		del i0, i1, i2, l

		l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( l.numBoundSamples(), 5 )
		self.assertEqual( l.hasAttribute( "sceneInterface:link.time" ), False )
		i0 = l.child("instance0")
		
		self.assertEqual( i0.hasAttribute( "sceneInterface:link.time" ), True )
		self.assertEqual( i0.readAttribute( "sceneInterface:link.time", 1 ).value, 0 )
		self.assertEqual( i0.readAttribute( "sceneInterface:link.time", 2 ).value, 3 )
		
		self.assertEqual( i0.numBoundSamples(), 2 )
		self.assertEqual( i0.numTransformSamples(), 1 )
		self.assertEqual( i0.readTransformAtSample(0), IECore.M44dData() )
		A0 = i0.child("A")
		self.assertEqual( A0.hasAttribute( "sceneInterface:link.time" ), False )
		
		self.assertEqual( A0.numBoundSamples(), 2 )
		self.assertEqual( A0.numTransformSamples(), 2 )
		self.failUnless( LinkedSceneTest.compareBBox( A0.readBoundAtSample(0), IECore.Box3d(IECore.V3d( -1,-1,-1 ), IECore.V3d( 1,1,1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( A0.readBoundAtSample(1), IECore.Box3d(IECore.V3d( 0,-1,-1 ), IECore.V3d( 2,1,1 ) ) ) )
		self.assertEqual( A0.readTransformAtSample(0), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ) )
		self.assertEqual( A0.readTransformAtSample(1), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )
		i1 = l.child("instance1")
		
		self.assertEqual( i1.hasAttribute( "sceneInterface:link.time" ), True )
		self.assertEqual( i1.readAttribute( "sceneInterface:link.time", 1 ).value, 0 )
		self.assertEqual( i1.readAttribute( "sceneInterface:link.time", 2 ).value, 1 )
		self.assertEqual( i1.readAttribute( "sceneInterface:link.time", 3 ).value, 2 )
		self.assertEqual( i1.readAttribute( "sceneInterface:link.time", 4 ).value, 3 )
		
		self.assertEqual( i1.numBoundSamples(), 4 )
		self.assertEqual( i1.numTransformSamples(), 1 )
		A1 = i1.child("A")
		self.assertEqual( A1.numTransformSamples(), 4 )
		self.assertEqual( A1.readTransformAtSample(0), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ) )
		self.assertEqual( A1.readTransformAtSample(1), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )
		self.assertEqual( A1.readTransformAtSample(2), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )
		self.assertEqual( A1.readTransformAtSample(3), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )
		
		self.assertEqual( A1.hasAttribute( "sceneInterface:link.time" ), False )
		
		i2 = l.child("instance2")
		
		self.assertEqual( i2.hasAttribute( "sceneInterface:link.time" ), True )
		self.assertEqual( i2.readAttribute( "sceneInterface:link.time", 0 ).value, 0 )
		self.assertEqual( i2.readAttribute( "sceneInterface:link.time", 1 ).value, 0.5 )
		self.assertEqual( i2.readAttribute( "sceneInterface:link.time", 2 ).value, 1 )
		
		self.assertEqual( i2.numBoundSamples(), 3 )
		self.assertEqual( i2.numTransformSamples(), 1 )
		A2 = i2.child("A")
		self.assertEqual( A2.numBoundSamples(), 3 )
		self.assertEqual( A2.numTransformSamples(), 3 )
		self.assertEqual( A2.readTransform(1.0), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1.5, 0, 0 ) ) ) )
		self.assertEqual( A2.readTransformAtSample(0), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ) )
		self.assertEqual( A2.readTransformAtSample(1), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1.5, 0, 0 ) ) ) )
		self.assertEqual( A2.readTransformAtSample(2), IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ) )
		
		self.assertEqual( A2.hasAttribute( "sceneInterface:link.time" ), False )

	
	def testNestedTimeRemapping( self ):
		
		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
	
		A = m.child("A")
		
		l2 = IECore.LinkedScene( "/tmp/test3.lscc", IECore.IndexedIO.OpenMode.Write )
		t2 = l2.createChild("transform2")
		i2 = t2.createChild("instance2")
		i2.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 0.0 ), 0.0 )
		i2.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( m, 2.0 ), 1.0 )
		
		del l2, i2, t2
		l2 = IECore.LinkedScene( "/tmp/test3.lscc", IECore.IndexedIO.OpenMode.Read )
		
		l1 = IECore.LinkedScene( "/tmp/test2.lscc", IECore.IndexedIO.OpenMode.Write )
		t1 = l1.createChild("transform1")
		i1 = t1.createChild("instance1")
		i1.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( l2, 0.0 ), 0.0 )
		i1.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( l2, 2.0 ), 1.0 )
		
		del l1, i1, t1
		l1 = IECore.LinkedScene( "/tmp/test2.lscc", IECore.IndexedIO.OpenMode.Read )
		
		l0 = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
		t0 = l0.createChild("transform0")
		i0 = t0.createChild("instance0")
		i0.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( l1, 0.0 ), 0.0 )
		i0.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( l1, 2.0 ), 1.0 )
		
		del l0, i0, t0
		
		l0 = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		l = IECore.LinkedScene( "/tmp/testTop.lscc", IECore.IndexedIO.OpenMode.Write )
		t = l.createChild("transform")
		i = t.createChild("instance")
		i.writeLink( l0 )
		
		del l, i, t
		
		
		del m, l0, l1, l2
		
		l = IECore.LinkedScene( "/tmp/testTop.lscc", IECore.IndexedIO.OpenMode.Read )
		t = l.child("transform")
		i = t.child("instance")
		t0 = i.child("transform0")
		i0 = t0.child("instance0")
		t1 = i0.child("transform1")
		i1 = t1.child("instance1")
		t2 = i1.child("transform2")
		i2 = t2.child("instance2")
		A = i2.child("A")
		
		# this location shouldn't be retimed:
		self.assertEqual( i.hasAttribute( "sceneInterface:link.time" ), True )
		self.assertEqual( i.readAttribute( "sceneInterface:link.time", 0.25 ).value, 0.25 )
		
		# this location should be sped up by a factor of 2:
		self.assertEqual( i0.hasAttribute( "sceneInterface:link.time" ), True )
		self.assertEqual( i0.readAttribute( "sceneInterface:link.time", 0.25 ).value, 0.5 )
		
		# this one is remapped twice, so it's sped up by a factor of 4:
		self.assertEqual( i1.hasAttribute( "sceneInterface:link.time" ), True )
		self.assertEqual( i1.readAttribute( "sceneInterface:link.time", 0.25 ).value, 1 )
		
		# and this one is remapped three times, so it's sped up by a factor of 8:
		self.assertEqual( i2.hasAttribute( "sceneInterface:link.time" ), True )
		self.assertEqual( i2.readAttribute( "sceneInterface:link.time", 0.25 ).value, 2 )
		
		# sanity check:
		self.assertEqual( i.readAttribute( "sceneInterface:link.time", 0 ).value, 0 )
		self.assertEqual( i0.readAttribute( "sceneInterface:link.time", 0 ).value, 0 )
		self.assertEqual( i1.readAttribute( "sceneInterface:link.time", 0 ).value, 0 )
		self.assertEqual( i2.readAttribute( "sceneInterface:link.time", 0 ).value, 0 )
		
		# test multiple retiming of the transform:
		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		Aa = m.child("A")
		self.assertEqual( Aa.readTransformAsMatrix( 0.1 ), A.readTransformAsMatrix( 0.1 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.2 ), A.readTransformAsMatrix( 0.2 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.3 ), A.readTransformAsMatrix( 0.3 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.4 ), A.readTransformAsMatrix( 0.4 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.5 ), A.readTransformAsMatrix( 0.5 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.6 ), A.readTransformAsMatrix( 0.6 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.7 ), A.readTransformAsMatrix( 0.7 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.8 ), A.readTransformAsMatrix( 0.8 / 8 ) )
		self.assertEqual( Aa.readTransformAsMatrix( 0.9 ), A.readTransformAsMatrix( 0.9 / 8 ) )
	
	def readSavedScenes( self, fileVersion ):

		def recurseCompare( basePath, virtualScene, realScene, atLink = True ) :
			self.assertEqual( basePath, virtualScene.path() )

			if atLink :	
	
				self.assertEqual( set(virtualScene.readTags(IECore.SceneInterface.TagFilter.DescendantTag)), set(realScene.readTags(IECore.SceneInterface.TagFilter.DescendantTag)) )

			else: # attributes and tranforms at link location are not loaded.

				self.assertEqual( set(virtualScene.attributeNames()), set(realScene.attributeNames()) )
				for attr in realScene.attributeNames() :
					self.assertTrue( virtualScene.hasAttribute( attr ) )
					self.assertEqual( virtualScene.numAttributeSamples(attr), realScene.numAttributeSamples(attr) )
					for s in xrange(0,virtualScene.numAttributeSamples(attr)) :
						self.assertEqual( virtualScene.readAttributeAtSample(attr, s), realScene.readAttributeAtSample(attr, s) )

				self.assertEqual( virtualScene.numTransformSamples(), realScene.numTransformSamples() )
				for s in xrange(0,virtualScene.numTransformSamples()) :
					self.assertEqual( virtualScene.readTransformAtSample(s), realScene.readTransformAtSample(s) )

				self.assertEqual( set(virtualScene.readTags()), set(realScene.readTags()) )
				self.assertEqual( set(virtualScene.readTags(IECore.SceneInterface.TagFilter.LocalTag|IECore.SceneInterface.TagFilter.DescendantTag)), set(realScene.readTags(IECore.SceneInterface.TagFilter.LocalTag|IECore.SceneInterface.TagFilter.DescendantTag)) )

			self.assertEqual( virtualScene.numBoundSamples(), realScene.numBoundSamples() )
			for s in xrange(0,virtualScene.numBoundSamples()) :
				self.assertEqual( virtualScene.readBoundAtSample(s), realScene.readBoundAtSample(s) )

			self.assertEqual( virtualScene.hasObject(), realScene.hasObject() )
			if virtualScene.hasObject() :
				self.assertEqual( virtualScene.numObjectSamples(), realScene.numObjectSamples() )
				for s in xrange(0,virtualScene.numObjectSamples()) :
					self.assertEqual( virtualScene.readObjectAtSample(s), realScene.readObjectAtSample(s) )

			self.assertEqual( set(virtualScene.childNames()), set(realScene.childNames()) )
			for c in virtualScene.childNames() :
				self.assertTrue( virtualScene.hasChild(c) )
				recurseCompare( basePath + [ str(c) ], virtualScene.child(c), realScene.child(c), False )

		env = IECore.LinkedScene( "test/IECore/data/sccFiles/environment%s.lscc" % fileVersion, IECore.IndexedIO.OpenMode.Read )	# created by testWriting() when generateTestFiles=True and testFilesSuffix is defined.
		l = IECore.LinkedScene( "test/IECore/data/sccFiles/instancedSpheres%s.lscc" % fileVersion, IECore.IndexedIO.OpenMode.Read )	# created by testWriting() when generateTestFiles=True and testFilesSuffix is defined.
		m = IECore.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )

		base = env.child('base')
		self.assertEqual( set(base.childNames()), set(['test1','test2','test3','test4','test5']) )
		test1 = base.child('test1')
		self.assertEqual( test1.path(), [ "base", "test1" ] )
		recurseCompare( test1.path(), test1, l )
		test2 = base.child('test2')
		self.assertEqual( test2.path(), [ "base", "test2" ] )
		recurseCompare( test2.path(), test2, l.child('instance0') )
		test3 = base.child('test3')
		self.assertEqual( test3.path(), [ "base", "test3" ] )
		recurseCompare( test3.path(), test3, l.child('instance1') )
		test4 = base.child('test4')
		self.assertEqual( test4.path(), [ "base", "test4" ] )
		recurseCompare( test4.path(), test4, l.child('instance2') )
		test5 = base.child('test5')
		self.assertEqual( test5.path(), [ "base", "test5" ] )
		recurseCompare( test5.path(), test5, l.child('instance1').child('A') )
		
		
		# attributes like sceneInterface:link.root, sceneInterface:link.fileName, and sceneInterface:link.time shouldn't show up at links, although they might be there...
		self.assertEqual( test1.child('instance0').attributeNames(), [] )
		self.assertEqual( test1.child('instance1').attributeNames(), [ 'testAttr' ] )
		self.assertEqual( test1.child('instance2').attributeNames(), [] )
		
		
		# hasAttribute should tell the truth though...
		self.assertEqual( test1.child('instance0').hasAttribute( "sceneInterface:link.fileName" ), True )
		self.assertEqual( test1.child('instance0').hasAttribute( "sceneInterface:link.root" ), True )
		
		self.assertEqual( test1.child('instance1').hasAttribute( "sceneInterface:link.fileName" ), True )
		self.assertEqual( test1.child('instance1').hasAttribute( "sceneInterface:link.root" ), True )
		
		self.assertEqual( test1.child('instance2').hasAttribute( "sceneInterface:link.fileName" ), True )
		self.assertEqual( test1.child('instance2').hasAttribute( "sceneInterface:link.root" ), True )
		
		self.assertEqual( test1.child('instance0').path(), [ "base", "test1", "instance0" ] )
		recurseCompare( test1.child('instance0').path(), test1.child('instance0'), m )
		recurseCompare( test2.path(), test2, m )
		recurseCompare( test3.path(), test3, m )
		recurseCompare( test4.path(), test4, m.child('A') )
		recurseCompare( test5.path(), test5, m.child('A') )

		recurseCompare( test1.path(), env.scene( [ 'base', 'test1' ] ), l )
		recurseCompare( test1.path(), env.scene( [ 'base' ] ).child( 'test1' ), l )

	def testReadingFormats( self ):

		self.readSavedScenes( "" )		# tests first LinkedScene file format, with tags represented under the entry "tags"
		self.readSavedScenes( "_newTags" ) # tests second LinkedScene file format, with tags represented in separated entries: "localTags", "descendentTags" and "ancestorTags".

	def testTags( self ) :

		def testSet( values ):
			return set( map( lambda s: IECore.InternedString(s), values ) )

		# create a base scene
		l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
		a = l.createChild('a')
		a.writeTags( [ "testA" ] )
		b = l.createChild('b')
		b.writeTags(  [ "testB" ] )
		l.writeTags( [ "tags" ] )
		del a, b, l

		# now create a linked scene that should inherit the tags from the base one, plus add other ones
		l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		a = l.child('a')
		b = l.child('b')

		self.assertEqual( set(l.readTags(IECore.SceneInterface.TagFilter.EveryTag)), testSet(["testA", "testB", "tags"]) )
		self.assertEqual( set(l.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["tags"]) )
		self.assertEqual( set(a.readTags(IECore.SceneInterface.TagFilter.AncestorTag)), testSet(["tags"]) )
		self.assertEqual( set(a.readTags(IECore.SceneInterface.TagFilter.DescendantTag)), set() )
		self.assertEqual( set(a.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["testA"]) )
		self.assertEqual( set(b.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["testB"]) )
		self.assertEqual( set(b.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["testB"]) )
		self.assertTrue( l.hasTag("testA", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertTrue( l.hasTag("testB", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertFalse( l.hasTag("testA", IECore.SceneInterface.TagFilter.LocalTag) )
		self.assertFalse( l.hasTag("testB", IECore.SceneInterface.TagFilter.LocalTag) )
		self.assertTrue( a.hasTag("testA", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertFalse( a.hasTag("testB", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertTrue( b.hasTag("testB", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertFalse( b.hasTag("testA", IECore.SceneInterface.TagFilter.EveryTag) )

		l2 = IECore.LinkedScene( "/tmp/test2.lscc", IECore.IndexedIO.OpenMode.Write )

		A = l2.createChild('A')
		A.writeLink( l )
		A.writeTags( ['linkedA'] )	# creating tag after link

		B = l2.createChild('B')

		# creating a link to a branch of an external file.
		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :
			# will have warnings as the branch inherits ancestor tags...
			B.writeLink( a )
		if not len( messageHandler.messages ):
			self.fail( "Was expecting a warning message when linking to a location that had ancestor tags!" )
		else :
			for messageInfo in messageHandler.messages:
				if not messageInfo.message.startswith( "Detected ancestor tags" ) :
					self.fail( messageHandler.message )

		C = l2.createChild('C')
		c = C.createChild('c')
		c.writeLink( l )
		C.writeTags( [ 'C' ] )

		D = l2.createChild('D')
		D.writeTags( [ 'D' ] )
		D.writeLink( a )	# creating link after tag

		del l, a, b, l2, A, B, C, c, D

		l2 = IECore.LinkedScene( "/tmp/test2.lscc", IECore.IndexedIO.OpenMode.Read )
		A = l2.child("A")
		Aa = A.child("a")
		B = l2.child("B")
		C = l2.child("C")
		c = C.child("c")
		ca = c.child("a")
		D = l2.child("D")

		self.assertTrue( l2.hasTag("testA", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertTrue( l2.hasTag("testB", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertFalse( l2.hasTag("t", IECore.SceneInterface.TagFilter.EveryTag) )

		self.assertEqual( set(l2.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet([]) )
		self.assertEqual( set(l2.readTags(IECore.SceneInterface.TagFilter.DescendantTag)), testSet(["testA", "testB","tags", "C", "D","linkedA"]) )
		self.assertEqual( set(l2.readTags(IECore.SceneInterface.TagFilter.LocalTag|IECore.SceneInterface.TagFilter.DescendantTag)), testSet(["testA", "testB","tags", "C", "D","linkedA"]) )
		self.assertEqual( set(l2.readTags(IECore.SceneInterface.TagFilter.AncestorTag)), testSet([]) )
		self.assertEqual( set(A.readTags(IECore.SceneInterface.TagFilter.EveryTag)), testSet(["testA","testB", "tags","linkedA"]) )
		self.assertTrue( A.hasTag( "linkedA", IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( A.hasTag( "tags", IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( A.hasTag( "testA", IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertTrue( A.hasTag( "testB", IECore.SceneInterface.TagFilter.EveryTag ) )
		self.assertFalse( A.hasTag("C", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertEqual( set(A.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["tags","linkedA"]) )
		self.assertEqual( set(Aa.readTags(IECore.SceneInterface.TagFilter.EveryTag)), testSet(["tags","testA", "linkedA"]) )
		self.assertEqual( set(Aa.readTags(IECore.SceneInterface.TagFilter.AncestorTag)), testSet(["tags", "linkedA"]) )
		self.assertEqual( set(Aa.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["testA"]) )
		self.assertTrue( Aa.hasTag("testA", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertFalse( Aa.hasTag("testB", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertEqual( set(B.readTags(IECore.SceneInterface.TagFilter.EveryTag)), testSet(["testA", "tags"]) )	# should not list "linkedA" as the link pointed to a child location.
		self.assertEqual( set(C.readTags(IECore.SceneInterface.TagFilter.EveryTag)), testSet(["testA","testB","tags","C"]) )
		self.assertEqual( set(C.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["C"]) )
		self.assertEqual( set(c.readTags(IECore.SceneInterface.TagFilter.EveryTag)), testSet(["C", "testA", "testB","tags"]) )
		self.assertEqual( set(c.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["tags"]) )
		self.assertEqual( set(c.readTags(IECore.SceneInterface.TagFilter.DescendantTag)), testSet([ "testA", "testB" ]) )
		self.assertEqual( set(ca.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["testA"]) )
		self.assertEqual( set(ca.readTags(IECore.SceneInterface.TagFilter.AncestorTag)), testSet(["C", "tags"]) )
		self.assertTrue( ca.hasTag("testA", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertFalse( ca.hasTag("testB", IECore.SceneInterface.TagFilter.EveryTag) )
		self.assertEqual( set(C.readTags(IECore.SceneInterface.TagFilter.LocalTag)), testSet(["C"]) )
		self.assertEqual( set(D.readTags(IECore.SceneInterface.TagFilter.EveryTag)), testSet(["tags", "D", "testA"]) )
	
	def testMissingLinkedScene( self ) :
		
		import shutil
		shutil.copyfile( "test/IECore/data/sccFiles/animatedSpheres.scc", "/tmp/toBeRemoved.scc" )
		
		m = IECore.SceneCache( "/tmp/toBeRemoved.scc", IECore.IndexedIO.OpenMode.Read )
		A = m.child("A")
		
		l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
		i0 = l.createChild("instance0")
		i0.writeLink( m )
		i1 = l.createChild("instance1")
		i1.writeLink( m )
		i1.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) ), 0.0 )
		i2 = l.createChild("instance2")
		i2.writeLink( A )
		i2.writeTransform( IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) ), 0.0 )
		del i0, i1, i2, l, m, A

		l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sorted(l.childNames()), [ "instance0", "instance1", "instance2" ] )
		i0 = l.child( "instance0" )
		self.assertEqual( sorted(i0.childNames()), [ "A", "B" ] )
		i1 = l.child( "instance1" )
		self.assertEqual( sorted(i1.childNames()), [ "A", "B" ] )
		i2 = l.child( "instance2" )
		self.assertEqual( i2.childNames(), [ "a" ] )
		del l, i0, i1, i2
		
		os.remove( "/tmp/toBeRemoved.scc" )
		IECore.SharedSceneInterfaces.clear()
		
		l = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( sorted(l.childNames()), [ "instance0", "instance1", "instance2" ] )
		i0 = l.child( "instance0" )
		self.assertEqual( i0.childNames(), [] )
		i1 = l.child( "instance1" )
		self.assertEqual( i1.childNames(), [] )
		i2 = l.child( "instance2" )
		self.assertEqual( i2.childNames(), [] )

	def testLinkBoundTransformMismatch( self ) :
		
		scene = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Write )
		child = scene.createChild( "child" )
		mesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ) )
		child.writeObject( mesh, 0 )
		
		del scene, child
		
		child = IECore.SceneCache( "/tmp/test.scc", IECore.IndexedIO.OpenMode.Read ).child( "child" )
		
		linked = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Write )
		parent = linked.createChild( "parent" )
		transform = IECore.M44dData( IECore.M44d.createTranslated( IECore.V3d( 1, 0, 0 ) ) )
		parent.writeTransform( transform, 1.0 )
		parent.writeObject( mesh, 1.0 )
		childLink = parent.createChild( "childLink" )
		childLink.writeTransform( transform, 1.0 )
		childLink.writeAttribute( IECore.LinkedScene.linkAttribute, IECore.LinkedScene.linkAttributeData( child ), 1.0 )
		
		del linked, parent, child, childLink
		
		linked = IECore.LinkedScene( "/tmp/test.lscc", IECore.IndexedIO.OpenMode.Read )
		parent = linked.child( "parent" )
		childLink = parent.child( "childLink" )
		
		# there are 2 bound samples, because the link has bounds at time 0, but a transform at time 1
		self.assertEqual( linked.numBoundSamples(), 2 )
		self.assertEqual( parent.numBoundSamples(), 2 )
		self.assertEqual( childLink.numBoundSamples(), 1 )
		
		self.assertEqual( linked.numTransformSamples(), 1 )
		self.assertEqual( parent.numTransformSamples(), 1 )
		self.assertEqual( childLink.numTransformSamples(), 1 )
		
		# object at the origin
		self.failUnless( LinkedSceneTest.compareBBox( childLink.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( 0 ), IECore.V3d( 1 ) ) ) )
		
		# transformed the childLink by ( 1, 0, 0 ) and added an object at the origin
		self.assertEqual( childLink.readTransformAtSample( 0 ), transform )
		self.failUnless( LinkedSceneTest.compareBBox( parent.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( 0 ), IECore.V3d( 2, 1, 1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( parent.readBoundAtSample( 0 ), parent.readBoundAtSample( 1 ) ) )
		
		# transformed the parent by ( 1, 0, 0 )
		self.assertEqual( parent.readTransformAtSample( 0 ), transform )
		self.failUnless( LinkedSceneTest.compareBBox( linked.readBoundAtSample( 0 ), IECore.Box3d( IECore.V3d( 1, 0, 0 ), IECore.V3d( 3, 1, 1 ) ) ) )
		self.failUnless( LinkedSceneTest.compareBBox( linked.readBoundAtSample( 0 ), linked.readBoundAtSample( 1 ) ) )
	
	def testMemoryIndexedIOReadWrite( self ) :
		
		# create inital file structure in memory:
		mio = IECore.MemoryIndexedIO( IECore.CharVectorData(), IECore.IndexedIO.OpenMode.Write )
		
		# write to the actual linkedscene:
		scc = IECore.SceneCache( mio )
		l = IECore.LinkedScene( scc )
		
		c0 = l.createChild("child0")
		c1 = l.createChild("child1")
		
		c0.writeAttribute( "testAttr", IECore.StringData("test0"), 0 )
		c1.writeAttribute( "testAttr", IECore.StringData("test1"), 0 )
		
		# write the "file" to memory
		del l, scc, c0, c1
		
		# can we read it back again?
		mioData = mio.buffer()
		mio = IECore.MemoryIndexedIO( mioData, IECore.IndexedIO.OpenMode.Read )
		
		scc = IECore.SceneCache( mio )
		l = IECore.LinkedScene( scc )
		
		self.assertEqual( set( l.childNames() ), set( ["child0", "child1"] ) )
		
		# no write access!
		self.assertRaises( RuntimeError, l.createChild, "child2" )
		
		c0 = l.child("child0")
		c1 = l.child("child1")
		
		self.assertEqual( c0.readAttribute( "testAttr", 0 ), IECore.StringData( "test0" ) )
		self.assertEqual( c1.readAttribute( "testAttr", 0 ), IECore.StringData( "test1" ) )
		
if __name__ == "__main__":
	unittest.main()

