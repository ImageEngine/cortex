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
		self.assert_( s==s )

		expectedParameterNamesAndTypes = {
			"boolParm" : IECore.TypeId.BoolData,
			"intParm" : IECore.TypeId.IntData,
			"floatParm" : IECore.TypeId.FloatData,
			"bvec2Parm" : IECore.TypeId.V2iData,
			"bvec3Parm" : IECore.TypeId.V3iData,
			"ivec2Parm" : IECore.TypeId.V2iData,
			"ivec3Parm" : IECore.TypeId.V3iData,
			"vec2Parm" : IECore.TypeId.V2fData,
			"vec3Parm" : IECore.TypeId.V3fData,
			"vec4Parm" : IECore.TypeId.Color4fData,
			"s.f" : IECore.TypeId.FloatData,
			"s.i" : IECore.TypeId.IntData,
			"s2D" : IECoreGL.Texture.staticTypeId(),
			"mat3Parm" : IECore.TypeId.M33fData,
			"mat4Parm" : IECore.TypeId.M44fData,
		}

		parameterNames = s.uniformParameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndTypes ) )
		for n in expectedParameterNamesAndTypes.keys() :
			self.assert_( n in parameterNames )
			self.assertTrue( s.uniformParameter( n ) is not None )
			self.assertTrue( s.uniformParameter( n + "VeryUnlikelySuffix" ) is None )
			self.assertEqual( s.uniformParameterType( n ), expectedParameterNamesAndTypes[n] )

		expectedNamesAndValues = {
			"boolParm" : IECore.BoolData( 0 ),
			"intParm" : IECore.IntData( 0 ),
			"floatParm" : IECore.FloatData( 0 ),
			"bvec2Parm" : IECore.V2iData( IECore.V2i( 0 ) ),
			"ivec2Parm" : IECore.V2iData( IECore.V2i( 0 ) ),
			"vec2Parm" : IECore.V2fData( IECore.V2f( 0 ) ),
			"bvec3Parm" : IECore.V3iData( IECore.V3i( 0 ) ),
			"ivec3Parm" : IECore.V3iData( IECore.V3i( 0 ) ),
			"vec3Parm" : IECore.V3fData( IECore.V3f( 0 ) ),
			"vec4Parm" : IECore.Color4fData( IECore.Color4f( 0 ) ),
		}
		# Initial values are not necessarily zero. So we just test for the value type here.
		for name, value in expectedNamesAndValues.items() :
			self.assertEqual( type(s.getUniformParameter( name )), type(value) )

		# must bind a shader before setting parameters
		s.bind()

		s.setUniformParameter( "intParm", IECore.IntData( 1 ) )
		self.assertEqual( s.getUniformParameter( "intParm" ), IntData( 1 ) )

		s.setUniformParameter( "boolParm", IECore.IntData( 2 ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( 1 ) )

		s.setUniformParameter( "boolParm", IECore.BoolData( False ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( False ) )

		s.setUniformParameter( "boolParm", IECore.BoolData( True ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( True ) )

		s.setUniformParameter( "boolParm", IECore.BoolData( 2 ) )
		self.assertEqual( s.getUniformParameter( "boolParm" ), BoolData( 1 ) )

		s.setUniformParameter( "floatParm", IECore.FloatData( 1 ) )
		self.assertEqual( s.getUniformParameter( "floatParm" ), FloatData( 1 ) )

		s.setUniformParameter( "bvec2Parm", IECore.V2iData( IECore.V2i( 0, 1 ) ) )
		self.assertEqual( s.getUniformParameter( "bvec2Parm" ), IECore.V2iData( IECore.V2i( 0, 1 ) ) )

		s.setUniformParameter( "bvec3Parm", IECore.V3iData( IECore.V3i( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "bvec3Parm" ), IECore.V3iData( IECore.V3i( 1 ) ) )

		s.setUniformParameter( "ivec2Parm", IECore.V2iData( IECore.V2i( 1, 2 ) ) )
		self.assertEqual( s.getUniformParameter( "ivec2Parm" ), IECore.V2iData( IECore.V2i( 1, 2 ) ) )

		s.setUniformParameter( "ivec3Parm", IECore.V3iData( IECore.V3i( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "ivec3Parm" ), IECore.V3iData( IECore.V3i( 1 ) ) )

		s.setUniformParameter( "vec2Parm", IECore.V2fData( IECore.V2f( 1, 2 ) ) )
		self.assertEqual( s.getUniformParameter( "vec2Parm" ), IECore.V2fData( V2f( 1, 2 ) ) )

		s.setUniformParameter( "vec3Parm", IECore.Color3fData( IECore.Color3f( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "vec3Parm" ), IECore.V3fData( IECore.V3f( 1 ) ) )

		s.setUniformParameter( "vec4Parm", IECore.Color4fData( IECore.Color4f( 1 ) ) )
		self.assertEqual( s.getUniformParameter( "vec4Parm" ), IECore.Color4fData( IECore.Color4f( 1 ) ) )

		s.setUniformParameter( "s.i", IECore.IntData( 1 ) )
		self.assertEqual( s.getUniformParameter( "s.i" ), IECore.IntData( 1 ) )

		s.setUniformParameter( "s.f", IECore.FloatData( 1 ) )
		self.assertEqual( s.getUniformParameter( "s.f" ), IECore.FloatData( 1 ) )

		s.setUniformParameter( "mat3Parm", IECore.M33fData( IECore.M33f.createTranslated( IECore.V2f( 1, 2 ) ) ) )
		self.assertEqual( s.getUniformParameter( "mat3Parm" ), IECore.M33fData( IECore.M33f.createTranslated( IECore.V2f( 1, 2 ) ) ) )

		s.setUniformParameter( "mat4Parm", IECore.M44fData( IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) ) ) )
		self.assertEqual( s.getUniformParameter( "mat4Parm" ), IECore.M44fData( IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) ) ) )

		# check that setting invalid values throws an Exception
		self.assertRaises( Exception, s.setUniformParameter, "iDontExist", IECore.FloatData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "boolParm", IECore.FloatData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "intParm", IECore.FloatData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "floatParm", IECore.IntData( 2 ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec2Parm", IECore.V3iData( IECore.V3i( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec3Parm", IECore.V3fData( IECore.V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec2Parm", IECore.V3fData( IECore.V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec3Parm", IECore.V2fData( IECore.V2f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec4Parm", IECore.V3fData( IECore.V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat4Parm", IECore.V3fData( IECore.V3f( 2 ) ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat3Parm", IECore.V3fData( IECore.V3f( 2 ) ) )

		# check that both booldata is valid for bools
		self.assert_( s.uniformValueValid( "boolParm", IECore.BoolData( True ) ) )

		# check uniform parameters defined from vector items
		self.assert_( s.uniformVectorValueValid( "vec2Parm", IECore.V2fVectorData( [ IECore.V2f() ] ) ) )
		self.assert_( not s.uniformVectorValueValid( "vec2Parm", IECore.FloatVectorData( [ 1 ] ) ) )
		self.assert_( s.uniformVectorValueValid( "floatParm", IECore.FloatVectorData( [ 1 ] ) ) )
		self.assert_( not s.uniformVectorValueValid( "floatParm", IECore.IntVectorData( [ 1 ] ) ) )
		self.assert_( s.uniformVectorValueValid( "vec3Parm", IECore.V3fVectorData( [ IECore.V3f() ] ) ) )
		self.assert_( s.uniformVectorValueValid( "vec3Parm", IECore.Color3fVectorData( [ IECore.Color3f() ] ) ) )
		self.assert_( not s.uniformVectorValueValid( "vec3Parm", IECore.V2fVectorData( [ IECore.V2f() ] ) ) )

		v = IECore.V2fVectorData( [ IECore.V2f(9,9), IECore.V2f(1,2), IECore.V2f(9,9) ] )
		s.setUniformParameterFromVector( "vec2Parm", v, 1 )
		self.assertEqual( s.getUniformParameter( "vec2Parm" ), IECore.V2fData( IECore.V2f( 1, 2 ) ) )
	
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

		s = IECoreGL.Shader( vertexSource, fragmentSource )
		self.assert_( s==s )

		expectedParameterNamesAndTypes = {
			"intParm" : IECore.TypeId.IntVectorData,
			"floatParm" : IECore.TypeId.FloatVectorData,
			"ivec2Parm" : IECore.TypeId.V2iVectorData,
			"ivec3Parm" : IECore.TypeId.V3iVectorData,
			"vec2Parm" : IECore.TypeId.V2fVectorData,
			"vec3Parm" : IECore.TypeId.V3fVectorData,
			"vec4Parm" : IECore.TypeId.Color4fVectorData,
			"mat3Parm" : IECore.TypeId.M33fVectorData,
			"mat4Parm" : IECore.TypeId.M44fVectorData,
		}

		parameterNames = s.uniformParameterNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndTypes ) )
		
		for n in expectedParameterNamesAndTypes.keys() :
			self.assertTrue( n in parameterNames )
			self.assertTrue( s.uniformParameter( n ) is not None )
			self.assertTrue( s.uniformParameter( n + "VeryUnlikelySuffix" ) is None )
			
			self.assertEqual( s.uniformParameterType( n ), expectedParameterNamesAndTypes[n] )

		expectedNamesAndValues = {
			"intParm" : IECore.IntVectorData( ),
			"floatParm" : IECore.FloatVectorData( ),
			"ivec2Parm" : IECore.V2iVectorData( ),
			"vec2Parm" : IECore.V2fVectorData( ),
			"ivec3Parm" : IECore.V3iVectorData( ),
			"vec3Parm" : IECore.V3fVectorData( ),
			"vec4Parm" : IECore.Color4fVectorData( ),
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
				
		
		checkGetSet( self, s, "intParm", IECore.IntVectorData( range(0,4) ) )
		checkGetSet( self, s, "floatParm", IECore.FloatVectorData( [ 0.0, 1.0, 2.0, 3.0 ]) )
		checkGetSet( self, s, "ivec2Parm", IECore.V2iVectorData( [ IECore.V2i( 1, 2 ), IECore.V2i( 5, 4 ), IECore.V2i( 8, 5 ), IECore.V2i( 2, 3 ) ] ) )
		checkGetSet( self, s, "ivec3Parm", IECore.V3iVectorData( [ IECore.V3i( 1 ), IECore.V3i( -2, 3, 5 ), IECore.V3i( 4 ), IECore.V3i( 10,12,45 ) ] ) )
		checkGetSet( self, s, "vec2Parm", IECore.V2fVectorData( [ IECore.V2f( 1, 2 ), IECore.V2f( 5, 0.5 ), IECore.V2f( -12, 3 ), IECore.V2f( -1, 22 ) ] ) )
		checkGetSet( self, s, "vec3Parm", IECore.Color3fVectorData( [ IECore.Color3f( 1 ), IECore.Color3f( 2,3,4 ), IECore.Color3f( 3 ), IECore.Color3f( 4 ) ] ), IECore.V3fVectorData( [ IECore.V3f( 1 ), IECore.V3f( 2,3,4 ), IECore.V3f( 3 ), IECore.V3f( 4 ) ] ) )
		checkGetSet( self, s, "vec4Parm", IECore.Color4fVectorData( [ IECore.Color4f( 1 ), IECore.Color4f( 1.0,0.5,0.25, 1.0 ), IECore.Color4f( 4 ), IECore.Color4f( 3 ) ] ) )
		checkGetSet( self, s, "mat3Parm", IECore.M33fVectorData( [ IECore.M33f.createTranslated( IECore.V2f( 1, 2 ) ), IECore.M33f.createTranslated( IECore.V2f( -1, 3 ) ), IECore.M33f.createTranslated( IECore.V2f( 2, 6 ) ), IECore.M33f.createTranslated( IECore.V2f( 4, 3 ) ) ] ) )
		checkGetSet( self, s, "mat4Parm", IECore.M44fVectorData( [ IECore.M44f.createTranslated( IECore.V3f( 1, 2, 3 ) ), IECore.M44f.createTranslated( IECore.V3f( -3, 8, -3 ) ), IECore.M44f.createTranslated( IECore.V3f( 2, 5, 9 ) ), IECore.M44f.createTranslated( IECore.V3f( 7, 6, 4 ) ) ] ) )
		
		
		# check that setting invalid values throws an Exception
		self.assertRaises( Exception, s.setUniformParameter, "iDontExist", IECore.FloatVectorData( [2] ) )
		self.assertRaises( Exception, s.setUniformParameter, "intParm", IECore.FloatVectorData( [2] ) )
		self.assertRaises( Exception, s.setUniformParameter, "floatParm", IECore.IntVectorData( [2] ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec2Parm", IECore.V3iVectorData( [IECore.V3i( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "ivec3Parm", IECore.V3fVectorData( [IECore.V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec2Parm", IECore.V3fVectorData( [IECore.V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec3Parm", IECore.V2fVectorData( [IECore.V2f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "vec4Parm", IECore.V3fVectorData( [IECore.V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat4Parm", IECore.V3fVectorData( [IECore.V3f( 2 )] ) )
		self.assertRaises( Exception, s.setUniformParameter, "mat3Parm", IECore.V3fVectorData( [IECore.V3f( 2 )] ) )


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
		self.assert_( s==s )

		expectedParameterNamesAndValues = {
			"floatParm" : IECore.FloatVectorData(),
			"vec2Parm" : IECore.V2fVectorData(),
			"vec3Parm" : IECore.V3fVectorData(),
			"vec4Parm" : IECore.Color4fVectorData(),
		}

		parameterNames = s.vertexAttributeNames()
		self.assertEqual( len( parameterNames ), len( expectedParameterNamesAndValues ) )
		for n in expectedParameterNamesAndValues.keys() :
			self.assert_( n in parameterNames )
			self.assertTrue( s.vertexAttribute( n ) is not None )
			self.assertTrue( s.vertexAttribute( n + "VeryUnlikelySuffix" ) is None )

		s.setVertexParameter( "floatParm", IECore.FloatVectorData( [ 1 ] ), False )
		s.setVertexParameter( "vec2Parm", IECore.V2fVectorData( [ IECore.V2f( 1, 2 ) ] ), False )
		s.setVertexParameter( "vec3Parm", IECore.Color3fVectorData( [ IECore.Color3f( 1 ) ] ), True )
		s.setVertexParameter( "vec4Parm", IECore.Color4fVectorData( [ IECore.Color4f( 1 ) ] ) )

		s.unsetVertexParameters()
		
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

		s = IECoreGL.Shader( vertexSource, fragmentSource )
		
		self.failUnless( s.vertexValueValid( "floatParm", IECore.FloatVectorData() ) )
		self.failIf( s.vertexValueValid( "floatParm", IECore.V3fVectorData() ) )
		
		self.failUnless( s.vertexValueValid( "vec3Parm", IECore.V3fVectorData() ) )
		self.failIf( s.vertexValueValid( "vec3Parm", IECore.FloatVectorData() ) )
		
	def testGeometryShader( self ) :
	
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
		
		self.failUnless( "geometryShaderParameter" in s.uniformParameterNames() )

	def testEmptyGeometryShader( self ) :
	
		s = IECoreGL.Shader( IECoreGL.Shader.defaultVertexSource(), "", IECoreGL.Shader.defaultFragmentSource() )

if __name__ == "__main__":
    unittest.main()
