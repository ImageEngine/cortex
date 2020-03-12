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
import imath

import IECore

class TestParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = IECore.Parameter( "name", "description", IECore.FloatData( 1 ) )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IECore.FloatData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.getCurrentPresetName(), "" )
		self.assertEqual (p.userData(), IECore.CompoundObject() )

		v = IECore.IntData( 2 )
		p.setValue( v )
		self.assertEqual( p.getValue(), v )
		self.assertRaises( RuntimeError, IECore.Parameter, "name", "description", None )		# passing None as default value should raise exception as opposed to segfault!

	def testUserData( self ):
		compound = IECore.CompoundObject()
		compound["first"] = IECore.IntData()
		compound["second"] = IECore.QuatfData()
		compound["third"] = IECore.StringData("test")
		p = IECore.Parameter( "name", "description", IECore.FloatData( 1 ), userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assertTrue(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = IECore.CharData('1')
		data["first"] = data["fourth"]

	def testKeywordConstructor( self ) :

		p = IECore.Parameter(
			name = "n",
			description = "d",
			defaultValue = IECore.FloatData( 20 )
		)

		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, IECore.FloatData( 20 ) )
		self.assertEqual( p.getValue(), p.defaultValue )

	def testPresets( self ) :

		# Presets as tuple
		p = IECore.Parameter(
			name = "n",
			description = "d",
			defaultValue = IECore.FloatData( 20 ),
			presets = (
				( "p1", IECore.FloatData( 40 ) ),
				( "p2", IECore.IntData( 60 ) ),
				( "p3", IECore.CompoundData() ),
				( "p4", IECore.FloatData( 20 ) ),
			),
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 4 )
		self.assertEqual( pr["p1"], IECore.FloatData( 40 ) )
		self.assertEqual( pr["p2"], IECore.IntData( 60 ) )
		self.assertEqual( pr["p3"], IECore.CompoundData() )
		self.assertEqual( pr["p4"], IECore.FloatData( 20 ) )

		for k, v in pr.items() :
			p.setValue( k )
			self.assertEqual( p.getValue(), v )
			self.assertEqual( p.getCurrentPresetName(), k )

		self.assertRaises( RuntimeError, p.setValue, "thisIsNotAPreset" )
		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.FloatData( 1000 ) )

		# Presets as list
		p = IECore.Parameter(
			name = "n",
			description = "d",
			defaultValue = IECore.FloatData( 20 ),
			presets = [
				( "p1", IECore.FloatData( 40 ) ),
				( "p2", IECore.IntData( 60 ) ),
				( "p3", IECore.CompoundData() ),
				( "p4", IECore.FloatData( 20 ) ),
			],
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 4 )
		self.assertEqual( pr["p1"], IECore.FloatData( 40 ) )
		self.assertEqual( pr["p2"], IECore.IntData( 60 ) )
		self.assertEqual( pr["p3"], IECore.CompoundData() )
		self.assertEqual( pr["p4"], IECore.FloatData( 20 ) )

		# overriding presets

		p.setPresets( [] )
		self.assertEqual( p.getPresets(), dict() )

		p.setPresets(
			[
				( "p5", IECore.FloatData( 40 ) ),
				( "p1", IECore.IntData( 60 ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["p5"], IECore.FloatData( 40 ) )
		self.assertEqual( pr["p1"], IECore.IntData( 60 ) )
		self.assertEqual( p.presetNames(), ("p5", "p1") )
		p.setValue("p1")
		self.assertEqual( p.getValue(), IECore.IntData(60) )

	def testOrderedPresets( self ) :

		p = IECore.Parameter(
			name = "n",
			description = "d",
			defaultValue = IECore.FloatData( 20 ),
			presets = (
				( "p1", IECore.FloatData( 40 ) ),
				( "p2", IECore.IntData( 60 ) ),
				( "p3", IECore.CompoundData() ),
				( "p4", IECore.FloatData( 20 ) ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( IECore.FloatData( 40 ), IECore.IntData( 60 ), IECore.CompoundData(), IECore.FloatData( 20 ) ) )

	def testRunTimeTyping( self ) :

		c = IECore.IntParameter(
			name = "i",
			description = "d",
			defaultValue = 10,
		)
		self.assertEqual( c.typeId(), IECore.TypeId.IntParameter )
		self.assertEqual( c.typeName(), "IntParameter" )
		self.assertTrue( c.isInstanceOf( "IntParameter" ) )
		self.assertTrue( c.isInstanceOf( "Parameter" ) )
		self.assertTrue( c.isInstanceOf( IECore.TypeId.IntParameter ) )
		self.assertTrue( c.isInstanceOf( IECore.TypeId.Parameter ) )

		c = IECore.V3fParameter(
			name = "i",
			description = "d",
			defaultValue = imath.V3f( 1 ),
		)
		self.assertEqual( c.typeId(), IECore.TypeId.V3fParameter )
		self.assertEqual( c.typeName(), "V3fParameter" )
		self.assertTrue( c.isInstanceOf( "V3fParameter" ) )
		self.assertTrue( c.isInstanceOf( "Parameter" ) )
		self.assertTrue( c.isInstanceOf( IECore.TypeId.V3fParameter ) )
		self.assertTrue( c.isInstanceOf( IECore.TypeId.Parameter ) )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = IECore.Parameter( "p", "description", IECore.FloatData( 1 ) )
		q = IECore.Parameter( "q", "description", IECore.IntData( 2 ) )
		self.assertTrue( p.getValue() == IECore.FloatData( 1 ) )
		p.smartSetValue( q.getValue() )
		self.assertTrue( p.getValue() == IECore.IntData( 2 ) )

	def testNoneIsValid( self ) :

		p = IECore.Parameter( "p", "description", IECore.FloatData( 1 ) )

		self.assertFalse( p.valueValid( None )[0] )

class TestNumericParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = IECore.IntParameter( "name", "description", 1 )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IECore.IntData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual (p.userData(), IECore.CompoundObject() )

		p = IECore.IntParameter( "name", "description", 5, 0, 10 )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IECore.IntData( 5 ) )
		self.assertEqual( p.numericDefaultValue, 5 )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.minValue, 0 )
		self.assertEqual( p.maxValue, 10 )

		self.assertRaises( RuntimeError, IECore.IntParameter, "name", "description", 15, 0, 10 )

	def testUserData( self ):
		compound = IECore.CompoundObject()
		compound["first"] = IECore.IntData()
		compound["second"] = IECore.QuatfData()
		compound["third"] = IECore.StringData("test")
		p = IECore.IntParameter( "name", "description", 1, userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assertTrue(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = IECore.CharData('1')
		data["first"] = data["fourth"]

	def testKeywordConstructor( self ) :

		p = IECore.IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
		)

		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IECore.IntData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )

		p = IECore.IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
			minValue = -10,
			maxValue = 10,
		)

		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IECore.IntData( 1 ) )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.minValue, -10 )
		self.assertEqual( p.maxValue, 10 )
		self.assertTrue( p.hasMinValue() )
		self.assertTrue( p.hasMaxValue() )

		p = IECore.IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
			minValue = -10
		)

		self.assertTrue( p.hasMinValue() )
		self.assertFalse( p.hasMaxValue() )

		p = IECore.IntParameter(
			name = "name",
			description = "description",
			defaultValue = 1,
			maxValue = 10
		)

		self.assertFalse( p.hasMinValue() )
		self.assertTrue( p.hasMaxValue() )

	def testLimits( self ) :

		p = IECore.FloatParameter( "n", "d", 0, -100, 100 )
		self.assertRaises( Exception, p.setValidatedValue, IECore.FloatData( -1000 ) )
		self.assertRaises( Exception, p.setValidatedValue, IECore.FloatData( 101 ) )
		self.assertRaises( Exception, p.setValidatedValue, IECore.IntData( 0 ) )
		p.setValue( IECore.FloatData( 10 ) )

	def testPresets( self ) :

		p = IECore.IntParameter(
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
		self.assertEqual( pr["one"], IECore.IntData( 1 ) )
		self.assertEqual( pr["two"], IECore.IntData( 2 ) )
		self.assertEqual( pr["three"], IECore.IntData( 3 ) )

		# overriding presets
		p.setPresets(
			[
				( "four", IECore.IntData( 4 ) ),
				( "one", IECore.IntData( 1 ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["four"], IECore.IntData( 4 ) )
		self.assertEqual( pr["one"], IECore.IntData( 1 ) )
		self.assertEqual( p.presetNames(), ("four", "one") )
		p.setValue("four")
		self.assertEqual( p.getValue(), IECore.IntData(4) )

	def testOrderedPresets( self ) :

		p = IECore.IntParameter(
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
		self.assertEqual( p.presetValues(), ( IECore.IntData( 10 ), IECore.IntData( 1 ), IECore.IntData( 20 ), IECore.IntData( 30 ) ) )

	def testSetGet( self ) :

		p = IECore.IntParameter( "name", "description", 1 )
		p.setValue( IECore.IntData( 10 ) )
		self.assertEqual( p.getValue(), IECore.IntData( 10 ) )
		self.assertEqual( p.getNumericValue(), 10 )
		p.setNumericValue( 20 )
		self.assertEqual( p.getValue(), IECore.IntData( 20 ) )
		self.assertEqual( p.getNumericValue(), 20 )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = IECore.IntParameter( "p", "description", 1 )
		q = IECore.IntParameter( "q", "description", 2 )
		self.assertTrue( p.getValue() == IECore.IntData( 1 ) )
		p.smartSetValue( q.getValue() )
		self.assertTrue( p.getValue() == IECore.IntData( 2 ) )
		p.smartSetValue( 3 )
		self.assertTrue( p.getValue() == IECore.IntData( 3 ) )
		p.smartSetValue( IECore.IntData(4) )
		self.assertTrue( p.getValue() == IECore.IntData( 4 ) )

	def testDefaultValue( self ) :

		p = IECore.IntParameter( "p", "description", 1 )

		self.assertEqual( p.numericDefaultValue, 1 )
		self.assertRaises( AttributeError, setattr, p, "numericDefaultValue", 2 )


class TestTypedParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = IECore.V2fParameter( "n", "d", imath.V2f( 10 ) )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, IECore.V2fData( imath.V2f( 10 ) ) )
		self.assertEqual( p.getValue(), IECore.V2fData( imath.V2f( 10 ) ) )
		self.assertEqual (p.userData(), IECore.CompoundObject() )

	def testUserData( self ):
		compound = IECore.CompoundObject()
		compound["first"] = IECore.IntData()
		compound["second"] = IECore.QuatfData()
		compound["third"] = IECore.StringData("test")
		p = IECore.Box3dParameter( "name", "description", imath.Box3d(), userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assertTrue(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = IECore.CharData('1')
		data["first"] = data["fourth"]

	def testPresets( self ) :

		p = IECore.V3fParameter(
			name = "n",
			description = "d",
			defaultValue = imath.V3f( 2 ),
			presets = (
				( "one", imath.V3f( 1 ) ),
				( "two", imath.V3f( 2 ) ),
				( "three", imath.V3f( 3 ) ),
			),
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 3 )
		self.assertEqual( pr["one"], IECore.V3fData( imath.V3f( 1 ) ) )
		self.assertEqual( pr["two"], IECore.V3fData( imath.V3f( 2 ) ) )
		self.assertEqual( pr["three"], IECore.V3fData( imath.V3f( 3 ) ) )

		p.setValue( "one" )
		self.assertEqual( p.getValue(), IECore.V3fData( imath.V3f( 1 ) ) )

		# overriding presets
		p.setPresets(
			[
				( "four", IECore.V3fData( imath.V3f(4) ) ),
				( "one", IECore.V3fData( imath.V3f(1) ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["four"], IECore.V3fData( imath.V3f(4) ) )
		self.assertEqual( pr["one"], IECore.V3fData( imath.V3f(1) ) )
		self.assertEqual( p.presetNames(), ("four", "one") )
		p.setValue("four")
		self.assertEqual( p.getValue(), IECore.V3fData(imath.V3f(4)) )

	def testPresetsOnly( self ) :

		p = IECore.V3fParameter(
			name = "n",
			description = "d",
			defaultValue = imath.V3f( 2 ),
			presets = (
				( "one", imath.V3f( 1 ) ),
				( "two", imath.V3f( 2 ) ),
				( "three", imath.V3f( 3 ) ),
			),
			presetsOnly = True,
		)

		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.V3fData( imath.V3f( 20 ) ) )

		p = IECore.V3fParameter(
			name = "n",
			description = "d",
			defaultValue = imath.V3f( 2 ),
			presets = (
				( "one", imath.V3f( 1 ) ),
				( "two", imath.V3f( 2 ) ),
				( "three", imath.V3f( 3 ) ),
			),
			presetsOnly = False,
		)

		p.setValue( IECore.V3fData( imath.V3f( 20 ) ) )

	def testTypedValueFns( self ) :

		p = IECore.StringParameter( name="n", description="d", defaultValue = "10" )
		self.assertEqual( p.getTypedValue(), "10" )
		p.setTypedValue( "20" )
		self.assertEqual( p.getTypedValue(), "20" )

		self.assertEqual( p.typedDefaultValue, "10" )
		self.assertRaises( AttributeError, setattr, p, "typedDefaultValue", "20" )

		p = IECore.V3fParameter( name="n", description="d", defaultValue = imath.V3f( 1, 2, 3 ) )
		self.assertEqual( p.getTypedValue(), imath.V3f( 1, 2, 3 ) )
		p.setTypedValue( imath.V3f( 12, 13, 14 ) )
		self.assertEqual( p.getTypedValue(), imath.V3f( 12, 13, 14 ) )
		self.assertEqual( p.getValue(), IECore.V3fData( imath.V3f( 12, 13, 14 ) ) )

		self.assertEqual( p.typedDefaultValue, imath.V3f( 1, 2, 3 ) )
		self.assertRaises( AttributeError, setattr, p, "typedDefaultValue", imath.V3f( 4, 5, 6 ) )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = IECore.V2fParameter( "p", "description", imath.V2f( 10 ) )
		q = IECore.V2fParameter( "q", "description", imath.V2f( 2 ) )
		self.assertTrue( p.getValue() == IECore.V2fData( imath.V2f( 10 ) ) )
		p.smartSetValue( q.getValue() )
		self.assertTrue( p.getValue() == IECore.V2fData( imath.V2f( 2 ) ) )
		p.smartSetValue( imath.V2f( 3 ) )
		self.assertTrue( p.getValue() == IECore.V2fData( imath.V2f( 3 ) ) )
		p.smartSetValue( IECore.V2fData( imath.V2f( 4 ) ) )
		self.assertTrue( p.getValue() == IECore.V2fData( imath.V2f( 4 ) ) )

	def testOrderedPresets( self ) :

		p = IECore.StringParameter(
			name = "n",
			description = "d",
			defaultValue = "huh?",
			presets = (
				( "p1", "a" ),
				( "p2", IECore.StringData( "b" ) ),
				( "p3", "c" ),
				( "p4", "d" ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( IECore.StringData( "a" ), IECore.StringData( "b" ), IECore.StringData( "c" ), IECore.StringData( "d" ) ) )

	def testInterpretation( self ) :

		p = IECore.V3fParameter( name="n", description="d", defaultValue = imath.V3f( 1, 2, 3 ) )
		self.assertEqual( p.defaultValue, IECore.V3fData( imath.V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.defaultValue.getInterpretation(), IECore.GeometricData.Interpretation.None_ )
		self.assertEqual( p.getValue(), IECore.V3fData( imath.V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.getValue().getInterpretation(), IECore.GeometricData.Interpretation.None_ )

		value = IECore.V3fData( imath.V3f( 12, 13, 14 ) )
		value.setInterpretation( IECore.GeometricData.Interpretation.Vector )
		p.setValue( value )
		self.assertNotEqual( p.getValue(), IECore.V3fData( imath.V3f( 12, 13, 14 ) ) )
		self.assertEqual( p.getValue(), value )
		self.assertEqual( p.getValue().getInterpretation(), IECore.GeometricData.Interpretation.Vector )

		dv = IECore.V3fData( imath.V3f( 1, 2, 3 ), IECore.GeometricData.Interpretation.Normal )
		p = IECore.V3fParameter( name="n", description="d", defaultValue = dv )
		self.assertNotEqual( p.defaultValue, IECore.V3fData( imath.V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.defaultValue, dv )
		self.assertEqual( p.defaultValue.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertNotEqual( p.getValue(), IECore.V3fData( imath.V3f( 1, 2, 3 ) ) )
		self.assertEqual( p.getValue(), dv )
		self.assertEqual( p.getValue().getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		dv = IECore.V3fVectorData( [ imath.V3f( 1, 2, 3 ) ], IECore.GeometricData.Interpretation.Normal )
		p = IECore.V3fVectorParameter( name="n", description="d", defaultValue = dv )
		self.assertNotEqual( p.defaultValue, IECore.V3fVectorData( [ imath.V3f( 1, 2, 3 ) ] ) )
		self.assertEqual( p.defaultValue, dv )
		self.assertEqual( p.defaultValue.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertNotEqual( p.getValue(), IECore.V3fVectorData( [ imath.V3f( 1, 2, 3 ) ] ) )
		self.assertEqual( p.getValue(), dv )
		self.assertEqual( p.getValue().getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		p.setValue( IECore.V3fVectorData( [ imath.V3f( 12, 13, 14 ) ] ) )
		self.assertEqual( p.getValue(), IECore.V3fVectorData( [ imath.V3f( 12, 13, 14 ) ] ) )
		self.assertEqual( p.getValue().getInterpretation(), IECore.GeometricData.Interpretation.None_ )

class TestValidatedStringParameter( unittest.TestCase ) :

	def test( self ) :

		p = IECore.ValidatedStringParameter(
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
		self.assertEqual (p.userData(), IECore.CompoundObject() )

		self.assertRaises( RuntimeError, p.setValidatedValue, IECore.StringData( "A" ) )
		p.setValue( IECore.StringData( "100" ) )
		self.assertEqual( p.getValue(), IECore.StringData( "100" ) )

		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertTrue( "100" in pr.keys() )
		self.assertTrue( "200" in pr.keys() )

	def testUserData( self ):
		compound = IECore.CompoundObject()
		compound["first"] = IECore.IntData()
		compound["second"] = IECore.QuatfData()
		compound["third"] = IECore.StringData("test")
		p = IECore.ValidatedStringParameter(
			name = "n",
			description = "d",
			regex = "[0-9]*",
			regexDescription = "Value must be an integer",
			userData = compound
		)
		self.assertEqual( p.userData(), compound )
		self.assertTrue(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = IECore.CharData('1')
		data["first"] = data["fourth"]

	def testOrderedPresets( self ) :

		p = IECore.ValidatedStringParameter(
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
		self.assertEqual( p.presetValues(), ( IECore.StringData( "a" ), IECore.StringData( "b" ), IECore.StringData( "c" ), IECore.StringData( "d" ) ) )

class TestDirNameParameter( unittest.TestCase ) :

	def test( self ) :

		p = IECore.DirNameParameter(
			name = "f",
			description = "d",
			defaultValue = "test",
			check = IECore.DirNameParameter.CheckType.MustExist,
			allowEmptyString = True
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.mustExist, True )
		self.assertEqual( p.allowEmptyString, True )
		self.assertEqual (p.userData(), IECore.CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

	def testMustNotExist( self ):
		p = IECore.DirNameParameter(
				name = "f",
				description = "d",
				defaultValue = "/lucioSaysThisDirectoryDoesNotExist",
				check = IECore.DirNameParameter.CheckType.MustExist,
				allowEmptyString = True,
		)
		self.assertRaises( RuntimeError, p.validate )

class TestFileNameParameter( unittest.TestCase ) :

	def test( self ) :

		p = IECore.FileNameParameter(
			name = "f",
			description = "d",
			extensions = "tif tiff jpg cin",
			check = IECore.FileNameParameter.CheckType.DontCare,
			allowEmptyString = True
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.extensions, [ "tif", "tiff", "jpg", "cin" ] )
		self.assertEqual( p.mustExist, False )
		self.assertEqual( p.allowEmptyString, True )
		self.assertEqual (p.userData(), IECore.CompoundObject() )

		for e in p.extensions :
			p.setValidatedValue( IECore.StringData("hello." + e) )

		p.setValue( IECore.StringData( "test" ) )
		self.assertRaises( RuntimeError, p.validate )

	def testUserData( self ):
		compound = IECore.CompoundObject()
		compound["first"] = IECore.IntData()
		compound["second"] = IECore.QuatfData()
		compound["third"] = IECore.StringData("test")
		p = IECore.FileNameParameter(
			name = "f",
			description = "d",
			extensions = "tif tiff jpg cin",
			check = IECore.FileNameParameter.CheckType.DontCare,
			allowEmptyString = True,
			userData = compound
		)
		self.assertEqual( p.userData(), compound )
		self.assertTrue(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = IECore.CharData('1')
		data["first"] = data["fourth"]

	def testNoExtensions( self ) :

		p = IECore.FileNameParameter(
			name = "f",
			description = "d",
		)
		self.assertEqual( p.extensions, [] )

		p.setValue( IECore.StringData( "hello.tif" ) )
		p.setValue( IECore.StringData( "hello" ) )

	def testNotADirectory( self ) :

		p = IECore.FileNameParameter(
			name = "f",
			description = "d",
			defaultValue = "test",
			check = IECore.FileNameParameter.CheckType.MustExist,
			allowEmptyString = True
		)

		self.assertRaises( RuntimeError, p.validate )
		self.assertEqual( p.valueValid()[0], False )

class TestValidation( unittest.TestCase ) :

	def test( self ) :

		i = IECore.IntParameter( name = "n", description = "d", defaultValue = 10 )
		self.assertTrue( i.valueValid( IECore.IntData( 1 ) ) )
		self.assertTrue( not i.valueValid( IECore.FloatData( 1 ) )[0] )

	def testLazyValidation( self ) :

		i = IECore.IntParameter( name = "n", description = "d", defaultValue = 10 )
		i.validate( IECore.IntData( 10 ) )
		self.assertRaises( RuntimeError, i.validate, IECore.FloatData( 20 ) )
		i.setValue( IECore.IntData( 10 ) )
		i.validate()
		i.setValue( IECore.FloatData( 10 ) )
		self.assertRaises( RuntimeError, i.validate )
		self.assertRaises( RuntimeError, i.getValidatedValue )

		i = IECore.V3fParameter( name = "n", description = "d", defaultValue = imath.V3f( 10 ) )
		i.validate( IECore.V3fData( imath.V3f( 10 ) ) )
		self.assertRaises( RuntimeError, i.validate, IECore.FloatData( 20 ) )
		i.setValue( IECore.V3fData( imath.V3f( 20 ) ) )
		i.validate()
		i.setValue( IECore.FloatData( 10 ) )
		self.assertRaises( RuntimeError, i.validate )
		self.assertRaises( RuntimeError, i.getValidatedValue )

class TestObjectParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		p = IECore.ObjectParameter( name = "name", description = "description", defaultValue = IECore.CompoundObject(), type = IECore.TypeId.CompoundObject )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IECore.CompoundObject() )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.getCurrentPresetName(), "" )
		self.assertEqual( p.validTypes(), [IECore.TypeId.CompoundObject] )

		self.assertTrue( p.valueValid( IECore.CompoundObject() )[0] )
		self.assertTrue( not p.valueValid( IECore.IntData( 1 ) )[0] )

	def testConstructor2( self ) :

		p = IECore.ObjectParameter( name = "name", description = "description", defaultValue = IECore.CompoundObject(), types = [ IECore.TypeId.CompoundObject, IECore.TypeId.FloatData ] )
		self.assertEqual( p.name, "name" )
		self.assertEqual( p.description, "description" )
		self.assertEqual( p.defaultValue, IECore.CompoundObject() )
		self.assertEqual( p.getValue(), p.defaultValue )
		self.assertEqual( p.getCurrentPresetName(), "" )
		self.assertEqual( len( p.validTypes() ), 2 )
		self.assertTrue( IECore.TypeId.CompoundObject in p.validTypes() )
		self.assertTrue( IECore.TypeId.FloatData in p.validTypes() )

		self.assertTrue( p.valueValid( IECore.CompoundObject() )[0] )
		self.assertTrue( p.valueValid( IECore.FloatData( 1 ) )[0] )
		self.assertTrue( not p.valueValid( IECore.IntData( 1 ) )[0] )

	def testUserData( self ) :

		p = IECore.ObjectParameter( name = "name", description = "description", defaultValue = IECore.ObjectVector(), type = IECore.TypeId.ObjectVector, userData = IECore.CompoundObject( { "A" : IECore.IntData( 10 ) } ) )
		self.assertEqual( p.userData(), IECore.CompoundObject( { "A" : IECore.IntData( 10 ) } ) )

		p = IECore.ObjectParameter( name = "name", description = "description", defaultValue = IECore.ObjectVector(), type = IECore.TypeId.ObjectVector )
		self.assertEqual (p.userData(), IECore.CompoundObject() )

	def testErrorMessage( self ) :

		p = IECore.ObjectParameter( name = "name", description = "description", defaultValue = IECore.FloatData( 1 ), types = [IECore.TypeId.FloatData] )
		self.assertEqual( p.valueValid( IECore.V3fData( imath.V3f( 1 ) ) )[1], "Object is not of type FloatData" )

		p = IECore.ObjectParameter( name = "name", description = "description", defaultValue = IECore.FloatData( 1 ), types = [IECore.TypeId.FloatData, IECore.TypeId.IntData] )
		self.assertEqual( p.valueValid( IECore.V3fData( imath.V3f( 1 ) ) )[1], "Object is not of type FloatData or IntData" )

		p = IECore.ObjectParameter( name = "name", description = "description", defaultValue = IECore.FloatData( 1 ), types = [IECore.TypeId.FloatData, IECore.TypeId.DoubleData, IECore.TypeId.IntData] )
		self.assertEqual( p.valueValid( IECore.V3fData( imath.V3f( 1 ) ) )[1], "Object is not of type FloatData, DoubleData or IntData" )

	def testOrderedPresets( self ) :

		p = IECore.ObjectParameter(
			name = "n",
			description = "d",
			defaultValue = IECore.FloatData( 20 ),
			types = [ IECore.Object.staticTypeId() ],
			presets = (
				( "p1", IECore.FloatData( 40 ) ),
				( "p2", IECore.IntData( 60 ) ),
				( "p3", IECore.CompoundData() ),
				( "p4", IECore.FloatData( 20 ) ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual( p.presetValues(), ( IECore.FloatData( 40 ), IECore.IntData( 60 ), IECore.CompoundData(), IECore.FloatData( 20 ) ) )

		# overriding presets
		p.setPresets(
			[
				( "four", IECore.V3fData( imath.V3f(4) ) ),
				( "one", IECore.V3fData( imath.V3f(1) ) ),
			]
		)
		pr = p.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["four"], IECore.V3fData( imath.V3f(4) ) )
		self.assertEqual( pr["one"], IECore.V3fData( imath.V3f(1) ) )
		self.assertEqual( p.presetNames(), ("four", "one") )
		p.setValue("four")
		self.assertEqual( p.getValue(), IECore.V3fData(imath.V3f(4)) )

class TestTypedObjectParameter( unittest.TestCase ) :

	def testConstructor( self ) :

		objectVector = IECore.ObjectVector()
		p = IECore.ObjectVectorParameter( "n", "d", objectVector )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, objectVector )
		self.assertEqual( p.getValue(), objectVector )
		self.assertEqual( p.userData(), IECore.CompoundObject() )

	def testUserData( self ):
		compound = IECore.CompoundObject()
		compound["first"] = IECore.IntData()
		compound["second"] = IECore.QuatfData()
		compound["third"] = IECore.StringData("test")
		p = IECore.ObjectVectorParameter( "name", "description", IECore.ObjectVector(), userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assertTrue(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = IECore.CharData('1')
		data["first"] = data["fourth"]

	def testPresets( self ) :

		preset1 = IECore.ObjectVector( [ IECore.IntData( 1 ) ] )
		preset2 = IECore.ObjectVector( [ IECore.IntData( 2 ) ] )
		preset3 = IECore.ObjectVector( [ IECore.IntData( 3 ) ] )

		p = IECore.ObjectVectorParameter(
			name = "n",
			description = "d",
			defaultValue = preset2,
			presets = (
				( "one", preset1 ),
				( "two", preset2 ),
				( "three", preset3 ),
			),
			presetsOnly = True,
		)

		pr = p.getPresets()
		self.assertEqual( len( pr ), 3 )
		self.assertEqual( pr["one"], preset1 )
		self.assertEqual( pr["two"], preset2 )
		self.assertEqual( pr["three"], preset3 )

		p.setValue( "one" )
		self.assertEqual( p.getValue(), preset1 )

	def testPresetsOnly( self ) :

		preset1 = IECore.ObjectVector( [ IECore.IntData( 1 ) ] )
		preset2 = IECore.ObjectVector( [ IECore.IntData( 2 ) ] )
		preset3 = IECore.ObjectVector( [ IECore.IntData( 3 ) ] )

		four = IECore.ObjectVector( [ IECore.IntData( 4 ) ] )

		p = IECore.ObjectVectorParameter(
			name = "n",
			description = "d",
			defaultValue = preset2,
			presets = (
				( "one", preset1 ),
				( "two", preset2 ),
				( "three", preset3 ),
			),
			presetsOnly = True,
		)

		self.assertRaises( RuntimeError, p.setValidatedValue, four )

		p = IECore.ObjectVectorParameter(
			name = "n",
			description = "d",
			defaultValue = preset2,
			presets = (
				( "one", preset1 ),
				( "two", preset2 ),
				( "three", preset3 ),
			),
			presetsOnly = False,
		)

		p.setValidatedValue( four )

	def testOrderedPresets( self ) :

		p = IECore.ObjectVectorParameter(
			name = "n",
			description = "d",
			defaultValue = IECore.ObjectVector( [ IECore.IntData( 1 ) ] ),
			presets = (
				( "p1", IECore.ObjectVector( [ IECore.IntData( 1 ) ] ) ),
				( "p2", IECore.ObjectVector( [ IECore.IntData( 2 ) ] ) ),
				( "p3", IECore.ObjectVector( [ IECore.IntData( 3 ) ] ) ),
				( "p4", IECore.ObjectVector( [ IECore.IntData( 4 ) ] ) ),
			),
			presetsOnly = True,
		)

		self.assertEqual( p.presetNames(), ( "p1", "p2", "p3", "p4" ) )
		self.assertEqual(
			p.presetValues(),
			(
				IECore.ObjectVector( [ IECore.IntData( 1 ) ] ),
				IECore.ObjectVector( [ IECore.IntData( 2 ) ] ),
				IECore.ObjectVector( [ IECore.IntData( 3 ) ] ),
				IECore.ObjectVector( [ IECore.IntData( 4 ) ] ),
			)
		)

class TestIntVectorParameter( unittest.TestCase ) :

	def test( self ) :

		dv = IECore.IntVectorData()

		p = IECore.IntVectorParameter(
			name = "f",
			description = "d",
			defaultValue = dv,
			presets = (
				( "preset1", IECore.IntVectorData( [ 1, 2 ] ) ),
			)
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.userData(), IECore.CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

class TestTransformationMatixParameter( unittest.TestCase ) :

	def test( self ) :

		tm = IECore.TransformationMatrixfData()
		p = IECore.TransformationMatrixfParameter(
			name = "f",
			description = "d",
			defaultValue = tm,
		)
		p.validate()

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.valueValid()[0], True )

		self.assertTrue( isinstance( p.getTypedValue().translate, imath.V3f ) )
		self.assertEqual( p.getTypedValue().translate, imath.V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotate, imath.Eulerf( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotationOrientation, imath.Quatf( 1,0,0,0 ) )
		self.assertEqual( p.getTypedValue().scale, imath.V3f( 1,1,1 ) )
		self.assertEqual( p.getTypedValue().shear, imath.V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivot, imath.V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivotTranslation, imath.V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivot, imath.V3f( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivotTranslation, imath.V3f( 0,0,0 ) )

		tm = IECore.TransformationMatrixdData()
		p = IECore.TransformationMatrixdParameter(
			name = "f",
			description = "d",
			defaultValue = tm,
		)
		p.validate()

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.valueValid()[0], True )

		self.assertTrue( isinstance( p.getTypedValue().translate, imath.V3d ) )
		self.assertEqual( p.getTypedValue().translate, imath.V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotate, imath.Eulerd( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotationOrientation, imath.Quatd( 1,0,0,0 ) )
		self.assertEqual( p.getTypedValue().scale, imath.V3d( 1,1,1 ) )
		self.assertEqual( p.getTypedValue().shear, imath.V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivot, imath.V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().rotatePivotTranslation, imath.V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivot, imath.V3d( 0,0,0 ) )
		self.assertEqual( p.getTypedValue().scalePivotTranslation, imath.V3d( 0,0,0 ) )

class TestPathVectorParameter( unittest.TestCase ) :

	def test( self ) :

		dv = IECore.StringVectorData()

		p = IECore.PathVectorParameter(
			name = "f",
			description = "d",
			defaultValue = dv,
			check = IECore.PathVectorParameter.CheckType.MustExist,
			allowEmptyList = True,
			presets = (
				( "preset1", IECore.StringVectorData( [ 'one', 'two' ] ) ),
			)
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.mustExist, True )
		self.assertEqual( p.allowEmptyList, True )
		self.assertEqual( p.userData(), IECore.CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

	def testMustNotExist( self ):

		dv = IECore.StringVectorData()
		dv.append( "/ThisDirectoryDoesNotExist " )

		p = IECore.PathVectorParameter(
				name = "f",
				description = "d",
				defaultValue = dv,
				check = IECore.PathVectorParameter.CheckType.MustExist,
				allowEmptyList = False,
		)
		self.assertRaises( RuntimeError, p.validate )

class TestFileSequenceVectorParameter( unittest.TestCase ) :

	def test( self ) :

		dv = IECore.StringVectorData()

		p = IECore.FileSequenceVectorParameter(
			name = "f",
			description = "d",
			defaultValue = dv,
			check = IECore.FileSequenceVectorParameter.CheckType.MustExist,
			allowEmptyList = True,
			presets = (
				( "preset1", IECore.StringVectorData( [ 'one', 'two' ] ) ),
			)
		)

		self.assertEqual( p.name, "f" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.mustExist, True )
		self.assertEqual( p.allowEmptyList, True )
		self.assertEqual( p.userData(), IECore.CompoundObject() )
		self.assertEqual( p.valueValid()[0], True )
		p.validate()

if __name__ == "__main__":
        unittest.main()
