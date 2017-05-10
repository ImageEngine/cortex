##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

class PointsAlgoTest( unittest.TestCase ) :

	def points( self ) :

		testObject = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( x ) for x in range( 0, 10 ) ] ) )

		testObject["a"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( 0.5 ) )
		testObject["b"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["c"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.FloatVectorData( [ 0 ] ) )
		testObject["d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( range( 0, 10 ) ) )
		testObject["e"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( range( 0, 10 ) ) )
		self.assertTrue( testObject.arePrimitiveVariablesValid() )

		return testObject

	def testPointsConstantToVertex( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsConstantToUniform( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] ) )

	def testPointsConstantToVarying( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsConstantToFaceVarying( self ) :
		points = self.points()
		p = points["a"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0.5 ] * 10 ) )

	def testPointsVertexToConstant( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsVertexToUniform( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsVertexToVarying( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsVertexToFaceVarying( self ) :
		points = self.points()
		p = points["b"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsUniformToConstant( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( 0 ) )

	def testPointsUniformToVertex( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsUniformToVarying( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsUniformToFaceVarying( self ) :
		points = self.points()
		p = points["c"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p.data, IECore.FloatVectorData( [ 0 ] * 10 ) )

	def testPointsVaryingToConstant( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsVaryingToVertex( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsVaryingToUniform( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsVaryingToFaceVarying( self ) :
		points = self.points()
		p = points["d"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		orig = range( 0, 9 )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsFaceVaryingToConstant( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Constant )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( p.data, IECore.FloatData( sum(range(0,10))/10. ) )

	def testPointsFaceVaryingToVertex( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Vertex )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )

	def testPointsFaceVaryingToUniform( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Uniform )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p.data, IECore.FloatVectorData( [ sum(range(0,10))/10. ] ) )

	def testPointsFaceVaryingToVarying( self ) :
		points = self.points()
		p = points["e"]
		IECore.PointsAlgo.resamplePrimitiveVariable(points, p, IECore.PrimitiveVariable.Interpolation.Varying )

		self.assertEqual( p.interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p.data, IECore.FloatVectorData( range( 0, 10 ) ) )


if __name__ == "__main__":
	unittest.main()
