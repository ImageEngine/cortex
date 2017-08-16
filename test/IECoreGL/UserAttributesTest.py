##########################################################################
#
#  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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
import os.path
import os
import math

class UserAtributesTest( unittest.TestCase ) :

	def testUserAttributesInDeferredMode( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.worldBegin()

		self.assertEqual( r.getAttribute( "user:notSetYet" ), None )

		r.setAttribute( "user:test", IECore.FloatData( 1 ) )
		self.assertEqual( r.getAttribute( "user:test" ), IECore.FloatData( 1 ) )

		r.attributeBegin()

		self.assertEqual( r.getAttribute( "user:test" ), IECore.FloatData( 1 ) )

		r.setAttribute( "user:test2", IECore.IntData( 10 ) )
		self.assertEqual( r.getAttribute( "user:test2" ), IECore.IntData( 10 ) )

		r.attributeEnd()

		self.assertEqual( r.getAttribute( "user:test" ), IECore.FloatData( 1 ) )
		self.assertEqual( r.getAttribute( "user:test2" ), None )

		r.worldEnd()

	def performProceduralTest( self, threaded ) :
	
		errors = list()

		class SimpleProcedural( IECore.ParameterisedProcedural ):

			def __init__( s, level = 0 ):
				IECore.ParameterisedProcedural.__init__( s )
				s.__level = level

			def doBound( s, args ) :
				return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )

			def doRender( s, renderer, args ):

				try:
					if s.__level == 0 :
						with IECore.AttributeBlock( renderer ) :
							renderer.setAttribute( "user:myTestAttribute", IECore.IntData(11) )
							# rendering a child procedural
							SimpleProcedural( 1 ).render( renderer )
							self.assertEqual( renderer.getAttribute( "user:myTestAttribute" ), IECore.IntData(11) )	
							# rendering child procedural from inside a Group
							g = IECore.Group()
							g.addChild( SimpleProcedural( 2 ) )
							g.render( renderer )
							
					elif s.__level == 1 :
						self.assertEqual( renderer.getAttribute( "user:myTestAttribute" ), IECore.IntData(11) )	
						
					elif s.__level == 2 :
						self.assertEqual( renderer.getAttribute( "user:myTestAttribute" ), IECore.IntData(11) )	
						
				except Exception, e :
					errors.append( IECore.exceptionInfo()[1] )

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		with IECore.WorldBlock( r ) :
			r.setAttribute( "gl:procedural:reentrant", IECore.BoolData( threaded ) )
			p = SimpleProcedural()
			p.render( r )
		
		if errors :
			raise Exception, "ERRORS:\n".join( errors )

	def testUserAttributesInSingleThreadedProcedural( self ) :

		self.performProceduralTest( False )
		
	def testUserAttributesInMultiThreadedProcedural( self ) :
	
		self.performProceduralTest( True )

	def testUserAttributesInImmediateMode( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.worldBegin()

		self.assertEqual( r.getAttribute( "user:notSetYet" ), None )

		r.setAttribute( "user:test", IECore.FloatData( 1 ) )
		self.assertEqual( r.getAttribute( "user:test" ), IECore.FloatData( 1 ) )

		r.attributeBegin()

		self.assertEqual( r.getAttribute( "user:test" ), IECore.FloatData( 1 ) )

		r.setAttribute( "user:test2", IECore.IntData( 10 ) )
		self.assertEqual( r.getAttribute( "user:test2" ), IECore.IntData( 10 ) )

		r.attributeEnd()

		self.assertEqual( r.getAttribute( "user:test" ), IECore.FloatData( 1 ) )
		self.assertEqual( r.getAttribute( "user:test2" ), None )

		r.worldEnd()


if __name__ == "__main__":
    unittest.main()
