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
import IECore
import IECoreRI
import os.path
import os

class ArchiveRecordTest( unittest.TestCase ) :

	def test( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testArchiveRecord.rib" )
		r.worldBegin()
		r.command( "ri:archiveRecord", { "type" : IECore.StringData( "verbatim" ), "record" : IECore.StringData( "Geometry \"teapot\"\n" ) } )
		r.worldEnd()

		l = file( "test/IECoreRI/output/testArchiveRecord.rib" ).readlines()
		self.assert_( "Geometry \"teapot\"\n" in l )
		
	def testFormatCatcher( self ) :
	
		# passing printf style format strings in the record would blow up the renderer
		# so check we're catching those
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testArchiveRecord.rib" )
		r.worldBegin()
		r.command( "ri:archiveRecord", { "type" : IECore.StringData( "verbatim" ), "record" : IECore.StringData( "NAUGHTY %s %f" ) } )
		r.worldEnd()

		l = "".join( file( "test/IECoreRI/output/testArchiveRecord.rib" ).readlines() )
		self.assert_( not "NAUGHTY" in l )
	
	def tearDown( self ) :
	
		if os.path.exists( "test/IECoreRI/output/testArchiveRecord.rib" ) :
			os.remove( "test/IECoreRI/output/testArchiveRecord.rib" )
				
if __name__ == "__main__":
    unittest.main()   
