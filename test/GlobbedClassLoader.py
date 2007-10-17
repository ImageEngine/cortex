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
import IECore

class TestGlobbedClassLoader( unittest.TestCase ) :

	def test( self ) :
	
		l = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) )
		
		gl = IECore.GlobbedClassLoader( classLoader = l )		

		self.assertEqual( gl.classNames(), ["bad", "maths/multiply", "parameterTypes", "path.With.Dot/multiply", "presetParsing", 'stringParsing'] )
		self.assertEqual( gl.classNames( "*multiply" ), ["maths/multiply", "path.With.Dot/multiply"] )
		self.assertEqual( gl.classNames( "p*" ), ["parameterTypes", "path.With.Dot/multiply", "presetParsing"] )
		self.assertEqual( gl.versions( "maths/multiply" ), [ 1, 2 ] )
		
		o = gl.load( "maths/multiply" )()
		self.assertEqual( len( o.parameters() ), 2 )

		h = gl.help( "maths/multiply" )
		self.assertEqual( h.split( '\n' )[0], 'Name    : multiply' )
		
	def testContexts( self ) :
	
		l = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) )
		gl = IECore.GlobbedClassLoader( contexts = [ "maths/" ], classLoader = l )
		self.assertEqual( gl.classNames(), [ "maths/multiply" ] )
		o = gl.load( "*multiply" )()
		self.assertEqual( len( o.parameters() ), 2 )
		
if __name__ == "__main__":
        unittest.main()
