##########################################################################
#
#  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

import math
import unittest

import arnold

import IECore
import IECoreArnold

class CurvesTest( unittest.TestCase ) :

	def testMotion( self ) :

		c1 = IECore.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ) )
		c2 = IECore.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ) )

		c1["P"] = IECore.PrimitiveVariable(
			IECore.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ IECore.V3f( 1 ) ] * 4 ),
		)

		c2["P"] = IECore.PrimitiveVariable(
			IECore.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ IECore.V3f( 2 ) ] * 4 ),
		)

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = IECoreArnold.NodeAlgo.convert( [ c1, c2 ], [ -0.25, 0.25 ] )

			a = arnold.AiNodeGetArray( n, "points" )
			self.assertEqual( a.contents.nelements, 4 )
			self.assertEqual( a.contents.nkeys, 2 )

			for i in range( 0, 4 ) :
				self.assertEqual( arnold.AiArrayGetPnt( a, i ), arnold.AtPoint( 1 ) )
			for i in range( 4, 8 ) :
				self.assertEqual( arnold.AiArrayGetPnt( a, i ), arnold.AtPoint( 2 ) )

			a = arnold.AiNodeGetArray( n, "deform_time_samples" )
			self.assertEqual( a.contents.nelements, 2 )
			self.assertEqual( a.contents.nkeys, 1 )
			self.assertEqual( arnold.AiArrayGetFlt( a, 0 ), -0.25 )
			self.assertEqual( arnold.AiArrayGetFlt( a, 1 ), 0.25 )

	def testNPrimitiveVariable( self ) :

		c = IECore.CurvesPrimitive( IECore.IntVectorData( [ 4 ] ), IECore.CubicBasisf.catmullRom() )
		c["P"] = IECore.PrimitiveVariable(
			IECore.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ IECore.V3f( x, 0, 0 ) for x in range( 0, 4 ) ] )
		)

		with IECoreArnold.UniverseBlock( writable = True ) :

			# No N - should be a ribbon

			n = IECoreArnold.NodeAlgo.convert( c )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "ribbon" )
			self.assertEqual( arnold.AiNodeGetArray( n, "orientations" ).contents.nelements, 0 )

			# N - should be oriented

			c["N"] = IECore.PrimitiveVariable(
				IECore.PrimitiveVariable.Interpolation.Vertex,
				IECore.V3fVectorData( [ IECore.V3f( 0, math.sin( x ), math.cos( x ) ) for x in range( 0, 4 ) ] )
			)

			n = IECoreArnold.NodeAlgo.convert( c )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "oriented" )
			orientations = arnold.AiNodeGetArray( n, "orientations" )
			self.assertEqual( orientations.contents.nelements, 4 )

			for i in range( 0, 4 ) :
				self.assertEqual( arnold.AiArrayGetVec( orientations, i ), arnold.AtVector( 0, math.sin( i ), math.cos( i ) ) )

			# Motion blurred N - should be oriented and deforming

			c2 = c.copy()
			c2["N"] = IECore.PrimitiveVariable(
				IECore.PrimitiveVariable.Interpolation.Vertex,
				IECore.V3fVectorData( [ IECore.V3f( 0, math.sin( x + 0.2 ), math.cos( x + 0.2 ) ) for x in range( 0, 4 ) ] )
			)

			n = IECoreArnold.NodeAlgo.convert( [ c, c2 ], [ 0.0, 1.0 ] )
			self.assertEqual( arnold.AiNodeGetStr( n, "mode" ), "oriented" )

			orientations = arnold.AiNodeGetArray( n, "orientations" )
			self.assertEqual( orientations.contents.nelements, 4 )
			self.assertEqual( orientations.contents.nkeys, 2 )

			for i in range( 0, 4 ) :
				self.assertEqual( arnold.AiArrayGetVec( orientations, i ), arnold.AtVector( 0, math.sin( i ), math.cos( i ) ) )
				self.assertEqual( arnold.AiArrayGetVec( orientations, i + 4 ), arnold.AtVector( 0, math.sin( i + 0.2 ), math.cos( i + 0.2 ) ) )

if __name__ == "__main__":
    unittest.main()
