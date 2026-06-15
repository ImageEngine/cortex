##########################################################################
#
#  Copyright (c) 2026, Cinesite VFX Ltd. All rights reserved.
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
import imath

import IECore
import IECoreScene

class PointInstancerTest( unittest.TestCase ) :

	def testAccessors( self ) :

		instancer = IECoreScene.PointInstancer( 4 )

		self.assertIsInstance( instancer.getPosition(), IECoreScene.PrimitiveVariable.V3fIndexedView )
		self.assertFalse( instancer.getPosition() )

		instancer.setPosition( IECore.V3fVectorData( [ imath.V3f( x, 0, 0 ) for x in range( 4 ) ] ) )
		self.assertIsInstance( instancer.getPosition(), IECoreScene.PrimitiveVariable.V3fIndexedView )
		self.assertEqual( list( instancer.getPosition() ), [ imath.V3f( x, 0, 0 ) for x in range( 4 ) ] )

		instancer["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ imath.V3f( 1 ), imath.V3f( 2 ) ] ),
			IECore.IntVectorData( [ 1, 0, 0, 1 ] )
		)
		self.assertEqual( list( instancer.getPosition() ), [ imath.V3f( 2 ), imath.V3f( 1 ), imath.V3f( 1 ), imath.V3f( 2 ) ] )

		instancer["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Constant,
			IECore.V3fVectorData( [ imath.V3f( x, 0, 0 ) for x in range( 4 ) ] ),
		)
		self.assertFalse( instancer.getPosition() )

		instancer["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.IntVectorData( range( 4 ) ),
		)
		self.assertFalse( instancer.getPosition() )

	def testVisibilityQuery( self ) :

		instancer = IECoreScene.PointInstancer( 4 )
		instancer.setInvisibleIDs( IECore.Int64VectorData( [ 1, 3 ] ) )

		query = IECoreScene.PointInstancer.VisibilityQuery( instancer )
		self.assertEqual(
			[ query.visible( i ) for i in range( 0, 4 ) ],
			[ True, False, True, False ]
		)

		instancer.setID( IECore.Int64VectorData( [ 1, 3, 0, 4 ] ) )
		query = IECoreScene.PointInstancer.VisibilityQuery( instancer )
		self.assertEqual(
			[ query.visible( i ) for i in range( 0, 4 ) ],
			[ False, False, True, True ]
		)

	def testTransformQuery( self ) :

		instancer = IECoreScene.PointInstancer( 4 )
		instancer.setPosition( IECore.V3fVectorData( [ imath.V3f( x, 0, 0 ) for x in range( 4 ) ] ) )

		query = IECoreScene.PointInstancer.TransformQuery( instancer )
		for i in range( 4 ) :
			self.assertEqual( query.transform( i ), imath.M44f().translate( imath.V3f( i, 0, 0 ) ) )

if __name__ == "__main__":
    unittest.main()
