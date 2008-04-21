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
	
		p = Parameter(
			name = "n",
			description = "d",
			defaultValue = FloatData( 20 ),
			presets = {
				"p1" : FloatData( 40 ),
				"p2" : IntData( 60 ),
				"p3" : CompoundData(),
				"p4" : FloatData( 20 ),
			},
			presetsOnly = True,
		)
		
		pr = p.presets()
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
			presets = {
				"one" : 1,
				"two" : 2,
				"three" : 3,
			} 
		)
		
		pr = p.presets()
		self.assertEqual( len( pr ), 3 )
		self.assertEqual( pr["one"], IntData( 1 ) )
		self.assertEqual( pr["two"], IntData( 2 ) )
		self.assertEqual( pr["three"], IntData( 3 ) )
		
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
			presets = {
				"one" : V3f( 1 ),
				"two" : V3f( 2 ),
				"three" : V3f( 3 ),
			},
			presetsOnly = True, 
		)
		
		pr = p.presets()
		self.assertEqual( len( pr ), 3 )
		self.assertEqual( pr["one"], V3fData( V3f( 1 ) ) )
		self.assertEqual( pr["two"], V3fData( V3f( 2 ) ) )
		self.assertEqual( pr["three"], V3fData( V3f( 3 ) ) )
		
		p.setValue( "one" )
		self.assertEqual( p.getValue(), V3fData( V3f( 1 ) ) )
	
	def testPresetsOnly( self ) :
	
		p = V3fParameter(
			name = "n",
			description = "d",
			defaultValue = V3f( 2 ),
			presets = {
				"one" : V3f( 1 ),
				"two" : V3f( 2 ),
				"three" : V3f( 3 ),
			},
			presetsOnly = True, 
		)
		
		self.assertRaises( RuntimeError, p.setValidatedValue, V3fData( V3f( 20 ) ) )	

		p = V3fParameter(
			name = "n",
			description = "d",
			defaultValue = V3f( 2 ),
			presets = {
				"one" : V3f( 1 ),
				"two" : V3f( 2 ),
				"three" : V3f( 3 ),
			},
			presetsOnly = False, 
		)
		
		p.setValue( V3fData( V3f( 20 ) ) )
		
	def testTypedValueFns( self ) :
	
		p = StringParameter( name="n", description="d", defaultValue = "10" )
		self.assertEqual( p.getTypedValue(), "10" )
		p.setTypedValue( "20" )
		self.assertEqual( p.getTypedValue(), "20" )
		
		p = V3fParameter( name="n", description="d", defaultValue = V3f( 1, 2, 3 ) )
		self.assertEqual( p.getTypedValue(), V3f( 1, 2, 3 ) )
		p.setTypedValue( V3f( 12, 13, 14 ) )
		self.assertEqual( p.getTypedValue(), V3f( 12, 13, 14 ) )

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
		
