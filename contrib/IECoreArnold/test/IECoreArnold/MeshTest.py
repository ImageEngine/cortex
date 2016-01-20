##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

import os
import unittest

import arnold

import IECore
import IECoreArnold

class MeshTest( unittest.TestCase ) :

	def testUVs( self ) :

		r = IECoreArnold.Renderer()
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )
		with IECore.WorldBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			r.shader( "surface", "utility", { "shade_mode" : "flat", "color_mode" : "uv" } )
			IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) ).render( r )

		del r

		image = IECore.ImageDisplayDriver.removeStoredImage( "testHandle" )
		expectedImage = IECore.EXRImageReader( os.path.dirname( __file__ ) + "/data/meshImages/expectedMeshUVs.exr" ).read()

		self.failIf( IECore.ImageDiffOp()( imageA=image, imageB=expectedImage, maxError=0.003 ).value )

	def testNormals( self ) :

		r = IECoreArnold.Renderer()
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )
		with IECore.WorldBlock( r ) :
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			r.shader( "surface", "utility", { "shade_mode" : "flat", "color_mode" : "n" } )
			m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -0.9 ), IECore.V2f( 0.9 ) ) )
			m["N"] = IECore.PrimitiveVariable(
					IECore.PrimitiveVariable.Interpolation.Vertex,
					IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 1, 0, 0 ) ] )
			)
			m.render( r )

		del r

		image = IECore.ImageDisplayDriver.removeStoredImage( "testHandle" )

		e = IECore.PrimitiveEvaluator.create( image )
		result = e.createResult()

		# the utility shader encodes the normals in the range 0-1 rather than -1-1,
		# which is why we're checking G and B against .5 rather than 0.
		e.pointAtUV( IECore.V2f( 0.5 ), result )
		self.assertAlmostEqual( result.floatPrimVar( e.R() ), 1, 4 )
		self.assertAlmostEqual( result.floatPrimVar( e.G() ), 0.5, 4 )
		self.assertAlmostEqual( result.floatPrimVar( e.B() ), 0.5, 4 )

	def testVertexPrimitiveVariables( self ) :

		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m["myPrimVar"] = IECore.PrimitiveVariable(
			IECore.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( [ 0, 1, 2, 3 ] )
		)

		with IECoreArnold.UniverseBlock() :

			n = IECoreArnold.ToArnoldMeshConverter( m ).convert()
			a = arnold.AiNodeGetArray( n, "user:myPrimVar" )
			self.assertEqual( a.contents.nelements, 4 )
			for i in range( 0, 4 ) :
				self.assertEqual( arnold.AiArrayGetFlt( a, i ), i )

if __name__ == "__main__":
    unittest.main()
