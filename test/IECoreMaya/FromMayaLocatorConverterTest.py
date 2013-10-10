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
import maya.OpenMaya

import IECore
import IECoreMaya

class FromMayaLocatorConverterTest( IECoreMaya.TestCase ) :

	def setUp( self ) :

		maya.cmds.file( new=True, f=True )

		locatorShape = maya.cmds.createNode( "locator", name = "myLocator" )
		locatorTransform = "locator1"
		maya.cmds.setAttr( locatorTransform + ".translate", 1,2,3 )
		maya.cmds.setAttr( locatorShape + ".localPosition", 10,20,30 )
		maya.cmds.setAttr( locatorShape + ".localScale", 1,2,3 )
		
	def testFactory( self ) :
	
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "myLocator" )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaLocatorConverter.staticTypeId() ) )

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "myLocator", IECore.CoordinateSystem.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaLocatorConverter.staticTypeId() ) )
		
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "myLocator", IECore.Renderable.staticTypeId() )
		self.failUnless( converter.isInstanceOf( IECoreMaya.FromMayaLocatorConverter.staticTypeId() ) )
		
		converter = IECoreMaya.FromMayaDagNodeConverter.create( "myLocator", IECore.Writer.staticTypeId() )
		self.assertEqual( converter, None )
		
	def test( self ) :

		converter = IECoreMaya.FromMayaDagNodeConverter.create( "myLocator" )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaLocatorConverter ) ) )

		locator = converter.convert()
		self.assert_( locator.isInstanceOf( IECore.CoordinateSystem.staticTypeId() ) )

		self.assertEqual( locator.getName(), "myLocator" )
		m = locator.getTransform().transform()
		(s,h,r,t) = m.extractSHRT()
		self.assertEqual( s, IECore.V3f(1,2,3) )
		self.assertEqual( t, IECore.V3f(10,20,30) )
		self.assertEqual( r, IECore.V3f(0,0,0) )
		self.assertEqual( h, IECore.V3f(0,0,0) )
		
	def testConstructor( self ) :
	
		converter = IECoreMaya.FromMayaLocatorConverter( "myLocator" )
		camera = converter.convert()
		self.assert_( camera.isInstanceOf( IECore.CoordinateSystem.staticTypeId() ) )	
		
if __name__ == "__main__":
	IECoreMaya.TestProgram()
