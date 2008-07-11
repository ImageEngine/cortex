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
import maya.OpenMaya as OpenMaya

class ToMayaMeshConverterTest( unittest.TestCase ) :

	def testConversion( self ) :
	
		coreMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 10 ) ) )
		
		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		self.assert_( converter.isInstanceOf( IECoreMaya.ToMayaObjectConverter.staticTypeId() ) )
		self.assert_( converter.isInstanceOf( IECoreMaya.ToMayaConverter.staticTypeId() ) )
		self.assert_( converter.isInstanceOf( IECore.FromCoreConverter.staticTypeId() ) )

		transform = maya.cmds.createNode( "transform" )				
		self.assert_( converter.convert( transform ) )
		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]
	
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, vertex=True ), 8 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, face=True ), 6 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, boundingBox=True ), ( (-10, 10), (-10, 10), (-10, 10) ) )
		
	def testUVConversion( self ) :
	
		coreMesh = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		
		self.assert_( "s" in coreMesh )
		self.assert_( "t" in coreMesh )
		
		coreMesh[ "testUVSet_s" ] = coreMesh[ "s" ]		
		coreMesh[ "testUVSet_t" ] = coreMesh[ "t" ]		
		
		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		self.assert_( converter.isInstanceOf( IECoreMaya.ToMayaObjectConverter.staticTypeId() ) )
		self.assert_( converter.isInstanceOf( IECoreMaya.ToMayaConverter.staticTypeId() ) )
		self.assert_( converter.isInstanceOf( IECore.FromCoreConverter.staticTypeId() ) )

		transform = maya.cmds.createNode( "transform" )				
		self.assert_( converter.convert( transform ) )
		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]
	
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, vertex=True ), 382 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, face=True ), 760 )
		
		bb = maya.cmds.polyEvaluate( mayaMesh, boundingBox=True )
		
		self.assertAlmostEqual( bb[0][0], -1, 4 )
		self.assertAlmostEqual( bb[0][1],  1, 4 )		
		self.assertAlmostEqual( bb[1][0], -1, 4 )
		self.assertAlmostEqual( bb[1][1],  1, 4 )		
		self.assertAlmostEqual( bb[2][0], -1, 4 )
		self.assertAlmostEqual( bb[2][1],  1, 4 )												
		
		l = OpenMaya.MSelectionList()
		l.add( mayaMesh )
		p = OpenMaya.MDagPath()
		l.getDagPath( 0, p )
		
		fnMesh = OpenMaya.MFnMesh( p )
		u = OpenMaya.MFloatArray()
		v = OpenMaya.MFloatArray()
		
		fnMesh.getUVs( u, v )
		
		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )		
		
		fnMesh.getUVs( u, v, "testUVSet" )
		
		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )				
		
				
if __name__ == "__main__":
	MayaUnitTest.TestProgram()
