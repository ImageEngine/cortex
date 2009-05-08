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
import os.path
import IECore

class DisplayTest( unittest.TestCase ) :

	fileName = "test/IECore/data/display.cob"

	def test( self ) :

		c = IECore.Display()
		self.assertEqual( c.getName(), "default" )
		self.assertEqual( c.getType(), "exr" )
		self.assertEqual( c.getData(), "rgba" )
		self.assertEqual( c.parameters(), IECore.CompoundData() )

		cc = c.copy()
		self.assertEqual( cc.getName(), "default" )
		self.assertEqual( cc.getType(), "exr" )
		self.assertEqual( cc.getData(), "rgba" )
		self.assertEqual( cc.parameters(), IECore.CompoundData() )
		self.assertEqual( cc, c )

		IECore.Writer.create( cc, self.fileName ).write()
		ccc = IECore.Reader.create( self.fileName ).read()

		self.assertEqual( c, ccc )

		c.setName( "n" )
		self.assertEqual( c.getName(), "n" )

		c.setType( "t" )
		self.assertEqual( c.getType(), "t" )

		c.setData( "z" )
		self.assertEqual( c.getData(), "z" )

		c.parameters()["compression"] = IECore.StringData( "piz" )
		self.assertEqual( c.parameters()["compression"], IECore.StringData( "piz" ) )

		cc = c.copy()
		self.assertEqual( cc, c )

		IECore.Writer.create( cc, self.fileName ).write()
		ccc = IECore.Reader.create( self.fileName ).read()
		self.assertEqual( ccc, c )

	def tearDown( self ) :

		if os.path.isfile( self.fileName ) :
			os.remove( self.fileName )

if __name__ == "__main__":
        unittest.main()
