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
import imath

import IECore
import IECoreScene
import IECoreMaya

class ToMayaParticleConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 10 ) ] ) )
		converter = IECoreMaya.ToMayaObjectConverter.create( points )
		self.assertTrue( isinstance( converter, IECoreMaya.ToMayaParticleConverter ) )

	def testConversion( self ) :

		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 10 ) ] ) )

		parent = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaParticleConverter( points ).convert( parent )

		children = maya.cmds.listRelatives( parent )
		self.assertEqual( len( children ), 1 )

		particleShape = children[0]
		self.assertEqual( maya.cmds.nodeType( particleShape ), "particle" )

		self.assertEqual( maya.cmds.particle( particleShape, query=True, count=True ), 10 )

		for i in range( 0, 10 ) :
			self.assertEqual( maya.cmds.particle( attribute="position", q=True, order=i ), [ i, i, i ] )

		self.assertFalse( "P" in maya.cmds.particle( particleShape, query=True, perParticleVector=True ) )

	def testConversionFromDoubles( self ) :

		points = IECoreScene.PointsPrimitive( 10 )
		points["P"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3dVectorData( [ imath.V3d( x ) for x in range( 0, 10 ) ] ) )
		points["rgbPP"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3dVectorData( [ imath.V3d( x ) for x in range( 10, 20 ) ] ) )

		parent = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaParticleConverter( points ).convert( parent )

		children = maya.cmds.listRelatives( parent )
		self.assertEqual( len( children ), 1 )

		particleShape = children[0]
		self.assertEqual( maya.cmds.nodeType( particleShape ), "particle" )

		self.assertEqual( maya.cmds.particle( particleShape, query=True, count=True ), 10 )

		for i in range( 0, 10 ) :
			self.assertEqual( maya.cmds.particle( attribute="position", q=True, order=i ), [ i, i, i ] )

		self.assertFalse( "P" in maya.cmds.particle( particleShape, query=True, perParticleVector=True ) )

	def testRGBPPConversion( self ) :

		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 10 ) ] ) )
		points["rgbPP"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.Color3fVectorData( [ imath.Color3f( x ) for x in range( 10, 20 ) ] ) )
		parent = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaParticleConverter( points ).convert( parent )

		children = maya.cmds.listRelatives( parent )
		self.assertEqual( len( children ), 1 )

		particleShape = children[0]
		self.assertEqual( maya.cmds.nodeType( particleShape ), "particle" )

		self.assertEqual( maya.cmds.particle( particleShape, query=True, count=True ), 10 )

		for i in range( 0, 10 ) :
			self.assertEqual( maya.cmds.particle( attribute="rgbPP", q=True, order=i ), [ i+10, i+10, i+10 ] )

	def testCsConversion( self ) :

		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( x ) for x in range( 0, 10 ) ] ) )
		points["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.Color3fVectorData( [ imath.Color3f( x ) for x in range( 10, 20 ) ] ) )
		parent = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaParticleConverter( points ).convert( parent )

		children = maya.cmds.listRelatives( parent )
		self.assertEqual( len( children ), 1 )

		particleShape = children[0]
		self.assertEqual( maya.cmds.nodeType( particleShape ), "particle" )

		self.assertEqual( maya.cmds.particle( particleShape, query=True, count=True ), 10 )

		for i in range( 0, 10 ) :
			self.assertEqual( maya.cmds.particle( attribute="rgbPP", q=True, order=i ), [ i+10, i+10, i+10 ] )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
