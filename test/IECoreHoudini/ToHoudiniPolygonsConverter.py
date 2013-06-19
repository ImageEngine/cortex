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
import IECore
import IECoreHoudini
import unittest
import os

class TestToHoudiniPolygonsConverter( IECoreHoudini.TestCase ) :
	
	__testScene = "test/converterTest.hip"
	
	def mesh( self ) :
		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [ 1, 5, 4, 0, 2, 6, 5, 1, 3, 7, 6, 2, 0, 4, 7, 3, 2, 1, 0, 3, 5, 6, 7, 4 ] )
		mesh = IECore.MeshPrimitive( vertsPerFace, vertexIds )
		
		floatData = IECore.FloatData( 1.5 )
		v2fData = IECore.V2fData( IECore.V2f( 1.5, 2.5 ), IECore.GeometricData.Interpretation.Vector )
		v3fData = IECore.V3fData( IECore.V3f( 1.5, 2.5, 3.5 ) )
		color3fData = IECore.Color3fData( IECore.Color3f( 1.5, 2.5, 3.5 ) )
		intData = IECore.IntData( 1 )
		v2iData = IECore.V2iData( IECore.V2i( 1, 2 ) )
		v3iData = IECore.V3iData( IECore.V3i( 1, 2, 3 ) )
		stringData = IECore.StringData( "this is a string" )
		
		intRange = range( 1, 25 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		v2fVectorData = IECore.V2fVectorData( [ IECore.V2f( x, x+0.5 ) for x in intRange ] )
		v3fVectorData = IECore.V3fVectorData( [ IECore.V3f( x, x+0.5, x+0.75 ) for x in intRange ], IECore.GeometricData.Interpretation.Normal )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		intVectorData = IECore.IntVectorData( intRange )
		v2iVectorData = IECore.V2iVectorData( [ IECore.V2i( x, -x ) for x in intRange ] )
		v3iVectorData = IECore.V3iVectorData( [ IECore.V3i( x, -x, x*2 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %d!" % x for x in intRange ] )
		
		detailInterpolation = IECore.PrimitiveVariable.Interpolation.Constant
		pointInterpolation = IECore.PrimitiveVariable.Interpolation.Vertex
		primitiveInterpolation = IECore.PrimitiveVariable.Interpolation.Uniform
		vertexInterpolation = IECore.PrimitiveVariable.Interpolation.FaceVarying
		
		# add all valid detail attrib types
		mesh["floatDetail"] = IECore.PrimitiveVariable( detailInterpolation, floatData )
		mesh["v2fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2fData )
		mesh["v3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3fData )
		mesh["color3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, color3fData )
		mesh["intDetail"] = IECore.PrimitiveVariable( detailInterpolation, intData )
		mesh["v2iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2iData )
		mesh["v3iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3iData )
		mesh["stringDetail"] = IECore.PrimitiveVariable( detailInterpolation, stringData )
		
		# add all valid point attrib types
		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
		], IECore.GeometricData.Interpretation.Point )
		mesh["P"] = IECore.PrimitiveVariable( pointInterpolation, pData )
		mesh["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatVectorData[:8] )
		mesh["v2fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2fVectorData[:8] )
		mesh["v3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3fVectorData[:8] )
		mesh["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fVectorData[:8] )
		mesh["intPoint"] = IECore.PrimitiveVariable( pointInterpolation, intVectorData[:8] )
		mesh["v2iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2iVectorData[:8] )
		mesh["v3iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3iVectorData[:8] )
		mesh["stringPoint"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:8] )
		mesh["stringPointIndices"] = IECore.PrimitiveVariable( pointInterpolation, IECore.IntVectorData( range( 0, 8 ) ) )
		
		# add all valid primitive attrib types
		mesh["floatPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, floatVectorData[:6] )
		mesh["v2fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v2fVectorData[:6] )
		mesh["v3fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v3fVectorData[:6] )
		mesh["color3fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, color3fVectorData[:6] )
		mesh["intPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, intVectorData[:6] )
		mesh["v2iPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v2iVectorData[:6] )
		mesh["v3iPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v3iVectorData[:6] )
		mesh["stringPrim"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:6] )
		mesh["stringPrimIndices"] = IECore.PrimitiveVariable( primitiveInterpolation, IECore.IntVectorData( range( 0, 6 ) ) )
		
		# add all valid vertex attrib types
		mesh["floatVert"] = IECore.PrimitiveVariable( vertexInterpolation, floatVectorData )
		mesh["v2fVert"] = IECore.PrimitiveVariable( vertexInterpolation, v2fVectorData )
		mesh["v3fVert"] = IECore.PrimitiveVariable( vertexInterpolation, v3fVectorData )
		mesh["color3fVert"] = IECore.PrimitiveVariable( vertexInterpolation, color3fVectorData )
		mesh["intVert"] = IECore.PrimitiveVariable( vertexInterpolation, intVectorData )
		mesh["v2iVert"] = IECore.PrimitiveVariable( vertexInterpolation, v2iVectorData )
		mesh["v3iVert"] = IECore.PrimitiveVariable( vertexInterpolation, v3iVectorData )
		mesh["stringVert"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData )
		mesh["stringVertIndices"] = IECore.PrimitiveVariable( vertexInterpolation, IECore.IntVectorData( range( 0, 24 ) ) )
		
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
		dataIndices = prim["stringPointIndices"].data
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
		dataIndices = prim["stringPrimIndices"].data
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
		dataIndices = prim["stringVertIndices"].data
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
		numPoints = prim.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		origNumPoints = origSopPrim.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
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
		dataIndices = prim["stringPointIndices"].data
		
		if multipleConversions :
			defaultIndices = origSopPrim["stringPointIndices"].data
			for i in range( 0, origNumPoints ) :
				val = "" if ( defaultIndices[i] >= data.size() ) else data[ defaultIndices[i] ]
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
		dataIndices = prim["stringPrimIndices"].data
		
		if multipleConversions :
			defaultIndices = origSopPrim["stringPrimIndices"].data
			for i in range( 0, origNumPrims ) :
				val = "" if ( defaultIndices[i] >= data.size() ) else data[ defaultIndices[i] ]
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
		dataIndices = prim["stringVertIndices"].data
		
		if multipleConversions :
			defaultIndices = origSopPrim["stringVertIndices"].data
			for i in range( 0, origNumVerts ) :
				val = "" if ( defaultIndices[i] >= data.size() ) else data[ defaultIndices[i] ]
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
			self.assert_( key in result.keys() )
		
		self.assertTrue( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertTrue( result["v2fDetail"].data.getInterpretation(), IECore.GeometricData.Interpretation.Vector )
		self.assertTrue( result["v3fPoint"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertTrue( result["v3fPrim"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		self.assertTrue( result["v3fVert"].data.getInterpretation(), IECore.GeometricData.Interpretation.Normal )
		
	def testCreateConverter( self )  :
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( self.mesh() )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPolygonsConverter ) ) )

	def testFactory( self ) :
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.mesh() )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPolygonsConverter ) ) )
		
		self.failUnless( IECore.TypeId.MeshPrimitive in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )

	def testConversionIntoEmptySop( self ) :
		mesh = self.mesh()
		sop = self.emptySop()
		
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )
		
		self.comparePrimAndSop( mesh, sop )
	
	def testConversionIntoExistingSop( self ) :
		mesh = self.mesh()
		sop = self.meshSop()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, mesh )
		
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, False ) )
		
		self.comparePrimAndSop( mesh, sop )
		
	def testAppendingIntoExistingSop( self ) :
		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints )
		self.assert_( "floatDetail" not in result.keys() )
		self.assert_( "floatPoint" not in result.keys() )
	
	def testAppendingIntoLockedSop( self ) :
		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )
		
		sop.setHardLocked( True )
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
		
		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints )
		self.assert_( "floatDetail" not in result.keys() )
		self.assert_( "floatPoint" not in result.keys() )
	
	def testSaveLoad( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )
		
		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()
		sopPath = sop.path()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
		
		hou.hipFile.save( TestToHoudiniPolygonsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPolygonsConverter.__testScene )
		
		newSop = hou.node( sopPath )
		
		self.assert_( newSop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, newSop, orig )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( newSop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
		
	def testSaveLoadWithLockedSop( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )
		
		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()
		sopPath = sop.path()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )
		
		sop.setHardLocked( True )
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
		
		hou.hipFile.save( TestToHoudiniPolygonsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPolygonsConverter.__testScene )
		
		newSop = hou.node( sopPath )
		
		self.assert_( newSop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, newSop, orig )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( newSop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
	
	def testMultipleConversions( self ) :
		mesh = self.mesh()
		meshNumPoints = mesh.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.meshSop()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, mesh )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )

		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, sop, result, multipleConversions=True )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + 2*meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + meshNumPoints + i ], mesh["P"].data[i] )
		
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( mesh, sop, result, multipleConversions=True )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + 3*meshNumPoints )
		for i in range( 0, mesh["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], mesh["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + meshNumPoints + i ], mesh["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + 2*meshNumPoints + i ], mesh["P"].data[i] )

	def testObjectWasDeleted( self ) :
		mesh = self.mesh()
		sop = self.meshSop()
		
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		
		self.assert_( converter.convert( sop, False ) )
		
		self.comparePrimAndSop( mesh, sop )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		
		del mesh
		
		sop.setHardLocked( False )
		self.assertNotEqual( IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert(), result )
		self.assert_( converter.convert( sop, False ) )
		self.assertEqual( IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert(), result )

	def testWithUnacceptablePrimVars( self ) :
		mesh = self.mesh()
		mesh["badDetail"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.TransformationMatrixfData() )
		mesh["badPoint"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		mesh["badPrim"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		mesh["badVert"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		sop = self.emptySop()
		
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )
		
		self.assert_( "badDetail" not in [ x.name() for x in sop.geometry().globalAttribs() ] )
		self.assert_( "badPoint" not in [ x.name() for x in sop.geometry().pointAttribs() ] )
		self.assert_( "badPrim" not in [ x.name() for x in sop.geometry().primAttribs() ] )
		self.assert_( "badVert" not in [ x.name() for x in sop.geometry().vertexAttribs() ] )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( result, mesh )
		self.assert_( "badDetail" not in result )
		self.assert_( "badPoint" not in result )
		self.assert_( "badPrim" not in result )
		self.assert_( "badVert" not in result )
		
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
		
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( vertexAttr ) )
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

		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( vertexAttr ) )
		self.comparePrimAndSop( mesh, vertexAttr )
	
	def testEmptyString( self ) :
		mesh = self.mesh()
		sop = self.emptySop()
		mesh['stringPoint'].data[1] = ""
		
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )
		
		geo = sop.geometry()
		sopPoints = geo.points()
		data = mesh["stringPoint"].data
		dataIndices = mesh["stringPointIndices"].data
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPoints[i].attribValue( "stringPoint" ) )
		
		result = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertEqual( result.verticesPerFace, mesh.verticesPerFace )
		self.assertEqual( result.vertexIds, mesh.vertexIds )
		self.assertEqual( result.keys(), mesh.keys() )
		self.assertEqual( result["stringPoint"], mesh["stringPoint"] )
		self.assertEqual( result["stringPointIndices"], mesh["stringPointIndices"] )
	
	def testName( self ) :
		
		sop = self.emptySop()
		mesh = self.mesh()
		mesh.blindData()["name"] = IECore.StringData( "testGroup" )
		self.assert_( IECoreHoudini.ToHoudiniPolygonsConverter( mesh ).convert( sop ) )
		geo = sop.geometry()
		nameAttr = geo.findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testGroup" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testGroup" ]), mesh.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
	
	def testAttributeFilter( self ) :
		
		mesh = self.mesh()
		sop = self.emptySop()
		
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['P', 'Pw', 'color3fPoint', 'floatPoint', 'intPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'ieMeshInterpolation', 'intPrim', 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['color3fVert', 'floatVert', 'intVert', 'stringVert', 'v2fVert', 'v2iVert', 'v3fVert', 'v3iVert'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'floatDetail', 'intDetail', 'stringDetail', 'v2fDetail', 'v2iDetail', 'v3fDetail', 'v3iDetail'] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "P *3f*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['P', 'Pw', 'color3fPoint', 'v3fPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'ieMeshInterpolation', 'v3fPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['color3fVert', 'v3fVert'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'v3fDetail'] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "* ^*Detail ^int*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['P', 'Pw', 'color3fPoint', 'floatPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'ieMeshInterpolation', 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['color3fVert', 'floatVert', 'stringVert', 'v2fVert', 'v2iVert', 'v3fVert', 'v3iVert'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		# verify we can filter uvs
		mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		IECore.TriangulateOp()( input=mesh, copyInput=False )
		IECore.MeshNormalsOp()( input=mesh, copyInput=False )
		mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6, IECore.GeometricData.Interpretation.Color ) )
		mesh["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 4 ) )
		mesh["Pref"] = mesh["P"]
		
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		converter.parameters()["attributeFilter"].setTypedValue( "*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['N', 'P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		# have to filter the source attrs s, t and not uv
		converter.parameters()["attributeFilter"].setTypedValue( "* ^uv  ^pscale ^rest" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['N', 'P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "* ^s ^t  ^width ^Pref" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['N', 'P', 'Pw'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "* ^s ^width ^Cs" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['N', 'P', 'Pw', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), ['t'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
	
	def testStandardAttributeConversion( self ) :
		
		sop = self.emptySop()
		mesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		IECore.TriangulateOp()( input=mesh, copyInput=False )
		IECore.MeshNormalsOp()( input=mesh, copyInput=False )
		mesh["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 6, IECore.GeometricData.Interpretation.Color ) )
		mesh["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 4 ) )
		mesh["Pref"] = mesh["P"]
		
		self.assertTrue( mesh.arePrimitiveVariablesValid() )
		
		converter = IECoreHoudini.ToHoudiniPolygonsConverter( mesh )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sorted([ x.name() for x in geo.pointAttribs() ]), ['N', 'P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in geo.primAttribs() ]), ['ieMeshInterpolation'] )
		self.assertEqual( sorted([ x.name() for x in geo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in geo.globalAttribs() ]), [] )
		
		sData = mesh["s"].data
		tData = mesh["t"].data
		uvs = geo.findVertexAttrib( "uv" )
		
		i = 0
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( uvs )
				self.assertAlmostEqual( uvValues[0], sData[i] )
				self.assertAlmostEqual( uvValues[1], 1 - tData[i] )
				i += 1
		
		converter["convertStandardAttributes"].setTypedValue( False )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sorted([ x.name() for x in geo.pointAttribs() ]), ['N', 'P', 'Pref', 'Pw', 'width'] )
		self.assertEqual( sorted([ x.name() for x in geo.primAttribs() ]), ['ieMeshInterpolation', ] )
		self.assertEqual( sorted([ x.name() for x in geo.vertexAttribs() ]), ['Cs', 's', 't'] )
		self.assertEqual( sorted([ x.name() for x in geo.globalAttribs() ]), [] )
		
		i = 0
		s = geo.findVertexAttrib( "s" )
		t = geo.findVertexAttrib( "t" )
		for prim in geo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				self.assertAlmostEqual( vert.attribValue( s ), sData[i] )
				self.assertAlmostEqual( vert.attribValue( t ), tData[i] )
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
	
	def tearDown( self ) :
		
		if os.path.isfile( TestToHoudiniPolygonsConverter.__testScene ) :
			os.remove( TestToHoudiniPolygonsConverter.__testScene )

if __name__ == "__main__":
    unittest.main()
