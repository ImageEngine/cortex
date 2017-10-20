##########################################################################
#
#  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

import arnold

import IECore
import IECoreArnold

class CameraAlgoTest( unittest.TestCase ) :

	def testConvertPerspective( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert(
				IECore.Camera(
					parameters = {
						"projection" : "perspective",
						"projection:fov" : 45.0,
						"resolution" : IECore.V2i( 512 ),
						"screenWindow" : IECore.Box2f( IECore.V2f( -1, -0.5 ), IECore.V2f( 1, 0.5 ) )
					}
				),
				"testCamera"
			)

			self.assertTrue( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n ) ), "persp_camera" )
			self.assertEqual( arnold.AiNodeGetFlt( n, "fov" ), 45.0 )

			self.assertEqual( arnold.AiNodeGetVec2( n, "screen_window_min" ), arnold.AtVector2( -1, -0.5 ) )
			self.assertEqual( arnold.AiNodeGetVec2( n, "screen_window_max" ), arnold.AtVector2( 1, 0.5 ) )

	def testConvertCustomProjection( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert(
				IECore.Camera(
					parameters = {
						"projection" : "cyl_camera",
						"horizontal_fov" : 45.0,
						"vertical_fov" : 80.0,
					}
				),
				"testCamera"
			)

			self.assertTrue( arnold.AiNodeEntryGetName( arnold.AiNodeGetNodeEntry( n ) ), "cyl_camera" )
			self.assertEqual( arnold.AiNodeGetFlt( n, "horizontal_fov" ), 45.0 )
			self.assertEqual( arnold.AiNodeGetFlt( n, "vertical_fov" ), 80.0 )

if __name__ == "__main__":
    unittest.main()
