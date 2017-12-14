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

import ctypes
import unittest

import arnold
import imath

import IECore
import IECoreScene
import IECoreArnold

class AutomaticInstancingTest( unittest.TestCase ) :

	def __allNodes( self, type = arnold.AI_NODE_ALL, ignoreRoot = True ) :

		result = []
		i = arnold.AiUniverseGetNodeIterator( type )
		while not arnold.AiNodeIteratorFinished( i ) :
			node = arnold.AiNodeIteratorGetNext( i )
			if ignoreRoot and arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( node ) ) == "list_aggregate" and arnold.AiNodeGetName( node ) == "root" :
				continue
			result.append( node )

		return result

	def testOnByDefault( self ) :

		r = IECoreArnold.Renderer()

		with IECoreScene.WorldBlock( r ) :

			m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

			m.render( r )
			m.render( r )

			nodes = self.__allNodes( type = arnold.AI_NODE_SHAPE )
			self.assertEqual( len( nodes ), 2 )

			nodeTypes = [ arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n ) ) for n in nodes ]
			mesh = nodes[nodeTypes.index( "polymesh" )]
			instance = nodes[nodeTypes.index( "ginstance" )]

			self.assertEqual( arnold.AiNodeGetPtr( instance, "node" ), ctypes.addressof( mesh.contents ) )

	def testEnabling( self ) :

		r = IECoreArnold.Renderer()

		with IECoreScene.WorldBlock( r ) :

			r.setAttribute( "ai:automaticInstancing", IECore.BoolData( True ) )

			m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

			m.render( r )
			m.render( r )

			nodes = self.__allNodes( type = arnold.AI_NODE_SHAPE )
			self.assertEqual( len( nodes ), 2 )

			nodeTypes = [ arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n ) ) for n in nodes ]
			mesh = nodes[nodeTypes.index( "polymesh" )]
			instance = nodes[nodeTypes.index( "ginstance" )]

			self.assertEqual( arnold.AiNodeGetPtr( instance, "node" ), ctypes.addressof( mesh.contents ) )

	def testDisabling( self ) :

		r = IECoreArnold.Renderer()

		with IECoreScene.WorldBlock( r ) :

			r.setAttribute( "ai:automaticInstancing", IECore.BoolData( False ) )

			m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

			m.render( r )
			m.render( r )

			nodes = self.__allNodes( type = arnold.AI_NODE_SHAPE )
			self.assertEqual( len( nodes ), 2 )

			nodeTypes = [ arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n ) ) for n in nodes ]
			self.assertEqual( nodeTypes, [ "polymesh", "polymesh" ] )

	def testProceduralsShareInstances( self ) :

		class PlaneProcedural( IECoreScene.Renderer.Procedural ) :

			def __init__( self ) :

				IECoreScene.Renderer.Procedural.__init__( self )

			def bound( self ) :

				return imath.Box3f( imath.V3f( -10, -10, -0.01 ), imath.V3f( 10, 10, 0.01 ) )

			def render( self, renderer ) :

				IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -10 ), imath.V2f( 10 ) ) ).render( renderer )

			def hash( self ):

				h = IECore.MurmurHash()
				return h

		def arnoldMessageCallback( logMask, severity, msg, tabs ) :

			self.__arnoldMessages.append( msg )

		r = IECoreArnold.Renderer()

		messageCallback = arnold.AtMsgCallBack( arnoldMessageCallback )
		arnold.AiMsgSetCallback( messageCallback )
		self.__arnoldMessages = []
		arnold.AiMsgSetConsoleFlags( arnold.AI_LOG_ALL )

		with IECoreScene.WorldBlock( r ) :

			r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )

			for i in range( 0, 100 ) :
				r.procedural( PlaneProcedural() )

		# we can't check for the existence of ginstances by examining the nodes after rendering,
		# because it seems that after rendering, arnold reports the type of ginstances
		# as being the type of the thing they point to, rather than "ginstance". so instead we
		# check for evidence in the log.

		polyMeshStats = [ m for m in self.__arnoldMessages if m.startswith( "polymeshes" ) ][0]
		self.failUnless( "99" in polyMeshStats )

		# check that there are no bounding box warnings
		boundingBoxWarnings = [ m for m in self.__arnoldMessages if "bounding box" in m ]
		self.assertEqual( len( boundingBoxWarnings ), 0 )

if __name__ == "__main__":
    unittest.main()
