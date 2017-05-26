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

class TestToHoudiniCurvesConverter( IECoreHoudini.TestCase ) :
	
	if hou.applicationVersion()[0] >= 16:
		PointPositionAttribs = ['P']
	else:
		PointPositionAttribs = ['P', 'Pw']
	
	__testScene = "test/converterTest.hip"
	
	__curveCoordinates = [
		IECore.V3fVectorData( [ IECore.V3f( 2.42892,0,-1.04096 ), IECore.V3f( 1.69011,0,-9.88746 ), IECore.V3f( 5.74288,0,-4.50183 ), IECore.V3f( 2.69113,0,-2.78439 ), IECore.V3f( 5.8923,0,1.53021 ), IECore.V3f( 6.20965,-9.53674e-07,2.03933 ), IECore.V3f( 2.72012,0,2.5738 ), IECore.V3f( 1.76971,0,-0.632637 ) ] ),
		IECore.V3fVectorData( [ IECore.V3f( -0.560781,0,-1.04096 ), IECore.V3f( 2.21995,0,-6.31734 ), IECore.V3f( 4.77513,0,-6.61752 ), IECore.V3f( 4.10862,0,-2.78439 ), IECore.V3f( 4.29081,0,1.53021 ), IECore.V3f( 6.20965,-9.53674e-07,3.7489 ), IECore.V3f( -2.61584,0,2.5738 ), IECore.V3f( -1.45801,0,0.780965 ) ] ),
		IECore.V3fVectorData( [ IECore.V3f( 2.42892,0,-1.04096 ), IECore.V3f( 2.21995,0,-4.51254 ), IECore.V3f( 4.77513,0,-4.50183 ), IECore.V3f( 6.32944,0,-2.78439 ), IECore.V3f( 7.231,0,1.53021 ), IECore.V3f( 6.20965,-9.53674e-07,3.7489 ), IECore.V3f( 2.72012,0,2.5738 ), IECore.V3f( 1.76971,0,0.780965 ) ] ),
		IECore.V3fVectorData( [ IECore.V3f( 5.83427,0,-1.04096 ), IECore.V3f( 2.21995,0,-4.51254 ), IECore.V3f( 6.14141,0,-4.50183 ), IECore.V3f( 7.48932,0,-2.78439 ), IECore.V3f( 9.0197,0,1.53021 ), IECore.V3f( 6.20965,-9.53674e-07,1.2141 ), IECore.V3f( 2.72012,0,2.5738 ), IECore.V3f( 3.23728,0,0.780965 ) ] )
	]
	
	def curves( self, basis=IECore.CubicBasisf.linear(), periodic=False, numCurves=4 ) :
		vertsPerCurve = IECore.IntVectorData()
		pData = IECore.V3fVectorData()
		pData.setInterpretation( IECore.GeometricData.Interpretation.Point )
		
		for i in range( 0, numCurves ) :
			p = TestToHoudiniCurvesConverter.__curveCoordinates[i%4]
			
			if not periodic and basis == IECore.CubicBasisf.bSpline() :
				vertsPerCurve.append( len(p) + 4 )
			else :
				vertsPerCurve.append( len(p) )
			
			pData.extend( p )
		
		curves = IECore.CurvesPrimitive( vertsPerCurve, basis, periodic )
		
		floatData = IECore.FloatData( 1.5 )
		v2fData = IECore.V2fData( IECore.V2f( 1.5, 2.5 ) )
		v3fData = IECore.V3fData( IECore.V3f( 1.5, 2.5, 3.5 ) )
		color3fData = IECore.Color3fData( IECore.Color3f( 1.5, 2.5, 3.5 ) )
		intData = IECore.IntData( 1 )
		v2iData = IECore.V2iData( IECore.V2i( 1, 2 ) )
		v3iData = IECore.V3iData( IECore.V3i( 1, 2, 3 ) )
		stringData = IECore.StringData( "this is a string" )
		
		intRange = range( 1, pData.size()+1 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		v2fVectorData = IECore.V2fVectorData( [ IECore.V2f( x, x+0.5 ) for x in intRange ] )
		v3fVectorData = IECore.V3fVectorData( [ IECore.V3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		intVectorData = IECore.IntVectorData( intRange )
		v2iVectorData = IECore.V2iVectorData( [ IECore.V2i( x, -x ) for x in intRange ] )
		v3iVectorData = IECore.V3iVectorData( [ IECore.V3i( x, -x, x*2 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %06d!" % x for x in intRange ] )
		
		detailInterpolation = IECore.PrimitiveVariable.Interpolation.Constant
		pointInterpolation = IECore.PrimitiveVariable.Interpolation.Vertex
		primitiveInterpolation = IECore.PrimitiveVariable.Interpolation.Uniform
		
		# add all valid detail attrib types
		curves["floatDetail"] = IECore.PrimitiveVariable( detailInterpolation, floatData )
		curves["v2fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2fData )
		curves["v3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3fData )
		curves["color3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, color3fData )
		curves["intDetail"] = IECore.PrimitiveVariable( detailInterpolation, intData )
		curves["v2iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2iData )
		curves["v3iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3iData )
		curves["stringDetail"] = IECore.PrimitiveVariable( detailInterpolation, stringData )
		
		# add all valid point attrib types
		if not periodic and basis == IECore.CubicBasisf.bSpline() :
			
			modPData = IECore.V3fVectorData()
			modPData.setInterpretation( IECore.GeometricData.Interpretation.Point )
			floatPointData = IECore.FloatVectorData()
			v2fPointData = IECore.V2fVectorData()
			v3fPointData = IECore.V3fVectorData()
			color3fPointData = IECore.Color3fVectorData()
			intPointData = IECore.IntVectorData()
			v2iPointData = IECore.V2iVectorData()
			v3iPointData = IECore.V3iVectorData()
			stringPointData = IECore.StringVectorData()
			
			datas = [ modPData, floatPointData, v2fPointData, v3fPointData, color3fPointData, intPointData, v2iPointData, v3iPointData, stringPointData ]
			rawDatas = [ pData, floatVectorData, v2fVectorData, v3fVectorData, color3fVectorData, intVectorData, v2iVectorData, v3iVectorData, stringVectorData ]
			
			pIndex = 0
			for i in range( 0, numCurves ) :
				for j in range( 0, len(datas) ) :
					index = 8*i
					datas[j].extend( [ rawDatas[j][index], rawDatas[j][index] ] )
					datas[j].extend( rawDatas[j][index:index+8] )
					datas[j].extend( [ rawDatas[j][index+7], rawDatas[j][index+7] ] )
			
			curves["P"] = IECore.PrimitiveVariable( pointInterpolation, modPData )
			curves["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatPointData )
			curves["v2fPoint"] = IECore.PrimitiveVariable( pointInterpolation,v2fPointData )
			curves["v3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3fPointData )
			curves["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fPointData )
			curves["intPoint"] = IECore.PrimitiveVariable( pointInterpolation, intPointData )
			curves["v2iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2iPointData )
			curves["v3iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3iPointData )
		else :
			curves["P"] = IECore.PrimitiveVariable( pointInterpolation, pData )
			curves["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatVectorData[:8*numCurves] )
			curves["v2fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2fVectorData[:8*numCurves] )
			curves["v3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3fVectorData[:8*numCurves] )
			curves["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fVectorData[:8*numCurves] )
			curves["intPoint"] = IECore.PrimitiveVariable( pointInterpolation, intVectorData[:8*numCurves] )
			curves["v2iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2iVectorData[:8*numCurves] )
			curves["v3iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3iVectorData[:8*numCurves] )
			curves["stringPoint"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:8*numCurves] )
			curves["stringPointIndices"] = IECore.PrimitiveVariable( pointInterpolation, IECore.IntVectorData( range( 0, 8*numCurves ) ) )
			
		# add all valid primitive attrib types
		curves["floatPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, floatVectorData[:numCurves] )
		curves["v2fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v2fVectorData[:numCurves] )
		curves["v3fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v3fVectorData[:numCurves] )
		curves["color3fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, color3fVectorData[:numCurves] )
		curves["intPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, intVectorData[:numCurves] )
		curves["v2iPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v2iVectorData[:numCurves] )
		curves["v3iPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, v3iVectorData[:numCurves] )
		curves["stringPrim"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:numCurves] )
		curves["stringPrimIndices"] = IECore.PrimitiveVariable( primitiveInterpolation, IECore.IntVectorData( range( 0, numCurves ) ) )
		
		self.assert_( curves.arePrimitiveVariablesValid() )
		
		return curves
	
	def emptySop( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		null = geo.createNode( "null" )
		
		return null

	def curveSop( self, order=2, periodic=False, parent=None, coordIndex=0 ) :
		
		if not parent :
			obj = hou.node("/obj")
			parent = obj.createNode("geo", run_init_scripts=False)
		
		curve = parent.createNode( "curve" )
		curve.parm( "type" ).set( 1 ) # NURBS
		curve.parm( "order" ).set( order )
		curve.parm( "close" ).set( periodic )
		
		coordStr = ""
		coords = TestToHoudiniCurvesConverter.__curveCoordinates[coordIndex]
		for p in coords :
			coordStr += "%f,%f,%f " % ( p[0], p[1], p[2] )
		
		curve.parm( "coords" ).set( coordStr )

		return curve

	def curvesSop( self, numCurves=4, order=2, periodic=False ) :
		
		curves = [ self.curveSop( order, periodic ) ]
		
		geo = curves[0].parent()
		
		for i in range( 0, numCurves-1 ) :
			curves.append( self.curveSop( order, periodic, geo, i%4 ) )
		
		merge = geo.createNode( "merge" )
		
		for i in range( 0, len(curves) ) :
			merge.setInput( i, curves[i] )
		
		return merge
	
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
		self.assertEqual( len(sopPrims), prim.numCurves() )
		
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
			self.assertEqual( len(verts), prim.verticesPerCurve()[i] )
			verts.reverse()
			sopVerts.extend( verts )
		
		self.assertEqual( len(sopVerts), prim["P"].data.size() )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		self.assertEqual( result.verticesPerCurve(), prim.verticesPerCurve() )
		self.assertEqual( result.keys(), prim.keys() )
		for key in prim.keys() :
			self.assertEqual( result[key], prim[key] )
		self.assertEqual( result, prim )
		
	def compareOpenSplinePrimAndSop( self, prim, sop ) :
		geo = sop.geometry()
		for key in [ "floatDetail", "intDetail", "stringDetail" ] :
			self.assertEqual( prim[key].data.value, geo.attribValue( key ) )
		
		for key in [ "v2fDetail", "v3fDetail", "color3fDetail", "v2iDetail", "v3iDetail" ] :
			self.assertEqual( tuple(prim[key].data.value), geo.attribValue( key ) )
		
		sopPrims = geo.prims()
		
		pIndex = 0
		for i in range( prim.numCurves() ) :
			hVertices = sopPrims[i].vertices()
			self.assertEqual( len(hVertices) + 4, prim.verticesPerCurve()[i] )
			for j in range( len(hVertices) ) :
				
				for attr in geo.pointAttribs() :
					if attr.name() == "Pw" :
						continue
					
					data = prim[attr.name()].data
					if attr.name() in [ "floatPoint", "intPoint" ] :
						self.assertEqual( data[pIndex], hVertices[j].point().attribValue( attr.name() ) )
					else :
						self.assertEqual( tuple(data[pIndex]), hVertices[j].point().attribValue( attr.name() ) )
					
					if ( j == 0 or j == len(hVertices)-1 ) :
						if attr.name() in [ "floatPoint", "intPoint" ] :
							self.assertEqual( data[pIndex+1], hVertices[j].point().attribValue( attr.name() ) )
							self.assertEqual( data[pIndex+2], hVertices[j].point().attribValue( attr.name() ) )
						else :
							self.assertEqual( tuple(data[pIndex+1]), hVertices[j].point().attribValue( attr.name() ) )
							self.assertEqual( tuple(data[pIndex+2]), hVertices[j].point().attribValue( attr.name() ) )
				
				if ( j == 0 or j == len(hVertices)-1 ) :
					pIndex += 3
				else :
					pIndex += 1
		
		self.assertEqual( len(sopPrims), prim.numCurves() )
		
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
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		self.assertEqual( result.verticesPerCurve(), prim.verticesPerCurve() )
		self.assertEqual( result.keys(), prim.keys() )
		for key in prim.keys() :
			self.assertEqual( result[key], prim[key] )
		self.assertEqual( result, prim )
			
	def comparePrimAndAppendedSop( self, prim, sop, origSopPrim, multipleConversions=False ) :
		geo = sop.geometry()
		for key in [ "floatDetail", "intDetail", "stringDetail" ] :
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
			defaultData = origSopPrim["stringPoint"].data
			defaultIndices = origSopPrim["stringPointIndices"].data
			for i in range( 0, origNumPoints ) :
				val = "" if ( defaultIndices[i] >= defaultData.size() ) else defaultData[ defaultIndices[i] ]
				self.assertEqual( val, sopPoints[ i ].attribValue( "stringPoint" ) )
		else :
			defaultValues = [ "" ] * origNumPoints
			for i in range( 0, origNumPoints ) :
				self.assertEqual( defaultValues[i], sopPoints[ i ].attribValue( "stringPoint" ) )
		
		sopPrims = geo.prims()
		origNumPrims = origSopPrim.numCurves()
		self.assertEqual( len(sopPrims), origNumPrims + prim.numCurves() )
		
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
			defaultData = origSopPrim["stringPrim"].data
			defaultIndices = origSopPrim["stringPrimIndices"].data
			for i in range( 0, origNumPrims ) :
				val = "" if ( defaultIndices[i] >= defaultData.size() ) else defaultData[ defaultIndices[i] ]
				self.assertEqual( val, sopPrims[ i ].attribValue( "stringPrim" ) )
		else :
			defaultValues = [ "" ] * origNumPrims
			for i in range( 0, origNumPrims ) :
				self.assertEqual( defaultValues[i], sopPrims[ i ].attribValue( "stringPrim" ) )
		
		sopVerts = []
		for i in range( 0, len(sopPrims) ) :
			verts = list(sopPrims[i].vertices())
			verts.reverse()
			sopVerts.extend( verts )
			if i > origNumPrims :
				self.assertEqual( len(verts), prim.verticesPerCurve()[i-origNumPrims] )
		
		self.assertEqual( len(sopVerts), origSopPrim["P"].data.size() + prim["P"].data.size() )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		self.assertEqual( result.verticesPerCurve()[origNumPrims:], prim.verticesPerCurve() )
		for key in prim.keys() :
			self.assert_( key in result.keys() )
		
	def testCreateConverter( self )  :
		converter = IECoreHoudini.ToHoudiniCurvesConverter( self.curves() )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCurvesConverter ) ) )

	def testFactory( self ) :
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.curves() )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCurvesConverter ) ) )
		self.failUnless( IECore.TypeId.CurvesPrimitive in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )

	def testLinearConversion( self ) :
		sop = self.emptySop()
		
		curves = self.curves()
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
		curves = self.curves( periodic=True )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
		curves = self.curves( numCurves=1 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
		curves = self.curves( periodic=True, numCurves=1 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
		curves = self.curves( numCurves=100 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
		curves = self.curves( periodic=True, numCurves=100 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
	def testSplineConversion( self ) :
		sop = self.emptySop()
		spline = IECore.CubicBasisf.bSpline()
		
		curves = self.curves( basis=spline )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.compareOpenSplinePrimAndSop( curves, sop )
		
		curves = self.curves( basis=spline, periodic=True )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
		curves = self.curves( basis=spline, numCurves=1 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.compareOpenSplinePrimAndSop( curves, sop )
		
		curves = self.curves( basis=spline, periodic=True, numCurves=1 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
		
		curves = self.curves( basis=spline, numCurves=100 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.compareOpenSplinePrimAndSop( curves, sop )
		
		curves = self.curves( basis=spline, periodic=True, numCurves=100 )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		self.comparePrimAndSop( curves, sop )
	
	def testConversionIntoExistingSop( self ) :
		curves = self.curves()
		sop = self.curvesSop()
		
		orig = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		self.assertNotEqual( orig, curves )
		
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, False ) )
		
		self.comparePrimAndSop( curves, sop )
		
	def testAppendingIntoExistingSop( self ) :
		curves = self.curves()
		curvesNumPoints = curves.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.curvesSop()
		
		orig = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, curves )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, sop, orig )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )

		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints )
		self.assert_( "floatDetail" not in result.keys() )
		self.assert_( "floatPoint" not in result.keys() )
	
	def testAppendingIntoLockedSop( self ) :
		curves = self.curves()
		curvesNumPoints = curves.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.curvesSop()
		
		orig = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, curves )
		
		sop.setHardLocked( True )
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, sop, orig )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )
		
		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints )
		self.assert_( "floatDetail" not in result.keys() )
		self.assert_( "floatPoint" not in result.keys() )
	
	def testSaveLoad( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )
		
		curves = self.curves()
		curvesNumPoints = curves.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.curvesSop()
		sopPath = sop.path()
		
		orig = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, curves )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, sop, orig )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )
		
		hou.hipFile.save( TestToHoudiniCurvesConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniCurvesConverter.__testScene )
		
		newSop = hou.node( sopPath )
		
		self.assert_( newSop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, newSop, orig )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( newSop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )
		
	def testSaveLoadWithLockedSop( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )
		
		curves = self.curves()
		curvesNumPoints = curves.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.curvesSop()
		sopPath = sop.path()
		
		orig = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, curves )
		
		sop.setHardLocked( True )
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, sop, orig )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )
		
		hou.hipFile.save( TestToHoudiniCurvesConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniCurvesConverter.__testScene )
		
		newSop = hou.node( sopPath )
		
		self.assert_( newSop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, newSop, orig )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( newSop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )
	
	def testMultipleConversions( self ) :
		curves = self.curves()
		curvesNumPoints = curves.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		sop = self.curvesSop()
		
		orig = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertNotEqual( orig, curves )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, sop, orig )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )

		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, sop, result, multipleConversions=True )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + 2*curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + curvesNumPoints + i ], curves["P"].data[i] )
		
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( curves, sop, result, multipleConversions=True )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		resultNumPoints = result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( resultNumPoints, origNumPoints + 3*curvesNumPoints )
		for i in range( 0, curves["P"].data.size() ) :
			self.assertEqual( result["P"].data[ origNumPoints + i ], curves["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + curvesNumPoints + i ], curves["P"].data[i] )
			self.assertEqual( result["P"].data[ origNumPoints + 2*curvesNumPoints + i ], curves["P"].data[i] )

	def testObjectWasDeleted( self ) :
		curves = self.curves()
		sop = self.curvesSop()
		
		converter = IECoreHoudini.ToHoudiniCurvesConverter( curves )
		
		self.assert_( converter.convert( sop, False ) )
		
		self.comparePrimAndSop( curves, sop )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		
		del curves
		
		sop.setHardLocked( False )
		self.assertNotEqual( IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert(), result )
		self.assert_( converter.convert( sop, False ) )
		self.assertEqual( IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert(), result )

	def testWithUnacceptablePrimVars( self ) :
		curves = self.curves()
		curves["badDetail"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.TransformationMatrixfData() )
		curves["badPoint"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		curves["badPrim"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		curves["badVert"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		sop = self.emptySop()
		
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		
		self.assert_( "badDetail" not in [ x.name() for x in sop.geometry().globalAttribs() ] )
		self.assert_( "badPoint" not in [ x.name() for x in sop.geometry().pointAttribs() ] )
		self.assert_( "badPrim" not in [ x.name() for x in sop.geometry().primAttribs() ] )
		self.assert_( "badVert" not in [ x.name() for x in sop.geometry().vertexAttribs() ] )
		
		result = IECoreHoudini.FromHoudiniCurvesConverter( sop ).convert()
		self.assertNotEqual( result, curves )
		self.assert_( "badDetail" not in result )
		self.assert_( "badPoint" not in result )
		self.assert_( "badPrim" not in result )
		self.assert_( "badVert" not in result )
		
		del curves["badDetail"]
		del curves["badPoint"]
		del curves["badPrim"]
		del curves["badVert"]
		self.comparePrimAndSop( curves, sop )

	def testConvertingOverExistingAttribs( self ) :
		curves = self.curves()
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
		
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( primAttr ) )
		self.comparePrimAndSop( curves, primAttr )

	def testConvertingOverExistingAttribsWithDifferentTypes( self ) :
		curves = self.curves()
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
		pointAttr.parm( "class" ).set( 1 ) # point
		pointAttr.parm( "type" ).set( 1 ) # int
		pointAttr.parm( "size" ).set( 3 ) # 3 elements
		pointAttr.parm( "value1" ).set( 10 )
		pointAttr.parm( "value2" ).set( 11 )
		pointAttr.parm( "value3" ).set( 12 )
		
		primAttr = pointAttr.createOutputNode( "attribcreate", exact_type_name=True )
		primAttr.parm( "name" ).set( "floatPrim" )
		primAttr.parm( "class" ).set( 1 ) # point
		primAttr.parm( "type" ).set( 1 ) # int
		primAttr.parm( "size" ).set( 3 ) # 3 elements
		primAttr.parm( "value1" ).set( 10 )
		primAttr.parm( "value2" ).set( 11 )
		primAttr.parm( "value3" ).set( 12 )
		
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( primAttr ) )
		self.comparePrimAndSop( curves, primAttr )

	def testVertAttribsCantBeConverted( self ) :
		curves = self.curves()
		curves["floatVert"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( 1 ) )
		sop = self.emptySop()
		
		self.assert_( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
		
		allAttribs = [ x.name() for x in sop.geometry().globalAttribs() ]
		allAttribs.extend( [ x.name() for x in sop.geometry().pointAttribs() ] )
		allAttribs.extend( [ x.name() for x in sop.geometry().primAttribs() ] )
		allAttribs.extend( [ x.name() for x in sop.geometry().vertexAttribs() ] )
		self.assert_( "floatVert" not in allAttribs )
		
		del curves["floatVert"]
		self.comparePrimAndSop( curves, sop )
	
	def testBadCurve( self ) :
		
		curves = IECore.CurvesPrimitive( IECore.IntVectorData( [ 7 ] ), IECore.CubicBasisf.bSpline(), False )
		curves['P'] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 0 ), IECore.V3f( 0 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 2 ), IECore.V3f( 2 ) ] ) )
		self.failUnless( curves.arePrimitiveVariablesValid() )
		
		sop = self.emptySop()
		self.assertFalse( IECoreHoudini.ToHoudiniCurvesConverter( curves ).convert( sop ) )
	
	def testName( self ) :
		
		sop = self.emptySop()
		curves = self.curves()
		converter = IECoreHoudini.ToHoudiniCurvesConverter( curves )
		
		# unnamed unless we set the parameter
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sop.geometry().findPrimAttrib( "name" ), None )
		
		converter["name"].setTypedValue( "testCurves" )
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testCurves" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testCurves" ]), curves.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
		
		# blindData still works for backwards compatibility
		curves.blindData()["name"] = IECore.StringData( "blindCurves" )
		converter = IECoreHoudini.ToHoudiniCurvesConverter( curves )
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "blindCurves" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "blindCurves" ]), curves.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
		
		# name parameter takes preference over blindData
		converter["name"].setTypedValue( "testCurves" )
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testCurves" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testCurves" ]), curves.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )	
	
	def testAttributeFilter( self ) :
		
		curves = self.curves()
		sop = self.emptySop()
		
		converter = IECoreHoudini.ToHoudiniCurvesConverter( curves )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs + ['color3fPoint', 'floatPoint', 'intPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'intPrim', 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'floatDetail', 'intDetail', 'stringDetail', 'v2fDetail', 'v2iDetail', 'v3fDetail', 'v3iDetail'] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "P *3f*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs + ['color3fPoint', 'v3fPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'v3fPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'v3fDetail'] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "* ^*Detail ^int*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs + ['color3fPoint', 'floatPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		# verify we can filter uvs
		for key in curves.keys() :
			if key != "P" :
				del curves[key]
		rand = IECore.Rand32()
		curves["s"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ rand.nextf() for x in range( 0, 32 ) ] ) )
		curves["t"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ rand.nextf() for x in range( 0, 32 ) ] ) )
		curves["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 4, IECore.GeometricData.Interpretation.Color ) )
		curves["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 32 ) )
		curves["Pref"] = curves["P"]
		
		converter = IECoreHoudini.ToHoudiniCurvesConverter( curves )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs + ['pscale', 'rest', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		# have to filter the source attrs s, t and not uv
		converter.parameters()["attributeFilter"].setTypedValue( "* ^uv ^pscale ^rest" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]),TestToHoudiniCurvesConverter.PointPositionAttribs + ['pscale', 'rest', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "* ^s ^t  ^width ^Pref" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "* ^s  ^width ^Cs" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs + ['rest', 't'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
	
	def testStandardAttributeConversion( self ) :
		
		sop = self.emptySop()
		curves = self.curves()
		for key in curves.keys() :
			if key != "P" :
				del curves[key]
		rand = IECore.Rand32()
		curves["s"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ rand.nextf() for x in range( 0, 32 ) ] ) )
		curves["t"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ rand.nextf() for x in range( 0, 32 ) ] ) )
		curves["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Uniform, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 4, IECore.GeometricData.Interpretation.Color ) )
		curves["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 32 ) )
		curves["Pref"] = curves["P"]
		
		self.assertTrue( curves.arePrimitiveVariablesValid() )
		
		converter = IECoreHoudini.ToHoudiniCurvesConverter( curves )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sorted([ x.name() for x in geo.pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs + ['pscale', 'rest', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in geo.primAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in geo.vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in geo.globalAttribs() ]), [] )
		
		sData = curves["s"].data
		tData = curves["t"].data
		uvs = geo.findPointAttrib( "uv" )
		
		i = 0
		for point in geo.points() :
			uvValues = point.attribValue( uvs )
			self.assertAlmostEqual( uvValues[0], sData[i] )
			self.assertAlmostEqual( uvValues[1], 1 - tData[i] )
			i += 1
		
		converter["convertStandardAttributes"].setTypedValue( False )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertItemsEqual( sorted([ x.name() for x in geo.pointAttribs() ]), TestToHoudiniCurvesConverter.PointPositionAttribs + ['Pref', 's', 't', 'width'] )
		self.assertEqual( sorted([ x.name() for x in geo.primAttribs() ]), ['Cs'] )
		self.assertEqual( sorted([ x.name() for x in geo.vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in geo.globalAttribs() ]), [] )
		
		i = 0
		s = geo.findPointAttrib( "s" )
		t = geo.findPointAttrib( "t" )
		for point in geo.points() :
			self.assertAlmostEqual( point.attribValue( s ), sData[i] )
			self.assertAlmostEqual( point.attribValue( t ), tData[i] )
			i += 1
	
	def tearDown( self ) :
		
		if os.path.isfile( TestToHoudiniCurvesConverter.__testScene ) :
			os.remove( TestToHoudiniCurvesConverter.__testScene )

if __name__ == "__main__":
    unittest.main()
