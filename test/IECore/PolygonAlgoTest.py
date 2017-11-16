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

class PolygonAlgoTest( unittest.TestCase ) :

	def testNormal( self ) :

		p = IECore.V3fVectorData( [
			IECore.V3f( 0, 0, 0 ),
			IECore.V3f( 1, 0, 0 ),
			IECore.V3f( 1, 1, 0 ),
			IECore.V3f( 0, 1, 0 )
		] )

		self.assertEqual( IECore.polygonNormal( p ), IECore.V3f( 0, 0, 1 ) )

		p = IECore.V3fVectorData( [
			IECore.V3f( 0, 0, 0 ),
			IECore.V3f( 0, 1, 0 ),
			IECore.V3f( 1, 1, 0 ),
			IECore.V3f( 1, 0, 0 ),
		] )

		self.assertEqual( IECore.polygonNormal( p ), IECore.V3f( 0, 0, -1 ) )

	def testConcaveNormal( self ) :

		p = IECore.V3fVectorData( [
			IECore.V3f( 0, 0, 0 ),
			IECore.V3f( 1, -1, 0 ),
			IECore.V3f( 0.2, 0, 0 ),
			IECore.V3f( 1, 1, 0 ),
		] )

		self.assertEqual( IECore.polygonNormal( p ), IECore.V3f( 0, 0, 1 ) )

		p = IECore.V3fVectorData( [
			IECore.V3f( 0, 0, 0 ),
			IECore.V3f( 1, 1, 0 ),
			IECore.V3f( 0.2, 0, 0 ),
			IECore.V3f( 1, -1, 0 ),
		] )

		self.assertEqual( IECore.polygonNormal( p ), IECore.V3f( 0, 0, -1 ) )

	def testWinding2D( self ) :

		p = IECore.V2fVectorData( [
			IECore.V2f( 0, 0 ),
			IECore.V2f( 1, 0 ),
			IECore.V2f( 1, 1 ),
			IECore.V2f( 0, 1 ),
		] )

		self.assertEqual( IECore.polygonWinding( p ), IECore.Winding.CounterClockwise )
		self.assertNotEqual( IECore.polygonWinding( p ), IECore.Winding.Clockwise )

		p = IECore.V2fVectorData( [
			IECore.V2f( 0, 0 ),
			IECore.V2f( 0, 1 ),
			IECore.V2f( 1, 1 ),
			IECore.V2f( 1, 0 ),
		] )

		self.assertNotEqual( IECore.polygonWinding( p ), IECore.Winding.CounterClockwise )
		self.assertEqual( IECore.polygonWinding( p ), IECore.Winding.Clockwise )

	def testWinding3D( self ) :

		p = IECore.V3fVectorData( [
			IECore.V3f( 0, 0, 0 ),
			IECore.V3f( 1, 0, 0 ),
			IECore.V3f( 1, 1, 0 ),
			IECore.V3f( 0, 1, 0 ),
		] )

		self.assertEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, -1 ) ), IECore.Winding.CounterClockwise )
		self.assertNotEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, -1 ) ), IECore.Winding.Clockwise )
		self.assertEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, 1 ) ), IECore.Winding.Clockwise )
		self.assertNotEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, 1 ) ), IECore.Winding.CounterClockwise )

		p = IECore.V3fVectorData( [
			IECore.V3f( 0, 0, 0 ),
			IECore.V3f( 0, 1, 0 ),
			IECore.V3f( 1, 1, 0 ),
			IECore.V3f( 1, 0, 0 ),
		] )

		self.assertNotEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, -1 ) ), IECore.Winding.CounterClockwise )
		self.assertEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, -1 ) ), IECore.Winding.Clockwise )
		self.assertEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, 1 ) ), IECore.Winding.CounterClockwise )
		self.assertNotEqual( IECore.polygonWinding( p, IECore.V3f( 0, 0, 1 ) ), IECore.Winding.Clockwise )

	def testBound( self ) :

		p = IECore.V3fVectorData( [
			IECore.V3f( 0, 0, 0 ),
			IECore.V3f( 1, 0, 0 ),
			IECore.V3f( 1, 1, 0 ),
			IECore.V3f( 0, 1, 0 ),
		] )

		self.assertEqual( IECore.polygonBound( p ), IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1, 1, 0 ) ) )

	def testArea3D( self ) :

		r = IECore.Rand32()
		for i in range( 0, 1000 ) :

			p = IECore.V3fVectorData( [ r.nextV3f(), r.nextV3f(), r.nextV3f() ] )
			self.assertAlmostEqual( IECore.polygonArea( p ), IECore.triangleArea( p[0], p[1], p[2] ), 4 )

if __name__ == "__main__":
    unittest.main()
