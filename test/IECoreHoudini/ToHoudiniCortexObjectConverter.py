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

import hou
import IECore
import IECoreScene
import IECoreHoudini
import unittest
import os

class TestToHoudiniCortexObjectConverter( IECoreHoudini.TestCase ) :

	__testScene = "test/converterTest.hip"

	def mesh( self ) :

		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [ 1, 5, 4, 0, 2, 6, 5, 1, 3, 7, 6, 2, 0, 4, 7, 3, 2, 1, 0, 3, 5, 6, 7, 4 ] )
		mesh = IECoreScene.MeshPrimitive( vertsPerFace, vertexIds )

		intRange = range( 1, 25 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %d!" % x for x in intRange ] )

		detailInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Constant
		pointInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Vertex
		primitiveInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Uniform
		vertexInterpolation = IECoreScene.PrimitiveVariable.Interpolation.FaceVarying

		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
		], IECore.GeometricData.Interpretation.Point )
		mesh["P"] = IECoreScene.PrimitiveVariable( pointInterpolation, pData )
		mesh["floatPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, floatVectorData[:8] )
		mesh["color3fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, color3fVectorData[:8] )
		mesh["stringPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, stringVectorData[:8], IECore.IntVectorData( range( 0, 8 ) ) )

		mesh["floatPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, floatVectorData[:6] )
		mesh["color3fPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, color3fVectorData[:6] )
		mesh["stringPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, stringVectorData[:6], IECore.IntVectorData( range( 0, 6 ) ) )

		mesh["floatVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, floatVectorData )
		mesh["color3fVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, color3fVectorData )
		mesh["stringVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, stringVectorData, IECore.IntVectorData( range( 0, 24 ) ) )

		return mesh

	def points( self ) :

		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
			IECore.V3f( 8 ), IECore.V3f( 9 ), IECore.V3f( 10 ), IECore.V3f( 11 ),
		] )

		points = IECoreScene.PointsPrimitive( pData )

		intRange = range( 1, 13 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %d!" % x for x in intRange ] )

		detailInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Constant
		uniformInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Uniform
		pointInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Vertex

		points["floatPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, floatVectorData[:1] )
		points["color3fPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, color3fVectorData[:1] )
		points["stringPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, stringVectorData[:1], IECore.IntVectorData( [ 0 ] ) )

		points["floatPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, floatVectorData )
		points["color3fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, color3fVectorData )
		points["stringPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, stringVectorData, IECore.IntVectorData( range( 0, 12 ) ) )

		return points

	def ints( self ) :

		return IECore.IntVectorData( range( 0, 10 ) )

	def emptySop( self ) :

		return hou.node( "/obj" ).createNode( "geo", run_init_scripts=False ).createNode( "null" )

	def meshSop( self ) :

		box = hou.node( "/obj" ).createNode( "geo", run_init_scripts=False ).createNode( "box" )
		facet = box.createOutputNode( "facet" )
		facet.parm( "postnml" ).set(True)
		name = facet.createOutputNode( "name" )
		name.parm( "name1" ).set( "realGeo" )
		return name

	def pointsSop( self ) :

		return self.meshSop().createOutputNode( "scatter" )

	def testCreateConverter( self )  :

		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( self.mesh() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCortexObjectConverter ) ) )

		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( self.points() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCortexObjectConverter ) ) )

		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( self.ints() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCortexObjectConverter ) ) )

	def testFactory( self ) :

		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.ints() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCortexObjectConverter ) ) )

		# meshes still get polygons
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.mesh() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPolygonsConverter ) ) )

		self.assertTrue( IECore.TypeId.Object in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )

	def verifySop( self, sop, obj, name = "" ) :

		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 1 )
		self.assertEqual( prims[0].type(), hou.primType.Custom )
		self.assertEqual( prims[0].vertices()[0].point().number(), 0 )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop, name )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCortexObjectConverter ) ) )
		self.assertEqual( obj, converter.convert() )

	def testConversionIntoEmptySop( self ) :

		mesh = self.mesh()
		sop = self.emptySop()

		self.assertTrue( IECoreHoudini.ToHoudiniCortexObjectConverter( mesh ).convert( sop ) )
		self.verifySop( sop, mesh )

		points = self.points()
		self.assertTrue( IECoreHoudini.ToHoudiniCortexObjectConverter( points ).convert( sop ) )
		self.verifySop( sop, points )

		ints = self.ints()
		self.assertTrue( IECoreHoudini.ToHoudiniCortexObjectConverter( ints ).convert( sop ) )
		self.verifySop( sop, ints )

	def testConversionIntoExistingSop( self ) :

		mesh = self.mesh()
		sop = self.meshSop()

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertNotEqual( mesh, converter.convert() )

		self.assertTrue( IECoreHoudini.ToHoudiniCortexObjectConverter( mesh ).convert( sop, False ) )
		self.verifySop( sop, mesh )

		points = self.points()
		self.assertTrue( IECoreHoudini.ToHoudiniCortexObjectConverter( points ).convert( sop ) )
		self.verifySop( sop, points )

		ints = self.ints()
		self.assertTrue( IECoreHoudini.ToHoudiniCortexObjectConverter( ints ).convert( sop ) )
		self.verifySop( sop, ints )

	def verifyAppendedSop( self, sop, obj, orig, name = "", origName = "realGeo", numAppends = 1 ) :

		origNumPoints = orig.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		origNumPolys = orig.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform )

		prims = sop.geometry().prims()
		self.assertEqual( len(prims), origNumPolys + numAppends )
		origPrims = prims[:-numAppends]
		for prim in origPrims :
			self.assertEqual( prim.type(), hou.primType.Polygon )
			self.assertEqual( prim.attribValue( "name" ), origName )

		newPrims = prims[len(prims) - numAppends:]
		for i in range( 0, len(newPrims) ) :
			prim = newPrims[i]
			self.assertEqual( prim.type(), hou.primType.Custom )
			self.assertEqual( prim.vertices()[0].point().number(), origNumPoints + i )

		self.assertEqual( newPrims[-1].attribValue( "name" ), name )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop, name )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCortexObjectConverter ) ) )
		self.assertEqual( obj, converter.convert() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop, origName )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertEqual( orig, converter.convert() )

	def testAppendingIntoExistingSop( self ) :

		mesh = self.mesh()
		sop = self.meshSop()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, mesh )

		self.assertTrue( not sop.isHardLocked() )
		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )
		converter["name"].setTypedValue( "myMesh" )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.verifyAppendedSop( sop, mesh, orig, "myMesh" )

		sop.setHardLocked( False )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertEqual( orig, converter.convert() )

	def testAppendingIntoLockedSop( self ) :

		mesh = self.mesh()
		sop = self.meshSop()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, mesh )

		sop.setHardLocked( True )
		self.assertTrue( sop.isHardLocked() )
		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )
		converter["name"].setTypedValue( "myMesh" )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.verifyAppendedSop( sop, mesh, orig, "myMesh" )

		sop.setHardLocked( False )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertEqual( orig, converter.convert() )

	def testSaveLoad( self ) :

		hou.hipFile.clear( suppress_save_prompt=True )

		mesh = self.mesh()
		sop = self.emptySop()
		sopPath = sop.path()

		self.assertTrue( IECoreHoudini.ToHoudiniCortexObjectConverter( mesh ).convert( sop ) )
		self.verifySop( sop, mesh )

		hou.hipFile.save( TestToHoudiniCortexObjectConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniCortexObjectConverter.__testScene )

		newSop = hou.node( sopPath )
		self.assertTrue( newSop.isHardLocked() )
		self.verifySop( newSop, mesh )

	def testMultipleConversions( self ) :

		mesh = self.mesh()
		points = self.points()
		ints = self.ints()
		sop = self.meshSop()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, mesh )

		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )
		converter["name"].setTypedValue( "myMesh" )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.verifyAppendedSop( sop, mesh, orig, "myMesh" )

		self.assertTrue( sop.isHardLocked() )
		converter["src"].setValue( points )
		converter["name"].setTypedValue( "myPoints" )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.verifyAppendedSop( sop, points, orig, "myPoints", numAppends = 2 )

		self.assertTrue( sop.isHardLocked() )
		converter["src"].setValue( ints )
		converter["name"].setTypedValue( "myInts" )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.verifyAppendedSop( sop, ints, orig, "myInts", numAppends = 3 )

	def testObjectWasDeleted( self ) :

		mesh = self.mesh()
		sop = self.emptySop()

		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		self.verifySop( sop, mesh )

		del mesh

		result = IECoreHoudini.FromHoudiniCortexObjectConverter( sop ).convert()
		self.verifySop( sop, result )

	def testName( self ) :

		mesh = self.mesh()
		sop = self.emptySop()
		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )

		# unnamed unless we set the parameter
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sop.geometry().findPrimAttrib( "name" ), None )

		converter["name"].setTypedValue( "testObject" )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testObject" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testObject" ]), 1 )

		# blindData still works for backwards compatibility
		mesh.blindData()["name"] = IECore.StringData( "blindMesh" )
		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "blindMesh" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "blindMesh" ]), 1 )

		# name parameter takes preference over blindData
		converter["name"].setTypedValue( "testObject" )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testObject" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testObject" ]), 1 )

	def testAttributeFilter( self ) :

		mesh = self.mesh()
		sop = self.emptySop()

		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		result = IECoreHoudini.FromHoudiniCortexObjectConverter( sop ).convert()
		self.assertEqual( result.keys(), [ 'P', 'color3fPoint', 'color3fPrim', 'color3fVert', 'floatPoint', 'floatPrim', 'floatVert', 'stringPoint', 'stringPrim', 'stringVert' ] )

		converter.parameters()["attributeFilter"].setTypedValue( "P *3f*" )
		self.assertTrue( converter.convert( sop ) )
		result = IECoreHoudini.FromHoudiniCortexObjectConverter( sop ).convert()
		self.assertEqual( result.keys(), [ 'P', 'color3fPoint', 'color3fPrim', 'color3fVert' ] )

		converter.parameters()["attributeFilter"].setTypedValue( "* ^color* ^string*" )
		self.assertTrue( converter.convert( sop ) )
		result = IECoreHoudini.FromHoudiniCortexObjectConverter( sop ).convert()
		self.assertEqual( result.keys(), [ 'P', 'floatPoint', 'floatPrim', 'floatVert' ] )

		# verify From filter works as well
		fromConverter = IECoreHoudini.FromHoudiniCortexObjectConverter( sop )
		fromConverter.parameters()["attributeFilter"].setTypedValue( "P *Prim" )
		result = fromConverter.convert()
		self.assertEqual( result.keys(), [ 'P', 'floatPrim' ] )

		# verify we can filter uvs
		mesh = IECoreScene.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		IECoreScene.TriangulateOp()( input=mesh, copyInput=False )
		IECoreScene.MeshNormalsOp()( input=mesh, copyInput=False )
		mesh["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6, IECore.GeometricData.Interpretation.Color ) )
		mesh["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 4 ) )
		mesh["Pref"] = mesh["P"]

		# have to filter the source attrs
		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( mesh )
		converter.parameters()["attributeFilter"].setTypedValue( "* ^uv  ^pscale ^rest" )
		self.assertTrue( converter.convert( sop ) )
		result = IECoreHoudini.FromHoudiniCortexObjectConverter( sop ).convert()
		self.assertEqual( result.keys(), [ 'Cs', 'N', 'P', 'Pref', 'width' ] )

		converter.parameters()["attributeFilter"].setTypedValue( "* ^uv ^width ^Pref" )
		self.assertTrue( converter.convert( sop ) )
		result = IECoreHoudini.FromHoudiniCortexObjectConverter( sop ).convert()
		self.assertEqual( result.keys(), [ 'Cs', 'N', 'P' ] )

		# verify non-primitives do not break
		converter = IECoreHoudini.ToHoudiniCortexObjectConverter( IECore.IntData( 1 ) )
		converter.parameters()["attributeFilter"].setTypedValue( "* ^uv ^pscale ^rest" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( IECoreHoudini.FromHoudiniCortexObjectConverter( sop ).convert(), IECore.IntData( 1 ) )

	def tearDown( self ) :

		if os.path.isfile( TestToHoudiniCortexObjectConverter.__testScene ) :
			os.remove( TestToHoudiniCortexObjectConverter.__testScene )

if __name__ == "__main__":
    unittest.main()
