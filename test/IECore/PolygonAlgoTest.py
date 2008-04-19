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

import unittest
from IECore import *
import math

class PolygonAlgoTest( unittest.TestCase ) :

	def testNormal( self ) :
	
		p = V3fVectorData( [
			V3f( 0, 0, 0 ),
			V3f( 1, 0, 0 ),
			V3f( 1, 1, 0 ),
			V3f( 0, 1, 0 )
		] )
				
		self.assertEqual( polygonNormal( p ), V3f( 0, 0, 1 ) )
		
		p = V3fVectorData( [
			V3f( 0, 0, 0 ),
			V3f( 0, 1, 0 ),
			V3f( 1, 1, 0 ),
			V3f( 1, 0, 0 ),
		] )
		
		self.assertEqual( polygonNormal( p ), V3f( 0, 0, -1 ) )
	
	def testConcaveNormal( self ) :
		
		p = V3fVectorData( [
			V3f( 0, 0, 0 ),
			V3f( 1, -1, 0 ),
			V3f( 0.2, 0, 0 ),
			V3f( 1, 1, 0 ),
		] )
		
		self.assertEqual( polygonNormal( p ), V3f( 0, 0, 1 ) )

		p = V3fVectorData( [
			V3f( 0, 0, 0 ),
			V3f( 1, 1, 0 ),
			V3f( 0.2, 0, 0 ),
			V3f( 1, -1, 0 ),
		] )
		
		self.assertEqual( polygonNormal( p ), V3f( 0, 0, -1 ) )
	
	def testWinding2D( self ) :
	
		p = V2fVectorData( [
			V2f( 0, 0 ),
			V2f( 1, 0 ),
			V2f( 1, 1 ),
			V2f( 0, 1 ),
		] )
		
		self.assertEqual( polygonWinding( p ), Winding.CounterClockwise )
		self.assertNotEqual( polygonWinding( p ), Winding.Clockwise )

		p = V2fVectorData( [
			V2f( 0, 0 ),
			V2f( 0, 1 ),
			V2f( 1, 1 ),
			V2f( 1, 0 ),
		] )
		
		self.assertNotEqual( polygonWinding( p ), Winding.CounterClockwise )
		self.assertEqual( polygonWinding( p ), Winding.Clockwise )

	def testWinding3D( self ) :
	
		p = V3fVectorData( [
			V3f( 0, 0, 0 ),
			V3f( 1, 0, 0 ),
			V3f( 1, 1, 0 ),
			V3f( 0, 1, 0 ),
		] )
		
		self.assertEqual( polygonWinding( p, V3f( 0, 0, -1 ) ), Winding.CounterClockwise )
		self.assertNotEqual( polygonWinding( p, V3f( 0, 0, -1 ) ), Winding.Clockwise )
		self.assertEqual( polygonWinding( p, V3f( 0, 0, 1 ) ), Winding.Clockwise )
		self.assertNotEqual( polygonWinding( p, V3f( 0, 0, 1 ) ), Winding.CounterClockwise )

		p = V3fVectorData( [
			V3f( 0, 0, 0 ),
			V3f( 0, 1, 0 ),
			V3f( 1, 1, 0 ),
			V3f( 1, 0, 0 ),
		] )
		
		self.assertNotEqual( polygonWinding( p, V3f( 0, 0, -1 ) ), Winding.CounterClockwise )
		self.assertEqual( polygonWinding( p, V3f( 0, 0, -1 ) ), Winding.Clockwise )
		self.assertEqual( polygonWinding( p, V3f( 0, 0, 1 ) ), Winding.CounterClockwise )
		self.assertNotEqual( polygonWinding( p, V3f( 0, 0, 1 ) ), Winding.Clockwise )
	
	def testBound( self ) :
			
		p = V3fVectorData( [
			V3f( 0, 0, 0 ),
			V3f( 1, 0, 0 ),
			V3f( 1, 1, 0 ),
			V3f( 0, 1, 0 ),
		] )
		
		self.assertEqual( polygonBound( p ), Box3f( V3f( 0 ), V3f( 1, 1, 0 ) ) )
		
if __name__ == "__main__":
    unittest.main()   
