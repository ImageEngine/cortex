##########################################################################
#
#  Copyright (c) 2021, Hypothetical Inc. All rights reserved.
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
import imath

import pxr.Sdf

import IECore
import IECoreUSD

class DataAlgoTest( unittest.TestCase ) :

	def testRoleBinding( self ) :

		self.assertEqual( IECoreUSD.DataAlgo.role( IECore.GeometricData.Interpretation.Point ), "Point" )
		self.assertEqual( IECoreUSD.DataAlgo.role( IECore.GeometricData.Interpretation.Vector ), "Vector" )
		self.assertEqual( IECoreUSD.DataAlgo.role( IECore.GeometricData.Interpretation.Normal ), "Normal" )
		self.assertEqual( IECoreUSD.DataAlgo.role( IECore.GeometricData.Interpretation.UV ), "TextureCoordinate" )
		self.assertEqual( IECoreUSD.DataAlgo.role( IECore.GeometricData.Interpretation.Color ), "Color" )
		self.assertEqual( IECoreUSD.DataAlgo.role( IECore.GeometricData.Interpretation.Rational ), "" )

	def testValueTypeNameBinding( self ) :

		v3 = IECore.V3fData( imath.V3f( 0.0 ) )
		v2 = IECore.V2fData( imath.V2f( 0.0 ) )

		v3.setInterpretation( IECore.GeometricData.Interpretation.Point )
		self.assertEqual( IECoreUSD.DataAlgo.valueTypeName( v3 ).type.typeName, "GfVec3f" )

		v3.setInterpretation( IECore.GeometricData.Interpretation.Vector )
		self.assertEqual( IECoreUSD.DataAlgo.valueTypeName( v3 ).type.typeName, "GfVec3f" )

		v3.setInterpretation( IECore.GeometricData.Interpretation.Normal )
		self.assertEqual( IECoreUSD.DataAlgo.valueTypeName( v3 ).type.typeName, "GfVec3f" )

		v2.setInterpretation( IECore.GeometricData.Interpretation.UV )
		self.assertEqual( IECoreUSD.DataAlgo.valueTypeName( v2 ).type.typeName, "GfVec2f" )

		self.assertEqual( IECoreUSD.DataAlgo.valueTypeName( IECore.Color3fData( imath.Color3f( 0.0 ) ) ).type.typeName, "GfVec3f" )

		self.assertEqual( IECoreUSD.DataAlgo.valueTypeName( IECore.PathMatcherData() ), pxr.Sdf.ValueTypeName() )
		self.assertEqual( IECoreUSD.DataAlgo.valueTypeName( IECore.CompoundData() ), pxr.Sdf.ValueTypeName() )

	def testToUSDBinding( self ) :

		# Note : On the C++ side we are converting to VtValue, but the
		# bindings for that convert back to native Python types.

		for data, value in [
			( IECore.IntData( 10 ), 10 ),
			( IECore.FloatData( 2.5 ), 2.5 ),
			( IECore.IntVectorData( [ 1, 2, 3 ] ), [ 1, 2, 3 ] ),
			( IECore.PathMatcherData(), None ),
			( IECore.CompoundData(), {} ),
		] :
			self.assertEqual( IECoreUSD.DataAlgo.toUSD( data ), value )

	def testToFromInternalName( self ) :

		a = "a-name(that-is-bad)"

		b = IECoreUSD.SceneCacheDataAlgo.toInternalName( a )
		self.assertEqual( b, "a_____name____that_____is_____bad___" )

		c = IECoreUSD.SceneCacheDataAlgo.fromInternalName( b )
		self.assertEqual( a , c )

	def testToFromInternalPath( self ) :

		a = ["a-path", "(that)", "(is)", "(bad)"]

		b = IECoreUSD.SceneCacheDataAlgo.toInternalPath( a )
		self.assertEqual(
			b,
			[
				"__IECOREUSD_ROOT",
				"a_____path",
				"____that___",
				"____is___",
				"____bad___",
			]
		)

		c = IECoreUSD.SceneCacheDataAlgo.fromInternalPath( b )

		self.assertEqual( a, c )


if __name__ == "__main__":
	unittest.main()
