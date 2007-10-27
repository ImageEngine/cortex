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

class TestNURBSPrimitive( unittest.TestCase ) :

	def test( self ) :
	
		n = NURBSPrimitive()
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Vertex ), 16 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 4 )
		ObjectWriter( n, "test/IECore/nurbs.fio" ).write()
		nn = ObjectReader( "test/IECore/nurbs.fio" ).read()
		self.assertEqual( n, nn )
			
		n = NURBSPrimitive( 3, FloatVectorData( [ 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4 ] ), 0, 4,
			2, FloatVectorData( [ 0, 0, 1, 1 ] ), 0, 1 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Uniform ), 7 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Vertex ), 18 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.Varying ), 16 )
		self.assertEqual( n.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 16 )
		self.assertEqual( n.uOrder(), 3 )
		self.assertEqual( n.uKnot(), FloatVectorData( [ 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4 ] ) )
		self.assertEqual( n.uMin(), 0 )
		self.assertEqual( n.uMax(), 4 )
		self.assertEqual( n.vOrder(), 2 )
		self.assertEqual( n.vKnot(), FloatVectorData( [ 0, 0, 1, 1 ] ) )
		self.assertEqual( n.vMin(), 0 )
		self.assertEqual( n.vMax(), 1 )
		ObjectWriter( n, "test/IECore/nurbs.fio" ).write()
		nn = ObjectReader( "test/IECore/nurbs.fio" ).read()
		self.assertEqual( n, nn )
		nnn = nn.copy()
		self.assertEqual( nnn, n )		
		
	def tearDown( self ) :
	
		if os.path.isfile("test/IECore/nurbs.fio"):
			os.remove("test/IECore/nurbs.fio")
		
if __name__ == "__main__":
    unittest.main()   
