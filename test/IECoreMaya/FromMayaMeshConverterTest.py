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

import IECore
import IECoreMaya

class FromMayaMeshConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere, IECore.TypeId.MeshPrimitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )
		
		converter = IECoreMaya.FromMayaShapeConverter.create( sphere, IECore.TypeId.Primitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )
		
		converter = IECoreMaya.FromMayaObjectConverter.create( sphere )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaObjectConverter.create( sphere, IECore.TypeId.MeshPrimitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )
		
		converter = IECoreMaya.FromMayaObjectConverter.create( sphere, IECore.TypeId.Primitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

	def testConstructor( self ) :
	
		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaMeshConverter( sphere )
		
		m = converter.convert()
		
		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )

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

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["points"].getTypedValue(), True )
		m = converter.convert()
		self.assert_( "P" in m )
		self.assertEqual( m["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		converter["points"].setTypedValue( False )
		self.assert_( not "P" in converter.convert() )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["normals"].getTypedValue(), True )
		m = converter.convert()
		self.assert_( "N" in m )
		self.assertEqual( m["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		converter["normals"].setTypedValue( False )
		self.assert_( not "N" in converter.convert() )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["st"].getTypedValue(), True )
		self.assert_( "s" in converter.convert() )
		self.assert_( "t" in converter.convert() )
		converter["st"].setTypedValue( False )
		self.assert_( not "s" in converter.convert() )
		self.assert_( not "t" in converter.convert() )

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
		self.assertEqual( m.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 50 )
		self.assertEqual( m.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 42 )
		self.assertEqual( m["P"].data.size(), 42 )
		self.assertEqual( m["N"].data.size(), 180 )
		self.assertEqual( m["s"].data.size(), 180 )
		self.assertEqual( m["t"].data.size(), 180 )
		self.assert_( m["P"].data == converter.points() )
		self.assertEqual( m["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( m["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assert_( m["N"].data == converter.normals() )
		self.assert_( m["s"].data == converter.s( "map1" ) )
		self.assert_( m["t"].data == converter.t( "map1" ) )

		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ), IECore.V3f( 1.0001 ) ).contains( m.bound() ) )
		self.assert_( m.bound().contains( IECore.Box3f( IECore.V3f( -0.90 ), IECore.V3f( 0.90 ) ) ) )

	def testSpaces( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		maya.cmds.move( 1, 2, 3, sphere )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )

		self.assertEqual( converter["space"].getNumericValue(), IECoreMaya.FromMayaCurveConverter.Space.Object )
		m = converter.convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ), IECore.V3f( 1.0001 ) ).contains( m.bound() ) )

		converter["space"].setNumericValue( IECoreMaya.FromMayaShapeConverter.Space.World )
		m = converter.convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ) + IECore.V3f( 1, 2, 3 ), IECore.V3f( 1.0001 ) + IECore.V3f( 1, 2, 3 ) ).contains( m.bound() ) )

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

		self.assert_( IECore.polygonNormal( loop ).equalWithAbsError( IECore.V3f( 0, 1, 0 ), 0.0001 ) )

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

		maya.cmds.addAttr( plane, attributeType="float", longName="delightDouble", defaultValue=1 )
		maya.cmds.addAttr( plane, dataType="doubleArray", longName="delightDoubleArray" )
		maya.cmds.setAttr( plane + ".delightDoubleArray", ( 10, 11, 12, 13 ), type="doubleArray" )

		converter = IECoreMaya.FromMayaShapeConverter.create( plane, IECore.MeshPrimitive.staticTypeId() )
		m = converter.convert()

		self.assertEqual( len( m.keys() ), 7 )
		self.assertEqual( m["Double"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m["Double"].data, IECore.FloatData( 1 ) )
		self.assertEqual( m["DoubleArray"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
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
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ) + IECore.V3f( 1, 2, 3 ), IECore.V3f( 1.0001 ) + IECore.V3f( 1, 2, 3 ) ).contains( m.bound() ) )

	def testSharedSTIndices( self ) :
	
		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/twoTrianglesWithSharedUVs.ma", force = True, open = True )
		
		mesh = IECoreMaya.FromMayaShapeConverter.create( "pPlaneShape1" ).convert()
		
		self.failUnless( "stIndices" in mesh )
		self.assertEqual( mesh["stIndices"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["stIndices"].data, IECore.IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		
	def testSplitSTIndices( self ) :
			
		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/twoTrianglesWithSplitUVs.ma", force = True, open = True )
		
		mesh = IECoreMaya.FromMayaShapeConverter.create( "pPlaneShape1" ).convert()
		
		self.failUnless( "stIndices" in mesh )
		self.assertEqual( mesh["stIndices"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["stIndices"].data, IECore.IntVectorData( [ 0, 1, 5, 2, 4, 3 ] ) )	

	def testExtraSTs( self ) :
	
		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]
		
		converter = IECoreMaya.FromMayaShapeConverter.create( plane, IECore.MeshPrimitive.staticTypeId() )
		m = converter.convert()
		
		self.assert_( "s" in m )
		self.assert_( "t" in m )
		self.assert_( "stIndices" in m )
		self.assert_( "map1_s" not in m )
		self.assert_( "map1_t" not in m )
		self.assert_( "map1Indices" not in m )
		
		maya.cmds.polyUVSet( plane, copy=True, uvSet="map1", newUVSet="map2" )
		
		m = converter.convert()
				
		self.assert_( "s" in m )
		self.assert_( "t" in m )
		self.assert_( "stIndices" in m )
		self.assert_( "map1_s" not in m )
		self.assert_( "map1_t" not in m )
		self.assert_( "map1Indices" not in m )
		self.assert_( "map2_s" in m )
		self.assert_( "map2_t" in m )
		self.assert_( "map2Indices" in m )

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
		
		result = IECoreMaya.FromMayaMeshConverter( mayaMesh ).convert()
		
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 760 )
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.FaceVarying ), 2280 )
		
		self.assertEqual( coreMesh["s"], result["s"] )
		self.assertEqual( coreMesh["t"], result["t"] )
		
		for i in range( 0, 7 ) :
			self.assertEqual( coreMesh[ "testUVSet%d_s" % i ], result[ "testUVSet%d_s" %  i ] )
			self.assertEqual( coreMesh[ "testUVSet%d_t" %  i ], result[ "testUVSet%d_t" %  i ] )
	
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
		converter = IECoreMaya.FromMayaShapeConverter.create( mesh, IECore.MeshPrimitive.staticTypeId() )
		converter['colors'] = True
		m = converter.convert()
		self.assertEqual( m['Cs'].data, IECore.Color3fVectorData( [ IECore.Color3f(0), IECore.Color3f(1), IECore.Color3f(0.8), IECore.Color3f(0.5) ] ) )
		self.assertEqual( converter.colors("cAlpha",True), m['Cs'].data )

		# test rgba to rgb conversion
		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/colouredPlane.ma", force = True, open = True )
		sel = OpenMaya.MSelectionList()
		sel.add( mesh )
		planeObj = OpenMaya.MObject()
		sel.getDependNode( 0, planeObj )
		fnMesh = OpenMaya.MFnMesh( planeObj )
		fnMesh.setCurrentColorSetName( "cRGBA" )
		converter = IECoreMaya.FromMayaShapeConverter.create( mesh, IECore.MeshPrimitive.staticTypeId() )
		converter['colors'] = True
		m = converter.convert()
		self.assertEqual( m['Cs'].data, IECore.Color3fVectorData( [ IECore.Color3f( 1, 1, 0 ), IECore.Color3f( 1, 1, 1 ), IECore.Color3f( 0, 1, 1 ), IECore.Color3f( 0, 1, 0 ) ] ) )
		self.assertEqual( converter.colors("cRGBA",True), m['Cs'].data )

	def testExtraColors( self ):

		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/colouredPlane.ma", force = True, open = True )

		mesh = "pPlaneShape1"
		converter = IECoreMaya.FromMayaShapeConverter.create( mesh, IECore.MeshPrimitive.staticTypeId() )
		converter['extraColors'] = True
		m = converter.convert()
		self.assertEqual( m['cAlpha_Cs'].data, IECore.FloatVectorData( [ 0, 1, 0.8, 0.5 ] ) )
		self.assertEqual( converter.colors("cAlpha"), m['cAlpha_Cs'].data )
		self.assertEqual( m['cRGB_Cs'].data, IECore.Color3fVectorData( [ IECore.Color3f(1,0,0), IECore.Color3f(0), IECore.Color3f(0,0,1), IECore.Color3f(0,1,0) ] ) )
		self.assertEqual( converter.colors("cRGB"), m['cRGB_Cs'].data )
		self.assertEqual( m['cRGBA_Cs'].data, IECore.Color4fVectorData( [ IECore.Color4f( 1, 1, 0, 0.5 ), IECore.Color4f( 1, 1, 1, 1 ), IECore.Color4f( 0, 1, 1, 1 ), IECore.Color4f( 0, 1, 0, 0.5 ) ] ) )
		self.assertEqual( converter.colors("cRGBA"), m['cRGBA_Cs'].data )
		
if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
