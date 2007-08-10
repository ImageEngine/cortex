##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
from IECore import *

class FileFormatSwitchTest( unittest.TestCase ) :

	"""This tests that we can still read SQLiteIndexedIO based .cob files,
	and that they give us the same results as when using the new format."""
	
	def test( self ) :
	
		sqlBasedCobs = [
			"test/data/cobFiles/intDataTenAsSQL.cob",
			"test/data/cobFiles/compoundDataAsSQL.cob",
		]
		
		for cob in sqlBasedCobs :
			
			# read the sql format
			o = ObjectReader( cob ).read()
			
			# write out in new format
			ObjectWriter( o, "/tmp/test.cob" ).write()
			
			# check that new file is not SQLite based
			self.assertRaises( Exception, SQLiteIndexedIO, "/tmp/test.cob" "/", IndexedIOOpenMode.Read )
			# check that the new is FileIndexedIO based
			f = FileIndexedIO( "/tmp/test.cob", "/", IndexedIOOpenMode.Read )
			
			# read back object and check equality
			oo = ObjectReader( "/tmp/test.cob" ).read()
			
			self.assertEqual( o, oo )
		
if __name__ == "__main__":
	unittest.main()
