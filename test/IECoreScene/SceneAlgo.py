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


import unittest
import IECore
import IECoreScene

import imath


class SceneAlgoTest( unittest.TestCase ) :
	__testFile = "/tmp/test.scc"
	__testFile2 = "/tmp/test2.scc"

	def writeSCC( self ) :
		m = IECoreScene.SceneCache( SceneAlgoTest.__testFile, IECore.IndexedIO.OpenMode.Write )
		m.writeAttribute( "w", IECore.BoolData( True ), 1.0 )

		t = m.createChild( "t" )
		t.writeTransform( IECore.M44dData( imath.M44d().translate( imath.V3d( 1, 0, 0 ) ) ), 1.0 )
		t.writeAttribute( "wuh", IECore.BoolData( True ), 1.0 )

		s = t.createChild( "s" )
		s.writeObject( IECoreScene.SpherePrimitive( 1 ), 1.0 )
		s.writeAttribute( "glah", IECore.IntData( 15 ), 1.0 )

		s.writeTags( ["tagA", "tagB"] )

		del s, t, m


	def writeBigSCC( self ):
		m = IECoreScene.SceneCache( SceneAlgoTest.__testFile, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )

		for i in range(4096):
			box = IECoreScene.MeshPrimitive.createBox(imath.Box3f(imath.V3f(-i,-i,-i), imath.V3f(i,i,i)))
			r = t.createChild("t{0}".format(i))
			r.writeObject( box, 1.0 )
			r.writeAttribute("foo", IECore.IntData(1), 1.0)

	def testCopySceneHierarchyOnly( self ) :
		self.writeSCC()

		src = IECoreScene.SceneCache( SceneAlgoTest.__testFile, IECore.IndexedIO.OpenMode.Read )
		dst = IECoreScene.SceneCache( SceneAlgoTest.__testFile2, IECore.IndexedIO.OpenMode.Write )

		IECoreScene.SceneAlgo.copy( src, dst, 1, 1, 1.0, IECoreScene.SceneAlgo.ProcessFlags.None )

		del src, dst

		src = IECoreScene.SceneCache( SceneAlgoTest.__testFile2, IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( src.childNames(), ['t'] )
		self.assertFalse( src.hasAttribute( "w" ) )

		t = src.child( "t" )

		self.assertEqual( t.readTransform( 1.0 ), IECore.M44dData( imath.M44d() ) )
		self.assertFalse( t.hasAttribute( "wuh" ) )

		self.assertEqual( t.childNames(), ["s"] )
		s = t.child( "s" )
		self.assertFalse( s.hasObject() )
		self.assertFalse( s.hasAttribute( "glah" ) )

		self.assertEqual( s.readTags(), [] )

	def testCopySceneEverything( self ) :
		self.writeSCC()

		src = IECoreScene.SceneCache( SceneAlgoTest.__testFile, IECore.IndexedIO.OpenMode.Read )
		dst = IECoreScene.SceneCache( SceneAlgoTest.__testFile2, IECore.IndexedIO.OpenMode.Write )

		IECoreScene.SceneAlgo.copy( src, dst, 1, 1, 1.0, IECoreScene.SceneAlgo.ProcessFlags.All )

		del src, dst

		src = IECoreScene.SceneCache( SceneAlgoTest.__testFile2, IECore.IndexedIO.OpenMode.Read )

		self.assertEqual( src.childNames(), ['t'] )
		self.assertTrue( src.hasAttribute( "w" ) )

		t = src.child( "t" )

		self.assertEqual( t.readTransform( 1.0 ), IECore.M44dData( imath.M44d().translate( imath.V3d( 1, 0, 0 ) ) ) )
		self.assertTrue( t.hasAttribute( "wuh" ) )
		self.assertEqual( t.readAttribute( "wuh", 1.0 ), IECore.BoolData( True ) )

		self.assertEqual( t.childNames(), ["s"] )
		s = t.child( "s" )
		self.assertTrue( s.hasObject() )

		object = s.readObject( 1.0 )
		self.assertIsInstance( object, IECoreScene.SpherePrimitive )

		self.assertTrue( s.hasAttribute( "glah" ) )
		self.assertEqual( s.readAttribute( "glah", 1.0 ), IECore.IntData( 15 ) )

		self.assertTrue( "tagA" in s.readTags() )
		self.assertTrue( "tagB" in s.readTags() )


	def testMultithreadedCopy( self ):
		self.writeBigSCC()
		src = IECoreScene.SceneCache( SceneAlgoTest.__testFile, IECore.IndexedIO.OpenMode.Read )
		dst = IECoreScene.SceneCache( SceneAlgoTest.__testFile2, IECore.IndexedIO.OpenMode.Write )

		with IECore.tbb_task_scheduler_init(max_threads = 15) as taskScheduler:
			IECoreScene.SceneAlgo.copy( src, dst, 1, 1, 1.0, IECoreScene.SceneAlgo.ProcessFlags.All )

		del dst, src

		src = IECoreScene.SceneCache( SceneAlgoTest.__testFile2, IECore.IndexedIO.OpenMode.Read )

		t = src.child("t")
		self.assertEqual( len( t.childNames()), 4096 )


	def testMultithreadedRead( self ):

		self.writeBigSCC()
		src = IECoreScene.SceneCache( SceneAlgoTest.__testFile, IECore.IndexedIO.OpenMode.Read )

		for t in range(1, 16):
			# set thread
			with IECore.tbb_task_scheduler_init(max_threads = t) as taskScheduler:
				stats = IECoreScene.SceneAlgo.parallelReadAll( src, 1, 1, 1.0, IECoreScene.SceneAlgo.ProcessFlags.All )

				self.assertEqual(stats["locations"], 4096 + 2)
				self.assertEqual(stats["polygons"], 4096 * 6 ) # 4096 cubes each with 6 faces
				self.assertEqual(stats["curves"], 0)
				self.assertEqual(stats["points"], 0)
				self.assertEqual(stats["tags"], 4096 ) # default tag for polygon mesh
				self.assertEqual(stats["sets"], 0)
				self.assertEqual(stats["attributes"], 4096 * 2 )  # default attribute & custom attribute 'foo'



if __name__ == "__main__" :
	unittest.main()
