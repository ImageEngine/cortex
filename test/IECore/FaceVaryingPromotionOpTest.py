##########################################################################
#
#  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

class FaceVaryingPromotionOpTest( unittest.TestCase ) :

	__inputValues = {
		"Constant" : IECore.IntVectorData( [ 10 ] ),
		"Uniform" : IECore.IntVectorData( [ 1, 2 ] ),
		"Varying" : IECore.IntVectorData( [ 1, 2, 3, 4 ] ),
		"Vertex" : IECore.IntVectorData( [ 1, 2, 3, 4 ] ),
		"FaceVarying" : IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] ),
	}

	__indexValues = {
		"Constant" : IECore.IntVectorData( [ 0 ] ),
		"Uniform" : IECore.IntVectorData( [ 1, 0 ] ),
		"Varying" : IECore.IntVectorData( [ 3, 2, 1, 0 ] ),
		"Vertex" : IECore.IntVectorData( [ 3, 2, 1, 0 ] ),
		"FaceVarying" : IECore.IntVectorData( [ 5, 4, 3, 2, 1, 0 ] ),
	}

	__outputValues = {
		"Constant" : IECore.IntVectorData( [ 10 ] ),
		"Uniform" : IECore.IntVectorData( [ 1, 1, 1, 2, 2, 2 ] ),
		"Varying" : IECore.IntVectorData( [ 1, 2, 4, 1, 4, 3 ] ),
		"Vertex" : IECore.IntVectorData( [ 1, 2, 4, 1, 4, 3 ] ),
		"FaceVarying" : IECore.IntVectorData( [ 1, 2, 3, 4, 5, 6 ] ),
	}

	def __plane( self, indices = False ) :

		p = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		IECore.TriangulateOp()( input=p, copyInput=False )

		for n in self.__inputValues.keys() :

			i = getattr( IECore.PrimitiveVariable.Interpolation, n )
			if indices :
				p[n] = IECore.PrimitiveVariable( i, self.__inputValues[n], self.__indexValues[n] )
			else:
				p[n] = IECore.PrimitiveVariable( i, self.__inputValues[n] )

		return p

	def test( self ) :

		p = self.__plane()
		p2 = IECore.FaceVaryingPromotionOp()( input=p )
		self.failUnless( p2.arePrimitiveVariablesValid() )

		self.assertEqual( p2["Constant"], p["Constant"] )
		self.assertEqual( p2["Uniform"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Varying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Vertex"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["FaceVarying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Uniform"].data, self.__outputValues["Uniform"] )
		self.assertEqual( p2["Varying"].data, self.__outputValues["Varying"] )
		self.assertEqual( p2["Vertex"].data, self.__outputValues["Vertex"] )
		self.assertEqual( p2["FaceVarying"].data, self.__outputValues["FaceVarying"] )

		p = self.__plane()
		p2 = IECore.FaceVaryingPromotionOp()( input=p, promoteUniform=False )

		self.failUnless( p2.arePrimitiveVariablesValid() )

		self.assertEqual( p2["Constant"], p["Constant"] )
		self.assertEqual( p2["Uniform"].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p2["Varying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Vertex"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["FaceVarying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Uniform"].data, self.__inputValues["Uniform"] )
		self.assertEqual( p2["Varying"].data, self.__outputValues["Varying"] )
		self.assertEqual( p2["Vertex"].data, self.__outputValues["Vertex"] )
		self.assertEqual( p2["FaceVarying"].data, self.__outputValues["FaceVarying"] )

		p = self.__plane()
		p2 = IECore.FaceVaryingPromotionOp()( input=p, promoteVarying=False )

		self.failUnless( p2.arePrimitiveVariablesValid() )

		self.assertEqual( p2["Constant"], p["Constant"] )
		self.assertEqual( p2["Uniform"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Varying"].interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( p2["Vertex"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["FaceVarying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Uniform"].data, self.__outputValues["Uniform"] )
		self.assertEqual( p2["Varying"].data, self.__inputValues["Varying"] )
		self.assertEqual( p2["Vertex"].data, self.__outputValues["Vertex"] )
		self.assertEqual( p2["FaceVarying"].data, self.__outputValues["FaceVarying"] )

		p = self.__plane()
		p2 = IECore.FaceVaryingPromotionOp()( input=p, promoteVertex=False )

		self.failUnless( p2.arePrimitiveVariablesValid() )

		self.assertEqual( p2["Constant"], p["Constant"] )
		self.assertEqual( p2["Uniform"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Varying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Vertex"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p2["FaceVarying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Uniform"].data, self.__outputValues["Uniform"] )
		self.assertEqual( p2["Varying"].data, self.__outputValues["Varying"] )
		self.assertEqual( p2["Vertex"].data, self.__inputValues["Vertex"] )
		self.assertEqual( p2["FaceVarying"].data, self.__outputValues["FaceVarying"] )

		p = self.__plane()
		p2 = IECore.FaceVaryingPromotionOp()( input=p, primVarNames=IECore.StringVectorData( ["Varying"] ) )

		self.failUnless( p2.arePrimitiveVariablesValid() )

		self.assertEqual( p2["Constant"], p["Constant"] )
		self.assertEqual( p2["Uniform"].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( p2["Varying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Vertex"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p2["FaceVarying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Uniform"].data, self.__inputValues["Uniform"] )
		self.assertEqual( p2["Varying"].data, self.__outputValues["Varying"] )
		self.assertEqual( p2["Vertex"].data, self.__inputValues["Vertex"] )
		self.assertEqual( p2["FaceVarying"].data, self.__outputValues["FaceVarying"] )

	def testIndices( self ) :

		p = self.__plane( indices = True )
		p2 = IECore.FaceVaryingPromotionOp()( input=p )
		self.failUnless( p2.arePrimitiveVariablesValid() )

		self.assertEqual( p2["Constant"], p["Constant"] )
		self.assertEqual( p2["Uniform"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Varying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["Vertex"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( p2["FaceVarying"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		# data is unchanged
		self.assertEqual( p2["Uniform"].data, self.__inputValues["Uniform"] )
		self.assertEqual( p2["Varying"].data, self.__inputValues["Varying"] )
		self.assertEqual( p2["Vertex"].data, self.__inputValues["Vertex"] )
		self.assertEqual( p2["FaceVarying"].data, self.__inputValues["FaceVarying"] )

		# indices are changed
		self.assertEqual( p2["Uniform"].indices, IECore.IntVectorData( [ 1, 1, 1, 0, 0, 0 ] ) )
		self.assertEqual( p2["Varying"].indices, IECore.IntVectorData( [ 3, 2, 0, 3, 0, 1  ] ) )
		self.assertEqual( p2["Vertex"].indices, IECore.IntVectorData( [ 3, 2, 0, 3, 0, 1  ] ) )
		self.assertEqual( p2["FaceVarying"].indices, IECore.IntVectorData( [ 5, 4, 3, 2, 1, 0 ] ) )

if __name__ == "__main__":
    unittest.main()
