##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

import unittest
import os
import shutil

import IECore
import IECoreGL

IECoreGL.init( False )

class MeshPrimitiveTest( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/testMesh.tif"

	def testVertexAttributes( self ) :

		vertexSource = """
		#include "IECoreGL/VertexShader.h"
		
		IECOREGL_VERTEXSHADER_IN vec3 vertexP;
		IECOREGL_VERTEXSHADER_IN vec2 vertexst;
		IECOREGL_VERTEXSHADER_OUT vec4 stColor;

		void main()
		{
			vec4 pCam = gl_ModelViewMatrix * vec4( vertexP, 1 );
			gl_Position = gl_ProjectionMatrix * pCam;

			stColor = vec4(vertexst.x, vertexst.y, 0.0, 1.0);
		}
		"""

		fragmentSource = """
		varying vec4 stColor;

		void main()
		{
			gl_FragColor = stColor;
		}
		"""

		m = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob").read()

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.outputFileName, "tif", "rgba", {} )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -15 ) ) )
			r.shader( "surface", "showST",
				{ "gl:fragmentSource" : IECore.StringData( fragmentSource ),
				  "gl:vertexSource" : IECore.StringData( vertexSource   )
				}
			)

			m.render( r )

		reader = IECore.Reader.create( os.path.dirname( __file__ ) + "/expectedOutput/meshST.tif" )
		reader['colorSpace'] = 'linear'
		expectedImage = reader.read()
		actualImage = IECore.Reader.create( self.outputFileName ).read()
		
		self.assertEqual( IECore.ImageDiffOp()( imageA = expectedImage, imageB = actualImage, maxError = 0.05 ).value, False )

	def setUp( self ) :
		
		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )
	
	def tearDown( self ) :
		
		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )
	
if __name__ == "__main__":
    unittest.main()
