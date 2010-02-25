##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
import sys

import IECore
import IECoreRI

class SXRendererTest( unittest.TestCase ) :

	def __loadImage( self, fileName ) :
	
		i = IECore.Reader.create( fileName ).read()
		
		r = i["R"].data
		g = i["G"].data
		b = i["B"].data
		
		result = IECore.V3fVectorData()
		v = IECore.V3f
		for i in range( 0, len( r ) ) :
			result.append( v( r[i], g[i], b[i] ) )

		return result
		
	def __saveImage( self, data, dataWindow, fileName ) :
	
		image = IECore.ImagePrimitive( dataWindow, dataWindow )
		if isinstance( data, IECore.FloatVectorData ) :
			
			image["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, data )
			image["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, data )
			image["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, data )
		
		else :
		
			r = IECore.FloatVectorData()
			g = IECore.FloatVectorData()
			b = IECore.FloatVectorData()
			
			for c in data :
			
				r.append( c[0] )
				g.append( c[1] )
				b.append( c[2] )
			
			image["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, r )
			image["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, g )
			image["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, b )
			
		IECore.Writer.create( image, fileName ).write()
	
	def __rectanglePoints( self, box ) :
	
		p = IECore.V3fVectorData()
		n = IECore.V3fVectorData()
		i = IECore.V3fVectorData()
		s = IECore.FloatVectorData()
		t = IECore.FloatVectorData()
		for y in range( box.min.y, box.max.y + 1 ) :
			for x in range( box.min.x, box.max.x + 1 ) :
				p.append( IECore.V3f( x, y, 0 ) )
				n.append( IECore.V3f( 0, 0, 1 ) )
				i.append( IECore.V3f( 0, 0, -1 ) )
				s.append( float( x ) / box.size().x )
				t.append( float( y ) / box.size().y )
				
		return IECore.CompoundData( {
			"P" : p,
			"N" : n,
			"Ng" : n,
			"I" : i,
			"s" : s,
			"t" : t,
		} )
		
	def test( self ) :

		r = IECoreRI.SXRenderer()

		points = IECore.CompoundData( {
		
			"N" : self.__loadImage( "test/IECoreRI/data/sxInput/cowN.exr" ),
			"Ng" : self.__loadImage( "test/IECoreRI/data/sxInput/cowN.exr" ),
			"P" : self.__loadImage( "test/IECoreRI/data/sxInput/cowP.exr" ),
			"I" : self.__loadImage( "test/IECoreRI/data/sxInput/cowI.exr" ),
	
		} )
				
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxTest.sdl test/IECoreRI/shaders/sxTest.sl" ), 0 )
		
		r.shader( "surface", "test/IECoreRI/shaders/sxTest.sdl", { "noiseFrequency" : 1.0, "tint" : IECore.Color3f( 1 ) } )
		
		s = r.shade( points )
		
		self.assertEqual( len( s ), 4 )
		self.failUnless( "outputFloat" in s )
		self.failUnless( "outputColor" in s )
		self.failUnless( "Ci" in s )
		self.failUnless( "Oi" in s )
		
		self.assertEqual( s["outputFloat"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowFloat.cob" ).read() )
		self.assertEqual( s["outputColor"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowColor.cob" ).read() )
		self.assertEqual( s["Ci"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowCI.cob" ).read() )
		self.assertEqual( s["Oi"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowOI.cob" ).read() )
	
	def testSplineParameter( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/splineTest.sdl test/IECoreRI/shaders/splineTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()
		
		
		r.shader( "surface", "test/IECoreRI/shaders/splineTest.sdl", {
			"spl" : IECore.SplinefColor3fData(
				IECore.SplinefColor3f(
					IECore.CubicBasisf.catmullRom(),
					(
						( 0, IECore.Color3f( 1, 0, 0 ) ),
						( 0, IECore.Color3f( 1, 0, 0 ) ),
						( 1, IECore.Color3f( 0, 0, 1 ) ),
						( 1, IECore.Color3f( 0, 0, 1 ) ),
					)
				)
			)
		} )
				
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 10 ) )

		s = r.shade( self.__rectanglePoints( b ) )
		
		self.assertEqual( s["Ci"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/spline.cob" ).read() )
			
	# the sx library seems to crash if we don't provide every single predefined variable
	# it wants. but we want to allow users to not specify everything if they don't want to,
	# so we try to provide defaults to prevent the crash.
	def testMissingPredefinedVariables( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/splineTest.sdl test/IECoreRI/shaders/splineTest.sl" ), 0 )
		r = IECoreRI.SXRenderer()
		
		r.shader( "surface", "test/IECoreRI/shaders/splineTest.sdl", {} )
				
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 100 ) )
		points = self.__rectanglePoints( b )
		del points["t"] # remove information the shader requires

		s = r.shade( points )
	
	def testParameterTypes( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxParameterTest.sdl test/IECoreRI/shaders/sxParameterTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()
				
		r.shader( "surface", "test/IECoreRI/shaders/sxParameterTest.sdl", {
			"mustBeOne" : 1.0,
			"mustBeRed" : IECore.Color3f( 1, 0, 0 ),
			"mustBeTwo" : IECore.V3f( 2 ),
			"mustBeThree" : IECore.V3f( 3 ),
			"mustBeFour" : IECore.V3f( 4 ),
			"mustBeHelloWorld" : "helloWorld"
		} )
				
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 1 ) )

		s = r.shade( self.__rectanglePoints( b ) )
		
		self.assertEqual( s["Ci"][0], IECore.Color3f( 0, 1, 0 ) )
	
	def testStack( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxStackTest.sdl test/IECoreRI/shaders/sxStackTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()

		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 100 ) )
		points = self.__rectanglePoints( b )
		
		self.assertEqual( r.getAttribute( "color" ), IECore.Color3fData( IECore.Color3f( 1 ) ) )
		self.assertEqual( r.getAttribute( "opacity" ), IECore.Color3fData( IECore.Color3f( 1 ) ) )
		
		with IECore.WorldBlock( r ) :
		
			r.setAttribute( "color", IECore.Color3f( 1, 0, 0 ) )
			self.assertEqual( r.getAttribute( "color" ), IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) )
					
			r.shader( "surface", "test/IECoreRI/shaders/sxStackTest.sdl", { "blue" : 1.0 } )

			with IECore.AttributeBlock( r ) :

				r.setAttribute( "color", IECore.Color3f( 0, 1, 0 ) )
				self.assertEqual( r.getAttribute( "color" ), IECore.Color3fData( IECore.Color3f( 0, 1, 0 ) ) )

				r.shader( "surface", "test/IECoreRI/shaders/sxStackTest.sdl", { "blue" : 0.5 } )

				s = r.shade( points )

				for c in s["Ci"] :
					self.assertEqual( c, IECore.Color3f( 0, 0, 0.5 ) )
					
			self.assertEqual( r.getAttribute( "color" ), IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) )

			s = r.shade( points )
			
			for c in s["Ci"] :
				self.assertEqual( c, IECore.Color3f( 0, 0, 1 ) )
				
		self.assertEqual( r.getAttribute( "color" ), IECore.Color3fData( IECore.Color3f( 1 ) ) )
		self.assertEqual( r.getAttribute( "opacity" ), IECore.Color3fData( IECore.Color3f( 1 ) ) )
			
	def tearDown( self ) :
		
		files = [
			"test/IECoreRI/shaders/sxTest.sdl",
			"test/IECoreRI/shaders/splineTest.sdl",
			"test/IECoreRI/shaders/sxParameterTest.sdl",
			"test/IECoreRI/shaders/sxStackTest.sdl",
		]
		
		for f in files :
			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()
