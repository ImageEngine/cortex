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
import sys
import IECore
import socket

class TestPDCWriter( unittest.TestCase ) :

	def testBasics( self ) :

		r = IECore.Reader.create( "test/IECore/data/cobFiles/compoundData.cob" )
		p = r.read()

		w = IECore.Writer.create( p, "test/compoundData.cob" )
		w.write()

		r = IECore.Reader.create( "test/compoundData.cob" )
		p2 = r.read()

		self.assertEqual( p, p2 )

	def testHeader( self ) :

		o = IECore.IntData()

		w = IECore.Writer.create( o, "test/intData.cob" )
		w.header.getValue()["testHeaderData"] = IECore.StringData( "i am part of a header" )
		w.header.getValue()["testHeaderData2"] = IECore.IntData( 100 )
		w.write()

		h = IECore.Reader.create( "test/intData.cob" ).readHeader()

		for k in w.header.getValue().keys() :
			self.assertEqual( w.header.getValue()[k], h[k] )

		self.assertEqual( h["host"].value, socket.gethostname() )
		self.assertEqual( h["ieCoreVersion"].value, IECore.versionString() )
		self.assertEqual( h["typeName"].value, "IntData" )

if __name__ == "__main__":
	unittest.main()

