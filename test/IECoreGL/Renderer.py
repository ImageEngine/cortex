##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2011, John Haddon. All rights reserved.
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
import os
import os.path
import threading
import math
import shutil
import imath

import IECore
import IECoreScene
import IECoreImage
import IECoreGL

IECoreGL.init( False )

class TestRenderer( unittest.TestCase ) :

	def testOptions( self ) :

		os.environ["IECOREGL_TEXTURE_PATHS"] = "textureDefault"
		os.environ["IECOREGL_SHADER_PATHS"] = "shaderDefault"

		r = IECoreGL.Renderer()

		self.assertEqual( r.typeName(), "IECoreGL::Renderer" )

		self.assertEqual( r.getOption( "searchPath:texture" ), IECore.StringData( "textureDefault" ) )
		self.assertEqual( r.getOption( "gl:searchPath:texture" ), IECore.StringData( "textureDefault" ) )

		r.setOption( "searchPath:texture", IECore.StringData( "a" ) )
		self.assertEqual( r.getOption( "searchPath:texture" ), IECore.StringData( "a" ) )
		self.assertEqual( r.getOption( "gl:searchPath:texture" ), IECore.StringData( "a" ) )

		r.setOption( "gl:searchPath:texture", IECore.StringData( "b" ) )
		self.assertEqual( r.getOption( "searchPath:texture" ), IECore.StringData( "b" ) )
		self.assertEqual( r.getOption( "gl:searchPath:texture" ), IECore.StringData( "b" ) )

		self.assertEqual( r.getOption( "searchPath:shader" ), IECore.StringData( "shaderDefault" ) )
		self.assertEqual( r.getOption( "gl:searchPath:shader" ), IECore.StringData( "shaderDefault" ) )

		r.setOption( "searchPath:shader", IECore.StringData( "s" ) )
		self.assertEqual( r.getOption( "searchPath:shader" ), IECore.StringData( "s" ) )
		self.assertEqual( r.getOption( "gl:searchPath:shader" ), IECore.StringData( "s" ) )

		r.setOption( "gl:searchPath:shader", IECore.StringData( "t" ) )
		self.assertEqual( r.getOption( "searchPath:shader" ), IECore.StringData( "t" ) )
		self.assertEqual( r.getOption( "gl:searchPath:shader" ), IECore.StringData( "t" ) )

		self.assertEqual( r.getOption( "shutter" ), IECore.V2fData( imath.V2f( 0 ) ) )
		r.setOption( "shutter", IECore.V2fData( imath.V2f( 1, 2 ) ) )
		self.assertEqual( r.getOption( "shutter" ), IECore.V2fData( imath.V2f( 1, 2 ) ) )

		self.assertEqual( r.getOption( "gl:drawCoordinateSystems" ), IECore.BoolData( False ) )
		r.setOption( "gl:drawCoordinateSystems", IECore.BoolData( True ) )
		self.assertEqual( r.getOption( "gl:drawCoordinateSystems" ), IECore.BoolData( True ) )

	def testAttributes( self ) :

		deferred = IECoreGL.Renderer()
		deferred.setOption( "gl:mode", IECore.StringData( "deferred" ) )

		immediate = IECoreGL.Renderer()
		immediate.setOption( "gl:mode", IECore.StringData( "immediate" ) )


		for r in [ deferred, immediate ] :

			r.worldBegin()

			self.assertEqual( r.getAttribute( "color" ), IECore.Color3fData( imath.Color3f( 1 ) ) )
			self.assertEqual( r.getAttribute( "opacity" ), IECore.Color3fData( imath.Color3f( 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:color" ), IECore.Color4fData( imath.Color4f( 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:blend:color" ), IECore.Color4fData( imath.Color4f( 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:blend:srcFactor" ), IECore.StringData( "srcAlpha" ) )
			self.assertEqual( r.getAttribute( "gl:blend:dstFactor" ), IECore.StringData( "oneMinusSrcAlpha" ) )
			self.assertEqual( r.getAttribute( "gl:blend:equation" ), IECore.StringData( "add" ) )
			self.assertEqual( r.getAttribute( "gl:shade:transparent" ), IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:primitive:sortForTransparency" ), IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "name" ), IECore.StringData( "unnamed" ) )
			self.assertEqual( r.getAttribute( "doubleSided" ), IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:smoothing:points" ), IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:smoothing:lines" ), IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:smoothing:polygons" ), IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:procedural:reentrant" ), IECore.BoolData( True ) )
			if IECore.withFreeType() :
				self.assertEqual( r.getAttribute( "gl:textPrimitive:type" ), IECore.StringData( "mesh" ) )
			self.assertEqual( r.getAttribute( "gl:depthTest" ), IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:depthMask" ), IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:alphaTest" ), IECore.BoolData( False ) )

			self.assertEqual( r.getAttribute( "gl:alphaTest:mode" ), IECore.StringData( "always" ) )
			self.assertEqual( r.getAttribute( "gl:alphaTest:value" ), IECore.FloatData( 0.0 ) )

			self.assertEqual( r.getAttribute( "gl:visibility:camera" ), IECore.BoolData( True ) )

			self.assertEqual( r.getAttribute( "gl:automaticInstancing" ), IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "automaticInstancing" ), IECore.BoolData( True ) )

			r.setAttribute( "color", IECore.Color3fData( imath.Color3f( 0, 1, 2 ) ) )
			self.assertEqual( r.getAttribute( "color" ), IECore.Color3fData( imath.Color3f( 0, 1, 2 ) ) )

			# opacity is an odd one - it's set as a color but as it's averaged internally
			# the result you get should be a greyscale value.
			r.setAttribute( "opacity", IECore.Color3fData( imath.Color3f( 3, 1, 2 ) ) )
			self.assertEqual( r.getAttribute( "opacity" ), IECore.Color3fData( imath.Color3f( 2 ) ) )

			self.assertEqual( r.getAttribute( "gl:color" ), IECore.Color4fData( imath.Color4f( 0, 1, 2, 2 ) ) )
			r.setAttribute( "gl:color", IECore.Color4fData( imath.Color4f( 1, 2, 3, 4 ) ) )
			self.assertEqual( r.getAttribute( "gl:color" ), IECore.Color4fData( imath.Color4f( 1, 2, 3, 4 ) ) )

			r.setAttribute( "gl:blend:color", IECore.Color4fData( imath.Color4f( 0, 1, 0, 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:blend:color" ), IECore.Color4fData( imath.Color4f( 0, 1, 0, 1 ) ) )

			r.attributeBegin()
			r.setAttribute( "color", IECore.Color3fData( imath.Color3f( 0 ) ) )
			self.assertEqual( r.getAttribute( "gl:color" ), IECore.Color4fData( imath.Color4f( 0, 0, 0, 4 ) ) )
			r.attributeEnd()
			self.assertEqual( r.getAttribute( "gl:color" ), IECore.Color4fData( imath.Color4f( 1, 2, 3, 4 ) ) )

			factors = [ "zero", "one", "srcColor", "oneMinusSrcColor", "dstColor", "oneMinusDstColor",
				"srcAlpha", "oneMinusSrcAlpha", "dstAlpha", "oneMinusDstAlpha", "dstAlpha", "oneMinusDstAlpha",
				"constantColor", "oneMinusConstantColor", "constantAlpha", "oneMinusConstantAlpha" ]
			for f in factors :
				last = r.getAttribute( "gl:blend:dstFactor" )
				r.setAttribute( "gl:blend:srcFactor", IECore.StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:srcFactor" ), IECore.StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:dstFactor" ), last )
				last = r.getAttribute( "gl:blend:srcFactor" )
				r.setAttribute( "gl:blend:dstFactor", IECore.StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:srcFactor" ), IECore.StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:dstFactor" ), last )

			for e in ["add", "subtract", "reverseSubtract", "min", "max"] :
				r.setAttribute( "gl:blend:equation", IECore.StringData( e ) )
				self.assertEqual( r.getAttribute( "gl:blend:equation" ), IECore.StringData( e ) )

			r.setAttribute( "name", IECore.StringData( "sphere" ) )
			self.assertEqual( r.getAttribute( "name" ), IECore.StringData( "sphere" ) )

			r.setAttribute( "doubleSided", IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "doubleSided" ), IECore.BoolData( False ) )

			r.setAttribute( "gl:smoothing:points", IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:smoothing:points" ), IECore.BoolData( True ) )
			r.setAttribute( "gl:smoothing:lines", IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:smoothing:lines" ), IECore.BoolData( True ) )
			r.setAttribute( "gl:smoothing:polygons", IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:smoothing:polygons" ), IECore.BoolData( True ) )

			r.setAttribute( "gl:procedural:reentrant", IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:procedural:reentrant" ), IECore.BoolData( False ) )

			if IECore.withFreeType() :
				r.setAttribute( "gl:textPrimitive:type", IECore.StringData( "sprite" ) )
				self.assertEqual( r.getAttribute( "gl:textPrimitive:type" ), IECore.StringData( "sprite" ) )

			r.setAttribute( "gl:depthTest", IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:depthTest" ), IECore.BoolData( False ) )

			r.setAttribute( "gl:depthMask", IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:depthMask" ), IECore.BoolData( False ) )

			r.setAttribute( "gl:alphaTest", IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:alphaTest" ), IECore.BoolData( True ) )

			alphaTestModes = [ "never", "less", "equal", "lequal", "greater", "notequal", "gequal", "always" ]
			value = 0.1
			for m in alphaTestModes :
				last = r.getAttribute( "gl:alphaTest:value" )
				r.setAttribute( "gl:alphaTest:mode", IECore.StringData( m ) )
				self.assertEqual( r.getAttribute( "gl:alphaTest:mode" ), IECore.StringData( m ) )
				self.assertEqual( r.getAttribute( "gl:alphaTest:value" ), last )

				last = r.getAttribute( "gl:alphaTest:mode" )
				r.setAttribute( "gl:alphaTest:value", IECore.FloatData( value ) )
				self.assertEqual( r.getAttribute( "gl:alphaTest:value" ), IECore.FloatData( value ) )
				self.assertEqual( r.getAttribute( "gl:alphaTest:mode" ), last )

				value += 0.05

			r.setAttribute( "gl:visibility:camera", IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:visibility:camera" ), IECore.BoolData( False ) )

			r.setAttribute( "gl:automaticInstancing", IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:automaticInstancing" ), IECore.BoolData( False ) )
			self.assertEqual( r.getAttribute( "automaticInstancing" ), IECore.BoolData( False ) )
			r.setAttribute( "automaticInstancing", IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "automaticInstancing" ), IECore.BoolData( True ) )
			self.assertEqual( r.getAttribute( "gl:automaticInstancing" ), IECore.BoolData( True ) )

			r.worldEnd()

	def testOtherRendererAttributes( self ) :

		"""Attributes destined for other renderers should be silently ignored."""

		deferred = IECoreGL.Renderer()
		deferred.setOption( "gl:mode", IECore.StringData( "deferred" ) )

		immediate = IECoreGL.Renderer()
		immediate.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		with IECore.CapturingMessageHandler() as handler :

			for r in [ deferred, immediate ] :

				r.worldBegin()

				r.setAttribute( "ri:visibility:diffuse", IECore.IntData( 0 ) )

				r.worldEnd()

		self.assertEqual( len( handler.messages ), 0 )

	def testStackBug( self ) :

		# This should produce a yellow sphere in between two red spheres. It does in the DeferredRenderer but
		# currently fails in the ImmediateRenderer.

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.display( os.path.dirname( __file__ ) + "/output/testStackBug.tif", "tiff", "rgba", {} )
		r.worldBegin()

		r.shader( "surface", "rgbColor", { "red" : IECore.FloatData( 1 ), "green" : IECore.FloatData( 0 ), "blue" : IECore.FloatData( 0 ) } )

		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )

		r.attributeBegin()

		r.shader( "surface", "rgbColor", { "red" : IECore.FloatData( 1 ), "green" : IECore.FloatData( 1 ), "blue" : IECore.FloatData( 0 ) } )
		r.sphere( 1, -1, 1, 360, {} )

		r.attributeEnd()

		r.concatTransform( imath.M44f().translate( imath.V3f( -1, 0, 0 ) ) )
		r.sphere( 1, -1, 1, 360, {} )

		r.concatTransform( imath.M44f().translate( imath.V3f( 2, 0, 0 ) ) )
		r.sphere( 1, -1, 1, 360, {} )

		r.worldEnd()

		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/testStackBug.tif" ).read()
		dimensions = i.dataWindow.size() + imath.V2i( 1 )
		index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 0.5)
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 1 )
		self.assertEqual( i["B"][index], 0 )

		index = dimensions.x * int(dimensions.y * 0.5)
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )

		index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 1) - 1
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )

	def testPrimVars( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.display( os.path.dirname( __file__ ) + "/output/testPrimVars.tif", "tiff", "rgba", {} )
		r.worldBegin()

		r.shader( "surface", "rgbColor", {} )

		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )

		r.attributeBegin()

		# should make red, green and blue spheres
		r.sphere(
			1, -1, 1, 360,
			{
				"red" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ) ),
				"green" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0 ) ),
				"blue" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0 ) ),
			}
		)

		r.attributeEnd()

		r.concatTransform( imath.M44f().translate( imath.V3f( -1, 0, 0 ) ) )
		r.sphere(
			1, -1, 1, 360,
			{
				"red" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0 ) ),
				"green" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ) ),
				"blue" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0 ) ),
			}
		)

		r.concatTransform( imath.M44f().translate( imath.V3f( 2, 0, 0 ) ) )
		r.sphere(
			1, -1, 1, 360,
			{
				"red" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0 ) ),
				"green" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0 ) ),
				"blue" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 1 ) ),
			}
		)

		r.worldEnd()

		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/testPrimVars.tif" ).read()
		dimensions = i.dataWindow.size() + imath.V2i( 1 )
		index = dimensions.x * int(dimensions.y * 0.5)
		self.assertEqual( i["R"][index], 0 )
		self.assertEqual( i["G"][index], 1 )
		self.assertEqual( i["B"][index], 0 )

		index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 0.5)
		self.assertEqual( i["R"][index], 1 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 0 )

		index = dimensions.x * int(dimensions.y * 0.5) + int(dimensions.x * 1) - 1
		self.assertEqual( i["R"][index], 0 )
		self.assertEqual( i["G"][index], 0 )
		self.assertEqual( i["B"][index], 1 )

	## \todo Make this assert something
	def testShader( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( os.path.dirname( __file__ ) + "/shaders/include" ) )

		r.worldBegin()
		r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )
		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )
		r.sphere( 1, -1, 1, 360, {} )
		r.worldEnd()

		s = r.scene()

		s.render( IECoreGL.State( True ) )

	def __countChildrenRecursive( self, g ) :
		if not isinstance( g, IECoreGL.Group ):
			return 1
		count = 0
		for c in g.children():
			count += self.__countChildrenRecursive( c )
		return count

	def testEdits( self ):

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )

		r.worldBegin()
		r.worldEnd()

		with IECore.CapturingMessageHandler() as handler :

			r.attributeBegin()
			r.setAttribute( "gl:color", IECore.Color4fData( imath.Color4f( 1, 2, 3, 4 ) ) )
			r.attributeEnd()

		self.assertEqual( len( handler.messages ), 3 )

		with IECore.CapturingMessageHandler() as handler :

			r.command( "editBegin", {} )
			r.attributeBegin()
			r.setAttribute( "gl:color", IECore.Color4fData( imath.Color4f( 1, 2, 3, 4 ) ) )
			r.attributeEnd()
			r.command( "editEnd", {} )

		self.assertEqual( len( handler.messages ), 0 )

	def testRemoveObject( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		with IECoreScene.WorldBlock( r ) :

			r.setAttribute( "name", "sphereOne" )

			r.sphere( 1, -1, 1, 360, {} )

			r.setAttribute( "name", "sphereTwo" )

			r.sphere( 1, -1, 1, 360, {} )

			with IECoreScene.AttributeBlock( r ) :

				r.sphere( 1, -1, 1, 360, {} )

				r.setAttribute( "name", "sphereOne" )

				r.sphere( 1, -1, 1, 360, {} )
				r.sphere( 1, -1, 1, 360, {} )
				r.sphere( 1, -1, 1, 360, {} )

		s = r.scene()
		self.assertEqual( len( s.root().children() ), 3 )

		# check that trying to remove objects when not in an editBegin/editEnd block
		# fails and prints a message

		errorCatcher = IECore.CapturingMessageHandler()
		with errorCatcher :
			commandResult = r.command( "removeObject", { "name" : IECore.StringData( "sphereOne" ) } )

		self.assertEqual( commandResult, None )
		self.assertEqual( len( errorCatcher.messages ), 1 )

		# check we can remove one object without affecting the other

		r.command( "editBegin", {} )
		commandResult = r.command( "removeObject", { "name" : IECore.StringData( "sphereOne" ) } )
		r.command( "editEnd", {} )

		self.assertEqual( commandResult, IECore.BoolData( True ) )
		self.assertEqual( len( s.root().children() ), 2 )
		self.assertEqual( self.__countChildrenRecursive( s.root() ), 2 )

		# now we test that either the sphere and the following attribute block ( instantiates as a Group ) are removed
		r.command( "editBegin", {} )
		commandResult = r.command( "removeObject", { "name" : IECore.StringData( "sphereTwo" ) } )
		r.command( "editEnd", {} )

		self.assertEqual( commandResult, IECore.BoolData( True ) )
		self.assertEqual( len( s.root().children() ), 0 )

	def testEditQuery( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		with IECoreScene.WorldBlock( r ) :
			self.assertEqual( r.command( "editQuery", {} ), IECore.BoolData( False ) )

		self.assertEqual( r.command( "editQuery", {} ), IECore.BoolData( False ) )
		r.command( "editBegin", {} )
		self.assertEqual( r.command( "editQuery", {} ), IECore.BoolData( True ) )
		r.command( "editEnd", {} )
		self.assertEqual( r.command( "editQuery", {} ), IECore.BoolData( False ) )

	def testRemoveObjectDuringProcedural( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		with IECoreScene.WorldBlock( r ) :

			r.setAttribute( "name", "sphereOne" )

			r.sphere( 1, -1, 1, 360, {} )

			r.setAttribute( "name", "sphereTwo" )

			r.sphere( 1, -1, 1, 360, {} )

		s = r.scene()
		self.assertEqual( len( s.root().children() ), 2 )

		class RemovalProcedural( IECoreScene.Renderer.Procedural ):

			def __init__( proc ):
				IECoreScene.Renderer.Procedural.__init__( proc )

			def bound( proc ) :
				return imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) )

			def render( proc, renderer ):
				commandResult = renderer.command( "removeObject", { "name" : IECore.StringData( "sphereOne" ) } )
				self.assertEqual( commandResult, IECore.BoolData( True ) )

			def hash( self ):
				h = IECore.MurmurHash()
				return h

		r.command( "editBegin", {} )
		r.procedural( RemovalProcedural() )
		r.command( "editEnd", {} )

		self.assertEqual( len( s.root().children() ), 1 )
		self.assertEqual( self.__countChildrenRecursive( r.scene().root() ), 1 )

	def testRemoveObjectWithResourcesDuringProcedural( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		with IECoreScene.WorldBlock( r ) :

			with IECoreScene.AttributeBlock( r ) :

				r.setAttribute( "name", "sphereOne" )

				r.shader( "surface", "image", {
					"texture" : IECore.SplinefColor3fData(
						IECore.SplinefColor3f(
							IECore.CubicBasisf.catmullRom(),
							(
								( 0, imath.Color3f( 1 ) ),
								( 0, imath.Color3f( 1 ) ),
								( 1, imath.Color3f( 0 ) ),
								( 1, imath.Color3f( 0 ) ),
							),
						),
					),
				} )

				r.sphere( 1, -1, 1, 360, {} )

		s = r.scene()
		self.assertEqual( len( s.root().children()[0].children() ), 1 )

		s.render()

		class RemovalProcedural( IECoreScene.Renderer.Procedural ):

			def __init__( proc, level=0 ) :

				IECoreScene.Renderer.Procedural.__init__( proc )

			def bound( proc ) :

				return imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) )

			def render( proc, renderer ):

				commandResult = renderer.command( "removeObject", { "name" : IECore.StringData( "sphereOne" ) } )
				self.assertEqual( commandResult, IECore.BoolData( True ) )

			def hash( self ):

				h = IECore.MurmurHash()
				return h

		r.command( "editBegin", {} )

		# typically you wouldn't call a renderer method on a separate thread like this. we're just
		# doing it here to force the procedural onto a different thread. if left to its own devices
		# the renderer will run procedurals on different threads, but it equally well might call
		# them on the main thread. we force the procedural onto a separate thread so we can reliably
		# exercise a problem we're trying to address.
		t = threading.Thread( target=IECore.curry( r.procedural, RemovalProcedural() ) )
		t.start()
		t.join()

		# if an edit session removes objects which use gl resources (shaders, textures etc),
		# then it's essential that the editEnd call occurs on the thread with the correct gl context.
		# this is so the gl resources can be deleted in the correct context.
		r.command( "editEnd", {} )

		self.assertEqual( len( s.root().children() ), 0 )

	def testParallelRenders( self ):

		allScenes = []

		def threadedRendering():
			r = IECoreGL.Renderer()
			r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
			r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
			r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( os.path.dirname( __file__ ) + "/shaders/include" ) )

			r.worldBegin()
			r.shader( "surface", "failWithoutPreprocessing", {} )
			r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )
			r.worldEnd()
			allScenes.append( r.scene() )

		for i in range( 0, 100 ):
			newThread = threading.Thread(target=threadedRendering)
			newThread.start()

		while len(allScenes) < 100 :
			pass

		for s in allScenes :
			s.render( IECoreGL.State( True ) )

	class RecursiveProcedural( IECoreScene.Renderer.Procedural ):
		"""Creates a pyramid of spheres"""

		maxLevel = 5
		threadsUsed = set()

		def __init__( self, level = 0 ):
			IECoreScene.Renderer.Procedural.__init__( self )
			self.__level = level
			if level == 0 :
				self.threadsUsed.clear()

		def bound( self ) :
			return imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) )

		def render( self, renderer ):
			# registers this thread id
			self.threadsUsed.add( threading.currentThread().getName() )
			renderer.attributeBegin()
			renderer.setAttribute( "color", IECore.Color3fData( imath.Color3f( float(self.__level)/self.maxLevel, 0, 1 - float(self.__level)/self.maxLevel ) ) )
			renderer.transformBegin()
			renderer.concatTransform( imath.M44f().translate(imath.V3f( 0, 0.5, 0 )) )
			renderer.concatTransform( imath.M44f().scale( imath.V3f(0.5) ) )
			renderer.sphere( 1, -1, 1, 360, {} )
			renderer.transformEnd()
			# end of recursion
			if self.__level < self.maxLevel :
				renderer.transformBegin()
				renderer.concatTransform( imath.M44f().translate(imath.V3f( 0, -0.5, 0 )) )
				for i in range( 0, 2 ) :
					renderer.transformBegin()
					renderer.concatTransform( imath.M44f().translate(imath.V3f( (i - 0.5) , 0, 0)) )
					renderer.concatTransform( imath.M44f().scale( imath.V3f(0.5) ) )
					proc = TestRenderer.RecursiveProcedural( self.__level + 1 )
					renderer.procedural( proc )
					renderer.transformEnd()
				renderer.transformEnd()
			renderer.attributeEnd()

		def hash( self ):

			h = IECore.MurmurHash()
			return h

	def testMultithreadedProcedural( self ):

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( os.path.dirname( __file__ ) + "/shaders/include" ) )
		r.worldBegin()
		p = self.RecursiveProcedural()
		r.procedural( p )
		r.worldEnd()

		self.assert_( len(self.RecursiveProcedural.threadsUsed) > 1 )

	def testParallelMultithreadedProcedurals( self ):

		renders = []

		def newRender():
			r = IECoreGL.Renderer()
			r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
			r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
			r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( os.path.dirname( __file__ ) + "/shaders/include" ) )
			r.worldBegin()
			p = self.RecursiveProcedural()
			r.procedural( p )
			r.worldEnd()
			renders.append( 0 )

		threads = []
		for i in range( 0,10 ):
			newThread = threading.Thread(target=newRender)
			newThread.start()
			threads.append( newThread )

		for t in threads :
			t.join()

	def testDisableProceduralThreading( self ):

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( os.path.dirname( __file__ ) + "/shaders/include" ) )
		with IECoreScene.WorldBlock( r ) :
			r.setAttribute( "gl:procedural:reentrant", IECore.BoolData( False ) )
			p = self.RecursiveProcedural()
			r.procedural( p )

		self.assertEqual( len( self.RecursiveProcedural.threadsUsed ), 1 )

	def testObjectSpaceCulling( self ):

		p = self.RecursiveProcedural()

		def renderWithCulling( box ):
			r = IECoreGL.Renderer()
			r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
			r.worldBegin()
			r.sphere( 1.5, 0, 1, 360, {} )
			r.procedural( p )
			r.attributeBegin()
			if True:
				r.setAttribute( "gl:cullingSpace", IECore.StringData( "object" ) )
				r.setAttribute( "gl:cullingBox", IECore.Box3fData( box ) )
				# everything in this block is culled
				r.sphere( 1.5, 0, 1, 360, {} )
				r.procedural( p )
			r.attributeEnd()
			r.worldEnd()
			return self.__countChildrenRecursive( r.scene().root() )

		noCullingCounter = renderWithCulling( imath.Box3f() )

		# verify that only half of the things are renderer when the giving culling box is defined.
		self.assertEqual( renderWithCulling( imath.Box3f( imath.V3f(2,-1,-1), imath.V3f(3,1,1)  ) ) * 2, noCullingCounter )

	def testWorldSpaceCulling( self ):

		p = self.RecursiveProcedural()
		box = imath.Box3f( imath.V3f(0.001,-1,-1), imath.V3f(1,1,1) )

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.worldBegin()
		r.setAttribute( "gl:cullingSpace", IECore.StringData( "world" ) )
		r.setAttribute( "gl:cullingBox", IECore.Box3fData( box ) )
		r.sphere( 1, 0, 1, 360, {} )	# half-inside : 1 element
		r.procedural( p )	# half-inside: 32 elements (full procedural renders 63 elements)
		r.transformBegin()
		if True:
			r.concatTransform( imath.M44f().translate( imath.V3f(-2, 0, 0) ) )
			# everything in this block is culled
			r.sphere( 1, 0, 1, 360, {} )
			r.procedural( p )
		r.transformEnd()
		r.worldEnd()
		self.assertEqual( self.__countChildrenRecursive( r.scene().root() ), 33 )

	def testTransformsInImmediateRenderer( self ):
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.transformBegin()
		r.concatTransform( imath.M44f().rotate( imath.V3f( 1, 1, 1 ) ) )
		r.camera( "main", { "resolution" : IECore.V2iData( imath.V2i( 512 ) ), "projection" : IECore.StringData( "perspective" ) } )
		r.transformEnd()
		r.worldBegin()
		# confirm that the camera transformation is not affecting the world space matrix
		r.concatTransform( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) )
		self.assert_( r.getTransform().equalWithAbsError( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ), 1e-4 ) )
		# confirm that setting the world space transform does not affect the camera matrix (that was already set in openGL )
		r.setTransform( imath.M44f().translate( imath.V3f( 0, 1, 0 ) ) )
		self.assert_( r.getTransform().equalWithAbsError( imath.M44f().translate( imath.V3f( 0, 1, 0 ) ), 1e-4 ) )
		r.worldEnd()

	def testTransformsInDeferredRenderer( self ):
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.transformBegin()
		r.concatTransform( imath.M44f().rotate( imath.V3f( 1, 1, 1 ) ) )
		r.camera( "main", { "resolution" : IECore.V2iData( imath.V2i( 512 ) ), "projection" : IECore.StringData( "perspective" ) } )
		r.transformEnd()
		r.worldBegin()
		# confirm that the camera transformation is not affecting the world space matrix
		self.assert_( r.getTransform().equalWithAbsError( imath.M44f(), 1e-4 ) )
		r.concatTransform( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) )
		r.concatTransform( imath.M44f().rotate( imath.V3f( 1, 1, 1 ) ) )
		m = r.getTransform()
		r.transformBegin()
		if True:
			# confirm that the transformBegin did not change the current transform
			self.assert_( r.getTransform().equalWithAbsError( m, 1e-4 ) )
			# confirm that concatenate transform works
			r.concatTransform( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) )
			self.assert_( r.getTransform().equalWithAbsError( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) * m, 1e-4 ) )
			r.concatTransform( imath.M44f().scale( imath.V3f(0.5) ) )
			self.assert_( r.getTransform().equalWithAbsError( imath.M44f().scale( imath.V3f(0.5) ) * imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) * m, 1e-4 ) )

			# confirm that setting the world space transform works too
			m2 = imath.M44f().translate( imath.V3f( 0, 1, 0 ) )
			r.setTransform( m2 )
			self.assert_( r.getTransform().equalWithAbsError( m2, 1e-4 ) )

			r.attributeBegin()
			if True:
				# confirm that the attributeBegin did not change the current transform
				self.assert_( r.getTransform().equalWithAbsError( m2, 1e-4 ) )
				# confirm that setting the world space transform works too
				r.setTransform( imath.M44f().rotate( imath.V3f( 3, 1, 0 ) ) )
				self.assert_( r.getTransform().equalWithAbsError( imath.M44f().rotate( imath.V3f( 3, 1, 0 ) ), 1e-4 ) )
			r.attributeEnd()
			# confirms that attributeEnd recovers the matrix.
			self.assert_( r.getTransform().equalWithAbsError( m2, 1e-4 ) )

		r.transformEnd()
		# confirms that transformEnd recovers the matrix.
		self.assert_( r.getTransform().equalWithAbsError( m, 1e-4 ) )

		r.worldEnd()

	def testInstances(self):

		r = IECoreGL.Renderer()
		r.instanceBegin( "instanceA", {} )
		r.concatTransform( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) )
		r.transformBegin()
		r.concatTransform( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) )
		r.sphere( 1, -1, 1, 360, {} )
		r.concatTransform( imath.M44f().translate( imath.V3f( 1, 0, 0 ) ) )
		r.sphere( 1, -1, 1, 360, {} )
		r.transformEnd()
		r.concatTransform( imath.M44f().translate( imath.V3f( -1, 0, 0 ) ) )
		r.sphere( 1, -1, 1, 360, {} )
		r.instanceEnd()

		r.instanceBegin( "instanceB", {} )
		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, 10 ) ) )
		r.instance( "instanceA" )
		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, 20 ) ) )
		r.instance( "instanceA" )
		r.instanceEnd()

		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.worldBegin()
		r.concatTransform( imath.M44f().translate( imath.V3f( 0, 5, 0 ) ) )
		r.instance( "instanceB" )
		r.setTransform( imath.M44f().translate( imath.V3f( 0, 10, 0 ) ) )
		r.instance( "instanceB" )
		r.worldEnd()

		g = r.scene().root()

		self.assertEqual( self.__countChildrenRecursive( g ), 12 )
		self.assert_( g.bound().min().equalWithAbsError( imath.V3f( -1, 4, 9 ), 0.001 ) )
		self.assert_( g.bound().max().equalWithAbsError( imath.V3f( 4, 11, 31 ), 0.001 ) )

	def testCuriousCrashOnThreadedProceduralsAndAttribute( self ):

		myMesh = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob").read()

		class MyProc( IECoreScene.Renderer.Procedural ):
			def __init__( self, level = 0 ):
				IECoreScene.Renderer.Procedural.__init__( self )
				self.__level = level
			def bound( self ) :
				return imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) )
			def render( self, renderer ):
				if self.__level < 2 :
					for i in range( 0, 50 ) :
						renderer.procedural( MyProc( self.__level + 1 ) )
				else:
					g = IECoreScene.Group()
					g.addChild( myMesh )
					g.addState( IECoreScene.AttributeState( { "name" : IECore.StringData( str(self.__level) ) } ) )
					g.render( renderer )

			def hash( self ):
				h = IECore.MurmurHash()
				return h

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.worldBegin()
		p = MyProc()
		r.procedural( p )
		r.worldEnd()

	def testDepthTest( self ) :

		def doTest( depthTest, r, g, b ) :

			renderer = IECoreGL.Renderer()
			renderer.setOption( "gl:mode", IECore.StringData( "immediate" ) )
			renderer.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

			renderer.camera( "main", {
					"projection" : IECore.StringData( "orthographic" ),
					"resolution" : IECore.V2iData( imath.V2i( 256 ) ),
					"clippingPlanes" : IECore.V2fData( imath.V2f( 1, 1000 ) ),
					"screenWindow" : IECore.Box2fData( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
				}
			)
			renderer.display( os.path.dirname( __file__ ) + "/output/depthTest.tif", "tif", "rgba", {} )

			m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

			with IECoreScene.WorldBlock( renderer ) :

				renderer.setAttribute( "gl:depthTest", IECore.BoolData( depthTest ) )

				renderer.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -1 ) ) )
				renderer.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )
				m.render( renderer )

				renderer.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -1 ) ) )
				renderer.shader( "surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 0, 1, 0 ) ) } )
				m.render( renderer )

			i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/depthTest.tif" ).read()
			for p in i["R"] :
				self.assertEqual( p, r )
			for p in i["G"] :
				self.assertEqual( p, g )
			for p in i["B"] :
				self.assertEqual( p, b )

		doTest( True, 1, 0, 0 )
		doTest( False, 0, 1, 0 )

	def testCameraVisibility( self ) :

		def doRender( mode, visibility ) :

			r = IECoreGL.Renderer()
			r.setOption( "gl:mode", IECore.StringData( mode ) )
			r.setOption( "gl:searchPath:shaderInclude", IECore.StringData( "./glsl" ) )

			r.camera( "main", {
					"projection" : IECore.StringData( "perspective" ),
					"projection:fov" : IECore.FloatData( 20 ),
					"resolution" : IECore.V2iData( imath.V2i( 256 ) ),
					"clippingPlanes" : IECore.V2fData( imath.V2f( 1, 1000 ) ),
					"screenWindow" : IECore.Box2fData( imath.Box2f( imath.V2f( -3 ), imath.V2f( 3 ) ) )
				}
			)
			if mode=="immediate" :
				r.display( os.path.dirname( __file__ ) + "/output/testCameraVisibility.tif", "tif", "rgba", {} )

			with IECoreScene.WorldBlock( r ) :

				r.concatTransform( imath.M44f().translate( imath.V3f( 0, 0, -5 ) ) )

				r.setAttribute( "gl:visibility:camera", IECore.BoolData( visibility ) )
				r.points( 1, { "P" : IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 0 ) ] ) ) } )

			return r

		# test immediate renderer by checking images

		doRender( "immediate", True )
		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/testCameraVisibility.tif" ).read()
		self.failUnless( i["A"][256 * 128 + 128] > .99 )

		doRender( "immediate", False )
		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/output/testCameraVisibility.tif" ).read()
		self.assertEqual( i["A"], IECore.FloatVectorData( [ 0 ] * 256 * 256 ) )

		# test deferred renderer by checking scene

		r = doRender( "deferred", True )
		self.assertEqual(  len( r.scene().root().children()[0].children() ), 1 )

		r = doRender( "deferred", False )
		self.assertEqual( len( r.scene().root().children() ), 0 )

	def testWarningMessages( self ):
		r = IECoreGL.Renderer()

		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		# gl renderer only supports "surface" shaders, so it should complain about this:
		c = IECore.CapturingMessageHandler()
		with c :
			with IECoreScene.WorldBlock( r ):
				r.shader( "shader", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )

		self.assertEqual( len( c.messages ), 1 )
		self.assertEqual( c.messages[0].level, IECore.Msg.Level.Warning )

		# it should just ignore this, because of the "ri:" prefix:
		c = IECore.CapturingMessageHandler()
		with c :
			with IECoreScene.WorldBlock( r ):
				r.shader( "ri:shader", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )

		self.assertEqual( len( c.messages ), 0 )

		# this should work fine:
		c = IECore.CapturingMessageHandler()
		with c :
			with IECoreScene.WorldBlock( r ):
				r.shader( "gl:surface", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )

		self.assertEqual( len( c.messages ), 0 )


		# it should just ignore this, because of the "lg:" prefix:
		c = IECore.CapturingMessageHandler()
		with c :
			with IECoreScene.WorldBlock( r ):
				r.shader( "lg:shader", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )

		self.assertEqual( len( c.messages ), 0 )

		# this aint right!:
		c = IECore.CapturingMessageHandler()
		with c :
			with IECoreScene.WorldBlock( r ):
				r.shader( "gl:nonsense", "color", { "colorValue" : IECore.Color3fData( imath.Color3f( 1, 0, 0 ) ) } )

		self.assertEqual( len( c.messages ), 1 )
		self.assertEqual( c.messages[0].level, IECore.Msg.Level.Warning )

	def setUp( self ) :

		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )

	def tearDown( self ) :

		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
