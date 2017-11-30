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
import IECoreScene

class SceneInterfaceTest( unittest.TestCase ) :

	__testFile = "/tmp/test.scc"

	def writeSCC( self ) :

		m = IECoreScene.SceneCache( SceneInterfaceTest.__testFile, IECore.IndexedIO.OpenMode.Write )
		m.writeAttribute( "w", IECore.BoolData( True ), 1.0 )

		t = m.createChild( "t" )
		t.writeTransform( IECore.M44dData(IECore.M44d.createTranslated(IECore.V3d( 1, 0, 0 ))), 1.0 )
		t.writeAttribute( "wuh", IECore.BoolData( True ), 1.0 )

		s = t.createChild( "s" )
		s.writeObject( IECoreScene.SpherePrimitive( 1 ), 1.0 )
		s.writeAttribute( "glah", IECore.BoolData( True ), 1.0 )

	def testGet( self ) :

		self.writeSCC()

		instance1 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		instance2 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )

		self.assertTrue( instance1.isSame( instance2 ) )

		instance3 = IECoreScene.SceneInterface.create( SceneInterfaceTest.__testFile, IECore.IndexedIO.OpenMode.Read )
		instance4 = IECoreScene.SceneInterface.create( SceneInterfaceTest.__testFile, IECore.IndexedIO.OpenMode.Read )

		self.assertFalse( instance3.isSame( instance4 ) )
		self.assertFalse( instance3.isSame( instance1 ) )
		self.assertFalse( instance3.isSame( instance2 ) )

	def testErase( self ) :

		self.writeSCC()

		instance1 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		instance2 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		self.assertTrue( instance1.isSame( instance2 ) )

		IECoreScene.SharedSceneInterfaces.erase( SceneInterfaceTest.__testFile )

		instance3 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		self.assertFalse( instance3.isSame( instance1 ) )

		instance4 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		self.assertFalse( instance4.isSame( instance1 ) )
		self.assertTrue( instance4.isSame( instance3 ) )

	def testClear( self ) :

		self.writeSCC()

		instance1 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		instance2 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		self.assertTrue( instance1.isSame( instance2 ) )

		IECoreScene.SharedSceneInterfaces.clear()

		instance3 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		self.assertFalse( instance3.isSame( instance1 ) )

		instance4 = IECoreScene.SharedSceneInterfaces.get( SceneInterfaceTest.__testFile )
		self.assertFalse( instance4.isSame( instance1 ) )
		self.assertTrue( instance4.isSame( instance3 ) )

	def testVisibilityName( self ) :
		self.assertEqual( IECoreScene.SceneInterface.visibilityName, "scene:visible" )

if __name__ == "__main__":
	unittest.main()

