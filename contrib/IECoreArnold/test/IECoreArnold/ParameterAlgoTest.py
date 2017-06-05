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

import unittest

import arnold

import IECore
import IECoreArnold

class ParameterAlgoTest( unittest.TestCase ) :

	def testTypeErrors( self ) :

		self.assertRaisesRegexp(
			TypeError,
			"Expected an AtNode",
			IECoreArnold.ParameterAlgo.setParameter, None, "test", IECore.IntData( 10 )
		)

	def testSetParameter( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = arnold.AiNode( "standard" )

			IECoreArnold.ParameterAlgo.setParameter( n, "Kd", IECore.FloatData( 0.25 ) )
			IECoreArnold.ParameterAlgo.setParameter( n, "aov_emission", IECore.StringData( "test" ) )

			self.assertEqual( arnold.AiNodeGetFlt( n, "Kd" ), 0.25 )
			self.assertEqual( arnold.AiNodeGetStr( n, "aov_emission" ), "test" )

	def testGetParameter( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = arnold.AiNode( "standard" )

			self.assertEqual(
				IECoreArnold.ParameterAlgo.getParameter( n, "Kd" ),
				IECore.FloatData( arnold.AiNodeGetFlt( n, "Kd" ) )
			)

			self.assertEqual(
				IECoreArnold.ParameterAlgo.getParameter( n, "aov_emission" ),
				IECore.StringData( "emission" ),
			)

	def testDoubleData( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = arnold.AiNode( "standard" )

			IECoreArnold.ParameterAlgo.setParameter( n, "Kd", IECore.DoubleData( 0.25 ) )
			self.assertEqual( arnold.AiNodeGetFlt( n, "Kd" ), 0.25 )

			IECoreArnold.ParameterAlgo.setParameter( n, "customFloat", IECore.DoubleData( 0.25 ) )
			self.assertEqual( arnold.AiNodeGetFlt( n, "customFloat" ), 0.25 )

			IECoreArnold.ParameterAlgo.setParameter( n, "customMatrix", IECore.M44dData( IECore.M44d( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ) ) )
			m = arnold.AtMatrix()
			arnold.AiNodeGetMatrix( n, "customMatrix", m )
			self.assertEqual(
				[ getattr( m, f[0] ) for f in m._fields_ ],
				[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 ],
			)

	def testStringArray( self ) :

		with IECoreArnold.UniverseBlock( writable = True ) :

			n = arnold.AiNode( "polymesh" )
			IECoreArnold.ParameterAlgo.setParameter( n, "trace_sets", IECore.StringVectorData( [ "a", "b" ] ) )

			a = arnold.AiNodeGetArray( n, "trace_sets" )
			self.assertEqual( a.contents.nelements, 2 )
			self.assertEqual( arnold.AiArrayGetStr( a, 0 ), "a" )
			self.assertEqual( arnold.AiArrayGetStr( a, 1 ), "b" )

if __name__ == "__main__":
    unittest.main()
