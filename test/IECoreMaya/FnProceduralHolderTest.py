##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import os
import maya.cmds
import maya.OpenMaya

import IECore
import IECoreGL
import IECoreMaya

class FnProceduralHolderTest( IECoreMaya.TestCase ) :

	class SphereProcedural( IECore.ParameterisedProcedural ) :

		def __init__( self ) :

			IECore.ParameterisedProcedural.__init__( self, "" )

			self.parameters().addParameters(

				[
					IECore.FloatParameter(
						"radius",
						"",
						1.0,
					),

					IECore.V3fParameter(
						"translate",
						"",
						IECore.V3f( 0 ),
					),

					IECore.V3fParameter(
						"extraTranslate",
						"",
						IECore.V3f( 0 ),
					),
				]

			)

		def doBound( self, args ) :

			result = IECore.Box3f( IECore.V3f( -args["radius"].value ), IECore.V3f( args["radius"].value ) )
			return result

		def doRender( self, renderer, args ) :

			with IECore.AttributeBlock( renderer ) :

				g = IECore.Group()
				g.addState( IECore.AttributeState( { "name" : IECore.StringData( "mySphere" ) } ) )
				g.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( args["translate"].value ) ) )

				innerGroup = IECore.Group()
				innerGroup.setTransform( IECore.MatrixTransform( IECore.M44f.createTranslated( args["extraTranslate"].value ) ) )
				innerGroup.addChild( IECore.SpherePrimitive( args["radius"].value, -1, 1, 360 ) )

				g.addChild( innerGroup )

				g.render( renderer )

		def doRenderState( self, renderer, args ) :

			pass

	def testScene( self ) :

		node = maya.cmds.createNode( "ieProceduralHolder" )

		fnPH = IECoreMaya.FnProceduralHolder( node )
		fnPH.setParameterised( self.SphereProcedural() )

		radiusAttr = fnPH.parameterPlugPath( fnPH.getProcedural()["radius"] )

		prevScene = None
		for i in range( 0, 10000 ) :

			maya.cmds.setAttr( radiusAttr, i + 1 )
			scene = fnPH.scene()
			self.failUnless( isinstance( scene, IECoreGL.Scene ) )
			self.failIf( prevScene is not None and scene.isSame( prevScene ) )
			prevScene = scene

	def testCreationName( self ) :

		fnPH = IECoreMaya.FnProceduralHolder.create( "bob", "read", 1 )
		self.assertEqual( fnPH.fullPathName(), u"|bob|bobShape" )

		fnPH = IECoreMaya.FnProceduralHolder.create( "bob1", "read", 1 )
		self.assertEqual( fnPH.fullPathName(), u"|bob1|bobShape1" )

		fnPH = IECoreMaya.FnProceduralHolder.create( "bob", "read", 1 )
		self.assertEqual( fnPH.fullPathName(), u"|bob2|bobShape2" )

	def testCreationWithoutVersion( self ) :

		fnPH = IECoreMaya.FnProceduralHolder.create( "bob", "read" )
		self.assertEqual( fnPH.getParameterised()[2], 1 )

		fnPH = IECoreMaya.FnProceduralHolder.create( "bob", "read", None )
		self.assertEqual( fnPH.getParameterised()[2], 1 )

		fnPH = IECoreMaya.FnProceduralHolder.create( "bob", "read", -1 )
		self.assertEqual( fnPH.getParameterised()[2], 1 )

	def testMelCreation( self ) :

		import maya.mel as mel
		mel.eval( 'ieProceduralHolderCreate( "read", "read", "1" )' )

		self.failUnless( maya.cmds.objExists( "readShape" ) )

	def testComponentNames( self ) :

		fnPH = IECoreMaya.FnProceduralHolder.create( "ernie", "read" )
		cobPath = os.getcwd() + "/test/IECore/data/cobFiles/pSphereShape1.cob"
		fnPH.getParameterised()[0].parameters()["files"]["name"].setTypedValue( cobPath )
		fnPH.setNodeValues()

		self.assertEqual( fnPH.componentNames(), set( [ u'unnamed' ] ) )

	def testComponentTransforms( self ) :

		node = maya.cmds.createNode( "ieProceduralHolder" )

		fnPH = IECoreMaya.FnProceduralHolder( node )
		fnPH.setParameterised( self.SphereProcedural() )

		transformPlug = fnPH.componentTransformPlugPath( "mySphere" )
		self.failUnless( maya.cmds.objExists( transformPlug ) )

		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentTranslate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentRotate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentScale" ), [ ( 1, 1, 1 ) ] )

		boundPlug = fnPH.componentBoundPlugPath( "mySphere" )
		self.failUnless( maya.cmds.objExists( boundPlug ) )

		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMin" ), [ ( -1, -1, -1 ) ] )
		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMax" ), [ ( 1, 1, 1 ) ] )
		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundCenter" ), [ ( 0, 0, 0 ) ] )

		procedural = fnPH.getProcedural()
		procedural["translate"].setValue( IECore.V3f( 2, 0, 0 ) )
		procedural["radius"].setNumericValue( 2 )
		fnPH.setNodeValues()

		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentTranslate" ), [ ( 2, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentRotate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentScale" ), [ ( 1, 1, 1 ) ] )

		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMin" ), [ ( 0, -2, -2 ) ] )
		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMax" ), [ ( 4, 2, 2 ) ] )
		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundCenter" ), [ ( 2, 0, 0 ) ] )

	def testMoreComplicatedComponentTransforms( self ) :

		node = maya.cmds.createNode( "ieProceduralHolder" )

		fnPH = IECoreMaya.FnProceduralHolder( node )
		fnPH.setParameterised( self.SphereProcedural() )

		transformPlug = fnPH.componentTransformPlugPath( "mySphere" )
		self.failUnless( maya.cmds.objExists( transformPlug ) )

		boundPlug = fnPH.componentBoundPlugPath( "mySphere" )
		self.failUnless( maya.cmds.objExists( boundPlug ) )

		procedural = fnPH.getProcedural()
		procedural["translate"].setValue( IECore.V3f( 2, 0, 0 ) )
		procedural["radius"].setNumericValue( 2 )
		procedural["extraTranslate"].setValue( IECore.V3f( 0, 2, 0 ) )
		fnPH.setNodeValues()

		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentTranslate" ), [ ( 2, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentRotate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( transformPlug + ".componentScale" ), [ ( 1, 1, 1 ) ] )

		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMin" ), [ ( 0, 0, -2 ) ] )
		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMax" ), [ ( 4, 4, 2 ) ] )
		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundCenter" ), [ ( 2, 2, 0 ) ] )

	def testComponentBoundMinMax( self ) :

		node = maya.cmds.createNode( "ieProceduralHolder" )

		fnPH = IECoreMaya.FnProceduralHolder( node )
		fnPH.setParameterised( self.SphereProcedural() )

		boundPlug = fnPH.componentBoundPlugPath( "mySphere" )
		self.failUnless( maya.cmds.objExists( boundPlug ) )

		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMin" ), [ ( -1, -1, -1 ) ] )
		self.assertEqual( maya.cmds.getAttr( boundPlug + ".componentBoundMax" ), [ ( 1, 1, 1 ) ] )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
