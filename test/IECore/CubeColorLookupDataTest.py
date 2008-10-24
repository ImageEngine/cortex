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

import unittest
import math
import os
from IECore import *

class CubeColorLookupDataTest( unittest.TestCase ) :

	def testConstructor( self ) :
	
		d1 = CubeColorLookupfData()

		
	def testIO( self ) :
	
		d1 = CubeColorLookupfData()		
		ObjectWriter( d1, "test/IECore/CubeColorLookupData.cob" ).write()		
		self.assertEqual( d1, ObjectReader( "test/IECore/CubeColorLookupData.cob" ).read() )
		
		d2 = CubeColorLookupdData()
		ObjectWriter( d2, "test/IECore/CubeColorLookupData.cob" ).write()		
		self.assertEqual( d2, ObjectReader( "test/IECore/CubeColorLookupData.cob" ).read() )
		
		d3 = CubeColorLookupfData( 
			CubeColorLookupf( 
				V3i( 2, 2, 2 ), 
				
				[
					Color3f( 1, 1, 1 ), 
					Color3f( 2, 2, 2 ), 
					Color3f( 3, 3, 3 ), 
					Color3f( 4, 4, 4 ), 
					Color3f( 5, 5, 5 ), 
					Color3f( 6, 6, 6 ), 
					Color3f( 7, 7, 7 ), 
					Color3f( 8, 8, 8 ), 
				], 
				
				Box3f(
					V3f( -1, -1, -1 ), 				
					V3f( 1, 1, 1 )
				),
				
				CubeColorLookupf.Interpolation.None )
		)
		ObjectWriter( d3, "test/IECore/CubeColorLookupData.cob" ).write()		
		self.assertEqual( d3, ObjectReader( "test/IECore/CubeColorLookupData.cob" ).read() )
				
		
	def setUp(self):
        
		if os.path.isfile( "test/IECore/CubeColorLookupData.cob" ) :
			os.remove( "test/IECore/CubeColorLookupData.cob" )			

	def tearDown(self):
        
		if os.path.isfile( "test/IECore/CubeColorLookupData.cob" ) :
			os.remove( "test/IECore/CubeColorLookupData.cob" )	
		
				
		
if __name__ == "__main__":
	unittest.main()
	
