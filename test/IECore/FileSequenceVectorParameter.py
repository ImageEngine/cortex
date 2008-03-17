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
import os

class TestFileSequenceVectorParameter( unittest.TestCase ) :

	def mkSequence( self, sequence ) :
	
		directory = "test/sequences/parameterTest"
		
		for f in sequence.fileNames() :
			os.system( "touch " + directory + "/" + f )

	def test( self ) :	
	
		s1 = IECore.FileSequence( "a.#.tif", IECore.FrameRange( 1, 10 ) )
		self.mkSequence( s1 )
		
		s2 = IECore.FileSequence( "b.#.tif", IECore.FrameRange( 5, 20 ) )
		self.mkSequence( s2 )
		
		p = IECore.FileSequenceVectorParameter( name = "n", description = "d", check = IECore.FileSequenceVectorParameter.CheckType.MustExist )
		
		# should raise because it's not a valid sequence string		
		t = IECore.StringVectorData()
		t.append( "hello" )
		self.assertRaises( RuntimeError, p.setValidatedValue, t )
		
		# should raise because it doesn't exist
		t = IECore.StringVectorData()
		t.append( "hello.###.tif" )
		self.assertRaises( RuntimeError, p.setValidatedValue, t )
		
		t = IECore.StringVectorData()
		t.append( "test/sequences/parameterTest/a.#.tif" )
		p.setValidatedValue( t )
		
		p = IECore.FileSequenceVectorParameter( name = "n", description = "d", check = IECore.FileSequenceVectorParameter.CheckType.MustNotExist )
		
		# should raise because it's not a valid sequence string		
		t = IECore.StringVectorData()
		t.append( "hello" )
		self.assertRaises( RuntimeError, p.setValidatedValue, t )
		
		# should be fine because it's a valid string and no sequence like that exists
		t = IECore.StringVectorData()
		t.append( "hello.###.tif" )
		p.setValidatedValue( t )
		
		# should raise because the sequence exists
		t = IECore.StringVectorData()
		t.append( "test/sequences/parameterTest/a.#.tif" )
		self.assertRaises( RuntimeError, p.setValidatedValue, t )
		
		p = IECore.FileSequenceVectorParameter( name = "n", description = "d", check = IECore.FileSequenceVectorParameter.CheckType.DontCare )
		# should raise because it's not a valid sequence string		
		t = IECore.StringVectorData()
		t.append( "hello" )
		self.assertRaises( RuntimeError, p.setValidatedValue, t )
		
		t = IECore.StringVectorData()
		t.append( "hello.###.tif" )
		p.setValidatedValue( t )
		
		t = IECore.StringVectorData()
		t.append( "test/sequences/parameterTest/a.#.tif" )
		t.append( "test/sequences/parameterTest/b.#.tif" )
		p.setValidatedValue( t )	
		
		fs = p.getFileSequenceValues()	
		
		self.assertEqual( len(fs), 2 )
		self.assertEqual( fs[0], IECore.ls( "test/sequences/parameterTest/a.#.tif" ) )
		self.assertEqual( fs[1], IECore.ls( "test/sequences/parameterTest/b.#.tif" ) )		
		
		p.setFileSequenceValues( [ IECore.FileSequence( "a.###.tif", IECore.FrameRange( 1, 10 ) ) ] )
		
		t = IECore.StringVectorData()
		t.append( "a.###.tif" )
		self.assertEqual( p.getValue(), t )
		
	def testEmptyString( self ) :
	
		p = IECore.FileSequenceVectorParameter( name = "n", description = "d", check = IECore.FileSequenceVectorParameter.CheckType.MustExist, allowEmptyList=True )
		# should be fine, as we allow the empty list, and that should override the file existence checks
		p.setValidatedValue( IECore.StringVectorData( ) )
		self.assertEqual( p.valueValid( IECore.StringVectorData( ) )[0], True )
		
	def testNotAStringVector( self ) :
	
		p = IECore.FileSequenceVectorParameter( name = "n", description = "d" )
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.IntData( 1 ) )
		
	def testExtensions( self ) :
	
		p = IECore.FileSequenceVectorParameter( name = "n", description = "d", check = IECore.FileSequenceVectorParameter.CheckType.DontCare, extensions="tif exr jpg" )
		self.assertEqual( p.extensions, [ "tif", "exr", "jpg" ] )
		
		t = IECore.StringVectorData()
		t.append( "a.#.tif" )
		self.assert_( p.valueValid( t )[0] )
		
		t = IECore.StringVectorData()
		t.append( "a.#.gif" )
		self.assert_( not p.valueValid( t )[0] )
		
		t = IECore.StringVectorData()
		t.append( "dsds.#" )
		self.assertRaises( RuntimeError, p.setValidatedValue, t )
		
		t = IECore.StringVectorData()
		t.append( "dsds.###.gif" )
		self.assertRaises( RuntimeError, p.setValidatedValue, t )
		
		t = IECore.StringVectorData()
		t.append( "dsds.###.tif" )
		p.setValidatedValue( t )
		
		
	def setUp( self ):
	
		directory = "test/sequences/parameterTest"
		
		if os.path.exists( directory ) :
		
			os.system( "rm -rf " + directory )
			
		os.system( "mkdir -p " + directory )
		
	def tearDown( self ):
	
		directory = "test/sequences/parameterTest"
		
		if os.path.exists( directory ) :
		
			os.system( "rm -rf " + directory )
	

		
if __name__ == "__main__":
        unittest.main()
