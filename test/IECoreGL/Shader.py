##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreGL

IECoreGL.init( False )

class TestShader( unittest.TestCase ) :

	def testConstructor( self ) :

		self.assertRaises( RuntimeError, IECoreGL.Shader, "i don't think i'm valid", "me neither" )

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

		s1 = IECoreGL.Shader( vertexSource, fragmentSource )

		self.assertNotEqual( s1.program(), 0 )
		self.assertEqual( s1.vertexSource(), vertexSource )
		self.assertEqual( s1.geometrySource(), "" )
		self.assertEqual( s1.fragmentSource(), fragmentSource )

		s2 = IECoreGL.Shader( "", "", fragmentSource )

		self.assertNotEqual( s2.program(), 0 )
		self.assertNotEqual( s1.program(), s2.program() )
		self.assertEqual( s2.vertexSource(), "" )
		self.assertEqual( s2.geometrySource(), "" )
		self.assertEqual( s2.fragmentSource(), fragmentSource )

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

		s = IECoreGL.Shader( vertexSource, fragmentSource )
		self.assertTrue( s==s )

		expectedParameterNames = [
			"boolParm",
			"intParm",
			"floatParm",
			"bvec2Parm",
			"bvec3Parm",
			"ivec2Parm",
			"ivec3Parm",
			"vec2Parm",
			"vec3Parm",
			"vec4Parm",
			"s.f",
			"s.i",
			"s2D",
			"mat3Parm",
			"mat4Parm",
		]

		parameterNames = s.uniformParameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNames ) )
		for n in expectedParameterNames :
			self.assertTrue( n in parameterNames )
			self.assertTrue( s.uniformParameter( n ) is not None )
			self.assertTrue( s.uniformParameter( n + "VeryUnlikelySuffix" ) is None )
			self.assertEqual( s.uniformParameter( n ).size, 1 )

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
		uniform int intParm[2];
		uniform float floatParm[4];

		// uniform bvec2 bvec2Parm[4];
		// uniform bvec3 bvec3Parm[4];
		// uniform ivec4 bvec4Parm; // we have no suitable datatype for specifying this in IECore

		uniform ivec2 ivec2Parm[5];
		uniform ivec3 ivec3Parm[6];
		// uniform ivec4 ivec4Parm; // we have no suitable datatype for specifying this in IECore

		uniform vec2 vec2Parm[2];
		uniform vec3 vec3Parm[3];
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

		s = IECoreGL.Shader( vertexSource, fragmentSource )
		self.assertTrue( s==s )

		expectedParameterNamesAndSizes = {
			"intParm" : 2,
			"floatParm" : 4,
			"ivec2Parm" : 5,
			"ivec3Parm" : 6,
			"vec2Parm" : 2,
			"vec3Parm" : 3,
			"vec4Parm" : 4,
			"mat3Parm" : 4,
			"mat4Parm" : 4,
		}

		parameterNames = s.uniformParameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndSizes ) )

		for n in expectedParameterNamesAndSizes.keys() :
			self.assertTrue( n in parameterNames )
			self.assertTrue( s.uniformParameter( n ) is not None )
			self.assertTrue( s.uniformParameter( n + "VeryUnlikelySuffix" ) is None )

			self.assertEqual( s.uniformParameter( n ).size, expectedParameterNamesAndSizes[n] )

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

		s = IECoreGL.Shader( vertexSource, fragmentSource )
		self.assertTrue( s==s )

		expectedParameterNames = [
			"floatParm",
			"vec2Parm",
			"vec3Parm",
			"vec4Parm",
		]

		parameterNames = s.vertexAttributeNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNames ) )
		for n in expectedParameterNames :
			self.assertTrue( n in parameterNames )
			self.assertTrue( s.vertexAttribute( n ) is not None )
			self.assertTrue( s.vertexAttribute( n + "VeryUnlikelySuffix" ) is None )
			self.assertEqual( s.vertexAttribute( n ).size, 1 )

	def testGeometryShader( self ) :

		if IECoreGL.glslVersion() < 150 :
			# can't test geometry shaders if they don't exist
			return

		geometrySource = """
		#version 150

		layout( triangles ) in;
		layout( triangle_strip, max_vertices=3 ) out;

		uniform float geometryShaderParameter = 0;

		void main()
		{
			for( int i = 0; i < gl_in.length(); i++)
			{
				gl_Position = gl_in[i].gl_Position + vec4( geometryShaderParameter, 0, 0, 1 );
				EmitVertex();
			}
		}
		"""

		s = IECoreGL.Shader( IECoreGL.Shader.defaultVertexSource(), geometrySource, IECoreGL.Shader.defaultFragmentSource() )

		self.assertTrue( "geometryShaderParameter" in s.uniformParameterNames() )

	def testEmptyGeometryShader( self ) :

		s = IECoreGL.Shader( IECoreGL.Shader.defaultVertexSource(), "", IECoreGL.Shader.defaultFragmentSource() )

	def testStandardParameters( self ) :

		s = IECoreGL.Shader.constant()
		self.assertEqual( s.csParameter(), s.uniformParameter( "Cs" ) )

if __name__ == "__main__":
    unittest.main()
