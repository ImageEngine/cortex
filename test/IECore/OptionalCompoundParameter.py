##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

	
class TestOptionalCompoundParameter( unittest.TestCase ) :

	def testAttributeAccess( self ):

		p = OptionalCompoundParameter( "n", "d", members = [ StringParameter( "a", "a", "", presets = ( ( "b", StringData( "b" ) ), ), presetsOnly = True ) ] )
		p["a"].setTypedValue( "My compound attribute" )
		p.c = "My python attribute"
		self.assertEqual( p.c, "My python attribute" )
		self.assertEqual( p["a"].getTypedValue(), "My compound attribute" )
		self.assertEqual( p.userData(), CompoundObject() )

	def testConstructor( self ) :
	
		p = OptionalCompoundParameter( "n", "d", members = [ StringParameter( "a", "a", "", presets = ( ( "b", StringData( "b" ) ), ), presetsOnly = True ) ] )
		self.assertEqual( p.name, "n" )
		self.assertEqual( p.description, "d" )
		self.assertEqual( len( p.keys() ), 1 )
		self.assertEqual( len( p.values() ), 1 )
		self.assertEqual( len( p ), 1 )
		self.assertEqual (p.userData(), CompoundObject() )
	
	def testLateValidation( self ) :
	
		p = OptionalCompoundParameter(
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
		self.assertEqual( p["i"].getValue(),  IntData( 10 ) )
		self.assertEqual( p["f"].getValue(),  FloatData( 20 ) )
		
		p.setValue( CompoundObject( { "i" : IntData( 10 ) } ) )
		self.assertRaises( RuntimeError, p.validate )
		self.assertRaises( RuntimeError, p.getValidatedValue )
		p["f"].setValue( FloatData( 20 ) )
		p.validate()

		p.setValue( CompoundObject( { "idontbelong" : IntData( 10 ), "i" : IntData( 10 ), "f" : FloatData( 20 ) } ) )
		self.assertRaises( RuntimeError, p.validate )
		self.assertRaises( RuntimeError, p.getValidatedValue )
		del p.getValue()["idontbelong"]
		p.validate()

	def testOptionalValidation( self ):

		p = OptionalCompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 1, presets = ( ( "10", 10 ), ( "11", 11 ) ), presetsOnly = True ),
				FloatParameter( "f", "d", 2 )
			]
		)
		self.assertRaises( RuntimeError, p.validate )
		p.setObligatoryParameterNames( [ 'f' ] )
		self.assertRaises( RuntimeError, p.validate )
		p.setParameterUndefined( 'i' )
		self.assert_( p.getParameterUndefined( 'i' ) )
		self.assert_( not p.getParameterUndefined( 'f' ) )
		p.validate()
		p.setParameterUndefined( 'f' )
		self.assertRaises( RuntimeError, p.validate )
		p.setObligatoryParameterNames( [ ] )
		p.validate()
		self.assertEqual( len( p.getObligatoryParameterNames() ), 0 )

	def testSmartSetValue( self ):
		"""Test python overwriting: smartSetValue()"""
		p = OptionalCompoundParameter(
			name = "c",
			description = "d",
			members = [
				IntParameter( "i", "d", 1 ),
				FloatParameter( "f", "d", 2 )
			],
			userData = CompoundObject( { "a": BoolData( False ) } )
		)

		q = OptionalCompoundParameter(
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
		p = OptionalCompoundParameter(
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
				
									
if __name__ == "__main__":
        unittest.main()