class TestCompoundParameter( unittest.TestCase ) :

	def testZEndGarbageCollection( self ):
		import gc
		# test if garbage collection is still working after all tests with compound parameters.
		gc.collect()
		RefCounted.collectGarbage()

		self.assertEqual( RefCounted.numWrappedInstances(), 0 )

	def testUserData( self ):

		p = CompoundParameter( "n", "d", [], userData = CompoundObject( { "test": StringData("hi"), "test2": IntData(2), "test3": CompoundObject( { "test4": FloatData( 1.0 ) } ) } ) )

		p2 = CompoundParameter( "n", "d", [], userData = { "test": StringData("hi"), "test2": IntData(2), "test3": { "test4": FloatData( 1.0 ) } } )
 
		self.assertEqual( p.userData(), p2.userData() )

	def testDerivedClassElement( self ):
		
		class DerivedStringParameter( StringParameter ):
			pass

		p = DerivedStringParameter( "a", "a", "contents" )
		c = CompoundParameter( "n", "d", members = [ p ] )
		self.assertEqual( type( c[ "a" ] ), DerivedStringParameter )

	def testDerivedClass( self ):

		class DerivedCompoundParameter( CompoundParameter ):

			def valueValid( self, value ) :
				return ( True, "" )

		p = DerivedCompoundParameter( "n", "d", members = [ StringParameter( "a", "a", "", presets = { "b": StringData( "b" ) }, presetsOnly = True ) ] )
		p.validate()

	def testConstructor( self ) :
		p = CompoundParameter( "n", "d" )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, CompoundObject() )
		self.assertEqual( len( p.keys() ), 0 )
		self.assertEqual( len( p.values() ), 0 )
		self.assertEqual( len( p ), 0 )
		self.assertEqual (p.userData(), CompoundObject() )
		
		p = CompoundParameter( "n", "d", [] )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, CompoundObject() )
		self.assertEqual( len( p.keys() ), 0 )
		self.assertEqual( len( p.values() ), 0 )
		self.assertEqual( len( p ), 0 )
		self.assertEqual (p.userData(), CompoundObject() )
	
		p = CompoundParameter(
			name = "compound",
			description = "innit nice",
			members = [
				IntParameter( "i", "d", 1 ),
				FloatParameter( "f", "d", 2 ),
			]
		)
		
		
		d = CompoundObject()
		d["i"] = IntData( 1 )
		d["f"] = FloatData( 2 )
		self.assertEqual( p.name, "compound" )
		self.assertEqual( p.description, "innit nice" )
		self.assertEqual( p.defaultValue, d )
		self.assertEqual( len( p.keys() ), 2 )
		self.assertEqual( len( p.values() ), 2 )
		self.assertEqual( len( p ), 2 )
		self.assertEqual( p.keys(), ["i", "f"] )
		self.assertEqual( p.values()[0].name, "i" )
		self.assertEqual( p.values()[1].name, "f" )

	def testConstDefaultValue( self ):
		a = CompoundParameter( "a", "a desc", 
			members = [
				StringParameter( "b", "b desc", "ok"),
			]
		)
		c = a.getValue()
		c.b.value = 'error!'
		self.assertEqual( a.b.defaultValue.value, "ok")

	def testUserData( self ):
		compound = CompoundObject()
		compound["first"] = IntData()
		compound["second"] = QuatfData()
		compound["third"] = StringData("test")
		p = CompoundParameter( "n", "d", [], userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = CharData('1')
		data["first"] = data["fourth"]
		
	def testAccess( self ) :
	
		p = CompoundParameter(
			name = "compound",
			description = "innit nice",
			members = [
				IntParameter( "i", "d", 1 ),
				FloatParameter( "f", "d", 2 ),
			]
		)
		
		self.assertEqual( p["i"].name, "i" )
		self.assertEqual( p.i.name, "i" )
		self.assertEqual( p.parameter( "i" ).name, "i" )
			
	def testPresets( self ) :
		
		p = CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 1, presets = {
						"one" : 1,
						"two" : 2,
						"ambiguous" : 4,
						"four" : 4,
						"otherAmbiguous" : 4,
					},
					presetsOnly = True,
				),
				FloatParameter( "f", "d", 2, presets = {
						"one" : 1,
						"two" : 2,
						"three" : 3,
						"four": 4,
					},
					presetsOnly = True,
				)
			]
		)
		
		self.assertEqual( p.presetsOnly, True )
		
		pr = p.presets()
		self.assertEqual( len( pr ), 3 )
		self.assert_( "one" in pr.keys() )
		self.assert_( "two" in pr.keys() )
		
		p.setValue( "two" )
		self.assertEqual( p["i"].getValue().value, 2 )
		self.assertEqual( p["f"].getValue().value, 2 )
		self.assertEqual( p.getCurrentPresetName(), "two" )

		p.setValue( "four" )
		self.assertEqual( p.getCurrentPresetName(), "four" )
		
		p = CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 1, presets = {
						"one" : 1,
						"two" : 2,
					},
					presetsOnly = True,
				),
				FloatParameter( "f", "d", 1, presets = {
						"one" : 1,
						"two" : 2,
						"three" : 3,
					},
					presetsOnly = True,
				)
			]
		)
		
		self.assertEqual( p.presetsOnly, True )
		
	def testLateValidation( self ) :
	
		p = CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 1 ),
				FloatParameter( "f", "d", 2 )
			]
		)
		
		p.validate()
		p.setValue( CompoundObject( { "i" : IntData( 10 ), "f" : FloatData( 20 ) } ) )
		p.validate()
		self.assertEqual( p.i.getValue(),  IntData( 10 ) )
		self.assertEqual( p.f.getValue(),  FloatData( 20 ) )
		
		p.setValue( CompoundObject( { "i" : IntData( 10 ) } ) )
		self.assertRaises( RuntimeError, p.validate )
		self.assertRaises( RuntimeError, p.getValidatedValue )
		p.f.setValue( FloatData( 20 ) )
		p.validate()

		p.setValue( CompoundObject( { "idontbelong" : IntData( 10 ), "i" : IntData( 10 ), "f" : FloatData( 20 ) } ) )
		self.assertRaises( RuntimeError, p.validate )
		self.assertRaises( RuntimeError, p.getValidatedValue )
		del p.getValue()["idontbelong"]
		p.validate()	

	def testAddParameters( self ) :

		p = CompoundParameter(
			name = "c",
			description = "d",
			members = []
		)
		
		self.assertEqual( len( p ), 0 )
		
		p.addParameters(
			[
				IntParameter( "i", "d", 1 ),
				FloatParameter( "f", "d", 2 )
			]
		)
		
		self.assertEqual( len( p ), 2 )
		
	def testAddParametersDefault( self ) :
	
		p = CompoundParameter(
			name = "c",
			description = "d",
			members = []
		)
		
		self.assertEqual( p.defaultValue, CompoundObject() )
		
		p.addParameter( IntParameter( name = "i", description = "d", defaultValue = 10 ) )
		
		self.assertEqual( len( p.defaultValue ), 1 )
		self.assertEqual( p.defaultValue, CompoundObject( { "i" : IntData( 10 ) } ) )
		
		p.addParameter( FloatParameter( name = "f", description = "d", defaultValue = 20 ) )
		
		self.assertEqual( len( p.defaultValue ), 2 )
		self.assertEqual( p.defaultValue, CompoundObject( { "i" : IntData( 10 ), "f" : FloatData( 20 ) } ) )

	def testRemoveParameters( self ) :
		a = CompoundParameter( "a", "a desc", 
				members = [
					StringParameter( "b", "b desc", "test 1 ok!"),
					StringParameter( "d", "d desc", "test 2 failed!"),
				]
			)
		c = a.getValue()
		r = a.defaultValue
		a.removeParameter( "d" )
		r = a.defaultValue
		try:
			r['d']
		except:
			pass
		else:
			raise Exception, "Should have generated an exception."

		r = a.getValue()
		try:
			r['d']
		except:
			pass
		else:
			raise Exception, "Should have generated an exception."
		
	def testAddParametersPresets( self ) :
	
		p = CompoundParameter(
			name = "c",
			description = "d",
			members = []
		)
		
		self.assertEqual( p.presets(), {} )
		
		p.addParameter( IntParameter( name = "i", description = "d", defaultValue = 10, presets = { "one" : 1, "two" : 2  } ) )
		
		self.assertEqual( len( p.presets() ), 2 )
		self.assertEqual( p.presets(), { "one" : CompoundObject( { "i" : IntData( 1 ) } ), "two" : CompoundObject( { "i" : IntData( 2 ) } ) } )
		
		fParam = FloatParameter( name = "f", description = "d", defaultValue = 20, presets = { "one" : 1 } )
		p.addParameter( fParam )

		self.assertEqual( len( p.presets() ), 1 )
		self.assertEqual( p.presets(), { "one" : CompoundObject( { "i" : IntData( 1 ), "f" : FloatData( 1 ) } ) } )

		p.insertParameter( IntParameter( name = "x", description = "x", defaultValue = 10 ), fParam )
		self.assertEqual( p.keys(), [ "i", "x", "f" ] )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 1 ),
				FloatParameter( "f", "d", 2 )
			],
			userData = CompoundObject( { "a": BoolData( False ) } )
		)

		q = CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 10 ),
				FloatParameter( "f", "d", 20 )
			],
		)

		self.assert_( p["i"].getTypedValue() == 1 )
		self.assert_( p["f"].getValue() == FloatData( 2 ) )
		p.smartSetValue( CompoundObject( { "i": IntData(10), "f": FloatData(20) } ) )
		self.assert_( p["i"].getTypedValue() == 10 )
		self.assert_( p["f"].getValue() == FloatData( 20 ) )
		p.smartSetValue( { "i": 4, "f": 4 } )
		self.assert_( p["i"].getTypedValue() == 4 )
		self.assert_( p["f"].getValue() == FloatData( 4 ) )

		# adding another CompoundParameter
		p.addParameter( q )
		r = p.getValue().copy()
		r['c']['i'].value = 15
		self.assert_( p['c']['i'].getTypedValue() == 10 )
		p.smartSetValue( r )
		self.assert_( p['c']['i'].getTypedValue() == 15 )
		p.smartSetValue( { 'i': 1, 'f': 2, 'c': { 'i': 3, 'f': 4 } } )
		self.assert_( p['i'].getTypedValue() == 1 )
		self.assert_( p['f'].getValue() == FloatData( 2 ) )
		self.assert_( p['c']['i'].getTypedValue() == 3 )
		self.assert_( p['c']['f'].getValue() == FloatData( 4 ) )

	def testSmartSetItem( self ):
		"""Test smart __setitem__"""
		p = CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 1 ),
			],
		)
		self.assert_( p["i"].getTypedValue() == 1 )
		p["i"] = 20
		self.assert_( p["i"].getTypedValue() == 20 )
		p["i"] = IntData(30)
		self.assert_( p["i"].getTypedValue() == 30 )
				
