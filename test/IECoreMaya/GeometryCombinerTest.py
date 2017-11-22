##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

import os
import maya.cmds

import IECore
import IECoreScene
import IECoreMaya

class GeometryCombinerTest( IECoreMaya.TestCase ) :

	def test( self ) :

		combiner = maya.cmds.createNode( "ieGeometryCombiner" )

		self.assertEqual( maya.cmds.getAttr( combiner + ".convertPrimVars" ), 0 )
		self.assertEqual( maya.cmds.getAttr( combiner + ".convertBlindData" ), 0 )

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		circle = maya.cmds.circle()
		circle = maya.cmds.listRelatives( circle, shapes=True )[0]

		maya.cmds.connectAttr( sphere + ".worldMesh", combiner + ".inputGeometry", nextAvailable=True )
		maya.cmds.connectAttr( circle + ".worldSpace", combiner + ".inputGeometry", nextAvailable=True )

		combined = IECoreMaya.FromMayaPlugConverter.create( combiner + ".outputGroup" ).convert()

		self.failUnless( isinstance( combined, IECoreScene.Group ) )
		self.assertEqual( len( combined.children() ), 2 )

		self.failUnless( isinstance( combined.children()[0], IECoreScene.MeshPrimitive ) )
		self.failUnless( isinstance( combined.children()[1], IECoreScene.CurvesPrimitive ) )

	def testPrimVars( self ) :

		combiner = maya.cmds.createNode( "ieGeometryCombiner" )

		self.assertEqual( maya.cmds.getAttr( combiner + ".convertPrimVars" ), 0 )

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]
		maya.cmds.addAttr( sphere, attributeType="float", longName="delightTest", defaultValue=1 )

		maya.cmds.connectAttr( sphere + ".worldMesh", combiner + ".inputGeometry", nextAvailable=True )

		combined = IECoreMaya.FromMayaPlugConverter.create( combiner + ".outputGroup" ).convert()

		self.failIf( "Test" in combined.children()[0] )

		maya.cmds.setAttr( combiner + ".convertPrimVars", 1 )

		combined = IECoreMaya.FromMayaPlugConverter.create( combiner + ".outputGroup" ).convert()
		primVar = combined.children()[0]["Test"]

		self.assertEqual( primVar.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( primVar.data, IECore.FloatData( 1 ) )

	def testBlindData( self ) :

		combiner = maya.cmds.createNode( "ieGeometryCombiner" )

		self.assertEqual( maya.cmds.getAttr( combiner + ".convertBlindData" ), 0 )

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]
		maya.cmds.addAttr( sphere, dataType="string", longName="ieString" )
		maya.cmds.setAttr( sphere + ".ieString", "banana", type="string" )

		maya.cmds.connectAttr( sphere + ".worldMesh", combiner + ".inputGeometry", nextAvailable=True )

		combined = IECoreMaya.FromMayaPlugConverter.create( combiner + ".outputGroup" ).convert()

		self.failIf( "ieString" in combined.children()[0].blindData() )

		maya.cmds.setAttr( combiner + ".convertBlindData", 1 )
		maya.cmds.setAttr( combiner + ".blindDataAttrPrefix", "ie", type="string" )

		combined = IECoreMaya.FromMayaPlugConverter.create( combiner + ".outputGroup" ).convert()

		self.assertEqual( combined.children()[0].blindData()["ieString"].value, "banana" )

	def testSpaces( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		maya.cmds.move( 1, 2, 3, sphere )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		combiner = maya.cmds.createNode( "ieGeometryCombiner" )
		maya.cmds.connectAttr( sphere + ".worldMesh", combiner + ".inputGeometry", nextAvailable=True )

		self.assertEqual( maya.cmds.getAttr( combiner + ".conversionSpace" ), IECoreMaya.FromMayaShapeConverter.Space.World.real )

		combined = IECoreMaya.FromMayaPlugConverter.create( combiner + ".outputGroup" ).convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ) + IECore.V3f( 1, 2, 3 ), IECore.V3f( 1.0001 ) + IECore.V3f( 1, 2, 3 ) ).contains( combined.bound() ) )

		maya.cmds.setAttr( combiner + ".conversionSpace", IECoreMaya.FromMayaShapeConverter.Space.Object.real )

		combined = IECoreMaya.FromMayaPlugConverter.create( combiner + ".outputGroup" ).convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ), IECore.V3f( 1.0001 ) ).contains( combined.bound() ) )


if __name__ == "__main__":
	IECoreMaya.TestProgram()
