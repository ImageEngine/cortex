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
import sys
import tempfile
import unittest

import pxr.Usd
import pxr.Sdf

import imath

import IECore
import IECoreScene
import IECoreUSD

class SceneCacheFileFormatTest( unittest.TestCase ) :

	def setUp( self ) :
		self.__temporaryDirectory = None

	def tearDown( self ) :
		if self.__temporaryDirectory is not None :
			IECoreScene.SharedSceneInterfaces.clear()
			shutil.rmtree( self.__temporaryDirectory )

	def temporaryDirectory( self ) :

		if self.__temporaryDirectory is None :
			self.__temporaryDirectory = tempfile.mkdtemp( prefix = "ieCoreUSDTest" )

		return self.__temporaryDirectory

	def _writeScene( self, fileName, transformRootChildren=False, writeInvalidUSDName=False, writeCs=False, writeAttribute=False ):
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )
		s = t.createChild( "s" )

		if writeAttribute:
			u = s.createChild( "u" )
			t.writeAttribute( "user:testColor3fAttribute", IECore.Color3fData( imath.Color3f( [ 0.5, 0.6, 0.1] ) ), 0 )
			t.writeAttribute( "user:testAttribute", IECore.FloatData( 12.0 ), 0 )
			t.writeAttribute( "user:testAttribute", IECore.FloatData( 21.0 ), 1 )
			u.writeAttribute( "user:testAttribute", IECore.FloatData( 24.0 ), 0 )
			t.writeAttribute( "render:testAttribute", IECore.FloatData( 12.0 ), 0 )
			t.writeAttribute( "render:testAttribute", IECore.FloatData( 21.0 ), 1 )
			u.writeAttribute( "render:testAttribute", IECore.FloatData( 24.0 ), 0 )
			meshU = IECoreScene.MeshPrimitive.createSphere( 1 )
			meshU["customConstant"] = IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Constant,
				IECore.FloatData( 12.0 )
			)
			u.writeObject( meshU, 1.0 )

		if writeInvalidUSDName:
			invalidUSDName = m.createChild( "r-invalid" )

		if transformRootChildren:
			t.writeTransform( IECore.M44dData(imath.M44d().translate(imath.V3d( 2, 0, 0 ))), 2.0 )

		boxA = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		if writeCs:
			boxA["Cs"] = IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.Color3fVectorData(
				[ imath.Color3f( p.x *.5, p.y * 0.5, p.z * 0.5 ) for p in boxA["P"].data ]
				)
			)
		s.writeObject( boxA, 1.0 )


		boxB = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 1 ), imath.V3f( 2 ) ) )
		if writeCs:
			boxB["Cs"] = IECoreScene.PrimitiveVariable(
				IECoreScene.PrimitiveVariable.Interpolation.Vertex,
				IECore.Color3fVectorData(
				[ imath.Color3f( p.z * .5, p.y * .5, p.x * .5 ) for p in boxA["P"].data ]
				)
			)
		s.writeObject( boxB, 2.0 )
		# need to delete all the SceneCache references to finalise the file
		del m, t, s,
		if writeInvalidUSDName:
			del invalidUSDName

	def testRegistration( self ):
		for supportedExtension in ( "scc", "lscc" ):
			self.assertTrue( supportedExtension in pxr.Sdf.FileFormat.FindAllFileFormatExtensions() )

	def testConstruction( self ) :
		fileName = os.path.join( self.temporaryDirectory(), "testUSDConstruction.scc" )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )
		del m, t

		stage = pxr.Usd.Stage.Open( fileName )
		self.assertTrue( stage )

	def testTimeSetting( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDTimeSettings.scc" )

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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDHierarchy.scc" )
		self._writeScene( fileName, writeInvalidUSDName=True )

		# root children
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		cortexRoot = root.GetPrimAtPath( "/{}".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		self.assertTrue( "t" in cortexRoot.GetChildrenNames() )
		self.assertTrue( IECoreUSD.SceneCacheDataAlgo.toInternalName( "r-invalid" ) in cortexRoot.GetChildrenNames() )

		# root child
		tUsd = cortexRoot.GetChild( "t" )
		# check t has no Object
		self.assertTrue( root.GetPrimAtPath( "/{}/t".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) ).GetTypeName() == "Xform" )
		self.assertEqual( tUsd.GetChildrenNames(), ["s"] )

		# read object
		prim = root.GetPrimAtPath( "/{}/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		self.assertTrue( prim )
		self.assertEqual( prim.GetTypeName(), "Mesh" )

		# grand child path
		sUsd = tUsd.GetChild( "s" )
		self.assertEqual( sUsd.GetPath().pathString, "/{}/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		self.assertFalse( sUsd.GetChildrenNames() )

		# test round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportHierarchy.scc" )
		stage.Export( exportPath )

		# root child
		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		self.assertEqual( sorted( scene.childNames() ), sorted( ["t", "r-invalid"] ) )

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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDBounds.scc" )
		self._writeScene( fileName )

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "{}/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

		extent = prim.GetAttribute( "extent" )
		self.assertEqual( extent.Get( 24.0 ), pxr.Vt.Vec3fArray( 2, [ pxr.Gf.Vec3f(0, 0, 0), pxr.Gf.Vec3f(1, 1, 1) ] ) )
		self.assertEqual( extent.Get( 48.0 ), pxr.Vt.Vec3fArray( 2, [ pxr.Gf.Vec3f(1, 1, 1), pxr.Gf.Vec3f(2, 2, 2) ] ) )

		# round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportBounds.scc" )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		location = scene.scene( ["t", "s"] )

		self.assertEqual( location.readBound( 1 ), imath.Box3d( imath.V3d( 0 ), imath.V3d( 1 ) ) )
		self.assertEqual( location.readBound( 2 ), imath.Box3d( imath.V3d( 1 ), imath.V3d( 2 ) ) )

	def testTransform( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDTranform.scc" )
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
		prim = root.GetPrimAtPath( "/{}/t".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

		transform = prim.GetAttribute( "xformOp:transform" )
		self.assertEqual( transform.Get( 24.0 ), pxr.Gf.Matrix4d(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0 ) )
		self.assertEqual( transform.Get( 48.0 ), pxr.Gf.Matrix4d(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0, 0.0, 0.0, 1.0 ) )

		# round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportTransform.scc" )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		t = scene.child( "t" )
		self.assertEqual( t.readTransformAsMatrix( 1 ).translation(), transformA )
		self.assertEqual( t.readTransformAsMatrix( 2 ).translation(), transformB )

	def testAnimatedVisibility( self ):
		fps = 24.0

		fileName = os.path.join( "test", "IECoreScene", "data", "animatedVisibility.scc" )
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()

		parentPrim = root.GetPrimAtPath( "/{}/parentVisibility".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
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
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportVisibility.scc" )
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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDTags.scc" )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		t = m.createChild( "t" )
		s = t.createChild( "s" )
		t.writeTags( ["t1", "all", "asset-(12)"] )
		s.writeTags( ["s1", "all", "asset-(12)"] )
		del m, t, s

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()

		tagPrim = root.GetPrimAtPath( "/{}/t".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		self.assertTrue( tagPrim )

		tags = { "t1" : [ "/t" ], "s1" : [ "/t/s" ], "all" : [ "/t", "/t/s" ], "asset-(12)" : [ "/t", "/t/s" ] }
		for tag, paths in tags.items():
			internalPaths = [ "/{}{}".format( IECoreUSD.SceneCacheDataAlgo.internalRootName(), x ) for x in paths ]
			relationship = tagPrim.GetRelationship( "collection:{}:includes".format( IECoreUSD.SceneCacheDataAlgo.toInternalName( tag ) ) )
			sdfPaths = [ pxr.Sdf.Path( x ) for x in internalPaths ]
			for target in relationship.GetTargets():
				self.assertTrue( target in sdfPaths )

		# round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportTags.scc" )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
		for tag, paths in tags.items():
			for path in paths:
				child = scene.scene( IECoreScene.SceneInterface.stringToPath( path ) )
				self.assertTrue( tag in child.readTags() )

	def testPointsAndTopology( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDPointsAndTopology.scc" )
		self._writeScene( fileName )

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "/{}/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

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
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportPointsAndTopology.scc" )
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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDDefaultPrimVars.scc" )
		self._writeScene( fileName, writeCs=True )

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "{}/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

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

		# cs
		displayColor = prim.GetAttribute( "primvars:displayColor" )
		displayColorData = displayColor.Get( 24.0 )
		self.assertEqual(
			displayColorData[0],
			pxr.Gf.Vec3f(0.0, 0.0, 0.0)
		)

		self.assertEqual(
			displayColorData[5],
			pxr.Gf.Vec3f(0.5, 0.5, 0.5)
		)

		self.assertEqual(
			displayColorData[7],
			pxr.Gf.Vec3f(0.0, 0.5, 0.5)
		)

		# round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportDefaultPrimVars.scc" )
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

		# Cs
		csData = mesh["Cs"].data
		self.assertEqual( csData[0], imath.Color3f( 0, 0, 0 ) )
		self.assertEqual( csData[5], imath.Color3f( 0.5, 0.5, 0.5 ) )
		self.assertEqual( csData[7], imath.Color3f( 0, 0.5, 0.5 ) )

	def testCustomPrimvars( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDCustomPrimVar.scc" )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		box = m.createChild( "box" )
		mesh = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( 0 ), imath.V3f( 1 ) ) )
		vertexSize = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		uniformSize = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		v = IECore.V3fVectorData( [ imath.Color3f( 12, 12, 12 ) ] * vertexSize, IECore.GeometricData.Interpretation.Point )
		mesh["customPoint"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, v )
		mesh["customIndexedInt"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.IntVectorData( [12] ), IECore.IntVectorData( [ 0 ] * uniformSize ) )
		self.assertTrue( mesh.arePrimitiveVariablesValid() )

		# Test a shallow copy of a primvar which causes IECore.Object to write a reference into the IndexedIO file
		mesh["Pref"] = mesh["P"]

		box.writeObject( mesh, 1.0 )
		del m, box

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "{}/box".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

		customPoint = prim.GetAttribute( "primvars:customPoint" )
		# All primvars match the primvar schema, and therefore are not custom
		self.assertFalse( customPoint.GetMetadata( "custom" ) )

		# type
		self.assertEqual( customPoint.GetMetadata( "typeName" ), pxr.Sdf.ValueTypeNames.Point3fArray )

		# interpolation
		self.assertEqual( customPoint.GetMetadata( "interpolation" ), "vertex" )

		# value
		self.assertEqual( customPoint.Get( 24.0 ), pxr.Vt.Vec3fArray( vertexSize, pxr.Gf.Vec3f( 12 ) ) )

		# indices
		indices = prim.GetAttribute( "primvars:customIndexedInt:indices" )
		self.assertTrue( indices )
		self.assertEqual( indices.Get( 24.0 ), pxr.Vt.IntArray( [ 0 ] * uniformSize ) )

		# uniform interpolation
		customIndexedInt = prim.GetAttribute( "primvars:customIndexedInt" )
		self.assertEqual( customIndexedInt.GetMetadata( "interpolation" ), pxr.UsdGeom.Tokens.uniform )

		# Pref
		Pref = prim.GetAttribute( "primvars:Pref" )
		self.assertIsNotNone( Pref )
		self.assertFalse( Pref.GetMetadata( "custom" ) )
		self.assertEqual( Pref.GetMetadata( "typeName" ), pxr.Sdf.ValueTypeNames.Point3fArray )
		self.assertEqual( Pref.Get( 24.0 ), pxr.UsdGeom.Mesh( prim ).GetPointsAttr().Get( 24.0 ) )
		self.assertEqual( Pref.GetMetadata( "interpolation" ), pxr.UsdGeom.Tokens.vertex )

		# round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportCustomPrimVar.scc" )
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
		self.assertEqual( customPoint.data, IECore.V3fVectorData( [ imath.V3f( 12.0 ) ] * vertexSize, IECore.GeometricData.Interpretation.Point ) )

		# indices
		self.assertEqual( mesh["customIndexedInt"].indices, IECore.IntVectorData( [ 0 ] * uniformSize ) )

		# copy
		self.assertEqual( mesh["Pref"], mesh["P"] )

	def testCornersAndCreases( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDCornerAndCreases.scc" )
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
		prim = root.GetPrimAtPath( "{}/plane".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

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
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportCornerAndCreases.scc" )
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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDPointsPrimitive.scc" )
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
		prim = root.GetPrimAtPath( "/{}/points".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

		self.assertEqual( prim.GetTypeName(), "Points" )

		points = prim.GetAttribute( "points" )
		self.assertEqual( points.Get( 24.0 )[0], pxr.Gf.Vec3f( 1, 2, 3 ) )
		self.assertEqual( points.Get( 24.0 )[1], pxr.Gf.Vec3f( 12, 13, 14 ) )

		widths = prim.GetAttribute( "widths" )
		self.assertAlmostEqual( widths.Get( 24.0 )[0], 0.12 )
		self.assertEqual( widths.Get( 24.0 )[1], 1.0 )

		# round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportPointsPrimitive.scc" )
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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDCurvesPrimitive.scc" )
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
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportPointsPrimitive.scc" )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( exportPath )

		for baseName, base in basis.items():
			isBezier = baseName == "bezier"
			# USD
			primPathUSD = "{}/curves/{}".format( IECoreUSD.SceneCacheDataAlgo.internalRootName(), baseName )
			primPathCortex = "/curves/{}".format( baseName )
			prim = root.GetPrimAtPath( primPathUSD )

			# Cortex
			location = scene.scene( IECoreScene.SceneInterface.stringToPath( primPathCortex ) )
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

		fileName = os.path.join( self.temporaryDirectory(), "testUSDCamera.scc" )
		m = IECoreScene.SceneCache( fileName, IECore.IndexedIO.OpenMode.Write )
		cam = m.createChild( "cam" )
		cam.writeObject( c, 1.0 )
		del m, cam

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()
		prim = root.GetPrimAtPath( "{}/cam".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

		# focal
		self.assertEqual( prim.GetAttribute( "focalLength" ).Get( 24.0 ), 35 )

		# aperture
		self.assertEqual( prim.GetAttribute( "horizontalAperture" ).Get( 24.0 ), 36 )
		self.assertEqual( prim.GetAttribute( "verticalAperture" ).Get( 24.0 ), 24 )

		# aperture offset
		self.assertEqual( prim.GetAttribute( "horizontalApertureOffset" ).Get( 24.0 ), 1 )
		self.assertEqual( prim.GetAttribute( "verticalApertureOffset" ).Get( 24.0 ), -1 )

		# round trip
		exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportCamera.scc" )
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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDLinked.scc" )
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
		prim = root.GetPrimAtPath( "/{}/linked".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

		self.assertTrue( root.GetChildrenNames(), ["linked", "lChild", "cortex_tags"] )

		self.assertTrue( prim.GetChild( "t" ) )
		self.assertTrue( prim.GetPrimAtPath( "t/s" ) )

		self.assertTrue( root.GetPrimAtPath( "/{}/linked/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) ) )
		self.assertTrue( root.GetPrimAtPath( "/{}/lChild/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) ) )

		self.assertFalse( prim.GetPrimAtPath( "t/s" ).GetChildrenNames() )

		# link root from another scene
		meshPrim = root.GetPrimAtPath( "/{}/linked/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		pointsAttr = meshPrim.GetAttribute( "points" )
		pointsData = pointsAttr.Get( 24.0 )

		self.assertTrue( len( pointsData ) == 8 )
		self.assertEqual( pointsData[0], pxr.Gf.Vec3f( 0, 0, 0 ) )
		self.assertEqual( pointsData[7], pxr.Gf.Vec3f( 0, 1, 1 ) )

		# link child from another scene
		meshPrim = root.GetPrimAtPath( "/{}/lChild/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		pointsAttr = meshPrim.GetAttribute( "points" )
		pointsData = pointsAttr.Get( 24.0 )

		self.assertTrue( len( pointsData ) == 8 )
		self.assertEqual( pointsData[0], pxr.Gf.Vec3f( 0, 0, 0 ) )
		self.assertEqual( pointsData[7], pxr.Gf.Vec3f( 0, 1, 1 ) )

	def testSceneWrite( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDWrite.scc" )
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

		exportPath = os.path.join( self.temporaryDirectory(), "testExport.scc" )
		stage.Export( exportPath )
		self.assertTrue( os.path.exists( exportPath ) )

		# root
		layer = pxr.Sdf.Layer.FindOrOpen( linkFileName )

		# check reference
		self.assertTrue( layer.GetExternalReferences()[0] == fileName )

		exportPath = os.path.join( self.temporaryDirectory(), "testExportLinked.lscc" )
		layer.Export( exportPath )
		self.assertTrue( os.path.exists( exportPath ) )

		readLayer = pxr.Sdf.Layer.FindOrOpen( exportPath )
		linkedSceneRead = IECoreScene.SharedSceneInterfaces.get( exportPath )

		for linkPrimPath, linkedRootPath in ( ( "/linked", "/linked/t" ), ( "/lChild", "/lChild" ) ):
			internalPrimPath = "/{}{}".format( IECoreUSD.SceneCacheDataAlgo.internalRootName(), linkPrimPath )
			# USD
			prim = readLayer.GetPrimAtPath( internalPrimPath )
			reference = prim.referenceList.prependedItems[0]

			# Cortex
			locationPath = IECoreScene.SceneInterface.stringToPath( linkedRootPath )
			location = linkedSceneRead.scene( locationPath )

			linkLocationPath = IECoreScene.SceneInterface.stringToPath( linkPrimPath )
			linkLocation = linkedSceneRead.scene( linkLocationPath )

			# transform
			if linkPrimPath == "/linked":
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
			self.assertEqual( linkLocation.readAttribute( IECoreScene.LinkedScene.fileNameLinkAttribute, 0 ).value, fileName )

			# link root
			if linkPrimPath != "/linked":
				self.assertEqual( reference.primPath, pxr.Sdf.Path( "/{}/t".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) ) )
				self.assertEqual( linkLocation.readAttribute( IECoreScene.LinkedScene.rootLinkAttribute, 0 ), IECore.InternedStringVectorData( ["t"] ) )
			else:
				self.assertEqual( reference.primPath, pxr.Sdf.Path( "/{}".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) ) )
				self.assertEqual( linkLocation.readAttribute( IECoreScene.LinkedScene.rootLinkAttribute, 0 ), IECore.InternedStringVectorData( [] ) )

	def testMultipleReference( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDMultipleReference.usd" )

		linkedFileNameA = os.path.join( self.temporaryDirectory(), "testUSDMultipleReferenceA.scc" )
		self._writeScene( linkedFileNameA, transformRootChildren=True )

		linkedFileNameB = os.path.join( self.temporaryDirectory(), "testUSDMultipleReferenceB.scc" )
		self._writeScene( linkedFileNameB, transformRootChildren=True )

		root = pxr.Usd.Stage.CreateNew( fileName )
		xform = pxr.UsdGeom.Xform.Define( root, "/child" )
		xformPrim = root.GetPrimAtPath( "/child" )

		xformPrim.GetReferences().AddReference( linkedFileNameA )
		xformPrim.GetReferences().AddReference( linkedFileNameB, "/{}/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

		root.GetRootLayer().Save()
		del root, xformPrim

		# round trip
		layer = pxr.Sdf.Layer.FindOrOpen( fileName )
		exportPath = os.path.join( self.temporaryDirectory(), "testExportLinked.lscc" )

		with IECore.CapturingMessageHandler() as mh :
			layer.Export( exportPath )

		self.assertEqual( len( mh.messages ), 1 )
		self.assertEqual( mh.messages[0].level, IECore.Msg.Level.Warning )
		self.assertEqual( mh.messages[0].context, "SceneCacheFileFormat::writeLocation" )
		self.assertEqual( mh.messages[0].message, 'Unsupported multiple reference at location "/child", writing only the first reference.' )

	def testReferenceUnsupportedExtension( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testUSDUnsupportedExtension.usd" )
		linkedFileNameA = os.path.join( self.temporaryDirectory(), "testUSDUnsupportedExtension.scc" )
		self._writeScene( linkedFileNameA, transformRootChildren=True )

		unSupportedExtensionFilePath = os.path.join( self.temporaryDirectory(), "testUSDUnsupportedExtension.foo" )
		os.rename( linkedFileNameA, unSupportedExtensionFilePath )

		root = pxr.Usd.Stage.CreateNew( fileName )
		xform = pxr.UsdGeom.Xform.Define( root, "/child" )
		xformPrim = root.GetPrimAtPath( "/child" )

		xformPrim.GetReferences().AddReference( unSupportedExtensionFilePath )
		root.GetRootLayer().Save()
		del root, xformPrim

		# round trip
		layer = pxr.Sdf.Layer.FindOrOpen( fileName )
		exportPath = os.path.join( self.temporaryDirectory(), "testExportUnsupportedExtension.lscc" )

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
		fileName = os.path.join( self.temporaryDirectory(), "testUSDTimeOffsetLinked.scc" )
		self._writeScene( fileName, transformRootChildren=True )

		for start, end in ( ( 0.5, 2.0 ), ( 1.5, 2.5 ), ( 1.0, 1.5) ):
			linkedScene = IECoreScene.SharedSceneInterfaces.get( fileName )
			linkFileName = "{}/testUSDTimeOffsetLink_{}_{}.lscc".format( self.temporaryDirectory(), start, end )
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
			offsetPrim = stage.GetPrimAtPath( "/{}/linkedOffset".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
			valueClip = pxr.Usd.ClipsAPI( offsetPrim )
			clip = valueClip.GetClips()["default"]

			self.assertEqual(
				[ os.path.normcase( str( p ) ) for p in clip["assetPaths"] ],
				[ os.path.normcase( str( p ) ) for p in pxr.Sdf.AssetPathArray( [ pxr.Sdf.AssetPath( fileName, fileName ) ] ) ]
			)
			self.assertEqual( clip["primPath"], "/{}".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )

			self.assertEqual( clip["times"], pxr.Vt.Vec2dArray( [ pxr.Gf.Vec2d( start * fps, fps ), pxr.Gf.Vec2d( end * fps, 2.0 * fps ) ] ) )
			self.assertEqual( clip["active"], pxr.Vt.Vec2dArray( [ pxr.Gf.Vec2d( start * fps, 0 ) ] ) )
			# required so we don't reuse the previous iteration stage
			del stage

			## test round trip
			layer = pxr.Sdf.Layer.FindOrOpen( linkFileName )
			exportPath = os.path.join( self.temporaryDirectory(), "testUSDExportTimeOffsetLink.lscc" )
			layer.Export( exportPath )
			del layer

			# root child
			scene = IECoreScene.SharedSceneInterfaces.get( exportPath )
			location = scene.scene( IECoreScene.SceneInterface.stringToPath( "/linkedOffset/t" ) )

			# verify we have the same linkAttribute as original
			timeLinkSamples = location.numAttributeSamples( IECoreScene.LinkedScene.timeLinkAttribute )
			self.assertEqual( location.attributeSampleTime( IECoreScene.LinkedScene.timeLinkAttribute, 0 ), start )
			self.assertEqual( location.attributeSampleTime( IECoreScene.LinkedScene.timeLinkAttribute, 1 ), end )

	def testPerFrameWrite( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testPerFrameWrite.scc" )
		frames = list( range( 1, 25 ) )
		for i in frames:
			stage = pxr.Usd.Stage.CreateInMemory()
			sphere = pxr.UsdGeom.Sphere.Define( stage, "/Sphere" )
			attr = sphere.GetRadiusAttr()
			attr.Set( 10 , 1 )
			attr.Set( 100, 24 )
			args = {
				"perFrameWrite" : "1",
				"currentFrame"  : str( i ),
				"firstFrame"    : str( frames[0] ),
				"lastFrame"     : str( frames[-1] ),
			}

			stage.Export( fileName, args=args )

		scene = IECoreScene.SharedSceneInterfaces.get( fileName )
		sphere = scene.child( "Sphere" )

		self.assertTrue( sphere )
		self.assertTrue( isinstance( sphere.readObject( 0 ), IECoreScene.SpherePrimitive ) )

		self.assertEqual( sphere.readBound( 0 ), imath.Box3d( imath.V3d( -10 ), imath.V3d( 10 ) ) )
		self.assertNotEqual( sphere.readBound( 0.5 ), sphere.readBound( 0 ) )
		self.assertEqual( sphere.readBound( 1 ), imath.Box3d( imath.V3d( -100 ), imath.V3d( 100 ) ) )

	def testWriteTimeSamplesWitinFrameRange( self ):
		fileName = os.path.join( self.temporaryDirectory(), "testTimeSampleWithinFrameRange.scc" )
		frames = list( range( 1, 25 ) )
		stage = pxr.Usd.Stage.CreateInMemory()
		sphere = pxr.UsdGeom.Sphere.Define( stage, "/Sphere" )
		attr = sphere.GetRadiusAttr()
		attr.Set( 10 , 1 )
		attr.Set( 100, 24 )
		args = {
			"firstFrame"    : str( frames[0] ),
			"lastFrame"     : "12",
		}

		stage.Export( fileName, args=args )

		scene = IECoreScene.SharedSceneInterfaces.get( fileName )
		sphere = scene.child( "Sphere" )

		self.assertEqual( sphere.readBound( 0 ), imath.Box3d( imath.V3d( -10 ), imath.V3d( 10 ) ) )
		self.assertEqual( sphere.readBound( 1 ), sphere.readBound( 0.5 ) )

	def testCustomAttribute( self ):
		fileName = "{}/testCustomAttribute.scc".format( self.temporaryDirectory() )
		self._writeScene( fileName, writeAttribute=True )

		# root
		stage = pxr.Usd.Stage.Open( fileName )
		root = stage.GetPseudoRoot()

		tPrim = stage.GetPrimAtPath( "/{}/t".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		self.assertFalse( tPrim.GetAttribute( "scene:visible" ) )

		attribute = tPrim.GetAttribute( "primvars:user:testAttribute" )
		renderAttribute = tPrim.GetAttribute( "primvars:testAttribute" )

		def testCustomAttribute( attribute ):
			self.assertTrue( attribute )

			self.assertFalse( attribute.GetMetadata( "cortex_isConstantPrimitiveVariable" ) )
			self.assertEqual( attribute.GetMetadata( "interpolation" ), "constant" )
			self.assertTrue( attribute.GetMetadata( "custom" ) )

			if attribute.GetName() in ( "primvars:user:testAttribute", "primvars:testAttribute" ):
				self.assertEqual( attribute.Get( 0 ), 12.0 )
				self.assertEqual( attribute.Get( 24 ), 21.0 )
			else:
				self.assertEqual( attribute.Get( 0 ), 1 )

		testCustomAttribute( attribute )

		# inherited
		sPrim = stage.GetPrimAtPath( "/{}/t/s".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		self.assertFalse( sPrim.GetAttribute( "sceneInterface:animatedObjectPrimVars" ) )
		primVarAPI = pxr.UsdGeom.PrimvarsAPI( sPrim )
		primVar = primVarAPI.FindPrimvarWithInheritance( "primvars:user:testAttribute" )
		testCustomAttribute( primVar.GetAttr() )

		renderPrimVar = primVarAPI.FindPrimvarWithInheritance( "primvars:testAttribute" )
		testCustomAttribute( renderPrimVar.GetAttr() )

		# local override
		uPrim = stage.GetPrimAtPath( "/{}/t/s/u".format( IECoreUSD.SceneCacheDataAlgo.internalRootName() ) )
		attribute = uPrim.GetAttribute( "primvars:user:testAttribute" )
		self.assertEqual( attribute.Get( 0 ), 24.0 )
		self.assertEqual( attribute.Get( 24 ), 24.0 )

		renderAttribute = uPrim.GetAttribute( "primvars:testAttribute" )
		self.assertEqual( renderAttribute.Get( 0 ), 24.0 )
		self.assertEqual( renderAttribute.Get( 24 ), 24.0 )

		# constant prim var
		attribute = uPrim.GetAttribute( "primvars:customConstant" )

		self.assertTrue( attribute.GetMetadata( "cortex_isConstantPrimitiveVariable" ) )
		self.assertEqual( attribute.GetMetadata( "interpolation" ), "constant" )
		# All primvars match the primvar schema, and therefore are not custom
		self.assertFalse( attribute.GetMetadata( "custom" ) )

		# round trip
		exportPath = "{}/testUSDExportCustomAttribute.scc".format( self.temporaryDirectory() )
		stage.Export( exportPath )

		scene = IECoreScene.SharedSceneInterfaces.get( fileName )

		tLocation = scene.child( "t" )
		for attributeName in ( "user:testAttribute", "render:testAttribute" ):
			self.assertAlmostEqual( tLocation.readAttribute( attributeName, 0 ), IECore.FloatData( 12.0 ) )
			self.assertAlmostEqual( tLocation.readAttribute( attributeName, 1 ), IECore.FloatData( 21.0 ) )

		uLocation = scene.scene( [ "t", "s", "u" ] )
		for attributeName in ( "user:testAttribute", "render:testAttribute" ):
			self.assertAlmostEqual( uLocation.readAttribute( attributeName, 0 ), IECore.FloatData( 24.0 ) )
			self.assertAlmostEqual( uLocation.readAttribute( attributeName, 1 ), IECore.FloatData( 24.0 ) )

		mesh = uLocation.readObject( 0 )
		primitiveVariable = mesh["customConstant"]
		self.assertTrue( primitiveVariable )
		self.assertEqual( primitiveVariable.interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )

if __name__ == "__main__":
	unittest.main()