class TestValidatedStringParameter( unittest.TestCase ) :

	def test( self ) :
	
		p = ValidatedStringParameter(
			name = "n",
			description = "d",
			regex = "[0-9]*",
			regexDescription = "Value must be an integer",
			presets = {
				"100" : "100",
				"200" : "200",
			}
		)
		
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.regex, "[0-9]*" )
		self.assertEqual (p.userData(), CompoundObject() )
		
		self.assertRaises( RuntimeError, p.setValidatedValue, StringData( "A" ) )
		p.setValue( StringData( "100" ) )
		self.assertEqual( p.getValue(), StringData( "100" ) )
		
		pr = p.presets()
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
			presets = {
				"one" : mesh1,
				"two" : mesh2,
				"three" : mesh3,
			},
			presetsOnly = True, 
		)
		
		pr = p.presets()
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
			presets = {
				"one" : mesh1,
				"two" : mesh2,
				"three" : mesh3,
			},
			presetsOnly = True, 
		)
		
		self.assertRaises( RuntimeError, p.setValidatedValue, mesh4 )	

		p = MeshPrimitiveParameter(
			name = "n",
			description = "d",
			defaultValue = mesh2,
			presets = {
				"one" : mesh1,
				"two" : mesh2,
				"three" : mesh3,
			},
			presetsOnly = False, 
		)
		
		p.setValue( mesh4 )	

class TestPathVectorParameter( unittest.TestCase ) :

	def test( self ) :
	
		dv = StringVectorData()
	
		p = PathVectorParameter(
			name = "f",
			description = "d",
			defaultValue = dv,
			check = PathVectorParameter.CheckType.MustExist,
			allowEmptyList = True
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
										
if __name__ == "__main__":
        unittest.main()
