##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

class TestReader(unittest.TestCase):

	def testSupportedExtensions( self ) :

		e = IECore.Reader.supportedExtensions()
		for ee in e :
			self.assert_( type( ee ) is str )

		expectedExtensions = [ "pdc", "cob", "obj" ]

		for ee in expectedExtensions :
			self.assert_( ee in e )

		e = IECore.Reader.supportedExtensions( IECore.TypeId.ParticleReader )
		for ee in e :
			self.assert_( type( ee ) is str )

		self.assertEqual( set( [ "pdc", "mc" ] ), set( e ) )

		self.assert_( not "obj" in e )

	def test( self ) :

		"""
		check if we can create a reader from a blank file.
		this should definitely NOT create a valid reader
		"""

		self.assertRaises( RuntimeError, IECore.Reader.create, 'test/IECore/data/empty' )
		self.assertRaises( RuntimeError, IECore.Reader.create, 'test/IECore/data/null' )
		self.assertRaises( RuntimeError, IECore.Reader.create, 'test/IECore/data/null.cin' )

	def testCanRead( self ) :

		# every reader subclass should have a canRead() static method, unless it's an abstract base class

		def isReader( x ) :

			abstractReaders = ( IECore.Reader, IECore.ParticleReader )

			try :
				return issubclass( x, IECore.Reader ) and x not in abstractReaders
			except TypeError :
				return False

		allIECore = [ getattr( IECore, x ) for x in dir( IECore ) ]
		allReaders = [ x for x in allIECore if isReader( x ) ]

		for reader in allReaders :
			self.failUnless( hasattr( reader, "canRead" ) )

if __name__ == "__main__":
	unittest.main()

