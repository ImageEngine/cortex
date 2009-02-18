##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

class DateTimeDataTest( unittest.TestCase ) :

	def testConstructor( self ) :
	
		dd = IECore.DateTimeData()
		
		d = datetime.datetime( 2009, 2, 12 )
		dd = IECore.DateTimeData( d )
		
		
	def testValueAccess( self ) :
	
		d = datetime.datetime( 2009, 2, 12 )
		dd = IECore.DateTimeData( d )
				
		self.assertEqual( dd.value.year, 2009 )
	
	def testCopy( self ) :
	
		d = IECore.DateTimeData( datetime.datetime.now() )
		dd = d.copy()
		
		self.assertEqual( d, dd )
		
		# Same time tomorrow
		d.value += datetime.timedelta( days = 1 )
		
		self.assertNotEqual( d, dd )

	def testIO( self ) :
		
		d = IECore.DateTimeData()		
		IECore.ObjectWriter( d, "test/IECore/DateTimeData.cob" ).write()		
		dd = IECore.ObjectReader( "test/IECore/DateTimeData.cob" ).read()		
		self.assertEqual( d, dd )
		
		d = IECore.DateTimeData( datetime.datetime.now() )		
		IECore.ObjectWriter( d, "test/IECore/DateTimeData.cob" ).write()		
		dd = IECore.ObjectReader( "test/IECore/DateTimeData.cob" ).read()		
		self.assertEqual( d, dd )
		
	def testRepr( self ) :	
	
		d = datetime.datetime.now()
		dd = IECore.DateTimeData(d)
		
		self.assertEqual( repr(dd), "IECore.DateTimeData( " + repr(d) + " )" ) 
		
		self.assertEqual( dd, eval( repr(dd) ) )
		
	def setUp(self):
        
		if os.path.isfile( "test/IECore/DateTimeData.cob" ) :
			os.remove( "test/IECore/DateTimeData.cob" )			

	def tearDown(self):
        
		if os.path.isfile( "test/IECore/DateTimeData.cob" ) :
			os.remove( "test/IECore/DateTimeData.cob" )

		
if __name__ == "__main__":
    unittest.main()   

