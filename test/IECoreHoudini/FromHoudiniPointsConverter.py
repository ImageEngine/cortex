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
import IECore
import IECoreHoudini
import unittest
import os
import math

class TestFromHoudiniPointsConverter( IECoreHoudini.TestCase ) :

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

	# creates a converter
	def testCreateConverter( self )  :
		box = self.createBox()
		converter = IECoreHoudini.FromHoudiniPointsConverter( box )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		
		return converter

	# creates a converter
	def testFactory( self ) :
		box = self.createBox()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( box )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( box, resultType = IECore.TypeId.PointsPrimitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( box, resultType = IECore.TypeId.Parameter )
		self.assertEqual( converter, None )
		
		self.failUnless( IECore.TypeId.PointsPrimitive in IECoreHoudini.FromHoudiniGeometryConverter.supportedTypes() )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( IECore.TypeId.PointsPrimitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( [ IECore.TypeId.PointsPrimitive ] )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )

	# performs geometry conversion
	def testDoConversion( self ) :
		converter = self.testCreateConverter()
		result = converter.convert()
		self.assert_( result.isInstanceOf( IECore.TypeId.PointsPrimitive ) )

	def testConvertFromHOMGeo( self ) :
		geo = self.createPoints().geometry()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo )
		self.failUnless( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		
		result = converter.convert()
		self.failUnless( result.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		
		converter2 = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo, IECore.TypeId.PointsPrimitive )
		self.failUnless( converter2.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )

	# convert a mesh
	def testConvertMesh( self ) :
		torus = self.createTorus()
		converter = IECoreHoudini.FromHoudiniPointsConverter( torus )
		result = converter.convert()
		self.assertEqual( result.typeId(), IECore.PointsPrimitive.staticTypeId() )
		
		bbox = result.bound()
		self.assertEqual( bbox.min.x, -2.0 )
		self.assertEqual( bbox.max.x, 2.0 )
		self.assertEqual( result.numPoints, 100 )
		for i in range( result.numPoints ) :
			self.assert_( result["P"].data[i].x >= bbox.min.x )
			self.assert_( result["P"].data[i].x <= bbox.max.x )

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

		detail_i3 = geo.createNode( "attribcreate", node_name = "detail_i3", exact_type_name=True )
		detail_i3.parm("name").set("detail_i3")
		detail_i3.parm("class").set(0)
		detail_i3.parm("type").set(1)
		detail_i3.parm("size").set(3)
		detail_i3.parm("value1").set(123)
		detail_i3.parm("value2").set(456.789) # can we catch it out with a float?
		detail_i3.parm("value3").set(789)
		detail_i3.setInput( 0, vert_v3f )

		out = geo.createNode( "null", node_name="OUT" )
		out.setInput( 0, detail_i3 )

		# convert it all
		converter = IECoreHoudini.FromHoudiniPointsConverter( out )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		
		result = converter.convert()
		self.assert_( result.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		
		bbox = result.bound()
		self.assertEqual( bbox.min.x, -2.0 )
		self.assertEqual( bbox.max.x, 2.0 )
		self.assertEqual( result.numPoints, 100 )
		for i in range( result.numPoints ) :
			self.assert_( result["P"].data[i].x >= bbox.min.x )
			self.assert_( result["P"].data[i].x <= bbox.max.x )
		
		# test point attributes
		self.assert_( "P" in result )
		self.assertEqual( result['P'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result['P'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result['P'].data.size(), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assert_( "N" in result )
		self.assertEqual( result['N'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result['N'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result['N'].data.size(), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( result["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		# test detail attributes
		self.assert_( "detail_i3" in result )
		self.assertEqual( result['detail_i3'].data.typeId(), IECore.TypeId.V3iData )
		self.assertEqual( result['detail_i3'].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( result['detail_i3'].data.value.x, 123 )
		self.assertEqual( result['detail_i3'].data.value.y, 456 )
		self.assertEqual( result['detail_i3'].data.value.z, 789 )

		# test primitive attributes
		self.assert_( "Cd" not in result )
		
		# test vertex attributes
		attrs = [ "vert_f1", "vert_f2", "vert_f3", "vert_i1", "vert_i2", "vert_i3", "vert_v3f" ]
		for a in attrs :
			self.assert_( a not in result )
		
		self.assert_( result.arePrimitiveVariablesValid() )

	# test prim/vertex attributes on a single primitive (mesh)
	def testConvertMeshPrimVertAttributes( self ) :
		torus = self.createTorus()
		torus.parm( "type" ).set( 1 )
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

		vert_quat = geo.createNode( "attribcreate", node_name = "vert_quat", exact_type_name=True )
		vert_quat.parm("name").set("orient")
		vert_quat.parm("class").set(3)
		vert_quat.parm("size").set(4)
		vert_quat.parm("value1").setExpression("$VTX*0.1")
		vert_quat.parm("value2").setExpression("$VTX*0.2")
		vert_quat.parm("value3").setExpression("$VTX*0.3")
		vert_quat.parm("value4").setExpression("$VTX*0.4")
		vert_quat.setInput( 0, vert_f3 )

		vert_quat2 = geo.createNode( "attribcreate", node_name = "vert_quat2", exact_type_name=True )
		vert_quat2.parm("name").set("quat_2")
		vert_quat2.parm("class").set(3)
		vert_quat2.parm("size").set(4)
		vert_quat2.parm("typeinfo").set(6)  # set type info to quaternion
		vert_quat2.parm("value1").setExpression("$VTX*0.2")
		vert_quat2.parm("value2").setExpression("$VTX*0.4")
		vert_quat2.parm("value3").setExpression("$VTX*0.6")
		vert_quat2.parm("value4").setExpression("$VTX*0.8")
		vert_quat2.setInput( 0, vert_quat )


		vert_m44create = geo.createNode( "attribcreate", node_name = "vert_m44create", exact_type_name=True )
		vert_m44create.parm("name").set("m44")
		vert_m44create.parm("class").set(3)
		vert_m44create.parm("size").set(16)
		vert_m44create.parm("typeinfo").set(7)  # set type info to transformation matrix
		vert_m44create.setInput( 0, vert_quat2 )


		vert_m44 = geo.createNode( "attribwrangle", node_name = "vert_m44", exact_type_name=True )
		vert_m44.parm("snippet").set("4@m44 = maketransform(0,0,{ 10, 20, 30 },{ 30, 45, 60},{ 3, 4, 5 },{ 0, 0, 0 });")
		vert_m44.parm("class").set(3)
		vert_m44.setInput( 0, vert_m44create )

		vert_i1 = geo.createNode( "attribcreate", node_name = "vert_i1", exact_type_name=True )
		vert_i1.parm("name").set("vert_i1")
		vert_i1.parm("class").set(3)
		vert_i1.parm("type").set(1)
		vert_i1.parm("value1").setExpression("$VTX*0.1")
		vert_i1.setInput( 0, vert_m44 )

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
		converter = IECoreHoudini.FromHoudiniPointsConverter( out )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		
		result = converter.convert()
		self.assert_( result.isInstanceOf( IECore.TypeId.PointsPrimitive ) )
		
		bbox = result.bound()
		self.assertEqual( bbox.min.x, -2.0 )
		self.assertEqual( bbox.max.x, 2.0 )
		self.assertEqual( result.numPoints, 100 )
		for i in range( result.numPoints ) :
			self.assert_( result["P"].data[i].x >= bbox.min.x )
			self.assert_( result["P"].data[i].x <= bbox.max.x )
		
		# test point attributes
		self.assert_( "P" in result )
		self.assertEqual( result['P'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result['P'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result['P'].data.size(), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assert_( "N" in result )
		self.assertEqual( result['N'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result['N'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result['N'].data.size(), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
		self.assertEqual( result["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

		# test detail attributes
		self.assert_( "detail_i3" in result )
		self.assertEqual( result['detail_i3'].data.typeId(), IECore.TypeId.V3iData )
		self.assertEqual( result['detail_i3'].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( result['detail_i3'].data.value.x, 123 )
		self.assertEqual( result['detail_i3'].data.value.y, 456 )
		self.assertEqual( result['detail_i3'].data.value.z, 789 )

		# test primitive attributes
		self.assert_( "Cs" in result )
		self.assertEqual( result["Cs"].data.typeId(), IECore.TypeId.Color3fVectorData )
		self.assertEqual( result["Cs"].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( result["Cs"].data.size(), result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
		
		for i in range( 0, result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) ) :
			for j in range( 0, 3 ) :
				self.assert_( result["Cs"].data[i][j] >= 0.0 )
				self.assert_( result["Cs"].data[i][j] <= 1.0 )

		# test vertex attributes
		attrs = [ "vert_f1", "vert_f2", "vert_f3", "orient", "quat_2", "vert_i1", "vert_i2", "vert_i3", "vert_v3f", "vertStringIndices" ]
		for a in attrs :
			self.assert_( a in result )
			self.assertEqual( result[a].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
			self.assertEqual( result[a].data.size(), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
		
		self.assertEqual( result["vert_f1"].data.typeId(), IECore.FloatVectorData.staticTypeId() )
		self.assertEqual( result["vert_f2"].data.typeId(), IECore.V2fVectorData.staticTypeId() )
		self.assertEqual( result["vert_f3"].data.typeId(), IECore.V3fVectorData.staticTypeId() )
		self.assertEqual( result["orient"].data.typeId(), IECore.QuatfVectorData.staticTypeId() )
		
		for i in range( 0, result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) ) :
			for j in range( 0, 3 ) :
				self.assert_( result["vert_f3"].data[i][j] >= 0.0 )
				self.assert_( result["vert_f3"].data[i][j] < 400.1 )

			self.assertAlmostEqual( result["orient"].data[i][0], i * 0.4,5 )
			self.assertAlmostEqual( result["orient"].data[i][1], i * 0.1,5 )
			self.assertAlmostEqual( result["orient"].data[i][2], i * 0.2,5 )
			self.assertAlmostEqual( result["orient"].data[i][3], i * 0.3,5 )

			self.assertAlmostEqual( result["quat_2"].data[i][0], i * 0.8,5 )
			self.assertAlmostEqual( result["quat_2"].data[i][1], i * 0.2,5 )
			self.assertAlmostEqual( result["quat_2"].data[i][2], i * 0.4,5 )
			self.assertAlmostEqual( result["quat_2"].data[i][3], i * 0.6,5 )

		self.assertEqual( result["vert_i1"].data.typeId(), IECore.IntVectorData.staticTypeId() )
		self.assertEqual( result["vert_i2"].data.typeId(), IECore.V2iVectorData.staticTypeId() )
		self.assertEqual( result["vert_i3"].data.typeId(), IECore.V3iVectorData.staticTypeId() )
		
		for i in range( 0, result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) ) :
			for j in range( 0, 3 ) :
				self.assert_( result["vert_i3"].data[i][j] < 10 )
		
		self.assertEqual( result["vert_v3f"].data.typeId(), IECore.V3fVectorData.staticTypeId() )
		
		self.assertEqual( result["vertString"].data.typeId(), IECore.TypeId.StringVectorData )
		self.assertEqual( result["vertString"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( result["vertStringIndices"].data.typeId(), IECore.TypeId.IntVectorData )
		
		for i in range( 0, result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) ) :
			self.assertEqual( result["vertString"].data[i], "string %d!" % i )
			self.assertEqual( result["vertStringIndices"].data[i], i )
		
		self.assertEqual( result["m44"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( result["m44"].data.typeId(), IECore.M44fVectorData.staticTypeId() )

		matrixScale = IECore.M44f.extractSHRT( result["m44"].data[0] )[0]
		matrixRot = IECore.M44f.extractSHRT( result["m44"].data[0] )[2]
		matrixTranslation = IECore.M44f.extractSHRT( result["m44"].data[0] )[3]
		self.assertEqual( matrixTranslation, IECore.V3f( 10,20,30 ) )
		self.assertTrue( matrixRot.equalWithRelError( IECore.V3f( math.pi / 6, math.pi / 4, math.pi / 3 ), 1.e-5 ) )
		self.assertTrue( matrixScale.equalWithRelError( IECore.V3f( 3, 4, 5 ), 1.e-5 ) )
		
		self.assert_( result.arePrimitiveVariablesValid() )
	
	# convert some points
	def testConvertPoints( self ) :
		points = self.createPoints()
		converter = IECoreHoudini.FromHoudiniPointsConverter( points )
		result = converter.convert()
		self.assertEqual( result.typeId(), IECore.PointsPrimitive.staticTypeId() )
		self.assertEqual( points.parm('npts').eval(), result.numPoints )
		self.assert_( "P" in result.keys() )
		self.assert_( "N" in result.keys() )
		self.assert_( result.arePrimitiveVariablesValid() )

	# simple attribute conversion
	def testSetupAttributes( self ) :
		points = self.createPoints()
		geo = points.parent()
		attr = geo.createNode( "attribcreate", exact_type_name=True )
		attr.setInput( 0, points )
		attr.parm("name").set( "test_attribute" )
		attr.parm("type").set(0) # float
		attr.parm("size").set(1) # 1 element
		attr.parm("value1").set(123.456)
		attr.parm("value2").set(654.321)
		converter = IECoreHoudini.FromHoudiniPointsConverter( attr )
		result = converter.convert()
		self.assert_( "test_attribute" in result.keys() )
		self.assertEqual( result["test_attribute"].data.size(), points.parm('npts').eval() )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		return attr

	# testing point attributes and types
	def testPointAttributes( self ) :
		attr = self.testSetupAttributes()
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.FloatVectorData )
		self.assert_( result["test_attribute"].data[0] > 123.0 )
		self.assertEqual( result["test_attribute"].data.size(), 5000 )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(1) # integer
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.IntVectorData )
		self.assertEqual( result["test_attribute"].data[0], 123 )
		self.assertEqual( result["test_attribute"].data.size(), 5000 )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(0) # float
		attr.parm("size").set(2) # 2 elementS
		attr.parm("value2").set(456.789)
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2fVectorData )
		self.assertEqual( result["test_attribute"].data[0], IECore.V2f( 123.456, 456.789 ) )
		self.assertEqual( result["test_attribute"].data.size(), 5000 )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2iVectorData )
		self.assertEqual( result["test_attribute"].data[0], IECore.V2i( 123, 456 ) )
		self.assertEqual( result["test_attribute"].data.size(), 5000 )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(0) # float
		attr.parm("size").set(3) # 3 elements
		attr.parm("value3").set(999.999)
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3fVectorData )
		self.assertEqual( result["test_attribute"].data[0],IECore.V3f( 123.456, 456.789, 999.999 ) )
		self.assertEqual( result["test_attribute"].data.size(), 5000 )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3iVectorData )
		self.assertEqual( result["test_attribute"].data[0], IECore.V3i( 123, 456, 999 ) )
		self.assertEqual( result["test_attribute"].data.size(), 5000 )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set( 3 ) # string
		attr.parm( "string" ).set( "string $PT!" )
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.StringVectorData )
		self.assertEqual( result["test_attribute"].data[10], "string 10!" )
		self.assertEqual( result["test_attribute"].data.size(), 5000 )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( result["test_attributeIndices"].data.typeId(), IECore.TypeId.IntVectorData )
		self.assertEqual( result["test_attributeIndices"].data[10], 10 )
		self.assertEqual( result["test_attributeIndices"].data.size(), 5000 )
		self.assertEqual( result["test_attributeIndices"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assert_( result.arePrimitiveVariablesValid() )

	# testing detail attributes and types
	def testDetailAttributes( self ) :
		attr = self.testSetupAttributes()
		attr.parm("class").set(0) # detail attribute
		
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		attr.parm("value1").set(123.456)
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.FloatData )
		self.assert_( result["test_attribute"].data > IECore.FloatData( 123.0 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(1) # integer
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.IntData )
		self.assertEqual( result["test_attribute"].data, IECore.IntData( 123 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(0) # float
		attr.parm("size").set(2) # 2 elementS
		attr.parm("value2").set(456.789)
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2fData )
		self.assertEqual( result["test_attribute"].data.value, IECore.V2f( 123.456, 456.789 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V2iData )
		self.assertEqual( result["test_attribute"].data.value, IECore.V2i( 123, 456 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(0) # float
		attr.parm("size").set(3) # 3 elements
		attr.parm("value3").set(999.999)
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3fData )
		self.assertEqual( result["test_attribute"].data.value, IECore.V3f( 123.456, 456.789, 999.999 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assert_( result.arePrimitiveVariablesValid() )
		
		attr.parm("type").set(1) # int
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.V3iData )
		self.assertEqual( result["test_attribute"].data.value, IECore.V3i( 123, 456, 999 ) )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assert_( result.arePrimitiveVariablesValid() )

		attr.parm("type").set( 3 ) # string
		attr.parm( "string" ).set( "string!" )
		result = IECoreHoudini.FromHoudiniPointsConverter( attr ).convert()
		self.assertEqual( result["test_attribute"].data.typeId(), IECore.TypeId.StringData )
		self.assertEqual( result["test_attribute"].data.value, "string!" )
		self.assertEqual( result["test_attribute"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assert_( result.arePrimitiveVariablesValid() )
	
	# testing that float[4] doesn't work!
	def testFloat4attr( self ) : # we can't deal with float 4's right now
		attr = self.testSetupAttributes()
		attr.parm("name").set( "test_attribute" )
		attr.parm("size").set(4) # 4 elements per point-attribute
		converter = IECoreHoudini.FromHoudiniPointsConverter( attr )
		result = converter.convert()
		self.assert_( "test_attribute" not in result.keys() ) # invalid due to being float[4]
		self.assert_( result.arePrimitiveVariablesValid() )
	
	# testing conversion of animating geometry
	def testAnimatingGeometry( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		facet = geo.createNode( "facet" )
		facet.parm("postnml").set(True)
		mountain = geo.createNode( "mountain" )
		mountain.parm("offset1").setExpression( "$FF" )
		points = geo.createNode( "scatter" )
		facet.setInput( 0, torus )
		mountain.setInput( 0, facet )
		points.setInput( 0, mountain )
		converter = IECoreHoudini.FromHoudiniPointsConverter( points )
		hou.setFrame(1)
		points_1 = converter.convert()
		hou.setFrame(2)
		converter = IECoreHoudini.FromHoudiniPointsConverter( points )
		points_2 = converter.convert()
		self.assertNotEqual( points_1["P"].data, points_2["P"].data )

	# testing we can handle an object being deleted
	def testObjectWasDeleted( self ) :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		converter = IECoreHoudini.FromHoudiniPointsConverter( torus )
		g1 = converter.convert()
		torus.destroy()
		g2 = converter.convert()
		self.assertEqual( g2, g1 )
		self.assertRaises( RuntimeError, IECore.curry( IECoreHoudini.FromHoudiniPointsConverter, torus ) )
	
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
		popnet = geo.createNode( "popnet" )
		location = popnet.createNode( "location" )
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
		
		hou.setFrame( 5 )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( pointAttr )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		points = converter.convert()
		
		self.assertEqual( type(points), IECore.PointsPrimitive )
		self.assertEqual( points.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 21 )
		self.assertEqual( points["float3detail"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( type(points["float3detail"].data), IECore.V3fData )
		self.assert_( points["float3detail"].data.value.equalWithRelError( IECore.V3f( 1, 2, 3 ), 1e-10 ) )
		self.assertEqual( type(points["float3point"].data), IECore.V3fVectorData )
		self.assertEqual( points["float3point"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		for p in points["float3point"].data :
			self.assert_( p.equalWithRelError( IECore.V3f( 1, 2, 3 ), 1e-10 ) )
		
		self.assert_( points.arePrimitiveVariablesValid() )
		
		add = pointAttr.createOutputNode( "add" )
		add.parm( "keep" ).set( 1 ) # deletes primitive and leaves points
		
		converter = IECoreHoudini.FromHoudiniPointsConverter( add )
		points2 = converter.convert()
		del points['generator']
		del points['generatorIndices']
		del points['born']
		del points['source']
		self.assertEqual( points2, points )
	
	def testMultipleParticlePrimitives( self ) :
		
		obj = hou.node("/obj")
		geo = obj.createNode( "geo", run_init_scripts=False )
		popnet = geo.createNode( "popnet" )
		fireworks = popnet.createNode( "fireworks" )
		
		hou.setFrame( 15 )
		converter = IECoreHoudini.FromHoudiniPointsConverter( popnet )
		points = converter.convert()
		
		self.assertEqual( type(points), IECore.PointsPrimitive )
		self.assertEqual( points.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 24 )
		self.assertEqual( points["accel"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( type(points["accel"].data), IECore.V3fVectorData )
		self.assertEqual( points["accel"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertEqual( points["nextid"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( points["nextid"].data, IECore.IntData( 25 ) )
		self.assertTrue( points.arePrimitiveVariablesValid() )
		
		add = popnet.createOutputNode( "add" )
		add.parm( "keep" ).set( 1 ) # deletes primitive and leaves points
		
		converter = IECoreHoudini.FromHoudiniPointsConverter( add )
		points2 = converter.convert()
		# showing that prim attribs don't get converted because the interpolation size doesn't match
		self.assertEqual( points2, points )
	
	def testName( self ) :
		
		points = self.createPoints()
		particles = points.createOutputNode( "add" )
		particles.parm( "addparticlesystem" ).set( True )
		name = particles.createOutputNode( "name" )
		name.parm( "name1" ).set( "points" )
		box = points.parent().createNode( "box" )
		name2 = box.createOutputNode( "name" )
		name2.parm( "name1" ).set( "box" )
		merge = name.createOutputNode( "merge" )
		merge.setInput( 1, name2 )
		
		converter = IECoreHoudini.FromHoudiniPointsConverter( merge )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		self.assertFalse( "nameIndices" in result )
		# both shapes were converted as one PointsPrimitive
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 5008 )
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge, "points" )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		self.assertFalse( "nameIndices" in result )
		# only the named points were converted
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 5000 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge, "box", IECore.TypeId.PointsPrimitive )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		self.assertFalse( "nameIndices" in result )
		# only the named points were converted
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 8 )
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )
	
	def testAttributeFilter( self ) :
		
		points = self.createPoints()
		particles = points.createOutputNode( "add" )
		particles.parm( "addparticlesystem" ).set( True )
		
		# add vertex normals
		facet = particles.createOutputNode( "facet", node_name = "add_point_normals" )
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
		
		converter = IECoreHoudini.FromHoudiniPointsConverter( detail )
		self.assertEqual( sorted(converter.convert().keys()), [ "Cs", "N", "P", "detailAttr", "varmap" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "P" )
		self.assertEqual( sorted(converter.convert().keys()), [ "P" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "* ^N ^varmap" )
		self.assertEqual( sorted(converter.convert().keys()), [ "Cs", "P", "detailAttr" ] )
		# P must be converted
		converter.parameters()["attributeFilter"].setTypedValue( "* ^P" )
		self.assertTrue( "P" in converter.convert().keys() )
	
	def testStandardAttributeConversion( self ) :
		
		points = self.createPoints()
		color = points.createOutputNode( "color" )
		color.parm( "colortype" ).set( 2 )
		rest = color.createOutputNode( "rest" )
		scale = rest.createOutputNode( "attribcreate" )
		scale.parm( "name1" ).set( "pscale" )
		scale.parm( "value1v1" ).setExpression( "$PT" )
		
		converter = IECoreHoudini.FromHoudiniPointsConverter( scale )
		result = converter.convert()
		if hou.applicationVersion()[0] >= 15 :
			self.assertEqual( result.keys(), [ "Cs", "N", "P", "Pref", "width" ] )
		else :
			self.assertEqual( result.keys(), [ "Cs", "N", "P", "Pref", "varmap", "width" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["Pref"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		
		converter["convertStandardAttributes"].setTypedValue( False )
		result = converter.convert()
		if hou.applicationVersion()[0] >= 15 :
			self.assertEqual( result.keys(), [ "Cd", "N", "P", "pscale", "rest" ] )
		else :
			self.assertEqual( result.keys(), [ "Cd", "N", "P", "pscale", "rest", "varmap" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["rest"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["N"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

if __name__ == "__main__":
    unittest.main()
