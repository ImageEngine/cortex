##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

from IECore import *

class TestMeshPrimitive( unittest.TestCase ) :

	def test( self ) :

		m = MeshPrimitive()
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 0 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 0 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 0 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 0 )
		self.assertEqual( m.numFaces(), 0 )
		self.assertEqual( m.verticesPerFace, IntVectorData() )
		self.assertEqual( m.vertexIds, IntVectorData() )
		self.assertEqual( m.interpolation, "linear" )
		self.assertEqual( m, m.copy() )

		iface = IndexedIO.create( "test/IECore/mesh.fio", IndexedIO.OpenMode.Write )
		m.save( iface, "test" )
		mm = Object.load( iface, "test" )
		self.assertEqual( m, mm )

		vertsPerFace = IntVectorData( [ 3, 3 ] )
		vertexIds = IntVectorData( [ 0, 1, 2, 1, 2, 3 ] )

		m = MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 2 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 6 )
		self.assertEqual( m.numFaces(), 2 )
		self.assertEqual( m.verticesPerFace, vertsPerFace )
		self.assert_( not m.verticesPerFace.isSame( vertsPerFace ) )
		self.assertEqual( m.vertexIds, vertexIds )
		self.assert_( not m.vertexIds.isSame( vertexIds ) )
		self.assertEqual( m.interpolation, "catmullClark" )
		self.assertEqual( m, m.copy() )
		m.save( iface, "test" )
		mm = Object.load( iface, "test" )
		self.assertEqual( m, mm )

		m.setTopology( m.verticesPerFace, m.vertexIds, "catmullClark" )


		mm = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()

		self.assert_( mm.arePrimitiveVariablesValid() );

	def testSetInterpolation( self ) :

		m = MeshPrimitive()
		self.assertEqual( m.interpolation, "linear" )
		m.interpolation = "catmullClark"
		self.assertEqual( m.interpolation, "catmullClark" )

	def testEmptyMeshConstructor( self ) :

		m = MeshPrimitive( IntVectorData(), IntVectorData(), "linear", V3fVectorData() )
		self.assert_( m.arePrimitiveVariablesValid() )

	def testEqualityOfEmptyMeshes( self ) :
	
		self.assertEqual( MeshPrimitive(), MeshPrimitive() )
		
	def testHash( self ) :
	
		m = MeshPrimitive( IntVectorData(), IntVectorData(), "linear", V3fVectorData() )
		h = m.hash()
		
		m.setTopology( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 1, 2 ] ), "linear" )
		self.assertNotEqual( m.hash(), h )
		h = m.hash()
		
		m.setTopology( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 2, 1 ] ), "linear" )
		self.assertNotEqual( m.hash(), h )
		h = m.hash()
		
		m.setTopology( IntVectorData( [ 3 ] ), IntVectorData( [ 0, 2, 1 ] ), "catmullClark" )
		self.assertNotEqual( m.hash(), h )
		h = m.hash()
		
		m["primVar"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, IntData( 10 ) )
		self.assertNotEqual( m.hash(), h )
		
	def tearDown( self ) :

		if os.path.isfile("test/IECore/mesh.fio"):
			os.remove("test/IECore/mesh.fio")

if __name__ == "__main__":
    unittest.main()
