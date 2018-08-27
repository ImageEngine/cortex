##########################################################################
#
#  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of Image Engine Design Inc nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
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
import imath

import IECore
import IECoreVDB
from VDBTestCase import VDBTestCase

class VDBObjectTest( VDBTestCase ) :

	def setUp( self ) :
		VDBTestCase.setUp( self )

	def testCanLoadVDBFromFile( self ) :
		sourcePath = os.path.join( self.dataDir, "sphere.vdb" )
		vdbObject = IECoreVDB.VDBObject( sourcePath )

		gridNames = vdbObject.gridNames()
		self.assertEqual(gridNames, ['ls_sphere'])

		metadata = vdbObject.metadata('ls_sphere')

		# skip the file size
		del metadata['file_mem_bytes']

		expected = IECore.CompoundObject(
			{
				'name': IECore.StringData( 'ls_sphere' ),
				'file_voxel_count': IECore.Int64Data( 270638 ),
				'file_bbox_min': IECore.V3iData( imath.V3i( -62, -62, -62 ) ),
				'file_bbox_max': IECore.V3iData( imath.V3i( 62, 62, 62 ) ),
				'is_local_space': IECore.BoolData( 0 ),
				'is_saved_as_half_float': IECore.BoolData( 1 ),
				'value_type': IECore.StringData( 'float' ),
				'class': IECore.StringData( 'level set' ),
				#'file_mem_bytes': IECore.Int64Data( 2643448 ),
				'vector_type': IECore.StringData( 'invariant' )
			}
		)

		self.assertEqual( metadata, expected )

	def testCanReadDoubleMetadata( self ) :
		sourcePath = os.path.join( self.dataDir, "sphere.vdb" )
		vdb = IECoreVDB.VDBObject( sourcePath )

		grid = vdb.findGrid( "ls_sphere" )
		grid.updateMetadata( { "test" : 0.0 } )
		vdb.insertGrid( grid )

		metadata = vdb.metadata( "ls_sphere" )

		# skip the file size
		del metadata['file_mem_bytes']

		expected = IECore.CompoundObject(
			{
				'name' : IECore.StringData( 'ls_sphere' ),
				'file_voxel_count' : IECore.Int64Data( 270638 ),
				'file_bbox_min' : IECore.V3iData( imath.V3i( -62, -62, -62 ) ),
				'file_bbox_max' : IECore.V3iData( imath.V3i( 62, 62, 62 ) ),
				'is_local_space' : IECore.BoolData( 0 ),
				'is_saved_as_half_float' : IECore.BoolData( 1 ),
				'value_type' : IECore.StringData( 'float' ),
				'class' : IECore.StringData( 'level set' ),
				#'file_mem_bytes': IECore.Int64Data( 2643448 ),
				'vector_type' : IECore.StringData( 'invariant' ),
				'test' : IECore.DoubleData( 0.0 )
			}
		)

		self.assertEqual( metadata, expected )


	def testCanEstimateMemoryUsage( self ):

		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		vdbObject = IECoreVDB.VDBObject( sourcePath )

		self.assertTrue(788000 <= vdbObject.memoryUsage() <= 788200)

		d = vdbObject.findGrid("density")

		def incValue( value ):
			return value + 1

		d.mapAll( incValue )

		self.assertTrue(7022000 <= vdbObject.memoryUsage() <= 7022300)

	def testCanRemoveGrid( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )

		vdbObject = IECoreVDB.VDBObject( sourcePath )
		self.assertEqual(vdbObject.gridNames(), ['density'])
		h1 = vdbObject.hash()
		vdbObject.removeGrid('density')
		self.assertEqual(vdbObject.gridNames(), [])

		h2 = vdbObject.hash()
		self.assertNotEqual(h1, h2)

		emptyVDBObject = IECoreVDB.VDBObject()
		h3 = emptyVDBObject.hash()
		self.assertEqual(h3, h2)

	def testHashIdenticalForFileLoadedTwice( self ):
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )

		vdbObject1 = IECoreVDB.VDBObject( sourcePath )
		vdbObject2 = IECoreVDB.VDBObject( sourcePath )

		self.assertEqual( vdbObject1.hash(), vdbObject2.hash() )

	def testCopiedVDBObjectHasIdenticalHash( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		vdbObject = IECoreVDB.VDBObject( sourcePath )
		vdbObjectCopy = vdbObject.copy()

		self.assertEqual( vdbObject.hash(), vdbObjectCopy.hash() )

	def testCanGetGridFromObject( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		vdbObject = IECoreVDB.VDBObject( sourcePath )

		grid = vdbObject.findGrid( "density" )
		self.assertEqual(grid.leafCount(), 3117)

	def testIfDifferentFromFile( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		vdbObject = IECoreVDB.VDBObject( sourcePath )

		vdbObjectCopy = vdbObject.copy()


		self.assertEqual( vdbObject.unmodifiedFromFile(), True )

		# getting a non const grid means the object could have changed from the file.
		densityGrid = vdbObject.findGrid( "density" )
		self.assertFalse( vdbObject.unmodifiedFromFile() )

		self.assertEqual( vdbObjectCopy.unmodifiedFromFile(), True )

		# remove the density grid and ensure we track the VDB object has changed from the file
		vdbObject2 = vdbObjectCopy.copy()
		self.assertTrue( vdbObject2.unmodifiedFromFile() )
		vdbObject2.removeGrid( "density" )
		self.assertFalse( vdbObject2.unmodifiedFromFile() )

	def testCanAddGridFromOneObjectToAnother( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		smoke = IECoreVDB.VDBObject( sourcePath )
		h = smoke.hash()
		self.assertTrue( smoke.unmodifiedFromFile() )

		sourcePath = os.path.join( self.dataDir, "sphere.vdb" )
		sphere = IECoreVDB.VDBObject( sourcePath )
		self.assertTrue( sphere.unmodifiedFromFile() )

		sphereLevelSet = sphere.findGrid( "ls_sphere" )
		smoke.insertGrid( sphereLevelSet )
		self.assertFalse( smoke.unmodifiedFromFile() )  # in practice this comes from the ls_sphere grid having the modified from file flag set.

		self.assertEqual( set( smoke.gridNames() ), set( ['ls_sphere', 'density'] ) )
		h1 = smoke.hash()

		self.assertNotEqual( h, h1 )

	# This demonstrates a problem in that you can modify a grid in python
	# and the hash on the VDBObject isn't updated correctly unless we
	# insert again into the VDBObject
	def testModifyingGridModifiesHash( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		smoke = IECoreVDB.VDBObject( sourcePath )

		d = smoke.findGrid( "density" )
		h = smoke.hash()

		def incValue( value ) :
			return value + 1

		d.mapAll( incValue )

		h1 = smoke.hash()
		self.assertEqual( h, h1 )

		smoke.insertGrid ( d )

		d2 = smoke.findGrid( "density" )

		h2 = smoke.hash()
		self.assertNotEqual(h2, h)

	def testFindGridMakesACopy( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		smoke = IECoreVDB.VDBObject( sourcePath )

		smoke2 = smoke.copy()
		d2 = smoke2.findGrid( "density" )

		def incValue( value ) :
			return value + 1

		d2.mapAll( incValue )

		d = smoke.findGrid( "density" )

		d2Value = list( d2.citerAllValues() )[0]
		dValue = list( d.citerAllValues() )[0]

		self.assertEqual( d2Value['value'], dValue['value'] + 1 )

		# we've requested mutable grids from both vdb objects so they could have been edited.
		self.assertFalse( smoke.unmodifiedFromFile() )
		self.assertFalse( smoke2.unmodifiedFromFile() )

	def testFilename( self ) :
		sourcePath = os.path.join( self.dataDir, "smoke.vdb" )
		smoke = IECoreVDB.VDBObject( sourcePath )

		# store the path to the file used to initialise the VDBObject (can be used when sending this VDB to the renderer)
		self.assertEqual( smoke.fileName(), sourcePath )

		# make sure an empty VDBObject returns a empty filename
		emptyVDB = IECoreVDB.VDBObject()
		self.assertEqual( emptyVDB.fileName(), "" )


if __name__ == "__main__":
	unittest.main()

