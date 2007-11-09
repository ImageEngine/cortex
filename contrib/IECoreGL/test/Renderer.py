##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

import VersionControl
VersionControl.setVersion( "IECore", "2" ) ## \todo need to get version from build config somehow, or have the coreGL module initialise the core module somehow
from IECore import *

from IECoreGL import *
init( False )

class TestRenderer( unittest.TestCase ) :

	def testOptions( self ) :
	
		os.environ["IECOREGL_TEXTURE_PATHS"] = "textureDefault"
		os.environ["IECOREGL_SHADER_PATHS"] = "shaderDefault"
		
		r = Renderer()
		
		self.assertEqual( r.typeName(), "IECoreGL::Renderer" )
		
		self.assertEqual( r.getOption( "searchPath:texture" ), StringData( "textureDefault" ) )
		self.assertEqual( r.getOption( "gl:searchPath:texture" ), StringData( "textureDefault" ) )
		
		r.setOption( "searchPath:texture", StringData( "a" ) )
		self.assertEqual( r.getOption( "searchPath:texture" ), StringData( "a" ) )
		self.assertEqual( r.getOption( "gl:searchPath:texture" ), StringData( "a" ) )
		
		r.setOption( "gl:searchPath:texture", StringData( "b" ) )
		self.assertEqual( r.getOption( "searchPath:texture" ), StringData( "b" ) )
		self.assertEqual( r.getOption( "gl:searchPath:texture" ), StringData( "b" ) )
		
		self.assertEqual( r.getOption( "searchPath:shader" ), StringData( "shaderDefault" ) )
		self.assertEqual( r.getOption( "gl:searchPath:shader" ), StringData( "shaderDefault" ) )
		
		r.setOption( "searchPath:shader", StringData( "s" ) )
		self.assertEqual( r.getOption( "searchPath:shader" ), StringData( "s" ) )
		self.assertEqual( r.getOption( "gl:searchPath:shader" ), StringData( "s" ) )
		
		r.setOption( "gl:searchPath:shader", StringData( "t" ) )
		self.assertEqual( r.getOption( "searchPath:shader" ), StringData( "t" ) )
		self.assertEqual( r.getOption( "gl:searchPath:shader" ), StringData( "t" ) )
		
		self.assertEqual( r.getOption( "shutter" ), V2fData( V2f( 0 ) ) )
		r.setOption( "shutter", V2fData( V2f( 1, 2 ) ) )
		self.assertEqual( r.getOption( "shutter" ), V2fData( V2f( 1, 2 ) ) )
		
	def testAttributes( self ) :
	
		deferred = Renderer()
		deferred.setOption( "gl:mode", StringData( "deferred" ) )
		
		immediate = Renderer()
		immediate.setOption( "gl:mode", StringData( "immediate" ) )
		
		
		for r in [ deferred, immediate ] :
		
			r.worldBegin()
		
			self.assertEqual( r.getAttribute( "color" ), Color3fData( Color3f( 1 ) ) )
			self.assertEqual( r.getAttribute( "opacity" ), Color3fData( Color3f( 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:color" ), Color4fData( Color4f( 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:blend:color" ), Color4fData( Color4f( 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:blend:srcFactor" ), StringData( "srcAlpha" ) )
			self.assertEqual( r.getAttribute( "gl:blend:dstFactor" ), StringData( "oneMinusSrcAlpha" ) )
			self.assertEqual( r.getAttribute( "gl:blend:equation" ), StringData( "add" ) )
			self.assertEqual( r.getAttribute( "gl:shade:transparent" ), BoolData( False ) )
			self.assertEqual( r.getAttribute( "gl:primitive:sortForTransparency" ), BoolData( True ) )
		
			r.setAttribute( "color", Color3fData( Color3f( 0, 1, 2 ) ) )
			self.assertEqual( r.getAttribute( "color" ), Color3fData( Color3f( 0, 1, 2 ) ) )
		
			# opacity is an odd one - it's set as a color but as it's averaged internally
			# the result you get should be a greyscale value.
			r.setAttribute( "opacity", Color3fData( Color3f( 3, 1, 2 ) ) )
			self.assertEqual( r.getAttribute( "opacity" ), Color3fData( Color3f( 2 ) ) )
		
			self.assertEqual( r.getAttribute( "gl:color" ), Color4fData( Color4f( 0, 1, 2, 2 ) ) )
			r.setAttribute( "gl:color", Color4fData( Color4f( 1, 2, 3, 4 ) ) )
			self.assertEqual( r.getAttribute( "gl:color" ), Color4fData( Color4f( 1, 2, 3, 4 ) ) )
		
			r.setAttribute( "gl:blend:color", Color4fData( Color4f( 0, 1, 0, 1 ) ) )
			self.assertEqual( r.getAttribute( "gl:blend:color" ), Color4fData( Color4f( 0, 1, 0, 1 ) ) )
		
			r.attributeBegin()
			r.setAttribute( "color", Color3fData( Color3f( 0 ) ) )
			self.assertEqual( r.getAttribute( "gl:color" ), Color4fData( Color4f( 0, 0, 0, 4 ) ) )
			r.attributeEnd()
			self.assertEqual( r.getAttribute( "gl:color" ), Color4fData( Color4f( 1, 2, 3, 4 ) ) )
		
			factors = [ "zero", "one", "srcColor", "oneMinusSrcColor", "dstColor", "oneMinusDstColor",
				"srcAlpha", "oneMinusSrcAlpha", "dstAlpha", "oneMinusDstAlpha", "dstAlpha", "oneMinusDstAlpha",
				"constantColor", "oneMinusConstantColor", "constantAlpha", "oneMinusConstantAlpha" ]
			for f in factors :
				last = r.getAttribute( "gl:blend:dstFactor" )
				r.setAttribute( "gl:blend:srcFactor", StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:srcFactor" ), StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:dstFactor" ), last )
				last = r.getAttribute( "gl:blend:srcFactor" )
				r.setAttribute( "gl:blend:dstFactor", StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:srcFactor" ), StringData( f ) )
				self.assertEqual( r.getAttribute( "gl:blend:dstFactor" ), last )
			
			for e in ["add", "subtract", "reverseSubtract", "min", "max"] :
				r.setAttribute( "gl:blend:equation", StringData( e ) )
				self.assertEqual( r.getAttribute( "gl:blend:equation" ), StringData( e ) )
				
			r.worldEnd()
	
	def testStacks( self ) :
	
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( "test/shaders" ) )
		r.worldBegin()
		
		# we have to make this here so that the shaders that get made are made in the
		# correct GL context. My understanding is that all shaders should work in all
		# GL contexts in the address space, but that doesn't seem to be the case.
		#w = SceneViewer( "scene", r.scene() )
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 0, 0 ) ) } )
		r.geometry( "sphere", {}, {} )
		
		r.concatTransform( M44f.createTranslated( V3f( -1, 0, 0 ) ) )
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 0, 1, 0 ) ) } )
		r.geometry( "sphere", {}, {} )
		
		r.concatTransform( M44f.createTranslated( V3f( 2, 0, 0 ) ) )
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 0, 0, 1 ) ) } )
		r.geometry( "sphere", {}, {} )
		
		r.worldEnd()
	
		#w.start()
		
	def testStackBug( self ) :
	
		# This should produce a yellow sphere in between two red spheres.
		
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( "test/shaders" ) )
		r.worldBegin()
		
		#w = SceneViewer( "scene", r.scene() )
		
		r.shader( "surface", "rgbColor", { "red" : FloatData( 1 ), "green" : FloatData( 0 ), "blue" : FloatData( 0 ) } )
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		
		r.attributeBegin()

		r.shader( "surface", "rgbColor", { "red" : FloatData( 1 ), "green" : FloatData( 1 ), "blue" : FloatData( 0 ) } )
		r.geometry( "sphere", {}, {} )
		
		r.attributeEnd()
		
		r.concatTransform( M44f.createTranslated( V3f( -1, 0, 0 ) ) )
		r.geometry( "sphere", {}, {} )
		
		r.concatTransform( M44f.createTranslated( V3f( 2, 0, 0 ) ) )
		r.geometry( "sphere", {}, {} )
		
		
		r.worldEnd()
	
		#w.start()
		
	def testPrimVars( self ) :
		
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( "test/shaders" ) )
		r.worldBegin()
		
		#w = SceneViewer( "scene", r.scene() )
		
		r.shader( "surface", "rgbColor", { "red" : FloatData( 1 ), "green" : FloatData( 0 ), "blue" : FloatData( 0 ) } )
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		
		r.attributeBegin()

		# the central sphere should be yellow
		r.geometry( "sphere", {}, { "green" : PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 1 ) ) } )
		
		r.attributeEnd()
		
		r.concatTransform( M44f.createTranslated( V3f( -1, 0, 0 ) ) )
		r.geometry( "sphere", {}, {} )
		
		r.concatTransform( M44f.createTranslated( V3f( 2, 0, 0 ) ) )
		r.geometry( "sphere", {}, {} )
		
		r.worldEnd()
	
		#w.start()
		
	def testImagePrimitive( self ) :
	
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( "test/shaders" ) )
		
		r.worldBegin()
		#w = SceneViewer( "scene", r.scene() )		
		
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 0, 0 ) ) } )
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		r.concatTransform( M44f.createScaled( V3f( 0.005 ) ) )
		
		i = Reader.create( "test/images/numbers.exr" ).read()
		i.render( r )
		
		r.worldEnd()
	
		#w.start()
		
	def testAlphaBlending( self ) :
	
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( "test/shaders" ) )
		
		r.worldBegin()
		#w = SceneViewer( "scene", r.scene() )		
		
		r.setAttribute( "gl:blend:srcFactor", StringData( "one" ) )
		r.setAttribute( "gl:blend:dstFactor", StringData( "one" ) )
		r.setAttribute( "gl:blend:equation", StringData( "add" ) )
				
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		r.concatTransform( M44f.createScaled( V3f( 0.004 ) ) )
		
		r.concatTransform( M44f.createTranslated( V3f( -150, -200, 0 ) ) )
		i = Reader.create( "test/images/numberWithAlpha.exr" ).read()
		i.render( r )
		
		r.concatTransform( M44f.createTranslated( V3f( 300, 300, 1 ) ) )
		i.render( r )
		
		r.worldEnd()
	
		#w.start()
	
	def testProcedural( self ) :
	
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( "test/shaders" ) )
		
		r.worldBegin()
		#w = SceneViewer( "scene", r.scene() )		
		
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 0, 0 ) ) } )

		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		
		p = ReadProcedural()
		p.files["name"].setValue( StringData( "test/models/sphere.cob" ) )
		r.procedural( p )
		
		r.worldEnd()
	
		#w.start()
			
if __name__ == "__main__":
    unittest.main()   
