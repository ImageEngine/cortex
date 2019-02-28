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

import os.path
import maya.cmds
import maya.OpenMaya as OpenMaya
import imath

import IECore
import IECoreScene
import IECoreMaya

class FromMayaMeshConverterTest( IECoreMaya.TestCase ) :

	__testFile = "/tmp/test.scc"

	def writeTestScc( self ):

		scene = IECoreScene.SceneCache( self.__testFile, IECore.IndexedIO.OpenMode.Write )
		sc = scene.createChild( str( 1 ) )
		mesh = IECoreScene.MeshPrimitive.createBox(imath.Box3f(imath.V3f(0),imath.V3f(1)))

		creaseLengths = [ 3, 2 ]
		creaseIds = [ 1, 2, 3, 4, 5 ]
		creaseSharpnesses = [ 1, 5 ]
		mesh.setCreases( IECore.IntVectorData( creaseLengths ), IECore.IntVectorData( creaseIds ), IECore.FloatVectorData( creaseSharpnesses ) )

		sc.writeObject( mesh, 0 )

		del scene

	def testFactory( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere, IECoreScene.TypeId.MeshPrimitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere, IECoreScene.TypeId.Primitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( sphere )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( sphere, IECoreScene.TypeId.MeshPrimitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( sphere, IECoreScene.TypeId.Primitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

	def testConstructor( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaMeshConverter( sphere )

		m = converter.convert()

		self.failUnless( isinstance( m, IECoreScene.MeshPrimitive ) )

	def testParameters( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["interpolation"].getTypedValue(), "default" )
		p = converter.convert()
		self.assertEqual( p.interpolation, "linear" )
		self.assertTrue( "N" in p )
		converter["interpolation"].setTypedValue( "linear" )
		p = converter.convert()
		self.assertEqual( p.interpolation, "linear" )
		converter["interpolation"].setTypedValue( "catmullClark" )
		p = converter.convert()
		self.assertFalse( "N" in p )
		self.assertEqual( p.interpolation, "catmullClark" )

	def testInterpolationType( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		# first time creates the plug
		IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( sphere, "catmullClark" )
		mesh = IECoreMaya.FromMayaShapeConverter.create( sphere ).convert()
		self.assertEqual( mesh.interpolation, "catmullClark" )

		# second time, just update the plug
		IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( sphere, "linear" )
		mesh = IECoreMaya.FromMayaShapeConverter.create( sphere ).convert()
		self.assertEqual( mesh.interpolation, "linear" )

		# accepts the labels for the presets "subdiv" -> "catmullClark"
		IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( sphere, "subdiv" )
		mesh = IECoreMaya.FromMayaShapeConverter.create( sphere ).convert()
		self.assertEqual( mesh.interpolation, "catmullClark" )

		# accepts the labels for the presets "poly" -> "linear"
		IECoreMaya.ToMayaMeshConverter.setMeshInterpolationAttribute( sphere, "poly" )
		mesh = IECoreMaya.FromMayaShapeConverter.create( sphere ).convert()
		self.assertEqual( mesh.interpolation, "linear" )

	def testSphere( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )

		m = converter.convert()

		# check topology
		self.assertEqual( m.verticesPerFace.size(), 50 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 50 )
		self.assertEqual( m.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 42 )
		self.assertEqual( m["P"].data.size(), 42 )
		self.assertEqual( m["N"].data.size(), 180 )
		self.assertEqual( m["uv"].data.size(), 64 )
		self.assertEqual( m["uv"].indices.size(), 180 )
		self.assertEqual( m["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( m["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertEqual( m["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )

		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1.0001 ), imath.V3f( 1.0001 ) ), m.bound() ) )
		self.assertTrue( IECore.BoxAlgo.contains( m.bound(), imath.Box3f( imath.V3f( -0.90 ), imath.V3f( 0.90 ) ) ) )

	def testSpaces( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		maya.cmds.move( 1, 2, 3, sphere )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )

		self.assertEqual( converter["space"].getNumericValue(), IECoreMaya.FromMayaCurveConverter.Space.Object )
		m = converter.convert()
		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1.0001 ), imath.V3f( 1.0001 ) ), m.bound() ) )

		converter["space"].setNumericValue( IECoreMaya.FromMayaShapeConverter.Space.World )
		m = converter.convert()
		self.assertTrue( imath.Box3f( imath.V3f( -1.0001 ) + imath.V3f( 1, 2, 3 ), imath.V3f( 1.0001 ) + imath.V3f( 1, 2, 3 ) ), m.bound() )

	def testNormalsOnlyWhenLinear( self ) :

		# adding normals to a mesh which will be rendered subdivided is a very bad thing to do.
		# make sure we aren't doing it.

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )

		m = converter.convert()
		self.assert_( "N" in m )
		self.assertEqual( m["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		converter["interpolation"].setTypedValue( "catmullClark" )
		m = converter.convert()
		self.assert_( not "N" in m )

	def testWindingOrder( self ) :

		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( plane )

		m = converter.convert()

		p = m["P"].data
		vertexIds = m.vertexIds
		self.assertEqual( vertexIds.size(), 4 )
		loop = IECore.V3fVectorData( [ p[vertexIds[0]], p[vertexIds[1]], p[vertexIds[2]], p[vertexIds[3]] ] )

		self.assert_( IECore.polygonNormal( loop ).equalWithAbsError( imath.V3f( 0, 1, 0 ), 0.0001 ) )

	def testBlindData( self ) :

		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]

		maya.cmds.addAttr( plane, dataType="string", longName="ieString" )
		maya.cmds.setAttr( plane + ".ieString", "banana", type="string" )

		converter = IECoreMaya.FromMayaShapeConverter.create( plane )
		converter['blindDataAttrPrefix'] = IECore.StringData("ie")
		m = converter.convert()

		self.assertEqual( len( m.blindData().keys() ), 2 )
		self.assertEqual( m.blindData()["name"], IECore.StringData( "pPlaneShape1" ) )
		self.assertEqual( m.blindData()["ieString"], IECore.StringData( "banana" ) )

	def testPrimVars( self ) :

		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]

		maya.cmds.addAttr( plane, attributeType="float", longName="iePrimVarDouble", defaultValue=1 )
		maya.cmds.addAttr( plane, dataType="doubleArray", longName="iePrimVarDoubleArray" )
		maya.cmds.setAttr( plane + ".iePrimVarDoubleArray", ( 10, 11, 12, 13 ), type="doubleArray" )

		converter = IECoreMaya.FromMayaShapeConverter.create( plane, IECoreScene.MeshPrimitive.staticTypeId() )
		m = converter.convert()

		self.assertEqual( set( m.keys() ), set( [ "P", "N", "uv", "Double", "DoubleArray" ] ) )
		self.assertEqual( m["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( m["uv"].data, IECore.V2fVectorData( [ imath.V2f( 0, 0 ), imath.V2f( 1, 0 ), imath.V2f( 0, 1 ), imath.V2f( 1, 1 ) ], IECore.GeometricData.Interpretation.UV ) )
		self.assertEqual( m["uv"].indices, IECore.IntVectorData( [ 0, 1, 3, 2 ] ) )
		self.assertEqual( m["Double"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m["Double"].data, IECore.FloatData( 1 ) )
		self.assertEqual( m["DoubleArray"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m["DoubleArray"].data, IECore.FloatVectorData( [ 10, 11, 12, 13 ] ) )

	def testConvertFromPlug( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		maya.cmds.move( 1, 2, 3, sphere )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaPlugConverter.create( sphere + ".worldMesh" )

		converter["space"].setNumericValue( IECoreMaya.FromMayaShapeConverter.Space.World )
		m = converter.convert()
		self.assertEqual( m["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( m["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertTrue( IECore.BoxAlgo.contains( imath.Box3f( imath.V3f( -1.0001 ) + imath.V3f( 1, 2, 3 ), imath.V3f( 1.0001 ) + imath.V3f( 1, 2, 3 ) ), m.bound() ) )

	def testSharedUVIndices( self ) :

		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/twoTrianglesWithSharedUVs.ma", force = True, open = True )

		mesh = IECoreMaya.FromMayaShapeConverter.create( "pPlaneShape1" ).convert()

		self.failUnless( "uv" in mesh )
		self.assertEqual( mesh["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["uv"].indices, IECore.IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		self.assertEqual( mesh["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )

	def testSplitUVIndices( self ) :

		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/twoTrianglesWithSplitUVs.ma", force = True, open = True )

		mesh = IECoreMaya.FromMayaShapeConverter.create( "pPlaneShape1" ).convert()

		self.failUnless( "uv" in mesh )
		self.assertEqual( mesh["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["uv"].indices, IECore.IntVectorData( [ 0, 1, 5, 2, 4, 3 ] ) )
		self.assertEqual( mesh["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )

	def testExtraSTs( self ) :

		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( plane, IECoreScene.MeshPrimitive.staticTypeId() )
		m = converter.convert()

		self.assert_( "uv" in m )
		# map1 is the default set
		self.assert_( "map1" not in m )

		maya.cmds.polyUVSet( plane, copy=True, uvSet="map1", newUVSet="map2" )

		m = converter.convert()

		self.assert_( "uv" in m )
		self.assert_( "map1" not in m )
		self.assert_( "map2" in m )

		self.assertEqual( m["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.assertEqual( m["map2"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )

	def testManyUVConversionsFromPlug( self ) :

		# load a mesh with indexed UVs
		scc = IECoreScene.SceneCache( "test/IECore/data/sccFiles/animatedSpheres.scc", IECore.IndexedIO.OpenMode.Read )
		coreMesh = scc.scene( [ "A", "a" ] ).readObject( 0 )
		self.assertTrue( "uv" in coreMesh )
		self.assertEqual( coreMesh["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )

		for i in range( 0, 7 ) :
			coreMesh[ "testUVSet%d" % i ] = IECoreScene.PrimitiveVariable( coreMesh["uv"].interpolation, coreMesh["uv"].data.copy(), coreMesh["uv"].indices.copy() )

		self.assertTrue( coreMesh.arePrimitiveVariablesValid() )

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

		result = IECoreMaya.FromMayaMeshConverter( mayaMesh ).convert()

		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 400 )
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ), 1560 )

		self.assertEqual( coreMesh["uv"], result["uv"] )

		for i in range( 0, 7 ) :
			self.assertEqual( coreMesh[ "testUVSet%d" % i ], result[ "testUVSet%d" %  i ] )

	def testColors( self ):

		# test alpha to rgb conversion
		mesh = "pPlaneShape1"
		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/colouredPlane.ma", force = True, open = True )
		sel = OpenMaya.MSelectionList()
		sel.add( mesh )
		planeObj = OpenMaya.MObject()
		sel.getDependNode( 0, planeObj )
		fnMesh = OpenMaya.MFnMesh( planeObj )
		fnMesh.setCurrentColorSetName( "cAlpha" )
		converter = IECoreMaya.FromMayaShapeConverter.create( mesh, IECoreScene.MeshPrimitive.staticTypeId() )
		converter['colors'] = True
		m = converter.convert()
		self.assertEqual( m['Cs'].data, IECore.Color3fVectorData( [ imath.Color3f(0), imath.Color3f(1), imath.Color3f(0.8), imath.Color3f(0.5) ] ) )

		# test rgba to rgb conversion
		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/colouredPlane.ma", force = True, open = True )
		sel = OpenMaya.MSelectionList()
		sel.add( mesh )
		planeObj = OpenMaya.MObject()
		sel.getDependNode( 0, planeObj )
		fnMesh = OpenMaya.MFnMesh( planeObj )
		fnMesh.setCurrentColorSetName( "cRGBA" )
		converter = IECoreMaya.FromMayaShapeConverter.create( mesh, IECoreScene.MeshPrimitive.staticTypeId() )
		converter['colors'] = True
		m = converter.convert()
		self.assertEqual( m['Cs'].data, IECore.Color3fVectorData( [ imath.Color3f( 1, 1, 0 ), imath.Color3f( 1, 1, 1 ), imath.Color3f( 0, 1, 1 ), imath.Color3f( 0, 1, 0 ) ] ) )

	def testExtraColors( self ):

		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/colouredPlane.ma", force = True, open = True )

		mesh = "pPlaneShape1"
		converter = IECoreMaya.FromMayaShapeConverter.create( mesh, IECoreScene.MeshPrimitive.staticTypeId() )
		converter['extraColors'] = True
		m = converter.convert()
		self.assertEqual( m['cAlpha_Cs'].data, IECore.FloatVectorData( [ 0, 1, 0.8, 0.5 ] ) )
		self.assertEqual( m['cRGB_Cs'].data, IECore.Color3fVectorData( [ imath.Color3f(1,0,0), imath.Color3f(0), imath.Color3f(0,0,1), imath.Color3f(0,1,0) ] ) )
		self.assertEqual( m['cRGBA_Cs'].data, IECore.Color4fVectorData( [ imath.Color4f( 1, 1, 0, 0.5 ), imath.Color4f( 1, 1, 1, 1 ), imath.Color4f( 0, 1, 1, 1 ), imath.Color4f( 0, 1, 0, 0.5 ) ] ) )

	def testCreases( self ):

		cube = maya.cmds.polyCube()[0]
		fnMesh = OpenMaya.MFnMesh( IECoreMaya.dagPathFromString(cube) )

		cornerIds = OpenMaya.MUintArray()
		cornerIds.append( 5 )

		cornerSharpnesses = OpenMaya.MDoubleArray()
		cornerSharpnesses.append( 10 )

		fnMesh.setCreaseVertices( cornerIds, cornerSharpnesses )

		edgeIds = OpenMaya.MUintArray()
		edgeIds.append( 0 )
		edgeIds.append( 1 )

		edgeSharpnesses = OpenMaya.MDoubleArray()
		edgeSharpnesses.append( 1 )
		edgeSharpnesses.append( 5 )

		fnMesh.setCreaseEdges( edgeIds, edgeSharpnesses )

		# store which vertices belong to the affected edges

		util = OpenMaya.MScriptUtil()

		vertices = []
		for edgeId in edgeIds :

			edgeVertices = util.asInt2Ptr()
			fnMesh.getEdgeVertices( edgeId, edgeVertices )

			vertices.append( util.getInt2ArrayItem( edgeVertices, 0, 0 ) )
			vertices.append( util.getInt2ArrayItem( edgeVertices, 0, 1 ) )

		# convert and test

		cube = maya.cmds.listRelatives( cube, shapes=True )[0]

		converter = IECoreMaya.FromMayaMeshConverter.create( cube, IECoreScene.MeshPrimitive.staticTypeId() )
		cortexCube = converter.convert()

		self.assertEqual( cortexCube.cornerIds(), IECore.IntVectorData( [ 5 ] ) )
		self.assertEqual( cortexCube.cornerSharpnesses(), IECore.FloatVectorData( [ 10.0 ] ) )

		self.assertEqual( cortexCube.creaseLengths(), IECore.IntVectorData( [ 2, 2 ] ) )
		self.assertEqual( cortexCube.creaseIds(), IECore.IntVectorData( vertices ) )
		self.assertEqual( cortexCube.creaseSharpnesses(), IECore.FloatVectorData( [ 1, 5 ] ) )

	def testCreasesFromPlug( self ):

		self.writeTestScc()

		maya.cmds.file( new=True, f=True )
		node = maya.cmds.createNode( 'ieSceneShape' )
		maya.cmds.setAttr( node+'.file', FromMayaMeshConverterTest.__testFile, type='string' )
		maya.cmds.setAttr( node+".queryPaths[0]", "/1", type="string")

		# Test mesh coming out of the sceneshape

		converter = IECoreMaya.FromMayaPlugConverter.create( node + ".outObjects[0]" )
		cortexCube = converter.convert()

		self.assertEqual( cortexCube.numFaces(), 6 )  # checking that we got the right mesh

		self.assertEqual( cortexCube.creaseLengths(), IECore.IntVectorData( [ 2, 2, 2 ] ) )
		self.assertEqual( cortexCube.creaseIds(), IECore.IntVectorData( [ 3, 2, 2, 1, 5, 4 ] ) )
		self.assertEqual( cortexCube.creaseSharpnesses(), IECore.FloatVectorData( [ 1, 1, 5 ] ) )

		# Test mesh flowing into the mesh node

		mesh = maya.cmds.createNode( 'mesh' )
		maya.cmds.connectAttr( node+".outObjects[0]", mesh + ".inMesh" )

		converter = IECoreMaya.FromMayaPlugConverter.create( mesh + ".inMesh" )
		cortexCube = converter.convert()

		self.assertEqual( cortexCube.numFaces(), 6 )  # checking that we got the right mesh

		self.assertEqual( cortexCube.creaseLengths(), IECore.IntVectorData( [ 2, 2, 2 ] ) )
		self.assertEqual( cortexCube.creaseIds(), IECore.IntVectorData( [ 3, 2, 2, 1, 5, 4 ] ) )
		self.assertEqual( cortexCube.creaseSharpnesses(), IECore.FloatVectorData( [ 1, 1, 5 ] ) )

	def tearDown( self ) :

		for f in [ FromMayaMeshConverterTest.__testFile ]:
			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
