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

import IECore
import os

class TestFileSequenceParameter( unittest.TestCase ) :

	def mkSequence( self, sequence ) :
	
		directory = "test/sequences/parameterTest"
	
		os.system( "rm -rf " + directory )
		os.system( "mkdir -p " + directory )
		
		for f in sequence.fileNames() :
			os.system( "touch " + directory + "/" + f )

	def test( self ) :	
	
		s = IECore.FileSequence( "a.#.tif", IECore.FrameRange( 1, 10 ) )
		self.mkSequence( s )
		
		p = IECore.FileSequenceParameter( name = "n", description = "d", check = IECore.FileSequenceParameter.CheckType.MustExist )
		# should raise because it's not a valid sequence string
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.StringData( "hello" ) )
		# should raise because it doesn't exist
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.StringData( "hello.###.tif" ) )
		p.setValidatedValue( IECore.StringData( "test/sequences/parameterTest/a.#.tif" ) )
		
		p = IECore.FileSequenceParameter( name = "n", description = "d", check = IECore.FileSequenceParameter.CheckType.MustNotExist )
		# should raise because it's not a valid sequence string
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.StringData( "hello" ) )
		# should be fine because it's a valid string and no sequence like that exists
		p.setValidatedValue( IECore.StringData( "hello.###.tif" ) )
		# should raise because the sequence exists
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.StringData( "test/sequences/parameterTest/a.#.tif" ) )
		
		p = IECore.FileSequenceParameter( name = "n", description = "d", check = IECore.FileSequenceParameter.CheckType.DontCare )
		# should raise because it's not a valid sequence string
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.StringData( "hello" ) )
		p.setValidatedValue( IECore.StringData( "hello.###.tif" ) )
		p.setValidatedValue( IECore.StringData( "test/sequences/parameterTest/a.#.tif" ) )
		
		self.assertEqual( p.getFileSequenceValue(), IECore.ls( "test/sequences/parameterTest/a.#.tif" ) )
		p.setFileSequenceValue( IECore.FileSequence( "a.###.tif", IECore.FrameRange( 1, 10 ) ) )
		self.assertEqual( p.getValue(), IECore.StringData( "a.###.tif" ) )
		
	def testEmptyString( self ) :
	
		p = IECore.FileSequenceParameter( name = "n", description = "d", check = IECore.FileSequenceParameter.CheckType.MustExist, allowEmptyString=True )
		# should be fine, as we allow the empty string, and that should override the file existence checks
		p.setValidatedValue( IECore.StringData( "" ) )
		self.assertEqual( p.valueValid( IECore.StringData( "" ) )[0], True )
		
	def testNotAString( self ) :
	
		p = IECore.FileSequenceParameter( name = "n", description = "d" )
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.IntData( 1 ) )
		
		
if __name__ == "__main__":
        unittest.main()
