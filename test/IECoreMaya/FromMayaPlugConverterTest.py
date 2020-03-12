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


class FromMayaPlugConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		locator = maya.cmds.spaceLocator( position=( 1, 2, 3 ) )[0]

		converter = IECoreMaya.FromMayaPlugConverter.create( locator + ".translateX" )
		self.assertTrue( converter )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaPlugConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( locator + ".scaleX" )
		self.assertTrue( converter )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaPlugConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( locator + ".scale" )
		self.assertTrue( converter )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaPlugConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( IECoreMaya.plugFromString( locator + ".translateX" ) )
		self.assertTrue( converter )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaPlugConverter.staticTypeId() ) )

	def testNumericConverterFactory( self ) :

		locator = maya.cmds.spaceLocator( position=( 1, 2, 3 ) )[0]
		maya.cmds.addAttr(ln="testEnum", at="enum", en="A:B:C")

		converter = IECoreMaya.FromMayaPlugConverter.create( locator + ".scaleX" )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaNumericPlugConverterdd.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( locator + ".scaleX", IECore.Data.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaNumericPlugConverterdd.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( locator + ".scaleX", IECore.FloatData.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaNumericPlugConverterdf.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".testEnum", IECore.StringData.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaEnumPlugConverterst.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".testEnum", IECore.ShortData.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaEnumPlugConvertersh.staticTypeId() ) )

	def testTypedConverterFactory( self ) :

		texture = maya.cmds.createNode( "file" )

		converter = IECoreMaya.FromMayaPlugConverter.create( texture + ".fileTextureName" )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaStringPlugConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( texture + ".fileTextureName", IECore.StringData.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaStringPlugConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaPlugConverter.create( texture + ".fileTextureName", IECore.Data.staticTypeId() )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.FromMayaStringPlugConverter.staticTypeId() ) )

	def testTransformationMatrix( self ) :

		locator = maya.cmds.spaceLocator( position=( 1, 2, 3 ) )[0]

		converter = IECoreMaya.FromMayaPlugConverter.create( str( locator ) + ".worldMatrix", IECore.TransformationMatrixdData.staticTypeId() )
		self.assertTrue( converter )

		transform = converter.convert()
		self.assertTrue( transform.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )

	def testPointArrayData( self ) :

		import itertools
		import maya.OpenMaya as om

		data = [ [ 0.1, 0.2, 0.3, 1 ], [ 0.4, 0.5, 0.6, 1 ] ]

		locator = maya.cmds.spaceLocator()[0]
		maya.cmds.addAttr( locator, ln="myPoints", dt="pointArray" )
		maya.cmds.setAttr( locator + "." + "myPoints", 2, *data, type="pointArray" )

		sl = om.MSelectionList()
		sl.add( locator )
		o = om.MObject()
		sl.getDependNode( 0, o )
		fn = om.MFnDependencyNode( o )
		plug = fn.findPlug( "myPoints" )

		converter = IECoreMaya.FromMayaPlugConverter.create( plug )
		self.assertTrue( converter )

		converted = converter.convert()
		self.assertTrue( converted.isInstanceOf( IECore.V3dVectorData.staticTypeId() ) )

		for point, index in itertools.product( range( 2 ), range( 3 ) ):
			self.assertAlmostEqual( converted[ point ][ index ], data[ point ][ index ] )

		self.assertEqual( converted.getInterpretation(), IECore.GeometricData.Interpretation.Point )

	def testVectorArrayData( self ) :

		import itertools
		import maya.OpenMaya as om

		data = [ [ 0.1, 0.2, 0.3 ], [ 0.4, 0.5, 0.6 ] ]

		locator = maya.cmds.spaceLocator()[0]
		maya.cmds.addAttr( locator, ln="myVectors", dt="vectorArray" )
		maya.cmds.setAttr( locator + "." + "myVectors", 2, *data, type="vectorArray" )

		sl = om.MSelectionList()
		sl.add( locator )
		o = om.MObject()
		sl.getDependNode( 0, o )
		fn = om.MFnDependencyNode( o )
		plug = fn.findPlug( "myVectors" )

		converter = IECoreMaya.FromMayaPlugConverter.create( plug )
		self.assertTrue( converter )

		converted = converter.convert()
		self.assertTrue( converted.isInstanceOf( IECore.V3dVectorData.staticTypeId() ) )

		for point, index in itertools.product( range( 2 ), range( 3 ) ):
			self.assertAlmostEqual( converted[ point ][ index ], data[ point ][ index ] )

		self.assertEqual( converted.getInterpretation(), IECore.GeometricData.Interpretation.Vector )

	def testMultiPlug( self ):

		maya.cmds.polyPlane(sx=1, sy=1)
		bs = maya.cmds.blendShape()[0]
		multi = bs + '.inputTarget[0].baseWeights'
		maya.cmds.setAttr(multi + '[1]', 8)
		maya.cmds.setAttr(multi + '[3]', 9)

		converter = IECoreMaya.FromMayaPlugConverter.create(multi)
		self.assertTrue( converter )
		data = converter.convert()

		self.assertEqual(data['indices'][0], 1)
		self.assertEqual(data['indices'][1], 3)
		self.assertEqual(data['data'][0], 8)
		self.assertEqual(data['data'][1], 9)

	def testSingleNumericPlugs(self):

		locator = maya.cmds.spaceLocator()[0]

		attr = ("tBool", IECore.BoolData, True)
		maya.cmds.addAttr(ln=attr[0], at="bool")
		maya.cmds.setAttr(locator + '.' + attr[0], attr[2])
		converter = IECoreMaya.FromMayaPlugConverter.create( IECoreMaya.plugFromString(locator + '.' + attr[0]) )
		self.assertTrue(converter)
		cValue = converter.convert()
		self.assertTrue(isinstance(cValue, attr[1]))
		self.assertAlmostEqual(cValue.value, attr[2])

		attr = ("tFloat", IECore.FloatData, 0.123456)
		maya.cmds.addAttr(ln=attr[0], at="float")
		maya.cmds.setAttr(locator + '.' + attr[0], attr[2])
		converter = IECoreMaya.FromMayaPlugConverter.create( IECoreMaya.plugFromString(locator + '.' + attr[0]) )
		self.assertTrue(converter)
		cValue = converter.convert()
		self.assertTrue(isinstance(cValue, attr[1]))
		self.assertAlmostEqual(cValue.value, attr[2])

		attr = ("tDouble", IECore.DoubleData, 0.123456)
		maya.cmds.addAttr(ln=attr[0], at="double")
		maya.cmds.setAttr(locator + '.' + attr[0], attr[2])
		converter = IECoreMaya.FromMayaPlugConverter.create( IECoreMaya.plugFromString(locator + '.' + attr[0]) )
		self.assertTrue(converter)
		cValue = converter.convert()
		self.assertTrue(isinstance(cValue, attr[1]))
		self.assertAlmostEqual(cValue.value, attr[2])

		attr = ("tShort", IECore.ShortData, 1)
		maya.cmds.addAttr(ln=attr[0], at="enum", en="A:B:C")
		maya.cmds.setAttr(locator + '.' + attr[0], attr[2])
		converter = IECoreMaya.FromMayaPlugConverter.create( IECoreMaya.plugFromString(locator + '.' + attr[0]) )
		self.assertTrue(converter)
		cValue = converter.convert()
		self.assertTrue(isinstance(cValue, attr[1]))
		self.assertAlmostEqual(cValue.value, attr[2])

		# read enum as string
		converter = IECoreMaya.FromMayaPlugConverter.create( IECoreMaya.plugFromString(locator + '.' + attr[0]), IECore.StringData.staticTypeId() )
		self.assertTrue(converter)
		cValue = converter.convert()
		self.assertTrue(isinstance(cValue, IECore.StringData))
		self.assertAlmostEqual(cValue.value, "B")

if __name__ == "__main__":
	IECoreMaya.TestProgram()
