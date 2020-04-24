##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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


class ToMayaPlugConverterTest( IECoreMaya.TestCase ) :

	def testSinglePlugConversion( self ) :

		locator = maya.cmds.spaceLocator()[0]

		converter = IECoreMaya.ToMayaPlugConverter.create( IECore.DoubleData( 10 ) )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaPlugConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECore.FromCoreConverter.staticTypeId() ) )

		converter.convert( locator + ".translateX" )
		self.assertAlmostEqual( maya.cmds.getAttr( locator + ".translateX" ), 10 )

	def testUnitPlugConversion(self):

		timeNode = maya.cmds.createNode('time')
		maya.cmds.currentUnit( time='film' )  # set to 24fps
		converter = IECoreMaya.ToMayaPlugConverter.create( IECore.DoubleData( 1.23 ) )
		converter.convert(timeNode + '.outTime')
		self.assertAlmostEqual(maya.cmds.getAttr(timeNode + '.outTime'), 1.23 * 24.0)

		locator = maya.cmds.spaceLocator()[0]
		converter = IECoreMaya.ToMayaPlugConverter.create( IECore.DoubleData( IECore.degreesToRadians(90.0) ) )
		converter.convert(locator + '.rotateX' )
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.rotateX'), 90.0)

		converter = IECoreMaya.ToMayaPlugConverter.create( IECore.DoubleData( 1.23456) )
		converter.convert(locator + '.translateX' )
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.translateX'), 1.23456)

	def testNumericPlugConversion(self):

		locator = maya.cmds.spaceLocator()[0]

		attr = ('tInt', 'long', IECore.IntData(7))
		maya.cmds.addAttr(ln=attr[0], at=attr[1])
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.' + attr[0]), attr[2].value)

		attr = ('tShort', 'short', IECore.ShortData(8))
		maya.cmds.addAttr(ln=attr[0], at=attr[1])
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.' + attr[0]), attr[2].value)

		attr = ('tByte', 'byte', IECore.ShortData(9))
		maya.cmds.addAttr(ln=attr[0], at=attr[1])
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.' + attr[0]), attr[2].value)

		attr = ('tBool', 'bool', IECore.BoolData(True))
		maya.cmds.addAttr(ln=attr[0], at=attr[1])
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.' + attr[0]), attr[2].value)

		attr = ('tFloat', 'float', IECore.FloatData(1.23456))
		maya.cmds.addAttr(ln=attr[0], at=attr[1])
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.' + attr[0]), attr[2].value)

		attr = ('tDouble', 'double', IECore.DoubleData(2.3456))
		maya.cmds.addAttr(ln=attr[0], at=attr[1])
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.' + attr[0]), attr[2].value)

		attr = ('tChar', 'char', IECore.CharData('A'))
		maya.cmds.addAttr(ln=attr[0], at=attr[1])
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertEqual(maya.cmds.getAttr(locator + '.' + attr[0]), ord(str(attr[2].value)))

		attr = ('tEnum', 'enum', IECore.ShortData(2))
		maya.cmds.addAttr(ln=attr[0], at=attr[1], en="A:B:C:D:E")
		converter = IECoreMaya.ToMayaPlugConverter.create(attr[2])
		converter.convert(locator + '.' + attr[0])
		self.assertAlmostEqual(maya.cmds.getAttr(locator + '.' + attr[0]), attr[2].value)

	def testMultiPlugConversion(self):

		maya.cmds.polyPlane()
		bs = maya.cmds.blendShape()[0]
		multi = bs + '.inputTarget[0].baseWeights'

		data = IECore.CompoundData({'data' : IECore.FloatVectorData( [8, 9]), 'indices' : IECore.IntVectorData( [3, 5])})
		converter = IECoreMaya.ToMayaPlugConverter.create(data)
		converter.convert(multi)
		self.assertEqual(maya.cmds.getAttr(multi + '[3]'), 8)
		self.assertEqual(maya.cmds.getAttr(multi + '[5]'), 9)


if __name__ == "__main__":
	IECoreMaya.TestProgram()
