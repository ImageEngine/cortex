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

import unittest

from IECore import *

class TestPointsPrimitive( unittest.TestCase ) :

	def testPrimitiveVariable( self ) :

		v = PrimitiveVariable( PrimitiveVariable.Interpolation.Constant, FloatData( 1 ) )
		self.assertEqual( v.interpolation, PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( v.data, FloatData( 1 ) )

		v.interpolation = PrimitiveVariable.Interpolation.Vertex
		self.assertEqual( v.interpolation, PrimitiveVariable.Interpolation.Vertex )
		v.data = IntVectorData( [ 1, 2, 3, 4 ] )
		self.assertEqual( v.data, IntVectorData( [ 1, 2, 3, 4 ] ) )

	def testPrimitive( self ) :

		"""This test mainly tests the Primitive aspects of the PointPrimitive"""

		p = PointsPrimitive( 10 )

		self.assertEqual( p.numPoints, 10 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Varying ), 10 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Vertex ), 10 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 10 )

		self.assertEqual( p, p )
		self.assertEqual( p, p.copy() )

		# try adding a primvar
		self.assertEqual( len( p ), 0 )
		self.assert_( not "P" in p )
		p["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData() )
		self.assertEqual( p, p )
		self.assertEqual( p, p.copy() )
		self.assertEqual( len( p ), 1 )
		self.assert_( "P" in p )
		self.assertEqual( p["P"].data, V3fVectorData() )

		# and removing it
		self.assertEqual( p["P"].interpolation, PrimitiveVariable.Interpolation.Vertex )
		del p["P"]
		self.assertEqual( len( p ), 0 )
		self.assert_( not "P" in p )

		# and adding it and another
		p["P"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData() )
		self.assert_( not "N" in p )
		p["N"] = PrimitiveVariable( PrimitiveVariable.Interpolation.Vertex, V3fVectorData() )
		self.assert_( "N" in p )
		self.assertEqual( len( p ), 2 )
		self.assertEqual( p["N"].data, V3fVectorData() )
		self.assertEqual( p["N"].interpolation, PrimitiveVariable.Interpolation.Vertex )

		# and overwriting one with the other
		p["N"] = p["P"]
		self.assert_( p["N"].data.isSame( p["P"].data ) )

	def testConstructors( self ) :

		p = PointsPrimitive( 20 )
		self.assertEqual( p.numPoints, 20 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Varying ), 20 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Vertex ), 20 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 20 )
		self.assertEqual( len( p ), 0 )

		p = PointsPrimitive( V3fVectorData( [ V3f( 1 ) ] ) )
		self.assertEqual( p.numPoints, 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Varying ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Vertex ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 1 )
		self.assertEqual( len( p ), 1 )
		self.assert_( "P" in p )
		self.assertEqual( p["P"].data, V3fVectorData( [ V3f( 1 ) ] ) )
		self.assertEqual( p["P"].interpolation, PrimitiveVariable.Interpolation.Vertex )

		p = PointsPrimitive( V3fVectorData( [ V3f( 1 ) ] ), FloatVectorData( [ 1 ] ) )
		self.assertEqual( p.numPoints, 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Varying ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.Vertex ), 1 )
		self.assertEqual( p.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 1 )
		self.assertEqual( len( p ), 2 )
		self.assert_( "P" in p )
		self.assert_( "r" in p )
		self.assertEqual( p["P"].data, V3fVectorData( [ V3f( 1 ) ] ) )
		self.assertEqual( p["P"].interpolation, PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( p["r"].data, FloatVectorData( [ 1 ] ) )
		self.assertEqual( p["r"].interpolation, PrimitiveVariable.Interpolation.Vertex )

	def testNumPointsAccess( self ) :

		p = PointsPrimitive( 20 )
		self.assertEqual( p.numPoints, 20 )
		p.numPoints = 40
		self.assertEqual( p.numPoints, 40 )

if __name__ == "__main__":
    unittest.main()
