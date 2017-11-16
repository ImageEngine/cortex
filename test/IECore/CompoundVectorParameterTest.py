##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

class TestCompoundVectorParameter( unittest.TestCase ) :

	def testConstruction( self ) :

		c = IECore.CompoundVectorParameter( 'a', 'dest' )
		# test valid parameters
		c.addParameter( IECore.IntVectorParameter( 'a', '', IECore.IntVectorData( [ 1, 2 ] ) ) )
		c.addParameter( IECore.BoolVectorParameter( 'b', '', IECore.BoolVectorData( [ False, False ] ) ) )
		c.addParameters( [ IECore.V2fVectorParameter( 'c', '', IECore.V2fVectorData( [ IECore.V2f(), IECore.V2f() ] ) ),
							IECore.StringVectorParameter( 'd', '', IECore.StringVectorData( [ 'one', 'two' ] ) ) ] )

		self.assertEqual( len(c.keys()), 4 )

		def addInvalid():
			c.addParameter( IECore.StringParameter( 'xx', '', 'value' ) )

		# test invalid parameters
		self.assertRaises( TypeError, addInvalid )

	def testValidation( self ):

		c = IECore.CompoundVectorParameter( 'a', 'dest' )
		c.addParameter( IECore.IntVectorParameter( 'a', '', IECore.IntVectorData( [ 1, 2 ] ) ) )
		c.addParameter( IECore.BoolVectorParameter( 'b', '', IECore.BoolVectorData( [ False, False ] ) ) )

		c.validate()

		c.addParameter( IECore.IntVectorParameter( 'c', '', IECore.IntVectorData( [ 1, 2,3 ] ) ) )

		self.assertRaises( Exception, c.validate )
		try :
			c.validate()
		except Exception, e :
			self.failUnless(
				( 'Parameter "c" has wrong size ( expected 2 but found 3 )' in str( e ) ) or
				( 'Parameter "a" has wrong size ( expected 3 but found 2 )' in str( e ) ) or
				( 'Parameter "b" has wrong size ( expected 3 but found 2 )' in str( e ) )
			)

if __name__ == "__main__":
	unittest.main()

