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

class TestClassLoader( unittest.TestCase ) :

	def test( self ) :
	
		l = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) )
		
		self.assertEqual( l.classNames(), ["bad", "maths/multiply", "parameterTypes", "path.With.Dot/multiply", "presetParsing", 'stringParsing'] )
		self.assertEqual( l.classNames( "p*" ), ["parameterTypes", "path.With.Dot/multiply", "presetParsing"] )
		self.assertEqual( l.getDefaultVersion( "maths/multiply" ), 2 )
		self.assertEqual( l.getDefaultVersion( "presetParsing" ), 1 )
		self.assertEqual( l.getDefaultVersion( "stringParsing" ), 1 )
		self.assertEqual( l.versions( "maths/multiply" ), [ 1, 2 ] )
		
		o = l.load( "maths/multiply" )()
		self.assertEqual( len( o.parameters() ), 2 )
		
	def testStaticLoaders( self ) :
	
		l = IECore.ClassLoader.defaultOpLoader()
		ll = IECore.ClassLoader.defaultOpLoader()
		self.assert_( l is ll )
		self.assert_( isinstance( l, IECore.ClassLoader ) )
		
		l = IECore.ClassLoader.defaultProceduralLoader()
		ll = IECore.ClassLoader.defaultProceduralLoader()
		self.assert_( l is ll )
		self.assert_( isinstance( l, IECore.ClassLoader ) )
		
	def testRefresh( self ) :
	
		l = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) )
		
		c = l.classNames()
		self.assertEqual( l.getDefaultVersion( "maths/multiply" ), 2 )
		l.setDefaultVersion( "maths/multiply", 1 )
		self.assertEqual( l.getDefaultVersion( "maths/multiply" ), 1 )
		
		l.refresh()
		self.assertEqual( c, l.classNames() )
		self.assertEqual( l.getDefaultVersion( "maths/multiply" ), 1 )
		
	def testDotsInPath( self ) :
	
		l = IECore.ClassLoader( IECore.SearchPath( "test/ops", ":" ) )
		
		c = l.load( "path.With.Dot/multiply" )
		
if __name__ == "__main__":
        unittest.main()
