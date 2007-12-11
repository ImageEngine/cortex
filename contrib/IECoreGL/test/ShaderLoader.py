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
import os.path

from IECore import *

from IECoreGL import *
init( False )

class TestShaderLoader( unittest.TestCase ) :

	def test( self ) :
		
		sp = SearchPath( os.path.dirname( __file__ ) + "/shaders", ":" )

		l = ShaderLoader( sp )
		
		s = l.load( "3dLabs/Toon" )	
		self.assert_( s.typeName()=="Shader" )
		
		ss = l.load( "3dLabs/Toon" )
		self.assert_( s.isSame( ss ) )
		
		# shader is too complicated for my graphics card
		s = l.load( "3dLabs/Mandel" )	
		self.assert_( s.typeName()=="Shader" )
		
		self.assert_( ShaderLoader.defaultShaderLoader().isSame( ShaderLoader.defaultShaderLoader() ) )
	
	def testPreprocessing( self ) :
	
		sp = SearchPath( os.path.dirname( __file__ ) + "/shaders", ":" )
		psp = SearchPath( os.path.dirname( __file__ ) + "/shaders/include", ":" )
		
		# this should work
		l = ShaderLoader( sp, psp )
		s = l.load( "failWithoutPreprocessing" )
		
		# but turning off preprocessing should cause a throw
		l = ShaderLoader( sp )
		self.assertRaises( RuntimeError, l.load, "failWithoutPreprocessing" )
		
if __name__ == "__main__":
    unittest.main()   
