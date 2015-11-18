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

class TestToHoudiniPointsConverter( IECoreHoudini.TestCase ) :
	
	__testScene = "test/converterTest.hip"
	
	def points( self ) :
		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
			IECore.V3f( 8 ), IECore.V3f( 9 ), IECore.V3f( 10 ), IECore.V3f( 11 ),
		] )
		
		points = IECore.PointsPrimitive( pData )
		
		floatData = IECore.FloatData( 1.5 )
		v2fData = IECore.V2fData( IECore.V2f( 1.5, 2.5 ) )
		v3fData = IECore.V3fData( IECore.V3f( 1.5, 2.5, 3.5 ) )
		v3fData = IECore.V3fData( IECore.V3f( 1.5, 2.5, 3.5 ) )
		color3fData = IECore.Color3fData( IECore.Color3f( 1.5, 2.5, 3.5 ) )
		intData = IECore.IntData( 1 )
		v2iData = IECore.V2iData( IECore.V2i( 1, 2 ) )
		v3iData = IECore.V3iData( IECore.V3i( 1, 2, 3 ) )
		stringData = IECore.StringData( "this is a string" )
		
		intRange = range( 1, 13 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		v2fVectorData = IECore.V2fVectorData( [ IECore.V2f( x, x+0.5 ) for x in intRange ] )
		v3fVectorData = IECore.V3fVectorData( [ IECore.V3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		quatVectorData = IECore.QuatfVectorData( [ IECore.Quatf( x, x+0.25, x+0.5, x+0.75 ) for x in intRange ] )
		intVectorData = IECore.IntVectorData( intRange )
		v2iVectorData = IECore.V2iVectorData( [ IECore.V2i( x, -x ) for x in intRange ] )
		v3iVectorData = IECore.V3iVectorData( [ IECore.V3i( x, -x, x*2 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %06d!" % x for x in intRange ] )
		
		detailInterpolation = IECore.PrimitiveVariable.Interpolation.Constant
		uniformInterpolation = IECore.PrimitiveVariable.Interpolation.Uniform
		pointInterpolation = IECore.PrimitiveVariable.Interpolation.Vertex
		
		# add all valid detail attrib types
		points["floatDetail"] = IECore.PrimitiveVariable( detailInterpolation, floatData )
		points["v2fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2fData )
		points["v3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3fData )
		points["color3fDetail"] = IECore.PrimitiveVariable( detailInterpolation, color3fData )
		points["intDetail"] = IECore.PrimitiveVariable( detailInterpolation, intData )
		points["v2iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v2iData )
		points["v3iDetail"] = IECore.PrimitiveVariable( detailInterpolation, v3iData )
		points["stringDetail"] = IECore.PrimitiveVariable( detailInterpolation, stringData )
		
		# add all valid prim attrib types
		points["floatPrim"] = IECore.PrimitiveVariable( uniformInterpolation, floatVectorData[:1] )
		points["v2fPrim"] = IECore.PrimitiveVariable( uniformInterpolation, v2fVectorData[:1] )
		points["v3fPrim"] = IECore.PrimitiveVariable( uniformInterpolation, v3fVectorData[:1] )
		points["color3fPrim"] = IECore.PrimitiveVariable( uniformInterpolation, color3fVectorData[:1] )
		points["quatPrim"] = IECore.PrimitiveVariable( uniformInterpolation, quatVectorData[:1] )
		points["intPrim"] = IECore.PrimitiveVariable( uniformInterpolation, intVectorData[:1] )
		points["v2iPrim"] = IECore.PrimitiveVariable( uniformInterpolation, v2iVectorData[:1] )
		points["v3iPrim"] = IECore.PrimitiveVariable( uniformInterpolation, v3iVectorData[:1] )
		points["stringPrim"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:1] )
		points["stringPrimIndices"] = IECore.PrimitiveVariable( uniformInterpolation, IECore.IntVectorData( [ 0 ] ) )
		
		# add all valid point attrib types
		points["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatVectorData )
		points["v2fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2fVectorData )
		points["v3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3fVectorData )
		points["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fVectorData )
		points["quatPoint"] = IECore.PrimitiveVariable( pointInterpolation, quatVectorData )
		points["intPoint"] = IECore.PrimitiveVariable( pointInterpolation, intVectorData )
		points["v2iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v2iVectorData )
		points["v3iPoint"] = IECore.PrimitiveVariable( pointInterpolation, v3iVectorData )
		points["stringPoint"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData )
		points["stringPointIndices"] = IECore.PrimitiveVariable( pointInterpolation, IECore.IntVectorData( range( 0, 12 ) ) )
		
		return points
	
	def emptySop( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		null = geo.createNode( "null" )
		
		return null

	def pointsSop( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		box = geo.createNode( "box" )
		facet = box.createOutputNode( "facet" )
		facet.parm( "postnml" ).set(True)
		points = facet.createOutputNode( "scatter" )
		
		return points
	
	def comparePrimAndSop( self, prim, sop ) :
		geo = sop.geometry()
		for key in [ "floatDetail", "intDetail", "stringDetail" ] :
			self.assertEqual( prim[key].data.value, geo.attribValue( key ) )
		
		for key in [ "v2fDetail", "v3fDetail", "color3fDetail", "v2iDetail", "v3iDetail" ] :
			self.assertEqual( tuple(prim[key].data.value), geo.attribValue( key ) )
		
		sopPrims = geo.prims()
		for key in [ "floatPrim", "intPrim", "stringPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPrims[i].attribValue( key ) )
		
		for key in [ "v2fPrim", "v3fPrim", "color3fPrim", "v2iPrim", "v3iPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPrims[i].attribValue( key ) )
		
		self.assertEqual( geo.findPrimAttrib( "quatPrim" ).qualifier(), "Quaternion" )
		data = prim["quatPrim"].data
		for i in range( 0, data.size() ) :
			components = ( data[i][1], data[i][2], data[i][3], data[i][0] )
			self.assertEqual( components, sopPrims[i].attribValue( "quatPrim" ) )
		
		data = prim["stringPrim"].data
		dataIndices = prim["stringPrimIndices"].data
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPrims[i].attribValue( "stringPrim" ) )
		
		sopPoints = geo.points()
		for key in [ "floatPoint", "intPoint" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPoints[i].attribValue( key ) )
		
		for key in [ "P", "v2fPoint", "v3fPoint", "color3fPoint", "v2iPoint", "v3iPoint" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPoints[i].attribValue( key ) )
		
		self.assertEqual( geo.findPointAttrib( "quatPoint" ).qualifier(), "Quaternion" )
		data = prim["quatPoint"].data
		for i in range( 0, data.size() ) :
			components = ( data[i][1], data[i][2], data[i][3], data[i][0] )
			self.assertEqual( components, sopPoints[i].attribValue( "quatPoint" ) )

		data = prim["stringPoint"].data
		dataIndices = prim["stringPointIndices"].data
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPoints[i].attribValue( "stringPoint" ) )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.keys(), prim.keys() )
		for key in prim.keys() :
			self.assertEqual( result[key], prim[key] )
		self.assertEqual( result, prim )
			
	def comparePrimAndAppendedSop( self, prim, sop, origSopPrim, multipleConversions=0 ) :
		geo = sop.geometry()
		# verify detail attribs
		for key in [ "floatDetail", "intDetail", "stringDetail" ] :
			self.assertEqual( prim[key].data.value, geo.attribValue( key ) )
		
		for key in [ "v2fDetail", "v3fDetail", "color3fDetail", "v2iDetail", "v3iDetail" ] :
			self.assertEqual( tuple(prim[key].data.value), geo.attribValue( key ) )
		
		# verify prim attribs
		sopPrims = geo.prims()
		numPrims = multipleConversions + 1
		self.assertEqual( len(sopPrims), numPrims )
		
		for key in [ "floatPrim", "intPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				for sopPrim in sopPrims :
					self.assertEqual( data[i], sopPrim.attribValue( key ) )
		
		for key in [ "v2fPrim", "v3fPrim", "color3fPrim", "v2iPrim", "v3iPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				for sopPrim in sopPrims :
					self.assertEqual( tuple(data[i]), sopPrim.attribValue( key ) )
		
		data = prim["stringPrim"].data
		dataIndices = prim["stringPrimIndices"].data
		for i in range( 0, data.size() ) :
			for sopPrim in sopPrims :
				self.assertEqual( data[ dataIndices[i] ], sopPrim.attribValue( "stringPrim" ) )
		
		# verify points attribs
		sopPoints = geo.points()
		self.assertEqual( len(sopPoints), origSopPrim.numPoints + prim.numPoints )
		
		for key in [ "floatPoint", "intPoint" ] :
			data = prim[key].data
			
			if multipleConversions :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ 0 ] * origSopPrim.numPoints
			
			for i in range( 0, origSopPrim.numPoints ) :
				self.assertEqual( defaultValue[i], sopPoints[ i ].attribValue( key ) )
			
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPoints[ origSopPrim.numPoints + i ].attribValue( key ) )
			
		for key in [ "P", "v2fPoint", "v3fPoint", "color3fPoint", "v2iPoint", "v3iPoint" ] :
			data = prim[key].data
			
			if multipleConversions or key is "P" :
				defaultValue = origSopPrim[key].data
			else :
				defaultValue = [ [ 0 ] * data[0].dimensions() ] * origSopPrim.numPoints
			
			for i in range( 0, origSopPrim.numPoints ) :
				self.assertEqual( tuple(defaultValue[i]), sopPoints[ i ].attribValue( key ) )
			
			for i in range( 0, data.size() ) :
				self.assertEqual( tuple(data[i]), sopPoints[ origSopPrim.numPoints + i ].attribValue( key ) )
		
		data = prim["stringPoint"].data
		dataIndices = prim["stringPointIndices"].data
		
		if multipleConversions :
			defaultData = origSopPrim["stringPoint"].data
			defaultIndices = origSopPrim["stringPointIndices"].data
			for i in range( 0, origSopPrim.numPoints ) :
				val = "" if ( defaultIndices[i] >= defaultData.size() ) else defaultData[ defaultIndices[i] ]
				self.assertEqual( val, sopPoints[ i ].attribValue( "stringPoint" ) )
		else :
			defaultValues = [ "" ] * origSopPrim.numPoints
			for i in range( 0, origSopPrim.numPoints ) :
				self.assertEqual( defaultValues[i], sopPoints[ i ].attribValue( "stringPoint" ) )

		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPoints[ origSopPrim.numPoints + i ].attribValue( "stringPoint" ) )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		for key in prim.keys() :
			# prim attribs don't make it through on multiple conversions because the interpolation size is incorrect
			if not( multipleConversions and "Prim" in key ) :
				self.assert_( key in result.keys() )
		
	def testCreateConverter( self )  :
		converter = IECoreHoudini.ToHoudiniPointsConverter( self.points() )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPointsConverter ) ) )

	def testFactory( self ) :
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.points() )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPointsConverter ) ) )
		self.failUnless( IECore.TypeId.PointsPrimitive in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )

	def testConversionIntoEmptySop( self ) :
		points = self.points()
		sop = self.emptySop()
		
		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), points )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop ) )
		
		self.comparePrimAndSop( points, sop )
	
	def testConversionIntoExistingSop( self ) :
		points = self.points()
		sop = self.pointsSop()
		
		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )
		
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, False ) )
		
		self.comparePrimAndSop( points, sop )
		
	def testAppendingIntoExistingSop( self ) :
		points = self.points()
		sop = self.pointsSop()
		
		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )

		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints )
		self.assert_( "floatDetail" not in result.keys() )
		self.assert_( "floatPoint" not in result.keys() )
	
	def testAppendingIntoLockedSop( self ) :
		points = self.points()
		sop = self.pointsSop()
		
		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )
		
		sop.setHardLocked( True )
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
		
		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints )
		self.assert_( "floatDetail" not in result.keys() )
		self.assert_( "floatPoint" not in result.keys() )
	
	def testSaveLoad( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )
		
		points = self.points()
		sop = self.pointsSop()
		sopPath = sop.path()
		
		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
		
		hou.hipFile.save( TestToHoudiniPointsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPointsConverter.__testScene )
		
		newSop = hou.node( sopPath )
		
		self.assert_( newSop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, newSop, orig )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( newSop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
		
	def testSaveLoadWithLockedSop( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )
		
		points = self.points()
		sop = self.pointsSop()
		sopPath = sop.path()
		
		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )
		
		sop.setHardLocked( True )
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
		
		hou.hipFile.save( TestToHoudiniPointsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPointsConverter.__testScene )
		
		newSop = hou.node( sopPath )
		
		self.assert_( newSop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, newSop, orig )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( newSop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
	
	def testMultipleConversions( self ) :
		points = self.points()
		sop = self.pointsSop()
		
		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )
		
		self.assert_( not sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, sop, orig )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )

		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, sop, result, multipleConversions=1 )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + 2*points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
			self.assertEqual( result["P"].data[ orig.numPoints + points.numPoints + i ], points["P"].data[i] )
		
		self.assert_( sop.isHardLocked() )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assert_( sop.isHardLocked() )
		
		self.comparePrimAndAppendedSop( points, sop, result, multipleConversions=2 )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + 3*points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
			self.assertEqual( result["P"].data[ orig.numPoints + points.numPoints + i ], points["P"].data[i] )
			self.assertEqual( result["P"].data[ orig.numPoints + 2*points.numPoints + i ], points["P"].data[i] )

	def testObjectWasDeleted( self ) :
		points = self.points()
		sop = self.pointsSop()
		
		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		
		self.assert_( converter.convert( sop, False ) )
		
		self.comparePrimAndSop( points, sop )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		
		del points
		
		sop.setHardLocked( False )
		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), result )
		self.assert_( converter.convert( sop, False ) )
		self.assertEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), result )

	def testWithUnacceptablePrimVars( self ) :
		points = self.points()
		points["badDetail"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.TransformationMatrixfData() )
		points["badPoint"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		sop = self.emptySop()
		
		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), points )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop ) )
		
		self.assert_( "badDetail" not in [ x.name() for x in sop.geometry().globalAttribs() ] )
		self.assert_( "badPoint" not in [ x.name() for x in sop.geometry().pointAttribs() ] )
		
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( result, points )
		self.assert_( "badDetail" not in result )
		self.assert_( "badPoint" not in result )
		
		del points["badDetail"]
		del points["badPoint"]
		self.comparePrimAndSop( points, sop )

	def testConvertingOverExistingAttribs( self ) :
		points = self.points()
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
		
		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( pointAttr ).convert(), points )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( pointAttr ) )
		self.comparePrimAndSop( points, pointAttr )

	def testConvertingOverExistingAttribsWithDifferentTypes( self ) :
		points = self.points()
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
		
		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( pointAttr ).convert(), points )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( pointAttr ) )
		self.comparePrimAndSop( points, pointAttr )

	def testVertAttribsCantBeConverted( self ) :
		points = self.points()
		points["floatVert"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( 1 ) )
		sop = self.emptySop()
		
		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), points )
		self.assert_( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop ) )
		
		allAttribs = [ x.name() for x in sop.geometry().globalAttribs() ]
		allAttribs.extend( [ x.name() for x in sop.geometry().pointAttribs() ] )
		allAttribs.extend( [ x.name() for x in sop.geometry().primAttribs() ] )
		allAttribs.extend( [ x.name() for x in sop.geometry().vertexAttribs() ] )
		self.assert_( "floatVert" not in allAttribs )
		
		del points["floatVert"]
		
		self.comparePrimAndSop( points, sop )
		
	def testAttributeFilter( self ) :
		
		points = self.points()
		sop = self.emptySop()
		
		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['P', 'Pw', 'color3fPoint', 'floatPoint', 'intPoint', "quatPoint", 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'intPrim', "quatPrim", 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'floatDetail', 'intDetail', 'stringDetail', 'v2fDetail', 'v2iDetail', 'v3fDetail', 'v3iDetail'] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "P *3f*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['P', 'Pw', 'color3fPoint', 'v3fPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'v3fPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'v3fDetail'] )
		
		converter.parameters()["attributeFilter"].setTypedValue( "* ^*Detail ^int* ^*Prim" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['P', 'Pw', 'color3fPoint', 'floatPoint', "quatPoint", 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
	
	def testStandardAttributeConversion( self ) :
		
		sop = self.emptySop()
		points = IECore.PointsPrimitive( IECore.V3fVectorData( [ IECore.V3f( 1 ) ] * 10 ) )
		points["Cs"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 1, 0, 0 ) ] * 10, IECore.GeometricData.Interpretation.Color ) )
		points["width"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 10 ) )
		points["Pref"] = points["P"]
		
		self.assertTrue( points.arePrimitiveVariablesValid() )
		
		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['Cd', 'P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
		
		converter["convertStandardAttributes"].setTypedValue( False )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), ['Cs', 'P', 'Pref', 'Pw', 'width'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )
	
	def testName( self ) :
		
		sop = self.emptySop()
		points = self.points()
		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		
		# unnamed unless we set the parameter
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sop.geometry().findPrimAttrib( "name" ), None )
		
		converter["name"].setTypedValue( "testPoints" )
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testPoints" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testPoints" ]), points.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
		
		# blindData still works for backwards compatibility
		points.blindData()["name"] = IECore.StringData( "blindPoints" )
		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "blindPoints" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "blindPoints" ]), points.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
		
		# name parameter takes preference over blindData
		converter["name"].setTypedValue( "testPoints" )
		self.assert_( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testPoints" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testPoints" ]), points.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
	
	def tearDown( self ) :
		
		if os.path.isfile( TestToHoudiniPointsConverter.__testScene ) :
			os.remove( TestToHoudiniPointsConverter.__testScene )

if __name__ == "__main__":
    unittest.main()
