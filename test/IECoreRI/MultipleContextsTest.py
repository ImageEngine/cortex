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

class MultipleContextsTest( unittest.TestCase ) :

	def test( self ) :
	
		r1 = IECoreRI.Renderer( "test/IECoreRI/output/contextOne.rib" )
		r2 = IECoreRI.Renderer( "test/IECoreRI/output/contextTwo.rib" )
		
		self.assertEqual( r1.getAttribute( "doubleSided" ), IECore.BoolData( True ) )
		self.assertEqual( r2.getAttribute( "doubleSided" ), IECore.BoolData( True ) )
		
		r1.setAttribute( "doubleSided", IECore.BoolData( False ) )
		self.assertEqual( r1.getAttribute( "doubleSided" ), IECore.BoolData( False ) )
		self.assertEqual( r2.getAttribute( "doubleSided" ), IECore.BoolData( True ) )
		r1.setAttribute( "doubleSided", IECore.BoolData( True ) )
		self.assertEqual( r1.getAttribute( "doubleSided" ), IECore.BoolData( True ) )
		self.assertEqual( r2.getAttribute( "doubleSided" ), IECore.BoolData( True ) )
		
		r2.setAttribute( "doubleSided", IECore.BoolData( False ) )
		self.assertEqual( r1.getAttribute( "doubleSided" ), IECore.BoolData( True ) )
		self.assertEqual( r2.getAttribute( "doubleSided" ), IECore.BoolData( False ) )
		
	def tearDown( self ) :
	
		for f in [ "contextOne.rib", "contextTwo.rib" ] :
			ff = "test/IECoreRI/output/" + f
			if os.path.exists( ff ) :
				os.remove( ff )
				
if __name__ == "__main__":
    unittest.main()   
