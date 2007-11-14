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
import os.path

from IECore import *

from IECoreGL import *
init( False )

class TestShader( unittest.TestCase ) :

	def testConstructor( self ) :
	
		self.assertRaises( RuntimeError, Shader, "i don't think i'm valid", "me neither" )
		
		vertexSource = """
		void main()
		{
			gl_Position = ftransform();
		}
		"""
		
		fragmentSource = """
		void main()
		{
			gl_FragColor = vec4( 1, 0.5, 0.25, 1 );
		}
		"""
		
		Shader( vertexSource, fragmentSource )

	def testParameters( self ) :
	
		vertexSource = """
		attribute float floatAttrib;
		varying float varyingFloatParm;
		void main()
		{
			gl_Position = ftransform();
			varyingFloatParm = floatAttrib * gl_Position.x;
		}
		"""
	
		fragmentSource = """
		uniform bool boolParm;
		uniform int intParm;
		uniform float floatParm;
		
		uniform bvec2 bvec2Parm;
		uniform bvec3 bvec3Parm;
		// uniform ivec4 bvec4Parm; // we have no suitable datatype for specifying this in IECore
		
		uniform ivec2 ivec2Parm;
		uniform ivec3 ivec3Parm;
		// uniform ivec4 ivec4Parm; // we have no suitable datatype for specifying this in IECore
		
		uniform vec2 vec2Parm;
		uniform vec3 vec3Parm;
		uniform vec4 vec4Parm;
		
		uniform sampler2D s2D;
				
		uniform struct {
			int i;
			float f;
		} s;
		
		uniform mat3 mat3Parm;
		uniform mat4 mat4Parm;
		
		varying float varyingFloatParm;
		
		void main()
		{
			float x = vec4Parm.r + vec3Parm.g + vec2Parm.y + floatParm + float( intParm ) + float( boolParm );
			float xx = float( ivec2Parm.x ) + float( ivec3Parm.y ) + float( bvec2Parm.x ) + float( bvec3Parm.y );
			float xxx = float( s.i ) + s.f + texture2D( s2D, vec2Parm ).r;
			vec4 p = mat4Parm * gl_FragCoord;
			vec3 pp = mat3Parm * gl_FragCoord.xyz;
			gl_FragColor = vec4( x + xx + xxx + p.x + pp.x, gl_Color.g, varyingFloatParm, 1 );
		}
		"""

		s = Shader( vertexSource, fragmentSource )
		self.assert_( s==s )
		
		expectedParameterNamesAndTypes = {
			"boolParm" : TypeId.BoolData,
			"intParm" : TypeId.IntData,
			"floatParm" : TypeId.FloatData,
			"bvec2Parm" : TypeId.V2iData,
			"bvec3Parm" : TypeId.V3iData,
			"ivec2Parm" : TypeId.V2iData,
			"ivec3Parm" : TypeId.V3iData,
			"vec2Parm" : TypeId.V2fData,
			"vec3Parm" : TypeId.V3fData,
			"vec4Parm" : TypeId.Color4fData,
			"s.f" : TypeId.FloatData,
			"s.i" : TypeId.IntData,
			"s2D" : Texture.staticTypeId(),
			"mat3Parm" : TypeId.M33fData,
			"mat4Parm" : TypeId.M44fData,
		}
		
		parameterNames = s.parameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndTypes ) )
		for n in expectedParameterNamesAndTypes.keys() :
			self.assert_( n in parameterNames )
			self.assert_( s.hasParameter( n ) )
			self.assert_( not s.hasParameter( n + "VeryUnlikelySuffix" ) )
			self.assertEqual( s.parameterType( n ), expectedParameterNamesAndTypes[n] )
			
		expectedNamesAndValues = {
			"boolParm" : BoolData( 0 ),
			"intParm" : IntData( 0 ),
			"floatParm" : FloatData( 0 ),
			"bvec2Parm" : V2iData( V2i( 0 ) ),
			"ivec2Parm" : V2iData( V2i( 0 ) ),
			"vec2Parm" : V2fData( V2f( 0 ) ),
			"bvec3Parm" : V3iData( V3i( 0 ) ),
			"ivec3Parm" : V3iData( V3i( 0 ) ),
			"vec3Parm" : V3fData( V3f( 0 ) ),
			"vec4Parm" : Color4fData( Color4f( 0 ) ),
		}
		for name, value in expectedNamesAndValues.items() :
			self.assertEqual( s.getParameter( name ), value )
		
		# must bind a shader before setting parameters
		s.bind()
		
		s.setParameter( "intParm", IntData( 1 ) )
		self.assertEqual( s.getParameter( "intParm" ), IntData( 1 ) )
		
		s.setParameter( "boolParm", IntData( 2 ) )
		self.assertEqual( s.getParameter( "boolParm" ), BoolData( 1 ) )
		
		s.setParameter( "boolParm", BoolData( False ) )
		self.assertEqual( s.getParameter( "boolParm" ), BoolData( False ) )
		
		s.setParameter( "boolParm", BoolData( True ) )
		self.assertEqual( s.getParameter( "boolParm" ), BoolData( True ) )
		
		s.setParameter( "boolParm", BoolData( 2 ) )
		self.assertEqual( s.getParameter( "boolParm" ), BoolData( 1 ) )

		s.setParameter( "floatParm", FloatData( 1 ) )
		self.assertEqual( s.getParameter( "floatParm" ), FloatData( 1 ) )

		s.setParameter( "bvec2Parm", V2iData( V2i( 0, 1 ) ) )
		self.assertEqual( s.getParameter( "bvec2Parm" ), V2iData( V2i( 0, 1 ) ) )

		s.setParameter( "bvec3Parm", V3iData( V3i( 1 ) ) )
		self.assertEqual( s.getParameter( "bvec3Parm" ), V3iData( V3i( 1 ) ) )

		s.setParameter( "ivec2Parm", V2iData( V2i( 1, 2 ) ) )
		self.assertEqual( s.getParameter( "ivec2Parm" ), V2iData( V2i( 1, 2 ) ) )

		s.setParameter( "ivec3Parm", V3iData( V3i( 1 ) ) )
		self.assertEqual( s.getParameter( "ivec3Parm" ), V3iData( V3i( 1 ) ) )

		s.setParameter( "vec2Parm", V2fData( V2f( 1, 2 ) ) )
		self.assertEqual( s.getParameter( "vec2Parm" ), V2fData( V2f( 1, 2 ) ) )

		s.setParameter( "vec3Parm", Color3fData( Color3f( 1 ) ) )
		self.assertEqual( s.getParameter( "vec3Parm" ), V3fData( V3f( 1 ) ) )

		s.setParameter( "vec4Parm", Color4fData( Color4f( 1 ) ) )
		self.assertEqual( s.getParameter( "vec4Parm" ), Color4fData( Color4f( 1 ) ) )

		s.setParameter( "s.i", IntData( 1 ) )
		self.assertEqual( s.getParameter( "s.i" ), IntData( 1 ) )

		s.setParameter( "s.f", FloatData( 1 ) )
		self.assertEqual( s.getParameter( "s.f" ), FloatData( 1 ) )
		
		s.setParameter( "mat3Parm", M33fData( M33f.createTranslated( V2f( 1, 2 ) ) ) )
		self.assertEqual( s.getParameter( "mat3Parm" ), M33fData( M33f.createTranslated( V2f( 1, 2 ) ) ) )
		
		s.setParameter( "mat4Parm", M44fData( M44f.createTranslated( V3f( 1, 2, 3 ) ) ) )
		self.assertEqual( s.getParameter( "mat4Parm" ), M44fData( M44f.createTranslated( V3f( 1, 2, 3 ) ) ) )
		
		# check that setting invalid values throws an Exception
		self.assertRaises( Exception, s.setParameter, "iDontExist", FloatData( 2 ) )
		self.assertRaises( Exception, s.setParameter, "boolParm", FloatData( 2 ) )
		self.assertRaises( Exception, s.setParameter, "intParm", FloatData( 2 ) )
		self.assertRaises( Exception, s.setParameter, "floatParm", IntData( 2 ) )
		self.assertRaises( Exception, s.setParameter, "ivec2Parm", V3iData( V3i( 2 ) ) )
		self.assertRaises( Exception, s.setParameter, "ivec3Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setParameter, "vec2Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setParameter, "vec3Parm", V2fData( V2f( 2 ) ) )
		self.assertRaises( Exception, s.setParameter, "vec4Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setParameter, "mat4Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setParameter, "mat3Parm", V3fData( V3f( 2 ) ) )

		# check that both booldata is valid for bools
		self.assert_( s.valueValid( "boolParm", BoolData( True ) ) )

	def testRendererCall( self ) :
		
		## \todo We need image output from the renderer so we don't have to look at
		# it in a window.
		
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		r.setOption( "gl:searchPath:texture", StringData( os.path.dirname( __file__ ) + "/images" ) )
		r.worldBegin()
		
		# we have to make this here so that the shaders that get made are made in the
		# correct GL context. My understanding is that all shaders should work in all
		# GL contexts in the address space, but that doesn't seem to be the case.
		#w = SceneViewer( "scene", r.scene() )
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		
		r.attributeBegin()
		r.setAttribute( "color", Color3fData( Color3f( 1, 0, 1 ) ) )
		r.shader( "surface", "grey", { "greyValue" : FloatData( 0.5 ) } )
		r.geometry( "sphere", {}, {} )
		r.attributeEnd()
		
		r.attributeBegin()
		r.concatTransform( M44f.createTranslated( V3f( -1, 0, 0 ) ) )
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 0.5, 0.25 ) ) } )
		r.geometry( "sphere", {}, {} )
		r.attributeEnd()
		
		r.attributeBegin()
		r.concatTransform( M44f.createTranslated( V3f( 1, 0, 0 ) ) )
		r.shader( "surface", "image", { "texture" : StringData( "colorBarsH512x512.exr" ) } )
		r.geometry( "sphere", {}, {} )
		r.attributeEnd()
		
		r.attributeBegin()
		vertexSource = """
		void main()
		{
			gl_Position = ftransform() + vec4( 0, 1, 0, 0 );
		}
		"""
		
		fragmentSource = """
		void main()
		{
			gl_FragColor = vec4( 1, 0, 0, 1 );
		}
		"""
		
		r.shader( "surface", "redTranslated", { "gl:vertexSource" : StringData( vertexSource ), "gl:fragmentSource" : StringData( fragmentSource ) } )
		r.geometry( "sphere", {}, {} )
		r.attributeEnd()
		
		r.worldEnd()
	
		#w.start()
		
if __name__ == "__main__":
    unittest.main()   
