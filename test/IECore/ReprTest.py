##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

import IECore

class ReprTest( unittest.TestCase ) :

	def test( self ) :

		for v in [
			imath.V2i( 1 ),
			imath.V2f( 1 ),
			imath.V2d( 1.5 ),
			imath.V3i( 1 ),
			imath.V3f( 1 ),
			imath.V3d( 1.5 ),
			imath.Box2i(),
			imath.Box2i( imath.V2i( 1 ), imath.V2i( 1 ) ),
			imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ),
			imath.Box2f(),
			imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ),
			imath.Box3f(),
			"test",
			10,
			10.5
		] :
			self.assertTrue( type( v ) is type( eval( IECore.repr( v ) ) ) )
			self.assertEqual( v, eval( IECore.repr( v ) ) )
	
	def testInfinity( self ) :

		for v in [
			# Python raises "OverflowError : bad numeric conversion : positive overflow"
			# when passing `float( "inf" )` to `V2f``
			imath.V2d( float( "inf" ), float( "inf" ) ),
			imath.V3f( float( "inf" ), float( "inf" ), float( "inf" ) ),
			imath.V3d( float( "inf" ), float( "inf" ), float( "inf" ) ),
			imath.Color3f( float( "inf" ), float( "inf" ), float( "inf" ) ),
			imath.Color4f( float( "inf" ), float( "inf" ), float( "inf" ), float( "inf" ) ),
		] :
			with self.subTest( v = v ) :
				self.assertTrue( type( v ) is type( eval( IECore.repr( v ) ) ) )
				self.assertEqual( v, eval( IECore.repr( v ) ) )

				self.assertTrue( type( -v ) is type( eval( IECore.repr( -v ) ) ) )
				self.assertEqual( -v, eval( IECore.repr( -v ) ) )

				self.assertEqual( str( v ), "{}({})".format( type( v ).__name__, ", ".join( ["inf"] * v.dimensions() ) ) )
				self.assertEqual( str( -v ), "{}({})".format( type( v ).__name__, ", ".join( ["-inf"] * v.dimensions() ) ) )


if __name__ == "__main__":
	unittest.main()

