##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
import os.path
import math
import unittest
from IECore import *

class CurvesPrimitiveTest( unittest.TestCase ) :

	def testConstructors( self ) :
		
		c = CurvesPrimitive()
		self.assertEqual( c.verticesPerCurve(), IntVectorData() )
		self.assertEqual( c.basis(), CubicBasisf.linear() )
		self.assertEqual( c.periodic(), False )
		self.assertEqual( c.keys(), [] )
		
		c = CurvesPrimitive( IntVectorData( [ 2 ] ) )
		self.assertEqual( c.verticesPerCurve(), IntVectorData( [ 2 ] ) )
		self.assertEqual( c.basis(), CubicBasisf.linear() )
		self.assertEqual( c.periodic(), False )
		self.assertEqual( c.keys(), [] )
		
		c = CurvesPrimitive( IntVectorData( [ 4 ] ), CubicBasisf.bSpline() )
		self.assertEqual( c.verticesPerCurve(), IntVectorData( [ 4 ] ) )
		self.assertEqual( c.basis(), CubicBasisf.bSpline() )
		self.assertEqual( c.periodic(), False )
		self.assertEqual( c.keys(), [] )
		
		c = CurvesPrimitive( IntVectorData( [ 4 ] ), CubicBasisf.bSpline(), True )
		self.assertEqual( c.verticesPerCurve(), IntVectorData( [ 4 ] ) )
		self.assertEqual( c.basis(), CubicBasisf.bSpline() )
		self.assertEqual( c.periodic(), True )
		self.assertEqual( c.keys(), [] )
		
		i = IntVectorData( [ 4 ] )
		p = V3fVectorData( [ V3f( 0 ), V3f( 1 ), V3f( 2 ), V3f( 3 ) ] )
		c = CurvesPrimitive( i, CubicBasisf.bSpline(), True, p )
		self.assertEqual( c.verticesPerCurve(), IntVectorData( [ 4 ] ) )
		self.assertEqual( c.basis(), CubicBasisf.bSpline() )
		self.assertEqual( c.periodic(), True )
		self.assertEqual( c.keys(), [ "P" ] )
		self.assertEqual( c["P"].data, p )
		self.failIf( c["P"].data.isSame( p ) )
		self.failIf( c.verticesPerCurve().isSame( i ) )
		
	def testConstructorValidation( self ) :
	
		self.assertRaises( Exception, CurvesPrimitive, IntVectorData( [ 1 ] ) )
		self.assertRaises( Exception, CurvesPrimitive, IntVectorData( [ 3 ] ), CubicBasisf.bSpline() )
		self.assertRaises( Exception, CurvesPrimitive, IntVectorData( [ 5 ] ), CubicBasisf.bezier() )

	def testCopy( self ) :
	
		i = IntVectorData( [ 4 ] )
		p = V3fVectorData( [ V3f( 0 ), V3f( 1 ), V3f( 2 ), V3f( 3 ) ] )
		c = CurvesPrimitive( i, CubicBasisf.bSpline(), True, p )
		
		cc = c.copy()
		self.assertEqual( c, cc )

	def testIO( self ) :
	
		i = IntVectorData( [ 4 ] )
		p = V3fVectorData( [ V3f( 0 ), V3f( 1 ), V3f( 2 ), V3f( 3 ) ] )
		c = CurvesPrimitive( i, CubicBasisf.bSpline(), True, p )
		
		Writer.create( c, "test/IECore/data/curves.cob" ).write()
		
		cc = Reader.create( "test/IECore/data/curves.cob" ).read()
		
		self.assertEqual( cc, c )
		
		c = Reader.create( "test/IECore/data/cobFiles/torusCurves.cob" ).read()
	
	def testVariableSize( self ) :
	
		c = CurvesPrimitive( IntVectorData( [ 4 ] ), CubicBasisf.bSpline(), True )

		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Varying ), 4 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 4 )
		
		# asking for the constant size of a single curve makes no sense
		self.assertRaises( Exception, c.variableSize, PrimitiveVariable.Interpolation.Constant, 0 )
		# as does asking for the size of a nonexistent curve
		self.assertRaises( Exception, c.variableSize, PrimitiveVariable.Interpolation.Vertex, 1 )
		
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Uniform, 0 ), 1 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Vertex, 0 ), 4 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Varying, 0 ), 4 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.FaceVarying, 0 ), 4 )
		self.assertEqual( c.numSegments( 0 ), 4 )
	
		c = CurvesPrimitive( IntVectorData( [ 4 ] ), CubicBasisf.bSpline(), False )

		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Constant ), 1 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Vertex ), 4 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Varying ), 2 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.FaceVarying ), 2 )
		
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Uniform, 0 ), 1 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Vertex, 0 ), 4 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.Varying, 0 ), 2 )
		self.assertEqual( c.variableSize( PrimitiveVariable.Interpolation.FaceVarying, 0 ), 2 )
		self.assertEqual( c.numSegments( 0 ), 1 )
		
	def tearDown( self ) :
	
		if os.path.isfile( "test/IECore/data/curves.cob" ) :
			os.remove( "test/IECore/data/curves.cob" )

if __name__ == "__main__":
    unittest.main()   

