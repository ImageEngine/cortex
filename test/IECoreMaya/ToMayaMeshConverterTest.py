##########################################################################
#
#  Copyright (c) 2008-2014, Image Engine Design Inc. All rights reserved.
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
import maya.OpenMaya as OpenMaya

import IECore
import IECoreMaya

class ToMayaMeshConverterTest( IECoreMaya.TestCase ) :

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
		
		coreMesh[ "testUVSet_s" ] = IECore.PrimitiveVariable( coreMesh["s"].interpolation, coreMesh["s"].data.copy() )
		coreMesh[ "testUVSet_t" ] = IECore.PrimitiveVariable( coreMesh["t"].interpolation, coreMesh["t"].data.copy() )
		
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
		
		self.assertEqual( u[0], coreMesh[ "s" ].data[0] )
		self.assertEqual( v[0], 1.0 - coreMesh[ "t" ].data[0] )
		
		fnMesh.getUVs( u, v, "testUVSet" )

		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )
	
		self.assertEqual( u[12], coreMesh[ "testUVSet_s" ].data[12] )
		self.assertEqual( v[12], 1.0 - coreMesh[ "testUVSet_t" ].data[12] )

	def testUVConversionFromPlug( self ) :
		
		coreMesh = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		
		self.assert_( "s" in coreMesh )
		self.assert_( "t" in coreMesh )
		
		coreMesh[ "testUVSet_s" ] = IECore.PrimitiveVariable( coreMesh["s"].interpolation, coreMesh["s"].data.copy() )
		coreMesh[ "testUVSet_t" ] = IECore.PrimitiveVariable( coreMesh["t"].interpolation, coreMesh["t"].data.copy() )
		
		fn = IECoreMaya.FnOpHolder.create( "test", "meshMerge" )
		op = fn.getOp()
		with fn.parameterModificationContext() :
			op["input"].setValue( coreMesh )
		
		mayaMesh = maya.cmds.ls( maya.cmds.polyPlane(), dag=True, type="mesh" )[0]
		maya.cmds.connectAttr( fn.name()+".result", mayaMesh+".inMesh", force=True )
		
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
		
		self.assertEqual( u[0], coreMesh[ "s" ].data[0] )
		self.assertEqual( v[0], 1.0 - coreMesh[ "t" ].data[0] )
		
		fnMesh.getUVs( u, v, "testUVSet" )
		
		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )
		
		self.assertEqual( u[12], coreMesh[ "testUVSet_s" ].data[12] )
		self.assertEqual( v[12], 1.0 - coreMesh[ "testUVSet_t" ].data[12] )
	
	def testManyUVConversionsFromPlug( self ) :
		
		coreMesh = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		
		self.assertTrue( "s" in coreMesh )
		self.assertTrue( "t" in coreMesh )
		
		for i in range( 0, 7 ) :
			coreMesh[ "testUVSet%d_s" % i ] = IECore.PrimitiveVariable( coreMesh["s"].interpolation, coreMesh["s"].data.copy() )
			coreMesh[ "testUVSet%d_t" % i ] = IECore.PrimitiveVariable( coreMesh["t"].interpolation, coreMesh["t"].data.copy() )
		
		fn = IECoreMaya.FnOpHolder.create( "test", "meshMerge" )
		
		mayaMesh = maya.cmds.ls( maya.cmds.polyPlane(), dag=True, type="mesh" )[0]
		maya.cmds.connectAttr( fn.name()+".result", mayaMesh+".inMesh", force=True )
		
		op = fn.getOp()
		with fn.parameterModificationContext() :
			op["input"].setValue( coreMesh )
		
		maya.cmds.file( rename="/tmp/test.ma" )
		maya.cmds.file( save=True )
		maya.cmds.file( new=True, f=True )
		maya.cmds.file( "/tmp/test.ma", open=True )
		
		fnMesh = OpenMaya.MFnMesh( IECoreMaya.dagPathFromString( mayaMesh ) )
		self.assertEqual( fnMesh.numPolygons(), 760 )
		# When calling fnMesh.numFaceVertices() (and other MFnMesh API calls), given a mesh with 6
		# or more UV sets, which has never been evaluated before, the first call throws kFailure.
		# From within the ToMayaMeshConverter itself, the output plug appears fine, and the API calls
		# evaluate as expected. Despite this, the resulting mesh cannot be evaluated on the first try.
		# Making the mesh visible, or making any attempt to evaluate it, will trigger some unknown
		# internal updating, and subsequent attempts to evaluate it will succeed. Meshes with 5 or less
		# UV sets do not suffer from this problem. Leaving this test failing in case it is fixed in a
		# future Maya, and to mark the issue so users of ToMayaMeshConverter have breadcrumbs to follow.
		self.assertEqual( fnMesh.numFaceVertices(), 2280 ) # Known failure. See note above for an explanation.
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, vertex=True ), 382 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, face=True ), 760 )
		
		u = OpenMaya.MFloatArray()
		v = OpenMaya.MFloatArray()
		
		fnMesh.getUVs( u, v )
		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )
		self.assertEqual( u[0], coreMesh[ "s" ].data[0] )
		self.assertEqual( v[0], 1.0 - coreMesh[ "t" ].data[0] )
		
		for i in range( 0, 7 ) :
			
			fnMesh.getUVs( u, v, "testUVSet%d" % i )
			self.assertEqual( u.length(), 2280 )
			self.assertEqual( v.length(), 2280 )
			self.assertEqual( u[12], coreMesh[ "testUVSet%d_s" % i ].data[12] )
			self.assertEqual( v[12], 1.0 - coreMesh[ "testUVSet%d_t" % i ].data[12] )
	
	def testUVConversionFromMayaMesh( self ) :
		
		mayaMesh = maya.cmds.ls( maya.cmds.polyPlane(), dag=True, type="mesh" )[0]
		coreMesh = IECoreMaya.FromMayaMeshConverter( mayaMesh ).convert()
		
		transform = maya.cmds.createNode( "transform" )
		self.failUnless( IECoreMaya.ToMayaMeshConverter( coreMesh ).convert( transform ) )
		mayaMesh2 = maya.cmds.listRelatives( transform, shapes=True )[0]
		
		l = OpenMaya.MSelectionList()
		l.add( mayaMesh )
		l.add( mayaMesh2 )
		p = OpenMaya.MDagPath()
		p2 = OpenMaya.MDagPath()
		l.getDagPath( 0, p )
		l.getDagPath( 1, p2 )
		
		uvSets = []
		fnMesh = OpenMaya.MFnMesh( p )
		fnMesh.getUVSetNames( uvSets )
		
		uvSets2 = []
		fnMesh2 = OpenMaya.MFnMesh( p2 )
		fnMesh2.getUVSetNames( uvSets2 )
		
		self.assertEqual( uvSets, uvSets2 )
		
		# Check uvIndices
		coreMesh2 = IECoreMaya.FromMayaMeshConverter( mayaMesh2 ).convert()
		self.assertEqual( coreMesh["stIndices"].data, coreMesh2["stIndices"].data )
	
	def testShadingGroup( self ) :
	
		coreMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 10 ) ) )
		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		transform = maya.cmds.createNode( "transform" )
		converter.convert( transform )
		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]

		self.failUnless( mayaMesh in maya.cmds.sets( "initialShadingGroup", query=True ) )
		
	def testConstructor( self ) :
	
		coreMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 10 ) ) )
		
		converter = IECoreMaya.ToMayaMeshConverter( coreMesh )
		transform = maya.cmds.createNode( "transform" )
		converter.convert( transform )
		self.assertEqual( maya.cmds.nodeType( maya.cmds.listRelatives( transform, shapes=True )[0] ), "mesh" )
	
	def testNormals( self ) :
		
		sphere = maya.cmds.polySphere( subdivisionsX=4, subdivisionsY=3, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]
		maya.cmds.polySoftEdge( sphere, angle=145 )

		mesh = IECoreMaya.FromMayaShapeConverter.create( sphere ).convert()
		self.failUnless( "N" in mesh )
		self.failUnless( mesh.arePrimitiveVariablesValid() )
		self.assertEqual( mesh["N"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.failUnless( isinstance( mesh["N"].data, IECore.V3fVectorData ) )
		
		transform = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaObjectConverter.create( mesh ).convert( transform )
		newSphere = maya.cmds.listRelatives( transform, shapes=True )[0]
		
		normals3d = IECore.DataConvertOp()( data=mesh["N"].data, targetType=IECore.TypeId.V3dVectorData )
		del mesh["N"]
		mesh["N"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, normals3d )
		self.failUnless( mesh.arePrimitiveVariablesValid() )
		self.failUnless( isinstance( mesh["N"].data, IECore.V3dVectorData ) )
		
		transform2 = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaObjectConverter.create( mesh ).convert( transform2 )
		newSphere2 = maya.cmds.listRelatives( transform2, shapes=True )[0]
		
		for i in range( 0, len(maya.cmds.ls( sphere+'.vtx[*]', fl=True )) ) :
			origNormal = maya.cmds.polyNormalPerVertex( sphere+'.vtx['+str(i)+']', query=True, xyz=True )
			normal3f = maya.cmds.polyNormalPerVertex( newSphere+'.vtx['+str(i)+']', query=True, xyz=True )
			normal3d = maya.cmds.polyNormalPerVertex( newSphere2+'.vtx['+str(i)+']', query=True, xyz=True )
			self.assertEqual( origNormal, normal3f )
			self.assertEqual( origNormal, normal3d )

	def testSetMeshInterpolation( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		self.assertRaises( ValueError, maya.cmds.getAttr, sphere + ".ieMeshInterpolation" )

		IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( sphere )
		self.assertEqual( maya.cmds.getAttr( sphere + ".ieMeshInterpolation" ), 0 )

		coreMesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 10 ) ) )
		coreMesh.interpolation = "catmullClark"
		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		transform = maya.cmds.createNode( "transform" )
		self.assert_( converter.convert( transform ) )
		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]
		self.assertEqual( maya.cmds.getAttr( mayaMesh + ".ieMeshInterpolation" ), 1 )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
