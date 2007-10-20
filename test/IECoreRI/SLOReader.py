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
import os
from IECore import *
from IECoreRI import *
import os

class TestSLOReader( unittest.TestCase ) :

	def shaderPath( self ) :
	
		return os.environ["SHADER_PATH"]

	def assertExpectedParams( self, s, params ) :
	
		for p in params :
			self.assertEqual( s.parameters[p[0]].typeName(), p[1] )
			if p[1]=="FloatData" :
				self.assertAlmostEqual( s.parameters[p[0]].value, p[2] )
			else :
				self.assertEqual( s.parameters[p[0]].value, p[2] )
			if len( p )>3 :
				self.assertEqual( p[3], s.blindData()["ri:parameterTypeHints"][p[0]].value )

	def testPlastic( self ) :
	
		r = SLOReader( os.path.join( self.shaderPath(), "plastic.sdl" ) )
		s = r.read()
		self.assertEqual( s.name, "plastic" )
		self.assertEqual( s.type, "surface" )
		self.assertEqual( len( s.parameters ), 5 )
		
		self.assertExpectedParams ( s,
			[
				["Ks", "FloatData", 0.5],
				["Kd", "FloatData", 0.5],
				["Ka", "FloatData", 1],
				["roughness", "FloatData", 0.1],
				["specularcolor", "Color3fData", Color3f( 1 ) ],
			]
		)
		
	def testSpotLight( self ) :
	
		r = SLOReader( os.path.join( self.shaderPath(), "spotlight.sdl" ) )
		s = r.read()
		self.assertEqual( s.name, "spotlight" )
		self.assertEqual( s.type, "light" )
		self.assertEqual( len( s.parameters ), 12 )
		
		self.assertExpectedParams ( s,
			[
				["intensity", "FloatData", 1],
				["lightcolor", "Color3fData", Color3f( 1 ) ],
				["shadowmap", "StringData", ""],
				["blur", "FloatData", 0],
				["from", "V3fData", V3f( 0 ), "point" ],
				["to", "V3fData", V3f( 0, 0, 1 ), "point" ],
			]
		)	
		
				
if __name__ == "__main__":
    unittest.main()   
