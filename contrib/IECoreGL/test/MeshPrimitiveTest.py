##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
import random

from IECore import *

from IECoreGL import *
init( False )

class MeshPrimitiveTest( unittest.TestCase ) :

	## \todo Make this actually assert something
	def testVertexAttributes( self ) :
	
		vertexSource = """
		attribute vec2 st;
		varying vec4 stColor;
		
		void main()
		{
			gl_Position = ftransform();
			
			stColor = vec4(st.x, st.y, 0.0, 1.0);			
		}
		"""
	
		fragmentSource = """
		varying vec4 stColor;
		
		void main()
		{
			gl_FragColor = stColor;
		}
		"""
		
		m = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob").read()
		
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		
		r.worldBegin()
		# we have to make this here so that the shaders that get made are made in the
		# correct GL context. My understanding is that all shaders should work in all
		# GL contexts in the address space, but that doesn't seem to be the case.
		#w = SceneViewer( "scene", r.scene() )
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -15 ) ) )
		r.shader( "surface", "showS", 
			{ "gl:fragmentSource" : StringData( fragmentSource ),
			  "gl:vertexSource" :   StringData( vertexSource   ) 
			}
		)
		
		primVars = {}
		
		primVars["P"] = m["P"]		
		primVars["s"] = m["s"]
		primVars["t"] = m["t"]
		primVars["N"] = m["N"]		
		
		r.mesh( m.verticesPerFace, m.vertexIds, m.interpolation, primVars )
		
		r.worldEnd()
	
		#w.start()
		
if __name__ == "__main__":
    unittest.main()   
