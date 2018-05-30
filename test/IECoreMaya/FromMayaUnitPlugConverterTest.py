##########################################################################
#
#  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

import maya.cmds

import IECore
import IECoreMaya


class FromMayaUnitPlugConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		locator = maya.cmds.spaceLocator()[0]

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateX" )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaUnitPlugConverterd.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateX", IECore.DoubleData.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaUnitPlugConverterd.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateX", IECore.FloatData.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaUnitPlugConverterf.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateX", IECore.Data.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaUnitPlugConverterd.staticTypeId() ) )

	def testDistance( self ) :

		locator = maya.cmds.spaceLocator()[0]
		maya.cmds.move( 1, 2, 3, locator )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateX" )
		v = converter.convert()
		self.assert_( v.isInstanceOf( IECore.DoubleData.staticTypeId() ) )
		self.assertEqual( v.value, 1 )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateY", IECore.FloatData.staticTypeId() )
		v = converter.convert()
		self.assert_( v.isInstanceOf( IECore.FloatData.staticTypeId() ) )
		self.assertEqual( v.value, 2 )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateZ" )
		self.assertEqual( converter["distanceUnit"].getCurrentPresetName(), "Centimeters" )
		converter["distanceUnit"].setValue( "Meters" )
		v = converter.convert()
		self.assert_( v.isInstanceOf( IECore.DoubleData.staticTypeId() ) )
		self.assertEqual( v.value, 0.03 )

	def testAngle( self ) :

		locator = maya.cmds.spaceLocator()[0]
		maya.cmds.setAttr(locator + '.rotateX', 90.0 )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".rotateX" )
		self.assertEqual( converter["angleUnit"].getCurrentPresetName(), "Radians" )
		v = converter.convert()
		self.assert_( v.isInstanceOf( IECore.DoubleData.staticTypeId() ) )
		self.assertAlmostEqual( v.value, IECore.degreesToRadians(90.0) )

		converter["angleUnit"].setValue( "Degrees" )
		v = converter.convert()
		self.assert_( v.isInstanceOf( IECore.DoubleData.staticTypeId() ) )
		self.assertAlmostEqual( v.value, 90.0 )

	def testTime( self ) :
		timeNode = maya.cmds.createNode('time')
		converter = IECoreMaya.FromMayaPlugConverter.create( str( timeNode ) + ".outTime" )

		maya.cmds.currentUnit( time='film' )  # set to 24fps
		maya.cmds.setAttr(timeNode + '.outTime', 1.23 )
		time = maya.cmds.getAttr(timeNode + '.outTime')
		self.assertAlmostEqual(converter.convert().value * 24.0, time)

		maya.cmds.currentUnit( time='show' )  # set to 48fps
		maya.cmds.setAttr(timeNode + '.outTime', 1.23 )
		time = maya.cmds.getAttr(timeNode + '.outTime')
		converter["timeUnit"].setValue( "Hours" )
		self.assertAlmostEqual(converter.convert().value * 48.0, time / (60**2) )

	def testTypeIds( self ) :

		locator = maya.cmds.spaceLocator()[0]
		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".translateX" )
		self.assertEqual( converter.typeId(), IECoreMaya.TypeId.FromMayaUnitPlugConverterd )
		self.assertEqual( converter.typeName(), "FromMayaUnitPlugConverterd" )



if __name__ == "__main__":
	IECoreMaya.TestProgram()
