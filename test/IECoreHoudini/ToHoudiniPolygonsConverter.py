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

import hou
import imath
import IECore
import IECoreScene
import IECoreHoudini
import unittest
import os
from six.moves import range

class TestToHoudiniPolygonsConverter( IECoreHoudini.TestCase ) :

	__testScene = "test/converterTest.hip"

	if hou.applicationVersion()[0] >= 16:
		PointPositionAttribs = ['P']
	else:
		PointPositionAttribs = ['P', 'Pw']

	def mesh( self ) :
		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [ 1, 5, 4, 0, 2, 6, 5, 1, 3, 7, 6, 2, 0, 4, 7, 3, 2, 1, 0, 3, 5, 6, 7, 4 ] )
		mesh = IECoreScene.MeshPrimitive( vertsPerFace, vertexIds )

		floatData = IECore.FloatData( 1.5 )
		v2fData = IECore.V2fData( imath.V2f( 1.5, 2.5 ), IECore.GeometricData.Interpretation.Vector )
		v3fData = IECore.V3fData( imath.V3f( 1.5, 2.5, 3.5 ) )
		color3fData = IECore.Color3fData( imath.Color3f( 1.5, 2.5, 3.5 ) )
		intData = IECore.IntData( 1 )
		v2iData = IECore.V2iData( imath.V2i( 1, 2 ) )
		v3iData = IECore.V3iData( imath.V3i( 1, 2, 3 ) )
		stringData = IECore.StringData( "this is a string" )

		intRange = list(range( 1, 25))
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		v2fVectorData = IECore.V2fVectorData( [ imath.V2f( x, x+0.5 ) for x in intRange ] )
		v3fVectorData = IECore.V3fVectorData( [ imath.V3f( x, x+0.5, x+0.75 ) for x in intRange ], IECore.GeometricData.Interpretation.Normal )
		color3fVectorData = IECore.Color3fVectorData( [ imath.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		intVectorData = IECore.IntVectorData( intRange )
		v2iVectorData = IECore.V2iVectorData( [ imath.V2i( x, -x ) for x in intRange ] )
		v3iVectorData = IECore.V3iVectorData( [ imath.V3i( x, -x, x*2 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %06d!" % x for x in intRange ] )

		detailInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Constant
		pointInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Vertex
		primitiveInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Uniform
		vertexInterpolation = IECoreScene.PrimitiveVariable.Interpolation.FaceVarying

		# add all valid detail attrib types
		mesh["floatDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, floatData )
		mesh["v2fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v2fData )
		mesh["v3fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v3fData )
		mesh["color3fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, color3fData )
		mesh["intDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, intData )
		mesh["v2iDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v2iData )
		mesh["v3iDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v3iData )
		mesh["stringDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, stringData )

		# add all valid point attrib types
		pData = IECore.V3fVectorData( [
			imath.V3f( 0, 1, 2 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ),
			imath.V3f( 4 ), imath.V3f( 5 ), imath.V3f( 6 ), imath.V3f( 7 ),
		], IECore.GeometricData.Interpretation.Point )
		mesh["P"] = IECoreScene.PrimitiveVariable( pointInterpolation, pData )
		mesh["floatPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, floatVectorData[:8] )
		mesh["v2fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v2fVectorData[:8] )
		mesh["v3fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v3fVectorData[:8] )
		mesh["color3fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, color3fVectorData[:8] )
		mesh["intPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, intVectorData[:8] )
		mesh["v2iPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v2iVectorData[:8] )
		mesh["v3iPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v3iVectorData[:8] )
		mesh["stringPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, stringVectorData[:8], IECore.IntVectorData( list(range( 0, 8)) ) )

		# add all valid primitive attrib types
		mesh["floatPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, floatVectorData[:6] )
		mesh["v2fPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, v2fVectorData[:6] )
		mesh["v3fPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, v3fVectorData[:6] )
		mesh["color3fPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, color3fVectorData[:6] )
		mesh["intPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, intVectorData[:6] )
		mesh["v2iPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, v2iVectorData[:6] )
		mesh["v3iPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, v3iVectorData[:6] )
		mesh["stringPrim"] = IECoreScene.PrimitiveVariable( primitiveInterpolation, stringVectorData[:6], IECore.IntVectorData( list(range( 0, 6)) ) )

		# add all valid vertex attrib types
		mesh["floatVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, floatVectorData )
		mesh["v2fVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, v2fVectorData )
		mesh["v3fVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, v3fVectorData )
		mesh["color3fVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, color3fVectorData )
		mesh["intVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, intVectorData )
		mesh["v2iVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, v2iVectorData )
		mesh["v3iVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, v3iVectorData )
		mesh["stringVert"] = IECoreScene.PrimitiveVariable( vertexInterpolation, stringVectorData, IECore.IntVectorData( list(range( 0, 24)) ) )

		return mesh

	def emptySop( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		null = geo.createNode( "null" )

		return null

	def meshSop( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		box = geo.createNode( "box" )
		facet = box.createOutputNode( "facet" )
		facet.parm( "postnml" ).set(True)

		return facet

	def comparePrimAndSop( self, prim, sop ) :
		geo = sop.geometry()
		for key in [ "floatDetail", "intDetail", "stringDetail" ] :
			self.assertEqual( prim[key].data.value, geo.attribValue( key ) )

		for key in [ "v2fDetail", "v3fDetail", "color3fDetail", "v2iDetail", "v3iDetail" ] :
			self.assertEqual( tuple(prim[key].data.value), geo.attribValue( key ) )

		sopPoints = geo.points()
		for key in [ "floatPoint", "intPoint" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPoints[i].attribValue( key ) )

		for key in [ "P", "v2fPoint", "v3fPoint", "color3fPoint", "v2iPoint", "v3iPoint" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPoints[i].attribValue( key ) )

		data = prim["stringPoint"].data
		dataIndices = prim["stringPoint"].indices
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPoints[i].attribValue( "stringPoint" ) )

		sopPrims = geo.prims()
		self.assertEqual( len(sopPrims), prim.numFaces() )

		for key in [ "floatPrim", "intPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPrims[i].attribValue( key ) )

		for key in [ "v2fPrim", "v3fPrim", "color3fPrim", "v2iPrim", "v3iPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPrims[i].attribValue( key ) )

		data = prim["stringPrim"].data
		dataIndices = prim["stringPrim"].indices
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPrims[i].attribValue( "stringPrim" ) )

		sopVerts = []
		for i in range( 0, len(sopPrims) ) :
			verts = list(sopPrims[i].vertices())
			self.assertEqual( len(verts), prim.verticesPerFace[i] )
			verts.reverse()
			sopVerts.extend( verts )

		self.assertEqual( len(sopVerts), prim.vertexIds.size() )
		for i in range( 0, len(sopVerts) ) :
			self.assertEqual( sopVerts[i].point().number(), prim.vertexIds[i] )

		for key in [ "floatVert", "intVert" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopVerts[i].attribValue( key ) )

		for key in [ "v2fVert", "v3fVert", "color3fVert", "v2iVert", "v3iVert" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopVerts[i].attribValue( key ) )

		data = prim["stringVert"].data
		dataIndices = prim["stringVert"].indices
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopVerts[i].attribValue( "stringVert" ) )

		self.assertTrue( geo.findGlobalAttrib( "v2fDetail" ).isTransformedAsVector() )
		self.assertTrue( geo.findPointAttrib( "v3fPoint" ).isTransformedAsNormal() )
		self.assertTrue( geo.findPrimAttrib( "v3fPrim" ).isTransformedAsNormal() )
		self.assertTrue( geo.findVertexAttrib( "v3fVert" ).isTransformedAsNormal() )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertEqual( result.verticesPerFace, prim.verticesPerFace )
		self.assertEqual( result.vertexIds, prim.vertexIds )
		self.assertEqual( result.keys(), prim.keys() )
		for key in prim.keys() :
			self.assertEqual( result[key], prim[key] )
		self.assertEqual( result, prim )

		self.assertTrue( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertTrue( result["v2fDetail"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertTrue( result["v3fPoint"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertTrue( result["v3fPrim"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertTrue( result["v3fVert"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

	def comparePrimAndAppendedSop( self, prim, sop, origSopPrim, multipleConversions=False ) :
		geo = sop.geometry()
		for key in [ "floatDetail", "intDetail", "stringDetail", "stringDetail" ] :
			self.assertEqual( prim[key].data.value, geo.attribValue( key ) )

		for key in [ "v2fDetail", "v3fDetail", "color3fDetail", "v2iDetail", "v3iDetail" ] :
			self.assertEqual( tuple(prim[key].data.value), geo.attribValue( key ) )

		sopPoints = geo.points()
		numPoints = prim.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		origNumPoints = origSopPrim.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( len(sopPoints), origNumPoints + numPoints )

		for key in [ "floatPoint", "intPoint" ] :
			data = prim[key].data

			if multipleConversions :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ 0 ] * origNumPoints

			for i in range( 0, origNumPoints ) :
				self.assertEqual( defaultValue[i], sopPoints[ i ].attribValue( key ) )

			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPoints[ origNumPoints + i ].attribValue( key ) )

		for key in [ "P", "v2fPoint", "v3fPoint", "color3fPoint", "v2iPoint", "v3iPoint" ] :
			data = prim[key].data

			if multipleConversions or key is "P" :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ [ 0 ] * data[0].dimensions() ] * origNumPoints

			for i in range( 0, origNumPoints ) :
				self.assertEqual( tuple(defaultValue[i]), sopPoints[ i ].attribValue( key ) )

			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPoints[ origNumPoints + i ].attribValue( key ) )

		data = prim["stringPoint"].data
		dataIndices = prim["stringPoint"].indices

		if multipleConversions :
			defaultData = origSopPrim["stringPoint"].data
			defaultIndices = origSopPrim["stringPoint"].indices
			for i in range( 0, origNumPoints ) :
				val = "" if ( defaultIndices[i] >= defaultData.size() ) else defaultData[ defaultIndices[i] ]
				self.assertEqual( val, sopPoints[ i ].attribValue( "stringPoint" ) )
		else :
			defaultValues = [ "" ] * origNumPoints
			for i in range( 0, origNumPoints ) :
				self.assertEqual( defaultValues[i], sopPoints[ i ].attribValue( "stringPoint" ) )

		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPoints[ origNumPoints + i ].attribValue( "stringPoint" ) )

		sopPrims = geo.prims()
		origNumPrims = origSopPrim.numFaces()
		self.assertEqual( len(sopPrims), origNumPrims + prim.numFaces() )

		for key in [ "floatPrim", "intPrim" ] :
			data = prim[key].data

			if multipleConversions :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ 0 ] * origNumPrims

			for i in range( 0, origNumPrims ) :
				self.assertEqual( defaultValue[i], sopPrims[ i ].attribValue( key ) )

			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPrims[ origNumPrims + i ].attribValue( key ) )

		for key in [ "v2fPrim", "v3fPrim", "color3fPrim", "v2iPrim", "v3iPrim" ] :
			data = prim[key].data

			if multipleConversions :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ [ 0 ] * data[0].dimensions() ] * origNumPrims

			for i in range( 0, origNumPrims ) :
				self.assertEqual( tuple(defaultValue[i]), sopPrims[ i ].attribValue( key ) )

			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPrims[ origNumPrims + i ].attribValue( key ) )

		data = prim["stringPrim"].data
		dataIndices = prim["stringPrim"].indices

		if multipleConversions :
			defaultData = origSopPrim["stringPrim"].data
			defaultIndices = origSopPrim["stringPrim"].indices
			for i in range( 0, origNumPrims ) :
				val = "" if ( defaultIndices[i] >= defaultData.size() ) else defaultData[ defaultIndices[i] ]
				self.assertEqual( val, sopPrims[ i ].attribValue( "stringPrim" ) )
		else :
			defaultValues = [ "" ] * origNumPrims
			for i in range( 0, origNumPrims ) :
				self.assertEqual( defaultValues[i], sopPrims[ i ].attribValue( "stringPrim" ) )

		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPrims[ origNumPrims + i ].attribValue( "stringPrim" ) )

		sopVerts = []
		for i in range( 0, len(sopPrims) ) :
			verts = list(sopPrims[i].vertices())
			verts.reverse()
			sopVerts.extend( verts )
			if i > origNumPrims :
				self.assertEqual( len(verts), prim.verticesPerFace[i-origNumPrims] )

		origNumVerts = origSopPrim.vertexIds.size()
		self.assertEqual( len(sopVerts), origNumVerts + prim.vertexIds.size() )
		for i in range( 0, len(prim.vertexIds) ) :
			self.assertEqual( sopVerts[origNumVerts+i].point().number() - origNumPoints, prim.vertexIds[i] )

		for key in [ "floatVert", "intVert" ] :
			data = prim[key].data

			if multipleConversions :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ 0 ] * origNumVerts

			for i in range( 0, origNumVerts ) :
				self.assertEqual( defaultValue[i], sopVerts[ i ].attribValue( key ) )

			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopVerts[ origNumVerts + i ].attribValue( key ) )

		for key in [ "v2fVert", "v3fVert", "color3fVert", "v2iVert", "v3iVert" ] :
			data = prim[key].data

			if multipleConversions :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ [ 0 ] * data[0].dimensions() ] * origNumVerts

			for i in range( 0, origNumVerts ) :
				self.assertEqual( tuple(defaultValue[i]), sopVerts[ i ].attribValue( key ) )

			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopVerts[ origNumVerts + i ].attribValue( key ) )

		data = prim["stringVert"].data
		dataIndices = prim["stringVert"].indices

		if multipleConversions :
			defaultData = origSopPrim["stringVert"].data
			defaultIndices = origSopPrim["stringVert"].indices
			for i in range( 0, origNumVerts ) :
				val = "" if ( defaultIndices[i] >= defaultData.size() ) else defaultData[ defaultIndices[i] ]
				self.assertEqual( val, sopVerts[ i ].attribValue( "stringVert" ) )
		else :
			defaultValues = [ "" ] * origNumVerts
			for i in range( 0, origNumVerts ) :
				self.assertEqual( defaultValues[i], sopVerts[ i ].attribValue( "stringVert" ) )

		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopVerts[ origNumVerts + i ].attribValue( "stringVert" ) )

		self.assertTrue( geo.findGlobalAttrib( "v2fDetail" ).isTransformedAsVector() )
		self.assertTrue( geo.findPointAttrib( "v3fPoint" ).isTransformedAsNormal() )
		self.assertTrue( geo.findPrimAttrib( "v3fPrim" ).isTransformedAsNormal() )
		self.assertTrue( geo.findVertexAttrib( "v3fVert" ).isTransformedAsNormal() )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertEqual( result.verticesPerFace[origNumPrims:], prim.verticesPerFace )
		for i in range( 0, len(prim.vertexIds) ) :
			self.assertEqual( result.vertexIds[origNumVerts + i], prim.vertexIds[i] + origNumPoints )
		for key in prim.keys() :
			self.assertTrue( key in result.keys() )

		self.assertTrue( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertTrue( result["v2fDetail"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertTrue( result["v3fPoint"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertTrue( result["v3fPrim"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertTrue( result["v3fVert"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )

	def testCreateConverter( self )  :
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( self.mesh() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPolygonsConverter ) ) )

	def testFactory( self ) :
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.mesh() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPolygonsConverter ) ) )

		self.assertTrue( IECoreScene.TypeId.MeshPrimitive in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )

	def testConversionIntoEmptySop( self ) :
		mesh = self.mesh()
		sop = self.emptySop()

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )

		self.comparePrimAndSop( mesh, sop )

	def testConversionIntoExistingSop( self ) :
		mesh = self.mesh()
		sop = self.meshSop()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, mesh )

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, False ) )

		self.comparePrimAndSop( mesh, sop )

	def testAppendingIntoExistingSop( self ) :
		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )

		self.assertTrue( not sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, sop, orig )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints )
		self.assertTrue( "floatDetail" not in result.keys() )
		self.assertTrue( "floatPoint" not in result.keys() )

	def testAppendingIntoLockedSop( self ) :
		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )

		sop.setHardLocked( True )
		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, sop, orig )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints )
		self.assertTrue( "floatDetail" not in result.keys() )
		self.assertTrue( "floatPoint" not in result.keys() )

	def testSaveLoad( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )

		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()
		sopPath = sop.path()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )

		self.assertTrue( not sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, sop, orig )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

		hou.hipFile.save( TestToHoudiniPolygonsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPolygonsConverter.__testScene )

		newSop = hou.node( sopPath )

		self.assertTrue( newSop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, newSop, orig )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( newSop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

	def testSaveLoadWithLockedSop( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )

		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()
		sopPath = sop.path()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )

		sop.setHardLocked( True )
		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, sop, orig )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

		hou.hipFile.save( TestToHoudiniPolygonsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPolygonsConverter.__testScene )

		newSop = hou.node( sopPath )

		self.assertTrue( newSop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, newSop, orig )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( newSop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

	def testMultipleConversions( self ) :

		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()

		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )

		self.assertTrue( not sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, sop, orig )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, sop, result, multipleConversions=True )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + 2*meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + meshNumPoints + i ], mesh["P"].data[i] )

		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( mesh, sop, result, multipleConversions=True )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + 3*meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + meshNumPoints + i ], mesh["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + 2*meshNumPoints + i ], mesh["P"].data[i] )

	def testObjectWasDeleted( self ) :
		mesh = self.mesh()
		sop = self.meshSop()

		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )

		self.assertTrue( converter.convert( sop, False ) )

		self.comparePrimAndSop( mesh, sop )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()

		del mesh

		sop.setHardLocked( False )
		self.assertNotEqual( IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert(), result )
		self.assertTrue( converter.convert( sop, False ) )
		self.assertEqual( IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert(), result )

	def testWithUnacceptablePrimVars( self ) :
		mesh = self.mesh()
		mesh["badDetail"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.TransformationMatrixfData() )
		mesh["badPoint"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		mesh["badPrim"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Uniform, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		mesh["badVert"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		sop = self.emptySop()

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )

		self.assertTrue( "badDetail" not in [ x.name() for x in sop.geometry().globalAttribs() ] )
		self.assertTrue( "badPoint" not in [ x.name() for x in sop.geometry().pointAttribs() ] )
		self.assertTrue( "badPrim" not in [ x.name() for x in sop.geometry().primAttribs() ] )
		self.assertTrue( "badVert" not in [ x.name() for x in sop.geometry().vertexAttribs() ] )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( result, mesh )
		self.assertTrue( "badDetail" not in result )
		self.assertTrue( "badPoint" not in result )
		self.assertTrue( "badPrim" not in result )
		self.assertTrue( "badVert" not in result )

		del mesh["badDetail"]
		del mesh["badPoint"]
		del mesh["badPrim"]
		del mesh["badVert"]
		self.comparePrimAndSop( mesh, sop )

	def testConvertingOverExistingAttribs( self ) :
		mesh = self.mesh()
		sop = self.emptySop()
		detailAttr = sop.createOutputNode( "attribcreate", exact_type_name=True )
		detailAttr.parm( "name" ).set( "floatDetail" )
		detailAttr.parm( "class" ).set( 0 ) # detail
		detailAttr.parm( "type" ).set( 0 ) # float
		detailAttr.parm( "size" ).set( 1 ) # 1 element
		detailAttr.parm( "value1" ).set( 123.456 )

		pointAttr = detailAttr.createOutputNode( "attribcreate", exact_type_name=True )
		pointAttr.parm( "name" ).set( "floatPoint" )
		pointAttr.parm( "class" ).set( 2 ) # point
		pointAttr.parm( "type" ).set( 0 ) # float
		pointAttr.parm( "size" ).set( 1 ) # 1 element
		pointAttr.parm( "value1" ).set( 123.456 )

		primAttr = pointAttr.createOutputNode( "attribcreate", exact_type_name=True )
		primAttr.parm( "name" ).set( "floatPrim" )
		primAttr.parm( "class" ).set( 1 ) # prim
		primAttr.parm( "type" ).set( 0 ) # float
		primAttr.parm( "size" ).set( 1 ) # 1 element
		primAttr.parm( "value1" ).set( 123.456 )

		vertexAttr = primAttr.createOutputNode( "attribcreate", exact_type_name=True )
		vertexAttr.parm( "name" ).set( "floatVert" )
		vertexAttr.parm( "class" ).set( 3 ) # vertex
		vertexAttr.parm( "type" ).set( 0 ) # float
		vertexAttr.parm( "size" ).set( 1 ) # 1 element
		vertexAttr.parm( "value1" ).set( 123.456 )

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( vertexAttr ) )
		self.comparePrimAndSop( mesh, vertexAttr )

	def testConvertingOverExistingAttribsWithDifferentTypes( self ) :
		mesh = self.mesh()
		sop = self.emptySop()
		detailAttr = sop.createOutputNode( "attribcreate", exact_type_name=True )
		detailAttr.parm( "name" ).set( "floatDetail" )
		detailAttr.parm( "class" ).set( 0 ) # detail
		detailAttr.parm( "type" ).set( 1 ) # int
		detailAttr.parm( "size" ).set( 3 ) # 3 elements
		detailAttr.parm( "value1" ).set( 10 )
		detailAttr.parm( "value2" ).set( 11 )
		detailAttr.parm( "value3" ).set( 12 )

		pointAttr = detailAttr.createOutputNode( "attribcreate", exact_type_name=True )
		pointAttr.parm( "name" ).set( "floatPoint" )
		pointAttr.parm( "class" ).set( 2 ) # point
		pointAttr.parm( "type" ).set( 1 ) # int
		pointAttr.parm( "size" ).set( 3 ) # 3 elements
		pointAttr.parm( "value1" ).set( 10 )
		pointAttr.parm( "value2" ).set( 11 )
		pointAttr.parm( "value3" ).set( 12 )

		primAttr = pointAttr.createOutputNode( "attribcreate", exact_type_name=True )
		primAttr.parm( "name" ).set( "floatPrim" )
		primAttr.parm( "class" ).set( 1 ) # prim
		primAttr.parm( "type" ).set( 1 ) # int
		primAttr.parm( "size" ).set( 3 ) # 3 elements
		primAttr.parm( "value1" ).set( 10 )
		primAttr.parm( "value2" ).set( 11 )
		primAttr.parm( "value3" ).set( 12 )

		vertexAttr = primAttr.createOutputNode( "attribcreate", exact_type_name=True )
		vertexAttr.parm( "name" ).set( "floatVert" )
		vertexAttr.parm( "class" ).set( 3 ) # vert
		vertexAttr.parm( "type" ).set( 1 ) # int
		vertexAttr.parm( "size" ).set( 3 ) # 3 elements
		vertexAttr.parm( "value1" ).set( 10 )
		vertexAttr.parm( "value2" ).set( 11 )
		vertexAttr.parm( "value3" ).set( 12 )

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( vertexAttr ) )
		self.comparePrimAndSop( mesh, vertexAttr )

	def testEmptyString( self ) :
		mesh = self.mesh()
		sop = self.emptySop()
		mesh['stringPoint'].data[0] = ""

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )

		geo = sop.geometry()
		sopPoints = geo.points()
		data = mesh["stringPoint"].data
		dataIndices = mesh["stringPoint"].indices
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPoints[i].attribValue( "stringPoint" ) )

		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertEqual( result.verticesPerFace, mesh.verticesPerFace )
		self.assertEqual( result.vertexIds, mesh.vertexIds )
		self.assertEqual( result.keys(), mesh.keys() )

		self.assertEqual( result["stringPoint"], mesh["stringPoint"] )

	def testName( self ) :

		sop = self.emptySop()
		mesh = self.mesh()
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )

		# unnamed unless we set the parameter
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sop.geometry().findPrimAttrib( "name" ), None )

		converter["name"].setTypedValue( "testMesh" )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testMesh" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testMesh" ]), mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

		# blindData still works for backwards compatibility
		mesh.blindData()["name"] = IECore.StringData( "blindMesh" )
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "blindMesh" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "blindMesh" ]), mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

		# name parameter takes preference over blindData
		converter["name"].setTypedValue( "testMesh" )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testMesh" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testMesh" ]), mesh.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

	def testAttributeFilter( self ) :

		mesh = self.mesh()
		sop = self.emptySop()

		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniPolygonsConverter.PointPositionAttribs + ['color3fPoint', 'floatPoint', 'intPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'ieMeshInterpolation', 'intPrim', 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['color3fVert', 'floatVert', 'intVert', 'stringVert', 'v2fVert', 'v2iVert', 'v3fVert', 'v3iVert'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'floatDetail', 'intDetail', 'stringDetail', 'v2fDetail', 'v2iDetail', 'v3fDetail', 'v3iDetail'] )

		converter.parameters()["attributeFilter"].setTypedValue( "P *3f*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniPolygonsConverter.PointPositionAttribs + ['color3fPoint', 'v3fPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'ieMeshInterpolation', 'v3fPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['color3fVert', 'v3fVert'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'v3fDetail'] )

		converter.parameters()["attributeFilter"].setTypedValue( "* ^*Detail ^int*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniPolygonsConverter.PointPositionAttribs + ['color3fPoint', 'floatPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'ieMeshInterpolation', 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['color3fVert', 'floatVert', 'stringVert', 'v2fVert', 'v2iVert', 'v3fVert', 'v3iVert'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )

		# verify we can filter uvs
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		mesh = IECoreScene.MeshAlgo.triangulate( mesh )
		IECoreScene.MeshNormalsOp()( input=mesh, copyInput=False )
		mesh["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.V3fVectorData( [ imath.V3f( 1, 0, 0 ) ] * 6, IECore.GeometricData.Interpretation.Color ) )
		mesh["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 4 ) )
		mesh["Pref"] = mesh["P"]

		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		converter.parameters()["attributeFilter"].setTypedValue( "*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), sorted( TestToHoudiniPolygonsConverter.PointPositionAttribs + ['N', 'pscale', 'rest'] ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )

		# have to filter the source attrs
		converter.parameters()["attributeFilter"].setTypedValue( "* ^uv ^pscale ^rest" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), sorted( TestToHoudiniPolygonsConverter.PointPositionAttribs + ['N', 'pscale', 'rest'] ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )

		converter.parameters()["attributeFilter"].setTypedValue( "* ^width ^Pref" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]),  sorted( TestToHoudiniPolygonsConverter.PointPositionAttribs + ['N'] ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )

	def testStandardAttributeConversion( self ) :

		sop = self.emptySop()
		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		mesh = IECoreScene.MeshAlgo.triangulate( mesh )
		IECoreScene.MeshNormalsOp()( input=mesh, copyInput=False )
		mesh["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.V3fVectorData( [ imath.V3f( 1, 0, 0 ) ] * 6, IECore.GeometricData.Interpretation.Color ) )
		mesh["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 4 ) )
		mesh["Pref"] = mesh["P"]

		self.assertTrue( mesh.arePrimitiveVariablesValid() )

		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sorted([ x.name() for x in geo.pointAttribs() ]), sorted( TestToHoudiniPolygonsConverter.PointPositionAttribs + ['N', 'pscale', 'rest'] ) )
		self.assertEqual( sorted([ x.name() for x in geo.primAttribs() ]), ['ieMeshInterpolation'] )
		self.assertEqual( sorted([ x.name() for x in geo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in geo.globalAttribs() ]), [] )

		uvData = mesh["uv"].data
		indices = mesh["uv"].indices

		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvValues[0], uvData[indices[i]][0] )
				self.assertAlmostEqual( uvValues[1], uvData[indices[i]][1] )
				i += 1

		converter["convertStandardAttributes"].setTypedValue( False )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sorted([ x.name() for x in geo.pointAttribs() ]), sorted( TestToHoudiniPolygonsConverter.PointPositionAttribs + ['N', 'Pref', 'width'] ) )
		self.assertEqual( sorted([ x.name() for x in geo.primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in geo.vertexAttribs() ]), ['Cs', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in geo.globalAttribs() ]), [] )

		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvValues[0], uvData[indices[i]][0] )
				self.assertAlmostEqual( uvValues[1], uvData[indices[i]][1] )
				i += 1

	def testCannotTransformRest( self ) :

		sop = self.emptySop()
		mergeGeo = hou.node( "/obj" ).createNode( "geo", run_init_scripts=False )
		mergeGeo.parm( "tx" ).set( 10 )
		merge = mergeGeo.createNode( "object_merge" )
		merge.parm( "xformtype" ).set( 1 )
		merge.parm( "objpath1" ).set( sop.path() )

		mesh = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( 0 ), imath.V2f( 1 ) ) )
		mesh = IECoreScene.MeshAlgo.triangulate( mesh )
		IECoreScene.MeshNormalsOp()( input=mesh, copyInput=False )
		mesh["Pref"] = mesh["P"]
		prefData = mesh["Pref"].data
		self.assertTrue( mesh.arePrimitiveVariablesValid() )

		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		geo2 = merge.geometry()

		i = 0
		for point in geo.points() :
			restValue = point.attribValue( "rest" )
			self.assertAlmostEqual( imath.V3f( restValue[0], restValue[1], restValue[2] ), prefData[i] )
			self.assertTrue( point.position().isAlmostEqual( hou.Vector3(restValue) ) )
			i += 1

		i = 0
		for point in geo2.points() :
			restValue = point.attribValue( "rest" )
			self.assertAlmostEqual( imath.V3f( restValue[0], restValue[1], restValue[2] ), prefData[i] )
			self.assertFalse( point.position().isAlmostEqual( hou.Vector3(restValue) ) )
			i += 1

		# Pref shouldn't transform either
		converter["convertStandardAttributes"].setTypedValue( False )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		geo2 = merge.geometry()

		i = 0
		for point in geo.points() :
			restValue = point.attribValue( "Pref" )
			self.assertAlmostEqual( imath.V3f( restValue[0], restValue[1], restValue[2] ), prefData[i] )
			self.assertTrue( point.position().isAlmostEqual( hou.Vector3(restValue) ) )
			i += 1

		i = 0
		for point in geo2.points() :
			restValue = point.attribValue( "Pref" )
			self.assertAlmostEqual( imath.V3f( restValue[0], restValue[1], restValue[2] ), prefData[i] )
			self.assertFalse( point.position().isAlmostEqual( hou.Vector3(restValue) ) )
			i += 1

	def testInterpolation( self ) :

		mesh = self.mesh()
		sop = self.emptySop()
		self.assertEqual( mesh.interpolation, "linear" )
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )
		self.assertTrue( "ieMeshInterpolation" in [ x.name() for x in sop.geometry().primAttribs() ] )
		attrib = sop.geometry().findPrimAttrib( "ieMeshInterpolation" )
		for prim in sop.geometry().prims() :
			self.assertEqual( prim.attribValue( attrib ), "poly" )

		mesh.interpolation = "catmullClark"
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )
		self.assertTrue( "ieMeshInterpolation" in [ x.name() for x in sop.geometry().primAttribs() ] )
		attrib = sop.geometry().findPrimAttrib( "ieMeshInterpolation" )
		for prim in sop.geometry().prims() :
			self.assertEqual( prim.attribValue( attrib ), "subdiv" )

	def testExpandedUVRoundTrip( self ) :

		mesh = IECore.Reader.create( "test/IECore/data/cobFiles/twoTrianglesWithSharedUVs.cob" ).read()
		mesh["uv"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, mesh["uv"].expandedData(), None )
		mesh["uv"].indices = None
		uvData = mesh["uv"].data

		sop = self.emptySop()

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )

		geo = sop.geometry()
		self.assertTrue( "uv" in [ x.name() for x in geo.vertexAttribs() ] )
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvValues[0], uvData[i][0] )
				self.assertAlmostEqual( uvValues[1], uvData[i][1] )
				i += 1

		converter = IECoreHoudini.FromHoudiniPolygonsConverter( sop )
		result = converter.convert()
		self.assertEqual( result["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		# we cannot guarantee to generate the same data when extracting from Houdini
		# because we always generate indices, but we can generate correctly indexed data
		self.assertEqual( result["uv"].data.size(), 4 )
		self.assertEqual( result["uv"].indices.size(), 6 )
		for i in range( 0, mesh.variableSize( mesh["uv"].interpolation ) ) :
			self.assertEqual( mesh["uv"].data[i], result["uv"].data[ result["uv"].indices[i] ] )

	def testIndexedUVRoundTrip( self ) :

		mesh = IECore.Reader.create( "test/IECore/data/cobFiles/twoTrianglesWithSharedUVs.cob" ).read()
		uvData = mesh["uv"].data
		uvIndices = mesh["uv"].indices

		sop = self.emptySop()

		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )

		geo = sop.geometry()
		self.assertTrue( "uv" in [ x.name() for x in geo.vertexAttribs() ] )
		uvs = geo.findVertexAttrib( "uv" )

		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvValues[0], uvData[uvIndices[i]][0] )
				self.assertAlmostEqual( uvValues[1], uvData[uvIndices[i]][1] )
				i += 1

		converter = IECoreHoudini.FromHoudiniPolygonsConverter( sop )
		result = converter.convert()
		self.assertEqual( result["uv"].data.getInterpretation(), IECore.GeometricData.Interpretation.UV )
		# we cannot guarantee to generate the same indices when extracting from Houdini
		# nor the same data, but we can generate correctly indexed data
		self.assertEqual( result["uv"].data.size(), 4 )
		self.assertEqual( result["uv"].indices.size(), 6 )
		for i in range( 0, mesh.variableSize( mesh["uv"].interpolation ) ) :
			self.assertEqual( mesh["uv"].data[ mesh["uv"].indices[i] ], result["uv"].data[ result["uv"].indices[i] ] )

	def testCornersAndCreases( self ) :

		mesh = IECoreScene.MeshPrimitive.createBox( imath.Box3f( imath.V3f( -1 ), imath.V3f( 1 ) ) )
		# normals and UVs complicate the testing, and we don't need them to verify corners and creases
		del mesh["N"]
		del mesh["uv"]
		cornerIds = [ 5 ]
		cornerSharpnesses = [ 10.0 ]
		mesh.setCorners( IECore.IntVectorData( cornerIds ), IECore.FloatVectorData( cornerSharpnesses ) )
		creaseLengths = [ 3, 2 ]
		creaseIds = [ 1, 2, 3, 4, 5 ]  # note that these are vertex ids
		creaseSharpnesses = [ 1, 5 ]
		mesh.setCreases( IECore.IntVectorData( creaseLengths ), IECore.IntVectorData( creaseIds ), IECore.FloatVectorData( creaseSharpnesses ) )

		sop = self.emptySop()
		self.assertTrue( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )

		geo = sop.geometry()
		self.assertTrue( "cornerweight" in [ x.name() for x in geo.pointAttribs() ] )
		self.assertTrue( "creaseweight" in [ x.name() for x in geo.vertexAttribs() ] )

		# test corners
		cornerWeight = geo.findPointAttrib( "cornerweight" )
		for point in geo.points() :
			sharpness = 0.0
			if point.number() in cornerIds :
				sharpness = cornerSharpnesses[ cornerIds.index( point.number() ) ]
			self.assertEqual( point.attribValue( cornerWeight ), sharpness )

		# test creases
		expectedSharpnesses = [ 0 ] * 24
		# edge 1-2
		expectedSharpnesses[1] = 1
		expectedSharpnesses[2] = 1
		# edge 2-3
		expectedSharpnesses[6] = 1
		expectedSharpnesses[18] = 1
		# edge 4-5
		expectedSharpnesses[4] = 5
		expectedSharpnesses[10] = 5

		self.assertEqual( list(geo.vertexFloatAttribValues( "creaseweight" )), expectedSharpnesses )

		# make sure it round trips well enough
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertEqual( result.cornerIds(), mesh.cornerIds() )
		self.assertEqual( result.cornerSharpnesses(), mesh.cornerSharpnesses() )
		self.assertEqual( result.creaseLengths(), IECore.IntVectorData( [ 2, 2, 2 ] ) )
		self.assertEqual( result.creaseIds(), IECore.IntVectorData( [ 2, 3, 1, 2, 4, 5 ] ) )
		self.assertEqual( result.creaseSharpnesses(), IECore.FloatVectorData( [ 1, 1, 5 ] ) )
		# if we re-align result creases, everything else is an exact match
		mesh.setCreases( result.creaseLengths(), result.creaseIds(), result.creaseSharpnesses() )
		self.assertEqual( result, mesh )

	def tearDown( self ) :

		if os.path.isfile( TestToHoudiniPolygonsConverter.__testScene ) :
			os.remove( TestToHoudiniPolygonsConverter.__testScene )

if __name__ == "__main__":
    unittest.main()
