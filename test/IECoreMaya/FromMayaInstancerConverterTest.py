##########################################################################
#
#  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
import maya.OpenMaya
import pymel.core as pm

import imath
import math

import IECore
import IECoreMaya


class FromMayaInstancerConverter( IECoreMaya.TestCase ) :

	def setUp( self ) :

		super( FromMayaInstancerConverter, self ).setUp()

	def makeScene( self ):

		maya.cmds.polyCube()
		maya.cmds.polySphere()

		maya.cmds.particle( p = [[4, 0, 0], [4, 4, 0], [0, 4, 0], [0, 0, 0]], c = 1 )
		maya.cmds.addAttr( "particleShape1", ln = "rotationPP", dt = "vectorArray" )
		maya.cmds.addAttr( "particleShape1", ln = "instancePP", dt = "doubleArray" )
		maya.cmds.select( ["particle1", "pCube1", "pSphere1"], r = True )
		maya.cmds.particleInstancer( addObject = True, object = ["pCube1","pSphere1"] )
		maya.cmds.particleInstancer( "particleShape1", e = True, name = "instancer1", rotation = "rotationPP" )
		maya.cmds.particleInstancer( "particleShape1", e = True, name = "instancer1", objectIndex = "instancePP" )

		n = pm.PyNode( "particleShape1" )
		n.attr( "rotationPP" ).set( [pm.dt.Vector( 45, 0, 0 ), pm.dt.Vector( 0, 45, 0 ), pm.dt.Vector( 0, 0, 45 ), pm.dt.Vector( 45, 45, 0 )] )
		n.attr( "instancePP" ).set( [0, 1, 0, 1] )

	def makeRotationOrderOrUnitScene( self, rotationOrder, useRadians ) :

		maya.cmds.polyCube()

		maya.cmds.particle( p = [[0, 0, 0]], c = 1 )
		maya.cmds.addAttr( "particleShape1", ln = "rotationPP", dt = "vectorArray" )
		maya.cmds.addAttr( "particleShape1", ln = "instancePP", dt = "doubleArray" )
		maya.cmds.select( ["particle1", "pCube1" ], r = True )
		maya.cmds.particleInstancer( addObject = True, object = ["pCube1"] )
		maya.cmds.particleInstancer( "particleShape1", e = True, name = "instancer1", rotation = "rotationPP" )
		maya.cmds.particleInstancer( "particleShape1", e = True, name = "instancer1", objectIndex = "instancePP" )

		maya.cmds.setAttr( "instancer1.rotationOrder", rotationOrder )  # ZYX
		if useRadians :
			maya.cmds.setAttr( "instancer1.rotationAngleUnits", 1 )

		n = pm.PyNode( "particleShape1" )
		n.attr( "rotationPP" ).set( [pm.dt.Vector( 90, 90, 0 )] )
		n.attr( "instancePP" ).set( [0 ] )

	def testCanCreateConverterOfCorrectType( self ) :

		self.makeScene()
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "instancer1" )
		assert (converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaInstancerConverter ) ))

	def assertUnorderedEqual( self, a, b ) :
		self.assertEqual( len( a ), len( b ) )
		self.assertEqual( set( a ), set( b ) )

	def testConvertsToPointsPrimitive( self ) :

		self.makeScene()
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "instancer1" )
		convertedPoints = converter.convert()

		self.assertTrue( convertedPoints.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		self.assertEqual( convertedPoints.numPoints, 4 )

		self.assertUnorderedEqual( convertedPoints.keys(), ['P', 'age', 'id', 'instances', 'instanceType', 'orient'] )

		self.assertEqual( convertedPoints["P"].data[0], IECore.V3f( 4, 0, 0 ) )
		self.assertEqual( convertedPoints["P"].data[1], IECore.V3f( 4, 4, 0 ) )
		self.assertEqual( convertedPoints["P"].data[2], IECore.V3f( 0, 4, 0 ) )
		self.assertEqual( convertedPoints["P"].data[3], IECore.V3f( 0, 0, 0 ) )

		self.assertEqual( convertedPoints["id"].data[0], 0.0 )
		self.assertEqual( convertedPoints["id"].data[1], 1.0 )
		self.assertEqual( convertedPoints["id"].data[2], 2.0 )
		self.assertEqual( convertedPoints["id"].data[3], 3.0 )

		self.assertEqual( convertedPoints["age"].data[0], 0.0 )
		self.assertEqual( convertedPoints["age"].data[1], 0.0 )
		self.assertEqual( convertedPoints["age"].data[2], 0.0 )
		self.assertEqual( convertedPoints["age"].data[3], 0.0 )

		# instance indices to ensure we can instance the correct object
		self.assertEqual( convertedPoints["instanceType"].data[0], 0 )
		self.assertEqual( convertedPoints["instanceType"].data[1], 1 )
		self.assertEqual( convertedPoints["instanceType"].data[2], 0 )
		self.assertEqual( convertedPoints["instanceType"].data[3], 1 )

		# rotation is converted to orient
		self.assertEqual( convertedPoints["orient"].data[0], IECore.Eulerf( math.pi / 4.0, 0, 0 ).toQuat() )
		self.assertEqual( convertedPoints["orient"].data[1], IECore.Eulerf( 0, math.pi / 4.0, 0 ).toQuat() )
		self.assertEqual( convertedPoints["orient"].data[2], IECore.Eulerf( 0, 0, math.pi / 4.0 ).toQuat() )
		self.assertEqual( convertedPoints["orient"].data[3], IECore.Eulerf( math.pi / 4.0, math.pi / 4.0, 0 ).toQuat() )

		# check we're capturing the locations in maya we're instancing
		self.assertEqual( convertedPoints["instances"].data, IECore.StringVectorData( ['/pCube1', '/pSphere1'] ) )

	def assertEqualUnordered( self, a, b ) :
		self.assertEqual( len( a ), len( b ) )
		self.assertEqual( set( a ), set( b ) )

	def testCanChangeInstancerRotationOrder( self ):
		self.makeRotationOrderOrUnitScene( 5, False )

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "instancer1" )
		convertedPoints = converter.convert()

		self.assertTrue( convertedPoints.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		self.assertEqual( convertedPoints.numPoints, 1 )

		self.assertUnorderedEqual( convertedPoints.keys(), ['P', 'age', 'id', 'instances', 'instanceType', 'orient'] )

		self.assertEqual( convertedPoints["orient"].data[0], IECore.Eulerf( math.pi / 2.0, math.pi / 2.0, 0, IECore.Eulerf.Order.ZYX ).toQuat() )

	def testCanChangeInstancerRotationUnits( self ) :
		self.makeRotationOrderOrUnitScene( 0, True )

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "instancer1" )
		convertedPoints = converter.convert()

		self.assertTrue( convertedPoints.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		self.assertEqual( convertedPoints.numPoints, 1 )

		self.assertUnorderedEqual( convertedPoints.keys(), ['P', 'age', 'id', 'instances', 'instanceType', 'orient'] )

		self.assertEqual( convertedPoints["orient"].data[0], IECore.Eulerf( 90.0, 90.0, 0, IECore.Eulerf.Order.XYZ ).toQuat() )

if __name__ == "__main__" :
	IECoreMaya.TestProgram( plugins = ["ieCore"] )
