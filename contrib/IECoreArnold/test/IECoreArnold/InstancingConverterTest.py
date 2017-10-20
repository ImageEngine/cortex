##########################################################################
#
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import unittest
import ctypes
import threading
import random

import arnold

import IECore
import IECoreArnold

class InstancingConverterTest( unittest.TestCase ) :

	def test( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			c = IECoreArnold.InstancingConverter()

			m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			m2 = m1.copy()
			m3 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )

			am1 = c.convert( m1, "testMesh" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( am1 ) ), "polymesh" )

			am2 = c.convert( m2, "testInstance" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( am2 ) ), "ginstance" )
			self.assertEqual( arnold.AiNodeGetPtr( am2, "node" ), ctypes.addressof( am1.contents ) )

			am3 = c.convert( m3, "testMesh2" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( am3 ) ), "polymesh" )

	def testThreading( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			converter = IECoreArnold.InstancingConverter()

			meshes = []
			for i in range( 0, 1000 ) :
				meshes.append( IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -i ), IECore.V2f( i ) ) ) )

			def f( nodeList ) :
				for i in range( 0, 10000 ) :
					nodeList.append( converter.convert( random.choice( meshes ), "testMesh" ) )

			nodeLists = []
			threads = []
			for i in range( 0, 10 ) :
				nodeList = []
				nodeLists.append( nodeList )
				t = threading.Thread( target = f, args = ( nodeList, ) )
				threads.append( t )
				t.start()

			for t in threads :
				t.join()

			numPolyMeshNodes = 0
			numInstanceNodes = 0
			polyMeshAddresses = set()
			instancedNodeAddresses = []
			for nodeList in nodeLists :

				self.assertEqual( len( nodeList ), 10000 )

				for node in nodeList :

					nodeType = arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( node ) )
					self.failUnless( nodeType in ( "ginstance", "polymesh" ) )

					if nodeType == "ginstance" :
						numInstanceNodes += 1
						instancedNodeAddresses.append( arnold.AiNodeGetPtr( node, "node" ) )
					else :
						numPolyMeshNodes += 1
						polyMeshAddresses.add( ctypes.addressof( node.contents ) )

			self.assertEqual( numInstanceNodes + numPolyMeshNodes, 10000 * 10 )
			for address in instancedNodeAddresses :
				self.failUnless( address in polyMeshAddresses )

	def testUserSuppliedHash( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			c = IECoreArnold.InstancingConverter()

			m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			m2 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )

			h1 = IECore.MurmurHash()
			h2 = IECore.MurmurHash()

			h1.append( 10 )
			h2.append( 10 )

			am1a = c.convert( m1, h1, "testMesh" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( am1a ) ), "polymesh" )
			am1b = c.convert( m1, h1, "testInstance" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( am1b ) ), "ginstance" )
			self.assertEqual( arnold.AiNodeGetPtr( am1b, "node" ), ctypes.addressof( am1a.contents ) )

			am2a = c.convert( m2, h2, "testMesh" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( am2a ) ), "polymesh" )
			am2b = c.convert( m2, h2, "testInstance" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( am2b ) ), "ginstance" )
			self.assertEqual( arnold.AiNodeGetPtr( am2b, "node" ), ctypes.addressof( am2a.contents ) )

	def testMotion( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			c = IECoreArnold.InstancingConverter()

			m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			m2 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )

			n1 = c.convert( [ m1, m2 ], -0.25, 0.25, "testMesh" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n1 ) ), "polymesh" )
			self.assertEqual( arnold.AiArrayGetNumKeys( arnold.AiNodeGetArray( n1, "vlist" ).contents ), 2 )

			n2 = c.convert( [ m1, m2 ], -0.25, 0.25, "testInstance" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n2 ) ), "ginstance" )
			self.assertEqual( arnold.AiNodeGetPtr( n2, "node" ), ctypes.addressof( n1.contents ) )

			n3 = c.convert( [ m2, m1 ], -0.25, 0.25, "testMesh" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n1 ) ), "polymesh" )
			self.assertEqual( arnold.AiArrayGetNumKeys( arnold.AiNodeGetArray( n1, "vlist" ).contents ), 2 )

			n4 = c.convert( [ m1, m2 ], -0.5, 0.5, "testInstance" )
			self.assertEqual( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n1 ) ), "polymesh" )
			self.assertEqual( arnold.AiArrayGetNumKeys( arnold.AiNodeGetArray( n1, "vlist" ).contents ), 2 )

if __name__ == "__main__":
    unittest.main()
