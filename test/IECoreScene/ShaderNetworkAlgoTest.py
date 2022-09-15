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

import imath

import IECore
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

	def testConvertColorComponentOutputConnection( self ) :

		# OSL < 1.10

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"texture" : IECoreScene.Shader( "noise", "osl:shader" ),
				"surface" : IECoreScene.Shader( "plastic", "osl:surface" ),
			},
			connections = [
				( ( "texture", "out.r" ), ( "surface", "Kd" ) ),
				( ( "texture", "out.g" ), ( "surface", "Ks" ) ),
				( ( "texture", "out.g" ), ( "surface", "Kt" ) ),
			],
			output = "surface"
		)

		self.assertEqual( len( n ), 2 )

		IECoreScene.ShaderNetworkAlgo.convertOSLComponentConnections( n )
		self.assertEqual( len( n ), 4 )

		self.assertEqual( len( n.inputConnections( "surface" ) ), 3 )
		self.assertEqual( len( n.inputConnections( "texture" ) ), 0 )

		kdInput = n.input( ( "surface", "Kd" ) )
		self.assertEqual( kdInput.name, "out" )
		self.assertEqual( n.getShader( kdInput.shader ).type, "osl:shader" )
		self.assertEqual( n.getShader( kdInput.shader ).name, "MaterialX/mx_swizzle_color_float" )
		self.assertEqual( n.getShader( kdInput.shader ).parameters["channels"].value, "r" )

		ksInput = n.input( ( "surface", "Ks" ) )
		self.assertEqual( ksInput.name, "out" )
		self.assertEqual( n.getShader( ksInput.shader ).type, "osl:shader" )
		self.assertEqual( n.getShader( ksInput.shader ).name, "MaterialX/mx_swizzle_color_float" )
		self.assertEqual( n.getShader( ksInput.shader ).parameters["channels"].value, "g" )

		self.assertEqual( n.input( ( "surface", "Kt" ) ), ksInput )

		# OSL > 1.10

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"texture" : IECoreScene.Shader( "noise", "osl:shader" ),
				"surface" : IECoreScene.Shader( "plastic", "osl:surface" ),
			},
			connections = [
				( ( "texture", "out.r" ), ( "surface", "Kd" ) ),
				( ( "texture", "out.g" ), ( "surface", "Ks" ) ),
				( ( "texture", "out.g" ), ( "surface", "Kt" ) ),
			],
			output = "surface"
		)

		self.assertEqual( len( n ), 2 )

		IECoreScene.ShaderNetworkAlgo.convertOSLComponentConnections( n, 11000 )
		self.assertEqual( len( n ), 2 )

		self.assertEqual( len( n.inputConnections( "surface" ) ), 3 )
		self.assertEqual( len( n.inputConnections( "texture" ) ), 0 )

		self.assertEqual( n.input( ( "surface", "Kd" ) ), ( "texture", "out[0]" ) )
		self.assertEqual( n.input( ( "surface", "Ks" ) ), ( "texture", "out[1]" ) )
		self.assertEqual( n.input( ( "surface", "Kt" ) ), ( "texture", "out[1]" ) )

	def testConvertColorComponentInputConnection( self ) :

		# OSL < 1.10

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"texture1" : IECoreScene.Shader( "floatNoise", "osl:shader" ),
				"texture2" : IECoreScene.Shader( "floatNoise", "osl:shader" ),
				"surface" : IECoreScene.Shader(
					"plastic", "osl:surface",
					parameters = { "Cs" : imath.Color3f( 0.2, 0.3, 0.4 ) }
				)
			},
			connections = [
				( ( "texture1", "out" ), ( "surface", "Cs.r" ) ),
				( ( "texture2", "out" ), ( "surface", "Cs.b" ) ),
			],
			output = "surface"
		)

		self.assertEqual( len( n ), 3 )

		IECoreScene.ShaderNetworkAlgo.convertOSLComponentConnections( n )
		self.assertEqual( len( n ), 4 )

		self.assertFalse( n.input( ( "surface", "Cs.r" ) ) )
		self.assertFalse( n.input( ( "surface", "Cs.g" ) ) )
		self.assertFalse( n.input( ( "surface", "Cs.b" ) ) )

		csInput = n.input( ( "surface", "Cs" ) )
		self.assertEqual( csInput.name, "out" )

		packShader = n.getShader( csInput.shader )
		self.assertEqual( packShader.name, "MaterialX/mx_pack_color" )
		self.assertEqual( packShader.type, "osl:shader" )

		self.assertEqual( packShader.parameters["in1"], IECore.FloatData( 0.2 ) )
		self.assertEqual( packShader.parameters["in2"], IECore.FloatData( 0.3 ) )
		self.assertEqual( packShader.parameters["in3"], IECore.FloatData( 0.4 ) )

		self.assertEqual( n.input( ( csInput.shader, "in1" ) ), ( "texture1", "out" ) )
		self.assertEqual( n.input( ( csInput.shader, "in2" ) ), ( "", "" ) )
		self.assertEqual( n.input( ( csInput.shader, "in3" ) ), ( "texture2", "out" ) )

		# OSL > 1.10

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"texture1" : IECoreScene.Shader( "floatNoise", "osl:shader" ),
				"texture2" : IECoreScene.Shader( "floatNoise", "osl:shader" ),
				"surface" : IECoreScene.Shader(
					"plastic", "osl:surface",
					parameters = { "Cs" : imath.Color3f( 0.2, 0.3, 0.4 ) }
				)
			},
			connections = [
				( ( "texture1", "out" ), ( "surface", "Cs.r" ) ),
				( ( "texture2", "out" ), ( "surface", "Cs.b" ) ),
			],
			output = "surface"
		)

		self.assertEqual( len( n ), 3 )

		IECoreScene.ShaderNetworkAlgo.convertOSLComponentConnections( n, 11000 )
		self.assertEqual( len( n ), 3 )

		self.assertEqual( n.input( ( "surface", "Cs[0]" ) ), ( "texture1", "out" ) )
		self.assertEqual( n.input( ( "surface", "Cs[2]" ) ), ( "texture2", "out" ) )

		self.assertFalse( n.input( ( "surface", "Cs.r" ) ) )
		self.assertFalse( n.input( ( "surface", "Cs.g" ) ) )
		self.assertFalse( n.input( ( "surface", "Cs.b" ) ) )

	def testArnoldComponentConnectionsNotConverted( self ) :

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"flat1" : IECoreScene.Shader( "flat", "ai:shader" ),
				"flat2" : IECoreScene.Shader( "flat", "ai:surface" ),
			},
			connections = [
				( ( "flat1", "r" ), ( "flat2", "color.g" ) )
			],
			output = "flat2"
		)

		n2 = n.copy()
		IECoreScene.ShaderNetworkAlgo.convertOSLComponentConnections( n2 )
		self.assertEqual( n, n2 )

	def testAddRemoveComponentConnectionAdapters( self ) :
		source = IECoreScene.Shader( "source", "ai:shader" )

		dest = IECoreScene.Shader( "dest", "ai:surface" )
		dest.parameters["a"] = IECore.Color3fData( imath.Color3f( 0.0 ) )
		dest.parameters["b"] = IECore.Color3fData( imath.Color3f( 0.0 ) )
		dest.parameters["c"] = IECore.FloatData( 0.0 )
		dest.parameters["d"] = IECore.Color3fData( imath.Color3f( 0.0 ) )
		dest.parameters["e"] = IECore.FloatData( 0.0 )

		network = IECoreScene.ShaderNetwork()
		network.addShader( "source1", source )
		network.addShader( "source2", source )
		network.addShader( "source3", source )
		network.addShader( "dest", dest )
		network.setOutput( IECoreScene.ShaderNetwork.Parameter( "dest", "" ) )

		# Float to color connection
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source1", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "a.r" )
		) )
		# Swizzled color connection
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source2", "out.r" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "b.g" )
		) )
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source2", "out.g" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "b.b" )
		) )
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source2", "out.b" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "b.r" )
		) )
		# Color to float connection
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source3", "out.r" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "c" )
		) )
		# Manual pack ( should not be affected by adapters )
		pack = IECoreScene.Shader( "MaterialX/mx_pack_color", "osl:shader" )
		pack.parameters["in1"] = IECore.FloatData( 0.0 )
		network.addShader( "pack", pack )
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source1", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "pack", "in1" )
		) )
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "pack", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "d" )
		) )
		# Manual swizzle ( should not be affected by adapters )
		swizzle = IECoreScene.Shader( "MaterialX/mx_swizzle_color_float", "osl:shader" )
		swizzle.parameters["in"] = IECore.Color3fData( imath.Color3f( 0.0 ) )
		swizzle.parameters["channels"] = IECore.StringData( "R" )
		network.addShader( "swizzle", swizzle )
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "source2", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "swizzle", "in" )
		) )
		network.addConnection( IECoreScene.ShaderNetwork.Connection(
			IECoreScene.ShaderNetwork.Parameter( "swizzle", "out" ),
			IECoreScene.ShaderNetwork.Parameter( "dest", "e" )
		) )

		converted = network.copy()
		IECoreScene.ShaderNetworkAlgo.addComponentConnectionAdapters( converted )
		prefixMismatch = network.copy()
		IECoreScene.ShaderNetworkAlgo.addComponentConnectionAdapters( prefixMismatch, "doesNotExist" )
		prefixMatch = network.copy()
		IECoreScene.ShaderNetworkAlgo.addComponentConnectionAdapters( prefixMatch, "ai" )
		convertBack = converted.copy()
		IECoreScene.ShaderNetworkAlgo.removeComponentConnectionAdapters( convertBack )

		connectionDict = dict( [ ( i.destination.name, i ) for i in converted.inputConnections( "dest" ) ] )
		self.assertEqual( converted.getShader( connectionDict["a"].source.shader ).name, "MaterialX/mx_pack_color" )
		self.assertEqual( converted.getShader( connectionDict["b"].source.shader ).name, "MaterialX/mx_pack_color" )
		self.assertEqual( converted.getShader( connectionDict["c"].source.shader ).name, "MaterialX/mx_swizzle_color_float" )

		self.assertEqual( IECoreScene.ShaderNetworkAlgo.componentConnectionAdapterLabel(), IECore.InternedString( "cortex_autoAdapter" ) )
		self.assertEqual( converted.getShader( connectionDict["c"].source.shader ).blindData(), IECore.CompoundData( { "cortex_autoAdapter" : True } ) )

		# With a prefix that doesn't match, nothing happens
		self.assertEqual( prefixMismatch, network )
		# With a prefix that matches, everything gets adapted
		self.assertEqual( prefixMatch, converted )

		# Check round trip
		self.assertEqual( convertBack, network )

		# Check that the shaders we don't modify are reused when the network is copied ( the addresses are the same )
		self.assertTrue( network.getShader( "source1", _copy = False ).isSame( converted.getShader( "source1", _copy = False ) ) )
		self.assertTrue( network.getShader( "dest", _copy = False ).isSame( converted.getShader( "dest", _copy = False ) ) )

		dest.blindData()["cortex_autoAdapter"] = IECore.BoolData( True )
		badNetwork = IECoreScene.ShaderNetwork()
		badNetwork.addShader( "badHandle", dest )
		badNetwork.setOutput( IECoreScene.ShaderNetwork.Parameter( "badHandle", "" ) )
		with six.assertRaisesRegex( self, RuntimeError, "removeComponentConnectionAdapters : adapter is not of supported type and name: 'badHandle' ai:surface : dest" ) :
			IECoreScene.ShaderNetworkAlgo.removeComponentConnectionAdapters( badNetwork )

	def testConvertObjectVector( self ) :

		objectVector = IECore.ObjectVector( [
			IECoreScene.Shader( "noise", parameters = { "__handle" : "textureHandle" } ),
			IECoreScene.Shader( "standard_surface", parameters = { "base" : "link:textureHandle.r", "base_color" : "link:textureHandle" } ),
		] )

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

		self.assertEqual( IECoreScene.ShaderNetworkAlgo.convertObjectVector( objectVector ), shaderNetwork )

	def testSplineConversion( self ):
		parms = IECore.CompoundData()
		parms["testffbSpline"] = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.bSpline(),
			( ( 0, 1 ), ( 10, 2 ), ( 20, 0 ), ( 21, 2 ) ) ) )
		parms["testffbezier"] = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.bSpline(),
			( ( 0, 1 ), ( 0.2, 6 ), ( 0.3, 7 ), ( 0.4, 4 ), ( 0.5, 5 ) ) ) )
		parms["testfColor3fcatmullRom"] = IECore.SplinefColor3fData( IECore.SplinefColor3f( IECore.CubicBasisf.catmullRom(),
( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ), ( 30, imath.Color3f(5) ), ( 40, imath.Color3f(2) ), ( 50, imath.Color3f(6) ) ) ) )
		parms["testfColor3flinear"] = IECore.SplinefColor3fData( IECore.SplinefColor3f( IECore.CubicBasisf.linear(),
( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ) ) ) )
		parms["testffconstant"] = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.constant(),
			( ( 0, 1 ), ( 0.2, 6 ), ( 0.3, 7 ) ) ) )

		parmsExpanded = IECoreScene.ShaderNetworkAlgo.expandSplineParameters( parms )

		self.assertEqual( set( parmsExpanded.keys() ), set( [ i + suffix for suffix in [ "Basis", "Values", "Positions" ] for i in parms.keys() ] ) )
		self.assertEqual( type( parmsExpanded["testffbSplineBasis"] ), IECore.StringData )
		self.assertEqual( type( parmsExpanded["testffbSplinePositions"] ), IECore.FloatVectorData )
		self.assertEqual( type( parmsExpanded["testffbSplineValues"] ), IECore.FloatVectorData )
		self.assertEqual( type( parmsExpanded["testfColor3fcatmullRomValues"] ), IECore.Color3fVectorData )

		for name, extra in [
			( "testffbSpline", 0 ),
			( "testffbezier", 0 ),
			( "testfColor3fcatmullRom", 0 ),
			( "testfColor3flinear", 2 ),
			( "testffconstant", 3 )
		]:
			self.assertEqual( len( parms[name].value.keys() ) + extra, len( parmsExpanded[name + "Positions"] ) )

		parmsCollapsed = IECoreScene.ShaderNetworkAlgo.collapseSplineParameters( parmsExpanded )

		self.assertEqual( parmsCollapsed, parms )

		del parmsExpanded["testffbSplineBasis"]
		del parmsExpanded["testffbezierValues"]
		del parmsExpanded["testfColor3fcatmullRomPositions"]
		del parmsExpanded["testfColor3flinearBasis"]
		del parmsExpanded["testffconstantPositions"]

		parmsCollapsed = IECoreScene.ShaderNetworkAlgo.collapseSplineParameters( parmsExpanded )

		self.assertEqual( parmsCollapsed, parmsExpanded )

	def testColor4ComponentConnections( self ) :

		original = IECoreScene.ShaderNetwork(
			shaders = {
				"source" : IECoreScene.Shader( "noise" ),
				"output" : IECoreScene.Shader(
					"color_correct",
					parameters = {
						"input" : imath.Color4f( 1 ),
					}
				),
			},
			connections = [
				( ( "source", "r" ), ( "output", "input.g" ) ),
				( ( "source", "g" ), ( "output", "input.b" ) ),
				( ( "source", "b" ), ( "output", "input.r" ) ),
				( ( "source", "r" ), ( "output", "input.a" ) ),
			],
			output = "output",
		)

		converted = original.copy()
		IECoreScene.ShaderNetworkAlgo.addComponentConnectionAdapters( converted )

		unconverted = converted.copy()
		IECoreScene.ShaderNetworkAlgo.removeComponentConnectionAdapters( unconverted )
		self.assertEqual( unconverted, original )

if __name__ == "__main__":
	unittest.main()
