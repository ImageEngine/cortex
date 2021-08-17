##########################################################################
#
#  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

import os
import unittest
import IECore

class TestClassLoader( unittest.TestCase ) :

	def test( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) ) )

		self.assertEqual( l.classNames(), ["bad", "classParameterTest", "classVectorParameterTest", "colorSplineInput", "compoundObjectInOut", "floatParameter", os.path.join( "maths", "multiply" ), "matrixParameter", "mayaUserData", "meshMerge", "objectVectorInOut", "parameterTypes", os.path.join( "path.With.Dot", "multiply" ), "presetParsing", "splineInput", 'stringParsing', "unstorable" ] )
		self.assertEqual( l.classNames( "p*" ), ["parameterTypes", os.path.join( "path.With.Dot", "multiply" ), "presetParsing"] )
		self.assertEqual( l.getDefaultVersion( os.path.join( "maths", "multiply" ) ), 2 )
		self.assertEqual( l.getDefaultVersion( "presetParsing" ), 1 )
		self.assertEqual( l.getDefaultVersion( "stringParsing" ), 1 )
		self.assertEqual( l.versions( os.path.join( "maths", "multiply" ) ), [ 1, 2 ] )

		o = l.load( os.path.join( "maths", "multiply" ) )()
		self.assertEqual( len( o.parameters() ), 2 )

		self.assertEqual( l.versions( os.path.join( "maths", "multiply" ) ), [ 1, 2 ] )

	def testFinalSlash( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) + os.path.sep ) )
		self.assertEqual( l.classNames(), ["bad", "classParameterTest", "classVectorParameterTest", "colorSplineInput", "compoundObjectInOut", "floatParameter", os.path.join( "maths", "multiply" ), "matrixParameter", "mayaUserData", "meshMerge", "objectVectorInOut", "parameterTypes", os.path.join( "path.With.Dot", "multiply" ), "presetParsing", "splineInput", 'stringParsing', "unstorable" ] )

	def testStaticLoaders( self ) :

		l = IECore.ClassLoader.defaultOpLoader()
		ll = IECore.ClassLoader.defaultOpLoader()
		lll = IECore.ClassLoader.defaultLoader( "IECORE_OP_PATHS" )
		self.assertTrue( l is ll )
		self.assertTrue( l is lll )
		self.assertTrue( isinstance( l, IECore.ClassLoader ) )

	def testRefresh( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) ) )

		c = l.classNames()
		self.assertEqual( l.getDefaultVersion( os.path.join( "maths", "multiply" ) ), 2 )
		l.setDefaultVersion( os.path.join( "maths", "multiply" ), 1 )
		self.assertEqual( l.getDefaultVersion( os.path.join( "maths", "multiply" ) ), 1 )

		l.refresh()
		self.assertEqual( c, l.classNames() )
		self.assertEqual( l.getDefaultVersion( os.path.join( "maths", "multiply" ) ), 1 )

	def testDotsInPath( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) ) )

		c = l.load( os.path.join( "path.With.Dot", "multiply" ) )

	def testExceptions( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) ) )
		self.assertRaises( RuntimeError, l.getDefaultVersion, "thisOpDoesntExist" )
		self.assertRaises( RuntimeError, l.setDefaultVersion, "thisOpDoesntExist", 1 )
		self.assertRaises( TypeError, l.setDefaultVersion, os.path.join( "maths", "multiply" ), "iShouldBeAnInt" )
		self.assertRaises( RuntimeError, l.setDefaultVersion, os.path.join( "maths", "multiply" ), 10 )

	def testSearchPathAccessor( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) ) )
		self.assertEqual( l.searchPath(), IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) ) )

		# check a copy is returned so it can't be modified in place
		s = l.searchPath()
		s.paths = [ "a", "b", "c" ]
		self.assertEqual( l.searchPath(), IECore.SearchPath( os.path.join( "test", "IECore", "ops" ) ) )

if __name__ == "__main__":
        unittest.main()
