##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
import math

class MeshTangentsOpTest( unittest.TestCase ) :
	
	def testSimpleTriangleWithNoIndices( self ) :
	
		verticesPerFace = IECore.IntVectorData( [ 3 ] )
		vertexIds = IECore.IntVectorData( [ 0, 1, 2 ] )
		p = IECore.V3fVectorData( [ IECore.V3f( 0, 0, 0 ), IECore.V3f( 1, 0, 0 ), IECore.V3f( 0, 1, 0 ) ] )
		s = IECore.FloatVectorData( [ 0, 1, 0 ] )
		t = IECore.FloatVectorData( [ 0, 0, 1 ] )
		
		mesh = IECore.MeshPrimitive( verticesPerFace, vertexIds, "linear", p )
		mesh["s"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, s )
		mesh["t"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, t )
	
		mesh = IECore.MeshTangentsOp() (
			input = mesh,
			uPrimVarName = "s",
			vPrimVarName = "t",
			uTangentPrimVarName = "sTangent",
			vTangentPrimVarName = "tTangent",
			uvIndicesPrimVarName = ""
		)
		
		self.assert_( "sTangent" in mesh )
		self.assert_( "tTangent" in mesh )
		self.assert_( mesh.arePrimitiveVariablesValid() )
		
		self.assertEqual( mesh["sTangent"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["tTangent"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		
		for v in mesh["sTangent"].data :
			self.failUnless( v.equalWithAbsError( IECore.V3f( 1, 0, 0 ), 0.000001 ) )
		for v in mesh["tTangent"].data :
			self.failUnless( v.equalWithAbsError( IECore.V3f( 0, 1, 0 ), 0.000001 ) )
					
	def testJoinedUVEdges( self ) :
			
		mesh = IECore.ObjectReader( "test/IECore/data/cobFiles/twoTrianglesWithSharedUVs.cob" ).read()
		self.assert_( mesh.arePrimitiveVariablesValid() )
		
		mesh = IECore.MeshTangentsOp() (
			input = mesh,
			uPrimVarName = "s",
			vPrimVarName = "t",
			uTangentPrimVarName = "sTangent",
			vTangentPrimVarName = "tTangent",
		)
		
		self.assert_( not "uTangent" in mesh )
		self.assert_( not "vTangent" in mesh )
		self.assert_( "sTangent" in mesh )
		self.assert_( "tTangent" in mesh )
		self.assert_( mesh.arePrimitiveVariablesValid() )
		
		self.assertEqual( mesh["sTangent"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["tTangent"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		
		for v in mesh["sTangent"].data :
			self.failUnless( v.equalWithAbsError( IECore.V3f( 1, 0, 0 ), 0.000001 ) )
		for v in mesh["tTangent"].data :
			self.failUnless( v.equalWithAbsError( IECore.V3f( 0, 0, -1 ), 0.000001 ) )	
			
	def testSplitAndOpposedUVEdges( self ) :
	
		mesh = IECore.ObjectReader( "test/IECore/data/cobFiles/twoTrianglesWithSplitAndOpposedUVs.cob" ).read()
	
		mesh = IECore.MeshTangentsOp() (
			input = mesh,
			uPrimVarName = "s",
			vPrimVarName = "t",
			uTangentPrimVarName = "sTangent",
			vTangentPrimVarName = "tTangent",
		)
		
		self.assert_( not "uTangent" in mesh )
		self.assert_( not "vTangent" in mesh )
		self.assert_( "sTangent" in mesh )
		self.assert_( "tTangent" in mesh )
		self.assert_( mesh.arePrimitiveVariablesValid() )
		
		self.assertEqual( mesh["sTangent"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["tTangent"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )

		for v in mesh["sTangent"].data[:3] :
			self.failUnless( v.equalWithAbsError( IECore.V3f( -1, 0, 0 ), 0.000001 ) )
		for v in mesh["sTangent"].data[3:] :
			self.failUnless( v.equalWithAbsError( IECore.V3f( 1, 0, 0 ), 0.000001 ) )
			
		for v in mesh["tTangent"].data[:3] :
			self.failUnless( v.equalWithAbsError( IECore.V3f( 0, 0, 1 ), 0.000001 ) )
		for v in mesh["tTangent"].data[3:] :
			self.failUnless( v.equalWithAbsError( IECore.V3f( 0, 0, -1 ), 0.000001 ) )
		
if __name__ == "__main__":
	unittest.main()
