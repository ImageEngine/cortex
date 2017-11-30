##########################################################################
#
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

import unittest

import IECore
import IECoreScene
import IECoreImage
import IECoreGL

IECoreGL.init( False )

class ToGLConverterTest( unittest.TestCase ) :

	def testFactory( self ) :

		# mesh

		m = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )

		c = IECoreGL.ToGLConverter.create( m )
		self.failUnless( isinstance( c, IECoreGL.ToGLMeshConverter ) )

		c = IECoreGL.ToGLConverter.create( m, IECoreGL.Primitive.staticTypeId() )
		self.failUnless( isinstance( c, IECoreGL.ToGLMeshConverter ) )

		c = IECoreGL.ToGLConverter.create( m, IECoreGL.Texture.staticTypeId() )
		self.assertEqual( c, None )

		# points

		p = IECoreScene.PointsPrimitive( 10 )

		c = IECoreGL.ToGLConverter.create( p )
		self.failUnless( isinstance( c, IECoreGL.ToGLPointsConverter ) )

		c = IECoreGL.ToGLConverter.create( p, IECoreGL.Primitive.staticTypeId() )
		self.failUnless( isinstance( c, IECoreGL.ToGLPointsConverter ) )

		c = IECoreGL.ToGLConverter.create( p, IECoreGL.Texture.staticTypeId() )
		self.assertEqual( c, None )

		# curves

		cv = IECoreScene.CurvesPrimitive()

		c = IECoreGL.ToGLConverter.create( cv )
		self.failUnless( isinstance( c, IECoreGL.ToGLCurvesConverter ) )

		c = IECoreGL.ToGLConverter.create( cv, IECoreGL.Primitive.staticTypeId() )
		self.failUnless( isinstance( c, IECoreGL.ToGLCurvesConverter ) )

		c = IECoreGL.ToGLConverter.create( cv, IECoreGL.Texture.staticTypeId() )
		self.assertEqual( c, None )

		# splines

		spline = IECore.SplinefColor3fData(
			IECore.SplinefColor3f(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, IECore.Color3f( 1 ) ),
					( 0, IECore.Color3f( 1 ) ),
					( 1, IECore.Color3f( 0 ) ),
					( 1, IECore.Color3f( 0 ) ),
				),
			),
		)

		c = IECoreGL.ToGLConverter.create( spline )
		self.failUnless( isinstance( c, IECoreGL.SplineToGLTextureConverter ) )

		spline = IECore.SplinefColor4fData(
			IECore.SplinefColor4f(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, IECore.Color4f( 1 ) ),
					( 0, IECore.Color4f( 1 ) ),
					( 1, IECore.Color4f( 0 ) ),
					( 1, IECore.Color4f( 0 ) ),
				),
			),
		)

		c = IECoreGL.ToGLConverter.create( spline )
		self.failUnless( isinstance( c, IECoreGL.SplineToGLTextureConverter ) )

		spline = IECore.SplineffData(
			IECore.Splineff(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, 1 ),
					( 0, 1 ),
					( 1, 0 ),
					( 1, 0 ),
				),
			),
		)

		c = IECoreGL.ToGLConverter.create( spline )
		self.failUnless( isinstance( c, IECoreGL.SplineToGLTextureConverter ) )

		# images

		image = IECoreImage.ImagePrimitive.createRGBFloat( IECore.Color3f( 1, 0, 0 ), IECore.Box2i( IECore.V2i( 256 ), ), IECore.Box2i( IECore.V2i( 256 ) ) )
		c = IECoreGL.ToGLConverter.create( image )
		self.failUnless( isinstance( c, IECoreGL.ToGLTextureConverter ) )

		# compound data

		compoundData = IECore.CompoundData( {
			"dataWindow" : IECore.Box2iData( image.dataWindow ),
			"displayWindow" : IECore.Box2iData( image.displayWindow ),
			"channels" : {
				"R" : image["R"],
				"G" : image["G"],
				"B" : image["B"],
			}
		} )

		c = IECoreGL.ToGLConverter.create( compoundData )
		self.failUnless( isinstance( c, IECoreGL.ToGLTextureConverter ) )

if __name__ == "__main__":
    unittest.main()
