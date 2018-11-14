##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
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

import IECoreScene

class ShaderNetworkAlgoTest( unittest.TestCase ) :

	def testAddShaders( self ) :

		n1 = IECoreScene.ShaderNetwork(
			shaders = {
				"out" : IECoreScene.Shader( "lambert", "surface" ),
				"texture" : IECoreScene.Shader( "file", "shader" ),
			},
			output = ( "out", "" )
		)

		n2 = IECoreScene.ShaderNetwork(
			shaders = {
				"manifold" : IECoreScene.Shader( "uv", "shader" ),
				"texture" : IECoreScene.Shader( "noise", "shader" ),
			},
			connections = [
				( ( "manifold", "" ), ( "texture", "manifold" ) ),
			],
			output = ( "texture", "" )
		)

		c = n1.copy()
		p = IECoreScene.ShaderNetworkAlgo.addShaders( c, n2 )
		self.assertEqual( p, IECoreScene.ShaderNetwork.Parameter( "texture1", "" ) )

		self.assertEqual(
			c.shaders(),
			{
				"out" : n1.getShader( "out" ),
				"texture" : n1.getShader( "texture" ),
				"manifold" : n2.getShader( "manifold" ),
				"texture1" : n2.getShader( "texture" ),
			}
		)

		self.assertEqual(
			c.inputConnections( "texture1" ),
			[ c.Connection( ( "manifold", "" ), ( "texture1", "manifold" ) ) ],
		)

	def testRemoveUnusedShaders( self ) :

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"used1" : IECoreScene.Shader(),
				"used2" : IECoreScene.Shader(),
				"used3" : IECoreScene.Shader(),
				"unused1" : IECoreScene.Shader(),
				"unused2" : IECoreScene.Shader(),
				"unused3" : IECoreScene.Shader(),
			},
			connections = [
				( ( "used1", "out" ), ( "used2", "in" ) ),
				( ( "used2", "out" ), ( "used3", "in" ) ),
				( ( "unused1", "out" ), ( "unused2", "in" ) ),
				( ( "unused2", "out" ), ( "unused3", "in" ) ),
			],
			output = ( "used3", "" ),
		)

		IECoreScene.ShaderNetworkAlgo.removeUnusedShaders( n )
		self.assertEqual( set( n.shaders().keys() ), { "used1", "used2", "used3" } )

if __name__ == "__main__":
	unittest.main()
