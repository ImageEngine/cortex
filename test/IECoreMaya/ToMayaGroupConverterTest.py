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

import maya.cmds

import IECore
import IECoreScene
import IECoreMaya

class ToMayaGroupConverterTest( IECoreMaya.TestCase ) :

	def testConversion( self ) :

		g = IECoreScene.Group()
		g.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createScaled( IECore.V3f( 2 ) ) ) )

		c1 = IECoreScene.Group()
		c1.addChild( IECoreScene.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) ) ) )
		c1.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( 2, 0, 0 ) ) ) )
		g.addChild( c1 )

		c2 = IECoreScene.Group()
		c2.addChild( IECoreScene.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) ) ) )
		c2.setTransform( IECoreScene.MatrixTransform( IECore.M44f.createTranslated( IECore.V3f( -2, 0, 0 ) ) ) )
		g.addChild( c2 )

		p = maya.cmds.createNode( "transform" )

		IECoreMaya.ToMayaGroupConverter( g ).convert( p )

		mg = maya.cmds.listRelatives( p, fullPath=True )

		self.assertEqual( len( mg ), 1 )
		self.assertEqual( maya.cmds.nodeType( mg[0] ), "transform" )
		self.assertEqual( maya.cmds.getAttr( mg[0] + ".translate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( mg[0] + ".rotate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( mg[0] + ".scale" ), [ ( 2, 2, 2 ) ] )

		mgg = maya.cmds.listRelatives( mg[0], fullPath=True )

		self.assertEqual( len( mgg ), 2 )

		self.assertEqual( maya.cmds.nodeType( mgg[0] ), "transform" )
		self.assertEqual( maya.cmds.getAttr( mgg[0] + ".translate" ), [ ( 2, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( mgg[0] + ".rotate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( mgg[0] + ".scale" ), [ ( 1, 1, 1 ) ] )

		self.assertEqual( maya.cmds.nodeType( mgg[1] ), "transform" )
		self.assertEqual( maya.cmds.getAttr( mgg[1] + ".translate" ), [ ( -2, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( mgg[1] + ".rotate" ), [ ( 0, 0, 0 ) ] )
		self.assertEqual( maya.cmds.getAttr( mgg[1] + ".scale" ), [ ( 1, 1, 1 ) ] )

		m1 = maya.cmds.listRelatives( mgg[0], fullPath=True )
		self.assertEqual( len( m1 ), 1 )
		self.assertEqual( maya.cmds.nodeType( m1[0] ), "mesh" )
		self.assertEqual( maya.cmds.polyEvaluate( m1[0], face=True ), 6 )

		m2 = maya.cmds.listRelatives( mgg[1], fullPath=True )
		self.assertEqual( len( m2 ), 1 )
		self.assertEqual( maya.cmds.nodeType( m2[0] ), "mesh" )
		self.assertEqual( maya.cmds.polyEvaluate( m2[0], face=True ), 6 )

	def testNamedConversion( self ):

		g = IECoreScene.Group()
		g.addState( IECoreScene.AttributeState( { "name" : IECore.StringData( "topLevel" ) } ) )

		c1 = IECoreScene.Group()
		c1.addState( IECoreScene.AttributeState( { "name" : IECore.StringData( "topLevel/child1" ) } ) )
		g.addChild( c1 )

		c2 = IECoreScene.Group()
		c2.addState( IECoreScene.AttributeState( { "name" : IECore.StringData( "child2" ) } ) )
		g.addChild( c2 )

		c3 = IECoreScene.Group()
		c3.addState( IECoreScene.AttributeState( { "name" : IECore.StringData( "topLevel/child1/child3" ) } ) )
		c1.addChild( c3 )

		# nameless group
		g.addChild( IECoreScene.Group() )

		p = maya.cmds.createNode( "transform" )

		IECoreMaya.ToMayaGroupConverter( g ).convert( p )

		mg = maya.cmds.listRelatives( p, fullPath=True, ad=True )

		expectedNames = set( [ "|transform1|topLevel|child1|child3", "|transform1|topLevel|child1", "|transform1|topLevel|child2", "|transform1|topLevel|transform2", "|transform1|topLevel" ] )

		actualNames = set()
		for name in mg:
			actualNames.add( str( name ) )

		self.assertEqual( expectedNames, actualNames )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
