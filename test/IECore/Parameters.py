##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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
import os

from IECore import *

class TestParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = Parameter( "name", "description", FloatData( 1 ) )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, FloatData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.getCurrentPresetName(), "" )
		self.assertEqual (p.userData(), CompoundObject() )

		v = IntData( 2 )
		p.setValue( v )
		self.assertEqual( p.getValue(), v )
		self.assertRaises( RuntimeError, Parameter, "name", "description", None )		# passing None as default value should raise exception as opposed to segfault!

	def testUserData( self ):
		compound = CompoundObject()
		compound["first"] = IntData()
		compound["second"] = QuatfData()
		compound["third"] = StringData("test")
		p = Parameter( "name", "description", FloatData( 1 ), userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = CharData('1')
		data["first"] = data["fourth"]

	def testKeywordConstructor( self ) :

		p = Parameter(
			name = "n",
			description = "d",
			defaultValue = FloatData( 20 )
		)

		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, FloatData( 20 ) )
		self.assertEqual( p.getValue(), p.defaultValue )

	def testPresets( self ) :

		# Presets as tuple
		p = Parameter(
			name = "n",
			description = "d",
			defaultValue = FloatData( 20 ),
			presets = (
				( "p1", FloatData( 40 ) ),
				( "p2", IntData( 60 ) ),
				( "p3", CompoundData() ),
				( "p4", FloatData( 20 ) ),
			),
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 4 )
		self.assertEqual( pr["p1"], FloatData( 40 ) )
		self.assertEqual( pr["p2"], IntData( 60 ) )
		self.assertEqual( pr["p3"], CompoundData() )
		self.assertEqual( pr["p4"], FloatData( 20 ) )

		for k, v in pr.items() :
			p.setValue( k )
			self.assertEqual( p.getValue(), v )
			self.assertEqual( p.getCurrentPresetName(), k )

		self.assertRaises( RuntimeError, p.setValue, "thisIsNotAPreset" )
		self.assertRaises( RuntimeError, p.setValidatedValue, FloatData( 1000 ) )

		# Presets as list
		p = Parameter(
			name = "n",
			description = "d",
			defaultValue = FloatData( 20 ),
			presets = [
				( "p1", FloatData( 40 ) ),
				( "p2", IntData( 60 ) ),
				( "p3", CompoundData() ),
				( "p4", FloatData( 20 ) ),
			],
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 4 )
		self.assertEqual( pr["p1"], FloatData( 40 ) )
		self.assertEqual( pr["p2"], IntData( 60 ) )
		self.assertEqual( pr["p3"], CompoundData() )
		self.assertEqual( pr["p4"], FloatData( 20 ) )

		# overriding presets

		p.setPresets( [] )
		self.assertEqual( p.getPresets(), dict() )

		p.setPresets(
			[
				( "p5", FloatData( 40 ) ),
				( "p1", IntData( 60 ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["p5"], FloatData( 40 ) )
		self.assertEqual( pr["p1"], IntData( 60 ) )
		self.assertEqual( p.presetNames(), ("p5", "p1") )
		p.setValue("p1")
		self.assertEqual( p.getValue(), IntData(60) )

	def testOrderedPresets( self ) :

		p = Parameter(
			name = "n",
			description = "d",
			defaultValue = FloatData( 20 ),
			presets = (
				( "p1", FloatData( 40 ) ),
				( "p2", IntData( 60 ) ),
				( "p3", CompoundData() ),
				( "p4", FloatData( 20 ) ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( FloatData( 40 ), IntData( 60 ), CompoundData(), FloatData( 20 ) ) )

	def testRunTimeTyping( self ) :

		c = IntParameter(
			name = "i",
			description = "d",
			defaultValue = 10,
		)
		self.assertEqual( c.typeId(), TypeId.IntParameter )
		self.assertEqual( c.typeName(), "IntParameter" )
		self.assert_( c.isInstanceOf( "IntParameter" ) )
		self.assert_( c.isInstanceOf( "Parameter" ) )
		self.assert_( c.isInstanceOf( TypeId.IntParameter ) )
		self.assert_( c.isInstanceOf( TypeId.Parameter ) )

		c = V3fParameter(
			name = "i",
			description = "d",
			defaultValue = V3f( 1 ),
		)
		self.assertEqual( c.typeId(), TypeId.V3fParameter )
		self.assertEqual( c.typeName(), "V3fParameter" )
		self.assert_( c.isInstanceOf( "V3fParameter" ) )
		self.assert_( c.isInstanceOf( "Parameter" ) )
		self.assert_( c.isInstanceOf( TypeId.V3fParameter ) )
		self.assert_( c.isInstanceOf( TypeId.Parameter ) )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = Parameter( "p", "description", FloatData( 1 ) )
		q = Parameter( "q", "description", IntData( 2 ) )
		self.assert_( p.getValue() == FloatData( 1 ) )
		p.smartSetValue( q.getValue() )
		self.assert_( p.getValue() == IntData( 2 ) )
		
	def testNoneIsValid( self ) :
	
		p = Parameter( "p", "description", FloatData( 1 ) )

		self.failIf( p.valueValid( None )[0] )

class TestNumericParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = IntParameter( "name", "description", 1 )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IntData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual (p.userData(), CompoundObject() )

		p = IntParameter( "name", "description", 5, 0, 10 )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IntData( 5 ) )
		self.assertEqual( p.numericDefaultValue, 5 )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.minValue, 0 )
		self.assertEqual( p.maxValue, 10 )

		self.assertRaises( RuntimeError, IntParameter, "name", "description", 15, 0, 10 )

	def testUserData( self ):
		compound = CompoundObject()
		compound["first"] = IntData()
		compound["second"] = QuatfData()
		compound["third"] = StringData("test")
		p = IntParameter( "name", "description", 1, userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = CharData('1')
		data["first"] = data["fourth"]

	def testKeywordConstructor( self ) :

		p = IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
		)

		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IntData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )

		p = IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
			minValue = -10,
			maxValue = 10,
		)

		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IntData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.minValue, -10 )
		self.assertEqual( p.maxValue, 10 )
		self.assert_( p.hasMinValue() )
		self.assert_( p.hasMaxValue() )

		p = IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
			minValue = -10
		)

		self.assert_( p.hasMinValue() )
		self.failIf ( p.hasMaxValue() )

		p = IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
			maxValue = 10
		)

		self.failIf ( p.hasMinValue() )
		self.assert_( p.hasMaxValue() )

	def testLimits( self ) :

		p = FloatParameter( "n", "d", 0, -100, 100 )
		self.assertRaises( Exception, p.setValidatedValue, FloatData( -1000 ) )
		self.assertRaises( Exception, p.setValidatedValue, FloatData( 101 ) )
		self.assertRaises( Exception, p.setValidatedValue, IntData( 0 ) )
		p.setValue( FloatData( 10 ) )

	def testPresets( self ) :

		p = IntParameter(
			name = "n",
			description = "d",
			presets = (
				( "one", 1 ),
				( "two", 2 ),
				( "three", 3 ),
			)
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 3 )
		self.assertEqual( pr["one"], IntData( 1 ) )
		self.assertEqual( pr["two"], IntData( 2 ) )
		self.assertEqual( pr["three"], IntData( 3 ) )

		# overriding presets
		p.setPresets(
			[
				( "four", IntData( 4 ) ),
				( "one", IntData( 1 ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["four"], IntData( 4 ) )
		self.assertEqual( pr["one"], IntData( 1 ) )
		self.assertEqual( p.presetNames(), ("four", "one") )
		p.setValue("four")
		self.assertEqual( p.getValue(), IntData(4) )

	def testOrderedPresets( self ) :

		p = IntParameter(
			name = "n",
			description = "d",
			defaultValue = 1,
			presets = (
				( "p1", 10 ),
				( "p2", 1 ),
				( "p3", 20 ),
				( "p4", 30 ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( IntData( 10 ), IntData( 1 ), IntData( 20 ), IntData( 30 ) ) )

	def testSetGet( self ) :

		p = IntParameter( "name", "description", 1 )
		p.setValue( IntData( 10 ) )
		self.assertEqual( p.getValue(), IntData( 10 ) )
		self.assertEqual( p.getNumericValue(), 10 )
		p.setNumericValue( 20 )
		self.assertEqual( p.getValue(), IntData( 20 ) )
		self.assertEqual( p.getNumericValue(), 20 )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = IntParameter( "p", "description", 1 )
		q = IntParameter( "q", "description", 2 )
		self.assert_( p.getValue() == IntData( 1 ) )
		p.smartSetValue( q.getValue() )
		self.assert_( p.getValue() == IntData( 2 ) )
		p.smartSetValue( 3 )
		self.assert_( p.getValue() == IntData( 3 ) )
		p.smartSetValue( IntData(4) )
		self.assert_( p.getValue() == IntData( 4 ) )
		
	def testDefaultValue( self ) :
	
		p = IntParameter( "p", "description", 1 )
		
		self.assertEqual( p.numericDefaultValue, 1 )
		self.assertRaises( AttributeError, setattr, p, "numericDefaultValue", 2 )
		

class TestTypedParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = V2fParameter( "n", "d", V2f( 10 ) )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, V2fData( V2f( 10 ) ) )
		self.assertEqual( p.getValue(), V2fData( V2f( 10 ) ) )
		self.assertEqual (p.userData(), CompoundObject() )

	def testUserData( self ):
		compound = CompoundObject()
		compound["first"] = IntData()
		compound["second"] = QuatfData()
		compound["third"] = StringData("test")
		p = Box3dParameter( "name", "description", Box3d(), userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = CharData('1')
		data["first"] = data["fourth"]

	def testPresets( self ) :

		p = V3fParameter(
			name = "n",
			description = "d",
			defaultValue = V3f( 2 ),
			presets = (
				( "one", V3f( 1 ) ),
				( "two", V3f( 2 ) ),
				( "three", V3f( 3 ) ),
			),
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 3 )
		self.assertEqual( pr["one"], V3fData( V3f( 1 ) ) )
		self.assertEqual( pr["two"], V3fData( V3f( 2 ) ) )
		self.assertEqual( pr["three"], V3fData( V3f( 3 ) ) )

		p.setValue( "one" )
		self.assertEqual( p.getValue(), V3fData( V3f( 1 ) ) )

		# overriding presets
		p.setPresets(
			[
				( "four", V3fData( V3f(4) ) ),
				( "one", V3fData( V3f(1) ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["four"], V3fData( V3f(4) ) )
		self.assertEqual( pr["one"], V3fData( V3f(1) ) )
		self.assertEqual( p.presetNames(), ("four", "one") )
		p.setValue("four")
		self.assertEqual( p.getValue(), V3fData(V3f(4)) )

	def testPresetsOnly( self ) :

		p = V3fParameter(
			name = "n",
			description = "d",
			defaultValue = V3f( 2 ),
			presets = (
				( "one", V3f( 1 ) ),
				( "two", V3f( 2 ) ),
				( "three", V3f( 3 ) ),
			),
			presetsOnly = True,
		)

		self.assertRaises( RuntimeError, p.setValidatedValue, V3fData( V3f( 20 ) ) )

		p = V3fParameter(
			name = "n",
			description = "d",
			defaultValue = V3f( 2 ),
			presets = (
				( "one", V3f( 1 ) ),
				( "two", V3f( 2 ) ),
				( "three", V3f( 3 ) ),
			),
			presetsOnly = False,
		)

		p.setValue( V3fData( V3f( 20 ) ) )

	def testTypedValueFns( self ) :

		p = StringParameter( name="n", description="d", defaultValue = "10" )
		self.assertEqual( p.getTypedValue(), "10" )
		p.setTypedValue( "20" )
		self.assertEqual( p.getTypedValue(), "20" )
		
		self.assertEqual( p.typedDefaultValue, "10" )
		self.assertRaises( AttributeError, setattr, p, "typedDefaultValue", "20" )

		p = V3fParameter( name="n", description="d", defaultValue = V3f( 1, 2, 3 ) )
		self.assertEqual( p.getTypedValue(), V3f( 1, 2, 3 ) )
		p.setTypedValue( V3f( 12, 13, 14 ) )
		self.assertEqual( p.getTypedValue(), V3f( 12, 13, 14 ) )
		self.assertEqual( p.getValue(), V3fData( V3f( 12, 13, 14 ) ) )

		self.assertEqual( p.typedDefaultValue, V3f( 1, 2, 3 ) )
		self.assertRaises( AttributeError, setattr, p, "typedDefaultValue", V3f( 4, 5, 6 ) )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = V2fParameter( "p", "description", V2f( 10 ) )
		q = V2fParameter( "q", "description", V2f( 2 ) )
		self.assert_( p.getValue() == V2fData( V2f( 10 ) ) )
		p.smartSetValue( q.getValue() )
		self.assert_( p.getValue() == V2fData( V2f( 2 ) ) )
		p.smartSetValue( V2f( 3 ) )
		self.assert_( p.getValue() == V2fData( V2f( 3 ) ) )
		p.smartSetValue( V2fData( V2f( 4 ) ) )
		self.assert_( p.getValue() == V2fData( V2f( 4 ) ) )

	def testOrderedPresets( self ) :

		p = StringParameter(
			name = "n",
			description = "d",
			defaultValue = "huh?",
			presets = (
				( "p1", "a" ),
				( "p2", StringData( "b" ) ),
				( "p3", "c" ),
				( "p4", "d" ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( StringData( "a" ), StringData( "b" ), StringData( "c" ), StringData( "d" ) ) )		
	
	def testInterpretation( self ) :
		
		p = V3fParameter( name="n", description="d", defaultValue = V3f( 1, 2, 3 ) )
		self.assertEqual( p.defaultValue, V3fData( V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.defaultValue.getInterpretation(), GeometricData.Interpretation.Numeric )
		self.assertEqual( p.getValue(), V3fData( V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.getValue().getInterpretation(), GeometricData.Interpretation.Numeric )
		
		value = V3fData( V3f( 12, 13, 14 ) )
		value.setInterpretation( GeometricData.Interpretation.Vector )
		p.setValue( value )
		self.assertNotEqual( p.getValue(), V3fData( V3f( 12, 13, 14 ) ) )
		self.assertEqual( p.getValue(), value )
		self.assertEqual( p.getValue().getInterpretation(), GeometricData.Interpretation.Vector )

		dv = V3fData( V3f( 1, 2, 3 ), GeometricData.Interpretation.Normal )
		p = V3fParameter( name="n", description="d", defaultValue = dv )
		self.assertNotEqual( p.defaultValue, V3fData( V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.defaultValue, dv )
		self.assertEqual( p.defaultValue.getInterpretation(), GeometricData.Interpretation.Normal )
		self.assertNotEqual( p.getValue(), V3fData( V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.getValue(), dv )
		self.assertEqual( p.getValue().getInterpretation(), GeometricData.Interpretation.Normal )
		
		dv = V3fVectorData( [ V3f( 1, 2, 3 ) ], GeometricData.Interpretation.Normal )
		p = V3fVectorParameter( name="n", description="d", defaultValue = dv )
		self.assertNotEqual( p.defaultValue, V3fVectorData( [ V3f( 1, 2, 3 ) ] ) )
		self.assertEqual( p.defaultValue, dv )
		self.assertEqual( p.defaultValue.getInterpretation(), GeometricData.Interpretation.Normal )
		self.assertNotEqual( p.getValue(), V3fVectorData( [ V3f( 1, 2, 3 ) ] ) )
		self.assertEqual( p.getValue(), dv )
		self.assertEqual( p.getValue().getInterpretation(), GeometricData.Interpretation.Normal )
		
		p.setValue( V3fVectorData( [ V3f( 12, 13, 14 ) ] ) )
		self.assertEqual( p.getValue(), V3fVectorData( [ V3f( 12, 13, 14 ) ] ) )
		self.assertEqual( p.getValue().getInterpretation(), GeometricData.Interpretation.Numeric )

class TestValidatedStringParameter( unittest.TestCase ) :

	def test( self ) :

		p = ValidatedStringParameter(
			name = "n",
			description = "d",
			regex = "[0-9]*",
			regexDescription = "Value must be an integer",
			presets = (
				( "100", "100" ),
				( "200", "200" ),
			)
		)

		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.regex, "[0-9]*" )
		self.assertEqual (p.userData(), CompoundObject() )

		self.assertRaises( RuntimeError, p.setValidatedValue, StringData( "A" ) )
		p.setValue( StringData( "100" ) )
		self.assertEqual( p.getValue(), StringData( "100" ) )

		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assert_( "100" in pr.keys() )
		self.assert_( "200" in pr.keys() )

	def testUserData( self ):
		compound = CompoundObject()
		compound["first"] = IntData()
		compound["second"] = QuatfData()
		compound["third"] = StringData("test")
		p = ValidatedStringParameter(
			name = "n",
			description = "d",
			regex = "[0-9]*",
			regexDescription = "Value must be an integer",
			userData = compound
		)
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = CharData('1')
		data["first"] = data["fourth"]

	def testOrderedPresets( self ) :

		p = ValidatedStringParameter(
			name = "n",
			description = "d",
			regex = "*",
			regexDescription = "",
			defaultValue = "huh?",
			presets = (
				( "p1", "a" ),
				( "p2", "b" ),
				( "p3", "c" ),
				( "p4", "d" ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( StringData( "a" ), StringData( "b" ), StringData( "c" ), StringData( "d" ) ) )

class TestDirNameParameter( unittest.TestCase ) :

	def test( self ) :

		p = DirNameParameter(
			name = "f",
			description = "d",
			defaultValue = "test",
			check = DirNameParameter.CheckType.MustExist,
			allowEmptyString = True
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.mustExist, True )
		self.assertEqual( p.allowEmptyString, True )
		self.assertEqual (p.userData(), CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

	def testMustNotExist( self ):
		p = DirNameParameter(
				name = "f",
				description = "d",
				defaultValue = "/lucioSaysThisDirectoryDoesNotExist",
				check = DirNameParameter.CheckType.MustExist,
				allowEmptyString = True,
		)
		self.assertRaises( RuntimeError, p.validate )

class TestFileNameParameter( unittest.TestCase ) :

	def test( self ) :

		p = FileNameParameter(
			name = "f",
			description = "d",
			extensions = "tif tiff jpg cin",
			check = FileNameParameter.CheckType.DontCare,
			allowEmptyString = True
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.extensions, [ "tif", "tiff", "jpg", "cin" ] )
		self.assertEqual( p.mustExist, False )
		self.assertEqual( p.allowEmptyString, True )
		self.assertEqual (p.userData(), CompoundObject() )

		for e in p.extensions :
			p.setValidatedValue( StringData("hello." + e) )

		p.setValue( StringData( "test" ) )
		self.assertRaises( RuntimeError, p.validate )

	def testUserData( self ):
		compound = CompoundObject()
		compound["first"] = IntData()
		compound["second"] = QuatfData()
		compound["third"] = StringData("test")
		p = FileNameParameter(
			name = "f",
			description = "d",
			extensions = "tif tiff jpg cin",
			check = FileNameParameter.CheckType.DontCare,
			allowEmptyString = True,
			userData = compound
		)
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = CharData('1')
		data["first"] = data["fourth"]

	def testNoExtensions( self ) :

		p = FileNameParameter(
			name = "f",
			description = "d",
		)
		self.assertEqual( p.extensions, [] )

		p.setValue( StringData( "hello.tif" ) )
		p.setValue( StringData( "hello" ) )

	def testNotADirectory( self ) :

		p = FileNameParameter(
			name = "f",
			description = "d",
			defaultValue = "test",
			check = FileNameParameter.CheckType.MustExist,
			allowEmptyString = True
		)

		self.assertRaises( RuntimeError, p.validate )
		self.assertEqual( p.valueValid()[0], False )

class TestValidation( unittest.TestCase ) :

	def test( self ) :

		i = IntParameter( name = "n", description = "d", defaultValue = 10 )
		self.assert_( i.valueValid( IntData( 1 ) ) )
		self.assert_( not i.valueValid( FloatData( 1 ) )[0] )

	def testLazyValidation( self ) :

		i = IntParameter( name = "n", description = "d", defaultValue = 10 )
		i.validate( IntData( 10 ) )
		self.assertRaises( RuntimeError, i.validate, FloatData( 20 ) )
		i.setValue( IntData( 10 ) )
		i.validate()
		i.setValue( FloatData( 10 ) )
		self.assertRaises( RuntimeError, i.validate )
		self.assertRaises( RuntimeError, i.getValidatedValue )

		i = V3fParameter( name = "n", description = "d", defaultValue = V3f( 10 ) )
		i.validate( V3fData( V3f( 10 ) ) )
		self.assertRaises( RuntimeError, i.validate, FloatData( 20 ) )
		i.setValue( V3fData( V3f( 20 ) ) )
		i.validate()
		i.setValue( FloatData( 10 ) )
		self.assertRaises( RuntimeError, i.validate )
		self.assertRaises( RuntimeError, i.getValidatedValue )

class TestObjectParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = ObjectParameter( name = "name", description = "description", defaultValue = PointsPrimitive( 1 ), type = TypeId.PointsPrimitive )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, PointsPrimitive( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.getCurrentPresetName(), "" )
		self.assertEqual( p.validTypes(), [TypeId.PointsPrimitive] )

		self.assert_( p.valueValid( PointsPrimitive( 1 ) )[0] )
		self.assert_( not p.valueValid( IntData( 1 ) )[0] )

	def testConstructor2( self ) :

		p = ObjectParameter( name = "name", description = "description", defaultValue = PointsPrimitive( 1 ), types = [TypeId.PointsPrimitive, TypeId.FloatData] )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, PointsPrimitive( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.getCurrentPresetName(), "" )
		self.assertEqual( len( p.validTypes() ), 2 )
		self.assert_( TypeId.PointsPrimitive in p.validTypes() )
 		self.assert_( TypeId.FloatData in p.validTypes() )

		self.assert_( p.valueValid( PointsPrimitive( 1 ) )[0] )
		self.assert_( p.valueValid( FloatData( 1 ) )[0] )
		self.assert_( not p.valueValid( IntData( 1 ) )[0] )


	def testUserData( self ) :

		p = ObjectParameter( name = "name", description = "description", defaultValue = PointsPrimitive( 1 ), type = TypeId.PointsPrimitive, userData = CompoundObject( { "A" : IntData( 10 ) } ) )
		self.assertEqual( p.userData(), CompoundObject( { "A" : IntData( 10 ) } ) )

		p = ObjectParameter( name = "name", description = "description", defaultValue = PointsPrimitive( 1 ), type = TypeId.PointsPrimitive )
		self.assertEqual (p.userData(), CompoundObject() )

	def testErrorMessage( self ) :

		p = ObjectParameter( name = "name", description = "description", defaultValue = FloatData( 1 ), types = [TypeId.FloatData] )
		self.assertEqual( p.valueValid( V3fData( V3f( 1 ) ) )[1], "Object is not of type FloatData" )

		p = ObjectParameter( name = "name", description = "description", defaultValue = FloatData( 1 ), types = [TypeId.FloatData, TypeId.IntData] )
		self.assertEqual( p.valueValid( V3fData( V3f( 1 ) ) )[1], "Object is not of type FloatData or IntData" )

		p = ObjectParameter( name = "name", description = "description", defaultValue = FloatData( 1 ), types = [TypeId.FloatData, TypeId.DoubleData, TypeId.IntData] )
		self.assertEqual( p.valueValid( V3fData( V3f( 1 ) ) )[1], "Object is not of type FloatData, DoubleData or IntData" )

	def testOrderedPresets( self ) :

		p = ObjectParameter(
			name = "n",
			description = "d",
			defaultValue = FloatData( 20 ),
			types = [ Object.staticTypeId() ],
			presets = (
				( "p1", FloatData( 40 ) ),
				( "p2", IntData( 60 ) ),
				( "p3", CompoundData() ),
				( "p4", FloatData( 20 ) ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( FloatData( 40 ), IntData( 60 ), CompoundData(), FloatData( 20 ) ) )

		# overriding presets
		p.setPresets(
			[
				( "four", V3fData( V3f(4) ) ),
				( "one", V3fData( V3f(1) ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["four"], V3fData( V3f(4) ) )
		self.assertEqual( pr["one"], V3fData( V3f(1) ) )
		self.assertEqual( p.presetNames(), ("four", "one") )
		p.setValue("four")
		self.assertEqual( p.getValue(), V3fData(V3f(4)) )

class TestTypedObjectParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		mesh = MeshPrimitive()
		p = MeshPrimitiveParameter( "n", "d", mesh )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, mesh )
		self.assertEqual( p.getValue(), mesh )
		self.assertEqual( p.userData(), CompoundObject() )

	def testUserData( self ):
		compound = CompoundObject()
		compound["first"] = IntData()
		compound["second"] = QuatfData()
		compound["third"] = StringData("test")
		p = MeshPrimitiveParameter( "name", "description", MeshPrimitive(), userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = CharData('1')
		data["first"] = data["fourth"]

	def testPresets( self ) :

		mesh1 = MeshPrimitive( IntVectorData([3]), IntVectorData([0,1,2]) )
		mesh2 = MeshPrimitive( IntVectorData([3]), IntVectorData([1,2,3]) )
		mesh3 = MeshPrimitive( IntVectorData([3]), IntVectorData([2,3,4]) )

		p = MeshPrimitiveParameter(
			name = "n",
			description = "d",
			defaultValue = mesh2,
			presets = (
				( "one", mesh1 ),
				( "two", mesh2 ),
				( "three", mesh3 ),
			),
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 3 )
		self.assertEqual( pr["one"], mesh1 )
		self.assertEqual( pr["two"], mesh2 )
		self.assertEqual( pr["three"], mesh3 )

		p.setValue( "one" )
		self.assertEqual( p.getValue(), mesh1 )

	def testPresetsOnly( self ) :

		mesh1 = MeshPrimitive( IntVectorData([3]), IntVectorData([0,1,2]) )
		mesh2 = MeshPrimitive( IntVectorData([3]), IntVectorData([1,2,3]) )
		mesh3 = MeshPrimitive( IntVectorData([3]), IntVectorData([2,3,4]) )

		mesh4 = MeshPrimitive( IntVectorData([3]), IntVectorData([3,4,5]) )

		p = MeshPrimitiveParameter(
			name = "n",
			description = "d",
			defaultValue = mesh2,
			presets = (
				( "one", mesh1 ),
				( "two", mesh2 ),
				( "three", mesh3 ),
			),
			presetsOnly = True,
		)

		self.assertRaises( RuntimeError, p.setValidatedValue, mesh4 )

		p = MeshPrimitiveParameter(
			name = "n",
			description = "d",
			defaultValue = mesh2,
			presets = (
				( "one", mesh1 ),
				( "two", mesh2 ),
				( "three", mesh3 ),
			),
			presetsOnly = False,
		)

		p.setValue( mesh4 )

	def testOrderedPresets( self ) :

		p = PointsPrimitiveParameter(
			name = "n",
			description = "d",
			defaultValue = PointsPrimitive( 1 ),
			presets = (
				( "p1", PointsPrimitive( 1 ) ),
				( "p2", PointsPrimitive( 2 ) ),
				( "p3", PointsPrimitive( 3 ) ),
				( "p4", PointsPrimitive( 4 ) ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( PointsPrimitive( 1 ), PointsPrimitive( 2 ), PointsPrimitive( 3 ), PointsPrimitive( 4 ) ) )

	def testSmoothSkinningData( self ) :

		ssd = SmoothSkinningData()
		p = SmoothSkinningDataParameter( "n", "d", ssd )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, ssd )
		self.assertEqual( p.getValue(), ssd )
		self.assertEqual( p.userData(), CompoundObject() )

class TestIntVectorParameter( unittest.TestCase ) :

	def test( self ) :

		dv = IntVectorData()

		p = IntVectorParameter(
			name = "f",
			description = "d",
			defaultValue = dv,
			presets = (
				( "preset1", IntVectorData( [ 1, 2 ] ) ),
			)
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.userData(), CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

class TestTransformationMatixParameter( unittest.TestCase ) :

	def test( self ) :

		tm = TransformationMatrixfData()
		p = TransformationMatrixfParameter(
			name = "f",
			description = "d",
			defaultValue = tm,
		)
		p.validate()		
		
		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.valueValid()[0], True )

		self.failUnless( isinstance( p.getTypedValue().translate, V3f ) )
		self.assertEqual( p.getTypedValue().translate, V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotate, Eulerf( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotationOrientation, Quatf( 1,0,0,0 ) )
		self.assertEqual( p.getTypedValue().scale, V3f( 1,1,1 ) )
		self.assertEqual( p.getTypedValue().shear, V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivot, V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivotTranslation, V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivot, V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivotTranslation, V3f( 0,0,0 ) )
		
		tm = TransformationMatrixdData()
		p = TransformationMatrixdParameter(
			name = "f",
			description = "d",
			defaultValue = tm,
		)
		p.validate()		

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.valueValid()[0], True )
		
		self.failUnless( isinstance( p.getTypedValue().translate, V3d ) )
		self.assertEqual( p.getTypedValue().translate, V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotate, Eulerd( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotationOrientation, Quatd( 1,0,0,0 ) )
		self.assertEqual( p.getTypedValue().scale, V3d( 1,1,1 ) )
		self.assertEqual( p.getTypedValue().shear, V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivot, V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivotTranslation, V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivot, V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivotTranslation, V3d( 0,0,0 ) )		

class TestPathVectorParameter( unittest.TestCase ) :

	def test( self ) :

		dv = StringVectorData()

		p = PathVectorParameter(
			name = "f",
			description = "d",
			defaultValue = dv,
			check = PathVectorParameter.CheckType.MustExist,
			allowEmptyList = True,
			presets = (
				( "preset1", StringVectorData( [ 'one', 'two' ] ) ),
			)
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.mustExist, True )
		self.assertEqual( p.allowEmptyList, True )
		self.assertEqual( p.userData(), CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

	def testMustNotExist( self ):

		dv = StringVectorData()
		dv.append( "/ThisDirectoryDoesNotExist " )

		p = PathVectorParameter(
				name = "f",
				description = "d",
				defaultValue = dv,
				check = PathVectorParameter.CheckType.MustExist,
				allowEmptyList = False,
		)
		self.assertRaises( RuntimeError, p.validate )

class TestFileSequenceVectorParameter( unittest.TestCase ) :

	def test( self ) :

		dv = StringVectorData()

		p = FileSequenceVectorParameter(
			name = "f",
			description = "d",
			defaultValue = dv,
			check = FileSequenceVectorParameter.CheckType.MustExist,
			allowEmptyList = True,
			presets = (
				( "preset1", StringVectorData( [ 'one', 'two' ] ) ),
			)
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.mustExist, True )
		self.assertEqual( p.allowEmptyList, True )
		self.assertEqual( p.userData(), CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

class TestMarschnerParameter( unittest.TestCase ) :

	def test( self ) :

		p1 = MarschnerParameter( "m", "", True )
		self.failIf( "color" not in p1 )
		self.failIf( "absorption" in p1 )

		p2 = MarschnerParameter( "m", "", False )
		self.failIf( "absorption" not in p2 )
		self.failIf( "color" in p2 )

if __name__ == "__main__":
        unittest.main()
