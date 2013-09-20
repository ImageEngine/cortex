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
import IECoreHoudini
import unittest
import os

class TestToHoudiniCompoundObjectConverter( IECoreHoudini.TestCase ) :
	
	__testScene = "test/converterTest.hip"
	
	def mesh( self ) :
		
		vertsPerFace = IECore.IntVectorData( [ 4, 4, 4, 4, 4, 4 ] )
		vertexIds = IECore.IntVectorData( [ 1, 5, 4, 0, 2, 6, 5, 1, 3, 7, 6, 2, 0, 4, 7, 3, 2, 1, 0, 3, 5, 6, 7, 4 ] )
		mesh = IECore.MeshPrimitive( vertsPerFace, vertexIds )
		
		intRange = range( 1, 25 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %d!" % x for x in intRange ] )
		
		detailInterpolation = IECore.PrimitiveVariable.Interpolation.Constant
		pointInterpolation = IECore.PrimitiveVariable.Interpolation.Vertex
		primitiveInterpolation = IECore.PrimitiveVariable.Interpolation.Uniform
		vertexInterpolation = IECore.PrimitiveVariable.Interpolation.FaceVarying
		
		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
		], IECore.GeometricData.Interpretation.Point )
		mesh["P"] = IECore.PrimitiveVariable( pointInterpolation, pData )
		mesh["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatVectorData[:8] )
		mesh["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fVectorData[:8] )
		mesh["stringPoint"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:8] )
		mesh["stringPointIndices"] = IECore.PrimitiveVariable( pointInterpolation, IECore.IntVectorData( range( 0, 8 ) ) )
		
		mesh["floatPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, floatVectorData[:6] )
		mesh["color3fPrim"] = IECore.PrimitiveVariable( primitiveInterpolation, color3fVectorData[:6] )
		mesh["stringPrim"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:6] )
		mesh["stringPrimIndices"] = IECore.PrimitiveVariable( primitiveInterpolation, IECore.IntVectorData( range( 0, 6 ) ) )
		
		mesh["floatVert"] = IECore.PrimitiveVariable( vertexInterpolation, floatVectorData )
		mesh["color3fVert"] = IECore.PrimitiveVariable( vertexInterpolation, color3fVectorData )
		mesh["stringVert"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData )
		mesh["stringVertIndices"] = IECore.PrimitiveVariable( vertexInterpolation, IECore.IntVectorData( range( 0, 24 ) ) )
		
		return mesh
	
	def points( self ) :
		
		pData = IECore.V3fVectorData( [
			IECore.V3f( 0, 1, 2 ), IECore.V3f( 1 ), IECore.V3f( 2 ), IECore.V3f( 3 ),
			IECore.V3f( 4 ), IECore.V3f( 5 ), IECore.V3f( 6 ), IECore.V3f( 7 ),
			IECore.V3f( 8 ), IECore.V3f( 9 ), IECore.V3f( 10 ), IECore.V3f( 11 ),
		] )
		
		points = IECore.PointsPrimitive( pData )
		
		intRange = range( 1, 13 )
		floatVectorData = IECore.FloatVectorData( [ x+0.5 for x in intRange ] )
		color3fVectorData = IECore.Color3fVectorData( [ IECore.Color3f( x, x+0.5, x+0.75 ) for x in intRange ] )
		stringVectorData = IECore.StringVectorData( [ "string number %d!" % x for x in intRange ] )
		
		detailInterpolation = IECore.PrimitiveVariable.Interpolation.Constant
		uniformInterpolation = IECore.PrimitiveVariable.Interpolation.Uniform
		pointInterpolation = IECore.PrimitiveVariable.Interpolation.Vertex
		
		points["floatPrim"] = IECore.PrimitiveVariable( uniformInterpolation, floatVectorData[:1] )
		points["color3fPrim"] = IECore.PrimitiveVariable( uniformInterpolation, color3fVectorData[:1] )
		points["stringPrim"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData[:1] )
		points["stringPrimIndices"] = IECore.PrimitiveVariable( uniformInterpolation, IECore.IntVectorData( [ 0 ] ) )
		
		points["floatPoint"] = IECore.PrimitiveVariable( pointInterpolation, floatVectorData )
		points["color3fPoint"] = IECore.PrimitiveVariable( pointInterpolation, color3fVectorData )
		points["stringPoint"] = IECore.PrimitiveVariable( detailInterpolation, stringVectorData )
		points["stringPointIndices"] = IECore.PrimitiveVariable( pointInterpolation, IECore.IntVectorData( range( 0, 12 ) ) )
		
		return points
	
	def ints( self ) :
		
		return IECore.IntVectorData( range( 0, 10 ) )
	
	def compound( self ) :
		
		return IECore.CompoundObject( {
			"mesh" : self.mesh(),
			"points" : self.points(),
			"ints" : self.ints()
		} )
	
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
		
		converter = IECoreHoudini.ToHoudiniCompoundObjectConverter( self.compound() )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCompoundObjectConverter ) ) )
		
		self.assertRaises( Exception, IECore.curry( IECoreHoudini.ToHoudiniCompoundObjectConverter, self.mesh() ) )
	
	def testFactory( self ) :
		
		obj = self.compound()
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( obj )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCompoundObjectConverter ) ) )
		
		del obj["ints"]
		
		converter = IECoreHoudini.ToHoudiniGeometryConverter.create( obj )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.ToHoudiniCompoundObjectConverter ) ) )
		
		self.assertTrue( IECore.TypeId.CompoundObject in IECoreHoudini.ToHoudiniGeometryConverter.supportedTypes() )
	
	def verifySop( self, sop, obj, name = "" ) :
		
		prims = sop.geometry().prims()
		self.assertEqual( len(prims), 3 )
		for i in range( 0, len(prims) ) :
			self.assertEqual( prims[i].type(), hou.primType.Custom )
			self.assertEqual( prims[i].vertices()[0].point().number(), i )
			self.assertEqual( prims[i].attribValue( "name" ), obj.keys()[i] )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop, name )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCompoundObjectConverter ) ) )
		self.assertEqual( obj, converter.convert() )
	
	def testConversionIntoEmptySop( self ) :
		
		obj = self.compound()
		sop = self.emptySop()
		
		self.assertTrue( IECoreHoudini.ToHoudiniCompoundObjectConverter( obj ).convert( sop ) )
		self.verifySop( sop, obj )
	
	def testConversionIntoExistingSop( self ) :
		
		obj = self.compound()
		sop = self.meshSop()
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertNotEqual( obj, converter.convert() )
		
		self.assertTrue( IECoreHoudini.ToHoudiniCompoundObjectConverter( obj ).convert( sop ) )
		self.verifySop( sop, obj )
	
	def verifyAppendedSop( self, sop, obj, orig, origName = "realGeo", numAppends = 1 ) :
		
		origNumPoints = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex )
		origNumPolys = orig.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform )
		
		prims = sop.geometry().prims()
		numCustom = len(obj.values()) * numAppends
		self.assertEqual( len(prims), origNumPolys + numCustom )
		origPrims = prims[:-numCustom]
		for prim in origPrims :
			self.assertEqual( prim.type(), hou.primType.Polygon )
			self.assertEqual( prim.attribValue( "name" ), origName )
		
		newPrims = prims[len(prims) - numCustom:]
		for i in range( 0, len(newPrims) ) :
			prim = newPrims[i]
			self.assertEqual( prim.type(), hou.primType.Custom )
			self.assertEqual( prim.vertices()[0].point().number(), origNumPoints + i )
		
		for i in range( 0, len(newPrims), len(obj.keys()) ) :
			for j in range( 0, len(obj.keys()) ) :
				self.assertEqual( newPrims[i+j].attribValue( "name" ), obj.keys()[j] )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop, "* ^"+origName )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCompoundObjectConverter ) ) )
		self.assertEqual( obj, converter.convert() )
		
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop, origName )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertEqual( orig, converter.convert() )
	
	def testAppendingIntoExistingSop( self ) :
		
		obj = self.compound()
		sop = self.meshSop()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, obj )
		
		self.assertTrue( not sop.isHardLocked() )
		converter = IECoreHoudini.ToHoudiniCompoundObjectConverter( obj )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )
		
		self.verifyAppendedSop( sop, obj, orig )
		
		sop.setHardLocked( False )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertEqual( orig, converter.convert() )
	
	def testAppendingIntoLockedSop( self ) :
		
		obj = self.compound()
		sop = self.meshSop()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, obj )
		
		sop.setHardLocked( True )
		self.assertTrue( sop.isHardLocked() )
		converter = IECoreHoudini.ToHoudiniCompoundObjectConverter( obj )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )
		
		self.verifyAppendedSop( sop, obj, orig )
		
		sop.setHardLocked( False )
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( sop )
		self.assertTrue( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		self.assertEqual( orig, converter.convert() )
	
	def testSaveLoad( self ) :
		
		hou.hipFile.clear( suppress_save_prompt=True )
		
		obj = self.compound()
		sop = self.emptySop()
		sopPath = sop.path()
		
		self.assertTrue( IECoreHoudini.ToHoudiniCompoundObjectConverter( obj ).convert( sop ) )
		self.verifySop( sop, obj )
		
		hou.hipFile.save( TestToHoudiniCompoundObjectConverter.__testScene )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( TestToHoudiniCompoundObjectConverter.__testScene )
		
		newSop = hou.node( sopPath )
		self.assertTrue( newSop.isHardLocked() )
		self.verifySop( newSop, obj )
	
	def testMultipleConversions( self ) :
		
		obj = self.compound()
		sop = self.meshSop()
		
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( sop ).convert()
		self.assertNotEqual( orig, obj )
		
		converter = IECoreHoudini.ToHoudiniCompoundObjectConverter( obj )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )
		self.verifyAppendedSop( sop, obj, orig )
		
		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )		
		self.verifyAppendedSop( sop, obj, orig, numAppends = 2 )
		
		self.assertTrue( sop.isHardLocked() )
		self.assertTrue( converter.convert( sop, True ) )
		self.assertTrue( sop.isHardLocked() )
		self.verifyAppendedSop( sop, obj, orig, numAppends = 3 )
	
	def testObjectWasDeleted( self ) :
		
		obj = self.compound()
		sop = self.emptySop()
		
		converter = IECoreHoudini.ToHoudiniCompoundObjectConverter( obj )
		self.assertTrue( converter.convert( sop ) )
		self.verifySop( sop, obj )
		
		del obj
		
		result = IECoreHoudini.FromHoudiniCompoundObjectConverter( sop ).convert()
		self.verifySop( sop, result )
	
	def testName( self ) :
		
		obj = self.compound()
		sop = self.emptySop()
		converter = IECoreHoudini.ToHoudiniCompoundObjectConverter( obj )
		
		# members of the CompoundObject maintain their names
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), ( "ints", "mesh", "points" ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "ints" ]), 1 )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "mesh" ]), 1 )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "points" ]), 1 )
		
		# blindData is meaningless
		obj["mesh"].blindData()["name"] = IECore.StringData( "blindMesh" )
		converter = IECoreHoudini.ToHoudiniCompoundObjectConverter( obj )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), ( "ints", "mesh", "points" ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "ints" ]), 1 )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "mesh" ]), 1 )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "points" ]), 1 )
		
		# name parameter adds parent name
		converter["name"].setTypedValue( "compound" )
		self.assertTrue( converter.convert( sop ) )
		geo = sop.geometry()
		nameAttr = sop.geometry().findPrimAttrib( "name" )
		self.assertEqual( nameAttr.strings(), ( "compound/ints", "compound/mesh", "compound/points" ) )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "compound/ints" ]), 1 )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "compound/mesh" ]), 1 )
		self.assertEqual( len([ x for x in geo.prims() if x.attribValue( "name" ) == "compound/points" ]), 1 )
	
	def tearDown( self ) :
		
		if os.path.isfile( TestToHoudiniCompoundObjectConverter.__testScene ) :
			os.remove( TestToHoudiniCompoundObjectConverter.__testScene )

if __name__ == "__main__":
    unittest.main()
