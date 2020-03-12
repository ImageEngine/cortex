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
import imath

import IECore
import IECoreMaya

class FromMayaTransformConverterTest( IECoreMaya.TestCase ) :

	def test( self ) :

		locatorTransform = maya.cmds.spaceLocator()[0]

		c = IECoreMaya.FromMayaDagNodeConverter.create( str( locatorTransform ), IECore.TypeId.TransformationMatrixdData )

		self.assertEqual( IECoreMaya.TypeId.FromMayaTransformConverter, IECoreMaya.FromMayaTransformConverter.staticTypeId() )
		self.assertEqual( c.typeId(), IECoreMaya.FromMayaTransformConverter.staticTypeId() )

		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, imath.M44d() )

		maya.cmds.xform( locatorTransform, translation=( 1, 2, 3 ) )

		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, imath.M44d().translate( imath.V3d( 1, 2, 3 ) ) )

		group = maya.cmds.group( locatorTransform )

		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, imath.M44d().translate( imath.V3d( 1, 2, 3 ) ) )

		maya.cmds.xform( group, translation=( 1, 0, 10 ) )

		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, imath.M44d().translate( imath.V3d( 2, 2, 13 ) ) )

		c["space"].setValue( "Local" )

		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, imath.M44d().translate( imath.V3d( 1, 2, 3 ) ) )

		# test custom space
		customSpace = imath.M44f()
		customSpace.setScale( imath.V3f( 0.5, 0.5, 0.5 ) )
		c["space"].setValue( "Custom" )
		c["customSpace"].setValue( IECore.M44fData( customSpace ) )
		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		expectedResult = imath.M44d( 2, 0, 0, 0,   0, 2, 0, 0,   0, 0, 2, 0,   4, 4, 26, 1 )
		self.assertEqual( t.value.transform, expectedResult )
		# sanity check: if we apply the custom space to the result we should get the world space result
		self.assertEqual( t.value.transform * imath.M44d().scale( imath.V3d( 0.5, 0.5, 0.5 ) ), imath.M44d().translate( imath.V3d( 2, 2, 13 ) ) )

		locatorShape = maya.cmds.listRelatives( locatorTransform, children=True )[0]

		c = IECoreMaya.FromMayaTransformConverter( str( locatorShape ) )

		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, imath.M44d().translate( imath.V3d( 2, 2, 13 ) ) )

		c["space"].setValue( "Local" )

		t = c.convert()
		self.assertTrue( t.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )
		self.assertEqual( t.value.transform, imath.M44d() )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
