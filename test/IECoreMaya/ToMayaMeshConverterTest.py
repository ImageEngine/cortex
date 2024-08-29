##########################################################################
#
#  Copyright (c) 2008-2015, Image Engine Design Inc. All rights reserved.
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

import unittest

import maya.cmds
import maya.OpenMaya as OpenMaya
import imath

import IECore
import IECoreScene
import IECoreMaya

class ToMayaMeshConverterTest( IECoreMaya.TestCase ) :

	def testConversion( self ) :

		coreMesh = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -10 ), imath.V3f( 10 ) ) )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaObjectConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECore.FromCoreConverter.staticTypeId() ) )

		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )
		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]

		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, vertex=True ), 8 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, face=True ), 6 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, boundingBox=True ), ( (-10, 10), (-10, 10), (-10, 10) ) )

	def testUVConversion( self ) :

		coreMesh = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()

		self.assertTrue( "uv" in coreMesh )

		coreMesh[ "testUVSet" ] = IECoreScene.PrimitiveVariable( coreMesh["uv"].interpolation, coreMesh["uv"].data.copy() )

		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaObjectConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECoreMaya.ToMayaConverter.staticTypeId() ) )
		self.assertTrue( converter.isInstanceOf( IECore.FromCoreConverter.staticTypeId() ) )

		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )
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

		self.assertEqual( u[0], coreMesh[ "uv" ].data[0][0] )
		self.assertEqual( v[0], coreMesh[ "uv" ].data[0][1] )

		fnMesh.getUVs( u, v, "testUVSet" )

		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )

		self.assertEqual( u[12], coreMesh[ "testUVSet" ].data[12][0] )
		self.assertEqual( v[12], coreMesh[ "testUVSet" ].data[12][1] )

	def testUVConversionFromPlug( self ) :

		coreMesh = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()

		self.assertTrue( "uv" in coreMesh )

		coreMesh[ "testUVSet" ] = IECoreScene.PrimitiveVariable( coreMesh["uv"].interpolation, coreMesh["uv"].data.copy() )

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

		self.assertEqual( u[0], coreMesh[ "uv" ].data[0][0] )
		self.assertEqual( v[0], coreMesh[ "uv" ].data[0][1] )

		fnMesh.getUVs( u, v, "testUVSet" )

		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )

		self.assertEqual( u[12], coreMesh[ "testUVSet" ].data[12][0] )
		self.assertEqual( v[12], coreMesh[ "testUVSet" ].data[12][1] )

	@unittest.skipIf( maya.OpenMaya.MGlobal.apiVersion() < 201600, "Invisible meshes with 6+ UV sets cause seg faults prior to Maya 2016" )
	def testManyUVConversionsFromPlug( self ) :

		coreMesh = IECore.Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()

		self.assertTrue( "uv" in coreMesh )

		for i in range( 0, 7 ) :
			coreMesh[ "testUVSet%d" % i ] = IECoreScene.PrimitiveVariable( coreMesh["uv"].interpolation, coreMesh["uv"].data.copy() )

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
		# UV sets do not suffer from this problem. This was fixed in Maya 2016, but I'll leave
		# this explanation so users of ToMayaMeshConverter have breadcrumbs to follow.
		self.assertEqual( fnMesh.numFaceVertices(), 2280 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, vertex=True ), 382 )
		self.assertEqual( maya.cmds.polyEvaluate( mayaMesh, face=True ), 760 )

		u = OpenMaya.MFloatArray()
		v = OpenMaya.MFloatArray()

		fnMesh.getUVs( u, v )
		self.assertEqual( u.length(), 2280 )
		self.assertEqual( v.length(), 2280 )
		self.assertEqual( u[0], coreMesh[ "uv" ].data[0][0] )
		self.assertEqual( v[0], coreMesh[ "uv" ].data[0][1] )

		for i in range( 0, 7 ) :

			fnMesh.getUVs( u, v, "testUVSet%d" % i )
			self.assertEqual( u.length(), 2280 )
			self.assertEqual( v.length(), 2280 )
			self.assertEqual( u[12], coreMesh[ "testUVSet%d" % i ].data[12][0] )
			self.assertEqual( v[12], coreMesh[ "testUVSet%d" % i ].data[12][1] )

	def testUVConversionFromMayaMesh( self ) :

		mayaMesh = maya.cmds.ls( maya.cmds.polyPlane(), dag=True, type="mesh" )[0]
		coreMesh = IECoreMaya.FromMayaMeshConverter( mayaMesh ).convert()

		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( IECoreMaya.ToMayaMeshConverter( coreMesh ).convert( transform ) )
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
		# self.assertEqual( coreMesh["uv"].data, coreMesh2["uv"].data )
		self.assertEqual( coreMesh["uv"].indices, coreMesh2["uv"].indices )

	def testShadingGroup( self ) :

		coreMesh = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -10 ), imath.V3f( 10 ) ) )
		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		transform = maya.cmds.createNode( "transform" )
		converter.convert( transform )
		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]

		self.assertTrue( mayaMesh in maya.cmds.sets( "initialShadingGroup", query=True ) )

	def testConstructor( self ) :

		coreMesh = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -10 ), imath.V3f( 10 ) ) )

		converter = IECoreMaya.ToMayaMeshConverter( coreMesh )
		transform = maya.cmds.createNode( "transform" )
		converter.convert( transform )
		self.assertEqual( maya.cmds.nodeType( maya.cmds.listRelatives( transform, shapes=True )[0] ), "mesh" )

	def testNormals( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=4, subdivisionsY=3, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]
		maya.cmds.polySoftEdge( sphere, angle=145 )

		mesh = IECoreMaya.FromMayaShapeConverter.create( sphere ).convert()
		self.assertTrue( "N" in mesh )
		self.assertTrue( mesh.arePrimitiveVariablesValid() )
		self.assertEqual( mesh["N"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertTrue( isinstance( mesh["N"].data, IECore.V3fVectorData ) )

		transform = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaObjectConverter.create( mesh ).convert( transform )
		newSphere = maya.cmds.listRelatives( transform, shapes=True )[0]

		normals3d = IECore.DataConvertOp()( data=mesh["N"].data, targetType=IECore.TypeId.V3dVectorData )
		del mesh["N"]
		mesh["N"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, normals3d )
		self.assertTrue( mesh.arePrimitiveVariablesValid() )
		self.assertTrue( isinstance( mesh["N"].data, IECore.V3dVectorData ) )

		transform2 = maya.cmds.createNode( "transform" )
		IECoreMaya.ToMayaObjectConverter.create( mesh ).convert( transform2 )
		newSphere2 = maya.cmds.listRelatives( transform2, shapes=True )[0]

		for i in range( 0, len(maya.cmds.ls( sphere+'.vtx[*]', fl=True )) ) :
			origNormal = maya.cmds.polyNormalPerVertex( sphere+'.vtx['+str(i)+']', query=True, xyz=True )
			normal3f = maya.cmds.polyNormalPerVertex( newSphere+'.vtx['+str(i)+']', query=True, xyz=True )
			normal3d = maya.cmds.polyNormalPerVertex( newSphere2+'.vtx['+str(i)+']', query=True, xyz=True )
			for j in range( 0, len(origNormal) ) :
				self.assertAlmostEqual( origNormal[j], normal3f[j], 6 )
				self.assertAlmostEqual( origNormal[j], normal3d[j], 6 )

	def testSetMeshInterpolation( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		self.assertRaises( ValueError, maya.cmds.getAttr, sphere + ".ieMeshInterpolation" )

		IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( sphere )
		self.assertEqual( maya.cmds.getAttr( sphere + ".ieMeshInterpolation" ), 0 )

		coreMesh = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -10 ), imath.V3f( 10 ) ) )
		coreMesh.interpolation = "catmullClark"
		converter = IECoreMaya.ToMayaObjectConverter.create( coreMesh )
		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )
		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]
		self.assertEqual( maya.cmds.getAttr( mayaMesh + ".ieMeshInterpolation" ), 1 )

	def testCreases( self ) :

		cortexCube = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )

		cornerIds = [ 5 ]
		cornerSharpnesses = [ 10.0 ]

		cortexCube.setCorners( IECore.IntVectorData( cornerIds ), IECore.FloatVectorData( cornerSharpnesses ) )

		creaseLengths = [ 3, 2 ]
		creaseIds = [ 1, 2, 3, 4, 5 ]  # note that these are vertex ids
		creaseSharpnesses = [ 1, 5 ]

		cortexCube.setCreases( IECore.IntVectorData( creaseLengths ), IECore.IntVectorData( creaseIds ), IECore.FloatVectorData( creaseSharpnesses ) )

		converter = IECoreMaya.ToMayaObjectConverter.create( cortexCube )
		transform = maya.cmds.createNode( "transform" )
		self.assertTrue( converter.convert( transform ) )

		mayaMesh = maya.cmds.listRelatives( transform, shapes=True )[0]

		l = OpenMaya.MSelectionList()
		l.add( mayaMesh )
		p = OpenMaya.MDagPath()
		l.getDagPath( 0, p )

		fnMesh = OpenMaya.MFnMesh( p )

		# Test corners

		cornerIds = OpenMaya.MUintArray()
		cornerSharpnesses = OpenMaya.MDoubleArray()
		fnMesh.getCreaseVertices( cornerIds, cornerSharpnesses )

		testIds = OpenMaya.MUintArray()
		testIds.append( 5 )
		self.assertEqual( cornerIds, testIds )

		testSharpnesses = OpenMaya.MFloatArray()
		testSharpnesses.append( 10 )
		self.assertEqual( cornerSharpnesses, testSharpnesses )

		# Test edges

		edgeIds = OpenMaya.MUintArray()
		edgeSharpnesses = OpenMaya.MDoubleArray()
		fnMesh.getCreaseEdges( edgeIds, edgeSharpnesses )

		util = OpenMaya.MScriptUtil()

		result = []
		for edgeId, sharpness in zip( edgeIds, edgeSharpnesses ) :

			edgeVertices = util.asInt2Ptr()
			fnMesh.getEdgeVertices( edgeId, edgeVertices )

			result.append( (util.getInt2ArrayItem( edgeVertices, 0, 1 ),
							util.getInt2ArrayItem( edgeVertices, 0, 0 ),
							sharpness) )

		# we compare sets because maya reorders by edge index
		self.assertEqual( set( result ), set( [ ( 1, 2, 1.0 ), ( 2, 3, 1.0 ), ( 4, 5, 5.0 ) ] ) )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
