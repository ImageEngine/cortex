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

import os
import unittest
import random

import arnold

import IECore
import IECoreArnold

class PointsTest( unittest.TestCase ) :

	def testConverterResultType( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			p = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( i ) for i in range( 0, 10 ) ] ) )
			n = IECoreArnold.NodeAlgo.convert( p )

			self.failUnless( type( n ) is type( arnold.AiNode( "points" ) ) )

	def testMode( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			p = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( i ) for i in range( 0, 10 ) ] ) )

			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "disk" )

			p["type"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, "particle" )
			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "disk" )

			p["type"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, "disk" )
			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "disk" )

			p["type"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, "sphere" )
			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "sphere" )

			p["type"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, "patch" )
			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "quad" )

	def testDiskRendering( self ) :

		numPoints = 10
		p = IECore.V3fVectorData( numPoints )
		random.seed( 0 )
		for i in range( 0, numPoints ) :
			p[i] = IECore.V3f( random.random() * 4, random.random() * 4, random.random() * 4 )
		p = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, p )

		r = IECoreArnold.Renderer()
		r.setOption( "ai:AA_samples", IECore.IntData( 9 ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )
			}
		)
		r.display( "test", "ieDisplay", "rgba", { "driverType" : "ImageDisplayDriver", "handle" : "testHandle" } )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -2, -2, -10 ) ) )
			r.points( numPoints, { "P" : p } )

		image = IECore.ImageDisplayDriver.removeStoredImage( "testHandle" )
		del image["A"]

		expectedImage = IECore.Reader.create( os.path.dirname( __file__ ) + "/data/pointsImages/points.tif" ).read()

		IECore.ImageWriter.create( image, "/tmp/test.tif" ).write()

		self.assertEqual( IECore.ImageDiffOp()( imageA=image, imageB=expectedImage, maxError=0.01 ), IECore.BoolData( False ) )

	def testConstantPrimitiveVariable( self ) :

		p = IECore.PointsPrimitive( IECore.V3fVectorData( 10 ) )
		p["myPrimVar"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 10 ) )

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetInt( n, "myPrimVar" ), 10 )

	def testConstantArrayPrimitiveVariable( self ) :

		p = IECore.PointsPrimitive( IECore.V3fVectorData( 10 ) )
		p["myPrimVar"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.IntVectorData( range( 0, 10 ) ) )

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert( p )
			a = arnold.AiNodeGetArray( n, "myPrimVar" )
			self.assertEqual( arnold.AiArrayGetNumElements( a.contents ), 10 )
			for i in range( 0, 10 ) :
				self.assertEqual( arnold.AiArrayGetInt( a, i ), i )

	def testUniformPrimitiveVariable( self ) :

		p = IECore.PointsPrimitive( IECore.V3fVectorData( 10 ) )
		p["myPrimVar"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.IntData( 10 ) )

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetInt( n, "myPrimVar" ), 10 )

	def testVertexPrimitiveVariable( self ) :

		for interpolation in ( "Vertex", "Varying", "FaceVarying" ) :

			p = IECore.PointsPrimitive( IECore.V3fVectorData( 10 ) )
			p["myPrimVar"] = IECore.PrimitiveVariable( getattr( IECore.PrimitiveVariable.Interpolation, interpolation ), IECore.IntVectorData( range( 0, 10 ) ) )

			self.assertTrue( p.arePrimitiveVariablesValid() )

			with IECoreArnold.UniverseBlock( writable = True ) :

				n = IECoreArnold.NodeAlgo.convert( p )
				a = arnold.AiNodeGetArray( n, "myPrimVar" )
				self.assertEqual( arnold.AiArrayGetNumElements( a.contents ), 10 )
				for i in range( 0, 10 ) :
					self.assertEqual( arnold.AiArrayGetInt( a, i ), i )

	def testBooleanPrimitiveVariable( self ) :

		p = IECore.PointsPrimitive( IECore.V3fVectorData( 10 ) )
		p["truePrimVar"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.BoolData( True ) )
		p["falsePrimVar"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.BoolData( False ) )

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert( p )
			self.assertEqual( arnold.AiNodeGetBool( n, "truePrimVar" ), True )
			self.assertEqual( arnold.AiNodeGetBool( n, "falsePrimVar" ), False )

	def testMotion( self ) :

		p1 = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( 10 ) ] * 10 ) )
		p1["width"] = IECore.PrimitiveVariable(
			IECore.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( [ 1 ] * 10 ),
		)

		p2 = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( 20 ) ] * 10 ) )
		p2["width"] = IECore.PrimitiveVariable(
			IECore.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( [ 2 ] * 10 ),
		)

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert( [ p1, p2 ], -0.25, 0.25 )

			a = arnold.AiNodeGetArray( n, "points" )
			self.assertEqual( arnold.AiArrayGetNumElements( a.contents ), 10 )
			self.assertEqual( arnold.AiArrayGetNumKeys( a.contents ), 2 )

			r = arnold.AiNodeGetArray( n, "radius" )
			self.assertEqual( arnold.AiArrayGetNumElements( a.contents ), 10 )
			self.assertEqual( arnold.AiArrayGetNumKeys( a.contents ), 2 )

			for i in range( 0, 10 ) :
				self.assertEqual( arnold.AiArrayGetVec( a, i ), arnold.AtVector( 10 ) )
				self.assertEqual( arnold.AiArrayGetFlt( r, i ), 0.5 )
			for i in range( 11, 20 ) :
				self.assertEqual( arnold.AiArrayGetVec( a, i ), arnold.AtVector( 20 ) )
				self.assertEqual( arnold.AiArrayGetFlt( r, i ), 1 )

			self.assertEqual( arnold.AiNodeGetFlt( n, "motion_start" ), -0.25 )
			self.assertEqual( arnold.AiNodeGetFlt( n, "motion_end" ), 0.25 )

if __name__ == "__main__":
    unittest.main()
