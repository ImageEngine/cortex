##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

class PointsMotionOpTest( unittest.TestCase ) :

	def _buildPoints( self, time ):
		p = IECore.PointsPrimitive( 5 )
		p[ "P" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f(time*1), IECore.V3f(time*2), IECore.V3f(time*3), IECore.V3f(time*4), IECore.V3f(time*5) ] ) )
		p[ "id" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.IntVectorData( [ 1, 2, 3, 4, 5 ] ) )
		p[ "float" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Varying, IECore.FloatVectorData( [ time*1, time*2, time*3, time*4, time*5 ] ) )
		p[ "vec3" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f(time*2), IECore.V3f(time*3), IECore.V3f(time*4), IECore.V3f(time*5), IECore.V3f(time*6) ] ) )
		p[ "vec2" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V2fVectorData( [ IECore.V2f(time*2), IECore.V2f(time*3), IECore.V2f(time*4), IECore.V2f(time*5), IECore.V2f(time*6) ] ) )
		p[ "C" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.Color3fVectorData( [ IECore.Color3f(1), IECore.Color3f(2), IECore.Color3f(4), IECore.Color3f(5), IECore.Color3f(6) ] ) )
		p[ "C2" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.Color4fVectorData( [ IECore.Color4f(1), IECore.Color4f(2), IECore.Color4f(4), IECore.Color4f(5), IECore.Color4f(6) ] ) )
		p[ "G" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f(time) ] ) )
		p[ "c" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatData( time ) )
		return p

	def testConstruction( self ) :

		op = IECore.PointsMotionOp()

	def testPrimvarCopy( self ) :

		p1 = self._buildPoints( 1.0 )
		p2 = self._buildPoints( 2.0 )
		points = IECore.ObjectVector()
		points.append( p1 )
		points.append( p2 )

		op = IECore.PointsMotionOp()
		result = op( snapshotTimes = IECore.FloatVectorData( [ 1, 2 ] ), pointsPrimitives = points )
		# proves that the output correspond exactly to the input
		self.assertEqual( len(result), 2 )
		self.assertEqual( result[1], p1 )
		self.assertEqual( result[2], p2 )
		self.assertNotEqual( result[1], p2 )

		p1[ "P" ].data[3] = IECore.V3f(1,2,3)
		self.assertEqual( p1["P"].data[3], IECore.V3f(1,2,3) )
		# proves that destroying the original input value, keeps the output untouched.
		self.assertEqual( result[1]["P"].data[3], IECore.V3f(4) )

	def testInvalidInputParams( self ):

		p1 = self._buildPoints( 1.0 )
		p2 = self._buildPoints( 2.0 )
		points = IECore.ObjectVector()
		points.append( p1 )
		points.append( p2 )
		op = IECore.PointsMotionOp()
		op['pointsPrimitives'] = points

		# snapshots count != primitive count
		op['snapshotTimes'] = IECore.FloatVectorData( [ 1 ] )
		self.assertRaises( RuntimeError, op.__call__, **{} )

		# test no id
		op['snapshotTimes'] = IECore.FloatVectorData( [ 1, 2 ] )
		op['idPrimVarName'] = "dontExist"
		self.assertRaises( RuntimeError, op.__call__, **{} )
		op['idPrimVarName'] = "id"
		op()	# should work

		# test unmatching primvars
		del p1["vec2"]
		self.assertRaises( RuntimeError, op.__call__, **{} )
		del p2["vec2"]
		op()	# should work

		# test no P
		del p2["P"]
		self.assertRaises( RuntimeError, op.__call__, **{} )

	def testDifferentPointsOrder( self ):

		def rearrangeVec( vec ):
			lastIndex = len(vec) - 1
			for i in xrange(0,len(vec)/2 ):
				tmp = vec[i]
				vec[i] = vec[ lastIndex ]
				vec[ lastIndex ] = tmp
				lastIndex -= 1

		p1 = self._buildPoints( 1.0 )
		p2 = self._buildPoints( 1.0 )
		points = IECore.ObjectVector()
		points.append( p1 )
		points.append( p2 )
		op = IECore.PointsMotionOp()

		for primVar in p2.values() :
			if primVar.interpolation in [ IECore.PrimitiveVariable.Interpolation.Vertex, IECore.PrimitiveVariable.Interpolation.Varying, IECore.PrimitiveVariable.Interpolation.FaceVarying ] :
				rearrangeVec( primVar.data )

		self.assertNotEqual( p1, p2 )
		result = op( snapshotTimes = IECore.FloatVectorData( [ 1, 2 ] ), pointsPrimitives = points )
		self.assertEqual( result[1], p1 )
		self.assertEqual( result[2], p1 )

	def testSingleSnapshot( self ):

		p1 = self._buildPoints( 1.0 )
		points = IECore.ObjectVector()
		points.append( p1 )
		op = IECore.PointsMotionOp()
		result = op( snapshotTimes = IECore.FloatVectorData( [ 1 ] ), pointsPrimitives = points )
		self.assertEqual( len(result), 1 )
		self.assertEqual( result[1], p1 )

	def testNoSnapshots( self ):

		points = IECore.ObjectVector()
		op = IECore.PointsMotionOp()
		result = op( snapshotTimes = IECore.FloatVectorData( [] ), pointsPrimitives = points )
		self.assertEqual( len(result), 0 )

	def testMissingSnapshots( self ):

		p1 = self._buildPoints( 1.0 )
		p2 = self._buildPoints( 2.0 )
		p3 = self._buildPoints( 3.0 )
		p4 = self._buildPoints( 4.0 )
		p5 = self._buildPoints( 5.0 )
		points = IECore.ObjectVector()
		points.append( p1 )
		points.append( p2 )
		points.append( p3 )
		points.append( p4 )
		points.append( p5 )

		# change some ids to simulate dead particles, and birth of new particles
		p1["id"].data[0] = 10
		p1["id"].data[1] = 11
		p2["id"].data[1] = 11
		p4["id"].data[3] = 12
		p5["id"].data[3] = 12
		p5["id"].data[4] = 13

		op = IECore.PointsMotionOp()
		result = op( snapshotTimes = IECore.FloatVectorData( [ 1, 2, 3, 4, 5 ] ), pointsPrimitives = points, maskedPrimVars = IECore.StringVectorData( [ 'vec3' ] ) )

		self.assertEqual( len(result), 5 )
		# checking ids
		self.assertEqual( result[1]["id"].data, IECore.IntVectorData([ 10,11,3,4,5,1,2,12,13 ]) )
		self.assertEqual( result[1]["id"].data, result[2]["id"].data )
		self.assertEqual( result[1]["id"].data, result[3]["id"].data )
		self.assertEqual( result[1]["id"].data, result[4]["id"].data )
		self.assertEqual( result[1]["id"].data, result[5]["id"].data )
		# checking if unmasked prim var was filled with closest available value
		self.assertEqual( result[1]["P"].data, IECore.V3fVectorData( [ IECore.V3f(1*1), IECore.V3f(1*2), IECore.V3f(1*3), IECore.V3f(1*4), IECore.V3f(1*5),   IECore.V3f(2*1), IECore.V3f(3*2), IECore.V3f(4*4), IECore.V3f(5*5) ] ) )
		self.assertEqual( result[3]["P"].data, IECore.V3fVectorData( [ IECore.V3f(1*1), IECore.V3f(2*2), IECore.V3f(3*3), IECore.V3f(3*4), IECore.V3f(3*5),   IECore.V3f(3*1), IECore.V3f(3*2), IECore.V3f(4*4), IECore.V3f(5*5) ] ) )
		self.assertEqual( result[5]["P"].data, IECore.V3fVectorData( [ IECore.V3f(1*1), IECore.V3f(2*2), IECore.V3f(5*3), IECore.V3f(3*4), IECore.V3f(4*5),   IECore.V3f(5*1), IECore.V3f(5*2), IECore.V3f(5*4), IECore.V3f(5*5) ] ) )
		# checkin if masked prim var was filled with zero
		self.assertEqual( result[3]["vec3"].data, IECore.V3fVectorData( [ IECore.V3f(0), IECore.V3f(0), IECore.V3f(3*4), IECore.V3f(3*5), IECore.V3f(3*6),   IECore.V3f(3*2), IECore.V3f(3*3), IECore.V3f(0), IECore.V3f(0) ] ) )

if __name__ == "__main__":
    unittest.main()
