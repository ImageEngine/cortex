##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
	
	__torusTestFile = "contrib/IECoreHoudini/test/test_data/torus.cob"
	
	def cacheSop( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		torus = geo.createNode( "file" )
		torus.parm( "file" ).set( TestInterpolatedCacheReader.__torusTestFile )
		group = torus.createOutputNode( "group" )
		group.parm( "crname" ).set( "torus" )
		group.parm( "entity" ).set( 1 )
		cache = group.createOutputNode( "ieInterpolatedCacheReader" )
		cache.parm( "cacheSequence" ).set( "contrib/IECoreHoudini/test/test_data/torusVertCache.####.fio" )
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
		self.assertNotEqual( orig['Cd'], result['Cd'] )
		self.assertEqual( orig['pointId'], result['pointId'] )
	
	def testFrameMultiplier( self ) :
		cache = self.cacheSop()
		self.failUnless( isinstance( cache.geometry(), hou.Geometry ) )
		cache.parm( "frameMultiplier" ).set( 250 )
		self.assertRaises( hou.OperationFailed, cache.cook )
		self.failUnless( cache.errors() )
		shutil.copyfile( "contrib/IECoreHoudini/test/test_data/torusVertCache.0001.fio", "contrib/IECoreHoudini/test/test_data/torusVertCache.0250.fio" )
		self.failUnless( isinstance( cache.geometry(), hou.Geometry ) )
		os.remove( "contrib/IECoreHoudini/test/test_data/torusVertCache.0250.fio" )
	
	def testNonExistantFile( self ) :
		cache = self.cacheSop()
		cache.parm( "cacheSequence" ).set( "contrib/IECoreHoudini/test/test_data/fake.####.fio" )
		self.assertRaises( hou.OperationFailed, cache.cook )
		self.failUnless( cache.errors() )
		
	def testNoFileForFrame( self ) :
		cache = self.cacheSop()
		cache.parm( "cacheSequence" ).set( "contrib/IECoreHoudini/test/test_data/torusVertCache.####.fio" )
		hou.setFrame( 4 )
		self.assertRaises( hou.OperationFailed, cache.cook )
		self.failUnless( cache.errors() )
	
	def testBadObjectHandle( self ) :
		cache = self.cacheSop()
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" in orig )
		cache.parm( "objectFixes1" ).set( "nope" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" not in result )
		
	def testNoGroups( self ) :
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.destroy()
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" not in result )
		
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
		for key in [ "P", "Cd" ] :
			self.assert_( key in result )
			self.assertEqual( result[key].data.size(), numTorusPoints )
		
	def testObjectPrefixAndSuffix( self ) :
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "rus" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" not in result )
		cache.parm( "objectFixes1" ).set( "to" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" in result )
		
		cache.parm( "objectFixes1" ).set( "" )
		group.parm( "crname" ).set( "to" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" not in result )
		cache.parm( "objectFixes2" ).set( "rus" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" in result )
		
		cache.parm( "objectFixes1" ).set( "" )
		cache.parm( "objectFixes2" ).set( "" )
		group.parm( "crname" ).set( "oru" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" not in result )
		cache.parm( "objectFixes1" ).set( "t" )
		cache.parm( "objectFixes2" ).set( "s" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" in result )
	
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
		for key in [ "P", "Cd" ] :
			self.assert_( key in result )
			self.assertEqual( result[key].data.size(), 2 * numTorusPoints )
		
		for i in range( 0, numTorusPoints ) :
			self.assertNotEqual( result['P'].data[i], result['P'].data[numTorusPoints+i] )
			self.assertEqual( result['Cd'].data[i], result['Cd'].data[numTorusPoints+i] )
	
	def testAttributeMismatchBetweenFrames( self ) :
		cache = self.cacheSop()
		hou.setFrame( 1.5 )
		self.failUnless( isinstance( cache.geometry(), hou.Geometry ) )
		orig = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		hou.setFrame( 2.5 )
		self.assertEqual( cache.geometry(), None )
		self.failUnless( "FileIndexedIO: Entry not found" in cache.errors() )
		hou.setFrame( 3 )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.failUnless( isinstance( cache.geometry(), hou.Geometry ) )
		self.assertNotEqual( orig, result )
	
	def testAttrPrefixAndSuffix( self ) :
		cache = self.cacheSop()
		cache.parm( "attributeFixes1" ).set( "C" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" not in result )
		self.assert_( "pointId" in result )
		self.assert_( "d" in result )
		
		cache.parm( "attributeFixes1" ).set( "" )
		cache.parm( "attributeFixes2" ).set( "tId" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" in result )
		self.assert_( "pointId" not in result )
		self.assert_( "poin" in result )
		
		cache.parm( "attributeFixes1" ).set( "po" )
		cache.parm( "attributeFixes2" ).set( "tId" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" in result )
		self.assert_( "pointId" not in result )
		self.assert_( "in" in result )
		
		cache.parm( "attributeFixes1" ).set( "C" )
		cache.parm( "attributeFixes2" ).set( "tId" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" not in result )
		self.assert_( "pointId" not in result )
		self.assert_( "poin" in result )
		self.assert_( "d" in result )
		
		cache.parm( "attributeFixes1" ).set( "oin" )
		cache.parm( "attributeFixes2" ).set( "tI" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "Cd" in result )
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
		i = IECore.InterpolatedCache( cache.parm( "cacheSequence" ).eval(), 3, IECore.InterpolatedCache.Interpolation.Linear, IECore.OversamplesCalculator( 24, 1, 24 ) )
		self.assert_( "notData" in i.attributes( 'badObject' ) )
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
		i = IECore.InterpolatedCache( cache.parm( "cacheSequence" ).eval(), 3, IECore.InterpolatedCache.Interpolation.Linear, IECore.OversamplesCalculator( 24, 1, 24 ) )
		self.assert_( "splineColor4fData" in i.attributes( 'badObject' ) )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "splineColor4fData" not in result )
	
	def testShortP( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "badObject" )
		cache.parm( "attributeFixes1" ).set( "short" )
		self.assertRaises( hou.OperationFailed, cache.cook )
		self.failUnless( "Geometry/Cache mismatch" in cache.errors() )
	
	def testLongP( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "badObject" )
		cache.parm( "attributeFixes1" ).set( "long" )
		self.assertRaises( hou.OperationFailed, cache.cook )
		self.failUnless( "Geometry/Cache mismatch" in cache.errors() )

	def testWrongP( self ) :
		hou.setFrame( 3 )
		cache = self.cacheSop()
		group = cache.inputs()[0]
		group.parm( "crname" ).set( "badObject" )
		cache.parm( "attributeFixes1" ).set( "wrong" )
		result = IECoreHoudini.FromHoudiniPolygonsConverter( cache ).convert()
		self.assert_( "P" in result )

if __name__ == "__main__":
	unittest.main()
