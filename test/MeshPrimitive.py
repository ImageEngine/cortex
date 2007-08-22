##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
		self.assertEqual( m.verticesPerFace, IntVectorData() )
		self.assertEqual( m.vertexIds, IntVectorData() )
		self.assertEqual( m.interpolation, "linear" )
		self.assertEqual( m, m.copy() )
		m.save( "test/mesh.sql" )
		mm = Object.load( "test/mesh.sql" )
		self.assertEqual( m, mm )
		
		vertsPerFace = IntVectorData( [ 3, 3 ] )
		vertexIds = IntVectorData( [ 0, 1, 2, 1, 2, 3 ] )
		
		m = MeshPrimitive( vertsPerFace, vertexIds, "catmullClark" )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Uniform ), 2 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( m.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 6 )
		self.assertEqual( m.verticesPerFace, vertsPerFace )
		self.assert_( not m.verticesPerFace.isSame( vertsPerFace ) )
		self.assertEqual( m.vertexIds, vertexIds )
		self.assert_( not m.vertexIds.isSame( vertexIds ) )
		self.assertEqual( m.interpolation, "catmullClark" )
		self.assertEqual( m, m.copy() )
		m.save( "test/mesh.sql" )
		mm = Object.load( "test/mesh.sql" )
		self.assertEqual( m, mm )
		
	def tearDown( self ) :
	
		if os.path.isfile("test/mesh.sql"):
			os.remove("test/mesh.sql")
		
if __name__ == "__main__":
    unittest.main()   
