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
import shutil

class TestInterpolatedCacheReader( IECoreHoudini.TestCase ):
	
	__torusTestFile = "test/IECoreHoudini/data/torus.cob"
	
	def cacheSop( self, file=__torusTestFile ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		torus = geo.createNode( "file" )
		torus.parm( "file" ).set( file )
		group = torus.createOutputNode( "group" )
		group.parm( "crname" ).set( "torus" )
		group.parm( "entity" ).set( 1 )
		cache = group.createOutputNode( "ieInterpolatedCacheReader" )
		cache.parm( "cacheSequence" ).set( "test/IECoreHoudini/data/torusVertCache.####.fio" )
		cache.setDisplayFlag( True )
		cache.setRenderFlag( True )
		return cache
	
	def testCreate( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		sop = geo.createNode( "ieInterpolatedCacheReader" )
		self.failUnless( sop )
		self.assertEqual( "Sop/ieInterpolatedCacheReader", sop.type().nameWithCategory() )
	
	def testTimeDependency( self ) :
		cache = self.cacheSop()
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		hou.setFrame( 1.5 )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assertNotEqual( orig, result )
		self.assertNotEqual( orig['P'], result['P'] )
		self.assertNotEqual( orig['Cs'], result['Cs'] )
		self.assertEqual( orig['pointId'], result['pointId'] )
	
	def testSubFrameData( self ) :
		cache = self.cacheSop()
		self.failUnless( isinstance( cache.geometry(), hou.Geometry ) )
		cache.parm( "samplesPerFrame" ).set( 1 )
		cache.parm( "interpolation" ).set( IECore.InterpolatedCache.Interpolation.Linear )
		hou.setFrame( 1 )
		cache.cook( force=True )
		self.assertEqual( len(cache.geometry().points()), 100 )
		pos1 = cache.geometry().points()[0].position()
		hou.setFrame( 2 )
		cache.cook( force=True )
		self.assertEqual( len(cache.geometry().points()), 100 )
		pos2 = cache.geometry().points()[0].position()
		self.assertNotEqual( pos1, pos2 )
		hou.setFrame( 1.8 )
		cache.cook( force=True )
		self.assertEqual( len(cache.geometry().points()), 100 )
		
		# samplesPerFrame 1 ignores the file that exists for 1.8, and linear interpolation blends 1 and 2
		pos18 = cache.geometry().points()[0].position()
		self.assertNotEqual( pos18, pos1 )
		self.assertNotEqual( pos18, pos2 )
		self.failUnless( pos18.isAlmostEqual( pos1*0.2 + pos2*0.8 ) )
		
		# samplesPerFrame 2 doesn't have the correct files on disk
		cache.parm( "samplesPerFrame" ).set( 2 )
		cache.cook( force=True )
		self.failUnless( cache.warnings() )
		self.assertEqual( len(cache.geometry().points()), 0 )
		
		# samplesPerFrame 1 ignores the file that exists for 1.8, and no interpolation chooses frame 1
		cache.parm( "samplesPerFrame" ).set( 1 )
		cache.parm( "interpolation" ).set( IECore.InterpolatedCache.Interpolation.None )
		cache.cook( force=True )
		pos18 = cache.geometry().points()[0].position()
		self.assertEqual( pos18, pos1 )
		self.assertNotEqual( pos18, pos2 )
		
		# samplesPerFrame 1 ignores the file that exists for 1.8, and cubic interpolation can't find the necessary files
		cache.parm( "interpolation" ).set( IECore.InterpolatedCache.Interpolation.Cubic )
		cache.cook( force=True )
		self.failUnless( cache.warnings() )
		self.assertEqual( len(cache.geometry().points()), 0 )
		
		# samplesPerFrame 5 uses the file for 1.8 for linear and no interpolation
		cache.parm( "samplesPerFrame" ).set( 5 )
		cache.parm( "interpolation" ).set( IECore.InterpolatedCache.Interpolation.Linear )
		cache.cook( force=True )
		pos18 = cache.geometry().points()[0].position()
		self.assertNotEqual( pos18, pos1 )
		# this is true because the 1.8 file is an exact copy of frame 2
		self.assertEqual( pos18, pos2 )
		cache.parm( "interpolation" ).set( IECore.InterpolatedCache.Interpolation.None )
		cache.cook( force=True )
		pos18 = cache.geometry().points()[0].position()
		self.assertNotEqual( pos18, pos1 )
		self.assertEqual( pos18, pos2 )
		
		# samplesPerFrame 5 has enough surrounding files for cubic
		cache.parm( "interpolation" ).set( IECore.InterpolatedCache.Interpolation.Cubic )
		cache.cook( force=True )
		pos18 = cache.geometry().points()[0].position()
		self.assertNotEqual( pos18, pos1 )
		self.assertEqual( pos18, pos2 )
	
	def testNonExistantFile( self ) :
		cache = self.cacheSop()
		cache.parm( "cacheSequence" ).set( "test/IECoreHoudini/data/fake.####.fio" )
		cache.cook( force=True )
		self.failUnless( cache.warnings() )
		self.assertEqual( len(cache.geometry().points()), 0 )
		
	def testNoFileForFrame( self ) :
		cache = self.cacheSop()
		cache.parm( "cacheSequence" ).set( "test/IECoreHoudini/data/torusVertCache.####.fio" )
		hou.setFrame( 5 )
		cache.cook( force=True )
		self.failUnless( cache.warnings() )
		self.assertEqual( len(cache.geometry().points()), 0 )
	
	def testBadObjectHandle( self ) :
		cache = self.cacheSop()
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" in orig )
		cache.parm( "objectFixes1" ).set( "nope" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" not in result )
		
	def testNoGroups( self ) :
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.destroy()
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" not in result )
		
	def testEmptyGroup( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		groupA = cache.inputs()[0]
		null = cache.parent().createNode( "null" )
		groupB = null.createOutputNode( "group" )
		groupB.parm( "crname" ).set( "torus2" )
		groupB.parm( "entity" ).set( 1 )
		merge = groupA.createOutputNode( "merge" )
		merge.setInput( 1, groupB )
		cache.setInput( 0, merge )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		numTorusPoints = len(groupA.geometry().points())
		for key in [ "P", "Cs" ] :
			self.assert_( key in result )
			self.assertEqual( result[key].data.size(), numTorusPoints )
		
	def testObjectPrefixAndSuffix( self ) :
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "rus" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" not in result )
		cache.parm( "objectFixes1" ).set( "to" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" in result )
		
		cache.parm( "objectFixes1" ).set( "" )
		group.parm( "crname" ).set( "to" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" not in result )
		cache.parm( "objectFixes2" ).set( "rus" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" in result )
		
		cache.parm( "objectFixes1" ).set( "" )
		cache.parm( "objectFixes2" ).set( "" )
		group.parm( "crname" ).set( "oru" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" not in result )
		cache.parm( "objectFixes1" ).set( "t" )
		cache.parm( "objectFixes2" ).set( "s" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" in result )
	
	def testMultipleObjects( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		torus = cache.parent().createNode( "file" )
		torus.parm( "file" ).set( TestInterpolatedCacheReader.__torusTestFile )
		group = torus.createOutputNode( "group" )
		group.parm( "crname" ).set( "torus2" )
		group.parm( "entity" ).set( 1 )
		merge = cache.inputs()[0].createOutputNode( "merge" )
		merge.setInput( 1, group )
		cache.setInput( 0, merge )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		numTorusPoints = len(torus.geometry().points()) 
		for key in [ "P", "Cs" ] :
			self.assert_( key in result )
			self.assertEqual( result[key].data.size(), 2 * numTorusPoints )
		
		for i in range( 0, numTorusPoints ) :
			self.assertNotEqual( result['P'].data[i], result['P'].data[numTorusPoints+i] )
			self.assertEqual( result['Cs'].data[i], result['Cs'].data[numTorusPoints+i] )
	
	def testAttributeMismatchBetweenFrames( self ) :
		cache = self.cacheSop()
		hou.setFrame( 1.5 )
		self.failUnless( isinstance( cache.geometry(), hou.Geometry ) )
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		hou.setFrame( 2.5 )
		self.assertEqual( cache.geometry(), None )
		self.failUnless( "Entry not found" in cache.errors() )
		hou.setFrame( 3 )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( isinstance( cache.geometry(), hou.Geometry ) )
		self.assertNotEqual( orig, result )
	
	def testAttrPrefixAndSuffix( self ) :
		cache = self.cacheSop()
		cache.parm( "attributeFixes1" ).set( "C" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" not in result )
		self.assert_( "pointId" in result )
		self.assert_( "d" in result )
		
		cache.parm( "attributeFixes1" ).set( "" )
		cache.parm( "attributeFixes2" ).set( "tId" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" in result )
		self.assert_( "pointId" not in result )
		self.assert_( "poin" in result )
		
		cache.parm( "attributeFixes1" ).set( "po" )
		cache.parm( "attributeFixes2" ).set( "tId" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" in result )
		self.assert_( "pointId" not in result )
		self.assert_( "in" in result )
		
		cache.parm( "attributeFixes1" ).set( "C" )
		cache.parm( "attributeFixes2" ).set( "tId" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" not in result )
		self.assert_( "pointId" not in result )
		self.assert_( "poin" in result )
		self.assert_( "d" in result )
		
		cache.parm( "attributeFixes1" ).set( "oin" )
		cache.parm( "attributeFixes2" ).set( "tI" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cs" in result )
		self.assert_( "pointId" in result )
		
	def testNonDataAttr( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		torus = cache.parent().createNode( "file" )
		torus.parm( "file" ).set( TestInterpolatedCacheReader.__torusTestFile )
		group = torus.createOutputNode( "group" )
		group.parm( "crname" ).set( "badObject" )
		group.parm( "entity" ).set( 1 )
		merge = cache.inputs()[0].createOutputNode( "merge" )
		merge.setInput( 1, group )
		cache.setInput( 0, merge )
		i = IECore.InterpolatedCache( cache.parm( "cacheSequence" ).eval(), IECore.InterpolatedCache.Interpolation.Linear, IECore.OversamplesCalculator( 24, 1 ) )
		self.assert_( "notData" in i.attributes( 3, 'badObject' ) )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "notData" not in result )
		
	def testNonConvertableAttr( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		torus = cache.parent().createNode( "file" )
		torus.parm( "file" ).set( TestInterpolatedCacheReader.__torusTestFile )
		group = torus.createOutputNode( "group" )
		group.parm( "crname" ).set( "badObject" )
		group.parm( "entity" ).set( 1 )
		merge = cache.inputs()[0].createOutputNode( "merge" )
		merge.setInput( 1, group )
		cache.setInput( 0, merge )
		i = IECore.InterpolatedCache( cache.parm( "cacheSequence" ).eval(), IECore.InterpolatedCache.Interpolation.Linear, IECore.OversamplesCalculator( 24, 1 ) )
		self.assert_( "splineColor4fData" in i.attributes( 3, 'badObject' ) )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "splineColor4fData" not in result )
	
	def testShortP( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "badObject" )
		cache.parm( "attributeFixes1" ).set( "short" )
		cache.cook( force=True )
		self.failUnless( "Geometry/Cache mismatch" in cache.warnings() )
	
	def testLongP( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "badObject" )
		cache.parm( "attributeFixes1" ).set( "long" )
		cache.cook( force=True )
		self.failUnless( "Geometry/Cache mismatch" in cache.warnings() )

	def testWrongP( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "badObject" )
		cache.parm( "attributeFixes1" ).set( "wrong" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "P" in result )
	
	def testTransformAttribute( self ) :
		cache = self.cacheSop()
		hou.setFrame( 2 )
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		cache.parm( "transformAttribute" ).set( "transform" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assertNotEqual( orig, result )
		self.assertNotEqual( orig['P'], result['P'] )
		self.assertEqual( orig['P'].data.size(), result['P'].data.size() )
		self.assertEqual( orig['Cs'], result['Cs'] )
		self.assertEqual( orig['pointId'], result['pointId'] )
		
		i = IECore.InterpolatedCache( cache.parm( "cacheSequence" ).eval(), IECore.InterpolatedCache.Interpolation.Linear, IECore.OversamplesCalculator( 24, 1 ) )
		matrix = i.read( 2, "torus", "transform" ).value.transform
		origP = orig["P"].data
		resultP = result["P"].data
		for i in range( 0, origP.size() ) :
			self.assertNotEqual( resultP[i], origP[i] )
			self.assertEqual( resultP[i], matrix.multVecMatrix( origP[i] ) )
	
	def testPrimitiveGroupModeWithTransformAttribute( self ) :
		cache = self.cacheSop( file="test/IECoreHoudini/data/torusWithVertexNormals.bgeo" )
		cache.parm( "groupingMode" ).set( 0 )
		hou.parm( "/obj/geo1/group1/entity" ).set( 0 )
		hou.setFrame( 2 )
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		cache.parm( "transformAttribute" ).set( "transform" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assertNotEqual( orig, result )
		self.assertNotEqual( orig['P'], result['P'] )
		self.assertNotEqual( orig['N'], result['N'] )
		self.assertEqual( orig['P'].data.size(), result['P'].data.size() )
		self.assertEqual( orig['N'].data.size(), result['N'].data.size() )
		self.assertEqual( orig['Cs'], result['Cs'] )
		self.assertEqual( orig['pointId'], result['pointId'] )
		
		i = IECore.InterpolatedCache( cache.parm( "cacheSequence" ).eval(), IECore.InterpolatedCache.Interpolation.Linear, IECore.OversamplesCalculator( 24, 1 ) )
		matrix = i.read( 2, "torus", "transform" ).value.transform
		origP = orig["P"].data
		origN = orig["N"].data
		resultP = result["P"].data
		resultN = result["N"].data
		for i in range( 0, origP.size() ) :
			self.assertNotEqual( resultP[i], origP[i] )
			self.assertNotEqual( resultN[i], origN[i] )
			self.assertEqual( resultP[i], matrix.multVecMatrix( origP[i] ) )
			self.failUnless( resultN[i].equalWithAbsError( matrix.multDirMatrix( origN[i] ), 1e-6 ) )
	
	def testPrimitiveGroupModeWithMultipleObjects( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop( file="test/IECoreHoudini/data/torusWithVertexNormals.bgeo" )
		cache.parm( "groupingMode" ).set( 0 )
		hou.parm( "/obj/geo1/group1/entity" ).set( 0 )
		torus = cache.parent().createNode( "file" )
		torus.parm( "file" ).set( "test/IECoreHoudini/data/torusWithVertexNormals.bgeo" )
		group = torus.createOutputNode( "group" )
		group.parm( "crname" ).set( "torus2" )
		merge = cache.inputs()[0].createOutputNode( "merge" )
		merge.setInput( 1, group )
		cache.setInput( 0, merge )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		numTorusPoints = len(torus.geometry().points())
		for key in [ "P", "Cs" ] :
			self.assert_( key in result )
			self.assertEqual( result[key].data.size(), 2 * numTorusPoints )
		
		for i in range( 0, numTorusPoints ) :
			self.assertNotEqual( result['P'].data[i], result['P'].data[numTorusPoints+i] )
			self.assertEqual( result['Cs'].data[i], result['Cs'].data[numTorusPoints+i] )
		
		numTorusVerts = sum( [ len(x.vertices()) for x in torus.geometry().prims() ] )
		self.assert_( "N" in result )
		self.assertEqual( result["N"].data.size(), 2 * numTorusVerts )
		for i in range( 0, numTorusVerts ) :
			self.assertNotEqual( result['N'].data[i], result['N'].data[numTorusPoints+i] )
	
	def testPrimitiveGroupModeWithPrimAttribs( self ) :
		
		# Cd defaults to a Point attrib
		cache = self.cacheSop()
		self.failUnless( isinstance( cache.geometry().findPointAttrib( "Cd" ), hou.Attrib ) )
		self.failUnless( cache.geometry().findPrimAttrib( "Cd" ) is None )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( "Cs" in result.keys() )
		self.assertEqual( result["Cs"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( len(result["Cs"].data), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
		
		# Since the point and prim count match, Cd becomes a Primitive attrib if we use PrimitiveGroup mode
		group = hou.node( "/obj/geo1/group1" )
		group.parm( "entity" ).set( 0 )
		cache.parm( "groupingMode" ).set( 0 )
		self.failUnless( cache.geometry().findPointAttrib( "Cd" ) is None )
		self.failUnless( isinstance( cache.geometry().findPrimAttrib( "Cd" ), hou.Attrib ) )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( "Cs" in result.keys() )
		self.assertEqual( result["Cs"].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( len(result["Cs"].data), result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ) )
		
		# By creating Cd as a Point attrib before the cache, we can force it's type
		color = group.createOutputNode( "color" )
		cache.setInput( 0, color )
		self.failUnless( isinstance( cache.geometry().findPointAttrib( "Cd" ), hou.Attrib ) )
		self.failUnless( cache.geometry().findPrimAttrib( "Cd" ) is None )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( "Cs" in result.keys() )
		self.assertEqual( result["Cs"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( len(result["Cs"].data), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
	
	def testPrimitiveGroupModeWithVertexAttribs( self ) :
		
		# N defaults to a Point attrib
		hou.setFrame( 4 )
		cache = self.cacheSop()
		self.failUnless( isinstance( cache.geometry().findPointAttrib( "N" ), hou.Attrib ) )
		self.failUnless( cache.geometry().findVertexAttrib( "N" ) is None )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( "N" in result.keys() )
		self.assertEqual( result["N"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( len(result["N"].data), result.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ) )
		
		# Since N actually has more values than points should, N becomes a Vertex attrib if we use PrimitiveGroup mode
		group = hou.node( "/obj/geo1/group1" )
		group.parm( "entity" ).set( 0 )
		cache.parm( "groupingMode" ).set( 0 )
		self.failUnless( cache.geometry().findPointAttrib( "N" ) is None )
		self.failUnless( isinstance( cache.geometry().findVertexAttrib( "N" ), hou.Attrib ) )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( "N" in result.keys() )
		self.assertEqual( result["N"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( len(result["N"].data), result.variableSize( IECore.PrimitiveVariable.Interpolation.FaceVarying ) )
		
		# Even if we create N as a Point attrib before the cache, it remains a Vertex attrib since the sizes do not match
		facet = group.createOutputNode( "facet" )
		facet.parm( "postnml" ).set( True )
		cache.setInput( 0, facet )
		self.failUnless( isinstance( facet.geometry().findPointAttrib( "N" ), hou.Attrib ) )
		self.failUnless( facet.geometry().findVertexAttrib( "N" ) is None )
		self.failUnless( cache.geometry().findPointAttrib( "N" ) is None )
		self.failUnless( isinstance( cache.geometry().findVertexAttrib( "N" ), hou.Attrib ) )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( "N" in result.keys() )
		self.assertEqual( result["N"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( len(result["N"].data), result.variableSize( IECore.PrimitiveVariable.Interpolation.FaceVarying ) )

if __name__ == "__main__":
	unittest.main()
