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

import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.OpenMaya as OpenMaya

class PythonTest( unittest.TestCase ) :

	def testKeywordMethodName( self ) :
			
		# "as" is a keyword in Python 2.5, and later versions of the
		# interpreter may be more restrictive regarding its use as a 
		# method name. Unfortunately MTime relies on its use, and this
		# has been observed as a potential candidate for rogue "SyntaxError"
		# exceptions
		c = OpenMaya.MTime( 1, OpenMaya.MTime.kSeconds )
		self.assertEqual( c.as( OpenMaya.MTime.kMilliseconds ), 1000 )
		
		# Currently, http://www.python.org/doc/2.5.1/ref/keywords.html states
		# that "as" is only recognized as a keyword once with_statement has been
		# imported		
		code = \
"""from __future__ import with_statement
oneSecond = OpenMaya.MTime( 1, OpenMaya.MTime.kSeconds )
milli = oneSecond.as( OpenMaya.MTime.kMilliseconds ), 1000 )
"""
		try :
			exec( code )
			self.assert_( False )
		except SyntaxError, e:
			self.assertEqual( str(e), "invalid syntax (<string>, line 3)" )
		except :
			self.assert_( False )
		

if __name__ == "__main__":
	MayaUnitTest.TestProgram()
