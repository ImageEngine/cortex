##########################################################################
#
#  Copyright (c) 2021, Image Engine Design Inc. All rights reserved.
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

import os
import shutil
import tempfile
import unittest


import pxr

import imath

import IECore
import IECoreScene

class SceneCacheFileFormatTest( unittest.TestCase ) :

	def setUp( self ) :
		self.__temporaryDirectory = None

	def tearDown( self ) :
		if self.__temporaryDirectory is not None :
			shutil.rmtree( self.__temporaryDirectory )

	def temporaryDirectory( self ) :

		if self.__temporaryDirectory is None :
			self.__temporaryDirectory = tempfile.mkdtemp( prefix = "ieCoreUSDTest" )

		return self.__temporaryDirectory

	def _writeScene( self, fileName, transformRootChildren=False ):
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )
		s = t.createChild( "s" )

		if transformRootChildren:
			t.writeTransform( IECore.M44dData(imath.M44d().translate(imath.V3d( 2, 0, 0 ))), 2.0 )
		
		boxA = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		s.writeObject( boxA, 1.0 )

		boxB = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 1 ), imath.V3f( 2 ) ) )
		s.writeObject( boxB, 2.0 )
		# need to delete all the SceneCache references to finalise the file
		del m, t, s

	def testRegistration( self ):
		for supportedExtension in ( "scc", "lscc" ):
			self.assertTrue( supportedExtension in pxr.Sdf.FileFormat.FindAllFileFormatExtensions() )

	def testConstruction( self ) :
		fileName = "{}/testUSDConstruction.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )
		del m, t

		stage = pxr.Usd.Stage.Open( fileName )
		self.assertTrue( stage )

	def testTimeSetting( self ):
		fileName = "{}/testUSDTimeSettings.scc".format( self.temporaryDirectory() )

		for sampleType in ( "transform", "attribute", "object" ):
			m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
			t = m.createChild( "t" )
			if sampleType == "transform":
				t.writeTransform( IECore.M44dData(imath.M44d().translate(imath.V3d( 1, 0, 0 ))), 1.0 )
				t.writeTransform( IECore.M44dData(imath.M44d().translate(imath.V3d( 2, 0, 0 ))), 2.0 )
			elif sampleType == "attribute":
				t.writeAttribute( IECoreScene.SceneInterface.visibilityName, IECore.BoolData( True ), 1.0 )
				t.writeAttribute( IECoreScene.SceneInterface.visibilityName, IECore.BoolData( False ), 2.0 )
			else:
				boxA = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
				t.writeObject( boxA, 1.0 )

				boxB = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 1 ), imath.V3f( 2 ) ) )
				t.writeObject( boxB, 2.0 )

			del m, t

			# root
			stage = pxr.Usd.Stage.Open( fileName )
			root = stage.GetPseudoRoot()

			metadata = root.GetAllMetadata()

			self.assertEqual( metadata["startTimeCode"], 24.0 )
			self.assertEqual( metadata["endTimeCode"], 48.0 )

			self.assertEqual( metadata["timeCodesPerSecond"], 24.0 )

	def testHierarchy( self ):
		fileName = "{}/testUSDHierarchy.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName )

		# root children
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		self.assertTrue( "t" in root.GetChildrenNames() )

		# root child
		tUsd = root.GetChild( "t" )
		# check t has no Object
		self.assertTrue( root.GetPrimAtPath( "/t" ).GetTypeName() == "Xform" )
		self.assertEqual( tUsd.GetChildrenNames(), ["s"] )

		# read object
		prim = root.GetPrimAtPath( "/t/s" )
		self.assertTrue( prim )
		self.assertEqual( prim.GetTypeName(), "Mesh" )

		# grand child path
		sUsd = tUsd.GetChild( "s" )
		self.assertEqual( sUsd.GetPath().pathString, "/t/s" )
		self.assertFalse( sUsd.GetChildrenNames() )

		# test round trip
		exportPath = "{}/testUSDExportHierarchy.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		# root child
		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		self.assertEqual( scene.childNames(), ["t"] )

		# t child names
		t = scene.child( "t" )
		self.assertEqual( t.childNames(), ["s"] )
		# check t has no object
		self.assertRaises( RuntimeError, t.readObject, 0 )

		s = t.child( "s" )
		mesh = s.readObject( 0 )
		self.assertTrue( mesh )
		self.assertEqual( mesh.typeName(), "MeshPrimitive" )

		# grand child path
		self.assertEqual( s.path(), ["t", "s"] )
		self.assertFalse( s.childNames() )

	def testBound( self ):
		fileName = "{}/testUSDBounds.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName )

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/t/s" )

		extent = prim.GetAttribute( "extent" )
		self.assertEqual( extent.Get( 24.0 ), pxr.Vt.Vec3fArray( 2, [ pxr.Gf.Vec3f(0, 0, 0), pxr.Gf.Vec3f(1, 1, 1) ] ) )
		self.assertEqual( extent.Get( 48.0 ), pxr.Vt.Vec3fArray( 2, [ pxr.Gf.Vec3f(1, 1, 1), pxr.Gf.Vec3f(2, 2, 2) ] ) )

		# round trip
		exportPath = "{}/testUSDExportBounds.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.scene( ["t", "s"] )

		self.assertEqual( location.readBound( 1 ), imath.Box3d( imath.V3d( 0 ), imath.V3d( 1 ) ) )
		self.assertEqual( location.readBound( 2 ), imath.Box3d( imath.V3d( 1 ), imath.V3d( 2 ) ) )

	def testTransform( self ):
		fileName = "{}/testUSDTranform.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )
		transformA = imath.V3d( 1, 0, 0 )
		transformB = imath.V3d( 2, 0, 0 )
		t.writeTransform( IECore.M44dData( imath.M44d().translate( transformA ) ), 1.0 )
		t.writeTransform( IECore.M44dData( imath.M44d().translate( transformB ) ), 2.0 )
		del m, t

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/t" )

		transform = prim.GetAttribute( "xformOp:transform" )
		self.assertEqual( transform.Get( 24.0 ), pxr.Gf.Matrix4d(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0 ) )
		self.assertEqual( transform.Get( 48.0 ), pxr.Gf.Matrix4d(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0, 0.0, 0.0, 1.0 ) )

		# round trip
		exportPath = "{}/testUSDExportTransform.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		t = scene.child( "t" )
		self.assertEqual( t.readTransformAsMatrix( 1 ).translation(), transformA )
		self.assertEqual( t.readTransformAsMatrix( 2 ).translation(), transformB )

	def testAnimatedVisibility( self ):
		fps = 24.0

		fileName = "test/IECoreScene/data/animatedVisibility.scc".format( self.temporaryDirectory() )
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()

		parentPrim = root.GetPrimAtPath( "/parentVisibility" )
		parentVisibility = parentPrim.GetAttribute( "visibility" )

		# visible
		self.assertEqual( parentVisibility.Get( 1011 ), "inherited" )

		# invisible
		self.assertEqual( parentVisibility.Get( 1012 ), "invisible" )
		
		# still invisible
		self.assertEqual( parentVisibility.Get( 1021 ), "invisible" )

		# visible again
		self.assertEqual( parentVisibility.Get( 1022 ), "inherited" )

		# inherited
		inheritedVisibility = parentPrim.GetChild( "inherited" ).GetAttribute( "visibility" )

		# always inherited
		for frame in ( 1011, 1012, 1021, 1022 ):
			self.assertEqual( inheritedVisibility.Get( frame ), "inherited" )

		# round trip
		exportPath = "{}/testUSDExportVisibility.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		parent = scene.child( "parentVisibility" )

		# visible
		self.assertEqual( parent.readAttribute( IECoreScene.SceneInterface.visibilityName, 1011 / fps ).value, True )

		# invisible
		self.assertEqual( parent.readAttribute( IECoreScene.SceneInterface.visibilityName, 1012 / fps ).value, False )
		
		# still invisible
		self.assertEqual( parent.readAttribute( IECoreScene.SceneInterface.visibilityName, 1021 / fps ).value, False )

		# visible again
		self.assertEqual( parent.readAttribute( IECoreScene.SceneInterface.visibilityName, 1022 / fps ).value, True )

		inherited = parent.child( "inherited" )

		# always visible? weird...
		for frame in ( 1011, 1012, 1021, 1022 ):
			self.assertEqual( inherited.readAttribute( IECoreScene.SceneInterface.visibilityName, frame / fps ).value, True )

	def testTagsLoadedAsCollections( self ):
		# includes
		fileName = "{}/testUSDTags.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )
		s = t.createChild( "s" )
		t.writeTags( ["t1", "all"] )
		s.writeTags( ["s1", "all"] )
		del m, t, s
		
		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()

		tagPrim = root.GetChild( "t" )
		self.assertTrue( tagPrim )

		tags = { "t1" : [ "/t" ], "s1" : [ "/t/s" ], "all" : [ "/t", "/t/s" ] }
		for tag, paths in tags.items():
			relationship = tagPrim.GetRelationship( "collection:{}:includes".format( tag ) )
			sdfPaths = map( lambda x: pxr.Sdf.Path( x ), paths )
			for target in relationship.GetTargets():
				self.assertTrue( target in sdfPaths )

		# round trip
		exportPath = "{}/testUSDExportTags.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		for tag, paths in tags.items():
			for path in paths:
				child = scene.scene( IECoreScene.SceneInterface.stringToPath( path ) )
				self.assertTrue( tag in child.readTags() )

	def testPointsAndTopology( self ):
		fileName = "{}/testUSDPointsAndTopology.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName )

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/t/s" )

		points = prim.GetAttribute( "points" )

		self.assertEqual(
			points.Get( 24.0 ),
			pxr.Vt.Vec3fArray(
				8,
				(
					pxr.Gf.Vec3f(0.0, 0.0, 0.0),
					pxr.Gf.Vec3f(1.0, 0.0, 0.0),
					pxr.Gf.Vec3f(1.0, 1.0, 0.0),
					pxr.Gf.Vec3f(0.0, 1.0, 0.0),
					pxr.Gf.Vec3f(1.0, 0.0, 1.0),
					pxr.Gf.Vec3f(1.0, 1.0, 1.0),
					pxr.Gf.Vec3f(0.0, 0.0, 1.0),
					pxr.Gf.Vec3f(0.0, 1.0, 1.0)
				)
			)
		)

		faceVertexCounts = prim.GetAttribute( "faceVertexCounts" )
		self.assertEqual(
			faceVertexCounts.Get( 24.0 ),
			pxr.Vt.IntArray(6, (4, 4, 4, 4, 4, 4))
		)

		faceVertexIndices = prim.GetAttribute( "faceVertexIndices" )
		vertexIds = (3, 2, 1, 0, 1, 2, 5, 4, 4, 5, 7, 6, 6, 7, 3, 0, 2, 3, 7, 5, 0, 1, 4, 6)
		self.assertEqual(
			faceVertexIndices.Get( 24.0 ),
			pxr.Vt.IntArray(24, vertexIds )
		)

		# round trip
		exportPath = "{}/testUSDExportPointsAndTopology.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.scene( ["t", "s"] )
		mesh = location.readObject( 1.0 )

		pointsData = mesh["P"].data
		self.assertEqual( len( pointsData ), 8 )

		self.assertEqual( pointsData[0], imath.V3f( 0 ) )
		self.assertEqual( pointsData[7], imath.V3f( 0, 1, 1 ) )

		self.assertEqual( mesh.verticesPerFace, IECore.IntVectorData( [ 4 ] * 6 ) )

		self.assertEqual( mesh.vertexIds, IECore.IntVectorData( vertexIds ) )

	def testDefaultPrimVars( self ):
		fileName = "{}/testUSDDefaultPrimVars.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName )

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/t/s" )

		# orientation
		orientation = prim.GetAttribute( "orientation" )
		self.assertEqual( orientation.Get( 24.0 ), "rightHanded" )

		# normal
		normals = prim.GetAttribute( "normals" )
		normalsData = normals.Get( 24.0 )
		self.assertEqual(
			normalsData[0],
			pxr.Gf.Vec3f(0.0, 0.0, -1.0)
		)
			
		self.assertEqual(
			normalsData[8],
			pxr.Gf.Vec3f(0.0, 0.0, 1.0)
		)
			
		self.assertEqual(
			normalsData[16],
			pxr.Gf.Vec3f(0.0, 1.0, 0.0)
		)
		
		self.assertEqual(
			normalsData[23],
			pxr.Gf.Vec3f(0.0, -1.0, 0.0)
		)

		# uv
		uvs = prim.GetAttribute( "primvars:st" )
		uvsData = uvs.Get( 24.0 )
		self.assertEqual(
			uvsData[0],
			pxr.Gf.Vec2f(0.375, 0.0)
		)
			
		self.assertEqual(
			uvsData[8],
			pxr.Gf.Vec2f(0.375, 1.0)
		)
			
		self.assertEqual(
			uvsData[12],
			pxr.Gf.Vec2f(0.125, .25)
		)

		# uv are indexed
		self.assertTrue( prim.GetAttribute( "primvars:st:indices" ) )

		# round trip
		exportPath = "{}/testUSDExportDefaultPrimVars.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.scene( ["t", "s"] )
		mesh = location.readObject( 1.0 )

		# normals
		normalsData = mesh["N"].data
		self.assertEqual( normalsData[0], imath.V3f( 0, 0, -1 ) )
		self.assertEqual( normalsData[8], imath.V3f( 0, 0, 1 ) )
		self.assertEqual( normalsData[16], imath.V3f( 0, 1.0, 0 ) )
		self.assertEqual( normalsData[23], imath.V3f( 0, -1.0, 0 ) )

		# uv
		uvsData = mesh["uv"].data
		self.assertEqual( uvsData[0], imath.V2f( 0.375, 0 ) )
		self.assertEqual( uvsData[8], imath.V2f( 0.375, 1.0 ) )
		self.assertEqual( uvsData[12], imath.V2f( 0.125, 0.25 ) )

		# uv are indexed
		self.assertTrue( mesh["uv"].indices )

	def testCustomPrimvars( self ):
		fileName = "{}/testUSDCustomPrimVar.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		box = m.createChild( "box" )
		mesh = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		vertexSize = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		v = IECore.V3fVectorData( [ imath.Color3f( 12, 12, 12 ) ], IECore.GeometricData.Interpretation.Point )
		mesh["customPoint"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, v )
		mesh["customIndexedInt"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [12] ), IECore.IntVectorData( [ 0 ] * vertexSize ) )

		box.writeObject( mesh, 1.0 )
		del m, box

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/box" )

		customPoint = prim.GetAttribute( "primvars:customPoint" )
		# custom
		self.assertTrue( customPoint.GetMetadata( "custom" ) )

		# type
		self.assertEqual( customPoint.GetMetadata( "typeName" ), pxr.Sdf.ValueTypeNames.Point3fArray )

		# interpolation
		self.assertEqual( customPoint.GetMetadata( "interpolation" ), "vertex" )
		
		# value
		self.assertEqual( customPoint.Get( 24.0 ), pxr.Vt.Vec3fArray( 1, [ pxr.Gf.Vec3f( 12 ) ] ) )

		# indices
		indices = prim.GetAttribute( "primvars:customIndexedInt:indices" )
		self.assertTrue( indices )
		self.assertEqual( indices.Get( 24.0 ), pxr.Vt.IntArray( vertexSize, [ 0 ] * vertexSize ) )

		# uniform interpolation
		customIndexedInt = prim.GetAttribute( "primvars:customIndexedInt" )
		self.assertEqual( customIndexedInt.GetMetadata( "interpolation" ), pxr.UsdGeom.Tokens.uniform )

		# round trip
		exportPath = "{}/testUSDExportCustomPrimVar.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		box = scene.child( "box" )

		mesh = box.readObject( 1.0 )

		customPoint = mesh["customPoint"]
		self.assertTrue( customPoint )

		# type
		self.assertEqual( customPoint.data.typeName(), IECore.V3fVectorData.staticTypeName() )

		# interpolation
		self.assertEqual( customPoint.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		# value
		self.assertEqual( customPoint.data, IECore.V3fVectorData( [ imath.V3f( 12.0 ) ], IECore.GeometricData.Interpretation.Point ) )

		# indices
		self.assertEqual( mesh["customIndexedInt"].indices, IECore.IntVectorData( [0] * vertexSize ) )

	def testCornersAndCreases( self ):
		fileName = "{}/testUSDCornerAndCreases.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		plane = m.createChild( "plane" )
		planeMesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )

		# corners
		ids = IECore.IntVectorData( [ 0 ] )
		sharpnesses = IECore.FloatVectorData( [ 2 ]  )

		planeMesh.setCorners( ids, sharpnesses )

		# creases
		lengths = IECore.IntVectorData( [ 3 ] )
		ids = IECore.IntVectorData( [ 0, 1, 2 ] )
		sharpnesses = IECore.FloatVectorData( [ 4 ] )

		planeMesh.setCreases( lengths, ids, sharpnesses )

		plane.writeObject( planeMesh, 1.0 )
		del m, plane

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/plane" )

		# corner indices
		cornerIds = prim.GetAttribute( "cornerIndices" )
		self.assertEqual( cornerIds.Get( 24.0 )[0], 0 )

		# corner sharpness
		cornerSharpness = prim.GetAttribute( "cornerSharpnesses" )
		self.assertEqual( cornerSharpness.Get( 24.0 )[0], 2 )

		# crease indices
		creaseIds = prim.GetAttribute( "creaseIndices" )
		self.assertEqual( creaseIds.Get( 24.0 ), pxr.Vt.IntArray( 3, [0, 1, 2] ) )

		# crease sharpness
		creaseSharpness = prim.GetAttribute( "creaseSharpnesses" )
		self.assertEqual( creaseSharpness.Get( 24.0 )[0], 4.0 )

		# crease length
		creaseLength = prim.GetAttribute( "creaseLengths" )
		self.assertEqual( creaseLength.Get( 24.0 )[0], 3.0 )

		# round trip
		exportPath = "{}/testUSDExportCornerAndCreases.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.child( "plane" )

		mesh = location.readObject( 1.0 )

		# corner indices
		self.assertEqual( mesh.cornerIds()[0], 0 )

		# corner sharpness
		self.assertEqual( mesh.cornerSharpnesses()[0], 2 )

		# crease indices
		self.assertEqual( mesh.creaseIds(), IECore.IntVectorData( [ 0, 1, 2 ] ) )
		
		# crease sharpness
		self.assertEqual( mesh.creaseSharpnesses()[0], 4.0 )

		# crease length
		self.assertEqual( mesh.creaseLengths()[0], 3.0 )

	def testPointsPrimitive( self ):
		fileName = "{}/testUSDPointsPrimitive.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		points = m.createChild( "points" )

		p = IECoreScene.PointsPrimitive( 2 )
		p["P"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.V3fVectorData( [ imath.V3f( 1, 2, 3 ), imath.V3f( 12, 13, 14 ) ] )
		)
		p["width"] = IECoreScene.PrimitiveVariable(
			IECoreScene.PrimitiveVariable.Interpolation.Vertex,
			IECore.FloatVectorData( [ 0.12, 1.0 ] )
		)
		points.writeObject( p, 1.0 )
		del m, points

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/points" )

		self.assertEqual( prim.GetTypeName(), "Points" )

		points = prim.GetAttribute( "points" )
		self.assertEqual( points.Get( 24.0 )[0], pxr.Gf.Vec3f( 1, 2, 3 ) )
		self.assertEqual( points.Get( 24.0 )[1], pxr.Gf.Vec3f( 12, 13, 14 ) )

		widths = prim.GetAttribute( "widths" )
		self.assertAlmostEqual( widths.Get( 24.0 )[0], 0.12 )
		self.assertEqual( widths.Get( 24.0 )[1], 1.0 )

		# round trip
		exportPath = "{}/testUSDExportPointsPrimitive.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.child( "points" )

		points = location.readObject( 1.0 )
		pointsData = points["P"].data
		
		self.assertEqual( pointsData[0], imath.V3f( 1, 2, 3 ) )
		self.assertEqual( pointsData[1], imath.V3f( 12, 13, 14 ) )

		widthData = points["width"].data

		self.assertAlmostEqual( widthData[0], 0.12 )
		self.assertEqual( widthData[1], 1.0 )

	def testCurvesPrimitive( self ):
		fileName = "{}/testUSDCurvesPrimitive.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		curves = m.createChild( "curves" )

		basis = {
			"bSpline"    : IECore.CubicBasisf.bSpline(),
			"catmullRom" : IECore.CubicBasisf.catmullRom(),
			"linear"     : IECore.CubicBasisf.linear(),
			"bezier"     : IECore.CubicBasisf.bezier(),
			
		}

		children = []
		pointData = [
			imath.V3f( 0, 0, 0 ),
			imath.V3f( 0, 0, 0 ),
			imath.V3f( 0, 0, 0 ),
			imath.V3f( 0, 0.75, 0 ),
			imath.V3f( 0.25, 1, 0 ),
			imath.V3f( 1, 1, 0 ),
			imath.V3f( 1, 1, 0 ),
			imath.V3f( 1, 1, 0 ),
			imath.V3f( 0, 0, 1 ),
			imath.V3f( 0, 0, 1 ),
			imath.V3f( 0, 0, 1 ),
			imath.V3f( 0, 0.75, 1 ),
			imath.V3f( 0.25, 1, 1 ),
			imath.V3f( 1, 1, 1 ),
			imath.V3f( 1, 1, 1 ),
			imath.V3f( 1, 1, 1 )
		]
		# curve basis
		for baseName, base in basis.items():
			isBezier = baseName == "bezier"
			curveSize = [ 8, 8 ] if not isBezier else [ 7, 7 ]
			testObject = IECoreScene.CurvesPrimitive(

						IECore.IntVectorData( curveSize ),
						base,
						False,
						IECore.V3fVectorData(
							pointData if not isBezier else pointData[:-2]
						)
					)
			child = curves.createChild( baseName )
			children.append( child )
			child.writeObject( testObject, 1.0 )

		del m, curves
		for child in children:
			del child
		del children

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()

		# round trip
		exportPath = "{}/testUSDExportPointsPrimitive.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )

		for baseName, base in basis.items():
			isBezier = baseName == "bezier"
			# USD
			primPath = "/curves/{}".format( baseName )
			prim = root.GetPrimAtPath( primPath )

			# Cortex
			location = scene.scene( IECoreScene.SceneInterface.stringToPath( primPath ) )
			curve = location.readObject( 1.0 )

			self.assertEqual( prim.GetTypeName(), "BasisCurves" )
			self.assertEqual( curve.typeName(), "CurvesPrimitive" )

			if baseName == "linear":
				self.assertTrue( prim.GetAttribute( "basis" ).Get( 24.0 ) == None )
			else:
				self.assertTrue( prim.GetAttribute( "basis" ).Get( 24.0 ).lower() == baseName.lower() )

			self.assertEqual( curve.basis(), base )

			# points
			points = prim.GetAttribute( "points" ).Get( 24.0 )
			self.assertTrue( len( points ), 14 if isBezier else 16 )
			self.assertEqual( points[0], pxr.Gf.Vec3f( 0, 0, 0 ) )
			self.assertEqual( points[4], pxr.Gf.Vec3f( 0.25, 1, 0 ) )
			if isBezier:
				self.assertEqual( points[13], pxr.Gf.Vec3f( 1, 1, 1 ) )
			else:
				self.assertEqual( points[15], pxr.Gf.Vec3f( 1, 1, 1 ) )

			pointsData = curve["P"].data
			self.assertTrue( len( pointsData ), 14 if isBezier else 16 )
			self.assertEqual( pointsData[0], imath.V3f( 0 ) )
			self.assertEqual( pointsData[4], imath.V3f( 0.25, 1, 0 ) )
			if isBezier:
				self.assertEqual( pointsData[13], imath.V3f( 1 ) )
			else:
				self.assertEqual( pointsData[15], imath.V3f( 1 ) )

	def testCamera( self ):
		c = IECoreScene.Camera()

		c.setProjection("perspective" )
		c.setAperture(imath.V2f( 36, 24 ) )
		c.setApertureOffset(imath.V2f( 1, -1 ) )
		c.setFocalLength( 35 )

		fileName = "{}/testUSDCamera.scc".format( self.temporaryDirectory() )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		cam = m.createChild( "cam" )
		cam.writeObject( c, 1.0 )
		del m, cam

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/cam" )

		# focal
		self.assertEqual( prim.GetAttribute( "focalLength" ).Get( 24.0 ), 35 )

		# aperture
		self.assertEqual( prim.GetAttribute( "horizontalAperture" ).Get( 24.0 ), 36 )
		self.assertEqual( prim.GetAttribute( "verticalAperture" ).Get( 24.0 ), 24 )

		# aperture offset
		self.assertEqual( prim.GetAttribute( "horizontalApertureOffset" ).Get( 24.0 ), 1 )
		self.assertEqual( prim.GetAttribute( "verticalApertureOffset" ).Get( 24.0 ), -1 )

		# round trip
		exportPath = "{}/testUSDExportCamera.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.child( "cam" )

		camera = location.readObject( 1.0 )

		# focal
		self.assertEqual( camera.getFocalLength(), 35 )

		# aperture
		self.assertEqual( camera.getAperture(), imath.V2f( 36, 24 ) )

		# aperture offset
		self.assertEqual( camera.getApertureOffset(), imath.V2f( 1, -1 ) )

	def testLinkedSceneCache( self ):
		fileName = "{}/testUSDLinked.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName )

		linkFileName = "{}/testUSDLinkedScene.lscc".format( self.temporaryDirectory() )
		linkedScene = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Read )
		l = IECoreScene.LinkedScene( linkFileName, IECore.IndexedIO.OpenMode.Write )
		linked = l.createChild( "linked" )
		linked.writeLink( linkedScene )

		lChild = l.createChild( "lChild" )
		s = linkedScene.child( "t" )
		lChild.writeLink( s )
		del l, linked, linkedScene, lChild

		# root
		stage = pxr.Usd.Stage.Open( linkFileName )
		root = stage.GetPseudoRoot()
		prim = root.GetChild( "linked" )

		self.assertTrue( root.GetChildrenNames(), ["linked", "lChild", "cortex_tags"] )

		self.assertTrue( prim.GetChild( "t" ) )
		self.assertTrue( prim.GetPrimAtPath( "t/s" ) )

		self.assertTrue( root.GetPrimAtPath( "/linked/t/s" ) )
		self.assertTrue( root.GetPrimAtPath( "lChild/s" ) )

		self.assertFalse( prim.GetPrimAtPath( "t/s" ).GetChildrenNames() )

		# link root from another scene
		meshPrim = root.GetPrimAtPath( "/linked/t/s" )
		pointsAttr = meshPrim.GetAttribute( "points" )
		pointsData = pointsAttr.Get( 24.0 )

		self.assertTrue( len( pointsData ) == 8 )
		self.assertEqual( pointsData[0], pxr.Gf.Vec3f( 0, 0, 0 ) )
		self.assertEqual( pointsData[7], pxr.Gf.Vec3f( 0, 1, 1 ) )

		# link child from another scene
		meshPrim = root.GetPrimAtPath( "/lChild/s" )
		pointsAttr = meshPrim.GetAttribute( "points" )
		pointsData = pointsAttr.Get( 24.0 )

		self.assertTrue( len( pointsData ) == 8 )
		self.assertEqual( pointsData[0], pxr.Gf.Vec3f( 0, 0, 0 ) )
		self.assertEqual( pointsData[7], pxr.Gf.Vec3f( 0, 1, 1 ) )

	def testSceneWrite( self ):
		fileName = "{}/testUSDWrite.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName, transformRootChildren=True )

		linkFileName = "{}/testUSDLinkedScene.lscc".format( self.temporaryDirectory() )
		linkedScene = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Read )
		l = IECoreScene.LinkedScene( linkFileName, IECore.IndexedIO.OpenMode.Write )
		linked = l.createChild( "linked" )
		linked.writeLink( linkedScene )

		lChild = l.createChild( "lChild" )
		s = linkedScene.child( "t" )
		lChild.writeLink( s )
		del l, linked, linkedScene, lChild

		# root
		stage = pxr.Usd.Stage.Open( fileName )

		exportPath = "{}/testExport.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )
		self.assertTrue( os.path.exists( exportPath ) )

		# root
		layer = pxr.Sdf.Layer.FindOrOpen( linkFileName )

		# check reference
		self.assertTrue( layer.GetExternalReferences()[0] == fileName )

		exportPath = "{}/testExportLinked.lscc".format( self.temporaryDirectory() )
		layer.Export( exportPath )
		self.assertTrue( os.path.exists( exportPath ) )

		readLayer = pxr.Sdf.Layer.FindOrOpen( exportPath )
		linkedSceneRead = IECoreScene.SharedSceneInterfaces.get( exportPath )

		for primPath in ( "/linked/t", "/lChild" ):
			# USD
			prim = readLayer.GetPrimAtPath( primPath )
			reference = prim.referenceList.prependedItems[0]

			# Cortex
			locationPath = IECoreScene.SceneInterface.stringToPath( primPath )
			location = linkedSceneRead.scene( locationPath )

			# transform
			if primPath == "/linked/t":
				self.assertTrue( location.readTransform( 0 ).value.translation() == imath.V3d( 2, 0, 0 ) )
			else:
				self.assertTrue( location.readTransform( 0 ).value.translation() == imath.V3d( 0 ) )

			# read mesh across link
			self.assertEqual( location.childNames(), ["s"] )
			mesh = location.child( "s" ).readObject( 0 )
			self.assertTrue( mesh )
			pointsData = mesh["P"].data
			self.assertEqual( len( pointsData ), 8 )

			self.assertEqual( pointsData[0], imath.V3f( 0 ) )
			self.assertEqual( pointsData[7], imath.V3f( 0, 1, 1 ) )

			# link file path
			self.assertEqual( reference.assetPath, fileName )
			self.assertEqual( location.readAttribute( IECoreScene.LinkedScene.fileNameLinkAttribute, 0 ).value, fileName )

			# link root
			self.assertEqual( reference.primPath, pxr.Sdf.Path( "/t" ) )
			self.assertEqual( location.readAttribute( IECoreScene.LinkedScene.rootLinkAttribute, 0 ), IECore.InternedStringVectorData( ["t"] ) )

	def testMultipleReference( self ):
		fileName = "{}/testUSDMultipleReference.usd".format( self.temporaryDirectory() )

		linkedFileNameA = "{}/testUSDMultipleReferenceA.scc".format( self.temporaryDirectory() )
		self._writeScene( linkedFileNameA, transformRootChildren=True )

		linkedFileNameB = "{}/testUSDMultipleReferenceB.scc".format( self.temporaryDirectory() )
		self._writeScene( linkedFileNameB, transformRootChildren=True )

		root = pxr.Usd.Stage.CreateNew( fileName )
		xform = pxr.UsdGeom.Xform.Define( root, "/child" )
		xformPrim = root.GetPrimAtPath( "/child" )

		xformPrim.GetReferences().AddReference( linkedFileNameA )
		xformPrim.GetReferences().AddReference( linkedFileNameB, "/t/s" )

		root.GetRootLayer().Save()
		del root, xformPrim

		# round trip
		layer = pxr.Sdf.Layer.FindOrOpen( fileName )
		exportPath = "{}/testExportLinked.lscc".format( self.temporaryDirectory() )

		with IECore.CapturingMessageHandler() as mh :
			layer.Export( exportPath )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Warning )
		self.assertEqual( mh.messages[0].context, "SceneCacheFileFormat::writeLocation" )
		self.assertEqual( mh.messages[0].message, 'Unsupported multiple reference at location "/child", writing only the first reference.' )

	def testReferenceUnsupportedExtension( self ):
		fileName = "{}/testUSDUnsupportedExtension.usd".format( self.temporaryDirectory() )
		linkedFileNameA = "{}/testUSDUnsupportedExtension.scc".format( self.temporaryDirectory() )
		self._writeScene( linkedFileNameA, transformRootChildren=True )

		unSupportedExtensionFilePath = "{}/testUSDUnsupportedExtension.foo".format( self.temporaryDirectory() )
		os.rename( linkedFileNameA, unSupportedExtensionFilePath )

		root = pxr.Usd.Stage.CreateNew( fileName )
		xform = pxr.UsdGeom.Xform.Define( root, "/child" )
		xformPrim = root.GetPrimAtPath( "/child" )

		xformPrim.GetReferences().AddReference( unSupportedExtensionFilePath )
		root.GetRootLayer().Save()
		del root, xformPrim

		# round trip
		layer = pxr.Sdf.Layer.FindOrOpen( fileName )
		exportPath = "{}/testExportUnsupportedExtension.lscc".format( self.temporaryDirectory() )

		with IECore.CapturingMessageHandler() as mh :
			layer.Export( exportPath )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Warning )
		self.assertEqual( mh.messages[0].context, "SceneCacheFileFormat::writeLocation" )
		self.assertEqual( mh.messages[0].message, 'Unsupported file extension "{}" for reference at location "/child".'.format( unSupportedExtensionFilePath ) )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.child( "child" )

		self.assertFalse( location.childNames() )

	def testLinksTimeOffset( self ):
		fps = 24.0
		fileName = "{}/testUSDTimeOffsetLinked.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName, transformRootChildren=True )
		
		for start, end in ( ( 0.5, 2.0 ), ( 1.5, 2.5 ), ( 1.0, 1.5) ):
			linkedScene = IECoreScene.SharedSceneInterfaces.get( fileName )
			linkFileName = "{}/testUSDTimeOffsetLink.lscc".format( self.temporaryDirectory() )
			l = IECoreScene.LinkedScene( linkFileName, IECore.IndexedIO.OpenMode.Write )
			linkedOffset = l.createChild( "linkedOffset" )
			linkDataA = linkedOffset.linkAttributeData( linkedScene, 1.0 )
			linkDataB = linkedOffset.linkAttributeData( linkedScene, 2.0 )

			linkedOffset.writeAttribute( IECoreScene.LinkedScene.linkAttribute, linkDataA, start )
			linkedOffset.writeAttribute( IECoreScene.LinkedScene.linkAttribute, linkDataB, end )
			
			noTimeOffset = l.createChild( "linked" )
			noTimeOffset.writeLink( linkedScene )

			del l, linkedOffset, linkDataA, linkDataB, noTimeOffset, linkedScene

			# root
			stage = pxr.Usd.Stage.Open( linkFileName )
			offsetPrim = stage.GetPrimAtPath( "/linkedOffset/t" )
			valueClip = pxr.Usd.ClipsAPI( offsetPrim )
			clip = valueClip.GetClips()["default"]

			self.assertEqual( clip["assetPaths"], pxr.Sdf.AssetPathArray( [pxr.Sdf.AssetPath( fileName, fileName )] ) )
			self.assertEqual( clip["primPath"], "/t" )

			self.assertEqual( clip["times"], pxr.Vt.Vec2dArray( [ pxr.Gf.Vec2d( start * fps, fps ), pxr.Gf.Vec2d( end * fps, 2.0 * fps ) ] ) )
			self.assertEqual( clip["active"], pxr.Vt.Vec2dArray( [ pxr.Gf.Vec2d( start * fps, 0 ) ] ) )
			# required so we don't reuse the previous iteration stage
			del stage

			## test round trip
			layer = pxr.Sdf.Layer.FindOrOpen( linkFileName )
			exportPath = "{}/testUSDExportTimeOffsetLink.lscc".format( self.temporaryDirectory() )
			layer.Export( exportPath )
			del layer

			# root child
			scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
			location = scene.scene( IECoreScene.SceneInterface.stringToPath( "/linkedOffset/t" ) )

			# verify we have the same linkAttribute as original
			timeLinkSamples = location.numAttributeSamples( IECoreScene.LinkedScene.timeLinkAttribute )
			self.assertEqual( location.attributeSampleTime( IECoreScene.LinkedScene.timeLinkAttribute, 0 ), start )
			self.assertEqual( location.attributeSampleTime( IECoreScene.LinkedScene.timeLinkAttribute, 1 ), end )
