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

class TestToHoudiniPointsConverter( IECoreHoudini.TestCase ) :

	if hou.applicationVersion()[0] >= 16:
		PointPositionAttribs = ['P']
	else:
		PointPositionAttribs = ['P', 'Pw']

	__testScene = "test/converterTest.hip"

	def points( self ) :
		pData = IECore.V3fVectorData( [
			imath.V3f( 0, 1, 2 ), imath.V3f( 1 ), imath.V3f( 2 ), imath.V3f( 3 ),
			imath.V3f( 4 ), imath.V3f( 5 ), imath.V3f( 6 ), imath.V3f( 7 ),
			imath.V3f( 8 ), imath.V3f( 9 ), imath.V3f( 10 ), imath.V3f( 11 ),
		] )

		points = IECoreScene.PointsPrimitive( pData )

		floatData = IECore.FloatData( 1.5 )
		v2fData = IECore.V2fData( imath.V2f( 1.5, 2.5 ) )
		v3fData = IECore.V3fData( imath.V3f( 1.5, 2.5, 3.5 ) )
		v3fData = IECore.V3fData( imath.V3f( 1.5, 2.5, 3.5 ) )
		color3fData = IECore.Color3fData( imath.Color3f( 1.5, 2.5, 3.5 ) )
		intData = IECore.IntData( 1 )
		v2iData = IECore.V2iData( imath.V2i( 1, 2 ) )
		v3iData = IECore.V3iData( imath.V3i( 1, 2, 3 ) )
		stringData = IECore.StringData( "this is a string" )
		m33fData = IECore.M33fData( imath.M33f(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0) )
		m44fData = IECore.M44fData( imath.M44f(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0) )

		intRange = list(range( 1, 13))
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		v2fVectorData = IECore.V2fVectorData( [ imath.V2f( x, x+0.5 ) for x in intRange ] )
		v3fVectorData = IECore.V3fVectorData( [ imath.V3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ imath.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		quatVectorData = IECore.QuatfVectorData( [ imath.Quatf( x, x+0.25, x+0.5, x+0.75 ) for x in intRange ] )
		intVectorData = IECore.IntVectorData( intRange )
		v2iVectorData = IECore.V2iVectorData( [ imath.V2i( x, -x ) for x in intRange ] )
		v3iVectorData = IECore.V3iVectorData( [ imath.V3i( x, -x, x*2 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %06d!" % x for x in intRange ] )
		m33fVectorData = IECore.M33fVectorData( [ imath.M33f(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0) for x in intRange ] )
		m44fVectorData = IECore.M44fVectorData( [ imath.M44f(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,16.0) for x in intRange ] )

		detailInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Constant
		uniformInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Uniform
		pointInterpolation = IECoreScene.PrimitiveVariable.Interpolation.Vertex

		# add all valid detail attrib types
		points["floatDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, floatData )
		points["v2fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v2fData )
		points["v3fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v3fData )
		points["color3fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, color3fData )
		points["intDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, intData )
		points["v2iDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v2iData )
		points["v3iDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, v3iData )
		points["stringDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, stringData )
		points["m33fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, m33fData )
		points["m44fDetail"] = IECoreScene.PrimitiveVariable( detailInterpolation, m44fData )

		# add all valid prim attrib types
		points["floatPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, floatVectorData[:1] )
		points["v2fPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, v2fVectorData[:1] )
		points["v3fPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, v3fVectorData[:1] )
		points["color3fPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, color3fVectorData[:1] )
		points["quatPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, quatVectorData[:1] )
		points["intPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, intVectorData[:1] )
		points["v2iPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, v2iVectorData[:1] )
		points["v3iPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, v3iVectorData[:1] )
		points["stringPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, stringVectorData[:1], IECore.IntVectorData( [ 0 ] ) )
		points["m33fPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, m33fVectorData[:1] )
		points["m44fPrim"] = IECoreScene.PrimitiveVariable( uniformInterpolation, m44fVectorData[:1] )

		# add all valid point attrib types
		points["floatPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, floatVectorData )
		points["v2fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v2fVectorData )
		points["v3fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v3fVectorData )
		points["color3fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, color3fVectorData )
		points["quatPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, quatVectorData )
		points["intPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, intVectorData )
		points["v2iPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v2iVectorData )
		points["v3iPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, v3iVectorData )
		points["stringPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, stringVectorData, IECore.IntVectorData( list(range( 0, 12)) ) )
		points["m33fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, m33fVectorData )
		points["m44fPoint"] = IECoreScene.PrimitiveVariable( pointInterpolation, m44fVectorData )

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

		def toTuple( v ):
			if isinstance( v, imath.M33f ):
				return (
					v[0][0], v[0][1], v[0][2],
					v[1][0], v[1][1], v[1][2],
					v[2][0], v[2][1], v[2][2]
				)
			elif isinstance( v, imath.M44f ):
				return (
					v[0][0], v[0][1], v[0][2], v[0][3],
					v[1][0], v[1][1], v[1][2], v[1][3],
					v[2][0], v[2][1], v[2][2], v[2][3],
					v[3][0], v[3][1], v[3][2], v[3][3]
				)
			else:
				return tuple( v )

		for key in [ "v2fDetail", "v3fDetail", "color3fDetail", "v2iDetail", "v3iDetail", "m33fDetail", "m44fDetail" ] :
			self.assertEqual( toTuple(prim[key].data.value), geo.attribValue( key ) )

		sopPrims = geo.prims()
		for key in [ "floatPrim", "intPrim", "stringPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPrims[i].attribValue( key ) )

		for key in [ "v2fPrim", "v3fPrim", "color3fPrim", "v2iPrim", "v3iPrim", "m33fPrim", "m44fPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( toTuple(data[i]), sopPrims[i].attribValue( key ) )

		self.assertEqual( geo.findPrimAttrib( "quatPrim" ).qualifier(), "Quaternion" )
		data = prim["quatPrim"].data
		for i in range( 0, data.size() ) :
			components = ( data[i].v()[0], data[i].v()[1], data[i].v()[2], data[i].r() )
			self.assertEqual( components, sopPrims[i].attribValue( "quatPrim" ) )

		data = prim["stringPrim"].data
		dataIndices = prim["stringPrim"].indices
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPrims[i].attribValue( "stringPrim" ) )

		sopPoints = geo.points()
		for key in [ "floatPoint", "intPoint" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( data[i], sopPoints[i].attribValue( key ) )

		for key in [ "P", "v2fPoint", "v3fPoint", "color3fPoint", "v2iPoint", "v3iPoint", "m33fPoint", "m44fPoint" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				self.assertEqual( toTuple(data[i]), sopPoints[i].attribValue( key ) )

		self.assertEqual( geo.findPointAttrib( "quatPoint" ).qualifier(), "Quaternion" )
		data = prim["quatPoint"].data
		for i in range( 0, data.size() ) :
			components = ( data[i].v()[0], data[i].v()[1], data[i].v()[2], data[i].r() )
			self.assertEqual( components, sopPoints[i].attribValue( "quatPoint" ) )

		data = prim["stringPoint"].data
		dataIndices = prim["stringPoint"].indices
		for i in range( 0, data.size() ) :
			self.assertEqual( data[ dataIndices[i] ], sopPoints[i].attribValue( "stringPoint" ) )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.keys(), prim.keys() )
		for key in prim.keys() :
			self.assertEqual( result[key], prim[key], (result[key].interpolation, result[key].data, prim[key].interpolation, prim[key].data) )
		self.assertEqual( result, prim )

	def comparePrimAndAppendedSop( self, prim, sop, origSopPrim, multipleConversions=0 ) :
		geo = sop.geometry()
		# verify detail attribs
		for key in [ "floatDetail", "intDetail", "stringDetail" ] :
			self.assertEqual( prim[key].data.value, geo.attribValue( key ) )

		def toTuple( v ):
			if isinstance( v, imath.M33f ):
				return (
					v[0][0], v[0][1], v[0][2],
					v[1][0], v[1][1], v[1][2],
					v[2][0], v[2][1], v[2][2]
				)
			elif isinstance( v, imath.M44f ):
				return (
					v[0][0], v[0][1], v[0][2], v[0][3],
					v[1][0], v[1][1], v[1][2], v[1][3],
					v[2][0], v[2][1], v[2][2], v[2][3],
					v[3][0], v[3][1], v[3][2], v[3][3]
				)
			else:
				return tuple( v )

		for key in [ "v2fDetail", "v3fDetail", "color3fDetail", "v2iDetail", "v3iDetail", "m33fDetail", "m44fDetail" ] :
			self.assertEqual( toTuple(prim[key].data.value), geo.attribValue( key ) )

		# verify prim attribs
		sopPrims = geo.prims()
		numPrims = multipleConversions + 1
		self.assertEqual( len(sopPrims), numPrims )

		for key in [ "floatPrim", "intPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				for sopPrim in sopPrims :
					self.assertEqual( data[i], sopPrim.attribValue( key ) )

		for key in [ "v2fPrim", "v3fPrim", "color3fPrim", "v2iPrim", "v3iPrim", "m33fPrim", "m44fPrim" ] :
			data = prim[key].data
			for i in range( 0, data.size() ) :
				for sopPrim in sopPrims :
					self.assertEqual( toTuple(data[i]), sopPrim.attribValue( key ) )

		data = prim["stringPrim"].data
		dataIndices = prim["stringPrim"].indices
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

		for key in [ "P", "v2fPoint", "v3fPoint", "color3fPoint", "v2iPoint", "v3iPoint", "m33fPoint", "m44fPoint" ] :
			data = prim[key].data

			if multipleConversions or key is "P" :
				defaultValue = origSopPrim[key].data
			elif isinstance( data, IECore.M33fVectorData ) :
				defaultValue = [ [ 0 ] * 9 ] * origSopPrim.numPoints
			elif isinstance( data, IECore.M44fVectorData ) :
				defaultValue = [ [ 0 ] * 16 ] * origSopPrim.numPoints
			else :
				defaultValue = [ [ 0 ] * data[0].dimensions() ] * origSopPrim.numPoints

			for i in range( 0, origSopPrim.numPoints ) :
				self.assertEqual( toTuple(defaultValue[i]), sopPoints[ i ].attribValue( key ) )

			for i in range( 0, data.size() ) :
				self.assertEqual( toTuple(data[i]), sopPoints[ origSopPrim.numPoints + i ].attribValue( key ) )

		data = prim["stringPoint"].data
		dataIndices = prim["stringPoint"].indices

		if multipleConversions :
			defaultData = origSopPrim["stringPoint"].data
			defaultIndices = origSopPrim["stringPoint"].indices
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
				self.assertTrue( key in result.keys() )

	def testCreateConverter( self )  :
		converter = IECoreHoudini.ToHoudiniPointsConverter( self.points() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPointsConverter ) ) )

	def testFactory( self ) :
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( self.points() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniPointsConverter ) ) )
		self.assertTrue( IECoreScene.TypeId.PointsPrimitive in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )

	def testConversionIntoEmptySop( self ) :
		points = self.points()
		sop = self.emptySop()

		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), points )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop ) )

		self.comparePrimAndSop( points, sop )

	def testConversionIntoExistingSop( self ) :
		points = self.points()
		sop = self.pointsSop()

		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )

		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, False ) )

		self.comparePrimAndSop( points, sop )

	def testAppendingIntoExistingSop( self ) :
		points = self.points()
		sop = self.pointsSop()

		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )

		self.assertTrue( not sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( points, sop, orig )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )

		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints )
		self.assertTrue( "floatDetail" not in result.keys() )
		self.assertTrue( "floatPoint" not in result.keys() )

	def testAppendingIntoLockedSop( self ) :
		points = self.points()
		sop = self.pointsSop()

		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )

		sop.setHardLocked( True )
		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( points, sop, orig )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )

		sop.setHardLocked( False )
		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints )
		self.assertTrue( "floatDetail" not in result.keys() )
		self.assertTrue( "floatPoint" not in result.keys() )

	def testSaveLoad( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )

		points = self.points()
		sop = self.pointsSop()
		sopPath = sop.path()

		orig = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( orig, points )

		self.assertTrue( not sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( points, sop, orig )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )

		hou.hipFile.save( TestToHoudiniPointsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPointsConverter.__testScene )

		newSop = hou.node( sopPath )

		self.assertTrue( newSop.isHardLocked() )

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
		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( points, sop, orig )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )

		hou.hipFile.save( TestToHoudiniPointsConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniPointsConverter.__testScene )

		newSop = hou.node( sopPath )

		self.assertTrue( newSop.isHardLocked() )

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

		self.assertTrue( not sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( points, sop, orig )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )

		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

		self.comparePrimAndAppendedSop( points, sop, result, multipleConversions=1 )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertEqual( result.numPoints, orig.numPoints + 2*points.numPoints )
		for i in range( 0, points["P"].data.size() ) :
			self.assertEqual( result["P"].data[ orig.numPoints + i ], points["P"].data[i] )
			self.assertEqual( result["P"].data[ orig.numPoints + points.numPoints + i ], points["P"].data[i] )

		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )

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

		self.assertTrue( converter.convert( sop, False ) )

		self.comparePrimAndSop( points, sop )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()

		del points

		sop.setHardLocked( False )
		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), result )
		self.assertTrue( converter.convert( sop, False ) )
		self.assertEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), result )

	def testWithUnacceptablePrimVars( self ) :
		points = self.points()
		points["badDetail"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Constant, IECore.TransformationMatrixfData() )
		points["badPoint"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.DoubleVectorData( [ 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5 ] ) )
		sop = self.emptySop()

		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), points )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop ) )

		self.assertTrue( "badDetail" not in [ x.name() for x in sop.geometry().globalAttribs() ] )
		self.assertTrue( "badPoint" not in [ x.name() for x in sop.geometry().pointAttribs() ] )

		result = IECoreHoudini.FromHoudiniPointsConverter( sop ).convert()
		self.assertNotEqual( result, points )
		self.assertTrue( "badDetail" not in result )
		self.assertTrue( "badPoint" not in result )

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
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( pointAttr ) )
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
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( pointAttr ) )
		self.comparePrimAndSop( points, pointAttr )

	def testVertAttribsCantBeConverted( self ) :
		points = self.points()
		points["floatVert"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.FaceVarying, IECore.FloatVectorData( 1 ) )
		sop = self.emptySop()

		self.assertNotEqual( IECoreHoudini.FromHoudiniPointsConverter( sop ).convert(), points )
		self.assertTrue( IECoreHoudini.ToHoudiniPointsConverter( points ).convert( sop ) )

		allAttribs = [ x.name() for x in sop.geometry().globalAttribs() ]
		allAttribs.extend( [ x.name() for x in sop.geometry().pointAttribs() ] )
		allAttribs.extend( [ x.name() for x in sop.geometry().primAttribs() ] )
		allAttribs.extend( [ x.name() for x in sop.geometry().vertexAttribs() ] )
		self.assertTrue( "floatVert" not in allAttribs )

		del points["floatVert"]

		self.comparePrimAndSop( points, sop )

	def testAttributeFilter( self ) :

		points = self.points()
		sop = self.emptySop()

		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniPointsConverter.PointPositionAttribs + ['color3fPoint', 'floatPoint', 'intPoint', 'm33fPoint', 'm44fPoint', 'quatPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'floatPrim', 'intPrim', 'm33fPrim', 'm44fPrim', 'quatPrim', 'stringPrim', 'v2fPrim', 'v2iPrim', 'v3fPrim', 'v3iPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'floatDetail', 'intDetail', 'm33fDetail', 'm44fDetail', 'stringDetail', 'v2fDetail', 'v2iDetail', 'v3fDetail', 'v3iDetail'] )

		converter.parameters()["attributeFilter"].setTypedValue( "P *3f*" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniPointsConverter.PointPositionAttribs + ['color3fPoint', 'm33fPoint', 'v3fPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), ['color3fPrim', 'm33fPrim', 'v3fPrim'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), ['color3fDetail', 'm33fDetail', 'v3fDetail'] )

		converter.parameters()["attributeFilter"].setTypedValue( "* ^*Detail ^int* ^*Prim" )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), TestToHoudiniPointsConverter.PointPositionAttribs + ['color3fPoint', 'floatPoint', 'm33fPoint', 'm44fPoint', 'quatPoint', 'stringPoint', 'v2fPoint', 'v2iPoint', 'v3fPoint', 'v3iPoint'] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )

	def testStandardAttributeConversion( self ) :

		sop = self.emptySop()
		points = IECoreScene.PointsPrimitive( IECore.V3fVectorData( [ imath.V3f( 1 ) ] * 10 ) )
		points["Cs"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ imath.V3f( 1, 0, 0 ) ] * 10, IECore.GeometricData.Interpretation.Color ) )
		points["width"] = IECoreScene.PrimitiveVariable( IECoreScene.PrimitiveVariable.Interpolation.Vertex, IECore.FloatVectorData( [ 1 ] * 10 ) )
		points["Pref"] = points["P"]

		self.assertTrue( points.arePrimitiveVariablesValid() )

		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), sorted( TestToHoudiniPointsConverter.PointPositionAttribs + ['Cd', 'pscale', 'rest'] ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )

		converter["convertStandardAttributes"].setTypedValue( False )
		self.assertTrue( converter.convert( sop ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().pointAttribs() ]), sorted( TestToHoudiniPointsConverter.PointPositionAttribs + ['Cs', 'Pref', 'width'] ) )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().vertexAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in sop.geometry().globalAttribs() ]), [] )

	def testName( self ) :

		sop = self.emptySop()
		points = self.points()
		converter = IECoreHoudini.ToHoudiniPointsConverter( points )

		# unnamed unless we set the parameter
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		self.assertEqual( sop.geometry().findPrimAttrib( "name" ), None )

		converter["name"].setTypedValue( "testPoints" )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testPoints" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testPoints" ]), points.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

		# blindData still works for backwards compatibility
		points.blindData()["name"] = IECore.StringData( "blindPoints" )
		converter = IECoreHoudini.ToHoudiniPointsConverter( points )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "blindPoints" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "blindPoints" ]), points.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

		# name parameter takes preference over blindData
		converter["name"].setTypedValue( "testPoints" )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), tuple( [ "testPoints" ] ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "testPoints" ]), points.variableSize( IECoreScene.PrimitiveVariable.Interpolation.Uniform ) )

	def tearDown( self ) :

		if os.path.isfile( TestToHoudiniPointsConverter.__testScene ) :
			os.remove( TestToHoudiniPointsConverter.__testScene )

if __name__ == "__main__":
    unittest.main()
