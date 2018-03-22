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

import IECore
import IECoreScene
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

	def makeRotationOrderScene( self ):

		maya.cmds.polyCube()

		maya.cmds.particle( p = [[0, 0, 0]], c = 1 )
		maya.cmds.addAttr( "particleShape1", ln = "rotationPP", dt = "vectorArray" )
		maya.cmds.addAttr( "particleShape1", ln = "instancePP", dt = "doubleArray" )
		maya.cmds.select( ["particle1", "pCube1" ], r = True )
		maya.cmds.particleInstancer( addObject = True, object = ["pCube1"] )
		maya.cmds.particleInstancer( "particleShape1", e = True, name = "instancer1", rotation = "rotationPP" )
		maya.cmds.particleInstancer( "particleShape1", e = True, name = "instancer1", objectIndex = "instancePP" )

		maya.cmds.setAttr("instancer1.rotationOrder", 5) #ZYX

		n = pm.PyNode( "particleShape1" )
		n.attr( "rotationPP" ).set( [pm.dt.Vector( 90, 90, 0 )] )
		n.attr( "instancePP" ).set( [0 ] )

	def testCanCreateConverterOfCorrectType( self ) :

		self.makeScene()
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "instancer1" )
		assert (converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaInstancerConverter ) ))

	def testConvertsToPointsPrimitive( self ) :

		self.makeScene()
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "instancer1" )
		convertedPoints = converter.convert()

		self.assertTrue( convertedPoints.isInstanceOf( IECoreScene.TypeId.PointsPrimitive ) )
		self.assertEqual( convertedPoints.numPoints, 4 )

		self.assertEqual( convertedPoints.keys(), ['P', 'age', 'id', 'instances', 'instanceType', 'orient'] )

		self.assertEqual( convertedPoints["P"].data[0], imath.V3f( 4, 0, 0 ) )
		self.assertEqual( convertedPoints["P"].data[1], imath.V3f( 4, 4, 0 ) )
		self.assertEqual( convertedPoints["P"].data[2], imath.V3f( 0, 4, 0 ) )
		self.assertEqual( convertedPoints["P"].data[3], imath.V3f( 0, 0, 0 ) )

		self.assertEqual( convertedPoints["id"].data[0], 0.0 )
		self.assertEqual( convertedPoints["id"].data[1], 1.0 )
		self.assertEqual( convertedPoints["id"].data[2], 2.0 )
		self.assertEqual( convertedPoints["id"].data[3], 3.0 )

		self.assertEqual( convertedPoints["age"].data[0], 0.0 )
		self.assertEqual( convertedPoints["age"].data[1], 0.0 )
		self.assertEqual( convertedPoints["age"].data[2], 0.0 )
		self.assertEqual( convertedPoints["age"].data[3], 0.0 )

		# instance indices to ensure we can instance the correct object
		self.assertEqual( convertedPoints["instanceType"].data[0], 0.0 )
		self.assertEqual( convertedPoints["instanceType"].data[1], 1.0 )
		self.assertEqual( convertedPoints["instanceType"].data[2], 0.0 )
		self.assertEqual( convertedPoints["instanceType"].data[3], 1.0 )

		# rotation is converted to orient
		self.assertEqual( convertedPoints["orient"].data[0], imath.Eulerf( 45, 0, 0 ).toQuat() )
		self.assertEqual( convertedPoints["orient"].data[1], imath.Eulerf( 0, 45, 0 ).toQuat() )
		self.assertEqual( convertedPoints["orient"].data[2], imath.Eulerf( 0, 0, 45 ).toQuat() )
		self.assertEqual( convertedPoints["orient"].data[3], imath.Eulerf( 45, 45, 0 ).toQuat() )

		# check we're capturing the locations in maya we're instancing
		self.assertEqual( convertedPoints["instances"].data, IECore.StringVectorData( ['/pCube1', '/pSphere1'] ) )


	def testCanChangeInstancerRotationOrder( self ):
		self.makeRotationOrderScene()

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "instancer1" )
		convertedPoints = converter.convert()

		self.assertTrue( convertedPoints.isInstanceOf( IECoreScene.TypeId.PointsPrimitive ) )
		self.assertEqual( convertedPoints.numPoints, 1 )

		self.assertEqual( convertedPoints.keys(), ['P', 'age', 'id', 'instances', 'instanceType', 'orient'] )

		self.assertEqual( convertedPoints["orient"].data[0], imath.Eulerf( 90, 90, 0, imath.Eulerf.ZYX ).toQuat() )



if __name__ == "__main__" :
	IECoreMaya.TestProgram( plugins = ["ieCore"] )
