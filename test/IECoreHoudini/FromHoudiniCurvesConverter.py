##########################################################################
#
#  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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
import hou
import IECore
import IECoreScene
import IECoreHoudini
import unittest
import imath

class TestFromHoudiniCurvesConverter( IECoreHoudini.TestCase ) :

	__curveCoordinates = [
		"2.42892,0,-1.04096 1.69011,0,-9.88746 5.74288,0,-4.50183 2.69113,0,-2.78439 5.8923,0,1.53021 6.20965,-9.53674e-07,2.03933 2.72012,0,2.5738 1.76971,0,-0.632637",
		"-0.560781,0,-1.04096 2.21995,0,-6.31734 4.77513,0,-6.61752 4.10862,0,-2.78439 4.29081,0,1.53021 6.20965,-9.53674e-07,3.7489 -2.61584,0,2.5738 -1.45801,0,0.780965",
		"2.42892,0,-1.04096 2.21995,0,-4.51254 4.77513,0,-4.50183 6.32944,0,-2.78439 7.231,0,1.53021 6.20965,-9.53674e-07,3.7489 2.72012,0,2.5738 1.76971,0,0.780965",
		"5.83427,0,-1.04096 2.21995,0,-4.51254 6.14141,0,-4.50183 7.48932,0,-2.78439 9.0197,0,1.53021 6.20965,-9.53674e-07,1.2141 2.72012,0,2.5738 3.23728,0,0.780965"
	]

	def createCurve( self, order=2, periodic=False, parent=None, coordIndex=0 ) :

		if not parent :
			obj = hou.node("/obj")
			parent = obj.createNode("geo", run_init_scripts=False)

		curve = parent.createNode( "curve" )
		curve.parm( "type" ).set( 1 ) # NURBS
		curve.parm( "order" ).set( order )
		curve.parm( "close" ).set( periodic )
		curve.parm( "coords" ).set( TestFromHoudiniCurvesConverter.__curveCoordinates[coordIndex] )

		detailAttr = curve.createOutputNode( "attribcreate", exact_type_name=True )
		detailAttr.parm("name").set( "detailAttribute" )
		detailAttr.parm("class").set( 0 ) # detail attribute
		detailAttr.parm("type").set( 0 ) # float
		detailAttr.parm("size").set( 1 ) # 1 element
		detailAttr.parm("value1").set( 123.456 )

		pointAttr = detailAttr.createOutputNode( "attribcreate", exact_type_name=True )
		pointAttr.parm("name").set( "pointAttribute" )
		pointAttr.parm("class").set( 2 ) # point
		pointAttr.parm("type").set( 0 ) # float
		pointAttr.parm("size").set( 3 ) # 3 elements
		pointAttr.parm("value1").set( 123.456 )
		pointAttr.parm("value2").set( 654.321 )
		pointAttr.parm("value2").set( 789.123 )

		primAttr = pointAttr.createOutputNode( "attribcreate", exact_type_name=True )
		primAttr.parm("name").set( "primAttribute" )
		primAttr.parm("class").set( 1 ) # prim
		primAttr.parm("type").set( 1 ) # int
		primAttr.parm("size").set( 2 ) # 2 elements
		primAttr.parm("value1").set( 10 )
		primAttr.parm("value2").set( 45 )

		vertexAttr = primAttr.createOutputNode( "attribcreate", exact_type_name=True )
		vertexAttr.parm("name").set( "vertexAttribute" )
		vertexAttr.parm("class").set( 3 ) # vertex
		vertexAttr.parm("type").set( 0 ) # float
		vertexAttr.parm("size").set( 0 ) # 1 element
		vertexAttr.parm("value1").setExpression( "$VTX" )

		return vertexAttr

	def createCurves( self, numCurves, order=2, periodic=False ) :

		curves = [ self.createCurve( order, periodic ) ]

		geo = curves[0].parent()

		for i in range( 0, numCurves-1 ) :
			curves.append( self.createCurve( order, periodic, geo, i%4 ) )

		merge = geo.createNode( "merge" )

		for i in range( 0, len(curves) ) :
			merge.setInput( i, curves[i] )

		return merge

	def testCreateConverter( self )  :
		curve = self.createCurve()
		converter = IECoreHoudini.FromHoudiniCurvesConverter( curve )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

		return converter

	def testFactory( self ) :
		curve = self.createCurve()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( curve )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( curve, resultType = IECoreScene.TypeId.CurvesPrimitive )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( curve, resultType = IECore.TypeId.Parameter )
		self.assertEqual( converter, None )

		self.assertTrue( IECoreScene.TypeId.CurvesPrimitive in IECoreHoudini.FromHoudiniGeometryConverter.supportedTypes() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( IECoreScene.TypeId.CurvesPrimitive )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.createDummy( [ IECoreScene.TypeId.CurvesPrimitive ] )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

	def testConvertFromHOMGeo( self ) :
		geo = self.createCurve().geometry()
		converter = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

		result = converter.convert()
		self.assertTrue( result.isInstanceOf( IECoreScene.TypeId.CurvesPrimitive ) )

		converter2 = IECoreHoudini.FromHoudiniGeometryConverter.createFromGeo( geo, IECoreScene.TypeId.CurvesPrimitive )
		self.assertTrue( converter2.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

	def verifyLinearCurves( self, sop ) :
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		self.assertEqual( result.typeId(), IECoreScene.TypeId.CurvesPrimitive )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		geo = sop.geometry()
		bBox = result.bound()
		hBBox = geo.boundingBox()
		for i in range( 0, 3 ) :
			self.assertAlmostEqual( bBox.min()[i], hBBox.minvec()[i] )
			self.assertAlmostEqual( bBox.max()[i], hBBox.maxvec()[i] )

		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), len(geo.points()) )
		self.assertEqual( result.verticesPerCurve().size(), len(geo.prims()) )
		index = 0
		for i in range( len( result.verticesPerCurve() ) ) :
			self.assertEqual( result.verticesPerCurve()[i], geo.prims()[i].numVertices() )

			hVertices = geo.prims()[i].vertices()
			for j in range( len(hVertices) ) :
				self.assertEqual( tuple(result["P"].data[index+j]), tuple(hVertices[j].point().position()) )

			index += result.verticesPerCurve()[i]

		self.assertEqual( result.basis(), IECore.CubicBasisf.linear() )

		for attr in geo.globalAttribs() :
			if attr.name() == "varmap" :
				continue

			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
			self.assertEqual( result[attr.name()].data.value, geo.attribValue( attr.name() ) )

		sopPoints = geo.points()
		for attr in geo.pointAttribs() :
			if attr.name() == "Pw" :
				continue

			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

			data = result[attr.name()].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPoints[i].attribValue( attr.name() ) )

		sopPrims = geo.prims()
		self.assertEqual( len(sopPrims), len(result.verticesPerCurve()) )

		for attr in geo.primAttribs() :
			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

			data = result[attr.name()].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPrims[i].attribValue( attr.name() ) )

		sopVerts = []
		for i in range( 0, len(sopPrims) ) :
			verts = list(sopPrims[i].vertices())
			self.assertEqual( len(verts), result.verticesPerCurve()[i] )
			sopVerts.extend( verts )

		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( len(sopVerts), result["P"].data.size() )
		for attr in geo.vertexAttribs() :
			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )
			data = result[attr.name()].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopVerts[i].attribValue( attr.name() ) )

	def testOneLinearCurve( self ) :
		self.verifyLinearCurves( self.createCurve() )

	def testOneLinearPeriodicCurve( self ) :
		self.verifyLinearCurves( self.createCurve( periodic=True ) )

	def testFourLinearCurves( self ) :
		self.verifyLinearCurves( self.createCurves( 4 ) )

	def testFourLinearPeriodicCurves( self ) :
		self.verifyLinearCurves( self.createCurves( 4, periodic=True ) )

	def testManyLinearCurves( self ) :
		self.verifyLinearCurves( self.createCurves( 100 ) )

	def testManyLinearPeriodicCurves( self ) :
		self.verifyLinearCurves( self.createCurves( 100, periodic=True ) )

	def verifyBSplineCurves( self, sop ) :
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		self.assertEqual( result.typeId(), IECoreScene.TypeId.CurvesPrimitive )
		self.assertTrue( result.arePrimitiveVariablesValid() )

		geo = sop.geometry()
		bBox = result.bound()
		hBBox = geo.boundingBox()
		for i in range( 0, 3 ) :
			self.assertAlmostEqual( bBox.min()[i], hBBox.minvec()[i] )
			self.assertAlmostEqual( bBox.max()[i], hBBox.maxvec()[i] )

		if result.periodic() :
			extraPoints = 0
		else :
			extraPoints = 4

		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex ), len(geo.points()) + extraPoints*len(geo.prims()) )
		self.assertEqual( result.verticesPerCurve().size(), len(geo.prims()) )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		pIndex = 0
		for i in range( len( result.verticesPerCurve() ) ) :
			self.assertEqual( result.verticesPerCurve()[i], geo.prims()[i].numVertices() + extraPoints )

			hVertices = geo.prims()[i].vertices()
			for j in range( len(hVertices) ) :

				if not result.periodic() and ( j == 0 or j == len(hVertices)-1 ) :
					self.assertEqual( tuple(result["P"].data[pIndex]), tuple(hVertices[j].point().position()) )
					self.assertEqual( tuple(result["P"].data[pIndex+1]), tuple(hVertices[j].point().position()) )
					pIndex += 2

				self.assertEqual( tuple(result["P"].data[pIndex]), tuple(hVertices[j].point().position()) )
				pIndex += 1

		self.assertEqual( result.basis(), IECore.CubicBasisf.bSpline() )

		for attr in geo.globalAttribs() :
			if attr.name() == "varmap" :
				continue

			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Constant )
			self.assertEqual( result[attr.name()].data.value, geo.attribValue( attr.name() ) )

		sopPoints = geo.points()
		for attr in geo.pointAttribs() :
			if attr.name() == "Pw" :
				continue

			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		for attr in geo.vertexAttribs() :
			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Vertex )

		sopPrims = geo.prims()
		self.assertEqual( len(sopPrims), len(result.verticesPerCurve()) )

		for attr in geo.primAttribs() :
			self.assertTrue( attr.name() in result )
			self.assertEqual( result[attr.name()].interpolation, IECoreScene.PrimitiveVariable.Interpolation.Uniform )

			data = result[attr.name()].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPrims[i].attribValue( attr.name() ) )

		pIndex = 0
		for i in range( len( result.verticesPerCurve() ) ) :
			hVertices = sopPrims[i].vertices()
			self.assertEqual( len(hVertices) + extraPoints, result.verticesPerCurve()[i] )
			for j in range( len(hVertices) ) :

				for attr in geo.pointAttribs() :
					if attr.name() == "Pw" :
						continue

					data = result[attr.name()].data
					self.assertEqual( tuple(data[pIndex]), hVertices[j].point().attribValue( attr.name() ) )

					if not result.periodic() and ( j == 0 or j == len(hVertices)-1 ) :
						self.assertEqual( tuple(data[pIndex+1]), hVertices[j].point().attribValue( attr.name() ) )
						self.assertEqual( tuple(data[pIndex+2]), hVertices[j].point().attribValue( attr.name() ) )

				for attr in geo.vertexAttribs() :
					data = result[attr.name()].data
					self.assertEqual( data[pIndex], hVertices[j].attribValue( attr.name() ) )

					if not result.periodic() and ( j == 0 or j == len(hVertices)-1 ) :
						self.assertEqual( data[pIndex+1], hVertices[j].attribValue( attr.name() ) )
						self.assertEqual( data[pIndex+2], hVertices[j].attribValue( attr.name() ) )

				if not result.periodic() and ( j == 0 or j == len(hVertices)-1 ) :
					pIndex += 3
				else :
					pIndex += 1

	def testOneBSplineCurve( self ) :
		self.verifyBSplineCurves( self.createCurve( order=4 ) )

	def testOneBSplinePeriodicCurve( self ) :
		self.verifyBSplineCurves( self.createCurve( order=4, periodic=True ) )

	def testFourBSplineCurves( self ) :
		self.verifyBSplineCurves( self.createCurves( 4, order=4 ) )

	def testFourBSplinePeriodicCurves( self ) :
		self.verifyBSplineCurves( self.createCurves( 4, order=4, periodic=True ) )

	def testManyBSplineCurves( self ) :
		self.verifyBSplineCurves( self.createCurves( 100, order=4 ) )

	def testManyBSplinePeriodicCurves( self ) :
		self.verifyBSplineCurves( self.createCurves( 100, order=4, periodic=True ) )

	def testAnimatingGeometry( self ) :
		curves = self.createCurves( 4 )
		mountain = curves.createOutputNode( "mountain" )
		if hou.applicationVersion()[0] >= 16:
			mountain.parm("offsetx").setExpression( "$FF" )
		else:
			mountain.parm( "offset1" ).setExpression( "$FF" )
		converter = IECoreHoudini.FromHoudiniCurvesConverter( mountain )
		hou.setFrame( 1 )
		result1 = converter.convert()
		hou.setFrame( 2 )
		converter = IECoreHoudini.FromHoudiniCurvesConverter( mountain )
		result2 = converter.convert()
		self.assertNotEqual( result1["P"].data, result2["P"].data )
		self.assertNotEqual( result1, result2 )

	def testName( self ) :

		curves = self.createCurves( 4 )
		name = curves.createOutputNode( "name" )
		name.parm( "name1" ).set( "curvesA" )
		curves2 = self.createCurve( parent = curves.parent() )
		name2 = curves2.createOutputNode( "name" )
		name2.parm( "name1" ).set( "curvesB" )
		merge = name.createOutputNode( "merge" )
		merge.setInput( 1, name2 )

		converter = IECoreHoudini.FromHoudiniCurvesConverter( merge )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		# all curves were converted as one CurvesPrimitive
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 5 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge, "curvesA" )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		# only the named curves were converted
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 4 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge, "curvesB" )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )
		result = converter.convert()
		# names are not stored on the object at all
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertFalse( "name" in result )
		# only the named curves were converted
		self.assertEqual( result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ), 1 )
		self.assertTrue(  result.arePrimitiveVariablesValid() )

	def testAttributeFilter( self ) :

		curves = self.createCurves( 4 )
		uvunwrap = curves.createOutputNode( "uvunwrap" )
		converter = IECoreHoudini.FromHoudiniCurvesConverter( uvunwrap )
		self.assertEqual( sorted(converter.convert().keys()), ['P', 'detailAttribute', 'pointAttribute', 'primAttribute', 'uv', 'varmap', 'vertexAttribute'] )
		converter.parameters()["attributeFilter"].setTypedValue( "P" )
		self.assertEqual( sorted(converter.convert().keys()), [ "P" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "* ^primAttribute ^varmap" )
		self.assertEqual( sorted(converter.convert().keys()), ['P', 'detailAttribute', 'pointAttribute', 'uv', 'vertexAttribute'] )
		# P must be converted
		converter.parameters()["attributeFilter"].setTypedValue( "* ^P" )
		self.assertTrue( "P" in converter.convert().keys() )
		converter.parameters()["attributeFilter"].setTypedValue( "uv" )
		self.assertEqual( sorted(converter.convert().keys()), [ "P", "uv" ] )
		converter.parameters()["attributeFilter"].setTypedValue( "" )
		self.assertEqual( sorted(converter.convert().keys()), [ "P" ] )

	def testErrorStates( self ) :

		# no prims
		merge = self.createCurves( 4 )
		add = merge.createOutputNode( "add" )
		add.parm( "keep" ).set( True ) # deletes primitive and leaves points
		converter = IECoreHoudini.FromHoudiniCurvesConverter( add )
		self.assertRaises( RuntimeError, converter.convert )

		# non-curve prims
		merge = self.createCurves( 4 )
		merge.createInputNode( 5, "box" )
		converter = IECoreHoudini.FromHoudiniCurvesConverter( merge )
		self.assertRaises( RuntimeError, converter.convert )

		# many curves, different orders
		merge = self.createCurves( 4 )
		hou.parm( "/obj/geo3/curve3/order" ).set( 3 )
		converter = IECoreHoudini.FromHoudiniCurvesConverter( merge )
		self.assertRaises( RuntimeError, converter.convert )

		# many curves, different periodicity
		merge = self.createCurves( 4 )
		hou.parm( "/obj/geo4/curve3/close" ).set( True )
		converter = IECoreHoudini.FromHoudiniCurvesConverter( merge )
		self.assertRaises( RuntimeError, converter.convert )

	def testStandardAttributeConversion( self ) :

		curves = self.createCurves( 4 )
		color = curves.createOutputNode( "color" )
		color.parm( "colortype" ).set( 2 )
		rest = color.createOutputNode( "rest" )
		scale = rest.createOutputNode( "attribcreate" )
		scale.parm( "name1" ).set( "pscale" )
		scale.parm( "value1v1" ).setExpression( "$PT" )
		uvunwrap = scale.createOutputNode( "uvunwrap" )

		converter = IECoreHoudini.FromHoudiniCurvesConverter( uvunwrap )
		result = converter.convert()
		self.assertEqual( result.keys(), [ "Cs", "P", "Pref", "detailAttribute", "pointAttribute", "primAttribute", "uv", "varmap", "vertexAttribute", "width" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["Pref"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		uvData = result["uv"].data
		geo = uvunwrap.geometry()
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			for vert in prim.vertices() :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvValues[0], uvData[i][0] )
				self.assertAlmostEqual( uvValues[1], uvData[i][1] )
				i += 1

		converter["convertStandardAttributes"].setTypedValue( False )
		result = converter.convert()
		self.assertEqual( result.keys(), [ "Cd", "P", "detailAttribute", "pointAttribute", "primAttribute", "pscale", "rest", "uv", "varmap", "vertexAttribute" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["rest"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )

		uvData = result["uv"].data
		geo = uvunwrap.geometry()
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			for vert in prim.vertices() :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvData[i][0], uvValues[0] )
				self.assertAlmostEqual( uvData[i][1], uvValues[1] )
				i += 1

	def testAllOpenPolygonsConvertedAsLinearCurves( self ) :

		obj = hou.node( "/obj" )
		parent = obj.createNode( "geo", run_init_scripts = False )

		curves = [parent.createNode( "curve" ), parent.createNode( "curve" ), parent.createNode( "curve" )]

		curves[0].parm( "type" ).set( 0 )  # polygon
		curves[0].parm( "close" ).set( False )
		curves[0].parm( "coords" ).set( "0, 0, 0    0, 1, 0    1, 1, 0" )

		curves[1].parm( "type" ).set( 0 )  # polygon
		curves[1].parm( "close" ).set( False )
		curves[1].parm( "coords" ).set( "0,0,0    0,0,1    1,0,1" )

		curves[2].parm( "type" ).set( 0 )  # polygon
		curves[2].parm( "close" ).set( False )
		curves[2].parm( "coords" ).set( "0,0,0    1,0,0" )

		merge = parent.createNode( "merge" )

		for i in range( 0, len( curves ) ) :
			merge.setInput( i, curves[i] )

		# Use the base FromHoudiniGeometryConverter.create to verify we create a CurvesConverter for this open polygon detail
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )

		actualCurvesPrimitive = converter.convert()

		self.assertTrue( actualCurvesPrimitive.isInstanceOf( IECoreScene.CurvesPrimitive ) )
		self.assertTrue( "P" in actualCurvesPrimitive )
		self.assertEqual( actualCurvesPrimitive.verticesPerCurve(), IECore.IntVectorData( [3, 3, 2] ) )
		self.assertEqual( actualCurvesPrimitive.basis().standardBasis(), IECore.StandardCubicBasis.Linear )
		self.assertEqual( actualCurvesPrimitive["P"].data, IECore.V3fVectorData( [
			imath.V3f( 0, 0, 0 ), imath.V3f( 0, 1, 0 ), imath.V3f( 1, 1, 0 ),
			imath.V3f( 0, 0, 0 ), imath.V3f( 0, 0, 1 ), imath.V3f( 1, 0, 1 ),
			imath.V3f( 0, 0, 0 ), imath.V3f( 1, 0, 0 )
		], IECore.GeometricData.Interpretation.Point ) )

		# Now we close one of the polygons
		curves[2].parm( "close" ).set( True )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( merge )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )


if __name__ == "__main__":
    unittest.main()
