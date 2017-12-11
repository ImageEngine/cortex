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

import unittest
import imath

import IECore
import IECoreScene
import IECoreGL

IECoreGL.init( False )

class InstancingTest( unittest.TestCase ) :

	class RandomMeshProcedural( IECoreScene.Renderer.Procedural ) :

		def __init__( self, meshes, name="/1", depth=0, maxDepth=8 ) :

			IECoreScene.Renderer.Procedural.__init__( self )

			self.__meshes = meshes
			self.__depth = depth
			self.__maxDepth = maxDepth
			self.__name = name

		def bound( self ) :

			b = imath.Box3f()
			for m in self.__meshes :
				b.extendBy( m.bound() )
			return b

		def render( self, renderer ) :

			with IECoreScene.AttributeBlock( renderer ) :

				renderer.setAttribute( "name", IECore.StringData( self.__name ) )

				if self.__depth < self.__maxDepth :

					for n in ( "1", "2" ) :
						renderer.procedural(
							InstancingTest.RandomMeshProcedural(
								self.__meshes,
								self.__name + "/" + n,
								self.__depth + 1,
								self.__maxDepth,
							)
						)

				else :

					mesh = self.__meshes[ int( self.__name.split( "/" )[-1] ) - 1 ]
					mesh.render( renderer )

		def hash( self ):

			h = IECore.MurmurHash()
			return h

	def __collectMeshes( self, group, result ) :

		name = group.getState().get( IECoreGL.NameStateComponent.staticTypeId() )

		for c in group.children() :
			if isinstance( c, IECoreGL.Group ) :
				self.__collectMeshes( c, result )
			else :
				d = result.setdefault( name.name().split( "/" )[-1], [] )
				d.append( c )

	def testAutomaticInstancingOn( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		m2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )

		with IECoreScene.WorldBlock( r ) :

			r.procedural( self.RandomMeshProcedural( [ m1, m2 ] ) )

		meshes = {}
		self.__collectMeshes( r.scene().root(), meshes )

		for meshList in meshes.values() :
			for i in range( 0, len( meshList ) ) :
				self.failUnless( meshList[i].isSame( meshList[0] ) )

	def testAutomaticInstancingOff( self ) :

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		m2 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )

		with IECoreScene.WorldBlock( r ) :

			r.setAttribute( "automaticInstancing", IECore.BoolData( False ) )

			r.procedural( self.RandomMeshProcedural( [ m1, m2 ] ) )

		meshes = {}
		self.__collectMeshes( r.scene().root(), meshes )

		for meshList in meshes.values() :
			for i in range( 1, len( meshList ) ) :
				self.failIf( meshList[i].isSame( meshList[0] ) )

if __name__ == "__main__":
    unittest.main()
