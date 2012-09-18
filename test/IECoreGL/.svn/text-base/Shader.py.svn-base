##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

	def testUniformParameters( self ) :

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

		parameterNames = s.uniformParameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndTypes ) )
		for n in expectedParameterNamesAndTypes.keys() :
			self.assert_( n in parameterNames )
			self.assert_( s.hasUniformParameter( n ) )
			self.assert_( not s.hasUniformParameter( n + "VeryUnlikelySuffix" ) )
			self.assertEqual( s.uniformParameterType( n ), expectedParameterNamesAndTypes[n] )

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
		# Initial values are not necessarily zero. So we just test for the value type here.
		for name, value in expectedNamesAndValues.items() :
			self.assertEqual( type(s.getUniformParameter( name )), type(value) )

		# must bind a shader before setting parameters
		s.bind()

		s.setUniformParameter( "intParm", IntData( 1 ) )
		self.assertEqual( s.getUniformParameter( "intParm" ), IntData( 1 ) )

		s.setUniformParameter( "boolParm", IntData( 2 ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( 1 ) )

		s.setUniformParameter( "boolParm", BoolData( False ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( False ) )

		s.setUniformParameter( "boolParm", BoolData( True ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( True ) )

		s.setUniformParameter( "boolParm", BoolData( 2 ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( 1 ) )

		s.setUniformParameter( "floatParm", FloatData( 1 ) )
		self.assertEqual( s.getUniformParameter( "floatParm" ), FloatData( 1 ) )

		s.setUniformParameter( "bvec2Parm", V2iData( V2i( 0, 1 ) ) )
		self.assertEqual( s.getUniformParameter( "bvec2Parm" ), V2iData( V2i( 0, 1 ) ) )

		s.setUniformParameter( "bvec3Parm", V3iData( V3i( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "bvec3Parm" ), V3iData( V3i( 1 ) ) )

		s.setUniformParameter( "ivec2Parm", V2iData( V2i( 1, 2 ) ) )
		self.assertEqual( s.getUniformParameter( "ivec2Parm" ), V2iData( V2i( 1, 2 ) ) )

		s.setUniformParameter( "ivec3Parm", V3iData( V3i( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "ivec3Parm" ), V3iData( V3i( 1 ) ) )

		s.setUniformParameter( "vec2Parm", V2fData( V2f( 1, 2 ) ) )
		self.assertEqual( s.getUniformParameter( "vec2Parm" ), V2fData( V2f( 1, 2 ) ) )

		s.setUniformParameter( "vec3Parm", Color3fData( Color3f( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "vec3Parm" ), V3fData( V3f( 1 ) ) )

		s.setUniformParameter( "vec4Parm", Color4fData( Color4f( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "vec4Parm" ), Color4fData( Color4f( 1 ) ) )

		s.setUniformParameter( "s.i", IntData( 1 ) )
		self.assertEqual( s.getUniformParameter( "s.i" ), IntData( 1 ) )

		s.setUniformParameter( "s.f", FloatData( 1 ) )
		self.assertEqual( s.getUniformParameter( "s.f" ), FloatData( 1 ) )

		s.setUniformParameter( "mat3Parm", M33fData( M33f.createTranslated( V2f( 1, 2 ) ) ) )
		self.assertEqual( s.getUniformParameter( "mat3Parm" ), M33fData( M33f.createTranslated( V2f( 1, 2 ) ) ) )

		s.setUniformParameter( "mat4Parm", M44fData( M44f.createTranslated( V3f( 1, 2, 3 ) ) ) )
		self.assertEqual( s.getUniformParameter( "mat4Parm" ), M44fData( M44f.createTranslated( V3f( 1, 2, 3 ) ) ) )

		# check that setting invalid values throws an Exception
		self.assertRaises( Exception, s.setUniformParameter, "iDontExist", FloatData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "boolParm", FloatData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "intParm", FloatData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "floatParm", IntData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec2Parm", V3iData( V3i( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec3Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec2Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec3Parm", V2fData( V2f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec4Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat4Parm", V3fData( V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat3Parm", V3fData( V3f( 2 ) ) )

		# check that both booldata is valid for bools
		self.assert_( s.uniformValueValid( "boolParm", BoolData( True ) ) )

		# check uniform parameters defined from vector items
		self.assert_( s.uniformVectorValueValid( "vec2Parm", V2fVectorData( [ V2f() ] ) ) )
		self.assert_( not s.uniformVectorValueValid( "vec2Parm", FloatVectorData( [ 1 ] ) ) )
		self.assert_( s.uniformVectorValueValid( "floatParm", FloatVectorData( [ 1 ] ) ) )
		self.assert_( not s.uniformVectorValueValid( "floatParm", IntVectorData( [ 1 ] ) ) )
		self.assert_( s.uniformVectorValueValid( "vec3Parm", V3fVectorData( [ V3f() ] ) ) )
		self.assert_( s.uniformVectorValueValid( "vec3Parm", Color3fVectorData( [ Color3f() ] ) ) )
		self.assert_( not s.uniformVectorValueValid( "vec3Parm", V2fVectorData( [ V2f() ] ) ) )

		v = V2fVectorData( [ V2f(9,9), V2f(1,2), V2f(9,9) ] )
		s.setUniformParameterFromVector( "vec2Parm", v, 1 )
		self.assertEqual( s.getUniformParameter( "vec2Parm" ), V2fData( V2f( 1, 2 ) ) )
	
	def testUniformArrayParameters( self ) :
		
		# TODO: get bool/bvec2/bvec3 array parameters working, and test them.
		
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
		// uniform bool boolParm[4];
		uniform int intParm[4];
		uniform float floatParm[4];
		
		// uniform bvec2 bvec2Parm[4];
		// uniform bvec3 bvec3Parm[4];
		// uniform ivec4 bvec4Parm; // we have no suitable datatype for specifying this in IECore
		
		uniform ivec2 ivec2Parm[4];
		uniform ivec3 ivec3Parm[4];
		// uniform ivec4 ivec4Parm; // we have no suitable datatype for specifying this in IECore

		uniform vec2 vec2Parm[4];
		uniform vec3 vec3Parm[4];
		uniform vec4 vec4Parm[4];

		//uniform sampler2D s2D[4];

		uniform struct {
			int i;
			float f;
		} s;

		uniform mat3 mat3Parm[4];
		uniform mat4 mat4Parm[4];
		
		varying float varyingFloatParm;

		void main()
		{
			float x = vec4Parm[0].r + vec3Parm[0].g + vec2Parm[0].y + floatParm[0] + float( intParm[0] ); // + float( boolParm[0] );
			float xx = float( ivec2Parm[0].x ) + float( ivec3Parm[0].y ); // + float( bvec2Parm[0].x ) + float( bvec3Parm[0].y );
			float xxx = vec2Parm[0].x;
			vec4 p = mat4Parm[0] * gl_FragCoord;
			vec3 pp = mat3Parm[0] * gl_FragCoord.xyz;
			gl_FragColor = vec4( x + xx + xxx + p.x + pp.x, gl_Color.g, varyingFloatParm, 1 );
		}
		"""

		s = Shader( vertexSource, fragmentSource )
		self.assert_( s==s )

		expectedParameterNamesAndTypes = {
			"intParm" : TypeId.IntVectorData,
			"floatParm" : TypeId.FloatVectorData,
			"ivec2Parm" : TypeId.V2iVectorData,
			"ivec3Parm" : TypeId.V3iVectorData,
			"vec2Parm" : TypeId.V2fVectorData,
			"vec3Parm" : TypeId.V3fVectorData,
			"vec4Parm" : TypeId.Color4fVectorData,
			"mat3Parm" : TypeId.M33fVectorData,
			"mat4Parm" : TypeId.M44fVectorData,
		}

		parameterNames = s.uniformParameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndTypes ) )
		
		for n in expectedParameterNamesAndTypes.keys() :
			self.assert_( n in parameterNames )
			self.assert_( s.hasUniformParameter( n ) )
			self.assert_( not s.hasUniformParameter( n + "VeryUnlikelySuffix" ) )
			
			self.assertEqual( s.uniformParameterType( n ), expectedParameterNamesAndTypes[n] )

		expectedNamesAndValues = {
			"intParm" : IntVectorData( ),
			"floatParm" : FloatVectorData( ),
			"ivec2Parm" : V2iVectorData( ),
			"vec2Parm" : V2fVectorData( ),
			"ivec3Parm" : V3iVectorData( ),
			"vec3Parm" : V3fVectorData( ),
			"vec4Parm" : Color4fVectorData( ),
		}
		
		# Initial values are not necessarily zero. So we just test for the value type here.
		for name, value in expectedNamesAndValues.items() :
			self.assertEqual( type(s.getUniformParameter( name ) ), type( value ) )

		# must bind a shader before setting parameters
		s.bind()
		
		def checkGetSet( test, s, paramName, paramData, expected=None ) :
			s.setUniformParameter( paramName, paramData )
			fetchedData = s.getUniformParameter( paramName )
			if expected:
				test.assertEqual( fetchedData, expected )
			else :
				test.assertEqual( fetchedData, paramData )
				
		
		checkGetSet( self, s, "intParm", IntVectorData( range(0,4) ) )
		checkGetSet( self, s, "floatParm", FloatVectorData( [ 0.0, 1.0, 2.0, 3.0 ]) )
		checkGetSet( self, s, "ivec2Parm", V2iVectorData( [ V2i( 1, 2 ), V2i( 5, 4 ), V2i( 8, 5 ), V2i( 2, 3 ) ] ) )
		checkGetSet( self, s, "ivec3Parm", V3iVectorData( [ V3i( 1 ), V3i( -2, 3, 5 ), V3i( 4 ), V3i( 10,12,45 ) ] ) )
		checkGetSet( self, s, "vec2Parm", V2fVectorData( [ V2f( 1, 2 ), V2f( 5, 0.5 ), V2f( -12, 3 ), V2f( -1, 22 ) ] ) )
		checkGetSet( self, s, "vec3Parm", Color3fVectorData( [ Color3f( 1 ), Color3f( 2,3,4 ), Color3f( 3 ), Color3f( 4 ) ] ), V3fVectorData( [ V3f( 1 ), V3f( 2,3,4 ), V3f( 3 ), V3f( 4 ) ] ) )
		checkGetSet( self, s, "vec4Parm", Color4fVectorData( [ Color4f( 1 ), Color4f( 1.0,0.5,0.25, 1.0 ), Color4f( 4 ), Color4f( 3 ) ] ) )
		checkGetSet( self, s, "mat3Parm", M33fVectorData( [ M33f.createTranslated( V2f( 1, 2 ) ), M33f.createTranslated( V2f( -1, 3 ) ), M33f.createTranslated( V2f( 2, 6 ) ), M33f.createTranslated( V2f( 4, 3 ) ) ] ) )
		checkGetSet( self, s, "mat4Parm", M44fVectorData( [ M44f.createTranslated( V3f( 1, 2, 3 ) ), M44f.createTranslated( V3f( -3, 8, -3 ) ), M44f.createTranslated( V3f( 2, 5, 9 ) ), M44f.createTranslated( V3f( 7, 6, 4 ) ) ] ) )
		
		
		# check that setting invalid values throws an Exception
		self.assertRaises( Exception, s.setUniformParameter, "iDontExist", FloatVectorData( [2] ) )
		self.assertRaises( Exception, s.setUniformParameter, "intParm", FloatVectorData( [2] ) )
		self.assertRaises( Exception, s.setUniformParameter, "floatParm", IntVectorData( [2] ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec2Parm", V3iVectorData( [V3i( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec3Parm", V3fVectorData( [V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec2Parm", V3fVectorData( [V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec3Parm", V2fVectorData( [V2f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec4Parm", V3fVectorData( [V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat4Parm", V3fVectorData( [V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat3Parm", V3fVectorData( [V3f( 2 )] ) )


	def testVertexParameters( self ) :

		vertexSource = """
		attribute float floatParm;
		attribute vec2 vec2Parm;
		attribute vec3 vec3Parm;
		attribute vec4 vec4Parm;

		varying vec4 myColor;

		void main()
		{
			myColor = vec4( floatParm + vec2Parm.x, vec3Parm.y, vec4Parm.r, 1 );
			gl_Position = ftransform();
		}
		"""

		fragmentSource = """

		varying vec4 myColor;

		void main()
		{
			gl_FragColor = myColor;
		}
		"""

		s = Shader( vertexSource, fragmentSource )
		self.assert_( s==s )

		expectedParameterNamesAndValues = {
			"floatParm" : FloatVectorData(),
			"vec2Parm" : V2fVectorData(),
			"vec3Parm" : V3fVectorData(),
			"vec4Parm" : Color4fVectorData(),
		}

		parameterNames = s.vertexParameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndValues ) )
		for n in expectedParameterNamesAndValues.keys() :
			self.assert_( n in parameterNames )
			self.assert_( s.hasVertexParameter( n ) )
			self.assert_( not s.hasVertexParameter( n + "VeryUnlikelySuffix" ) )
			self.assert_( s.vertexValueValid( n, expectedParameterNamesAndValues[n] ) )

		s.setVertexParameter( "floatParm", FloatVectorData( [ 1 ] ), False )
		s.setVertexParameter( "vec2Parm", V2fVectorData( [ V2f( 1, 2 ) ] ), False )
		s.setVertexParameter( "vec3Parm", Color3fVectorData( [ Color3f( 1 ) ] ), True )
		s.setVertexParameter( "vec4Parm", Color4fVectorData( [ Color4f( 1 ) ] ) )

		s.unsetVertexParameters()

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

		r.concatTransform( M44f.createTranslated( V3f( 0, 0, 5 ) ) )

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
		
	def testVertexValueValid( self ) :
	
		## Shader.vertexValueValid should not throw exceptions, it should just return True or False
		
		vertexSource = """
		attribute float floatParm;
		attribute vec2 vec2Parm;
		attribute vec3 vec3Parm;
		attribute vec4 vec4Parm;

		varying vec4 myColor;

		void main()
		{
			myColor = vec4( floatParm + vec2Parm.x, vec3Parm.y, vec4Parm.r, 1 );
			gl_Position = ftransform();
		}
		"""

		fragmentSource = """

		varying vec4 myColor;

		void main()
		{
			gl_FragColor = myColor;
		}
		"""

		s = Shader( vertexSource, fragmentSource )
		
		self.failUnless( s.vertexValueValid( "floatParm", FloatVectorData() ) )
		self.failIf( s.vertexValueValid( "floatParm", V3fVectorData() ) )
		
		self.failUnless( s.vertexValueValid( "vec3Parm", V3fVectorData() ) )
		self.failIf( s.vertexValueValid( "vec3Parm", FloatVectorData() ) )
		

if __name__ == "__main__":
    unittest.main()
