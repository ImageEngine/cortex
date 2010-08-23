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
import math
import unittest
import IECore

class SplineDataTest( unittest.TestCase ) :

	def testConstructor( self ) :

		d = IECore.SplineffData()
		self.assertEqual( d.value, IECore.Splineff() )

		s = IECore.Splineff( IECore.CubicBasisf.bezier() )
		d = IECore.SplineffData( s )

		self.assertEqual( d.value, s )

	def testValueAccess( self ) :

		d = IECore.SplineffData()
		d.value.basis.step = 10

		self.assertEqual( d.value.basis.step, 10 )

		d.value = IECore.Splineff( IECore.CubicBasisf.linear() )
		self.assertEqual( d.value, IECore.Splineff( IECore.CubicBasisf.linear() ) )

	def testCopy( self ) :

		d = IECore.SplineffData()
		dd = d.copy()

		self.assertEqual( d, dd )

		dd.value.basis.step = 10

		self.assertNotEqual( d, dd )

	def testIO( self ) :

		s = IECore.Splineff( IECore.CubicBasisf.bezier() )
		s[0] = 1
		s[1] = 2
		s[2] = 3
		s[3] = 4

		sd = IECore.SplineffData( s )

		IECore.ObjectWriter( sd, "test/IECore/SplineData.cob" ).write()

		sdd = IECore.ObjectReader( "test/IECore/SplineData.cob" ).read()

		self.assertEqual( sd, sdd )

	def testColorIO( self ) :

		s = IECore.SplinefColor3f( IECore.CubicBasisf.linear() )
		s[0] = IECore.Color3f( 1 )
		s[1] = IECore.Color3f( 2 )
		s[2] = IECore.Color3f( 3 )
		s[3] = IECore.Color3f( 4 )

		sd = IECore.SplinefColor3fData( s )

		IECore.ObjectWriter( sd, "test/IECore/SplineData.cob" ).write()

		sdd = IECore.ObjectReader( "test/IECore/SplineData.cob" ).read()

		self.assertEqual( sd, sdd )

	def testRepr( self ) :

		s = IECore.SplinefColor3f( IECore.CubicBasisf.linear() )
		s[0] = IECore.Color3f( 1 )
		s[1] = IECore.Color3f( 2 )
		s[2] = IECore.Color3f( 3 )
		s[3] = IECore.Color3f( 4 )

		sd = IECore.SplinefColor3fData( s )

		self.assertEqual( repr(sd), "IECore.SplinefColor3fData( " + repr(s) + " )" )

		self.assertEqual( sd, eval( repr(sd) ) )

	def setUp(self):

		if os.path.isfile( "test/IECore/SplineData.cob" ) :
			os.remove( "test/IECore/SplineData.cob" )

	def tearDown(self):

		if os.path.isfile( "test/IECore/SplineData.cob" ) :
			os.remove( "test/IECore/SplineData.cob" )


if __name__ == "__main__":
    unittest.main()

