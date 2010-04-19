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

from IECore import *

from IECoreGL import *
init( False )

class TestCameraControl( unittest.TestCase ) :

	def test( self ) :

		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( "test/shaders" ) )

		r.worldBegin()
		c = PerspectiveCamera( horizontalFOV = 90 )
		#c = OrthographicCamera()
		s = r.scene()
		s.setCamera( c )
		w = SceneViewer( "scene", s )

		r.setAttribute( "gl:blend:srcFactor", StringData( "one" ) )
		r.setAttribute( "gl:blend:dstFactor", StringData( "one" ) )
		r.setAttribute( "gl:blend:equation", StringData( "add" ) )

		r.concatTransform( M44f.createTranslated( V3f( 0, 0, 5 ) ) )
		r.concatTransform( M44f.createScaled( V3f( 0.004 ) ) )

		r.concatTransform( M44f.createTranslated( V3f( -150, -200, 0 ) ) )
		i = Reader.create( "test/images/numberWithAlpha.exr" ).read()
		i.render( r )

		r.concatTransform( M44f.createTranslated( V3f( 300, 300, 1 ) ) )
		i.render( r )

		r.worldEnd()
		w.start()

if __name__ == "__main__":
    unittest.main()
