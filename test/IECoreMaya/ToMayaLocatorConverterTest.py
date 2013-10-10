##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

class ToMayaLocatorConverterTest( IECoreMaya.TestCase ) :

	def setUp( self ) :

		maya.cmds.file( new=True, f=True )

	def testFactory( self ) :
	
		converter = IECoreMaya.ToMayaObjectConverter.create( IECore.CoordinateSystem() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.ToMayaLocatorConverter.staticTypeId() ) )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.ToMayaLocatorConverter ) ) )

	def testNewLocator( self ) :
		
		m = IECore.M44f.createScaled(IECore.V3f(1,2,3)) * IECore.M44f.createTranslated(IECore.V3f(10,20,30))
		coordSys = IECore.CoordinateSystem( "myNamedCoordSys", IECore.MatrixTransform( m ) )
		
		parent = maya.cmds.createNode( "transform" )
		self.failUnless( IECoreMaya.ToMayaLocatorConverter( coordSys ).convert( parent ) )
		
		locators = maya.cmds.listRelatives( parent, children=True, fullPath=True, type="locator" )
		self.assertEqual( len(locators), 1 )
		self.assertEqual( locators[0].split( "|" )[-1], "myNamedCoordSys" )
		self.assertEqual( maya.cmds.getAttr( locators[0] + ".localPosition" )[0], (10,20,30) )
		self.assertEqual( maya.cmds.getAttr( locators[0] + ".localScale" )[0], (1,2,3) )
		
		newCoordSys = IECoreMaya.FromMayaLocatorConverter( locators[0] ).convert()
		self.assertEqual( coordSys.getName(), newCoordSys.getName() )
		self.assertEqual( coordSys.getTransform(), newCoordSys.getTransform() )
	
	def testWrongIECoreObject( self ) :
		
		converter = IECoreMaya.ToMayaLocatorConverter( IECore.MeshPrimitive() )
		
		parent = maya.cmds.createNode( "transform" )
		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :
			self.assertFalse( converter.convert( parent ) )
		
		self.assertEqual( len( messageHandler.messages ), 1 )
		self.assertEqual( messageHandler.messages[0].level, IECore.MessageHandler.Level.Warning )
		self.assertEqual( messageHandler.messages[0].context, "ToMayaLocatorConverter::doConversion" )
	
	def testWrongMayaNode( self ) :
		
		maya.cmds.polySphere( name="pSphere" )
		coordSys = IECore.CoordinateSystem( "myNamedCoordSys", IECore.MatrixTransform( IECore.M44f() ) )
		converter = IECoreMaya.ToMayaLocatorConverter( coordSys )
		
		messageHandler = IECore.CapturingMessageHandler()
		with messageHandler :
			self.assertFalse( converter.convert( "pSphereShape" ) )
		
		self.assertEqual( len( messageHandler.messages ), 1 )
		self.assertEqual( messageHandler.messages[0].level, IECore.MessageHandler.Level.Warning )
		self.assertEqual( messageHandler.messages[0].context, "ToMayaLocatorConverter::doConversion" )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
