##########################################################################
#
#  Copyright (p) 2009, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreRI

class PatchMeshTest( unittest.TestCase ) :

	outputFileName = "./test/IECoreRI/output/testPatchMesh.tif"

	def __performTest( self, curvesPrimitive, testImage ) :

		r = IECoreRI.Renderer( "" )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) ),
				"shutter" : IECore.V2fData( IECore.V2f( 0, 1 ) ),
			}
		)
		r.display( self.outputFileName, "tiff", "rgba", {} )
		r.setOption( "ri:pixelSamples", IECore.V2iData( IECore.V2i( 10 ) ) )
		r.worldBegin()

		r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

		curvesPrimitive.render( r )

		r.worldEnd()

		imageCreated = IECore.Reader.create( self.outputFileName ).read()
		expectedImage = IECore.Reader.create( testImage ).read()

		self.assertEqual( IECore.ImageDiffOp()( imageA=imageCreated, imageB=expectedImage, maxError=0.01 ), IECore.BoolData( False ) )

	def testLinearPeriodic( self ) :

		p = IECore.PatchMeshPrimitive(
			3,
			2,
			IECore.CubicBasisf.linear(),
			IECore.CubicBasisf.linear(),
			True,
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.2, 0.2, 0.0 ),
					IECore.V3f( 0.5, 0.2, 0.0 ),
					IECore.V3f( 0.8, 0.3, 0.5 ),
					IECore.V3f( 0.2, 0.8, 0.0 ),
					IECore.V3f( 0.5, 0.8, 0.0 ),
					IECore.V3f( 0.8, 0.9, 0.5 ),
				]
			)

		)

		self.__performTest( p, "./test/IECoreRI/data/patchMeshImages/linearPeriodic.tif" )

	def testLinear( self ) :

		p = IECore.PatchMeshPrimitive(
			2,
			2,
			IECore.CubicBasisf.linear(),
			IECore.CubicBasisf.linear(),
			False,
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.2, 0.2, 0 ),
					IECore.V3f( 0.2, 0.8, 0 ),
					IECore.V3f( 0.8, 0.2, 0 ),
					IECore.V3f( 0.8, 0.8, 0 ),
				]
			)

		)

		self.__performTest( p, "./test/IECoreRI/data/patchMeshImages/linear.tif" )

	def testBSplinePeriodic( self ) :

		p = IECore.PatchMeshPrimitive(
			4,
			4,
			IECore.CubicBasisf.bSpline(),
			IECore.CubicBasisf.bSpline(),
			True,
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.2, 0.2, 1.0 ),
					IECore.V3f( 0.3, 0.2, 0.0 ),
					IECore.V3f( 0.5, 0.2, 0.0 ),
					IECore.V3f( 0.8, 0.3, 1.5 ),

					IECore.V3f( 0.2, 0.3, 1.0 ),
					IECore.V3f( 0.3, 0.3, 0.0 ),
					IECore.V3f( 0.5, 0.3, 0.0 ),
					IECore.V3f( 0.8, 0.4, 1.5 ),

					IECore.V3f( 0.2, 0.5, 1.0 ),
					IECore.V3f( 0.3, 0.5, 0.0 ),
					IECore.V3f( 0.5, 0.5, 0.0 ),
					IECore.V3f( 0.8, 0.6, 1.5 ),

					IECore.V3f( 0.2, 0.8, 1.0 ),
					IECore.V3f( 0.3, 0.8, 0.0 ),
					IECore.V3f( 0.5, 0.8, 0.0 ),
					IECore.V3f( 0.8, 0.9, 1.5 ),
				]
			)

		)

		self.__performTest( p, "./test/IECoreRI/data/patchMeshImages/bSplinePeriodic.tif" )

	def testBSpline( self ) :

		p = IECore.PatchMeshPrimitive(
			4,
			4,
			IECore.CubicBasisf.bSpline(),
			IECore.CubicBasisf.bSpline(),
			False,
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.2, 0.2, 1.0 ),
					IECore.V3f( 0.3, 0.2, 0.0 ),
					IECore.V3f( 0.5, 0.2, 0.0 ),
					IECore.V3f( 0.8, 0.3, 1.5 ),

					IECore.V3f( 0.2, 0.3, 1.0 ),
					IECore.V3f( 0.3, 0.3, 0.0 ),
					IECore.V3f( 0.5, 0.3, 0.0 ),
					IECore.V3f( 0.8, 0.4, 1.5 ),

					IECore.V3f( 0.2, 0.5, 1.0 ),
					IECore.V3f( 0.3, 0.5, 0.0 ),
					IECore.V3f( 0.5, 0.5, 0.0 ),
					IECore.V3f( 0.8, 0.6, 1.5 ),

					IECore.V3f( 0.2, 0.8, 1.0 ),
					IECore.V3f( 0.3, 0.8, 0.0 ),
					IECore.V3f( 0.5, 0.8, 0.0 ),
					IECore.V3f( 0.8, 0.9, 1.5 ),
				]
			)

		)

		self.__performTest( p, "./test/IECoreRI/data/patchMeshImages/bSpline.tif" )

	def testBezier( self ) :

		p = IECore.PatchMeshPrimitive(
			4,
			4,
			IECore.CubicBasisf.bezier(),
			IECore.CubicBasisf.bezier(),
			False,
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.2, 0.2, 1.0 ),
					IECore.V3f( 0.3, 0.2, 0.0 ),
					IECore.V3f( 0.5, 0.2, 0.0 ),
					IECore.V3f( 0.8, 0.3, 1.5 ),

					IECore.V3f( 0.2, 0.3, 1.0 ),
					IECore.V3f( 0.3, 0.3, 0.0 ),
					IECore.V3f( 0.5, 0.3, 0.0 ),
					IECore.V3f( 0.8, 0.4, 1.5 ),

					IECore.V3f( 0.2, 0.5, 1.0 ),
					IECore.V3f( 0.3, 0.5, 0.0 ),
					IECore.V3f( 0.5, 0.5, 0.0 ),
					IECore.V3f( 0.8, 0.6, 1.5 ),

					IECore.V3f( 0.2, 0.8, 1.0 ),
					IECore.V3f( 0.3, 0.8, 0.0 ),
					IECore.V3f( 0.5, 0.8, 0.0 ),
					IECore.V3f( 0.8, 0.9, 1.5 ),
				]
			)

		)

		self.__performTest( p, "./test/IECoreRI/data/patchMeshImages/bezier.tif" )

	def testMotionBlur( self ) :

		p = IECore.PatchMeshPrimitive(
			4,
			4,
			IECore.CubicBasisf.bSpline(),
			IECore.CubicBasisf.bSpline(),
			True,
			False,
			IECore.V3fVectorData(
				[
					IECore.V3f( 0.2, 0.2, 1.0 ),
					IECore.V3f( 0.3, 0.2, 0.0 ),
					IECore.V3f( 0.5, 0.2, 0.0 ),
					IECore.V3f( 0.8, 0.3, 1.5 ),

					IECore.V3f( 0.2, 0.3, 1.0 ),
					IECore.V3f( 0.3, 0.3, 0.0 ),
					IECore.V3f( 0.5, 0.3, 0.0 ),
					IECore.V3f( 0.8, 0.4, 1.5 ),

					IECore.V3f( 0.2, 0.5, 1.0 ),
					IECore.V3f( 0.3, 0.5, 0.0 ),
					IECore.V3f( 0.5, 0.5, 0.0 ),
					IECore.V3f( 0.8, 0.6, 1.5 ),

					IECore.V3f( 0.2, 0.8, 1.0 ),
					IECore.V3f( 0.3, 0.8, 0.0 ),
					IECore.V3f( 0.5, 0.8, 0.0 ),
					IECore.V3f( 0.8, 0.9, 1.5 ),
				]
			)

		)

		p2 = IECore.TransformOp()( input=p, matrix=IECore.M44fData( IECore.M44f.createTranslated( IECore.V3f( -0.1, 0, 0 ) ) ) )

		m = IECore.MotionPrimitive()
		m[0] = p
		m[1] = p2

		self.__performTest( m, "./test/IECoreRI/data/patchMeshImages/motionBlur.tif" )

	def setUp( self ) :

		if os.path.exists( self.outputFileName ) :
			os.remove( self.outputFileName )

	def tearDown( self ) :

		if os.path.exists( self.outputFileName ) :
			os.remove( self.outputFileName )

if __name__ == "__main__":
    unittest.main()
