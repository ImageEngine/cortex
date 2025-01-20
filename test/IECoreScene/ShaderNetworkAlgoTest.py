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

import pathlib
import unittest

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

		source = IECoreScene.ShaderNetwork(
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

		n = source.copy()
		IECoreScene.ShaderNetworkAlgo.removeUnusedShaders( n )
		self.assertEqual( set( n.shaders().keys() ), { "used1", "used2", "used3" } )

		# Test a network with a cycle - this is invalid, but we don't want it to crash
		n = source.copy()
		n.addConnection( ( ( "used3", "out" ), ( "used2", "in2" ) ) )
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

		# With a prefix that doesn't match, nothing happens
		self.assertEqual( prefixMismatch, network )
		# With a prefix that matches, everything gets adapted
		self.assertEqual( prefixMatch, converted )

		# Check round trip
		self.assertEqual( convertBack, network )

		# Check that the shaders we don't modify are reused when the network is copied ( the addresses are the same )
		self.assertTrue( network.getShader( "source1", _copy = False ).isSame( converted.getShader( "source1", _copy = False ) ) )
		self.assertTrue( network.getShader( "dest", _copy = False ).isSame( converted.getShader( "dest", _copy = False ) ) )

		# Check that we can unconvert a network converted using legacy adaptor blind data.

		legacyConverted = IECore.ObjectReader( str( pathlib.Path( __file__ ).parent / "data" / "legacyComponentConnectionAdaptors.cob" ) ).read()
		legacyConvertedBack = legacyConverted.copy()
		IECoreScene.ShaderNetworkAlgo.removeComponentConnectionAdapters( legacyConvertedBack )
		self.assertEqual( legacyConvertedBack, network )

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

		shaderNetworkOrig = IECoreScene.ShaderNetwork(
			shaders = { "test" : IECoreScene.Shader( "test", "osl:shader", parms ) },
			output = "test"
		)
		shaderNetwork = shaderNetworkOrig.copy()
		IECoreScene.ShaderNetworkAlgo.expandSplines( shaderNetwork )

		parmsExpanded = shaderNetwork.outputShader().parameters

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

		IECoreScene.ShaderNetworkAlgo.collapseSplines( shaderNetwork )

		self.assertEqual( shaderNetwork, shaderNetworkOrig )

		# Collapsing can fail in various ways where we just print an error, maybe these could be exceptions
		# instead ... I'm picturing a scenario where one of our USDs has been run through an unknown translation
		# process that might corrupt it
		shaderNetworkExpandedGood = IECoreScene.ShaderNetwork(
			shaders = {
				"source" : IECoreScene.Shader( "source" ),
				"adapt" : IECoreScene.Shader( "Utility/__ColorToArray", "osl:shader" ),
				"test" : IECoreScene.Shader( "test", "osl:shader", parmsExpanded )
			},
			output = "test"
		)

		shaderNetworkBadSplineConnection = shaderNetworkExpandedGood.copy()
		shaderNetworkBadSplineConnection.addConnection( ( ( "adapt", "out4" ), ( "test", "notASpline" ) ) )

		with IECore.CapturingMessageHandler() as mh :
			IECoreScene.ShaderNetworkAlgo.collapseSplines( shaderNetworkBadSplineConnection )
		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Error )
		self.assertEqual( mh.messages[0].message, 'Invalid spline plug name "notASpline"' )

		shaderNetworkBadSplineConnection = shaderNetworkExpandedGood.copy()
		shaderNetworkBadSplineConnection.addConnection( ( ( "adapt", "out4" ), ( "test", "notASplineValues" ) ) )

		with IECore.CapturingMessageHandler() as mh :
			IECoreScene.ShaderNetworkAlgo.collapseSplines( shaderNetworkBadSplineConnection )
		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Error )
		self.assertEqual( mh.messages[0].message, 'Invalid connection to spline parameter that doesn\'t exist "test.notASplineValues"' )

		shaderNetworkBadSplineConnection = shaderNetworkExpandedGood.copy()
		shaderNetworkBadSplineConnection.addConnection( ( ( "adapt", "out4" ), ( "test", "testffbSplineValues" ) ) )
		shaderNetworkBadSplineConnection.addConnection( ( ( "source", "out" ), ( "adapt", "inX" ) ) )

		with IECore.CapturingMessageHandler() as mh :
			IECoreScene.ShaderNetworkAlgo.collapseSplines( shaderNetworkBadSplineConnection )
		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Error )
		self.assertEqual( mh.messages[0].message, 'Invalid spline adapter input name "inX"' )


		# Test with partial splines that don't have the right parameters to be valid ( these silently do
		# nothing, because maybe you just happen to have some parameters that look a bit like splines )
		del parmsExpanded["testffbSplineBasis"]
		del parmsExpanded["testffbezierValues"]
		del parmsExpanded["testfColor3fcatmullRomPositions"]
		del parmsExpanded["testfColor3flinearBasis"]
		del parmsExpanded["testffconstantPositions"]

		shaderNetworkInvalidOrig = IECoreScene.ShaderNetwork(
			shaders = { "test" : IECoreScene.Shader( "test", "osl:shader", parmsExpanded ) },
			output = "test"
		)
		shaderNetworkInvalid = shaderNetworkInvalidOrig.copy()
		IECoreScene.ShaderNetworkAlgo.collapseSplines( shaderNetworkInvalid )
		self.assertEqual( shaderNetworkInvalid, shaderNetworkInvalidOrig )

	def __splineConversionArnold( self, shaderName, valueName, valueType, parms ):

		shaderNetworkOrig = IECoreScene.ShaderNetwork(
			shaders = { "test" : IECoreScene.Shader( shaderName, "ai:surface", parms ) },
			output = "test"
		)
		shaderNetwork = shaderNetworkOrig.copy()
		IECoreScene.ShaderNetworkAlgo.expandSplines( shaderNetwork )

		parmsExpanded = shaderNetwork.outputShader().parameters

		self.assertEqual( type( parmsExpanded["interpolation"] ), IECore.IntVectorData )
		self.assertEqual( type( parmsExpanded["position"] ), IECore.FloatVectorData )
		self.assertEqual( type( parmsExpanded[valueName] ), valueType )

		IECoreScene.ShaderNetworkAlgo.collapseSplines( shaderNetwork )

		self.assertEqual( shaderNetwork, shaderNetworkOrig )

	def testSplineConversionArnold( self ):

		parmsRgb = IECore.CompoundData()
		parmsRgb["ramp"] = IECore.SplinefColor3fData( IECore.SplinefColor3f( IECore.CubicBasisf.catmullRom(),
( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ), ( 30, imath.Color3f(5) ), ( 40, imath.Color3f(2) ), ( 50, imath.Color3f(6) ) ) ) )

		self.__splineConversionArnold( "ramp_rgb", "color", IECore.Color3fVectorData, parmsRgb )

		parmsFloat = IECore.CompoundData()
		parmsFloat["ramp"] = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.constant(),
			( ( 0, 1 ), ( 0.2, 6 ), ( 0.3, 7 ) ) ) )

		self.__splineConversionArnold( "ramp_float", "value", IECore.FloatVectorData, parmsFloat )

	def __splineConversionRenderman( self, shaderType ):

		parms = IECore.CompoundData()
		parms["colorRamp"] = IECore.SplinefColor3fData( IECore.SplinefColor3f( IECore.CubicBasisf.catmullRom(),
( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ), ( 30, imath.Color3f(5) ), ( 40, imath.Color3f(2) ), ( 50, imath.Color3f(6) ) ) ) )
		parms["floatRamp"] = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.constant(),
			( ( 0, 1 ), ( 0.2, 6 ), ( 0.3, 7 ) ) ) )

		shaderNetworkOrig = IECoreScene.ShaderNetwork(
			shaders = { "test" : IECoreScene.Shader( "PxrSplineMap", shaderType, parms ) },
			output = "test"
		)
		shaderNetwork = shaderNetworkOrig.copy()
		IECoreScene.ShaderNetworkAlgo.expandSplines( shaderNetwork )

		parmsExpanded = shaderNetwork.outputShader().parameters

		self.assertEqual( type( parmsExpanded["colorRamp_Interpolation"] ), IECore.StringData )
		self.assertEqual( type( parmsExpanded["colorRamp_Knots"] ), IECore.FloatVectorData )
		self.assertEqual( type( parmsExpanded["colorRamp_Colors"] ), IECore.Color3fVectorData )

		self.assertEqual( type( parmsExpanded["floatRamp_Interpolation"] ), IECore.StringData )
		self.assertEqual( type( parmsExpanded["floatRamp_Knots"] ), IECore.FloatVectorData )
		self.assertEqual( type( parmsExpanded["floatRamp_Floats"] ), IECore.FloatVectorData )

		IECoreScene.ShaderNetworkAlgo.collapseSplines( shaderNetwork )

		self.assertEqual( shaderNetwork, shaderNetworkOrig )

	def testSplineConversionRenderman( self ):

		self.__splineConversionRenderman( "osl:shader" )
		self.__splineConversionRenderman( "ri:surface" )

	def testSplineInputs( self ):

		fC3fcatmullRom = IECore.SplinefColor3fData( IECore.SplinefColor3f(
			IECore.CubicBasisf.catmullRom(),
			( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ), ( 20, imath.Color3f(0) ), ( 30, imath.Color3f(5) ), ( 40, imath.Color3f(2) ), ( 50, imath.Color3f(6) ) )
		) )
		fC3flinear = IECore.SplinefColor3fData( IECore.SplinefColor3f(
			IECore.CubicBasisf.linear(),
			( ( 0, imath.Color3f(1) ), ( 10, imath.Color3f(2) ) )
		) )
		ffconstant = IECore.SplineffData( IECore.Splineff(
			IECore.CubicBasisf.constant(),
			( ( 0, 1 ), ( 0.2, 6 ), ( 0.3, 7 ) )
		) )

		nOrig = IECoreScene.ShaderNetwork(
			shaders = {
				"testSplines" : IECoreScene.Shader(
					"testSplines", "osl:shader",
					parameters = {
						"fC3fcatmullRom" : fC3fcatmullRom,
						"fC3flinear" : fC3flinear,
						"ffconstant" : ffconstant,
					}
				),
				"floatSource" : IECoreScene.Shader( "floatSource", "osl:shader" ),
				"colorSource" : IECoreScene.Shader( "colorSource", "osl:shader" ),
			},
			connections = [
				( ( "colorSource", "out" ), ( "testSplines", "fC3fcatmullRom[1].y" ) ),
				( ( "floatSource", "out" ), ( "testSplines", "fC3fcatmullRom[3].y.b" ) ),
				( ( "floatSource", "out" ), ( "testSplines", "fC3flinear[0].y.g" ) ),
				( ( "floatSource", "out" ), ( "testSplines", "ffconstant[2].y" ) ),
			],
			output = "testSplines"
		)

		n = nOrig.copy()
		IECoreScene.ShaderNetworkAlgo.convertToOSLConventions( n, 11000 )
		self.assertEqual( len( nOrig ), 3 )
		self.assertEqual( len( n ), 6 )

		convertedSplineParameters = n.getShader( "testSplines" ).parameters
		self.assertEqual( convertedSplineParameters["fC3fcatmullRomPositions"], IECore.FloatVectorData( [ 0, 10, 20, 30, 40, 50 ] ) )
		self.assertEqual( convertedSplineParameters["fC3flinearPositions"], IECore.FloatVectorData( [ 0, 0, 10, 10 ] ) )
		self.assertEqual( convertedSplineParameters["ffconstantPositions"], IECore.FloatVectorData( [ 0, 0, 0.2, 0.3, 0.3, 0.3 ] ) )


		self.assertEqual( n.input( ( "testSplines", "fC3fcatmullRomValues" ) ), ( "testSplines_fC3fcatmullRomInputArrayAdapter", "out6" ) )
		self.assertEqual( n.input( ( "testSplines", "fC3flinearValues" ) ), ( "testSplines_fC3flinearInputArrayAdapter", "out4" ) )
		self.assertEqual( n.input( ( "testSplines", "ffconstantValues" ) ), ( "testSplines_ffconstantInputArrayAdapter", "out6" ) )

		adapterParameters = n.getShader( "testSplines_fC3fcatmullRomInputArrayAdapter" ).parameters
		self.assertEqual(
			[ adapterParameters["in%i"%i].value for i in range( 6 ) ],
			[ imath.Color3f(1), imath.Color3f(2), imath.Color3f(0), imath.Color3f(5), imath.Color3f(2), imath.Color3f(6) ]
		)
		self.assertEqual( len( n.inputConnections( "testSplines_fC3fcatmullRomInputArrayAdapter" ) ), 2 )
		self.assertEqual( n.input( ( "testSplines_fC3fcatmullRomInputArrayAdapter", "in1" ) ), ( "colorSource", "out" ) )
		self.assertEqual( n.input( ( "testSplines_fC3fcatmullRomInputArrayAdapter", "in3[2]" ) ), ( "floatSource", "out" ) )

		adapter1Parameters = n.getShader( "testSplines_fC3flinearInputArrayAdapter" ).parameters
		self.assertEqual(
			[ adapter1Parameters["in%i"%i].value for i in range( 4 ) ],
			[ imath.Color3f(1), imath.Color3f(1), imath.Color3f(2), imath.Color3f(2) ]
		)
		self.assertEqual( len( n.inputConnections( "testSplines_fC3flinearInputArrayAdapter" ) ), 1 )
		self.assertEqual( n.input( ( "testSplines_fC3flinearInputArrayAdapter", "in1[1]" ) ), ( "floatSource", "out" ) )

		adapter2Parameters = n.getShader( "testSplines_ffconstantInputArrayAdapter" ).parameters
		self.assertEqual(
			[ adapter2Parameters["in%i"%i].value for i in range( 6 ) ],
			[ 1, 1, 6, 7, 7, 7 ]
		)
		self.assertEqual( len( n.inputConnections( "testSplines_ffconstantInputArrayAdapter" ) ), 1 )
		self.assertEqual( n.input( ( "testSplines_ffconstantInputArrayAdapter", "in3" ) ), ( "floatSource", "out" ) )

		# Check that we can get the same results using expandSplines, aside from convertToOSLConventions
		# also changing the component syntax
		nSplinesOnly = nOrig.copy()
		IECoreScene.ShaderNetworkAlgo.expandSplines( nSplinesOnly )
		self.assertEqual( nSplinesOnly.input( ( "testSplines_fC3fcatmullRomInputArrayAdapter", "in3.b" ) ), ( "floatSource", "out" ) )
		self.assertEqual( nSplinesOnly.input( ( "testSplines_fC3flinearInputArrayAdapter", "in1.g" ) ), ( "floatSource", "out" ) )
		nSplinesOnly.removeConnection( ( ( "floatSource", "out" ), ( "testSplines_fC3fcatmullRomInputArrayAdapter", "in3.b" ) ) )
		nSplinesOnly.removeConnection( ( ( "floatSource", "out" ), ( "testSplines_fC3flinearInputArrayAdapter", "in1.g" ) ) )
		nSplinesOnly.addConnection( ( ( "floatSource", "out" ), ( "testSplines_fC3fcatmullRomInputArrayAdapter", "in3[2]" ) ) )
		nSplinesOnly.addConnection( ( ( "floatSource", "out" ), ( "testSplines_fC3flinearInputArrayAdapter", "in1[1]" ) ) )
		self.assertEqual( n, nSplinesOnly )

		# But expandSplines won't do anything if the prefix is wrong
		nWrongPrefix = nOrig.copy()
		IECoreScene.ShaderNetworkAlgo.expandSplines( nWrongPrefix, "foo:" )
		self.assertEqual( nOrig, nWrongPrefix )

		# Check that collapseSplines does the reverse
		nReverse = nOrig.copy()
		IECoreScene.ShaderNetworkAlgo.expandSplines( nReverse )
		IECoreScene.ShaderNetworkAlgo.collapseSplines( nReverse )
		self.assertEqual( nOrig, nReverse )

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"testSplines" : IECoreScene.Shader(
					"testSplines", "osl:shader",
					parameters = {
						"fC3fcatmullRom" : IECore.SplinefColor3fData(
							IECore.SplinefColor3f( IECore.CubicBasisf.catmullRom(), [ ( 0, imath.Color3f(0) ) ] * 33 )
						),
					}
				),
				"floatSource" : IECoreScene.Shader( "floatSource", "osl:shader" ),
			},
			connections = [
				( ( "floatSource", "out" ), ( "testSplines", "fC3fcatmullRom[0].y.b" ) ),
			],
			output = "testSplines"
		)

		with self.assertRaisesRegex( Exception, r".*Cannot handle input to testSplines.fC3fcatmullRom\[0\].y.b : expanded spline has 33 control points, but max input adapter size is 32.*" ):
			IECoreScene.ShaderNetworkAlgo.convertToOSLConventions( n, 11000 )

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"testSplines" : IECoreScene.Shader(
					"testSplines", "osl:shader",
					parameters = { "fC3fcatmullRom" : fC3fcatmullRom }
				),
				"floatSource" : IECoreScene.Shader( "floatSource", "osl:shader" ),
			},
			connections = [
				( ( "floatSource", "out" ), ( "testSplines", "fC3fcatmullRom[xx].y.b" ) ),
			],
			output = "testSplines"
		)

		with self.assertRaisesRegex( Exception, "Invalid spline point index xx" ):
			IECoreScene.ShaderNetworkAlgo.convertToOSLConventions( n, 11000 )

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"testSplines" : IECoreScene.Shader(
					"testSplines", "osl:shader",
					parameters = { "fC3fcatmullRom" : fC3fcatmullRom }
				),
				"floatSource" : IECoreScene.Shader( "floatSource", "osl:shader" ),
			},
			connections = [
				( ( "floatSource", "out" ), ( "testSplines", "fC3fcatmullRom[-1].y.b" ) ),
			],
			output = "testSplines"
		)

		with self.assertRaisesRegex( Exception, "Spline index -1 is out of range in spline with 6 points." ):
			IECoreScene.ShaderNetworkAlgo.convertToOSLConventions( n, 11000 )

		n = IECoreScene.ShaderNetwork(
			shaders = {
				"testSplines" : IECoreScene.Shader(
					"testSplines", "osl:shader",
					parameters = { "fC3fcatmullRom" : fC3fcatmullRom }
				),
				"floatSource" : IECoreScene.Shader( "floatSource", "osl:shader" ),
			},
			connections = [
				( ( "floatSource", "out" ), ( "testSplines", "fC3fcatmullRom[100].y.b" ) ),
			],
			output = "testSplines"
		)

		with self.assertRaisesRegex( Exception, "Spline index 100 is out of range in spline with 6 points." ):
			IECoreScene.ShaderNetworkAlgo.convertToOSLConventions( n, 11000 )

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

	def testCustomComponentConnectionAdaptors( self ) :

		original = IECoreScene.ShaderNetwork(
			shaders = {
				"noise1" : IECoreScene.Shader( "noise", "ai:shader" ),
				"image" : IECoreScene.Shader( "image", "ai:shader", { "missing_texture_color" : imath.Color4f( 1, 2, 3, 4 ) } ),
				"noise2" : IECoreScene.Shader( "noise", "ai:shader", { "color1" : imath.Color3f( 1, 2, 3 ) } ),
			},
			connections = [
				( ( "noise1", "out.r" ), ( "image", "missing_texture_color.a" ) ),
				( ( "image", "out.r" ), ( "noise2", "color1.b" ) ),
				( ( "image", "out.g" ), ( "noise2", "color1.g" ) ),
				( ( "image", "out.b" ), ( "noise2", "color1.r" ) ),
			],
			output = ( "noise2", "out" )
		)

		for c in "rgbaxyz" :
			IECoreScene.ShaderNetworkAlgo.registerSplitAdapter(
				"ai", c, IECoreScene.Shader( "rgba_to_float", "ai:shader", { "mode" : c } ), "input", "out"
			)

		IECoreScene.ShaderNetworkAlgo.registerJoinAdapter(
			"ai", IECore.Color3fData.staticTypeId(), IECoreScene.Shader( "float_to_rgb", "ai:shader" ), ( "r", "g", "b" ), "out"
		)

		IECoreScene.ShaderNetworkAlgo.registerJoinAdapter(
			"ai", IECore.Color4fData.staticTypeId(), IECoreScene.Shader( "float_to_rgba", "ai:shader" ), ( "r", "g", "b", "a" ), "out"
		)

		def deregisterAdaptors() :

			for c in "rgbaxyz" :
				IECoreScene.ShaderNetworkAlgo.deregisterSplitAdapter( "ai", c )

			IECoreScene.ShaderNetworkAlgo.deregisterJoinAdapter( "ai", IECore.Color3fData.staticTypeId() )
			IECoreScene.ShaderNetworkAlgo.deregisterJoinAdapter( "ai", IECore.Color4fData.staticTypeId() )

		self.addCleanup( deregisterAdaptors )

		converted = original.copy()
		IECoreScene.ShaderNetworkAlgo.addComponentConnectionAdapters( converted )

		noise2Input = converted.input( ( "noise2", "color1" ) )
		noise2InputShader = converted.getShader( noise2Input.shader )
		self.assertEqual( noise2InputShader.name, "float_to_rgb" )
		self.assertEqual( noise2InputShader.type, "ai:shader" )
		for c in "rgb" :
			cInput = converted.input( ( noise2Input.shader, c ) )
			cInputShader = converted.getShader( cInput.shader )
			self.assertEqual( cInputShader.name, "rgba_to_float" )
			self.assertEqual( cInputShader.type, "ai:shader" )
			self.assertEqual( cInputShader.parameters["mode"], IECore.StringData( { "r" : "b", "g" : "g", "b" : "r" }[c] ) )
			self.assertEqual( converted.input( ( cInput.shader, "input" ) ), ( "image", "out" ) )

		imageInput = converted.input( ( "image", "missing_texture_color" ) )
		self.assertEqual( imageInput.name, "out" )
		imageInputShader = converted.getShader( imageInput.shader )
		self.assertEqual( imageInputShader.name, "float_to_rgba" )
		self.assertEqual( imageInputShader.type, "ai:shader" )

		noise1Split = converted.input( ( imageInput.shader, "a" ) )
		noise1SplitShader = converted.getShader( noise1Split.shader )
		self.assertEqual( noise1SplitShader.name, "rgba_to_float" )
		self.assertEqual( noise1SplitShader.type, "ai:shader" )
		self.assertEqual( converted.input( ( noise1Split.shader, "input" ) ), ( "noise1", "out" ) )
		self.assertEqual( noise1SplitShader.parameters["mode"], IECore.StringData( "r" ) )

		# Check that removing the adaptors gets us back to the original network.
		# We deregister the adaptors first because we want the removal process to
		# be completely independent of the current registrations.

		deregisterAdaptors()

		unconverted = converted.copy()
		IECoreScene.ShaderNetworkAlgo.removeComponentConnectionAdapters( unconverted )
		self.assertEqual( unconverted, original )

if __name__ == "__main__":
	unittest.main()
