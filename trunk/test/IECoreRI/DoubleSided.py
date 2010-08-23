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

class DoubleSidedTest( unittest.TestCase ) :

	def test( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/testDoubleSided.rib" )
		self.assertEqual( r.getAttribute( "doubleSided" ), IECore.BoolData( True ) )

		r.setAttribute( "doubleSided", IECore.BoolData( False ) )
		self.assertEqual( r.getAttribute( "doubleSided" ), IECore.BoolData( False ) )
		del r

		l = "".join( file( "test/IECoreRI/output/testDoubleSided.rib" ).readlines() )
		self.assert_( "Sides 1" in l )

		r = IECoreRI.Renderer( "test/IECoreRI/output/testDoubleSided.rib" )
		r.setAttribute( "doubleSided", IECore.BoolData( True ) )
		del r

		l = "".join( file( "test/IECoreRI/output/testDoubleSided.rib" ).readlines() )
		self.assert_( "Sides 2" in l )

	def tearDown( self ) :

		if os.path.exists( "test/IECoreRI/output/testDoubleSided.rib" ) :
			os.remove( "test/IECoreRI/output/testDoubleSided.rib" )

if __name__ == "__main__":
    unittest.main()
