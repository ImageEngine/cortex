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

class TestImmediateRenderer( unittest.TestCase ) :

	def test( self ) :
	
		r = Renderer()
		r.setOption( "gl:mode", StringData( "immediate" ) )
		r.setOption( "gl:searchPath:shader", StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		
		r.camera( "main", {
				"projection" : StringData( "perspective" ),
				"projection:fov" : FloatData( 45 ),
				"resolution" : V2iData( V2i( 256 ) ),
				"clippingPlanes" : V2fData( V2f( 1, 1000 ) ),
				"screenWindow" : Box2fData( Box2f( V2f( -0.5 ), V2f( 0.5 ) ) )
			}
		)
		r.display( "/tmp/immediate.tif", "tif", "rgba", {} )
		
		r.worldBegin()
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, -5 ) ) )
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 0, 0, 1 ) ) } )
		r.geometry( "sphere", {}, {} )
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 1, 0 ) ) )
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 1, 0 ) ) } )
		r.geometry( "sphere", {}, {} )
		r.worldEnd()

if __name__ == "__main__":
    unittest.main()   
