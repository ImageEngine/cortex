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

import unittest
import IECore
import IECoreGL
IECoreGL.init( False )
import os.path
import os
import shutil
import math

class OrientationTest( unittest.TestCase ) :

	## Makes a plane on the XY plane which appears with an anticlockwise winding order when
	# viewing down the negative Z axis.
	def makePlane( self ) :

		V = IECore.V3f
		p = IECore.V3fVectorData( [ V( 1, -1, 0 ), V( 1, 1, 0 ), V( -1, 1, 0 ), V( -1, -1, 0 ) ] )
		nVerts = IECore.IntVectorData( [ 4 ] )
		vertIds = IECore.IntVectorData( [ 0, 1, 2, 3 ] )

		return IECore.MeshPrimitive( nVerts, vertIds, "linear", p )

	def testAttribute( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.worldBegin()
		self.assertEqual( r.getAttribute( "rightHandedOrientation" ), IECore.BoolData( True ) )
		r.setAttribute( "rightHandedOrientation", IECore.BoolData( False ) )
		self.assertEqual( r.getAttribute( "rightHandedOrientation" ), IECore.BoolData( False ) )
		r.worldEnd()

	def testMesh( self ) :

		"""Check that anticlockwise winding order is considered front facing by default."""

		outputFileName = os.path.dirname( __file__ ) + "/output/testOrientation.tif"

		# render a single sided plane that shouldn't be backface culled
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.display( outputFileName, "tiff", "rgba", {} )
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 256 ) ) } )
		r.worldBegin()
		r.setAttribute( "doubleSided", IECore.BoolData( False ) )
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		self.makePlane().render( r )
		r.worldEnd()

		# check that something appears in the output image
		i = IECore.Reader.create( outputFileName ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )

		# render a plane that should be backface culled
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.display( outputFileName, "tiff", "rgba", {} )
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 256 ) ) } )
		r.worldBegin()
		r.setAttribute( "doubleSided", IECore.BoolData( False ) )
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		r.concatTransform( IECore.M44f.createRotated( IECore.V3f( 0, math.pi, 0 ) ) )
		self.makePlane().render( r )
		r.worldEnd()

		# check that nothing appears in the output image
		i = IECore.Reader.create( outputFileName ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 0 )

	def testFlippingTransforms( self ) :

		"""Check that flipping transforms are automatically compensated for."""

		outputFileName = os.path.dirname( __file__ ) + "/output/testOrientation.tif"

		# render a single sided plane that shouldn't be backface culled, even though
		# the negative transform has reversed the winding order
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.display( outputFileName, "tiff", "rgba", {} )
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 256 ) ) } )
		r.worldBegin()
		self.assertEqual( r.getAttribute( "rightHandedOrientation" ), IECore.BoolData( True ) )
		r.setAttribute( "doubleSided", IECore.BoolData( False ) )
		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
		r.concatTransform( IECore.M44f.createScaled( IECore.V3f( -1, 1, 1 ) ) )
		self.assertEqual( r.getAttribute( "rightHandedOrientation" ), IECore.BoolData( False ) )
		self.makePlane().render( r )
		r.worldEnd()

		# check that something appears in the output image
		i = IECore.Reader.create( outputFileName ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )

	def testFlippingTransformsAndTransformEnd( self ) :

		"""Check that flipped orientations are restored at transformEnd."""

		outputFileName = os.path.dirname( __file__ ) + "/output/testOrientation.tif"

		# render a single sided plane that shouldn't be backface culled, even though
		# the negative transform has reversed the winding order
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )
		r.display( outputFileName, "tiff", "rgba", {} )
		r.camera( "main", { "resolution" : IECore.V2iData( IECore.V2i( 256 ) ) } )
		r.worldBegin()
		if 1 :

			self.assertEqual( r.getAttribute( "rightHandedOrientation" ), IECore.BoolData( True ) )
			r.setAttribute( "doubleSided", IECore.BoolData( False ) )
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.transformBegin()
			if 1 :

				r.concatTransform( IECore.M44f.createScaled( IECore.V3f( -1, 1, 1 ) ) )
				self.assertEqual( r.getAttribute( "rightHandedOrientation" ), IECore.BoolData( False ) )
				self.makePlane().render( r )

			r.transformEnd() ## \todo We need to fix our transformEnd implementation.

			self.assertEqual( r.getAttribute( "rightHandedOrientation" ), IECore.BoolData( True ) )

		r.worldEnd()

		# check that something appears in the output image
		i = IECore.Reader.create( outputFileName ).read()
		e = IECore.PrimitiveEvaluator.create( i )
		result = e.createResult()
		a = e.A()
		e.pointAtUV( IECore.V2f( 0.5, 0.5 ), result )
		self.assertEqual( result.floatPrimVar( a ), 1 )

	def setUp( self ) :
		
		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )
	
	def tearDown( self ) :
		
		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
