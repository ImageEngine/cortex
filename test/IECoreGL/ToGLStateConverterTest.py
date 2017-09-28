##########################################################################
#
#  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreGL

IECoreGL.init( False )

class ToGLStateConverterTest( unittest.TestCase ) :

	def test( self ) :

		attributes = [
			( "doubleSided", IECore.BoolData( True ), IECoreGL.DoubleSidedStateComponent( True ) ),
			( "gl:primitive:wireframe", IECore.BoolData( True ), IECoreGL.Primitive.DrawWireframe( True ) ),
			( "gl:primitive:wireframeWidth", IECore.FloatData( 2.5 ), IECoreGL.Primitive.WireframeWidth( 2.5 ) ),
			( "gl:primitive:wireframeColor", IECore.Color4fData( IECore.Color4f( 0.1, 0.25, 0.5, 1 ) ), IECoreGL.WireframeColorStateComponent( IECore.Color4f( 0.1, 0.25, 0.5, 1 ) ) ),
			( "gl:primitive:bound", IECore.BoolData( True ), IECoreGL.Primitive.DrawBound( True ) ),
			( "gl:primitive:boundColor", IECore.Color4fData( IECore.Color4f( 0.1, 0.25, 0.5, 1 ) ), IECoreGL.BoundColorStateComponent( IECore.Color4f( 0.1, 0.25, 0.5, 1 ) ) ),
			( "gl:primitive:solid", IECore.BoolData( True ), IECoreGL.Primitive.DrawSolid( True ) ),
			( "gl:primitive:points", IECore.BoolData( True ), IECoreGL.Primitive.DrawPoints( True ) ),
			( "gl:primitive:pointWidth", IECore.FloatData( 2.5 ), IECoreGL.Primitive.PointWidth( 2.5 ) ),
			( "gl:primitive:pointColor", IECore.Color4fData( IECore.Color4f( 0.1, 0.25, 0.5, 1 ) ), IECoreGL.PointColorStateComponent( IECore.Color4f( 0.1, 0.25, 0.5, 1 ) ) ),
			( "gl:pointsPrimitive:useGLPoints", IECore.StringData( "forGLPoints" ), IECoreGL.PointsPrimitive.UseGLPoints( IECoreGL.GLPointsUsage.ForPointsOnly ) ),
			( "gl:pointsPrimitive:glPointWidth", IECore.FloatData( 1.5 ), IECoreGL.PointsPrimitive.GLPointWidth( 1.5 ) ),
			( "gl:curvesPrimitive:useGLLines", IECore.BoolData( True ), IECoreGL.CurvesPrimitive.UseGLLines( True ) ),
			( "gl:curvesPrimitive:glLineWidth", IECore.FloatData( 1.5 ), IECoreGL.CurvesPrimitive.GLLineWidth( 1.5 ) ),
			( "gl:curvesPrimitive:ignoreBasis", IECore.BoolData( True ), IECoreGL.CurvesPrimitive.IgnoreBasis( True ) ),
			( "gl:smoothing:points", IECore.BoolData( True ), IECoreGL.PointSmoothingStateComponent( True ) ),
			( "gl:smoothing:lines", IECore.BoolData( True ), IECoreGL.LineSmoothingStateComponent( True ) ),
			( "gl:smoothing:polygons", IECore.BoolData( True ), IECoreGL.PolygonSmoothingStateComponent( True ) ),
		]

		for name, value, component in attributes :
			a = IECore.CompoundObject()
			a[name] = value
			s = IECoreGL.ToGLStateConverter( a ).convert()
			c = s.get( component.typeId() )
			self.assertEqual( c.typeId(), component.typeId() )
			self.assertEqual( c.value, component.value )

	def testShaderAttribute( self ) :

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

		shader = IECore.Shader( "test", "surface", { "gl:vertexSource" : vertexSource, "gl:fragmentSource" : fragmentSource } )
		attributes = IECore.CompoundObject( { "gl:surface" : shader } )
		state = IECoreGL.ToGLStateConverter( attributes ).convert()

		shaderState = state.get( IECoreGL.ShaderStateComponent )
		self.assertTrue( isinstance( shaderState, IECoreGL.ShaderStateComponent ) )
		glShader = shaderState.shaderSetup().shader()
		self.assertTrue( glShader.vertexSource(), vertexSource )
		self.assertTrue( glShader.fragmentSource(), fragmentSource )

if __name__ == "__main__":
    unittest.main()
