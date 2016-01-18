##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

import re
import arnold
import unittest

import IECore
import IECoreArnold
		
class ProceduralTest( unittest.TestCase ) :
	
	class SphereProcedural( IECore.Renderer.Procedural ) :
	
		def __init__( self, radius=1 ) :
		
			IECore.Renderer.Procedural.__init__( self )
		
			self.__radius = radius
		
		def bound( self ) :
		
			return IECore.Box3f( IECore.V3f( -self.__radius ), IECore.V3f( self.__radius ) )
		
		def render( self, renderer ) :
		
			renderer.sphere( self.__radius, -1, 1, 360, {} )
		
		def hash( self ):
		
			h = IECore.MurmurHash()
			return h
		
	class TransformingProcedural( IECore.Renderer.Procedural ) :
	
		def __init__( self, transform, child ) :
		
			IECore.Renderer.Procedural.__init__( self )
		
			self.__transform = transform
			self.__child = child
			
		def bound( self ) :
		
			b = self.__child.bound()
			b = b.transform( self.__transform )
			return b
			
		def render( self, renderer ) :
		
			with IECore.TransformBlock( renderer ) :
				
				renderer.concatTransform( self.__transform )
				if isinstance( self.__child, IECore.VisibleRenderable ) :
					self.__child.render( renderer )
				else :
					renderer.procedural( self.__child )
		
		def hash( self ):
		
			h = IECore.MurmurHash()
			return h
		
	class ShaderProcedural( IECore.Renderer.Procedural ) :
		
		def __init__( self, shader, child ) :
		
			IECore.Renderer.Procedural.__init__( self )
			
			self.__shader = shader
			self.__child = child
			
		def bound( self ) :
		
			return self.__child.bound()
			
		def render( self, renderer ) :
		
			with IECore.AttributeBlock( renderer ) :
			
				self.__shader.render( renderer )
				renderer.procedural( self.__child )
		
		def hash( self ):
		
			h = IECore.MurmurHash()
			return h
		
	def arnoldMessageCallback( self, logMask, severity, msg, tabs ) :
	
		self.__arnoldMessages += msg

	def testTransformingProceduralBounds( self ) :
	
		r = IECoreArnold.Renderer()
		
		messageCallback = arnold.AtMsgCallBack( self.arnoldMessageCallback )
		arnold.AiMsgSetCallback( messageCallback )
		self.__arnoldMessages = ""
		
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )
		
		with IECore.WorldBlock( r ) : 
			
			r.procedural(
				self.TransformingProcedural(
					IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ),
					self.SphereProcedural()
				)
			)

		self.failIf( "incorrect user bounds" in self.__arnoldMessages )
		
	def testNestedTransformingProceduralBounds( self ) :
	
		r = IECoreArnold.Renderer()
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )

		messageCallback = arnold.AtMsgCallBack( self.arnoldMessageCallback )
		arnold.AiMsgSetCallback( messageCallback )
		self.__arnoldMessages = ""
		
		with IECore.WorldBlock( r ) : 
			
			r.procedural(
				self.TransformingProcedural(
					IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ),
					self.TransformingProcedural(
						IECore.M44f.createScaled( IECore.V3f( 0.2 ) ),
						self.SphereProcedural()
					)
				)
			)
		
		self.failIf( "incorrect user bounds" in self.__arnoldMessages )		

	def testProceduralInheritsShader( self ) :
	
		r = IECoreArnold.Renderer()
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )

		with IECore.WorldBlock( r ) :
				
			r.procedural(
				self.ShaderProcedural(
					IECore.Shader( "flat", "surface", { "color" : IECore.Color3f( 0, 1, 0 ) } ),
					self.SphereProcedural()
				)
			)

		i = IECore.ImageDisplayDriver.removeStoredImage( "testHandle" )
		
		e = IECore.ImagePrimitiveEvaluator( i )
		r = e.createResult()
		self.assertEqual( e.pointAtUV( IECore.V2f( 0.5 ), r ), True )
		self.assertEqual( r.floatPrimVar( e.R() ), 0 )
		self.assertEqual( r.floatPrimVar( e.G() ), 1 )
		self.assertEqual( r.floatPrimVar( e.B() ), 0 )
		
	def testEmptyProceduralIsIgnored( self ) :
	
		class EmptyProcedural( IECore.Renderer.Procedural ) :
		
			def __init__( self ) :
		
				IECore.Renderer.Procedural.__init__( self )
				
			def bound( self ) :
			
				return IECore.Box3f()
				
			def render( self, renderer ) :
			
				pass

			def hash( self ):

				h = IECore.MurmurHash()
				return h

		r = IECoreArnold.Renderer()
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )

		messageCallback = arnold.AtMsgCallBack( self.arnoldMessageCallback )
		arnold.AiMsgSetCallback( messageCallback )
		self.__arnoldMessages = ""
		
		with IECore.WorldBlock( r ) : 
			
			r.procedural( EmptyProcedural() )
		
		self.failIf( "ignoring parameter max" in self.__arnoldMessages )

	def testNoBound( self ) :

		r = IECoreArnold.Renderer( "/tmp/test.ass" )

		with IECore.WorldBlock( r ) :

			r.procedural(
				r.ExternalProcedural(
					"test.so",
					r.Procedural.noBound,
					{}
				)
			)

		l = "".join( open( "/tmp/test.ass" ).readlines() )

		self.assertTrue( "procedural" in l )
		self.assertFalse( "inf" in l )
		self.assertFalse( re.search( r"\bmin\b", l ) )
		self.assertFalse( re.search( r"\bmax\b", l ) )
		self.assertTrue( "load_at_init" in l )

if __name__ == "__main__":
    unittest.main()
