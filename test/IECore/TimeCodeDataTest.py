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

import os
import math
import unittest
import IECore
import datetime

class TimeCodeDataTest( unittest.TestCase ) :

	def testConstructor( self ) :

		self.assertTrue( IECore.TimeCodeData().isInstanceOf( IECore.TypeId.TimeCodeData ) )
		self.assertEqual( IECore.TimeCodeData().typeId(), IECore.TypeId.TimeCodeData )
		self.assertEqual( IECore.TimeCodeData().staticTypeId(), IECore.TypeId.TimeCodeData )

		t = IECore.TimeCode( 1, 2, 3, 4 )
		td = IECore.TimeCodeData( t )
		self.assertTrue( td.isInstanceOf( IECore.TypeId.TimeCodeData ) )
		self.assertEqual( td.typeId(), IECore.TypeId.TimeCodeData )
		self.assertEqual( td.staticTypeId(), IECore.TypeId.TimeCodeData )

	def testValueAccess( self ) :

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
		td = IECore.TimeCodeData( t )

		self.assertEqual( td.value.hours(), 12 )
		self.assertEqual( td.value.minutes(), 23 )
		self.assertEqual( td.value.seconds(), 49 )
		self.assertEqual( td.value.frame(), 15 )
		self.assertEqual( td.value.dropFrame(), True )
		self.assertEqual( td.value.colorFrame(), False )
		self.assertEqual( td.value.fieldPhase(), True )
		self.assertEqual( td.value.bgf0(), True )
		self.assertEqual( td.value.bgf1(), False )
		self.assertEqual( td.value.bgf2(), True )
		self.assertEqual( td.value.binaryGroup( 1 ), 1 )
		self.assertEqual( td.value.binaryGroup( 2 ), 2 )
		self.assertEqual( td.value.binaryGroup( 3 ), 3 )
		self.assertEqual( td.value.binaryGroup( 4 ), 4 )
		self.assertEqual( td.value.binaryGroup( 5 ), 5 )
		self.assertEqual( td.value.binaryGroup( 6 ), 6 )
		self.assertEqual( td.value.binaryGroup( 7 ), 7 )
		self.assertEqual( td.value.binaryGroup( 8 ), 8 )
		self.assertEqual( td.value.timeAndFlags(), 2460207445 )
		self.assertEqual( td.value.timeAndFlags( IECore.TimeCode.Packing.TV60 ), 2460207445 )
		self.assertEqual( td.value.timeAndFlags( IECore.TimeCode.Packing.TV50 ), 2460207381 )
		self.assertEqual( td.value.timeAndFlags( IECore.TimeCode.Packing.FILM24  ), 2460207381 )
		self.assertEqual( td.value.userData(), 2271560481 )

		tCopy = td.value
		tCopy.setHours( 8 )
		tCopy.setMinutes( 15 )
		tCopy.setSeconds( 30 )
		tCopy.setFrame( 3 )
		tCopy.setDropFrame( False )
		tCopy.setColorFrame( True )
		tCopy.setFieldPhase( False )
		tCopy.setBgf0( False )
		tCopy.setBgf1( True )
		tCopy.setBgf2( False )
		tCopy.setBinaryGroup( 1, 8 )
		tCopy.setBinaryGroup( 2, 7 )
		tCopy.setBinaryGroup( 3, 6 )
		tCopy.setBinaryGroup( 4, 5 )
		tCopy.setBinaryGroup( 5, 4 )
		tCopy.setBinaryGroup( 6, 3 )
		tCopy.setBinaryGroup( 7, 2 )
		tCopy.setBinaryGroup( 8, 1 )
		td.value = tCopy

		self.assertEqual( td.value.hours(), 8 )
		self.assertEqual( td.value.minutes(), 15 )
		self.assertEqual( td.value.seconds(), 30 )
		self.assertEqual( td.value.frame(), 3 )
		self.assertEqual( td.value.dropFrame(), False )
		self.assertEqual( td.value.colorFrame(), True )
		self.assertEqual( td.value.fieldPhase(), False )
		self.assertEqual( td.value.bgf0(), False )
		self.assertEqual( td.value.bgf1(), True )
		self.assertEqual( td.value.bgf2(), False )
		self.assertEqual( td.value.binaryGroup( 1 ), 8 )
		self.assertEqual( td.value.binaryGroup( 2 ), 7 )
		self.assertEqual( td.value.binaryGroup( 3 ), 6 )
		self.assertEqual( td.value.binaryGroup( 4 ), 5 )
		self.assertEqual( td.value.binaryGroup( 5 ), 4 )
		self.assertEqual( td.value.binaryGroup( 6 ), 3 )
		self.assertEqual( td.value.binaryGroup( 7 ), 2 )
		self.assertEqual( td.value.binaryGroup( 8 ), 1 )
		self.assertEqual( td.value.timeAndFlags(), 1209348227 )
		self.assertEqual( td.value.timeAndFlags( IECore.TimeCode.Packing.TV60 ), 1209348227 )
		self.assertEqual( td.value.timeAndFlags( IECore.TimeCode.Packing.TV50 ), 1209348227 )
		self.assertEqual( td.value.timeAndFlags( IECore.TimeCode.Packing.FILM24  ), 1209348099 )
		self.assertEqual( td.value.userData(), 305419896 )

	def testCopy( self ) :

		t = IECore.TimeCodeData( IECore.TimeCode( 1, 2, 3, 4 ) )
		tt = t.copy()

		self.assertEqual( t, tt )

		tCopy = t.value
		tCopy.setHours( t.value.hours() + 12 )
		t.value = tCopy

		self.assertNotEqual( t, tt )

	def testIO( self ) :

		t = IECore.TimeCodeData()
		IECore.ObjectWriter( t, os.path.join( "test", "IECore", "TimeCodeData.cob" ) ).write()
		tt = IECore.ObjectReader( os.path.join( "test", "IECore", "TimeCodeData.cob" ) ).read()
		self.assertEqual( t, tt )

		t = IECore.TimeCodeData( IECore.TimeCode( 12, 24, 12, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 ) )
		IECore.ObjectWriter( t, os.path.join( "test", "IECore", "TimeCodeData.cob" ) ).write()
		tt = IECore.ObjectReader( os.path.join( "test", "IECore", "TimeCodeData.cob" ) ).read()
		self.assertEqual( t, tt )

	def testRepr( self ) :

		t = IECore.TimeCode( 12, 24, 12, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 )
		tt = IECore.TimeCodeData( t )

		self.assertEqual( repr(tt), "IECore.TimeCodeData( " + repr(t) + " )" )
		self.assertEqual( tt, eval( repr(tt) ) )

	def testStr( self ) :

		t = IECore.TimeCode( 12, 24, 12, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 )
		tt = IECore.TimeCodeData( t )
		self.assertEqual( str(tt), str(t) )

	def testHash( self ) :

		t = IECore.TimeCode( 12, 24, 12, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 )

		self.assertEqual(
			IECore.TimeCodeData( t ).hash(),
			IECore.TimeCodeData( t ).hash()
		)

		tt = IECore.TimeCode( t )
		tt.setSeconds( t.seconds() + 5 )

		self.assertNotEqual(
			IECore.TimeCodeData( t ).hash(),
			IECore.TimeCodeData( tt ).hash()
		)

	def testHasBase( self ) :

		self.assertTrue( IECore.TimeCodeData.hasBase() )

	def setUp( self ) :

		if os.path.isfile( os.path.join( "test", "IECore", "TimeCodeData.cob" ) ) :
			os.remove( os.path.join( "test", "IECore", "TimeCodeData.cob" ) )

	def tearDown( self ) :

		if os.path.isfile( os.path.join( "test", "IECore", "TimeCodeData.cob" ) ) :
			os.remove( os.path.join( "test", "IECore", "TimeCodeData.cob" ) )

if __name__ == "__main__":
    unittest.main()

