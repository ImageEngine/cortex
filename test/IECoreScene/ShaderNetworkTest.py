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
import six

import IECore
import IECoreScene

import imath

class ShaderNetworkTest( unittest.TestCase ) :

	def testParameter( self ) :

		p = IECoreScene.ShaderNetwork.Parameter()
		self.assertEqual( p.shader, "" )
		self.assertEqual( p.name, "" )
		self.assertIsInstance( p.shader, str )
		self.assertIsInstance( p.name, str )

		p = IECoreScene.ShaderNetwork.Parameter( "x" )
		self.assertEqual( p.shader, "x" )
		self.assertEqual( p.name, "" )

		p = IECoreScene.ShaderNetwork.Parameter( "x", "diffuse" )
		self.assertEqual( p.shader, "x" )
		self.assertEqual( p.name, "diffuse" )

		p.shader = "y"
		self.assertEqual( p.shader, "y" )
		p.name = "specular"
		self.assertEqual( p.name, "specular" )

		self.assertEqual(
			repr( p ),
			'Parameter( "y", "specular" )'
		)

	def testParameterAsBool( self ) :

		self.assertFalse( IECoreScene.ShaderNetwork.Parameter() )
		self.assertTrue( IECoreScene.ShaderNetwork.Parameter( "x", "" ) )
		self.assertTrue( IECoreScene.ShaderNetwork.Parameter( "x", "y" ) )
		self.assertTrue( IECoreScene.ShaderNetwork.Parameter( "", "y" ) )

	def testConnection( self ) :

		c = IECoreScene.ShaderNetwork.Connection()
		self.assertEqual( c.source.shader, "" )
		self.assertEqual( c.source.name, "" )
		self.assertEqual( c.destination.shader, "" )
		self.assertEqual( c.destination.name, "" )

		p1 = IECoreScene.ShaderNetwork.Parameter( "x", "out" )
		p2 = IECoreScene.ShaderNetwork.Parameter( "y", "in" )

		c = IECoreScene.ShaderNetwork.Connection( p1, p2 )
		self.assertEqual( c.source, p1 )
		self.assertEqual( c.destination, p2 )

		c.source.name = "out.r"
		self.assertEqual( c.source.name, "out.r" )

		self.assertEqual(
			repr( c ),
			'Connection( Parameter( "x", "out.r" ), Parameter( "y", "in" ) )'
		)

	def testConstructor( self ) :

		shaders = {
			"out" : IECoreScene.Shader( "flat", "surface" ),
			"texture" : IECoreScene.Shader( "noise", "shader" ),
		}

		connections = [
			IECoreScene.ShaderNetwork.Connection(
				IECoreScene.ShaderNetwork.Parameter(
					"texture", "outColor"
				),
				IECoreScene.ShaderNetwork.Parameter(
					"out", "Cs"
				)
			)
		]

		n = IECoreScene.ShaderNetwork(
			shaders = shaders,
			connections = connections,
			output = IECoreScene.ShaderNetwork.Parameter( "out" )
		)
		self.assertEqual( n.shaders(), shaders )
		self.assertEqual( n.inputConnections( "out" ), connections )
		self.assertEqual( n.outputConnections( "texture" ), connections )
		self.assertEqual( n.getOutput(), IECoreScene.ShaderNetwork.Parameter( "out" ) )

	def testShaderAccessors( self ) :

		n = IECoreScene.ShaderNetwork()

		s1 = IECoreScene.Shader()
		s2 = IECoreScene.Shader()

		n.addShader( "s1", s1 )
		self.assertFalse( n.getShader( "s1" ).isSame( s1 ) )
		self.assertEqual( n.getShader( "s1" ), s1 )
		self.assertEqual( n.shaders(), { "s1" : s1 } )

		n.addShader( "s2", s2 )
		self.assertEqual( n.shaders(), { "s1" : s1, "s2" : s2 } )
		self.assertFalse( n.getShader( "s2" ).isSame( s2 ) )
		self.assertEqual( n.getShader( "s2" ), s2 )

		n.removeShader( "s1" )
		with self.assertRaises( RuntimeError ) :
			n.removeShader( "s1" )
		self.assertEqual( n.shaders(), { "s2" : s2 } )

		self.assertEqual( n.getShader( "s1" ), None )

	def testOutput( self ) :

		n = IECoreScene.ShaderNetwork()
		self.assertEqual( n.getOutput(), n.Parameter() )

		s1 = IECoreScene.Shader( "constant" )
		s2 = IECoreScene.Shader( "noise" )

		n.addShader( "s1", s1 )
		self.assertEqual( n.getOutput(), n.Parameter() )
		self.assertEqual( n.outputShader(), None )

		n.addShader( "s2", s2 )
		self.assertEqual( n.getOutput(), n.Parameter() )
		self.assertEqual( n.outputShader(), None )

		n.setOutput( n.Parameter( "s1", "" ) )
		self.assertEqual( n.getOutput(), n.Parameter( "s1", "" ) )
		self.assertEqual( n.outputShader(), s1 )

		n.removeShader( "s1" )
		self.assertEqual( n.getOutput(), n.Parameter() )
		self.assertEqual( n.outputShader(), None )

		n.setOutput( n.Parameter( "s2", "" ) )
		self.assertEqual( n.getOutput(), n.Parameter( "s2", "" ) )
		self.assertEqual( n.outputShader(), s2 )

		with six.assertRaisesRegex( self, RuntimeError, "Output shader \"s1\" not in network" ) :
			n.setOutput( n.Parameter( "s1", "" ) )

	def testAddAndRemoveConnection( self ) :

		n = IECoreScene.ShaderNetwork()

		s1 = IECoreScene.Shader()
		s2 = IECoreScene.Shader()

		c = IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter(
				"s1", "out",
			),
			IECoreScene.ShaderNetwork.Parameter(
				"s2", "in",
			),
		)

		with six.assertRaisesRegex( self, RuntimeError, "Source shader \"s1\" not in network" ) :
			n.addConnection( c )

		n.addShader( "s1", s1 )
		self.assertEqual( n.inputConnections( "s1" ), [] )
		self.assertEqual( n.outputConnections( "s1" ), [] )

		with six.assertRaisesRegex( self, RuntimeError, "Destination shader \"s2\" not in network" ) :
			n.addConnection( c )

		n.addShader( "s2", s2 )
		self.assertEqual( n.inputConnections( "s2" ), [] )
		self.assertEqual( n.outputConnections( "s2" ), [] )
		self.assertFalse( n.input( c.destination ) )

		n.addConnection( c )
		self.assertEqual( n.inputConnections( "s2" ), [ c ] )
		self.assertEqual( n.outputConnections( "s1" ), [ c ] )
		self.assertEqual( n.input( c.destination ), c.source )

		n.removeConnection( c )
		self.assertEqual( n.inputConnections( "s2" ), [] )
		self.assertEqual( n.outputConnections( "s1" ), [] )
		self.assertFalse( n.input( c.destination ) )

		with six.assertRaisesRegex( self, RuntimeError, "Connection \"s1.out -> s2.in\" not in network" ) :
			n.removeConnection( c )

	def testRemovingSourceShaderRemovesConnections( self ) :

		n = IECoreScene.ShaderNetwork()
		n.addShader( "s1", IECoreScene.Shader() )
		n.addShader( "s2", IECoreScene.Shader() )

		c = n.Connection( n.Parameter( "s1", "out" ), n.Parameter( "s2", "in" ) )
		n.addConnection( c )
		self.assertEqual( n.inputConnections( "s2" ), [ c ] )
		self.assertEqual( n.outputConnections( "s1" ), [ c ] )

		n.removeShader( "s1" )
		self.assertEqual( n.inputConnections( "s2" ), [] )

	def testRemovingDestinationShaderRemovesConnections( self ) :

		n = IECoreScene.ShaderNetwork()
		n.addShader( "s1", IECoreScene.Shader() )
		n.addShader( "s2", IECoreScene.Shader() )

		c = n.Connection( n.Parameter( "s1", "out" ), n.Parameter( "s2", "in" ) )
		n.addConnection( c )
		self.assertEqual( n.inputConnections( "s2" ), [ c ] )
		self.assertEqual( n.outputConnections( "s1" ), [ c ] )

		n.removeShader( "s2" )
		self.assertEqual( n.outputConnections( "s1" ), [] )

	def testHash( self ) :

		hashes = set()
		def assertHashUnique( s ) :
			h = s.hash()
			self.assertNotIn( h, hashes )
			hashes.add( h )

		n = IECoreScene.ShaderNetwork()
		assertHashUnique( n )

		s1 = IECoreScene.Shader()
		s2 = IECoreScene.Shader()

		n.addShader( "s1", s1 )
		assertHashUnique( n )

		n.addShader( "s2", s2 )
		assertHashUnique( n )

		n.addConnection(
			IECoreScene.ShaderNetwork.Connection(
				IECoreScene.ShaderNetwork.Parameter(
					"s2", "out",
				),
				IECoreScene.ShaderNetwork.Parameter(
					"s1", "in",
				),
			)
		)
		assertHashUnique( n )

		n.setOutput( n.Parameter( "s2" ) )
		assertHashUnique( n )

	def testHashIsIndependentOfConstructionOrder( self ) :

		s1 = IECoreScene.Shader( "constant", "surface" )
		s2 = IECoreScene.Shader( "noise", "shader" )

		n1 = IECoreScene.ShaderNetwork()
		n2 = IECoreScene.ShaderNetwork()
		self.assertEqual( n1.hash(), n2.hash() )

		n1.addShader( "s1", s1 )
		n2.addShader( "s2", s2  )
		self.assertNotEqual( n1.hash(), n2.hash() )

		n1.addShader( "s2", s2 )
		n2.addShader( "s1", s1 )
		self.assertEqual( n1.hash(), n2.hash() )

		n2.setOutput( n2.Parameter( "s1" ) )
		self.assertNotEqual( n1.hash(), n2.hash() )
		n1.setOutput( n1.Parameter( "s1" ) )
		self.assertEqual( n1.hash(), n2.hash() )

		c = IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter(
				"s2", "",
			),
			IECoreScene.ShaderNetwork.Parameter(
				"s1", "Cs",
			),
		)

		n1.addConnection( c )
		self.assertNotEqual( n1.hash(), n2.hash() )

		n2.addConnection( c )
		self.assertEqual( n1.hash(), n2.hash() )

	def testHashRepeatability( self ) :

		s = [
			IECoreScene.ShaderNetwork( shaders = { "flat" : IECoreScene.Shader( "flat" ) } )
			for i in range( 0, 10 )
		]

		self.assertEqual( len( { x.hash().toString() for x in s } ), 1 )

	def testEquality( self ) :

		s1 = IECoreScene.Shader( "constant", "surface" )
		s2 = IECoreScene.Shader( "noise", "shader" )

		n1 = IECoreScene.ShaderNetwork()
		n2 = IECoreScene.ShaderNetwork()
		self.assertEqual( n1, n2 )

		n1.addShader( "s1", s1 )
		n2.addShader( "s2", s2 )
		self.assertNotEqual( n1, n2 )

		n1.addShader( "s2", s2 )
		n2.addShader( "s1", s1 )
		self.assertEqual( n1, n2 )

		n2.setOutput( n2.Parameter( "s1" ) )
		self.assertNotEqual( n1, n2 )
		n1.setOutput( n2.Parameter( "s1" ) )
		self.assertEqual( n1, n2 )

		c = IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter(
				"s2", "",
			),
			IECoreScene.ShaderNetwork.Parameter(
				"s1", "Cs",
			),
		)

		n1.addConnection( c )

		# Ensure equality is order independent, as we compare lists,
		# we need to ensure n2 being a super-set of n1 doesn't pass.
		self.assertNotEqual( n1, n2 )
		self.assertNotEqual( n2, n1 )

		n2.addConnection( c )
		self.assertEqual( n1, n2 )

	def testEqualityIgnoresShaderIdentity( self ) :

		s1 = IECoreScene.Shader( "constant", "surface" )
		s2 = IECoreScene.Shader( "constant", "surface" )
		self.assertEqual( s1, s2 )

		n1 = IECoreScene.ShaderNetwork( { "s" : s1 } )
		n2 = IECoreScene.ShaderNetwork( { "s" : s2 } )

		self.assertEqual( n1, n2 )

	def testCopy( self ) :

		n1 = IECoreScene.ShaderNetwork(
			shaders = {
				"s1" : IECoreScene.Shader( "constant", "surface" ),
				"s2" : IECoreScene.Shader( "noise", "shader" ),
			},
			connections = [
				IECoreScene.ShaderNetwork.Connection(
					IECoreScene.ShaderNetwork.Parameter(
						"s2", "",
					),
					IECoreScene.ShaderNetwork.Parameter(
						"s1", "Cs",
					),
				)
			]
		)

		n2 = n1.copy()
		self.assertEqual( n1, n2 )

	def testSerialisation( self ) :

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"s1" : IECoreScene.Shader( "constant", "surface" ),
				"s2" : IECoreScene.Shader( "noise", "shader" ),
			},
			connections = [
				IECoreScene.ShaderNetwork.Connection(
					IECoreScene.ShaderNetwork.Parameter(
						"s2", "",
					),
					IECoreScene.ShaderNetwork.Parameter(
						"s1", "Cs",
					),
				)
			],
			output = IECoreScene.ShaderNetwork.Parameter( "s1" )
		)

		m = IECore.MemoryIndexedIO( IECore.CharVectorData(), [], IECore.IndexedIO.OpenMode.Append )
		n.save( m, "test" )
		self.assertEqual( IECore.Object.load( m, "test" ), n )

	def testPassParameterAsTuple( self ) :

		c = IECoreScene.ShaderNetwork.Connection( ( "fromShader", "fromName" ), ( "toShader", "toName" ) )
		self.assertEqual( c.source, IECoreScene.ShaderNetwork.Parameter( "fromShader", "fromName" ) )
		self.assertEqual( c.destination, IECoreScene.ShaderNetwork.Parameter( "toShader", "toName" ) )

	def testPassConnectionAsTuple( self ) :

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"s1" : IECoreScene.Shader( "constant", "surface" ),
				"s2" : IECoreScene.Shader( "noise", "shader" ),
			},
			connections = [
				( ( "s2", "" ), ( "s1", "Cs" ) ),
			],
			output = IECoreScene.ShaderNetwork.Parameter( "s1" )
		)

	def testShaderImmutability( self ) :

		s = IECoreScene.Shader( "constant", "surface" )
		n = IECoreScene.ShaderNetwork(
			shaders = {
				"s" : s
			}
		)

		def assertImmutable( network, handle, shader ) :

			self.assertFalse( network.getShader( handle ).isSame( shader ) )
			self.assertEqual( network.getShader( handle ), shader )

			shader.parameters["Cs"] = imath.Color3f( 1, 0, 0 )
			self.assertFalse( network.getShader( handle ).isSame( shader ) )
			self.assertNotEqual( network.getShader( handle ), shader )

		# The ShaderNetwork should have taken a copy of the shader,
		# so that we can no longer modify the network by modifying s.

		assertImmutable( n, "s", s )

		# The same applies when adding shaders via `addShader()` and
		# `setShader()`.

		s2 = IECoreScene.Shader( "constant", "surface" )
		n.addShader( "s2", s2 )
		assertImmutable( n, "s2", s2 )

		s3 = IECoreScene.Shader( "constant", "surface" )
		n.setShader( "s3", s3 )
		assertImmutable( n, "s3", s3 )

	def testAddShaderReturnValue( self ) :

		s1 = IECoreScene.Shader()
		s2 = IECoreScene.Shader()

		n = IECoreScene.ShaderNetwork()
		n.setShader( "s", s1 )

		h = n.addShader( "s", s2 )
		self.assertEqual( h, "s1" )
		self.assertIsInstance( h, str )

	def testMove( self ) :

		IECoreScene.testShaderNetworkMove()

	def testUniqueHandles( self ) :

		n = IECoreScene.ShaderNetwork()
		for i in range( 0, 20 ) :
			n.addShader( "test", IECoreScene.Shader() )

		self.assertEqual(
			set( n.shaders().keys() ),
			{ "test" } | { "test{0}".format( x ) for x in range( 1, 20 ) }
		)

	def testSubstitutions( self ):
		def runSubstitutionTest( shader, attributes ):
			n = IECoreScene.ShaderNetwork( shaders = { "s" : s } )
			a = IECore.CompoundObject( attributes )
			h = IECore.MurmurHash()
			n.hashSubstitutions( a, h )
			nSubst = n.copy()
			nSubst.applySubstitutions( a )
			return ( h, nSubst.getShader("s") )

		s = IECoreScene.Shader( "test", "surface",IECore.CompoundData( {
			"a" : IECore.StringData( "foo" ),
			"b" : IECore.FloatData( 42.42 ),
			"c" : IECore.StringVectorData( [ "foo", "bar" ] ),
		} ) )

		( h, sSubst ) = runSubstitutionTest( s, { "unused" : IECore.StringData( "blah" ) } )
		self.assertEqual( h, IECore.MurmurHash() )
		self.assertEqual( s, sSubst )

		s = IECoreScene.Shader( "test", "surface",IECore.CompoundData( {
			"a" : IECore.StringData( "pre<attr:fred>post" ),
			"b" : IECore.FloatData( 42.42 ),
			"c" : IECore.StringVectorData( [ "<attr:bob>", "pre<attr:carol>", "<attr:fred>post", "<attr:bob><attr:carol> <attr:fred>" ] ),
		} ) )
		( h, sSubst ) = runSubstitutionTest( s, { "unused" : IECore.StringData( "blah" ) } )
		# Now that we've got substitutions, the hash should be non-default
		self.assertNotEqual( h, IECore.MurmurHash() )

		# Everything gets substituted to empty, because no matching attributes provided
		self.assertNotEqual( s, sSubst )
		self.assertEqual( sSubst.parameters["a"].value, "prepost" )
		self.assertEqual( sSubst.parameters["c"][0], "" )
		self.assertEqual( sSubst.parameters["c"][1], "pre" )
		self.assertEqual( sSubst.parameters["c"][2], "post" )
		self.assertEqual( sSubst.parameters["c"][3], " " )

		( h2, sSubst2 ) = runSubstitutionTest( s, { "unused" : IECore.StringData( "blah2" ) } )
		# The attribute being changed has no impact
		self.assertEqual( h, h2 )
		self.assertEqual( sSubst, sSubst2 )

		( h3, sSubst3 ) = runSubstitutionTest( s, { "fred" : IECore.StringData( "CAT" ) } )
		self.assertNotEqual( h, h3 )
		self.assertNotEqual( s, sSubst3 )
		self.assertEqual( sSubst3.parameters["a"].value, "preCATpost" )
		self.assertEqual( sSubst3.parameters["c"][0], "" )
		self.assertEqual( sSubst3.parameters["c"][1], "pre" )
		self.assertEqual( sSubst3.parameters["c"][2], "CATpost" )
		self.assertEqual( sSubst3.parameters["c"][3], " CAT" )

		( h4, sSubst4 ) = runSubstitutionTest( s, { "fred" : IECore.StringData( "FISH" ) } )
		self.assertNotEqual( h3, h4 )
		self.assertEqual( sSubst4.parameters["c"][2], "FISHpost" )

		allAttributes = {
			"fred" : IECore.StringData( "FISH" ),
			"bob" : IECore.StringData( "CAT" ),
			"carol" : IECore.StringData( "BIRD" )
		}
		( h5, sSubst5 ) = runSubstitutionTest( s, allAttributes )
		self.assertNotEqual( h4, h5 )
		self.assertEqual( sSubst5.parameters["a"].value, "preFISHpost" )
		self.assertEqual( sSubst5.parameters["c"][0], "CAT" )
		self.assertEqual( sSubst5.parameters["c"][1], "preBIRD" )
		self.assertEqual( sSubst5.parameters["c"][2], "FISHpost" )
		self.assertEqual( sSubst5.parameters["c"][3], "CATBIRD FISH" )

		# Support a variety of different ways of using backslashes to escape substitutions
		s = IECoreScene.Shader( "test", "surface",IECore.CompoundData( {
			"a" : IECore.StringData( r"pre\<attr:fred\>post" ),
			"b" : IECore.FloatData( 42.42 ),
			"c" : IECore.StringVectorData( [ r"\<attr:bob\>", r"\<attr:carol>", r"<attr:fred\>" ] ),
		} ) )
		( h6, sSubst6 ) = runSubstitutionTest( s, {} )
		( h7, sSubst7 ) = runSubstitutionTest( s, allAttributes )
		self.assertEqual( h6, h7 )
		self.assertEqual( sSubst6, sSubst7 )
		self.assertEqual( sSubst6.parameters["a"].value, "pre<attr:fred>post" )
		self.assertEqual( sSubst6.parameters["c"][0], "<attr:bob>" )
		self.assertEqual( sSubst6.parameters["c"][1], "<attr:carol>" )
		self.assertEqual( sSubst6.parameters["c"][2], "<attr:fred>" )

if __name__ == "__main__":
	unittest.main()
