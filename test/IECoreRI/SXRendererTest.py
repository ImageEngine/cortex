##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import unittest
import os
import sys
import threading

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
		dPdu = IECore.V3fVectorData()
		dPdv = IECore.V3fVectorData()
		s = IECore.FloatVectorData()
		t = IECore.FloatVectorData()
		for y in range( box.min.y, box.max.y + 1 ) :
			for x in range( box.min.x, box.max.x + 1 ) :
				p.append( IECore.V3f( x, y, 0 ) )
				n.append( IECore.V3f( 0, 0, 1 ) )
				i.append( IECore.V3f( 0, 0, -1 ) )
				dPdu.append( IECore.V3f( 2, 0, 0 ) )
				dPdv.append( IECore.V3f( 0, 2, 0 ) )
				s.append( float( x ) / box.size().x )
				t.append( float( y ) / box.size().y )
				
		return IECore.CompoundData( {
			"P" : p,
			"N" : n,
			"Ng" : n,
			"I" : i,
			"dPdu" : dPdu,
			"dPdv" : dPdv,
			"s" : s,
			"t" : t,
		} )
	
	def __assertVectorDataAlmostEqual( self, data1, data2 ) :
	
		self.assertEqual( len( data1 ), len( data2 ) )
		self.assertEqual( data1.typeName(), data2.typeName() )
		
		if isinstance( data1, IECore.Color3fVectorData ) :
			for i in range( 0, len( data1 ) ) :
				self.failUnless( data1[i].equalWithAbsError( data2[i], 0.000001 ) )
		else : 
			for i in range( 0, len( data1 ) ) :
				self.assertAlmostEqual( data1[i], data2[i], 6 )
	
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
		
		self.assertEqual( len( s ), 6 )
		self.failUnless( "outputFloat" in s )
		self.failUnless( "outputColor" in s )
		self.failUnless( "Ci" in s )
		self.failUnless( "Oi" in s )
		self.failUnless( "P" in s )
		self.failUnless( "N" in s )
		
		self.__assertVectorDataAlmostEqual( s["outputFloat"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowFloat.cob" ).read() )
		self.__assertVectorDataAlmostEqual( s["outputColor"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowColor.cob" ).read() )
		self.__assertVectorDataAlmostEqual( s["Ci"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowCI.cob" ).read() )
		self.__assertVectorDataAlmostEqual( s["Oi"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowOI.cob" ).read() )
	
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
		
		self.__assertVectorDataAlmostEqual( s["Ci"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/spline.cob" ).read() )
			
	# make sure that users don't have to provide values for every varying shader parameter if
	# they don't want to. this used to crash.
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
			"mustBeHelloWorld" : "helloWorld",
			"mustBeOneTwoThree" : IECore.V3f( 1, 2, 3 ),
		} )
				
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 1 ) )

		s = r.shade( self.__rectanglePoints( b ) )
		
		self.assertEqual( s["Ci"][0], IECore.Color3f( 0, 1, 0 ) )
		
	def testFloat3PrimitiveVariable( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxParameterTest.sdl test/IECoreRI/shaders/sxParameterTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()
				
		r.shader( "surface", "test/IECoreRI/shaders/sxParameterTest.sdl", {
			"mustBeOne" : 1.0,
			"mustBeRed" : IECore.Color3f( 1, 0, 0 ),
			"mustBeTwo" : IECore.V3f( 2 ),
			"mustBeThree" : IECore.V3f( 3 ),
			"mustBeFour" : IECore.V3f( 4 ),
			"mustBeHelloWorld" : "helloWorld",
		} )
				
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 10 ) )

		points = self.__rectanglePoints( b )
		points["mustBeOneTwoThree"] = IECore.V3fVectorData( [ IECore.V3f( 1, 2, 3 ) ] * len( points["P"] ) )

		s = r.shade( points )
		
		for c in s["Ci"] :
			self.assertEqual( c, IECore.Color3f( 0, 1, 0 ) )
	
	def testIntParameterSupport( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxParameterTest.sdl test/IECoreRI/shaders/sxParameterTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()
				
		r.shader( "surface", "test/IECoreRI/shaders/sxParameterTest.sdl", {
			"mustBeOne" : IECore.IntData( 1 ),
			"mustBeRed" : IECore.Color3f( 1, 0, 0 ),
			"mustBeTwo" : IECore.V3f( 2 ),
			"mustBeThree" : IECore.V3f( 3 ),
			"mustBeFour" : IECore.V3f( 4 ),
			"mustBeHelloWorld" : "helloWorld",
			"mustBeOneTwoThree" : IECore.V3f( 1, 2, 3 ),
		} )
				
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 1 ) )

		s = r.shade( self.__rectanglePoints( b ) )
		
		self.assertEqual( s["Ci"][0], IECore.Color3f( 0, 1, 0 ) )
		
	def testBoolParameterSupport( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxParameterTest.sdl test/IECoreRI/shaders/sxParameterTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()
				
		r.shader( "surface", "test/IECoreRI/shaders/sxParameterTest.sdl", {
			"mustBeOne" : IECore.BoolData( True ),
			"mustBeRed" : IECore.Color3f( 1, 0, 0 ),
			"mustBeTwo" : IECore.V3f( 2 ),
			"mustBeThree" : IECore.V3f( 3 ),
			"mustBeFour" : IECore.V3f( 4 ),
			"mustBeHelloWorld" : "helloWorld",
			"mustBeOneTwoThree" : IECore.V3f( 1, 2, 3 ),
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
	
	def testNoShader( self ) :
	
		r = IECoreRI.SXRenderer()
		with IECore.WorldBlock( r ) :
		
			self.assertRaises( RuntimeError, r.shade, self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 100 ) ) ) )
	
	def testCoshaders( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxCoshaderTest.sdl test/IECoreRI/shaders/sxCoshaderTest.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxCoshaderTestMain.sdl test/IECoreRI/shaders/sxCoshaderTestMain.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 100 ) )
		points = self.__rectanglePoints( b )
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "shaderColor" : IECore.Color3f( 1, 0, 0 ), "__handle" : "cs1" } )
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "sColor" : IECore.Color3f( 0, 1, 0 ), "__handle" : "cs2" } )
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "tColor" : IECore.Color3f( 0, 0, 1 ), "__handle" : "cs3" } )
			r.shader( "surface", "test/IECoreRI/shaders/sxCoshaderTestMain", { "coshaders" : IECore.StringVectorData( [ "cs1", "cs2", "cs3" ] ) } )
		
			s = r.shade( points )
					
		self.assertEqual( s["Ci"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/coshaders.cob" ).read() )

	def testCoshadersWithGetVar( self ) :

		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxCoshaderTest.sdl test/IECoreRI/shaders/sxCoshaderTest.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxCoshaderTestMain.sdl test/IECoreRI/shaders/sxCoshaderTestMain.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 4 ) )
		points = self.__rectanglePoints( b )
		points["forGetVar"] = IECore.Color3fVectorData( [ IECore.Color3f( x[0], x[1], x[2] ) for x in points["P"] ] )
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "primVarName" : "forGetVar", "__handle" : "cs1" } )
			r.shader( "surface", "test/IECoreRI/shaders/sxCoshaderTestMain", { "coshaders" : IECore.StringVectorData( [ "cs1" ] ) } )
			s = r.shade( points )
					
		self.assertEqual( s["Ci"], points["forGetVar"] )
	
	def testGrids( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxGridTest.sdl test/IECoreRI/shaders/sxGridTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
		points = self.__rectanglePoints( b )
		
		with IECore.WorldBlock( r ) :
				
			r.shader( "surface", "test/IECoreRI/shaders/sxGridTest", {} )
		
			# not providing enough points for the grid should raise
			self.assertRaises( RuntimeError, r.shade, points, IECore.V2i( 100, 500 ) )	
			
			s = r.shade( points )
			del s["P"] # test data on disk was created before we supported P as an output
			del s["N"] # test data on disk was created before we supported N as an output
			self.assertEqual( s, IECore.ObjectReader( "test/IECoreRI/data/sxOutput/noGrid.cob" ).read() )
					
			s = r.shade( points, IECore.V2i( 21, 11 ) )
			del s["P"] # test data on disk was created before we supported P as an output
			del s["N"] # test data on disk was created before we supported N as an output
			self.assertEqual( s, IECore.ObjectReader( "test/IECoreRI/data/sxOutput/grid.cob" ).read() )

	def testMultipleGrids( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxGridTest.sdl test/IECoreRI/shaders/sxGridTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 19, 9 ) )
		points = self.__rectanglePoints( b )
		
		with IECore.WorldBlock( r ) :
				
			r.shader( "surface", "test/IECoreRI/shaders/sxGridTest", {} )
		
			# there are 20 x 10 points in the input, so this call should shade four 10 x 5 grids:
			r.shade( points, IECore.V2i( 10, 5 ) )


	def testPlaneShade( self ) :
		
		r = IECoreRI.SXRenderer()
		
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxStTest.sdl test/IECoreRI/shaders/sxStTest.sl" ), 0 )
		r.shader( "surface", "test/IECoreRI/shaders/sxStTest.sdl", {} )
		
		data = r.shadePlane( IECore.V2i( 64, 64 ) )
		del data["P"]
		del data["N"]
		self.assertEqual( data, IECore.Reader.create( "test/IECoreRI/data/sxOutput/shadePlaneCompoundData.cob" ).read() )
		
		image = r.shadePlaneToImage( IECore.V2i( 64, 64 ) )
		expectedImage = IECore.Reader.create( "test/IECoreRI/data/sxOutput/shadePlaneImage.exr" ).read()

		self.assertEqual( IECore.ImageDiffOp()( imageA=image, imageB=expectedImage, maxError=0 ), IECore.BoolData( False ) )
	
	def testVVector( self ) :

		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/splineTest.sdl test/IECoreRI/shaders/splineTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()

		r.shader( "surface", "test/IECoreRI/shaders/splineTest.sdl", {} )
	
		p = self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 10 ) ) )

		# this should work, as you might want to use a V3f primvar called v (velocity) in
		# your shader:
		p["v"] = p["P"]
		r.shade( p )

	def testWrongSize( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/splineTest.sdl test/IECoreRI/shaders/splineTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()
		
		r.shader( "surface", "test/IECoreRI/shaders/splineTest.sdl", {} )
				
		p = self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 10 ) ) )
		del p["t"][-10:]
		
		self.assertRaises( RuntimeError, r.shade, p )
	
	def testDisplacementShader( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxDisplacementTest.sdl test/IECoreRI/shaders/sxDisplacementTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		with IECore.WorldBlock( r ) :
		
			r.shader( "displacement", "test/IECoreRI/shaders/sxDisplacementTest.sdl", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			## need to use a grid topology if we want calculatenormal() to work
			s = r.shade( points, IECore.V2i( 21, 11 ) )
			
			self.assertEqual( len( s ), 2 )
			self.failUnless( "P" in s )
			self.failUnless( "N" in s )
			
			for i in range( 0, len( points["P"] ) ) :
				self.failUnless( s["P"][i].equalWithAbsError( points["P"][i] + points["N"][i], 0.001 ) )
				self.failUnless( s["N"][i].equalWithAbsError( IECore.V3f( 0, 0, 1 ), 0.001 ) )

	def testDisplacementAndSurfaceShaders( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxDisplacementTest.sdl test/IECoreRI/shaders/sxDisplacementTest.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxTest.sdl test/IECoreRI/shaders/sxTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		with IECore.WorldBlock( r ) :
		
			r.shader( "displacement", "test/IECoreRI/shaders/sxDisplacementTest.sdl", {} )
			r.shader( "surface", "test/IECoreRI/shaders/sxTest.sdl", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			## need to use a grid topology if we want calculatenormal() to work
			s = r.shade( points, IECore.V2i( 21, 11 ) )
			
			self.assertEqual( len( s ), 6 )
			self.failUnless( "P" in s )
			self.failUnless( "N" in s )
			self.failUnless( "Ci" in s )
			self.failUnless( "Oi" in s )
			self.failUnless( "outputFloat" in s )
			self.failUnless( "outputColor" in s )
			
			for i in range( 0, len( points["P"] ) ) :
				self.failUnless( s["P"][i].equalWithAbsError( points["P"][i] + points["N"][i], 0.001 ) )
				self.failUnless( s["N"][i].equalWithAbsError( IECore.V3f( 0, 0, 1 ), 0.001 ) )
	
	def testLights( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxLightTest.sdl test/IECoreRI/shaders/sxLightTest.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxIlluminanceTest.sdl test/IECoreRI/shaders/sxIlluminanceTest.sl" ), 0 )
	
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "test/IECoreRI/shaders/sxIlluminanceTest", {} )
			r.light( "test/IECoreRI/shaders/sxLightTest", "light0", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )
									
			for i in range( 0, len( points["P"] ) ) :
				c = s["Ci"][i]
				self.assertEqual( points["P"][i], IECore.V3f( c[0], c[1], c[2] ) )
	
	def testPredefinedPrimitiveVariables( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxPredefinedPrimitiveVariableTest.sdl test/IECoreRI/shaders/sxPredefinedPrimitiveVariableTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "test/IECoreRI/shaders/sxPredefinedPrimitiveVariableTest", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )
									
			for i in range( 0, len( points["P"] ) ) :
				self.assertEqual( s["Ci"][i], IECore.Color3f( 1, 1, 1 ) )

	def testNonPredefinedPrimitiveVariables( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxNonPredefinedPrimitiveVariableTest.sdl test/IECoreRI/shaders/sxNonPredefinedPrimitiveVariableTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "test/IECoreRI/shaders/sxNonPredefinedPrimitiveVariableTest", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			points["colorPrimVar"] = IECore.Color3fVectorData( [ IECore.Color3f( v[0], v[1], v[2] ) for v in points["P"] ] )
			points["floatPrimVar"] = points["s"]
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )
									
			for i in range( 0, len( points["P"] ) ) :
				c = points["colorPrimVar"][i]
				c[0] = points["s"][i]
				self.assertEqual( s["Ci"][i], c )

	def testNonPredefinedPrimitiveVariablesForCoshaders( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxCoshaderTestMain.sdl test/IECoreRI/shaders/sxCoshaderTestMain.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxCoshaderTest.sdl test/IECoreRI/shaders/sxCoshaderTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
					
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			points["colorPrimVar"] = IECore.Color3fVectorData( [ IECore.Color3f( v[0], v[1], v[2] ) for v in points["P"] ] )
			
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "__handle" : "cs1" } )
			r.shader( "surface", "test/IECoreRI/shaders/sxCoshaderTestMain", { "coshaders" : IECore.StringVectorData( [ "cs1" ] ) } )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )

			self.assertEqual( s["Ci"], points["colorPrimVar"] )

	def testUniformPrimitiveVariables( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxUniformPrimitiveVariableTest.sdl test/IECoreRI/shaders/sxUniformPrimitiveVariableTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "test/IECoreRI/shaders/sxUniformPrimitiveVariableTest", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			points["colorPrimVar"] = IECore.Color3fData( IECore.Color3f( 0, 0.5, 1 ) )
			points["floatPrimVar"] = IECore.FloatData( 16.0 )
			points["vectorPrimVar"] = IECore.V3fData( IECore.V3f( 0.25, 0.5, 2 ) )
			points["stringPrimVar"] = IECore.StringData( "hello shader!" )
			points["stringVectorPrimVar"] = IECore.StringVectorData( ["who's", "a", "good", "boy" ] )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )
			
			for i in range( 0, len( points["P"] ) ) :
				self.assertEqual( s["Ci"][i], IECore.Color3f( 0.125, 0.25, 0.75 ) )

	def testUniformPrimitiveVariableShaderParameters( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxUniformPrimitiveVariableShaderParameterTest.sdl test/IECoreRI/shaders/sxUniformPrimitiveVariableShaderParameterTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "test/IECoreRI/shaders/sxUniformPrimitiveVariableShaderParameterTest", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			points["colorPrimVar"] = IECore.Color3fData( IECore.Color3f( 0, 0.5, 1 ) )
			points["floatPrimVar"] = IECore.FloatData( 16.0 )
			points["vectorPrimVar"] = IECore.V3fData( IECore.V3f( 0.25, 0.5, 2 ) )
			points["stringPrimVar"] = IECore.StringData( "hello shader!" )
			points["stringVectorPrimVar"] = IECore.StringVectorData( ["who's", "a", "good", "boy" ] )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )
			
			for i in range( 0, len( points["P"] ) ) :
				self.assertEqual( s["Ci"][i], IECore.Color3f( 0.125, 0.25, 0.5 ) )


	def testThreading( self ) :
	
		# set up a renderer with a shader in it
		
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxTest.sdl test/IECoreRI/shaders/sxTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		r.shader( "surface", "test/IECoreRI/shaders/sxTest.sdl", { "noiseFrequency" : 1.0, "tint" : IECore.Color3f( 1 ) } )

		# and get some points to shade
		
		points = IECore.CompoundData( {
		
			"N" : self.__loadImage( "test/IECoreRI/data/sxInput/cowN.exr" ),
			"Ng" : self.__loadImage( "test/IECoreRI/data/sxInput/cowN.exr" ),
			"P" : self.__loadImage( "test/IECoreRI/data/sxInput/cowP.exr" ),
			"I" : self.__loadImage( "test/IECoreRI/data/sxInput/cowI.exr" ),
	
		} )

		# shade in lots of different threads at the same time
		
		def s( i ) :
		
			results[i] = r.shade( points )
		
		threads = []
		results = []
		for i in range( 0, 300 ) :
			threads.append( threading.Thread( target = IECore.curry( s, i ) ) )
			results.append( None )
		
		for t in threads :
			t.start()
			
		for t in threads :
			t.join()
				
		# and check that it all worked
		
		cowFloat = IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowFloat.cob" ).read()
		cowColor = IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowColor.cob" ).read()	
		cowCI =	IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowCI.cob" ).read()
		cowOI = IECore.ObjectReader( "test/IECoreRI/data/sxOutput/cowOI.cob" ).read()
		
		# check that the first set of results is close enough to the expected results.
		# we allow some small variation as 3delight's noise routines seem to yield
		# veryvery small differences between some versions.
		self.__assertVectorDataAlmostEqual( results[0]["outputFloat"], cowFloat )
		self.__assertVectorDataAlmostEqual( results[0]["outputColor"], cowColor )
		self.__assertVectorDataAlmostEqual( results[0]["Ci"], cowCI )
		self.__assertVectorDataAlmostEqual( results[0]["Oi"], cowOI )
		
		# check that all results are exactly equal to the first set. even if we
		# accept small variations between different 3delight versions we don't accept
		# variation within one version.
		for s in results :
		
			self.assertEqual( len( s ), 6 )
			self.failUnless( "outputFloat" in s )
			self.failUnless( "outputColor" in s )
			self.failUnless( "Ci" in s )
			self.failUnless( "Oi" in s )
			self.failUnless( "P" in s )
			self.failUnless( "N" in s )
					
			self.assertEqual( s["outputFloat"], results[0]["outputFloat"] )
			self.assertEqual( s["outputColor"], results[0]["outputColor"] )
			self.assertEqual( s["Ci"], results[0]["Ci"] )
			self.assertEqual( s["Oi"], results[0]["Oi"] )
	
	def testGetVar( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxGetVarTest.sdl test/IECoreRI/shaders/sxGetVarTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
					
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			points["floatValue1"] = points["s"]
			points["floatValue2"] = points["t"]
			
			r.shader( "surface", "test/IECoreRI/shaders/sxGetVarTest", { } )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )

			for i in range( 0, len( s["Ci"] ) ) :
				self.assertEqual( s["Ci"][i], IECore.Color3f( 0, points["floatValue1"][i], points["floatValue2"][i] ) )

	def testGetShaderInConstruct( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxGetShaderTest.sdl test/IECoreRI/shaders/sxGetShaderTest.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxCoshaderTest.sdl test/IECoreRI/shaders/sxCoshaderTest.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
					
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "__handle" : "cs1", "sColor" : IECore.Color3f( 0, 1, 0 ), } )
			r.shader( "surface", "test/IECoreRI/shaders/sxGetShaderTest", { "coshader" : IECore.StringData( "cs1" ) } )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )

			for i in range( 0, len( points["P"] ) ) :
				self.assertEqual( s["Ci"][i], IECore.Color3f( 0, points["s"][i], 0 ) )

	def testCoshadersStack( self ) :
	
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxCoshaderTest.sdl test/IECoreRI/shaders/sxCoshaderTest.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -o test/IECoreRI/shaders/sxCoshaderTestMain.sdl test/IECoreRI/shaders/sxCoshaderTestMain.sl" ), 0 )
		
		r = IECoreRI.SXRenderer()
		
		b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 100 ) )
		points = self.__rectanglePoints( b )
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "shaderColor" : IECore.Color3f( 1, 0, 0 ), "__handle" : "cs1" } )
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "sColor" : IECore.Color3f( 0, 1, 0 ), "__handle" : "cs2" } )
			r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "tColor" : IECore.Color3f( 0, 0, 1 ), "__handle" : "cs3" } )
			
			with IECore.AttributeBlock( r ) :
			
				# these guys should be popped and therefore not affect the result
				r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "shaderColor" : IECore.Color3f( 1, 1, 1 ), "__handle" : "cs1" } )
				r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "sColor" : IECore.Color3f( 1, 1, 0 ), "__handle" : "cs2" } )
				r.shader( "shader", "test/IECoreRI/shaders/sxCoshaderTest", { "tColor" : IECore.Color3f( 0.5, 0, 0.25 ), "__handle" : "cs3" } )
			
			r.shader( "surface", "test/IECoreRI/shaders/sxCoshaderTestMain", { "coshaders" : IECore.StringVectorData( [ "cs1", "cs2", "cs3" ] ) } )
		
			s = r.shade( points )
					
		self.assertEqual( s["Ci"], IECore.ObjectReader( "test/IECoreRI/data/sxOutput/coshaders.cob" ).read() )

	def testLightsStack( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxLightTest.sdl test/IECoreRI/shaders/sxLightTest.sl" ), 0 )
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxIlluminanceTest.sdl test/IECoreRI/shaders/sxIlluminanceTest.sl" ), 0 )
	
		r = IECoreRI.SXRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.shader( "surface", "test/IECoreRI/shaders/sxIlluminanceTest", {} )
			r.light( "test/IECoreRI/shaders/sxLightTest", "light0", {} )
			
			with IECore.AttributeBlock( r ) :
				# this guy should be popped and therefore not affect the result
				r.light( "test/IECoreRI/shaders/sxLightTest", "light1", {} )
			
			b = IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 20, 10 ) )
			points = self.__rectanglePoints( b )
			
			s = r.shade( points, IECore.V2i( 21, 11 ) )
									
			for i in range( 0, len( points["P"] ) ) :
				c = s["Ci"][i]
				self.assertEqual( points["P"][i], IECore.V3f( c[0], c[1], c[2] ) )

	def testZeroLength( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/splineTest.sdl test/IECoreRI/shaders/splineTest.sl" ), 0 )

		r = IECoreRI.SXRenderer()
		
		r.shader( "surface", "test/IECoreRI/shaders/splineTest.sdl", {} )
				
		p = self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 10 ) ) )
		for k, v in p.items() :
			del v[:]
		
		self.assertRaises( RuntimeError, r.shade, p )
		
	def testThreadedTextureLookups( self ) :
	
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxTextureTest.sdl test/IECoreRI/shaders/sxTextureTest.sl" ), 0 )
		
		points = self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 255 ) ) )		
		
		# by default you should be able to run as many threads as the hardware will support
		# concurrently.
		
		for i in range( 0, 10 ) :
				
			r = IECoreRI.SXRenderer()

			r.shader( "surface", "test/IECoreRI/shaders/sxTextureTest.sdl", {
				"fileName" : os.path.realpath( "./test/IECoreRI/data/textures/uvMap.256x256.tdl" ),
			} )

			# note the -1 when determining the number of threads. 3delight behaviour changed around
			# 10.0.35, such that render:nthreads (which defaults to hardwareConcurrency()) is the
			# number of threads that will be making Sx calls of any sort, whereas prior to that it
			# was the number of threads that would actually call SxCallShader. because we've set up
			# the renderer on this thread, it's taken one off the count for the number of threads we
			# can spawn to do the shading.
			threads = []
			for i in range( 0, IECore.hardwareConcurrency() - 1 ) :
				threads.append( threading.Thread( target = IECore.curry( r.shade, points ) ) )

			for t in threads :
				t.start()

			for t in threads :
				t.join()
						
		# but if you want to use more then you need to let the library know about it
		# by calling setOption( "ri:render:nthreads" )
		
		for i in range( 0, 10 ) :
				
			r = IECoreRI.SXRenderer()
			
			# see above - we're adding one to number of threads we'll be using to do the shading,
			# because we've also used a thread (the current thread) to perform the setup.
			r.setOption( "ri:render:nthreads", IECore.IntData( IECore.hardwareConcurrency() * 2 + 1 ) )

			r.shader( "surface", "test/IECoreRI/shaders/sxTextureTest.sdl", {
				"fileName" : os.path.realpath( "./test/IECoreRI/data/textures/uvMap.256x256.tdl" ),
			} )

			threads = []
			for i in range( 0, IECore.hardwareConcurrency() * 2 ) :
				threads.append( threading.Thread( target = IECore.curry( r.shade, points ) ) )

			for t in threads :
				t.start()

			for t in threads :
				t.join()
		
	def testUserOptions( self ):
		
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxUserOptionTest.sdl test/IECoreRI/shaders/sxUserOptionTest.sl" ), 0 )
		
		points = self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 1 ) ) )	
		
		r = IECoreRI.SXRenderer()
		r.shader( "surface", "test/IECoreRI/shaders/sxUserOptionTest.sdl", {} )
		
		s = r.shade( points )
		self.assertEqual( s["Ci"][0], IECore.Color3f( 0,0,0 ) )
		
		r.setOption( "user:outputColor", IECore.FloatData( 1 ) )
		
		s = r.shade( points )
		self.assertEqual( s["Ci"][0], IECore.Color3f( 1,1,1 ) )
	
	def testStringArrayOptions( self ):
		
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxStringArrayOptionTest.sdl test/IECoreRI/shaders/sxStringArrayOptionTest.sl" ), 0 )
		
		points = self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 1 ) ) )	
		
		r = IECoreRI.SXRenderer()
		r.shader( "surface", "test/IECoreRI/shaders/sxStringArrayOptionTest.sdl", {} )
		
		s = r.shade( points )
		self.assertEqual( s["Ci"][0], IECore.Color3f( 0,0,0 ) )
		
		r.setOption( "user:stringArray", IECore.StringVectorData( ["this","should","work"] ) )
		
		s = r.shade( points )
		self.assertEqual( s["Ci"][0], IECore.Color3f( 1,1,1 ) )
		
	
	def testCoordinateSystems( self ):
		
		self.assertEqual( os.system( "shaderdl -Irsl -o test/IECoreRI/shaders/sxCoordSystemTest.sdl test/IECoreRI/shaders/sxCoordSystemTest.sl" ), 0 )
		
		points = self.__rectanglePoints( IECore.Box2i( IECore.V2i( 0 ), IECore.V2i( 1 ) ) )
		
		r = IECoreRI.SXRenderer()
		
		r.shader( "surface", "test/IECoreRI/shaders/sxCoordSystemTest.sdl", { "coordSysName" : IECore.StringData( "test1" ) } )
		
		r.transformBegin()
		r.setTransform( IECore.M44f.createTranslated( IECore.V3f(0,0,2) ) )
		r.coordinateSystem( "test1" )
		r.transformEnd()
		
		r.transformBegin()
		r.setTransform( IECore.M44f.createRotated( IECore.V3f(1,0,0) ) )
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f(0,0,2) ) )
		r.coordinateSystem( "test2" )
		r.transformEnd()
		
		s1 = r.shade( points )
		for i in range( len( s1["Ci"] ) ):
			self.assertEqual( points["P"][i] + IECore.V3f(0,0,2), IECore.V3f( s1["Ci"][i][0], s1["Ci"][i][1], s1["Ci"][i][2] ) )
		
		r.shader( "surface", "test/IECoreRI/shaders/sxCoordSystemTest.sdl", { "coordSysName" : IECore.StringData( "test2" ) } )
		s2 = r.shade( points )
		for i in range( len( s2["Ci"] ) ):
			shaderP = IECore.V3f( s2["Ci"][i][0], s2["Ci"][i][1], s2["Ci"][i][2] )
			transP = points["P"][i] * IECore.M44f.createTranslated( IECore.V3f(0,0,2) ) * IECore.M44f.createRotated( IECore.V3f(1,0,0) )
			
			self.failUnless( ( shaderP - transP ).length() < 1.e-5 )

	def tearDown( self ) :
				
		files = [
			"test/IECoreRI/shaders/sxTest.sdl",
			"test/IECoreRI/shaders/splineTest.sdl",
			"test/IECoreRI/shaders/sxParameterTest.sdl",
			"test/IECoreRI/shaders/sxStackTest.sdl",
			"test/IECoreRI/shaders/sxCoshaderTest.sdl",
			"test/IECoreRI/shaders/sxCoshaderTestMain.sdl",
			"test/IECoreRI/shaders/sxGridTest.sdl",
			"test/IECoreRI/shaders/sxDisplacementTest.sdl",
			"test/IECoreRI/shaders/sxIlluminanceTest.sdl",
			"test/IECoreRI/shaders/sxLightTest.sdl",
			"test/IECoreRI/shaders/sxStTest.sdl",
			"test/IECoreRI/shaders/sxPredefinedPrimitiveVariableTest.sdl",
			"test/IECoreRI/shaders/sxNonPredefinedPrimitiveVariableTest.sdl",
			"test/IECoreRI/shaders/sxGetVarTest.sdl",
			"test/IECoreRI/shaders/sxGetShaderTest.sdl",
			"test/IECoreRI/shaders/sxTextureTest.sdl",
			"test/IECoreRI/shaders/sxUniformPrimitiveVariableShaderParameterTest.sdl",
			"test/IECoreRI/shaders/sxUniformPrimitiveVariableTest.sdl",
			"test/IECoreRI/shaders/sxUserOptionTest.sdl",
			"test/IECoreRI/shaders/sxCoordSystemTest.sdl",
			"test/IECoreRI/shaders/sxStringArrayOptionTest.sdl",
		]
		
		for f in files :
			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
    unittest.main()
