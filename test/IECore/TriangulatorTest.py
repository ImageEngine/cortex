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

class TriangulatorTest( unittest.TestCase ) :

	def testV3f( self ) :
	
		#	 _   _ 
		#	| |_| |
		#	|_    |
		#	 _|   |
		#	|____| 
		
		p = V3fVectorData(
			[
				V3f( 0, 0, 0 ),
				V3f( 3, 0, 0 ),
				V3f( 3, 4, 0 ),
				V3f( 2, 4, 0 ),
				V3f( 2, 3, 0 ),
				V3f( 1, 3, 0 ),
				V3f( 1, 4, 0 ),
				V3f( 0, 4, 0 ),
				V3f( 0, 2, 0 ),
				V3f( 1, 2, 0 ),
				V3f( 1, 1, 0 ),
				V3f( 0, 1, 0 )
			]
		)
		
		inMesh = MeshPrimitive( IntVectorData( [ 12 ] ), IntVectorData( range( 0, 12 ) ), "linear", p )
		
		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( p )
				
		outMesh = builder.mesh()

		self.assertEqual( p, outMesh["P"].data )
		self.assertEqual( len( outMesh.verticesPerFace ), len( p ) - 2 )
		for x in outMesh.verticesPerFace :
			self.assertEqual( x, 3 )
		self.assertEqual( outMesh.variableSize( PrimitiveVariable.Interpolation.Vertex ), 12 )

		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 10 )
		
	def testOneHole( self ) :
	
		#  ______
		# |  __ |
		# | |_| |
		# |_____|
		#
		
		outer = V3fVectorData(
			[
				V3f( 0, 0, 0 ),
				V3f( 3, 0, 0 ),
				V3f( 3, 3, 0 ),
				V3f( 0, 3, 0 ),
			]
		)

		inner = V3fVectorData(
			[
				V3f( 1, 1, 0 ),
				V3f( 1, 2, 0 ),
				V3f( 2, 2, 0 ),
				V3f( 2, 1, 0 ),
			]
		)		

		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( [ outer, inner ] )

		outMesh = builder.mesh()
		
		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 8 )

	def testTwoHoles( self ) :
		
		#  __________
		# |  __  __  |
		# | |_| |_|  |
		# |__________|
		#
		
		outer = V3fVectorData(
			[
				V3f( 0, 0, 0 ),
				V3f( 5, 0, 0 ),
				V3f( 5, 3, 0 ),
				V3f( 0, 3, 0 ),
			]
		)

		inner1 = V3fVectorData(
			[
				V3f( 1, 1, 0 ),
				V3f( 1, 2, 0 ),
				V3f( 2, 2, 0 ),
				V3f( 2, 1, 0 ),
			]
		)		

		inner2 = V3fVectorData(
			[
				V3f( 3, 1, 0 ),
				V3f( 3, 2, 0 ),
				V3f( 4, 2, 0 ),
				V3f( 4, 1, 0 ),
			]
		)
		
		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( [ outer, inner1, inner2 ] )

		outMesh = builder.mesh()
		
		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 13 )
		
	def testBigCircle( self ) :
	
		numPoints = 10000
		loop = V3fVectorData( numPoints )
		
		for i in range( 0, numPoints ) :
		
			t = i * math.pi * 2 / numPoints
			loop[i] = V3f( math.cos( t ), math.sin( t ), 0 )
			
		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( loop )
		
	def testMultipleCalls( self ) :
	
		#  __   __
		# |_|  |_|
		#
		
		outline1 = V3fVectorData(
			[
				V3f( 0, 0, 0 ),
				V3f( 1, 0, 0 ),
				V3f( 1, 1, 0 ),
				V3f( 0, 1, 0 ),
			]
		)
		
		outline2 = V3fVectorData(
			[
				V3f( 3, 0, 0 ),
				V3f( 4, 0, 0 ),
				V3f( 4, 1, 0 ),
				V3f( 3, 1, 0 ),
			]
		)
		
		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( outline1 )
		triangulator.triangulate( outline2 )

		outMesh = builder.mesh()
				
		self.assertEqual( outMesh["P"].data.size(), 8 )
		self.assertEqual( outMesh.verticesPerFace, IntVectorData( [ 3, 3, 3, 3 ] ) )
		self.assertEqual( outMesh.variableSize( PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( outMesh.bound(), Box3f( V3f( 0 ), V3f( 4, 1, 0 ) ) )
		
		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 2 )
		
	def testMultipleCallsWithHoles( self ) :
	
		#  ______   ______
		# |  __ |  |  __ |
		# | |_| |  | |_| |
		# |_____|  |_____|
		#
		
		outer1 = V3fVectorData(
			[
				V3f( 0, 0, 0 ),
				V3f( 3, 0, 0 ),
				V3f( 3, 3, 0 ),
				V3f( 0, 3, 0 ),
			]
		)

		inner1 = V3fVectorData(
			[
				V3f( 1, 1, 0 ),
				V3f( 1, 2, 0 ),
				V3f( 2, 2, 0 ),
				V3f( 2, 1, 0 ),
			]
		)		

		outer2 = V3fVectorData(
			[
				V3f( 4, 0, 0 ),
				V3f( 7, 0, 0 ),
				V3f( 7, 3, 0 ),
				V3f( 4, 3, 0 ),
			]
		)

		inner2 = V3fVectorData(
			[
				V3f( 5, 1, 0 ),
				V3f( 5, 2, 0 ),
				V3f( 6, 2, 0 ),
				V3f( 6, 1, 0 ),
			]
		)		

		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( [ outer1, inner1 ] )
		triangulator.triangulate( [ outer2, inner2 ] )

		outMesh = builder.mesh()
		
		self.assertEqual( outMesh["P"].data.size(), 16 )
		self.assertEqual( outMesh.variableSize( PrimitiveVariable.Interpolation.Vertex ), 16 )
		self.assertEqual( outMesh.bound(), Box3f( V3f( 0 ), V3f( 7, 3, 0 ) ) )
		
		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 16 )	
		
	def testRightAlignedHoles( self ) :
		
		#  ______
		# |  __ |
		# | |_| |
		# |  __ |
		# | |_| |
		# |_____|
		
		outer = V3fVectorData(
			[
				V3f( 0, 0, 0 ),
				V3f( 3, 0, 0 ),
				V3f( 3, 5, 0 ),
				V3f( 0, 5, 0 ),
			]
		)

		inner1 = V3fVectorData(
			[
				V3f( 1, 1, 0 ),
				V3f( 1, 2, 0 ),
				V3f( 2, 2, 0 ),
				V3f( 2, 1, 0 ),
			]
		)		

		inner2 = V3fVectorData(
			[
				V3f( 1, 3, 0 ),
				V3f( 1, 4, 0 ),
				V3f( 2, 4, 0 ),
				V3f( 2, 3, 0 ),
			]
		)
		
		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( [ outer, inner1, inner2 ] )

		outMesh = builder.mesh()
				
		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 13 )

	def testColinearities( self ) :
		
		#  _._
		# |   |
		# .   .
		# |   |
		# .   .
		# |_._|
		#
		
		outer = V3fVectorData(
			[
				V3f( 1, 1, 0 ),
				V3f( 1, 2, 0 ),
				V3f( 1, 3, 0 ),
				V3f( 0.5, 3, 0 ),
				V3f( 0, 3, 0 ),
				V3f( 0, 2, 0 ),
				V3f( 0, 1, 0 ),
				V3f( 0, 0, 0 ),
				V3f( 0.5, 0, 0 ),
				V3f( 1, 0, 0 ),
			]
		)

		
		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( outer )

		outMesh = builder.mesh()
				
		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 3 )
		
		p = outMesh["P"].data
		self.assertEqual( p.size(), 10 )
		
		vertexIds = outMesh.vertexIds
		for i in range( 0, 10 ) :
			self.assert_( i in vertexIds )	
			
		self.assertEqual( outMesh.verticesPerFace, IntVectorData( [ 3 ] * 8 ) )
		
		i = 0
		for j in range( 0, 8 ) :
			
			p0 = p[vertexIds[i]]
			p1 = p[vertexIds[i+1]]
			p2 = p[vertexIds[i+2]]
			
			self.assert_( triangleArea( p0, p1, p2 ) > 0 )
			
	def testHoleAlignedWithVertex( self ) :
	
		#  ______
		# |  __ |
		# | |_| |
		# |___  |
		#    |__|
		#
		
		outer = V3fVectorData(
			[
				V3f( 0, 0, 0 ),
				V3f( 2, 0, 0 ),
				V3f( 2, -1, 0 ),
				V3f( 3, -1, 0 ),
				V3f( 3, 3, 0 ),
				V3f( 0, 3, 0 ),
			]
		)
		
		inner = V3fVectorData(
			[
				V3f( 1, 1, 0 ),
				V3f( 1, 2, 0 ),
				V3f( 2, 2, 0 ),
				V3f( 2, 1, 0 ),
			]
		)

		
		builder = MeshPrimitiveBuilder()	
		triangulator = V3fTriangulator( builder )
		
		triangulator.triangulate( [ outer, inner ] )

		outMesh = builder.mesh()
		
		e = PrimitiveEvaluator.create( outMesh )
		self.assertEqual( e.surfaceArea(), 9 )
		
if __name__ == "__main__":
    unittest.main()   
