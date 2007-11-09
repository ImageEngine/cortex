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
import random

import VersionControl
VersionControl.setVersion( "IECore", "2" ) ## \todo need to get version from build config somehow, or have the coreGL module initialise the core module somehow
from IECore import *

from IECoreGL import *
init( False )

class TestPointsPrimitive( unittest.TestCase ) :

	def testVertexAttributes( self ) :
	
		fragmentSource = """
		uniform int greyTo255;
		
		void main()
		{
			float g = float( greyTo255 ) / 255.0;
			gl_FragColor = vec4( g, g, g, 1 );
		}
		"""

		numPoints = 100
		p = V3fVectorData( numPoints )
		g = IntVectorData( numPoints )
		for i in range( 0, numPoints ) :
			p[i] = V3f( random.random() * 4, random.random() * 4, 0 )
			g[i] = int( random.uniform( 0.0, 255.0 ) )
		p = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, p )
		g = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, g )

		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		
		r.worldBegin()
		# we have to make this here so that the shaders that get made are made in the
		# correct GL context. My understanding is that all shaders should work in all
		# GL contexts in the address space, but that doesn't seem to be the case.
		w = SceneViewer( "scene", r.scene() )
		
		r.concatTransform( M44f.createTranslated( V3f( -2, -2, -5 ) ) )
		r.shader( "surface", "grey", { "gl:fragmentSource" : StringData( fragmentSource ) } )
		r.points( numPoints, { "P" : p, "greyTo255" : g } )
		
		r.worldEnd()
	
		w.start()
		
if __name__ == "__main__":
    unittest.main()   
