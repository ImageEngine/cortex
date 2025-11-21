##########################################################################
#
#  Copyright (c) 2025, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore

class RampDataTest( unittest.TestCase ) :

	def testConstructor( self ) :

		d = IECore.RampffData()
		self.assertEqual( d.value, IECore.Rampff() )

		s = IECore.Rampff( ( ( 0, 0 ), ( 1, 1 ) ), IECore.RampInterpolation.MonotoneCubic )
		d = IECore.RampffData( s )

		self.assertEqual( d.value, s )

	def testValueAccess( self ) :

		d = IECore.RampffData()
		d.value.interpolation = IECore.RampInterpolation.BSpline

		self.assertEqual( d.value.interpolation, IECore.RampInterpolation.BSpline )

		d.value = IECore.Rampff( ( ( 0, 0 ), ( 1, 1 ) ), IECore.RampInterpolation.Linear )
		self.assertEqual( d.value, IECore.Rampff( ( ( 0, 0 ), ( 1, 1 ) ), IECore.RampInterpolation.Linear ) )

	def testCopy( self ) :

		d = IECore.RampffData()
		dd = d.copy()

		self.assertEqual( d, dd )

		dd.value.interpolation = IECore.RampInterpolation.BSpline

		self.assertNotEqual( d, dd )

	def testIO( self ) :

		s = IECore.Rampff( ( ( 0, 1 ), ( 1, 2 ), ( 2, 3 ), ( 3, 4 ) ), IECore.RampInterpolation.MonotoneCubic )

		sd = IECore.RampffData( s )

		IECore.ObjectWriter( sd, os.path.join( "test", "IECore", "RampData.cob" ) ).write()

		sdd = IECore.ObjectReader( os.path.join( "test", "IECore", "RampData.cob" ) ).read()

		self.assertEqual( sd, sdd )

	def testColorIO( self ) :

		s = IECore.RampfColor3f(
			( ( 0, imath.Color3f(1) ), ( 1, imath.Color3f(2) ), ( 2, imath.Color3f(3) ), ( 3, imath.Color3f(4) ) ),
			IECore.RampInterpolation.MonotoneCubic
		)
		sd = IECore.RampfColor3fData( s )

		IECore.ObjectWriter( sd, os.path.join( "test", "IECore", "RampData.cob" ) ).write()

		sdd = IECore.ObjectReader( os.path.join( "test", "IECore", "RampData.cob" ) ).read()

		self.assertEqual( sd, sdd )

	def testRepr( self ) :

		s = IECore.RampfColor3f(
			( ( 0, imath.Color3f(1) ), ( 1, imath.Color3f(2) ), ( 2, imath.Color3f(3) ), ( 3, imath.Color3f(4) ) ),
			IECore.RampInterpolation.BSpline
		)

		sd = IECore.RampfColor3fData( s )

		self.assertEqual( repr(sd), "IECore.RampfColor3fData( " + repr(s) + " )" )

		self.assertEqual( sd, eval( repr(sd) ) )

	def testHash( self ) :

		points = ( ( 0, imath.Color3f(1) ), ( 1, imath.Color3f(2) ), ( 2, imath.Color3f(3) ), ( 3, imath.Color3f(4) ) )

		s = IECore.RampfColor3fData( IECore.RampfColor3f( points, IECore.RampInterpolation.Linear ) )

		h = s.hash()

		points = ( *points[:2], ( 3, imath.Color3f( 5 ) ) )
		s = IECore.RampfColor3fData( IECore.RampfColor3f( points, IECore.RampInterpolation.Linear ) )
		self.assertNotEqual( s.hash(), h )
		h = s.hash()

		points = points[:2]
		s = IECore.RampfColor3fData( IECore.RampfColor3f( points, IECore.RampInterpolation.Linear ) )
		self.assertNotEqual( s.hash(), h )
		h = s.hash()

		s = IECore.RampfColor3fData( IECore.RampfColor3f( points, IECore.RampInterpolation.BSpline ) )
		self.assertNotEqual( s.hash(), h )
		h = s.hash()

	def setUp(self):

		if os.path.isfile( os.path.join( "test", "IECore", "RampData.cob" ) ) :
			os.remove( os.path.join( "test", "IECore", "RampData.cob" ) )

	def tearDown(self):

		if os.path.isfile( os.path.join( "test", "IECore", "RampData.cob" ) ) :
			os.remove( os.path.join( "test", "IECore", "RampData.cob" ) )


if __name__ == "__main__":
    unittest.main()

