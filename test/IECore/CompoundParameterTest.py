##########################################################################
#
#  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

class CompoundParameterTest( unittest.TestCase ) :

	def testZEndGarbageCollection( self ):
		import gc
		# test if garbage collection is still working after all tests with compound parameters.
		gc.collect()
		IECore.RefCounted.collectGarbage()

		self.assertEqual( IECore.RefCounted.numWrappedInstances(), 0 )

	def testUserData( self ):

		p = IECore.CompoundParameter( "n", "d", [], userData = IECore.CompoundObject( { "test": IECore.StringData("hi"), "test2": IECore.IntData(2), "test3": IECore.CompoundObject( { "test4": IECore.FloatData( 1.0 ) } ) } ) )

		p2 = IECore.CompoundParameter( "n", "d", [], userData = { "test": IECore.StringData("hi"), "test2": IECore.IntData(2), "test3": { "test4": IECore.FloatData( 1.0 ) } } )

		self.assertEqual( p.userData(), p2.userData() )

	def testDerivedClassElement( self ):

		class DerivedStringParameter( IECore.StringParameter ):
			pass

		p = DerivedStringParameter( "a", "a", "contents" )
		c = IECore.CompoundParameter( "n", "d", members = [ p ] )
		self.assertEqual( type( c[ "a" ] ), DerivedStringParameter )

	def testDerivedClass( self ):

		class DerivedCompoundParameter( IECore.CompoundParameter ):

			def valueValid( self, value ) :
				return ( True, "" )

		p = DerivedCompoundParameter( "n", "d", members = [ IECore.StringParameter( "a", "a", "", presets = ( ( "b", IECore.StringData( "b" ) ), ), presetsOnly = True ) ] )
		p.validate()

	def testConstructor( self ) :
		p = IECore.CompoundParameter( "n", "d" )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, IECore.CompoundObject() )
		self.assertEqual( len( p.keys() ), 0 )
		self.assertEqual( len( p.values() ), 0 )
		self.assertEqual( len( p ), 0 )
		self.assertEqual (p.userData(), IECore.CompoundObject() )

		p = IECore.CompoundParameter( "n", "d", [] )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( p.defaultValue, IECore.CompoundObject() )
		self.assertEqual( len( p.keys() ), 0 )
		self.assertEqual( len( p.values() ), 0 )
		self.assertEqual( len( p ), 0 )
		self.assertEqual (p.userData(), IECore.CompoundObject() )

		p = IECore.CompoundParameter(
			name = "compound",
			description = "innit nice",
			members = [
				IECore.IntParameter( "i", "d", 1 ),
				IECore.FloatParameter( "f", "d", 2 ),
			]
		)


		d = IECore.CompoundObject()
		d["i"] = IECore.IntData( 1 )
		d["f"] = IECore.FloatData( 2 )
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
		a = IECore.CompoundParameter( "a", "a desc",
			members = [
				IECore.StringParameter( "b", "b desc", "ok"),
			]
		)
		c = a.getValue()
		c["b"].value = 'error!'
		self.assertEqual( a["b"].defaultValue.value, "ok")

	def testUserData( self ):
		compound = IECore.CompoundObject()
		compound["first"] = IECore.IntData()
		compound["second"] = IECore.QuatfData()
		compound["third"] = IECore.StringData("test")
		p = IECore.CompoundParameter( "n", "d", [], userData = compound )
		self.assertEqual( p.userData(), compound )
		self.assert_(not p.userData().isSame(compound) )
		data = p.userData()
		data["fourth"] = IECore.CharData('1')
		data["first"] = data["fourth"]

	def testAccess( self ) :

		p = IECore.CompoundParameter(
			name = "compound",
			description = "innit nice",
			members = [
				IECore.IntParameter( "i", "d", 1 ),
				IECore.FloatParameter( "f", "d", 2 ),
			]
		)

		self.assertEqual( p["i"].name, "i" )
		self.assertEqual( p.parameter( "i" ).name, "i" )

	def testPresets( self ) :

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 1, presets = (
						( "one", 1 ),
						( "two", 2 ),
						( "ambiguous", 4 ),
						( "four", 4 ),
						( "otherAmbiguous", 4 ),
					),
					presetsOnly = True,
				),
				IECore.FloatParameter( "f", "d", 2, presets = (
						( "one", 1 ),
						( "two", 2 ),
						( "three", 3 ),
						( "four", 4 ),
					),
					presetsOnly = True,
				)
			]
		)

		self.assertEqual( p.presetsOnly, True )

		pr = p.getPresets()
		self.assertEqual( len( pr ), 3 )
		self.assert_( "one" in pr.keys() )
		self.assert_( "two" in pr.keys() )

		p.setValue( "two" )
		self.assertEqual( p["i"].getValue().value, 2 )
		self.assertEqual( p["f"].getValue().value, 2 )
		self.assertEqual( p.getCurrentPresetName(), "two" )

		p.setValue( "four" )
		self.assertEqual( p.getCurrentPresetName(), "four" )
		self.assertRaises( RuntimeError, p.setPresets, [] )		# CompoundParameter created with adoptChildPresets=True does not allow overriding presets

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 1, presets = (
						( "one", 1 ),
						( "two", 2 ),
					),
					presetsOnly = True,
				),
				IECore.FloatParameter( "f", "d", 1, presets = (
						( "one", 1 ),
						( "two", 2 ),
						( "three", 3 ),
					),
					presetsOnly = True,
				)
			]
		)

		self.assertEqual( p.presetsOnly, True )

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
		)

		self.assertEqual( p.presetsOnly, False )
		self.assertEqual( len( p.getPresets() ), 0 )

	def testLateValidation( self ) :

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 1 ),
				IECore.FloatParameter( "f", "d", 2 )
			]
		)

		p.validate()
		p.setValue( IECore.CompoundObject( { "i" : IECore.IntData( 10 ), "f" : IECore.FloatData( 20 ) } ) )
		p.validate()
		self.assertEqual( p["i"].getValue(),  IECore.IntData( 10 ) )
		self.assertEqual( p["f"].getValue(),  IECore.FloatData( 20 ) )

		p.setValue( IECore.CompoundObject( { "i" : IECore.IntData( 10 ) } ) )
		p.validate()
		p.getValidatedValue()
		p["f"].setValue( IECore.FloatData( 20 ) )
		p.validate()

		p.setValue( IECore.CompoundObject( { "idontbelong" : IECore.IntData( 10 ), "i" : IECore.IntData( 10 ), "f" : IECore.FloatData( 20 ) } ) )
		self.assertRaises( RuntimeError, p.validate )
		self.assertRaises( RuntimeError, p.getValidatedValue )
		del p.getValue()["idontbelong"]
		p.validate()

	def testAddParameters( self ) :

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = []
		)

		self.assertEqual( len( p ), 0 )

		p.addParameters(
			[
				IECore.IntParameter( "i", "d", 1 ),
				IECore.FloatParameter( "f", "d", 2 )
			]
		)

		self.assertEqual( len( p ), 2 )

	def testAddParametersDefault( self ) :

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = []
		)

		self.assertEqual( p.defaultValue, IECore.CompoundObject() )

		p.addParameter( IECore.IntParameter( name = "i", description = "d", defaultValue = 10 ) )

		self.assertEqual( len( p.defaultValue ), 1 )
		self.assertEqual( p.defaultValue, IECore.CompoundObject( { "i" : IECore.IntData( 10 ) } ) )

		p.addParameter( IECore.FloatParameter( name = "f", description = "d", defaultValue = 20 ) )

		self.assertEqual( len( p.defaultValue ), 2 )
		self.assertEqual( p.defaultValue, IECore.CompoundObject( { "i" : IECore.IntData( 10 ), "f" : IECore.FloatData( 20 ) } ) )

	def testRemoveParameters( self ) :
		a = IECore.CompoundParameter( "a", "a desc",
				members = [
					IECore.StringParameter( "b", "b desc", "test 1 ok!"),
					IECore.StringParameter( "d", "d desc", "test 2 failed!"),
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

	def testDelParameters( self ) :

		a = IECore.CompoundParameter( "a", "a desc",
				members = [
					IECore.StringParameter( "b", "b desc", "test 1 ok!"),
					IECore.StringParameter( "d", "d desc", "test 2 failed!"),
				]
			)

		c = a.getValue()
		r = a.defaultValue

		del a["d"]
		self.assert_( not "d" in a )

		r = a.defaultValue
		self.assert_( not "d" in r )

		r = a.getValue()
		self.assert_( not "d" in r )


	def testAddParametersPresets( self ) :

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = []
		)

		self.assertEqual( p.getPresets(), {} )

		p.addParameter( IECore.IntParameter( name = "i", description = "d", defaultValue = 10, presets = ( ( "one", 1 ), ( "two", 2 ) ) ) )

		self.assertEqual( len( p.getPresets() ), 2 )
		self.assertEqual( p.getPresets(), { "one" : IECore.CompoundObject( { "i" : IECore.IntData( 1 ) } ), "two" : IECore.CompoundObject( { "i" : IECore.IntData( 2 ) } ) } )

		fParam = IECore.FloatParameter( name = "f", description = "d", defaultValue = 20, presets = ( ( "one", 1 ), ) )
		p.addParameter( fParam )

		self.assertEqual( len( p.getPresets() ), 1 )
		self.assertEqual( p.getPresets(), { "one" : IECore.CompoundObject( { "i" : IECore.IntData( 1 ), "f" : IECore.FloatData( 1 ) } ) } )

		p.insertParameter( IECore.IntParameter( name = "x", description = "x", defaultValue = 10 ), fParam )
		self.assertEqual( p.keys(), [ "i", "x", "f" ] )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 1 ),
				IECore.FloatParameter( "f", "d", 2 )
			],
			userData = IECore.CompoundObject( { "a": IECore.BoolData( False ) } )
		)

		q = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 10 ),
				IECore.FloatParameter( "f", "d", 20 )
			],
		)

		self.assert_( p["i"].getTypedValue() == 1 )
		self.assert_( p["f"].getValue() == IECore.FloatData( 2 ) )
		p.smartSetValue( IECore.CompoundObject( { "i": IECore.IntData(10), "f": IECore.FloatData(20) } ) )
		self.assert_( p["i"].getTypedValue() == 10 )
		self.assert_( p["f"].getValue() == IECore.FloatData( 20 ) )
		p.smartSetValue( { "i": 4, "f": 4 } )
		self.assert_( p["i"].getTypedValue() == 4 )
		self.assert_( p["f"].getValue() == IECore.FloatData( 4 ) )

		# adding another CompoundParameter
		p.addParameter( q )
		r = p.getValue().copy()
		r['c']['i'].value = 15
		self.assert_( p['c']['i'].getTypedValue() == 10 )
		p.smartSetValue( r )
		self.assert_( p['c']['i'].getTypedValue() == 15 )
		p.smartSetValue( { 'i': 1, 'f': 2, 'c': { 'i': 3, 'f': 4 } } )
		self.assert_( p['i'].getTypedValue() == 1 )
		self.assert_( p['f'].getValue() == IECore.FloatData( 2 ) )
		self.assert_( p['c']['i'].getTypedValue() == 3 )
		self.assert_( p['c']['f'].getValue() == IECore.FloatData( 4 ) )

	def testSmartSetItem( self ):
		"""Test smart __setitem__"""
		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 1 ),
			],
		)
		self.assert_( p["i"].getTypedValue() == 1 )
		p["i"] = 20
		self.assert_( p["i"].getTypedValue() == 20 )
		p["i"] = IECore.IntData(30)
		self.assert_( p["i"].getTypedValue() == 30 )

	def testAttributeAccessRemoval( self ) :

		# we used to allow access to child parameters
		# using the parent.child attribute notation, but
		# after deprecating it in version 4 we removed it
		# in version 5. check that it's removed.

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 1 ),
			],
		)

		self.assertRaises( AttributeError, getattr, p, "i" )

	def testParameterPath( self ) :

		p = IECore.CompoundParameter(
			name = "c",
			description = "d",
			members = [
				IECore.IntParameter( "i", "d", 1, ),
				IECore.FloatParameter( "f", "d", 2, ),
				IECore.CompoundParameter( "c", "d", members = [
						IECore.IntParameter( "j", "d", 10 ),
					]
				)
			]
		)

		self.assertEqual( p.parameterPath( p["i"] ), [ "i" ] )
		self.assertEqual( p.parameterPath( p["f"] ), [ "f" ] )
		self.assertEqual( p.parameterPath( p["c"]["j"] ), [ "c", "j" ] )
		self.assertEqual( p.parameterPath( IECore.IntParameter( "i", "d", 10 ) ), [] )
		self.assertEqual( p["c"].parameterPath( p["c"]["j"] ), [ "j" ] )

	def testParameterPathBug( self ) :

		p = IECore.CompoundParameter( name="c", description="" )
		p.addParameter(

			IECore.CompoundParameter(
				name = "n",
				description = "",
				members = [
					IECore.IntParameter( name="i", description="", defaultValue = 1 ),
					IECore.CompoundParameter( name="j", description="", members = [ IECore.IntParameter( "k", "", 10 ) ] )
				]
			)

		)

		self.assertEqual( p.parameterPath( p["n"]["i"] ), [ "n", "i" ] )
		self.assertEqual( p.parameterPath( p["n"]["j"]["k"] ), [ "n", "j", "k" ] )

	def testClearParameters( self ) :

		a = IECore.CompoundParameter( "a", "a desc",
			members = [
				IECore.StringParameter( "b", "b desc", "test 1 ok!"),
				IECore.StringParameter( "d", "d desc", "test 2 failed!"),
			]
		)

		self.assertEqual( len( a ), 2 )

		a.clearParameters()
		self.assertEqual( len( a ), 0 )
		self.assertEqual( a.keys(), [] )
		self.assertEqual( a.values(), [] )

		self.assertRaises( Exception, a.__getitem__, "b" )
		self.assertRaises( Exception, a.__getitem__, "d" )

	def testSetValueWithMissingData( self ) :

		c = IECore.CompoundParameter()

		c1 = IECore.StringParameter( "child1", "child1", "child1" )
		c.addParameter( c1 )

		preset = c.getValue()

		c2 = IECore.StringParameter( "child2", "child2", "child2" )
		c2value = c2.getValue()
		c.addParameter( c2 )

		c.setValue( preset )

		self.assertEqual( c2value, c["child2"].getValue() )

	def testItems( self ) :

		a = IECore.CompoundParameter( "a", "a desc",
			members = [
				IECore.StringParameter( "b", "b desc", "test 1 ok!"),
				IECore.StringParameter( "d", "d desc", "test 2 failed!"),
			]
		)

		items = a.items()
		self.assertEqual( len( items ), 2 )
		self.assertEqual( len( items[0] ), 2 )
		self.assertEqual( len( items[1] ), 2 )
		self.assertEqual( items[0][0], "b" )
		self.assertEqual( items[1][0], "d" )
		self.failUnless( items[0][1].isSame( a["b"] ) )
		self.failUnless( items[1][1].isSame( a["d"] ) )

	def testValueValidReason( self ) :

		i = IECore.IntParameter( "i", "", 1, 0, 10 )
		c = IECore.CompoundParameter(
			"c",
			members = [
				i
			]
		)

		childReason = i.valueValid( IECore.IntData( 20 ) )[1]
		compoundReason = c.valueValid( IECore.CompoundObject( { "i" : IECore.IntData( 20 ) } ) )[1]

		self.assertEqual( compoundReason, "i : " + childReason )

		cc = IECore.CompoundParameter(
			members = [
				c
			]
		)

		compoundCompoundReason = cc.valueValid( IECore.CompoundObject( { "c" : { "i" : IECore.IntData( 20 ) } } ) )[1]

		self.assertEqual( compoundCompoundReason, "c.i : " + childReason )

	def testAdoptChildPresets( self ) :

		# backward compatible behaviour

		c = IECore.CompoundParameter(
			"c",
			members = [
				IECore.IntParameter(
					"a",
					"description",
					1,
					presets = (
						( "one", 1 ),
						( "two", 2 ),
					),
					presetsOnly = True,
				),
				IECore.IntParameter(
					"b",
					"description",
					1,
					presets = (
						( "one", 1 ),
						( "two", 2 ),
					),
					presetsOnly = True,
				),
			],
		)

		self.assertEqual( len( c.getPresets() ), 2 )
		self.assertEqual( c.presetsOnly, True )

		# no adoption of presets

		c = IECore.CompoundParameter(
			"c",
			members = [
				IECore.IntParameter(
					"a",
					"description",
					1,
					presets = (
						( "one", 1 ),
						( "two", 2 ),
					),
					presetsOnly = True,
				),
				IECore.IntParameter(
					"b",
					"description",
					1,
					presets = (
						( "one", 1 ),
						( "two", 2 ),
					),
					presetsOnly = True,
				),
			],
			adoptChildPresets = False,
		)

		self.assertEqual( len( c.getPresets() ), 0 )
		self.assertEqual( c.presetsOnly, False )

		# no adoption of presets without use of keyword parameters

		c = IECore.CompoundParameter(
			"c",
			"description",
			[
				IECore.IntParameter(
					"a",
					"description",
					1,
					presets = (
						( "one", 1 ),
						( "two", 2 ),
					),
					presetsOnly = True,
				),
				IECore.IntParameter(
					"b",
					"description",
					1,
					presets = (
						( "one", 1 ),
						( "two", 2 ),
					),
					presetsOnly = True,
				),
			],
			IECore.CompoundObject( { "ud" : IECore.IntData( 10 ) } ),
			False,
		)

		self.assertEqual( len( c.getPresets() ), 0 )
		self.assertEqual( c.presetsOnly, False )
		self.assertEqual( c.userData()["ud"].value, 10 )

		# when adoptChildPresets we can also set presets explicitly...
		c['a'].setValue("one")
		c['b'].setValue("two")
		p1 = c.getValue().copy()
		c['a'].setValue("two")
		c['b'].setValue("one")
		p2 = c.getValue().copy()
		c.setValue( c.defaultValue )

		c.setPresets(
			[
				( "p1", p1 ),
				( "p2", p2 ),
			]
		)
		pr = c.getPresets()
		self.assertEqual( len( pr ), 2 )
		self.assertEqual( pr["p1"], p1 )
		self.assertEqual( pr["p2"], p2 )
		self.assertEqual( c.presetNames(), ( "p1", "p2" ) )
		c.setValue("p1")
		self.assertEqual( c.getValue(), p1 )
		c.setValue("p2")
		self.assertEqual( c.getValue(), p2 )

	def testDerivingInPython( self ) :

		class DerivedCompoundParameter( IECore.CompoundParameter ) :

			def __init__( self, name, description, userData = None ) :

				IECore.CompoundParameter.__init__( self, name, description, userData = userData )

		IECore.registerRunTimeTyped( DerivedCompoundParameter )

		c = IECore.CompoundParameter()
		c.addParameter( DerivedCompoundParameter( "d", "" ) )
		c["d"].addParameter( IECore.IntParameter( "i", "", 1 ) )

		self.assertEqual( c.parameterPath( c["d"]["i"] ), [ "d", "i" ] )

if __name__ == "__main__":
	unittest.main()
