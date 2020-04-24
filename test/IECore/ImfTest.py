##########################################################################
#
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

"""Unit test for Imf binding"""

import unittest
import IECore

class ImfTimeCodeTest( unittest.TestCase ) :

	def testConstructors( self ) :
		"""Test TimeCode constructors"""

		t = IECore.TimeCode()
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 1, 2, 3, 4 )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 1, 2, 3, 4, True )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 1, 2, 3, 4, True, True )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 1, 2, 3, 4, True, True, True, True, True, True )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 1, 2, 3, 4, True, True, True, True, True, True, 1, 2, 3, 4, 5, 6, 7, 8 )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 593058073 )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 593058073, 256 )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		t = IECore.TimeCode( 593058073, 256, IECore.TimeCode.Packing.FILM24 )
		self.assertTrue( isinstance( t, IECore.TimeCode ) )

		tt = IECore.TimeCode( t )
		self.assertTrue( isinstance( tt, IECore.TimeCode ) )

		self.assertRaises( RuntimeError, IECore.TimeCode, 25, 2, 3, 4 )
		self.assertRaises( RuntimeError, IECore.TimeCode, 1, 60, 3, 4 )
		self.assertRaises( RuntimeError, IECore.TimeCode, 1, 2, 60, 4 )
		self.assertRaises( RuntimeError, IECore.TimeCode, 1, 2, 3, 60 )

	def testCopyAndAssign( self ) :
		"""Test TimeCode copy construction and assignment"""

		t = IECore.TimeCode( 1, 2, 3, 4 )
		tt = t
		self.assertEqual( t, tt )

		ttt = IECore.TimeCode( t )
		self.assertEqual( tt, ttt )

	def testEquality( self ) :
		"""Test TimeCode comparison for equality"""

		self.assertEqual( IECore.TimeCode( 1, 2, 3, 4 ), IECore.TimeCode( 1, 2, 3, 4 ) )
		self.assertNotEqual( IECore.TimeCode( 1, 2, 3, 4 ), IECore.TimeCode( 4, 3, 2, 1 ) )

		self.assertEqual( IECore.TimeCode( 1, 2, 3, 4, fieldPhase = True ), IECore.TimeCode( 1, 2, 3, 4, False, False, True ) )

		t = IECore.TimeCode( hours = 12, minutes = 24, seconds = 12, frame = 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 )

		self.assertEqual( t, IECore.TimeCode( t.timeAndFlags(), t.userData() ) )

		self.assertNotEqual( t, IECore.TimeCode( t.timeAndFlags() ) )
		self.assertNotEqual( t, IECore.TimeCode( t.timeAndFlags( IECore.TimeCode.Packing.FILM24 ) ) )
		self.assertNotEqual( t, IECore.TimeCode( t.timeAndFlags( IECore.TimeCode.Packing.FILM24 ), t.userData() ) )

		tt = IECore.TimeCode( t.timeAndFlags( IECore.TimeCode.Packing.FILM24 ), t.userData(), IECore.TimeCode.Packing.FILM24 )
		self.assertNotEqual( t.dropFrame(), tt.dropFrame() )
		tt.setDropFrame( True )
		self.assertEqual( t, tt )

	def testMethods( self ) :
		"""Test TimeCode miscellaneous methods"""

		t = IECore.TimeCode(
			hours = 12,
			minutes = 23,
			seconds = 49,
			frame = 15,
			dropFrame = True,
			colorFrame = False,
			fieldPhase = True,
			bgf0 = True,
			bgf1 = False,
			bgf2 = True,
			binaryGroup1 = 1,
			binaryGroup2 = 2,
			binaryGroup3 = 3,
			binaryGroup4 = 4,
			binaryGroup5 = 5,
			binaryGroup6 = 6,
			binaryGroup7 = 7,
			binaryGroup8 = 8,
		)

		self.assertEqual( t.hours(), 12 )
		self.assertEqual( t.minutes(), 23 )
		self.assertEqual( t.seconds(), 49 )
		self.assertEqual( t.frame(), 15 )
		self.assertEqual( t.dropFrame(), True )
		self.assertEqual( t.colorFrame(), False )
		self.assertEqual( t.fieldPhase(), True )
		self.assertEqual( t.bgf0(), True )
		self.assertEqual( t.bgf1(), False )
		self.assertEqual( t.bgf2(), True )
		self.assertEqual( t.binaryGroup( 1 ), 1 )
		self.assertEqual( t.binaryGroup( 2 ), 2 )
		self.assertEqual( t.binaryGroup( 3 ), 3 )
		self.assertEqual( t.binaryGroup( 4 ), 4 )
		self.assertEqual( t.binaryGroup( 5 ), 5 )
		self.assertEqual( t.binaryGroup( 6 ), 6 )
		self.assertEqual( t.binaryGroup( 7 ), 7 )
		self.assertEqual( t.binaryGroup( 8 ), 8 )
		self.assertEqual( t.timeAndFlags(), 2460207445 )
		self.assertEqual( t.timeAndFlags( IECore.TimeCode.Packing.TV60 ), 2460207445 )
		self.assertEqual( t.timeAndFlags( IECore.TimeCode.Packing.TV50 ), 2460207381 )
		self.assertEqual( t.timeAndFlags( IECore.TimeCode.Packing.FILM24  ), 2460207381 )
		self.assertEqual( t.userData(), 2271560481 )

		t.setHours( 8 )
		t.setMinutes( 15 )
		t.setSeconds( 30 )
		t.setFrame( 3 )
		t.setDropFrame( False )
		t.setColorFrame( True )
		t.setFieldPhase( False )
		t.setBgf0( False )
		t.setBgf1( True )
		t.setBgf2( False )
		t.setBinaryGroup( 1, 8 )
		t.setBinaryGroup( 2, 7 )
		t.setBinaryGroup( 3, 6 )
		t.setBinaryGroup( 4, 5 )
		t.setBinaryGroup( 5, 4 )
		t.setBinaryGroup( 6, 3 )
		t.setBinaryGroup( 7, 2 )
		t.setBinaryGroup( 8, 1 )

		self.assertEqual( t.hours(), 8 )
		self.assertEqual( t.minutes(), 15 )
		self.assertEqual( t.seconds(), 30 )
		self.assertEqual( t.frame(), 3 )
		self.assertEqual( t.dropFrame(), False )
		self.assertEqual( t.colorFrame(), True )
		self.assertEqual( t.fieldPhase(), False )
		self.assertEqual( t.bgf0(), False )
		self.assertEqual( t.bgf1(), True )
		self.assertEqual( t.bgf2(), False )
		self.assertEqual( t.binaryGroup( 1 ), 8 )
		self.assertEqual( t.binaryGroup( 2 ), 7 )
		self.assertEqual( t.binaryGroup( 3 ), 6 )
		self.assertEqual( t.binaryGroup( 4 ), 5 )
		self.assertEqual( t.binaryGroup( 5 ), 4 )
		self.assertEqual( t.binaryGroup( 6 ), 3 )
		self.assertEqual( t.binaryGroup( 7 ), 2 )
		self.assertEqual( t.binaryGroup( 8 ), 1 )
		self.assertEqual( t.timeAndFlags(), 1209348227 )
		self.assertEqual( t.timeAndFlags( IECore.TimeCode.Packing.TV60 ), 1209348227 )
		self.assertEqual( t.timeAndFlags( IECore.TimeCode.Packing.TV50 ), 1209348227 )
		self.assertEqual( t.timeAndFlags( IECore.TimeCode.Packing.FILM24  ), 1209348099 )
		self.assertEqual( t.userData(), 305419896 )

	def testPackingModes( self ) :
		"""Test TimeCode Packing modes"""

		t = IECore.TimeCode( timeAndFlags = 192 )
		self.assertEqual( t, IECore.TimeCode( timeAndFlags = 192, packing = IECore.TimeCode.Packing.TV60 ) )
		self.assertNotEqual( t, IECore.TimeCode( timeAndFlags = 192, packing = IECore.TimeCode.Packing.TV50 ) )
		self.assertNotEqual( t,	IECore.TimeCode( timeAndFlags = 192, packing = IECore.TimeCode.Packing.FILM24 ) )

		tt = IECore.TimeCode()
		tt.setTimeAndFlags( 192 )
		self.assertEqual( t, tt )

		tv60 = IECore.TimeCode()
		tv60.setTimeAndFlags( 192, packing = IECore.TimeCode.Packing.TV60 )
		self.assertEqual( t, tv60 )

		tv50 = IECore.TimeCode()
		tv50.setTimeAndFlags( 128, packing = IECore.TimeCode.Packing.TV50 )
		self.assertNotEqual( t.dropFrame(), tv50.dropFrame() )
		tv50.setDropFrame( True )
		self.assertEqual( t, tv50 )

		film = IECore.TimeCode()
		film.setTimeAndFlags( 0, packing = IECore.TimeCode.Packing.FILM24 )
		self.assertNotEqual( t.dropFrame(), film.dropFrame() )
		self.assertNotEqual( t.dropFrame(), film.colorFrame() )
		film.setDropFrame( True )
		film.setColorFrame( True )
		self.assertEqual( t, film )

	def testRepr( self ) :
		"""Test TimeCode repr"""

		t = IECore.TimeCode( 12, 5, 3, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 )
		self.assertEqual( repr(t), "IECore.TimeCode( 12, 5, 3, 15, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 12, 0, 0 )" )
		self.assertEqual( t, eval( repr(t) ) )

	def testStr( self ) :
		"""Test TimeCode str"""

		t = IECore.TimeCode( 12, 5, 3, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 )
		self.assertEqual( str(t), "12:05:03:15" )

if __name__ == "__main__" :
	unittest.main()
