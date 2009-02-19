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

import math
import unittest
import datetime
import os
import shutil

from IECore import *

class SequenceLsOpTest( unittest.TestCase ) :

	def testConstruction( self ) :
	
		op = SequenceLsOp()
		
	def testModificationTime( self ) :
	
		now = datetime.datetime.now()
		oneHourAgo = now + datetime.timedelta( hours = -1 )
		
		s = FileSequence( "test/IECore/sequences/sequenceLsTest/s.#.tif", FrameRange( 1, 10 ) )
		os.system( "mkdir -p test/IECore/sequences/renumberTest" )
		
		for f in s.fileNames() :
			os.system( "touch '" + f + "'" )
			
		op = SequenceLsOp()	
		op['dir'] = StringData( "test/IECore/sequences/sequenceLsTest/" )
		op['contiguousSequencesOnly'] = True
		op['resultType'] = StringData( "stringVector" )
		op['advanced']['modificationTime']['enabled'] = True
		op['advanced']['modificationTime']['startTime'] = oneHourAgo
		op['advanced']['modificationTime']['mode'] = "before"		
		sequences = op()
		self.assertEqual( len(sequences), 0 )
		op['advanced']['modificationTime']['mode'] = "after"
		sequences = op()
		self.assertEqual( len(sequences), 1 )
		self.assertEqual( str( sequences[0] ), "test/IECore/sequences/sequenceLsTest/s.#.tif 1-10" )
		
		
	def setUp( self ) :
	
		if os.path.exists( "test/IECore/sequences/sequenceLsTest" ) :
			shutil.rmtree( "test/IECore/sequences/sequenceLsTest" )
			
		os.system( "mkdir -p test/IECore/sequences/sequenceLsTest" )
		
	def tearDown( self ) :
	
		if os.path.exists( "test/IECore/sequences/sequenceLsTest" ) :
			shutil.rmtree( "test/IECore/sequences/sequenceLsTest" )		
		
if __name__ == "__main__":
    unittest.main()   

