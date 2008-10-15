##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.cmds

class FromMayaTransformConverterTest( unittest.TestCase ) :

	def test( self ) :
		
		locatorTransform = maya.cmds.spaceLocator()[0]
		
		c = IECoreMaya.FromMayaDagNodeConverter.create( str( locatorTransform ), IECore.TypeId.TransformationMatrixdData )
		
		self.assertEqual( IECoreMaya.TypeId.FromMayaTransformConverter, IECoreMaya.FromMayaTransformConverter.staticTypeId() )
		self.assertEqual( c.typeId(), IECoreMaya.FromMayaTransformConverter.staticTypeId() )
		
		t = c.convert()
		self.assert_( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, IECore.M44d() )
		
		maya.cmds.xform( locatorTransform, translation=( 1, 2, 3 ) )
		
		t = c.convert()
		self.assert_( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, IECore.M44d.createTranslated( IECore.V3d( 1, 2, 3 ) ) )
		
		group = maya.cmds.group( locatorTransform )
		
		t = c.convert()
		self.assert_( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, IECore.M44d.createTranslated( IECore.V3d( 1, 2, 3 ) ) )
		
		maya.cmds.xform( group, translation=( 1, 0, 10 ) )
		
		t = c.convert()
		self.assert_( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, IECore.M44d.createTranslated( IECore.V3d( 2, 2, 13 ) ) )
		
		c["space"].setValue( "Local" )
		
		t = c.convert()
		self.assert_( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, IECore.M44d.createTranslated( IECore.V3d( 1, 2, 3 ) ) )
		
		locatorShape = maya.cmds.listRelatives( locatorTransform, children=True )[0]
		
		c = IECoreMaya.FromMayaTransformConverter( str( locatorShape ) )
		
		t = c.convert()
		self.assert_( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, IECore.M44d.createTranslated( IECore.V3d( 2, 2, 13 ) ) )
	
		c["space"].setValue( "Local" )
		
		t = c.convert()
		self.assert_( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, IECore.M44d() )
				
if __name__ == "__main__":
	MayaUnitTest.TestProgram()
