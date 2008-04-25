##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
from IECore import *
import IECoreRI

class SimpleProcedural( Renderer.Procedural ) :

	def __init__( self, scale ) :
	
		Renderer.Procedural.__init__( self, "name", "description" )
		self.__scale = scale
		self.__t = StringData( "hello" )
		self.__c = CompoundData()
		self.__c["a"] = IntData( 4 )

	def doBound( self, args ) :
	
		return Box3f( V3f( -self.__scale ), V3f( self.__scale ) )
		
	def doRender( self, renderer, args ) :
	
		renderer.transformBegin()
		
		m = M44f()
		m.scale( V3f( self.__scale ) )
		renderer.concatTransform( m )
		
		renderer.transformEnd()
		

class RendererTest( unittest.TestCase ) :

	def loadShader( self, shader ) :
	
		return IECoreRI.SLOReader( os.path.join( os.environ["SHADER_PATH"], shader + ".sdl" ) ).read()

	def testTypeName( self ) :
	
		r = IECoreRI.Renderer()
		self.assertEqual( r.typeName(), "IECoreRI::Renderer" )

	## \todo Make this test actually test something
	def testNoContext( self ) :
	
		r = IECoreRI.Renderer()

	## \todo Make this test actually test something
	def test( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/test.rib" )
		
		r.setOption( "ri:searchpath:shader", StringData( os.environ["SHADER_PATH"] ) )
		r.setOption( "ri:render:bucketorder", StringData( "zigzag" ) )
		r.setOption( "user:magicNumber", IntData( 42 ) )
		r.setOption( "ri:pixelSamples", V2iData( V2i( 8, 8 ) ) )
		
		r.worldBegin()
		
		r.transformBegin()
		r.attributeBegin()
		
		self.loadShader( "plastic" ).render( r )
		
		Reader.create( "test/IECoreRI/data/sphere.cob" ).read().render( r )
		
		r.attributeEnd()
		r.transformEnd()
		
		r.worldEnd()

	def testAttributes( self ) :
	
		tests = [
			# format is : name value expectedRib getAttributeShouldWork
			( "ri:shadingRate", FloatData( 2 ), "ShadingRate 2", True ),
			( "ri:matte", BoolData( 0 ), "Matte 0", True ),
			( "ri:matte", BoolData( 1 ), "Matte 1", True ),
			( "user:whatever", StringData( "whatever" ), "Attribute \"user\" \"string whatever\" [ \"whatever\" ]", True ),
			( "ri:color", Color3fData( Color3f( 0, 1, 1 ) ), "Color [ 0 1 1 ]", False ),
			( "color", Color3fData( Color3f( 1, 2, 3 ) ), "Color [ 1 2 3 ]", False ),
			( "ri:opacity", Color3fData( Color3f( 1, 1, 1 ) ), "Opacity [ 1 1 1 ]", False ),
			( "opacity", Color3fData( Color3f( 0, 1, 0 ) ), "Opacity [ 0 1 0 ]", False ),
			( "ri:sides", IntData( 1 ), "Sides 1", False ),
			( "ri:geometricApproximation:motionFactor", FloatData( 1 ), "GeometricApproximation \"motionfactor\" 1", False ),
			( "ri:geometricApproximation:focusFactor", FloatData( 1 ), "GeometricApproximation \"focusfactor\" 1", False ),
			( "ri:cull:hidden", IntData( 0 ), "Attribute \"cull\" \"int hidden\" [ 0 ]", False ),
			( "name", StringData( "oioi" ), "Attribute \"identifier\" \"string name\" [ \"oioi\" ]", True ),
			( "ri:trace:bias", FloatData( 2 ), "Attribute \"trace\" \"float bias\" [ 2 ]", True ),
		]
		
		for t in tests :
		
			r = IECoreRI.Renderer( "test/IECoreRI/output/testAttributes.rib" )
			r.worldBegin()
			r.setAttribute( t[0], t[1] )
			if t[3] :
				self.assertEqual( r.getAttribute( t[0] ), t[1] )
			r.worldEnd()
			
			l = "".join( file( "test/IECoreRI/output/testAttributes.rib" ).readlines() )
			l = " ".join( l.split() )
			self.assert_( t[2] in l )

		
	## \todo Make this test actually test something
	def testProcedural( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testProcedural.rib" )
		r.worldBegin()
		
		r.procedural( SimpleProcedural( 10.5 ) )
		
		r.worldEnd()
		
	def testGetOption( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testGetOption.rib" )
		
		r.camera( "main", { "resolution" : V2iData( V2i( 1024, 768 ) ) } )
		
		r.worldBegin()
		
		s = r.getOption( "shutter" )
		self.assertEqual( s, V2fData( V2f( 0 ) ) )
		
		self.assertEqual( r.getOption( "camera:resolution" ), V2iData( V2i( 1024, 768 ) ) )
		
		r.worldEnd()
		
	## \todo Make this test actually test something
	def testDisplay( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testDisplay.rib" )
		
		r.display( "test.tif", "tiff", "rgba", { "quantize" : FloatVectorData( [ 0, 1, 0, 1 ] ) } )
		
		r.worldBegin()
		r.worldEnd()
		
	## \todo Make this test actually test something
	def testSubDivs( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/subdiv.rib" )
		
		r.display( "test", "idisplay", "rgba", {} )
		
		r.worldBegin()
		
		t = M44f()
		t.translate( V3f( 0, 0, 10 ) )
		r.concatTransform( t )
		m = ObjectReader( "test/IECoreRI/data/openSubDivCube.cob" ).read()
		m.render( r )
		
		r.worldEnd()
	
	## \todo Make this test actually test something
	def testCommands( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/commands.rib" )
		
		r.worldBegin()
		
		r.command( "ri:readArchive", { "name" : StringData( "nameOfArchive" ) } )
		
		r.worldEnd()
		
	## \todo Make this test actually test something
	def testMotion( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/motion.rib" )
		
		r.worldBegin()
		
		m = MatrixMotionTransform()
		m[0] = M44f.createTranslated( V3f( 0, 1, 0 ) )
		m[1] = M44f.createTranslated( V3f( 0, 10, 0 ) )
		
		m.render( r )
		
		r.worldEnd()	
	
	## \todo Make this test actually test something
	def testStringPrimVars( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/stringPrimVars.rib" )	
		
		r.worldBegin()
		
		m = ObjectReader( "test/IECoreRI/data/stringPrimVars.cob" ).read()
		m.render( r )
		
		r.worldEnd()
		
	def testGetTransform( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/transform.rib" )
		
		r.worldBegin()
		
		self.assertEqual( r.getTransform(), M44f() )
		self.assertEqual( r.getTransform( "world" ), M44f() )
		self.assertEqual( r.getTransform( "object" ), M44f() )
		
		r.transformBegin()
		
		t = M44f.createTranslated( V3f( 1, 2, 3 ) ) * M44f.createScaled( V3f( 2, 1, 0 ) ) * M44f.createRotated( V3f( 20, 0, 90 ) )
		r.concatTransform( t )
		self.assert_( r.getTransform( "object" ).equalWithAbsError( t, 0.000001 ) )
		self.assert_( r.getTransform().equalWithAbsError( t, 0.000001 ) )
		
		r.coordinateSystem( "coordSys" )
		self.assert_( r.getTransform( "coordSys" ).equalWithAbsError( t, 0.000001 ) )
		
		r.transformEnd()
		
		self.assertEqual( r.getTransform(), M44f() )
		self.assertEqual( r.getTransform( "world" ), M44f() )
		self.assertEqual( r.getTransform( "object" ), M44f() )
		self.assert_( r.getTransform( "coordSys" ).equalWithAbsError( t, 0.000001 ) )
		
		r.worldEnd()
		
	def testIgnoreOtherAttributesAndOptions( self ) :
	
		m = CapturingMessageHandler()
		s = ScopedMessageHandler( m )
		
		r = IECoreRI.Renderer( "test/IECoreRI/output/transform.rib" )
		
		# this should be silently ignored
		r.setOption( "someOthereRenderer:someOtherOption", IntData( 10 ) )
		
		r.worldBegin()
		
		# this should be silently ignored
		r.setAttribute( "someOtherRenderer:someOtherAttribute", IntData( 10 ) )
		
		r.worldEnd()
		
		self.assertEqual( len( m.messages ), 0 )
	
	def testMissingShaders( self ) :
	
		"""Check that missing shaders don't throw an exception but print a message instead."""
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/missingShaders.rib" )
		
		r.worldBegin()
		
		r.shader( "surface", "aShaderWhichDoesntExist", {} )
		
		r.worldEnd()
		
	def testGetUserOption( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/getUserOption.rib" )
		
		o = {
			"user:f" : FloatData( 10 ),
			"user:i" : IntData( 100 ),
			"user:s" : StringData( "hello" ),
			"user:c" : Color3fData( Color3f( 1, 0, 0 ) ),
			"user:v" : V3fData( V3f( 1, 2, 3 ) ),
			"user:m" : M44fData( M44f.createTranslated( V3f( 1, 2, 3 ) ) ),
		}
		
		for k, v in o.items() :
		
			r.setOption( k, v )
			self.assertEqual( r.getOption( k ), v )
			
	def testGetUserAttribute( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/getUserAttribute.rib" )
		
		o = {
			"user:f" : FloatData( 10 ),
			"user:i" : IntData( 100 ),
			"user:s" : StringData( "hello" ),
			"user:c" : Color3fData( Color3f( 1, 0, 0 ) ),
			"user:v" : V3fData( V3f( 1, 2, 3 ) ),
			"user:m" : M44fData( M44f.createTranslated( V3f( 1, 2, 3 ) ) ),
		}
		
		for k, v in o.items() :
		
			r.setAttribute( k, v )
			self.assertEqual( r.getAttribute( k ), v )
			
	def tearDown( self ) :
	
		files = [
			"test/IECoreRI/output/test.rib",
			"test/IECoreRI/output/testAttributes.rib",
			"test/IECoreRI/output/testProcedural.rib",
			"test/IECoreRI/output/testGetOption.rib",
			"test/IECoreRI/output/testDisplay.rib",
			"test/IECoreRI/output/testCamera.rib",
			"test/IECoreRI/output/subdiv.rib",
			"test/IECoreRI/output/commands.rib",
			"test/IECoreRI/output/motion.rib",
			"test/IECoreRI/output/stringPrimVars.rib",
			"test/IECoreRI/output/transform.rib",
			"test/IECoreRI/output/missingShaders.rib",
			"test/IECoreRI/output/getUserOption.rib",
			"test/IECoreRI/output/getUserAttribute.rib",
		]
		
		for f in files :
			if os.path.exists( f ):
				os.remove( f )		
			
		
if __name__ == "__main__":
    unittest.main()   
