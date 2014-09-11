##########################################################################
#
#  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

class ExternalProceduralTest( unittest.TestCase ) :

	def test( self ) :

		p = IECore.ExternalProcedural(
			"yeti.so",
			IECore.Box3f( IECore.V3f( 1, 2, 3 ), IECore.V3f( 4, 5, 6 ) ),
			IECore.CompoundData( {
				"one" : 1,
				"two" : 2,
			} )
		)

		self.assertEqual( p.getFileName(), "yeti.so" )
		self.assertEqual( p.getBound(), IECore.Box3f( IECore.V3f( 1, 2, 3 ), IECore.V3f( 4, 5, 6 ) ) )
		self.assertEqual(
			p.parameters(),
			IECore.CompoundData( {
				"one" : 1,
				"two" : 2,
			} )
		)

		p2 = p.copy()

		self.assertEqual( p2.getFileName(), "yeti.so" )
		self.assertEqual( p2.getBound(), IECore.Box3f( IECore.V3f( 1, 2, 3 ), IECore.V3f( 4, 5, 6 ) ) )
		self.assertEqual(
			p2.parameters(),
			IECore.CompoundData( {
				"one" : 1,
				"two" : 2,
			} )
		)

		self.assertEqual( p, p2 )
		self.assertEqual( p.hash(), p2.hash() )

		p2.setFileName( "yeti2.so" )
		self.assertEqual( p2.getFileName(), "yeti2.so" )

		self.assertNotEqual( p, p2 )
		self.assertNotEqual( p.hash(), p2.hash() )

		m = IECore.MemoryIndexedIO( IECore.CharVectorData(), [], IECore.IndexedIO.OpenMode.Append )

		p.save( m, "test" )

		p3 = IECore.Object.load( m, "test" )

		self.assertEqual( p3, p )

if __name__ == "__main__":
    unittest.main()
