##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
#
#  Copyright (c) 2010-2015, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore
import IECoreScene
import IECoreHoudini
import unittest
import os
from six.moves import range

class TestFromHoudiniPolygonsConverter( IECoreHoudini.TestCase ) :

	def createBox( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		box = geo.createNode( "box" )

		return box

	def createTorus( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		torus.parm( "rows" ).set( 10 )
		torus.parm( "cols" ).set( 10 )

		return torus

	def createPoints( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		box = geo.createNode( "box" )
		facet = geo.createNode( "facet" )
		facet.parm("postnml").set(True)
		points = geo.createNode( "scatter" )
		points.parm( "npts" ).set( 5000 )
		facet.setInput( 0, box )
		points.setInput( 0, facet )

		return points

	def createPopNet( self ):
		obj = hou.node( '/obj' )
		geo = obj.createNode("geo", run_init_scripts=False)
		popNet = geo.createNode("dopnet", "popnet" )
		popObject = popNet.createNode( "popobject" )
		popSolver = popObject.createOutputNode( "popsolver" )
		output = popSolver.createOutputNode( "output" )
		output.setDisplayFlag( True )

		return popNet

	# creates a converter
	def testCreateConverter( self )  :
		box = self.createBox()
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( box )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

		return converter

	# creates a converter
	def testFactory( self ) :
		box = self.createBox()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( box )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( box, resultType = IECoreScene.TypeId.MeshPrimitive )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( box, resultType = IECore.TypeId.Parameter )
		self.assertEqual( converter, None )

		self.assertTrue( IECoreScene.TypeId.MeshPrimitive in IECoreHoudini.FromHoudiniGeometryConverter.supportedTypes() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( IECoreScene.TypeId.MeshPrimitive )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( [ IECoreScene.TypeId.MeshPrimitive ] )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

	# performs geometry conversion
	def testDoConversion( self ) :
		converter = self.testCreateConverter()
		result = converter.convert()
		self.assertTrue( result.isInstanceOf( IECoreScene.TypeId.MeshPrimitive ) )

	def testConvertFromHOMGeo( self ) :
		geo = self.createBox().geometry()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

		result = converter.convert()
		self.assertTrue( result.isInstanceOf( IECoreScene.TypeId.MeshPrimitive ) )

		converter2 = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo, IECoreScene.TypeId.MeshPrimitive )
		self.assertTrue( converter2.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

		## \todo: make sure we catch that bad_cast crash

	# convert a mesh
	def testConvertMesh( self ) :

		torus = self.createTorus()
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( torus )
		result = converter.convert()
		self.assertEqual( result.typeId(), IECoreScene.MeshPrimitive.staticTypeId() )

		bbox = result.bound()
		self.assertEqual( bbox.min().x, -1.5 )
		self.assertEqual( bbox.max().x, 1.5 )

		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 100 )
		self.assertEqual( result.numFaces(), 100 )

		self.assertEqual( len( result.verticesPerFace ), 100 )
		for i in range( len( result.verticesPerFace ) ) :
			self.assertEqual( result.verticesPerFace[i], 4 )

		self.assertEqual( len( result.vertexIds ), 400 )
		for i in range( len( result.vertexIds ) ) :
			self.assertTrue( result.vertexIds[i] >= 0 )
			self.assertTrue( result.vertexIds[i] < 100 )

	# test prim/vertex attributes
	def testConvertPrimVertAttributes( self ) :
		torus = self.createTorus()
		geo = torus.parent()

		# add vertex normals
		facet = geo.createNode( "facet", node_name = "add_point_normals" )
		facet.parm("postnml").set(True)
		facet.setInput( 0, torus )

		# add a primitive colour attributes
		primcol = geo.createNode( "primitive", node_name = "prim_colour" )
		primcol.parm("doclr").set(1)
		primcol.parm("diffr").setExpression("rand($PR)")
		primcol.parm("diffg").setExpression("rand($PR+1)")
		primcol.parm("diffb").setExpression("rand($PR+2)")
		primcol.setInput( 0, facet )

		# add a load of different vertex attributes
		vert_f1 = geo.createNode( "attribcreate", node_name = "vert_f1", exact_type_name=True )
		vert_f1.parm("name").set("vert_f1")
		vert_f1.parm("class").set(3)
		vert_f1.parm("value1").setExpression("$VTX*0.1")
		vert_f1.setInput( 0, primcol )

		vert_f2 = geo.createNode( "attribcreate", node_name = "vert_f2", exact_type_name=True )
		vert_f2.parm("name").set("vert_f2")
		vert_f2.parm("class").set(3)
		vert_f2.parm("size").set(2)
		vert_f2.parm("value1").setExpression("$VTX*0.1")
		vert_f2.parm("value2").setExpression("$VTX*0.1")
		vert_f2.setInput( 0, vert_f1 )

		vert_f3 = geo.createNode( "attribcreate", node_name = "vert_f3", exact_type_name=True )
		vert_f3.parm("name").set("vert_f3")
		vert_f3.parm("class").set(3)
		vert_f3.parm("size").set(3)
		vert_f3.parm("value1").setExpression("$VTX*0.1")
		vert_f3.parm("value2").setExpression("$VTX*0.1")
		vert_f3.parm("value3").setExpression("$VTX*0.1")
		vert_f3.setInput( 0, vert_f2 )

		vert_i1 = geo.createNode( "attribcreate", node_name = "vert_i1", exact_type_name=True )
		vert_i1.parm("name").set("vert_i1")
		vert_i1.parm("class").set(3)
		vert_i1.parm("type").set(1)
		vert_i1.parm("value1").setExpression("$VTX*0.1")
		vert_i1.setInput( 0, vert_f3 )

		vert_i2 = geo.createNode( "attribcreate", node_name = "vert_i2", exact_type_name=True )
		vert_i2.parm("name").set("vert_i2")
		vert_i2.parm("class").set(3)
		vert_i2.parm("type").set(1)
		vert_i2.parm("size").set(2)
		vert_i2.parm("value1").setExpression("$VTX*0.1")
		vert_i2.parm("value2").setExpression("$VTX*0.1")
		vert_i2.setInput( 0, vert_i1 )

		vert_i3 = geo.createNode( "attribcreate", node_name = "vert_i3", exact_type_name=True )
		vert_i3.parm("name").set("vert_i3")
		vert_i3.parm("class").set(3)
		vert_i3.parm("type").set(1)
		vert_i3.parm("size").set(3)
		vert_i3.parm("value1").setExpression("$VTX*0.1")
		vert_i3.parm("value2").setExpression("$VTX*0.1")
		vert_i3.parm("value3").setExpression("$VTX*0.1")
		vert_i3.setInput( 0, vert_i2 )

		vert_v3f = geo.createNode( "attribcreate", node_name = "vert_v3f", exact_type_name=True )
		vert_v3f.parm("name").set("vert_v3f")
		vert_v3f.parm("class").set(3)
		vert_v3f.parm("type").set(2)
		vert_v3f.parm("value1").setExpression("$VTX*0.1")
		vert_v3f.parm("value2").setExpression("$VTX*0.1")
		vert_v3f.parm("value3").setExpression("$VTX*0.1")
		vert_v3f.setInput( 0, vert_i3 )

		vertString = geo.createNode( "attribcreate", node_name = "vertString", exact_type_name=True )
		vertString.parm("name").set("vertString")
		vertString.parm("class").set(3)
		vertString.parm("type").set(3)
		vertString.parm("string").set("string $VTX!")
		vertString.setInput( 0, vert_v3f )

		detail_i3 = geo.createNode( "attribcreate", node_name = "detail_i3", exact_type_name=True )
		detail_i3.parm("name").set("detail_i3")
		detail_i3.parm("class").set(0)
		detail_i3.parm("type").set(1)
		detail_i3.parm("size").set(3)
		detail_i3.parm("value1").set(123)
		detail_i3.parm("value2").set(456.789) # can we catch it out with a float?
		detail_i3.parm("value3").set(789)
		detail_i3.setInput( 0, vertString )

		out = geo.createNode( "null", node_name="OUT" )
		out.setInput( 0, detail_i3 )

		# convert it all
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( out )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )

		result = converter.convert()
		self.assertTrue( result.isInstanceOf( IECoreScene.TypeId.MeshPrimitive ) )

		bbox = result.bound()
		self.assertEqual( bbox.min().x, -1.5 )
		self.assertEqual( bbox.max().x, 1.5 )

		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), 100 )
		self.assertEqual( result.numFaces(), 100 )

		self.assertEqual( len( result.verticesPerFace ), 100 )
		for i in range( len( result.verticesPerFace ) ) :
			self.assertEqual( result.verticesPerFace[i], 4 )

		self.assertEqual( len( result.vertexIds ), 400 )
		for i in range( len( result.vertexIds ) ) :
			self.assertTrue( result.vertexIds[i] >= 0 )
			self.assertTrue( result.vertexIds[i] < 100 )

		# test point attributes
		self.assertTrue( "P" in result )
		self.assertEqual( result['P'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result['P'].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result['P'].data.size(), result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertTrue( "N" in result )
		self.assertEqual( result['N'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result['N'].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result['N'].data.size(), result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( result["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		# test detail attributes
		self.assertTrue( "detail_i3" in result )
		self.assertEqual( result['detail_i3'].data.typeId(), IECore.TypeId.V3iData )
		self.assertEqual( result['detail_i3'].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( result['detail_i3'].data.value.x, 123 )
		self.assertEqual( result['detail_i3'].data.value.y, 456 )
		self.assertEqual( result['detail_i3'].data.value.z, 789 )

		# test primitive attributes
		self.assertTrue( "Cs" in result )
		self.assertEqual( result["Cs"].data.typeId(), IECore.TypeId.Color3fVectorData )
		self.assertEqual( result["Cs"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( result["Cs"].data.size(), result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

		for i in range( 0, result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) ) :
			for j in range( 0, 3 ) :
				self.assertTrue( result["Cs"].data[i][j] >= 0.0 )
				self.assertTrue( result["Cs"].data[i][j] <= 1.0 )

		# test vertex attributes
		attrs = [ "vert_f1", "vert_f2", "vert_f3", "vert_i1", "vert_i2", "vert_i3", "vert_v3f" ]
		for a in attrs :
			self.assertTrue( a in result )
			self.assertEqual( result[a].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
			self.assertEqual( result[a].data.size(), result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) )

		# test indexed vertex attributes
		for a in [ "vertString" ] :
			self.assertTrue( a in result )
			self.assertEqual( result[a].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
			self.assertEqual( result[a].indices.size(), result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) )

		self.assertEqual( result["vert_f1"].data.typeId(), IECore.FloatVectorData.staticTypeId() )
		self.assertEqual( result["vert_f2"].data.typeId(), IECore.V2fVectorData.staticTypeId() )
		self.assertEqual( result["vert_f3"].data.typeId(), IECore.V3fVectorData.staticTypeId() )

		for i in range( 0, result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) ) :
			for j in range( 0, 3 ) :
				self.assertTrue( result["vert_f3"].data[i][j] >= 0.0 )
				self.assertTrue( result["vert_f3"].data[i][j] < 0.4 )

		self.assertEqual( result["vert_i1"].data.typeId(), IECore.IntVectorData.staticTypeId() )
		self.assertEqual( result["vert_i2"].data.typeId(), IECore.V2iVectorData.staticTypeId() )
		self.assertEqual( result["vert_i3"].data.typeId(), IECore.V3iVectorData.staticTypeId() )

		for i in range( 0, result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) ) :
			for j in range( 0, 3 ) :
				self.assertEqual( result["vert_i3"].data[i][j], 0 )

		self.assertEqual( result["vert_v3f"].data.typeId(), IECore.V3fVectorData.staticTypeId() )

		self.assertEqual( result["vertString"].data.typeId(), IECore.TypeId.StringVectorData )
		self.assertEqual( result["vertString"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( result["vertString"].data.size(), 4 )
		self.assertEqual( result["vertString"].indices.typeId(), IECore.TypeId.IntVectorData )

		for i in range( 0, result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying ) ) :
			index = result["vertString"].indices[ result.vertexIds[i] ]
			self.assertEqual( result["vertString"].data[ index ], "string %d!" % index )

		self.assertTrue( result.arePrimitiveVariablesValid() )

	def testConvertNull( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		null = geo.createNode( "null" )
		m = IECoreHoudini.FromHoudiniPolygonsConverter( null ).convert()
		self.assertTrue( isinstance( m, IECoreScene.MeshPrimitive ) )
		self.assertEqual( m, IECoreScene.MeshPrimitive() )

	# convert some points
	def testConvertPoints( self ) :
		points = self.createPoints()
		m = IECoreHoudini.FromHoudiniPolygonsConverter( points ).convert()
		self.assertTrue( isinstance( m, IECoreScene.MeshPrimitive ) )
		self.assertEqual( m, IECoreScene.MeshPrimitive() )

	# simple attribute conversion
	def testSetupAttributes( self ) :
		torus = self.createTorus()
		geo = torus.parent()
		attr = geo.createNode( "attribcreate", exact_type_name=True )
		attr.setInput( 0, torus )
		attr.parm("name").set( "test_attribute" )
		attr.parm("type").set(0) # float
		attr.parm("size").set(1) # 1 element
		attr.parm("value1").set(123.456)
		attr.parm("value2").set(654.321)
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( attr )
		result = converter.convert()
		self.assertTrue( "test_attribute" in result.keys() )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		return attr

	# testing point attributes and types
	def testPointAttributes( self ) :
		attr = self.testSetupAttributes()

		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		attr.parm("value1").set(123.456)
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.FloatVectorData )
		self.assertTrue( result["test_attribute"].data[0] > 123.0 )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(1) # integer
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.IntVectorData )
		self.assertEqual( result["test_attribute"].data[0], 123 )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(0) # float
		attr.parm("size").set(2) # 2 elementS
		attr.parm("value2").set(456.789)
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2fVectorData )
		self.assertEqual( result["test_attribute"].data[0], imath.V2f( 123.456, 456.789 ) )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2iVectorData )
		self.assertEqual( result["test_attribute"].data[0], imath.V2i( 123, 456 ) )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(0) # float
		attr.parm("size").set(3) # 3 elements
		attr.parm("value3").set(999.999)
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result["test_attribute"].data[0],imath.V3f( 123.456, 456.789, 999.999 ) )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3iVectorData )
		self.assertEqual( result["test_attribute"].data[0], imath.V3i( 123, 456, 999 ) )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set( 3 ) # string
		attr.parm( "string" ).setExpression("'string %06d!' % pwd().curPoint().number()", hou.exprLanguage.Python)
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.StringVectorData )
		self.assertEqual( result["test_attribute"].data[10], "string 000010!" )
		self.assertEqual( result["test_attribute"].data.size(), 100 )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result["test_attribute"].indices[10], 10 )
		self.assertEqual( result["test_attribute"].indices.size(), 100 )
		self.assertTrue( result.arePrimitiveVariablesValid() )

	# testing detail attributes and types
	def testDetailAttributes( self ) :
		attr = self.testSetupAttributes()
		attr.parm("class").set(0) # detail attribute

		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		attr.parm("value1").set(123.456)
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.FloatData )
		self.assertTrue( result["test_attribute"].data > IECore.FloatData( 123.0 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(1) # integer
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.IntData )
		self.assertEqual( result["test_attribute"].data, IECore.IntData( 123 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(0) # float
		attr.parm("size").set(2) # 2 elementS
		attr.parm("value2").set(456.789)
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2fData )
		self.assertEqual( result["test_attribute"].data.value, imath.V2f( 123.456, 456.789 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2iData )
		self.assertEqual( result["test_attribute"].data.value, imath.V2i( 123, 456 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(0) # float
		attr.parm("size").set(3) # 3 elements
		attr.parm("value3").set(999.999)
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3fData )
		self.assertEqual( result["test_attribute"].data.value, imath.V3f( 123.456, 456.789, 999.999 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3iData )
		self.assertEqual( result["test_attribute"].data.value, imath.V3i( 123, 456, 999 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		attr.parm("type").set( 3 ) # string
		attr.parm( "string" ).set( "string!" )
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.StringData )
		self.assertEqual( result["test_attribute"].data.value, "string!" )
		self.assertEqual( result["test_attribute"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
		self.assertTrue( result.arePrimitiveVariablesValid() )

	# testing that float[4] doesn't work!
	def testFloat4attr( self ) : # we can't deal with float 4's right now
		attr = self.testSetupAttributes()
		attr.parm("name").set( "test_attribute" )
		attr.parm("size").set(4) # 4 elements per point-attribute
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( attr )
		result = converter.convert()
		self.assertTrue( "test_attribute" not in result.keys() ) # invalid due to being float[4]
		self.assertTrue( result.arePrimitiveVariablesValid() )

	# testing conversion of animating geometry
	def testAnimatingGeometry( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		torus = geo.createNode( "torus" )
		facet = geo.createNode( "facet" )
		facet.parm("postnml").set(True)
		mountain = geo.createNode( "mountain" )
		if hou.applicationVersion()[0] >= 16:
			mountain.parm( "offsetx" ).setExpression( "$FF" )
		else:
			mountain.parm("offset1").setExpression( "$FF" )
		facet.setInput( 0, torus )
		mountain.setInput( 0, facet )
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( mountain )
		hou.setFrame( 1 )
		mesh1 = converter.convert()
		hou.setFrame( 2 )
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( mountain )
		mesh2 = converter.convert()
		self.assertNotEqual( mesh1["P"].data, mesh2["P"].data )
		self.assertNotEqual( mesh1, mesh2 )

	# testing we can handle an object being deleted
	def testObjectWasDeleted( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( torus )
		g1 = converter.convert()
		torus.destroy()
		g2 = converter.convert()
		self.assertEqual( g2, g1 )
		self.assertRaises( RuntimeError, IECore.curry( IECoreHoudini.FromHoudiniPolygonsConverter, torus ) )

	# testing we can handle an object being deleted
	def testObjectWasDeletedFactory( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( torus )
		g1 = converter.convert()
		torus.destroy()
		g2 = converter.convert()
		self.assertEqual( g2, g1 )
		self.assertRaises( RuntimeError, IECore.curry( IECoreHoudini.FromHoudiniGeometryConverter.create, torus ) )

	# testing converting a Houdini particle primitive with detail and point attribs
	def testParticlePrimitive( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode( "geo", run_init_scripts=False )
		popnet = self.createPopNet()
		location = popnet.createNode( "poplocation" )
		popSolver = popnet.node( "popsolver1" )
		popSolver.setInput( 2, location )
		detailAttr = popnet.createOutputNode( "attribcreate", exact_type_name=True )
		detailAttr.parm("name").set( "float3detail" )
		detailAttr.parm("class").set( 0 ) # detail
		detailAttr.parm("type").set( 0 ) # float
		detailAttr.parm("size").set( 3 ) # 3 elements
		detailAttr.parm("value1").set( 1 )
		detailAttr.parm("value2").set( 2 )
		detailAttr.parm("value3").set( 3 )
		pointAttr = detailAttr.createOutputNode( "attribcreate", exact_type_name=True )
		pointAttr.parm("name").set( "float3point" )
		pointAttr.parm("class").set( 2 ) # point
		pointAttr.parm("type").set( 0 ) # float
		pointAttr.parm("size").set( 3 ) # 3 elements
		pointAttr.parm("value1").set( 1 )
		pointAttr.parm("value2").set( 2 )
		pointAttr.parm("value3").set( 3 )

		particleSystem = pointAttr.createOutputNode( "add" )
		particleSystem.parm( "addparticlesystem" ).set( True )

		hou.setFrame( 5 )
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( particleSystem )
		self.assertRaises( RuntimeError, converter.convert )

		add = particleSystem.createOutputNode( "add" )
		add.parm( "keep" ).set( 1 ) # deletes primitive and leaves points

		m = IECoreHoudini.FromHoudiniPolygonsConverter( add ).convert()
		self.assertTrue( isinstance( m, IECoreScene.MeshPrimitive ) )
		self.assertEqual( m, IECoreScene.MeshPrimitive() )

	# testing winding order
	def testWindingOrder( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		grid = geo.createNode( "grid" )
		grid.parm( "rows" ).set( 2 )
		grid.parm( "cols" ).set( 2 )

		mesh = IECoreHoudini.FromHoudiniPolygonsConverter( grid ).convert()
		p = mesh["P"].data
		vertexIds = mesh.vertexIds
		self.assertEqual( vertexIds.size(), 4 )

		loop = IECore.V3fVectorData( [ p[vertexIds[0]], p[vertexIds[1]], p[vertexIds[2]], p[vertexIds[3]] ] )
		self.assertTrue( IECore.polygonNormal( loop ).equalWithAbsError( imath.V3f( 0, 1, 0 ), 0.0001 ) )

	# testing vertex data order
	def testVertexDataOrder( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		grid = geo.createNode( "grid" )
		grid.parm( "rows" ).set( 2 )
		grid.parm( "cols" ).set( 2 )
		attr = grid.createOutputNode( "attribcreate", exact_type_name=True )
		attr.parm("name").set( "vertex" )
		attr.parm("class").set( 3 ) # vertex
		attr.parm("type").set( 0 ) # float
		attr.parm("value1").setExpression( "$VTX" )

		mesh = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertEqual( mesh["vertex"].data, IECore.FloatVectorData( [ 3, 2, 1, 0 ] ) )

	def testEmptyStringAttr( self ) :
		torus = self.createTorus()
		geo = torus.parent()
		attr = geo.createNode( "attribcreate", exact_type_name=True )
		attr.setInput( 0, torus )
		attr.parm("name").set( "test_attribute" )
		attr.parm("type").set(3) # string
		attr.parm("string").set("")
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( attr )
		result = converter.convert()
		self.assertTrue( "test_attribute" in result.keys() )
		self.assertEqual( result["test_attribute"].data.size(), 1 )
		self.assertEqual( result["test_attribute"].indices.size(), 100 )
		self.assertEqual( result["test_attribute"].data[0], "" )
		for i in range( 0, 100 ) :
			self.assertEqual( result["test_attribute"].indices[i], 0 )

		self.assertTrue( result.arePrimitiveVariablesValid() )

	def testName( self ) :

		torus = self.createTorus()
		name = torus.createOutputNode( "name" )
		name.parm( "name1" ).set( "torus" )
		box = torus.parent().createNode( "box" )
		name2 = box.createOutputNode( "name" )
		name2.parm( "name1" ).set( "box" )
		merge = name.createOutputNode( "merge" )
		merge.setInput( 1, name2 )

		converter = IECoreHoudini.FromHoudiniPolygonsConverter( merge )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		# both torii were converted as one mesh
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 106 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge, "torus" )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		# only the named polygons were converted
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 100 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge, "box" )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		# only the named polygons were converted
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 6 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )

		# the name filter will create a single MeshPrimitive
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge, "*" )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		result = converter.convert()

		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 106 )
		self.assertTrue( result.arePrimitiveVariablesValid() )

	def testAttributeFilter( self ) :

		torus = self.createTorus()

		# add vertex normals
		facet = torus.createOutputNode( "facet", node_name = "add_point_normals" )
		facet.parm("postnml").set(True)

		# add a primitive colour attributes
		primcol = facet.createOutputNode( "primitive", node_name = "prim_colour" )
		primcol.parm("doclr").set(1)
		primcol.parm("diffr").setExpression("rand($PR)")
		primcol.parm("diffg").setExpression("rand($PR+1)")
		primcol.parm("diffb").setExpression("rand($PR+2)")

		detail = primcol.createOutputNode( "attribcreate", node_name = "detail", exact_type_name=True )
		detail.parm("name").set("detailAttr")
		detail.parm("class").set(0)
		detail.parm("type").set(1)
		detail.parm("size").set(3)
		detail.parm("value1").set(123)
		detail.parm("value2").set(456.789) # can we catch it out with a float?
		detail.parm("value3").set(789)

		uvunwrap = detail.createOutputNode( "uvunwrap" )

		converter = IECoreHoudini.FromHoudiniPolygonsConverter( uvunwrap )
		self.assertEqual( sorted(converter.convert().keys()), [ "Cs", "N", "P", "detailAttr", "uv", "varmap" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "P" )
		self.assertEqual( sorted(converter.convert().keys()), [ "P" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "* ^N ^varmap" )
		self.assertEqual( sorted(converter.convert().keys()), [ "Cs", "P", "detailAttr", "uv" ] )
		# P must be converted
		converter.parameters()["attributeFilter"].setTypedValue( "* ^P" )
		self.assertTrue( "P" in converter.convert().keys() )
		# have to filter the source attr
		converter.parameters()["attributeFilter"].setTypedValue( "Cs" )
		self.assertEqual( sorted(converter.convert().keys()), [ "P" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "Cd" )
		self.assertEqual( sorted(converter.convert().keys()), [ "Cs", "P" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "uv Cd" )
		self.assertEqual( sorted(converter.convert().keys()), [ "Cs", "P", "uv" ] )

	def testStandardAttributeConversion( self ) :

		torus = self.createTorus()
		color = torus.createOutputNode( "color" )
		color.parm( "colortype" ).set( 2 )
		rest = color.createOutputNode( "rest" )
		scale = rest.createOutputNode( "attribcreate" )
		scale.parm( "name1" ).set( "pscale" )
		scale.parm( "value1v1" ).setExpression( "$PT" )
		uvunwrap = scale.createOutputNode( "uvunwrap" )

		converter = IECoreHoudini.FromHoudiniPolygonsConverter( uvunwrap )
		result = converter.convert()
		if hou.applicationVersion()[0] >= 15 :
			self.assertEqual( result.keys(), [ "Cs", "P", "Pref", "uv", "width" ] )
		else :
			self.assertEqual( result.keys(), [ "Cs", "P", "Pref", "uv", "varmap", "width" ] )

		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["Pref"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )

		uvData = result["uv"].data
		uvIndices = result["uv"].indices

		geo = uvunwrap.geometry()
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvData[ uvIndices[i] ][0], uvValues[0] )
				self.assertAlmostEqual( uvData[ uvIndices[i] ][1], uvValues[1] )
				i += 1

		converter["convertStandardAttributes"].setTypedValue( False )
		result = converter.convert()
		if hou.applicationVersion()[0] >= 15 :
			self.assertEqual( result.keys(), [ "Cd", "P", "pscale", "rest", "uv" ] )
		else :
			self.assertEqual( result.keys(), [ "Cd", "P", "pscale", "rest", "uv", "varmap" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["rest"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		uvData = result["uv"].data
		uvIndices = result["uv"].indices

		geo = uvunwrap.geometry()
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvData[ uvIndices[i] ][0], uvValues[0] )
				self.assertAlmostEqual( uvData[ uvIndices[i] ][1], uvValues[1] )
				i += 1

	def testWeldUVs( self ) :

		torus = self.createTorus()
		uvunwrap = torus.createOutputNode( "uvunwrap" )

		converter = IECoreHoudini.FromHoudiniPolygonsConverter( uvunwrap )
		result = converter.convert()
		if hou.applicationVersion()[0] >= 15 :
			self.assertEqual( result.keys(), [ "P", "uv" ] )
		else :
			self.assertEqual( result.keys(), [ "P", "uv", "varmap" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.assertEqual( result["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		uvData = result["uv"].data
		uvIndices = result["uv"].indices

		geo = uvunwrap.geometry()
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvData[ uvIndices[i] ][0], uvValues[0] )
				self.assertAlmostEqual( uvData[ uvIndices[i] ][1], uvValues[1] )
				i += 1

		converter["weldUVs"].setTypedValue( False )
		result = converter.convert()
		if hou.applicationVersion()[0] >= 15 :
			self.assertEqual( result.keys(), [ "P", "uv" ] )
		else :
			self.assertEqual( result.keys(), [ "P", "uv", "varmap" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		self.assertEqual( result["uv"].interpolation, IECoreScene.PrimitiveVariable.Interpolation.FaceVarying )

		uvData = result["uv"].data
		self.assertEqual( result["uv"].indices, None )

		geo = uvunwrap.geometry()
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvData[i][0], uvValues[0] )
				self.assertAlmostEqual( uvData[i][1], uvValues[1] )
				i += 1

	def testInterpolation( self ) :

		torus = self.createTorus()
		normals = torus.createOutputNode( "facet" )
		normals.parm( "postnml" ).set( True )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( normals ).convert()
		self.assertTrue( "ieMeshInterpolation" not in result.keys() )
		self.assertEqual( result.interpolation, "linear" )
		self.assertTrue( "N" in result.keys() )

		attr = normals.createOutputNode( "attribcreate", node_name = "interpolation", exact_type_name=True )
		attr.parm( "name" ).set( "ieMeshInterpolation" )
		attr.parm( "class" ).set( 1 ) # prim
		attr.parm( "type" ).set( 3 ) # string
		attr.parm( "string") .set( "subdiv" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertTrue( "ieMeshInterpolation" not in result.keys() )
		self.assertEqual( result.interpolation, "catmullClark" )
		self.assertTrue( "N" not in result.keys() )

		attr.parm( "string") .set( "poly" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( attr ).convert()
		self.assertTrue( "ieMeshInterpolation" not in result.keys() )
		self.assertEqual( result.interpolation, "linear" )
		self.assertTrue( "N" in result.keys() )

	def testRename( self ) :

		torus = self.createTorus()
		name = torus.createOutputNode( "name" )
		name.parm( "name1" ).set( "foo" )
		rename = name.createOutputNode( "name" )
		rename.parm( "name1" ).set( "bar" )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( rename )
		self.assertTrue( isinstance( converter, IECoreHoudini.FromHoudiniPolygonsConverter ) )
		self.assertTrue( isinstance( converter.convert(), IECoreScene.MeshPrimitive ) )


	def testRenamedUVsExportedWithInterpretation( self ):

		box = self.createBox() # create cube geometry
		quickShade = box.createOutputNode("uvquickshade") # quickly generate orthographically projected UVs
		attribute = quickShade.createOutputNode("attribute") # rename the 'uv' attribute 'foo'
		attribute.parm("frompt0").set("uv")
		attribute.parm("topt0").set("foo")

		convertToCortex = attribute.createOutputNode("ieCortexConverter")
		convertToCortex.parm("resultType").set(0) # convert to cortex
		convertToHoudini = convertToCortex.createOutputNode("ieCortexConverter")
		convertToHoudini.parm("resultType").set(1) # convert to back to houdini

		geo = convertToHoudini.geometry()
		fooAttrib = geo.findPointAttrib( "foo" )

		self.assertEqual( fooAttrib.qualifier(), "Texture Coord")

		roundTrippedUVs = []
		index = 0
		for pnt in geo.points() :
			roundTrippedUVs.append( pnt.attribValue( fooAttrib ) )

		self.assertEqual( roundTrippedUVs, [
			(0.0, 0.0, 0.0),
			(1.0, 0.0, 0.0),
			(1.0, 1.0, 0.0),
			(0.0, 1.0, 0.0),
			(0.0, 0.0, 0.0),
			(1.0, 0.0, 0.0),
			(1.0, 1.0, 0.0),
			(0.0, 1.0, 0.0)] )

	def testAttributeNamedUVIsAlwaysIndexed( self ):

		box = self.createBox() # create cube geometry
		attributeCreate = box.createOutputNode("attribcreate", exact_type_name=True)
		attributeCreate.parm("class").set(3) # vertex
		attributeCreate.parm("type").set(0) #float
		attributeCreate.parm("typeinfo").set(1) # None
		attributeCreate.parm("name").set("uv") #attribute is named uv
		attributeCreate.parm("size").set(3) #float3

		result = IECoreHoudini.FromHoudiniPolygonsConverter( attributeCreate ).convert()

		self.assertEqual( result["uv"].data, IECore.V2fVectorData( [imath.V2f( 0, 0 )], IECore.GeometricData.Interpretation.UV ) )
		self.assertEqual( result["uv"].indices, IECore.IntVectorData( [0] * 24 ) )

	def testCornersAndCreases( self ):

		box = self.createBox() # create cube geometry
		crease = box.createOutputNode("crease", exact_type_name=True)
		crease.parm("group").set("p5-6 p4-5-1")
		crease.parm("crease").set(1.29)
		dodgyCorner = crease.createOutputNode("crease", exact_type_name=True)
		dodgyCorner.parm("group").set("p7")
		dodgyCorner.parm("crease").set(10.0)
		dodgyCorner.parm("creaseattrib").set("cornerweight")
		corner = dodgyCorner.createOutputNode("attribpromote", exact_type_name=True)
		corner.parm("inname").set("cornerweight")
		corner.parm("inclass").set(3) # vertex
		corner.parm("method").set(1) # minimum (to avoid interpolation)

		result = IECoreHoudini.FromHoudiniPolygonsConverter( corner ).convert()
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result.keys(), [ "P" ] )

		self.assertEqual( result.cornerIds(), IECore.IntVectorData( [ 7 ] ) )
		self.assertEqual( result.cornerSharpnesses(), IECore.FloatVectorData( [ 10.0 ] ) )

		self.assertEqual( result.creaseLengths(), IECore.IntVectorData( [ 2, 2, 2 ] ) )
		self.assertEqual( result.creaseIds(), IECore.IntVectorData( [ 1, 5, 4, 5, 5, 6 ] ) )
		self.assertEqual( result.creaseSharpnesses(), IECore.FloatVectorData( [ 1.29, 1.29, 1.29 ] ) )

	def testMeshInterpolationCrash( self ):

		subnet = hou.node("/obj").createNode("subnet")
		geo = subnet.createNode( "geo", "geo" )

		torus = geo.createNode( "torus" )
		torus.parm( "rows" ).set( 1000 )
		torus.parm( "cols" ).set( 1000 )

		grouprange = torus.createOutputNode( "grouprange" )
		grouprange.parm( "groupname1" ).set( "mygroup" )
		grouprange.parm( "end1" ).set(50)

		name1 = grouprange.createOutputNode( "name" )
		name1.parm( "numnames" ).set( 2 )
		name1.parm( "name1" ).set( "/torus/all/of/it" )
		name1.parm( "name2" ).set( "/torus/part/of/it" )
		name1.parm( "group2" ).set( "mygroup" )

		name2 = name1.createOutputNode( "name" )
		name2.parm( "attribname" ).set( "ieMeshInterpolation" )
		name2.parm( "numnames" ).set( 2 )
		name2.parm( "name1" ).set( "poly" )
		name2.parm( "name2" ).set( "subdiv" )
		name2.parm( "group2" ).set( "mygroup" )

		groupdelete = name2.createOutputNode( "groupdelete" )
		groupdelete.parm( "group1" ).set( "mygroup" )

		objectMerge = geo.createNode( "object_merge" )
		objectMerge.parm( "objpath1" ).set( groupdelete.path() )
		objectMerge.parm( "group1" ).set( "@name=/torus/part/of/it" )
		objectMerge.parm( "xformtype" ).set( 1 )

		name3 = objectMerge.createOutputNode( "name" )
		name3.parm( "name1" ).set( "/" )
		name3.setDisplayFlag( True )
		name3.setRenderFlag( True )

		result = IECoreHoudini.FromHoudiniGeometryConverter.create( hou.node( '/obj/subnet1/geo/name3' ) ).convert()
		self.assertTrue( isinstance( result, IECoreScene.MeshPrimitive ) )

if __name__ == "__main__":
    unittest.main()
