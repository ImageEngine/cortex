##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2012, John Haddon. All rights reserved.
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
import shlex

class testParameterParser( unittest.TestCase ) :

	def testMultiply( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) )
		a = l.load( "maths/multiply" )()

		p = a.parameters()
		IECore.ParameterParser().parse( ["-a", "10", "-b", "20" ], p )

		self.assertEqual( a(), IECore.IntData( 200 ) )

	def testParameterTypes( self ) :

		a = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) ).load( "parameterTypes" )()

		IECore.ParameterParser().parse( [
			"-a", "10",
			"-b", "20.2",
			"-c", "40.5",
			"-d", "hello",
			"-e", "2", "4", "5",
			"-f", "one", "two", "three",
			"-g", "2", "4",
			"-h", "1", "4", "8",
			"-i", "2", "4",
			"-compound.j", "1", "4", "8",
			"-compound.k", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
			"-l", "1", "0", "0",
			"-m", "1", "1", "0", "1",
			"-o", "myFile.tif",
			"-p", "test",
			"-q", "true",
			"-r", "mySequence.####.tif",
			"-s", "-1", "-2", "10", "20",
			"-t", "-1", "-2", "-3", "10", "20", "30",
			"-u", "64", "128",
			"-v", "25", "26", "27",
			"-w", "0-500x250",
			"-x", '0', '0', '0', '1', '1', '1', '0', '0', '0', '0', '0', '0', 'Default', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
			"-y", '0', '0', '0', '1', '1', '1', '0', '0', '0', '0', '0', '0', 'Default', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',

			], a.parameters() )

		a()

	def testPresetParsing( self ) :

		a = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) ).load( "presetParsing" )()

		IECore.ParameterParser().parse( [
			"-h", "x",
			"-compound", "one",
			], a.parameters() )

		a()

	def testReadParsing( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) )
		a = l.load( "maths/multiply" )()

		p = a.parameters()
		IECore.ParameterParser().parse( ["-a", "read:test/IECore/data/cobFiles/intDataTen.cob", "-b", "30" ], p )

		self.assertEqual( a(), IECore.IntData( 300 ) )

	def testSerialising( self ) :

		a = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) ).load( "parameterTypes" )()

		IECore.ParameterParser().parse( [
			"-a", "10",
			"-b", "20.2",
			"-c", "40.5",
			"-d", "hello",
			"-e", "2", "4", "5",
			"-f", "one", "two", "three",
			"-g", "2", "4",
			"-h", "1", "4", "8",
			"-i", "2", "4",
			"-compound.j", "1", "4", "8",
			"-compound.k", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
			"-l", "1", "0", "0",
			"-m", "1", "1", "0", "1",
			"-o", "myFile.tif",
			"-p", "test",
			"-q", "true",
			"-r", "mySequence.####.tif",
			"-s", "-1", "-2", "10", "20",
			"-t", "-1", "-2", "-3", "10", "20", "30",
			"-u", "64", "128",
			"-v", "25", "26", "27",
			"-w", "0-500x250",
			"-x", '0', '0', '0', '1', '1', '1', '0', '0', '0', '0', '0', '0', 'Default', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
			"-y", '0', '0', '0', '1', '1', '1', '0', '0', '0', '0', '0', '0', 'Default', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
			], a.parameters() )

		a()

		# remove some parameters that don't have serializing/parsing methods yet.
		for name in [ 'p1', 'p2', 'p3', 'p4', 'p5', 'p6' ] :
			a.parameters().removeParameter( name )

		s = IECore.ParameterParser().serialise( a.parameters() )
		a = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) ).load( "parameterTypes" )()
		IECore.ParameterParser().parse( s, a.parameters() )

		a()

		# test alternate serialisation (without validation)
		notValidated = IECore.ParameterParser().serialise( a.parameters(), values = a.parameters().getValue() )
		IECore.ParameterParser().parse( notValidated, a.parameters() )

		a()

	def testSerialisingNonValidParameterValues( self ) :

		p = IECore.FileNameParameter(
			name = "f",
			description = "d",
			extensions = "cob scc fio pdc",
			check = IECore.FileNameParameter.CheckType.MustExist,
		)

		p.setValue( IECore.StringData( "test" ) )
		self.assertRaises( RuntimeError, p.getValidatedValue )
		self.assertRaises( RuntimeError, IECore.curry( IECore.ParameterParser().serialise, p ) )
		s = IECore.ParameterParser().serialise( p, values = p.getValue() )
		self.assertRaises( SyntaxError, IECore.curry( IECore.ParameterParser().parse, s, p ) )

		realImage = IECore.StringData( "test/IECore/data/cobFiles/ball.cob" )
		s = IECore.ParameterParser().serialise( p, values = realImage )
		self.assertRaises( RuntimeError, p.getValidatedValue )
		IECore.ParameterParser().parse( s, p )
		self.assertEqual( p.getValidatedValue(), realImage )

	def testStringParsing( self ) :

		a = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) ).load( "stringParsing" )()

		IECore.ParameterParser().parse( [
			"-emptyString", "",
			"-normalString", "hello",
			"-stringWithSpace", "hello there",
			"-stringWithManySpaces", "hello there old chap",
			], a.parameters() )

		a()

		a = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) ).load( "stringParsing" )()

		IECore.ParameterParser().parse( shlex.split("-emptyString '' -normalString 'hello' -stringWithSpace 'hello there' -stringWithManySpaces 'hello there old chap'"), a.parameters() )

		a()

	def testFlaglessParsing( self ) :

		parameters = IECore.CompoundParameter(

			members = [

				IECore.IntParameter(
					name = "a",
					description = "",
					defaultValue = 1
				),
				IECore.StringParameter(
					name = "b",
					description = "",
					defaultValue = "2"
				),
				IECore.IntParameter(
					name = "c",
					description = "",
					defaultValue = 3
				),

			],

			userData = {

				"parser" : {

					"flagless" : IECore.StringVectorData( [ "b", "c" ] )

				}

			}

		)

		# check that normal parsing still works

		IECore.ParameterParser().parse( [
				"-a", "10",
				"-b", "hello",
				"-c", "4"
			],
			parameters
		)

		self.assertEqual( parameters["a"].getNumericValue(), 10 )
		self.assertEqual( parameters["b"].getTypedValue(), "hello" )
		self.assertEqual( parameters["c"].getNumericValue(), 4 )

		# check that flagless parsing works

		IECore.ParameterParser().parse( [
				"-a", "15",
				"goodbye", "20"
			],
			parameters
		)

		self.assertEqual( parameters["a"].getNumericValue(), 15 )
		self.assertEqual( parameters["b"].getTypedValue(), "goodbye" )
		self.assertEqual( parameters["c"].getNumericValue(), 20 )

		# check that invalid stuff is still detected

		self.assertRaises( SyntaxError, IECore.ParameterParser().parse,
			[ "-iDontExist", "10" ],
			parameters,
		)

		self.assertRaises( SyntaxError, IECore.ParameterParser().parse,
			[ "-a" ],
			parameters,
		)

		self.assertRaises( SyntaxError, IECore.ParameterParser().parse,
			[ "too", "2", "many", "flaglessValues" ],
			parameters,
		)

	def testOptionalSerialisation( self ) :

		parameters = IECore.CompoundParameter(

			members = [

				IECore.IntParameter(
					name = "a",
					description = "",
					defaultValue = 1
				),
				IECore.StringParameter(
					name = "b",
					description = "",
					defaultValue = "2",
					userData = { "parser" : { "serialise" : IECore.BoolData( True ) } }
				),
				IECore.IntParameter(
					name = "c",
					description = "",
					defaultValue = 3,
					userData = { "parser" : { "serialise" : IECore.BoolData( False ) } }
				),

			]
		)

		s = IECore.ParameterParser().serialise( parameters )

		self.failIf( "-a" not in s )
		self.failIf( "-b" not in s )
		self.failIf( "-c" in s )

	def testEvalParsing( self ) :

		l = IECore.ClassLoader( IECore.SearchPath( "test/IECore/ops", ":" ) )
		a = l.load( "maths/multiply" )()

		p = a.parameters()
		IECore.ParameterParser().parse( ["-a", "python:IECore.IntData( 20 )", "-b", "30" ], p )
		self.assertEqual( p["a"].getValue(), IECore.IntData( 20 ) )
		self.assertEqual( a(), IECore.IntData( 600 ) )

	def testSplineParsing( self ) :

		p = IECore.CompoundParameter(
			members = [
				IECore.SplineffParameter(
					name = "a",
					description = "d",
					defaultValue = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.catmullRom(), ( ( 0, 0 ), ( 1, 1 ) ) ) ),
				),
			]
		)

		s = IECore.ParameterParser().serialise( p )
		v = p["a"].getValue().copy()

		p["a"].setValue( IECore.SplineffData() )
		self.assertNotEqual( p["a"].getValue(), v )

		IECore.ParameterParser().parse( s, p )

		self.assertEqual( p["a"].getValue(), v )

	def testTransformationMatrixParsing( self ) :

		p = IECore.CompoundParameter(
			members = [
				IECore.TransformationMatrixfParameter(
					name = "t",
					description = "d",
					defaultValue = IECore.TransformationMatrixf(),
				),
			]
		)

		args = [ "-t", '1', '2', '3', '10', '11', '12', '4', '5', '6', '7', '8', '9', 'ZYX', '1', '21', '22', '23', '26', '27', '28', '36', '37', '38', '46', '47', '48', '56', '57', '58' ]
		IECore.ParameterParser().parse( args, p )

		t = p["t"].getTypedValue()
		self.assertEqual( t.translate,IECore.V3f( 1,2,3 ) )
		self.assertEqual( t.scale,IECore.V3f( 10,11,12 ) )
		self.assertEqual( t.shear, IECore.V3f( 4,5,6 ) )
		self.assertEqual( t.rotate, IECore.V3f( 7,8,9 ) )
		self.assertEqual( t.rotate.order(),IECore.Eulerf.Order.ZYX )
		self.assertEqual( t.rotationOrientation, IECore.Quatf( 1,21,22,23 ) )
		self.assertEqual( t.rotatePivot, IECore.V3f( 26,27,28 ) )
		self.assertEqual( t.rotatePivotTranslation, IECore.V3f( 36,37,38 ) )
		self.assertEqual( t.scalePivot, IECore.V3f( 46,47,48 ) )
		self.assertEqual( t.scalePivotTranslation, IECore.V3f( 56,57,58 ) )

	def testLineSegmentParsing( self ) :

		p = IECore.CompoundParameter(

			members = [

				IECore.LineSegment3fParameter(
					name = "f",
					description = "",
					defaultValue = IECore.LineSegment3f( IECore.V3f( 1 ), IECore.V3f( 2 ) )
				),

				IECore.LineSegment3dParameter(
					name = "d",
					description = "",
					defaultValue = IECore.LineSegment3d( IECore.V3d( 1 ), IECore.V3d( 2 ) )
				),

			]

		)

		args = "-f 1 2 3 4 5 6 -d 6 5 4 3 2 1".split()
		IECore.ParameterParser().parse( args, p )

		self.assertEqual( p["f"].getTypedValue(), IECore.LineSegment3f( IECore.V3f( 1, 2, 3 ), IECore.V3f( 4, 5, 6 ) ) )
		self.assertEqual( p["d"].getTypedValue(), IECore.LineSegment3d( IECore.V3d( 6, 5, 4 ), IECore.V3d( 3, 2, 1 ) ) )

		self.assertEqual( IECore.ParameterParser().serialise( p ), args )

	def testDatetimeParsing( self ) :

		import datetime

		now = datetime.datetime.now()
		now = now.replace( microsecond = 0 )

		p = IECore.CompoundParameter(
			members = [
					IECore.DateTimeParameter(
						name = "testName",
						description = "testName description",
						defaultValue = now
					),
				]
		)

		s = IECore.ParameterParser().serialise( p )
		v = p["testName"].getValue().copy()

		p["testName"].setValue( IECore.DateTimeData() )
		self.assertNotEqual( p["testName"].getValue(), v )

		IECore.ParameterParser().parse( s, p )

		self.assertEqual( p["testName"].getValue(), v )

		IECore.ParameterParser().parse( ["-testName", "2009-02-13" ], p )
		IECore.ParameterParser().parse( ["-testName", "2009-02-13 10:42:13" ], p )
		IECore.ParameterParser().parse( ["-testName", "2009-02-13 10:42" ], p )

		IECore.ParameterParser().parse( ["-testName", "10:42" ], p )
		IECore.ParameterParser().parse( ["-testName", "18:12:32" ], p )

	def testObjectParsing( self ) :

		defaultMesh = IECore.MeshPrimitive(
			IECore.IntVectorData( [ 4 ] ), IECore.IntVectorData( [ 0, 1, 2, 3 ] ), "linear",
			IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ) ] )
		)

		p = IECore.CompoundParameter(
			members = [
				IECore.MeshPrimitiveParameter(
					name = "testName",
					description = "testName description",
					defaultValue = defaultMesh,
				),
			]
		)

		s = IECore.ParameterParser().serialise( p )
		v = p["testName"].getValue().copy()

		testMesh = IECore.MeshPrimitive(
			IECore.IntVectorData( [ 4 ] ), IECore.IntVectorData( [ 0, 2, 1, 3 ] ), "linear",
			IECore.V3fVectorData( [ IECore.V3f( 10 ), IECore.V3f( 20 ), IECore.V3f( 30 ), IECore.V3f( 40 ) ] )
		)
		p["testName"].setValue( testMesh )
		self.assertNotEqual( p["testName"].getValue(), v )

		IECore.ParameterParser().parse( s, p )

		self.assertEqual( p["testName"].getValue(), v )

	def testEmptyVector( self ) :
		""" Test that serializing then parsing a vector with no elements in it succeeds """

		p = IECore.StringVectorParameter( name = "name", description = "description", defaultValue = IECore.StringVectorData() )
		s = IECore.ParameterParser().serialise( p )
		IECore.ParameterParser().parse( s, p )

	def testNoValueProvidedSyntaxError( self ) :

		p = IECore.CompoundParameter(
			members = [
				IECore.StringParameter(
					name = "string",
					description = "d",
					defaultValue = '',
				),
				IECore.IntParameter(
					name = "int",
					description = "d",
					defaultValue = 0,
				),
				IECore.FloatParameter(
					name = "float",
					description = "d",
					defaultValue = 0.0,
				),
				IECore.V2iParameter(
					name = "v2i",
					description = "d",
					defaultValue = IECore.V2i( 0 ),
				),
				IECore.Box3fParameter(
					name = "box3f",
					description = "d",
					defaultValue = IECore.Box3f( IECore.V3f( 0 ), IECore.V3f( 1 ) ),
				),
				IECore.SplineffParameter(
					name = "spline",
					description = "d",
					defaultValue = IECore.SplineffData( IECore.Splineff( IECore.CubicBasisf.catmullRom(), ( ( 0, 0 ), ( 1, 1 ) ) ) ),
				),
			]
		)

		parser = IECore.ParameterParser()

		self.assertRaises( SyntaxError, parser.parse, ["-string"], p )
		self.assertRaises( SyntaxError, parser.parse, ["-int"], p )
		self.assertRaises( SyntaxError, parser.parse, ["-float"], p )
		self.assertRaises( SyntaxError, parser.parse, ["-v21"], p )
		self.assertRaises( SyntaxError, parser.parse, ["-box3f"], p )
		self.assertRaises( SyntaxError, parser.parse, ["-spline"], p )

	def testDerivedClassParsing( self ) :

		class DerivedParameter( IECore.StringParameter ) :

			def __init__( self, name, description, defaultValue ) :

				IECore.StringParameter.__init__( self, name, description, defaultValue )

		IECore.registerRunTimeTyped( DerivedParameter )

		p = IECore.CompoundParameter(
			members = [
				DerivedParameter( "n", "", "" ),
			],
		)

		p["n"].setTypedValue( "test" )

		s = IECore.ParameterParser().serialise( p )

		p["n"].setTypedValue( "ohDear" )

		IECore.ParameterParser().parse( s, p )

		self.assertEqual( p["n"].getTypedValue(), "test" )

	def testStringVectorParameterAcceptsFlags( self ) :

		p = IECore.CompoundParameter(
			members = [
				IECore.StringVectorParameter(
					"s",
					"",
					IECore.StringVectorData(),
				),
			],
		)

		self.assertRaises( SyntaxError, IECore.ParameterParser().parse, [ "-s", "something", "-ohNoAFlag" ], p )

		p["s"].userData()["parser"] = IECore.CompoundObject( { "acceptFlags" : IECore.BoolData( True ) } )

		IECore.ParameterParser().parse( [ "-s", "something", "-flagsAreFine" ], p )

		self.assertEqual( p["s"].getValue(), IECore.StringVectorData( [ "something", "-flagsAreFine" ] ) )

	def testBooleanParsingWithoutValues( self ) :

		p = IECore.CompoundParameter(
			members = [
				IECore.BoolParameter(
					"b",
					"",
					False
				),
				IECore.StringParameter(
					"s",
					"",
					""
				),
			],
		)

		IECore.ParameterParser().parse( [ "-b", "-s", "stringValue" ], p )

		self.assertEqual( p["b"].getTypedValue(), True )
		self.assertEqual( p["s"].getTypedValue(), "stringValue" )

	def testBooleanParsingWithoutValuesAndWithFlaglessArgs( self ) :

		parameters = IECore.CompoundParameter(

			members = [

				IECore.BoolParameter(
					name = "a",
					description = "",
					defaultValue = False
				),
				IECore.StringParameter(
					name = "b",
					description = "",
					defaultValue = "2"
				),
				IECore.IntParameter(
					name = "c",
					description = "",
					defaultValue = 3
				),

			],

			userData = {

				"parser" : {

					"flagless" : IECore.StringVectorData( [ "b", "c" ] )

				}

			}

		)

		# check that parsing with a specific value works

		IECore.ParameterParser().parse( [
				"-a", "True",
				"goodbye", "20"
			],
			parameters
		)

		self.assertEqual( parameters["a"].getTypedValue(), True )
		self.assertEqual( parameters["b"].getTypedValue(), "goodbye" )
		self.assertEqual( parameters["c"].getNumericValue(), 20 )

		# check that parsing without a value works too

		parameters["a"].setTypedValue( False )

		IECore.ParameterParser().parse( [
				"-a",
				"hello", "22"
			],
			parameters
		)

		self.assertEqual( parameters["a"].getTypedValue(), True )
		self.assertEqual( parameters["b"].getTypedValue(), "hello" )
		self.assertEqual( parameters["c"].getNumericValue(), 22 )

if __name__ == "__main__":
        unittest.main()

