##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

class StringUtilTest( unittest.TestCase ) :

	def testCmdLineNoQuoting( self ):

		self.assertEqual( "test", IECore.StringUtil.quoteCmdLineArg( "test" ) )
		self.assertEqual( "-flag", IECore.StringUtil.quoteCmdLineArg( "-flag" ) )
		self.assertEqual( "read:/file.exr", IECore.StringUtil.quoteCmdLineArg( "read:/file.exr" ) )

	def testCmdLineSingleQuote( self ):

		self.assertEqual( "'string with spaces'", IECore.StringUtil.quoteCmdLineArg( "string with spaces" ) )

	def testCmdLineEmptyString( self ):

		# since the shell does not handle empty string parameters, we have to come up with a non-empty parameter to represent it.
		self.assertEqual( "''", IECore.StringUtil.quoteCmdLineArg( "" ) )

	def testCmdLineQuotingReversibility( self ) :

		args = [ "test", "", "-flag", "%", "<minor>", ".", "*", "&&", '"', "'", "python:ls", "read:/tmp/test file name.txt" ]

		self.assertEqual( IECore.StringUtil.unquoteCmdLine( IECore.StringUtil.quotedCmdLine( args ) ), args )

	def testCmdLineQuotingRecursiveness( self ):

		args = [ "simple", "", "read:file", "with spaces", "complex?(!@#$%*^()&_){}[]<>~`" ]

		# test recursive quoting on lists
		quotedArgs = args
		for i in range( 0, 3 ) :

			quotedArgs = IECore.StringUtil.quoteCmdLineArgs( quotedArgs )

		for i in range( 0, 3 ) :

			quotedArgs = IECore.StringUtil.unquoteCmdLineArgs( quotedArgs )

		self.assertEqual( quotedArgs, args )

		# test recursive quoting a single parameter
		arg = IECore.StringUtil.quotedCmdLine( args )

		quotedArg = arg
		for i in range( 0, 3 ) :
			quotedArg = IECore.StringUtil.quotedCmdLine( [ quotedArg ] )

		for i in range( 0, 3 ) :
			quotedArg = IECore.StringUtil.unquoteCmdLine( quotedArg )[0]

		self.assertEqual( quotedArg, arg )


if __name__ == "__main__":
    unittest.main()

