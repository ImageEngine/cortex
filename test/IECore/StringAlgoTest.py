##########################################################################
#
#  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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
import six

import IECore

class StringAlgoTest( unittest.TestCase ) :

	def testMatch( self ) :

		for s, p, r in [
			( "", "", True ),
			( "a", "a", True ),
			( "a", "*", True ),
			( "ab", "a*", True ),
			( "cat", "dog", False ),
			( "dogfish", "*fish", True ),
			( "dogcollar", "*fish", False ),
			( "dog collar", "dog collar", True ),
			( "dog collar", "dog co*", True ),
			( "dog collar", "dog *", True ),
			( "dog collar", "dog*", True ),
			( "cat", "ca?", True ),
			( "", "?", False ),
			( "?", "?", True ),
			( "a", "[abc]", True ),
			( "catA", "cat[ABC]", True ),
			( "catD", "cat[A-Z]", True ),
			( "cars", "ca[rb]s", True ),
			( "cabs", "ca[rb]s", True ),
			( "cats", "ca[rb]s", False ),
			( "catD", "cat[CEF]", False ),
			( "catD", "cat[!CEF]", True ),
			( "cars", "ca[!r]s", False ),
			( "cabs", "ca[!r]s", True ),
			( "catch22", "c*[0-9]2", True ),
			( "x", "[0-9]", False ),
			( "x", "[!0-9]", True ),
			( "x", "[A-Za-z]", True ),
			( "[", "[a]", False ),
			# We should treat a leading or trailing
			# '-' as a regular character and not
			# a range specifier.
			( "_", "[-|]", False ),
			( "_", "[!-|]", True ),
			( "-", "[!-]", False ),
			( "x-", "x[d-]", True ),
			( "hyphen-ated", "*[-]ated", True ),
			# The following are mildly confusing, because we
			# must type two backslashes to end up with a single
			# backslash in the string literals we're constructing.
			( "\\", "\\\\", True ),   # \ matches \\
			( "d\\", "d\\\\", True ), # d\ matches d\\
			( "*", "\\*", True ),     # * matches \*
			( "a*", "a\\*", True ),   # a* matches a\*
			( "a", "\\a", True ),     # a matches \a
			( "\\", "\\x", False ),   # \ doesn't match \x
			( "?", "\\?", True ),     # ? matches \?
		] :

			if r :
				self.assertTrue( IECore.StringAlgo.match( s, p ), '"{0}" should match "{1}"'.format( s, p ) )
			else :
				self.assertFalse( IECore.StringAlgo.match( s, p ), '"{0}" shouldn\'t match "{1}"'.format( s, p ) )

			if " " not in s :
				self.assertEqual( IECore.StringAlgo.matchMultiple( s, p ), r )

	def testMatchMultiple( self ) :

		for s, p, r in [
			( "", "", True ),
			( "", "a", False ),
			( "", "a b", False ),
			( "a", "b a", True ),
			( "a", "c *", True ),
			( "ab", "c a*", True ),
			( "cat", "dog fish", False ),
			( "cat", "cad cat", True ),
			( "cat", "cad ", False ),
			( "cat", "cat ", True ),
			( "cat", "cadcat", False ),
			( "dogfish", "cat *fish", True ),
			( "dogcollar", "dog *fish", False ),
			( "dogcollar", "dog collar", False ),
			( "a1", "*1 b2", True ),
			( "abc", "a*d abc", True ),
			( "a", "a? a", True ),
			( "ab", "x? ab", True ),
			( "ab", "?x ab", True ),
			( "a1", "\\x a1", True ),
			( "R", "[RGB] *.[RGB]", True ),
			( "R", "[x] R", True ),
			( "diffuse.R", "[RGB] *.[RGB]", True ),
			( "diffuse.A", "[RGB] *.[RGB]", False ),
			( "bb", "*a b", False ),
			( "bb", "*a bb", True ),
			( "bb", "*a    bb", True ),
		] :

			if r :
				self.assertTrue( IECore.StringAlgo.matchMultiple( s, p ), '"{0}" should match "{1}"'.format( s, p ) )
			else :
				self.assertFalse( IECore.StringAlgo.matchMultiple( s, p ), '"{0}" shouldn\'t match "{1}"'.format( s, p ) )

	def testHasWildcards( self ) :

		for p, r in [
			( "", False ),
			( "a", False ),
			( "*", True ),
			( "a*", True ),
			( "a**", True ),
			( "a*b", True ),
			( "*a", True ),
			( "\\", True ),
			( "?", True ),
			( "\\?", True ),
			( "[abc]", True ),
		] :

			if r :
				self.assertTrue( IECore.StringAlgo.hasWildcards( p ), "{0} has wildcards".format( p ) )
			else :
				self.assertFalse( IECore.StringAlgo.hasWildcards( p ), "{0} doesn't have wildcards".format( p ) )

	def testMatchPaths( self ) :

		self.assertTrue( IECore.StringAlgo.match( [ "a", "b", "c" ], [ "a", "b", "c" ] ) )
		self.assertTrue( IECore.StringAlgo.match( [ "a", "b", "c" ], [ "a", "b", "*" ] ) )
		self.assertTrue( IECore.StringAlgo.match( [ "a", "b", "c" ], [ "*", "*", "*" ] ) )

		self.assertFalse( IECore.StringAlgo.match( [ "a", "b", "c" ], [ "a", "b", "d" ] ) )
		self.assertFalse( IECore.StringAlgo.match( [ "a", "b", "c" ], [ "*" ] ) )

	def testMatchPatternPath( self ) :

		self.assertEqual( IECore.StringAlgo.matchPatternPath( "/a/.../b*/d" ), [ "a", "...", "b*", "d" ] )
		self.assertEqual( IECore.StringAlgo.matchPatternPath( "" ), [] )
		self.assertEqual( IECore.StringAlgo.matchPatternPath( "a.b.c", separator = "." ), [ "a", "b", "c" ] )
		self.assertEqual( IECore.StringAlgo.matchPatternPath( "a...b", separator = "." ), [ "a", "...", "b" ] )

	def testSubstitute( self ) :

		d = {
			"frame" : 20,
			"a" : "apple",
			"b" : "bear",
		}

		self.assertEqual( IECore.StringAlgo.substitute( "$a/$b/something.###.tif", d ), "apple/bear/something.020.tif" )
		self.assertEqual( IECore.StringAlgo.substitute( "$a/$dontExist/something.###.tif", d ), "apple//something.020.tif" )
		self.assertEqual( IECore.StringAlgo.substitute( "${badlyFormed", d ), "" )

	def testSubstituteTildeInMiddle( self ) :

		self.assertEqual( IECore.StringAlgo.substitute( "a~b", {} ), "a~b" )

	def testSubstituteWithMask( self ) :

		d = {
			"frame" : 20,
			"a" : "apple",
			"b" : "bear",
		}

		self.assertEqual( IECore.StringAlgo.substitute( "~", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.TildeSubstitutions ), "~" )
		self.assertEqual( IECore.StringAlgo.substitute( "#", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.FrameSubstitutions ), "#" )
		self.assertEqual( IECore.StringAlgo.substitute( "$a/${b}", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.VariableSubstitutions ), "$a/${b}" )
		self.assertEqual( IECore.StringAlgo.substitute( "\\", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.EscapeSubstitutions ), "\\" )
		self.assertEqual( IECore.StringAlgo.substitute( "\\$a", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.EscapeSubstitutions ), "\\apple" )
		self.assertEqual( IECore.StringAlgo.substitute( "#${a}", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.FrameSubstitutions ), "#apple" )
		self.assertEqual( IECore.StringAlgo.substitute( "#${a}", d, IECore.StringAlgo.Substitutions.NoSubstitutions ), "#${a}" )

	def testFrameAndVariableSubstitutionsAreDifferent( self ) :

		d = { "frame" : 3 }

		# Turning off variable substitutions should have no effect on '#' substitutions.
		self.assertEqual( IECore.StringAlgo.substitute( "###.$frame", d ), "003.3" )
		self.assertEqual( IECore.StringAlgo.substitute( "###.$frame", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.VariableSubstitutions ), "003.$frame" )

		# Turning off '#' substitutions should have no effect on variable substitutions.
		self.assertEqual( IECore.StringAlgo.substitute( "###.$frame", d ), "003.3" )
		self.assertEqual( IECore.StringAlgo.substitute( "###.$frame", d, IECore.StringAlgo.Substitutions.AllSubstitutions & ~IECore.StringAlgo.Substitutions.FrameSubstitutions ), "###.3" )

	def testSubstitutions( self ) :

		self.assertEqual( IECore.StringAlgo.substitutions( "a" ), IECore.StringAlgo.Substitutions.NoSubstitutions )
		self.assertEqual( IECore.StringAlgo.substitutions( "~/something" ), IECore.StringAlgo.Substitutions.TildeSubstitutions )
		self.assertEqual( IECore.StringAlgo.substitutions( "$a" ), IECore.StringAlgo.Substitutions.VariableSubstitutions )
		self.assertEqual( IECore.StringAlgo.substitutions( "${a}" ), IECore.StringAlgo.Substitutions.VariableSubstitutions )
		self.assertEqual( IECore.StringAlgo.substitutions( "###" ), IECore.StringAlgo.Substitutions.FrameSubstitutions )
		self.assertEqual( IECore.StringAlgo.substitutions( "\\#" ), IECore.StringAlgo.Substitutions.EscapeSubstitutions )
		self.assertEqual( IECore.StringAlgo.substitutions( "${a}.###" ), IECore.StringAlgo.Substitutions.VariableSubstitutions | IECore.StringAlgo.Substitutions.FrameSubstitutions )

	def testHasSubstitutions( self ) :

		self.assertFalse( IECore.StringAlgo.hasSubstitutions( "a" ) )
		self.assertTrue( IECore.StringAlgo.hasSubstitutions( "~something" ) )
		self.assertTrue( IECore.StringAlgo.hasSubstitutions( "$a" ) )
		self.assertTrue( IECore.StringAlgo.hasSubstitutions( "${a}" ) )
		self.assertTrue( IECore.StringAlgo.hasSubstitutions( "###" ) )

	def testEscapedSubstitutions( self ) :

		d = {
			"frame" : 20,
			"a" : "apple",
			"b" : "bear",
		}

		self.assertEqual( IECore.StringAlgo.substitute( "\\${a}.\\$b", d ), "${a}.$b" )
		self.assertEqual( IECore.StringAlgo.substitute( "\\~", d ), "~" )
		self.assertEqual( IECore.StringAlgo.substitute( "\\#\\#\\#\\#", d ), "####" )
		# really we're passing \\ to substitute and getting back \ -
		# the extra slashes are escaping for the python interpreter.
		self.assertEqual( IECore.StringAlgo.substitute( "\\\\", d ), "\\" )
		self.assertEqual( IECore.StringAlgo.substitute( "\\", d ), "" )

		self.assertTrue( IECore.StringAlgo.hasSubstitutions( "\\" ) ) # must return true, because escaping affects substitution
		self.assertTrue( IECore.StringAlgo.hasSubstitutions( "\\\\" ) ) # must return true, because escaping affects substitution

	def testCustomVariableProvider( self ) :

		class MyVariableProvider( IECore.StringAlgo.VariableProvider ) :

			def frame( self ) :

				return 10

			def variable( self, name ) :

				if name == "recurse" :
					return "$norecurse", True
				elif name == "norecurse" :
					return "$norecurse", False
				else :
					return name * 2

		v = MyVariableProvider()

		self.assertEqual( IECore.StringAlgo.substitute( "#$x", v ), "10xx" )
		self.assertEqual( IECore.StringAlgo.substitute( "${y}", v ), "yy" )
		self.assertEqual( IECore.StringAlgo.substitute( "${recurse}", v ), "$norecurse" )

	def testFrameSubstitutions( self ) :

		self.assertEqual(
			IECore.StringAlgo.substitute( "###", { "frame" : 1 } ),
			"001"
		)

		self.assertEqual(
			IECore.StringAlgo.substitute( "###", { "frame" : 2.1 } ),
			"002"
		)

		with six.assertRaisesRegex( self, IECore.Exception, "expected IntData or FloatData" ) :
			IECore.StringAlgo.substitute( "###", { "frame" : "notAFrame" } )

	def testMatchMultipleScaling( self ) :

		# This test exposed an appalling performance problem when performing matches
		# against multiple patterns containing '*'.

		self.assertFalse(
			IECore.StringAlgo.matchMultiple(
				"FTR01_0086_0023",
				"*54_00* *66_0010 *66_0020 *66_0040 *74_0010 *77_0010 *84_0020 *87_0035 *108_0040 *90_0015 *103_0030 *108_00* *77_0010 *91A_0020 *103_0015 *86_0010"
			)
		)

if __name__ == "__main__":
	unittest.main()
