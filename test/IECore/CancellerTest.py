##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
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

class CancellerTest( unittest.TestCase ) :

	def testCheck( self ) :

		IECore.Canceller.check( None )

		c = IECore.Canceller()
		IECore.Canceller.check( c )
		c.cancel()

		with self.assertRaises( IECore.Cancelled ) :
			IECore.Canceller.check( c )

	def testFinally( self ) :

		def f() :

			c = IECore.Canceller()
			c.cancel()

			didFinally = False
			try :
				IECore.Canceller.check( c )
			finally :
				didFinally = True

			self.assertEqual( didFinally, True )

		self.assertRaises( IECore.Cancelled, f )

	def testCancelledClass( self ) :

		self.assertIsInstance( IECore.Cancelled(), RuntimeError )
		self.assertEqual( IECore.Cancelled.__module__, "IECore" )

	def testExceptionTranslation( self ) :

		def raiser( key ) :

			if key == "python" :
				raise IECore.Cancelled()
			else :
				c = IECore.Canceller()
				c.cancel()
				IECore.Canceller.check( c )

		c = IECore.LRUCache( raiser, 1 )

		with self.assertRaises( IECore.Cancelled ) as a :

			# Expected sequence of events is as follows :
			#
			# 1. `get()` enters C++.
			# 2. C++ enters back into Python, calling `raiser()`.
			# 3. `raiser()` raises a _python_ exception containing a _python_
			#    `IECore.Cancelled` object.
			# 4. `LRUCacheGetter` (in LRUCacheBinding.cpp) calls `translatePythonException()` to turn the
			#    Python exception back into an appropriate C++ one - a true instance of `IECore::Cancelled`.
			# 5. The C++ exception propagates back to `get()`, which translates it back into Python,
			#    yielding us an instance of `IECore.Cancelled`.
			#
			# If 4 or 5 fail to perform the appropriate translation, we'll end up with a generic
			# exception being thrown, meaning we can no longer determine that cancellation occurred.
			# And then bad things happen.

			c.get( "python" )

		with self.assertRaises( IECore.Cancelled ) as a :

			# Expected sequence of events is even more
			# convoluted :
			#
			# 1. `get()` enters C++.
			# 2. C++ enters back into Python, calling `raiser()`.
			# 3. `raiser()` calls `Canceller.check()` which throws `Cancelled` from C++.
			# 4. The C++ exception is translated back into Python.
			# 5. `LRUCacheGetter` (in LRUCacheBinding.cpp) calls `translatePythonException()` to turn the
			#    Python exception back into an appropriate C++ one - a true instance of `IECore::Cancelled`.
			# 6. The C++ exception propagates back to `get()`, which translates it back into Python,
			#    yielding us an instance of `IECore.Cancelled`.

			c.get( "c++" )

if __name__ == "__main__":
	unittest.main()
