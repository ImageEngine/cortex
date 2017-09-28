##########################################################################
#
#  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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
import sys
import shutil
import unittest
import IECore


class TestRelativePreset( unittest.TestCase ) :

	def testDiffData( self ):

		emptyPreset = IECore.RelativePreset()
		a = emptyPreset.getDiffData()
		self.assertTrue( isinstance( a, IECore.CompoundObject ) )
		self.assertEqual( len(a), 0 )
		emptyPreset.setDiffData( IECore.CompoundObject() )
		self.assertRaises( Exception, lambda : emptyPreset.setDiffData( IECore.CompoundData() ) )

	def testSparseSimpleChanges( self ) :

		def createTestObj():
			testObj = IECore.Parameterised( "testParameterised1" )
			testObj.parameters().addParameters(
				[
					IECore.BoolParameter( "a", "", True ),
					IECore.FloatParameter( "b", "", 1.0 ),
				]
			)
			return testObj

		def createTestObj2():
			testObj2 = IECore.Parameterised( "testParameterised2" )
			testObj2.parameters().addParameters(
				[
					IECore.BoolParameter( "a", "", True ),
					IECore.FloatParameter( "c", "", 0.0	),
				]
			)
			return testObj2

		testObjA = createTestObj()
		testObjB = createTestObj()
		testObjB["a"] = False
		r = IECore.RelativePreset( testObjB.parameters(), testObjA.parameters() )

		testObj2 = createTestObj2()

		self.failUnless( r.applicableTo( testObjA, testObjA.parameters() ) )
		self.failUnless( r.applicableTo( testObj2, testObj2.parameters() ) )

		r( testObjB, testObjB.parameters() )

		self.assertEqual( testObjB.parameters()["a"].getTypedValue(), False )
		self.assertEqual( testObjB.parameters()["b"].getTypedValue(), 1.0 )

		r( testObj2, testObj2.parameters() )

		self.assertEqual( testObj2.parameters()["a"].getTypedValue(), False )
		self.assertEqual( testObj2.parameters()["c"].getTypedValue(), 0.0 )

		r = IECore.RelativePreset( testObjB.parameters() )
		testObjB["a"] = True
		testObjB["b"] = 2.0
		r( testObjB, testObjB.parameters() )
		self.assertEqual( testObjB.parameters()["a"].getTypedValue(), False )
		self.assertEqual( testObjB.parameters()["b"].getTypedValue(), 1.0 )


	def testClasses( self ) :

		def createTestObj():
			testObj = IECore.Parameterised( "testParameterised1" )
			testObj.parameters().addParameters(
				[
					IECore.BoolParameter( "a", "", True ),
					IECore.ClassParameter( "b", "", "IECORE_OP_PATHS" ),
				]
			)
			return testObj

		def createTestObj2():
			testObj2 = IECore.Parameterised( "testParameterised2" )
			testObj2.parameters().addParameters(
				[
					IECore.ClassParameter( "b", "",	"IECORE_OP_PATHS" ),
				]
			)
			return testObj2

		testObjA = createTestObj()
		testObjB = createTestObj()
		testObj2 = createTestObj2()

		testObjB["b"].setClass( "maths/multiply", 2 )

		classes1 = testObjB.parameters()["b"].getClass( True )
		classes2 = testObj2.parameters()["b"].getClass( True )
		self.assertNotEqual( classes1[1:], classes2[1:] )

		r = IECore.RelativePreset( testObjB.parameters(), testObjA.parameters() )

		self.failUnless( r.applicableTo( testObjB, testObjB.parameters() ) )
		self.failUnless( r.applicableTo( testObj2, testObj2.parameters() ) )

		r( testObj2, testObj2.parameters() )

		classes1 = testObjB.parameters()["b"].getClass( True )
		classes2 = testObj2.parameters()["b"].getClass( True )

		self.assertEqual( classes1[1:], classes2[1:] )

		r = IECore.RelativePreset( testObjB.parameters() )
		testObjB["a"] = False
		testObjB["b"].setClass("", 0)
		r( testObjB, testObjB.parameters() )
		classes1 = testObjB.parameters()["b"].getClass( True )
		self.assertEqual( classes1[1:], classes2[1:] )
		self.assertEqual( testObjB["a"].getTypedValue(), True )

		testObjA["b"].setClass( "maths/multiply", 1 )
		classes1 = testObjB.parameters()["b"].getClass( True )
		classes2 = testObjA.parameters()["b"].getClass( True )
		self.assertNotEqual( classes1[1:], classes2[1:] )
		r = IECore.RelativePreset( testObjB.parameters(), testObjA.parameters() )
		r( testObj2, testObj2.parameters() )
		classes1 = testObjB.parameters()["b"].getClass( True )
		classes2 = testObj2.parameters()["b"].getClass( True )
		self.assertEqual( classes1[1:], classes2[1:] )

	def testCompareFilter( self ) :

		def createTestObj():
			testObj = IECore.Parameterised( "testParameterised1" )
			testObj.parameters().addParameters(
				[
					IECore.ClassParameter( "a", "", "IECORE_OP_PATHS" ),
					IECore.ClassParameter( "b", "", "IECORE_OP_PATHS" ),
				]
			)
			return testObj

		def filterCmp( p1, p2 ):
			# only keep track of changes on parameter 'a'
			return bool(p1.name in ['a', ''])

		testObjA = createTestObj()
		testObjB = createTestObj()

		testObjA["a"].setClass( "maths/multiply", 2 )
		testObjA["b"].setClass( "maths/multiply", 2 )
		testObjB["a"].setClass( "maths/multiply", 2 )
		testObjB["b"].setClass( "maths/multiply", 2 )
		testObjB["a"]["a"].setTypedValue( 10 )
		testObjB["a"]["b"].setTypedValue( 20 )
		testObjB["b"]["a"].setTypedValue( 30 )
		testObjB["b"]["b"].setTypedValue( 40 )

		r = IECore.RelativePreset( testObjB.parameters(), testObjA.parameters(), compareFilter = filterCmp )

		r( testObjA, testObjA.parameters() )

		self.assertEqual( testObjA["a"]["a"].getTypedValue(), 10 )
		self.assertEqual( testObjA["a"]["b"].getTypedValue(), 2 )
		self.assertEqual( testObjA["b"]["a"].getTypedValue(), 1 )
		self.assertEqual( testObjA["b"]["b"].getTypedValue(), 2 )

	def testClassVectors( self ) :

		def createTestObj() :
			testObj = IECore.Parameterised( "testParameterised1" )
			testObj.parameters().addParameters(
				[
					IECore.BoolParameter( "a", "", True ),
					IECore.ClassVectorParameter( "b", "", "IECORE_OP_PATHS" ),
				]
			)
			return testObj

		testObjA = createTestObj()
		testObjB = createTestObj()

		testObjB.parameters()["b"].setClasses(
			[
				( "mult", "maths/multiply", 2 ),
				( "coIO", "compoundObjectInOut", 1 ),
			]
		)

		def createTestObj2():
			testObj2 = IECore.Parameterised( "testParameterised2" )
			testObj2.parameters().addParameters(
				[
					IECore.ClassVectorParameter( "b", "", "IECORE_OP_PATHS" ),
				]
			)
			return testObj2

		testObj2 = createTestObj2()

		classes1 = [ c[1:] for c in testObjB.parameters()["b"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["b"].getClasses( True ) ]

		self.assertNotEqual( classes1, classes2 )
		self.assertEqual( len(classes1), 2 )

		r1 = IECore.RelativePreset( testObjB.parameters() )
		r2 = IECore.RelativePreset( testObjB.parameters(), testObjA.parameters() )

		self.failUnless( r1.applicableTo( testObjB, testObjB.parameters() ) )
		self.failUnless( r1.applicableTo( testObj2, testObj2.parameters() ) )
		self.failUnless( r2.applicableTo( testObj2, testObj2.parameters() ) )

		r2( testObj2, testObj2.parameters() )

		classes2 = [ c[1:] for c in testObj2.parameters()["b"].getClasses( True ) ]
		self.assertEqual( classes1, classes2 )

		# now we reorder ops...
		testObjC = createTestObj()
		classes1.reverse()
		testObjC["b"].setClasses( classes1 )
		r = IECore.RelativePreset( testObjC.parameters(), testObjB.parameters() )

		r( testObj2, testObj2.parameters() )
		classes2 = [ c[1:] for c in testObj2.parameters()["b"].getClasses( True ) ]
		self.assertEqual( classes1, classes2 )

		# now we remove an op
		testObj2["b"].removeClass( "mult" )
		classes2 = [ c[1] for c in testObj2.parameters()["b"].getClasses( True ) ]
		self.assertEqual( classes2, [ "coIO" ] )
		r = IECore.RelativePreset( testObj2.parameters(), testObjB.parameters() )
		r( testObjB, testObjB.parameters() )
		classes1 = [ c[1] for c in testObjB.parameters()["b"].getClasses( True ) ]
		self.assertEqual( classes1, [ "coIO" ] )

		# now we add it back
		testObj2["b"].setClass( "mult", "maths/multiply", 2 )
		testObj2["b"].setClass( "another", "maths/multiply", 2 )
		r = IECore.RelativePreset( testObj2.parameters(), testObjB.parameters() )
		r( testObjB, testObjB.parameters() )
		classes1 = [ c[1:] for c in testObjB.parameters()["b"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["b"].getClasses( True ) ]
		self.assertEqual( len(classes2), 3 )
		self.assertEqual( classes1, classes2 )

		# now we replace one of them
		testObj2["b"].setClass( "another", "compoundObjectInOut", 1 )
		r = IECore.RelativePreset( testObj2.parameters(), testObjB.parameters() )
		r( testObjB, testObjB.parameters() )
		classes1 = [ c[1:] for c in testObjB.parameters()["b"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["b"].getClasses( True ) ]
		self.assertEqual( len(classes2), 3 )
		self.assertEqual( classes1, classes2 )

	def testDeepClassVectors( self ) :

		def createParameterised():
			testObj = IECore.Parameterised( "testParameterised1" )
			testObj.parameters().addParameters(
				[
					IECore.BoolParameter( "a", "", True ),
					IECore.ClassVectorParameter( "b", "", "IECORE_OP_PATHS" ),
					IECore.BoolParameter( "c", "", False ),
				]
			)
			testObj.parameters()["b"].setClasses(
				[
					( "p0", "maths/multiply", 2 ),
					( "p1", "compoundObjectInOut", 1 ),
					( "p2", "classVectorParameterTest", 1),
					( "p3", "maths/multiply", 1 ),
					( "p4", "classParameterTest", 1),
				]
			)
			testObj["b"]["p2"]["cv"].setClasses(
				[
					( "p0", "maths/multiply", 2 ),
					( "p1", "compoundObjectInOut", 1 )
				]
			)
			testObj["b"]["p4"]["cp"].setClass( "maths/multiply", 2 )
			return testObj

		testObjA = createParameterised()

		testObjB = createParameterised()

		# test no differences
		r = IECore.RelativePreset( testObjB.parameters(), testObjA.parameters() )
		# the diff should be empty!
		self.assertEqual( r._RelativePreset__data, IECore.CompoundObject() )
		# applying empty diff should not break
		r( testObjB, testObjB.parameters() )
		self.assertEqual( testObjA.parameters().getValue(), testObjB.parameters().getValue() )

		# now we modify the object a lot!!
		testObjB["a"] = False
		testObjB["c"] = True
		testObjB["b"].removeClass( "p1" )
		testObjB["b"].removeClass( "p3" )
		testObjB["b"]["p0"]["a"] = False
		testObjB["b"]["p4"]["cp"]["a"] = 10
		testObjB["b"]["p2"]["cv"]["p0"]["a"] = 20
		testObjB["b"]["p2"]["cv"].setClass("p1", "maths/multiply", 2 )	# replace one op
		testObjB["b"]["p2"]["cv"].setClass("p3", "floatParameter", 1 )	# add one op
		# reoder "b" elements
		order = map( lambda c: c[1:], testObjB["b"].getClasses(True) )
		order.reverse()
		testObjB["b"].setClasses(order)

		# now we apply the changes on an object identical to the original one represented in the basic preset.
		testObj2 = createParameterised()
		r = IECore.RelativePreset( testObjB.parameters(), testObjA.parameters() )
		relPreset = r	# copy this preset, as it's going to be used later on again...
		r( testObj2, testObj2.parameters() )
		self.assertEqual( testObj2.parameters().getValue(), testObjB.parameters().getValue() )
		classes1 = [ c[1:] for c in testObjB.parameters()["b"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["b"].getClasses( True ) ]
		self.assertEqual( len(classes2), 3 )
		self.assertEqual( classes1, classes2 )
		classes1 = [ c[1:] for c in testObjB.parameters()["b"]["p2"]["cv"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["b"]["p2"]["cv"].getClasses( True ) ]
		self.assertEqual( testObjB.parameters()["b"]["p2"]["cv"].keys(), [ 'p0', 'p1', 'p3' ] )
		self.assertEqual( len(classes2), 3 )
		self.assertEqual( classes1, classes2 )

		# we test the other way around
		testObj2 = createParameterised()
		r = IECore.RelativePreset( testObj2.parameters(), testObjB.parameters() )
		r( testObjB, testObjB.parameters() )
		self.assertEqual( testObj2.parameters().getValue(), testObjB.parameters().getValue() )
		classes1 = [ c[1:] for c in testObjB.parameters()["b"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["b"].getClasses( True ) ]
		self.assertEqual( len(classes2), 5 )
		self.assertEqual( classes1, classes2 )
		classes1 = [ c[1:] for c in testObjB.parameters()["b"]["p2"]["cv"].getClasses( True ) ]
		classes2 = [ c[1:] for c in testObj2.parameters()["b"]["p2"]["cv"].getClasses( True ) ]
		self.assertEqual( len(classes2), 2 )
		self.assertEqual( classes1, classes2 )

		# now it gets tricky... we try to apply the changes on a different object...
		testObj2 = createParameterised()
		testObj2.parameters().removeParameter("a")		# remove "a"
		testObj2.parameters().removeParameter("c")		# replace "c" by something else
		testObj2.parameters().addParameter(
			IECore.IntParameter( "c", "", 10 )
		)
		testObj2.parameters().addParameter(				# add new parameter "d"
			IECore.IntParameter( "d", "", 10 )
		)
		testObj2["b"].removeClass("p1")				# we remove "p1" - the same parameter that the preset will try to remove later... it should ignore it
		testObj2["b"].setClass("p3", "compoundObjectInOut", 1 )	# we replace "p3" - the same parameter that the preset will try to remove later... it should not remove because it's a different class.
		testObj2["b"]["p4"]["cp"]["a"] = 30			# we want this value to be replaced by 10 which was the relative change on that parameter
		testObj2["b"]["p2"]["cv"].setClass("p1", "maths/multiply", 1 )	# replace an op that the preset will try to replace too...  but will not because the original class is different
		testObj2["b"]["p2"]["cv"].setClass("p3", "compoundObjectInOut", 1 )	# add one op that the preset will try to add too... name clashes!

		relPreset( testObj2, testObj2.parameters() )

		self.assertEqual( testObj2["c"].getTypedValue(), 10 )	# guarantee that the replaced parameter "c" was not affected by the Preset
		self.assert_( "p3" in testObj2["b"].keys() )			# ok, "p3" was not removed!
		self.assertEqual( testObj2["b"]["p4"]["cp"]["a"].getTypedValue(), 10 )	# ok, the preset changes were applied here
		self.assertEqual( testObj2["b"]["p2"]["cv"].getClass( "p1", True )[2], 1 )	# ok, the Preset did not change this one because it was looking for a compoundObjectInOut class name...
		self.assertEqual( testObj2["b"]["p2"]["cv"].getClass( "p3", True )[1], "compoundObjectInOut" )	# ok, the Preset did not remove the current "p3" parameter...
		self.assertEqual( testObj2["b"]["p2"]["cv"].getClass( "p2", True )[1], "maths/multiply" )	# ok, the Preset added a new parameter "p2" with the "p1" Preset...
		self.assertEqual( testObj2["b"]["p2"]["cv"].getClass( "p4", True )[1], "floatParameter" )	# ok, the Preset added a new parameter "p4" with the "p3" Preset...
		self.assertEqual( testObj2["b"]["p2"]["cv"].keys(), [ 'p0', 'p2', 'p4', 'p1', 'p3' ] )

	def __checkOrder( self, localChangesParam, applyToParam, expectedOrder ):
		r = IECore.RelativePreset( localChangesParam, baseVec() )
		testObj = IECore.Parameterised( "test" )
		testObj.parameters().addParameters(
			[
						applyToParam
			]
		)
		r( testObj, applyToParam )
		resultOrder = map( lambda c: c[1], applyToParam.getClasses( True ) )
		self.assertEqual( resultOrder, expectedOrder )

	def testBaseInsertions( self ):
		# easy tests applying to base vector
		self.__checkOrder( topInsert(), baseVec(), [ 'p5', 'p1', 'p2', 'p3', 'p4' ] )
		self.__checkOrder( doubleTopInsert(), baseVec(), [ 'p5', 'p6', 'p1', 'p2', 'p3', 'p4' ] )
		self.__checkOrder( bottomInsert(), baseVec(), [ 'p1', 'p2', 'p3', 'p4', 'p5' ] )
		self.__checkOrder( centerInsert(), baseVec(), [ 'p1', 'p2', 'p5', 'p3', 'p4' ] )
		self.__checkOrder( doubleCenterInsert(), baseVec(), [ 'p1', 'p2', 'p5', 'p6', 'p3', 'p4' ] )
		self.__checkOrder( twoInserts(), baseVec(), [ 'p1', 'p2', 'p5', 'p3', 'p6', 'p4' ] )

	def testEmptyVectorInsertions(self):
		# tests applying to empty classes vector
		self.__checkOrder( topInsert(), emptyVec(), [ 'p5' ] )
		self.__checkOrder( doubleTopInsert(), emptyVec(), [ 'p5', 'p6' ] )
		self.__checkOrder( bottomInsert(), emptyVec(), [ 'p5' ] )
		self.__checkOrder( centerInsert(), emptyVec(), [ 'p5' ] )
		self.__checkOrder( doubleCenterInsert(), emptyVec(), [ 'p5', 'p6' ] )
		self.__checkOrder( twoInserts(), emptyVec(), [ 'p5', 'p6' ] )

	def testRemovedP2Insertions( self ):
		# tests applying to "removed P2" vector
		self.__checkOrder( topInsert(), removedP2Vec(), [ 'p5', 'p1', 'p3', 'p4' ] )
		self.__checkOrder( doubleTopInsert(), removedP2Vec(), [ 'p5', 'p6', 'p1', 'p3', 'p4' ] )
		self.__checkOrder( bottomInsert(), removedP2Vec(), [ 'p1', 'p3', 'p4', 'p5' ] )
		self.__checkOrder( centerInsert(), removedP2Vec(), [ 'p1', 'p5', 'p3', 'p4' ] )
		self.__checkOrder( doubleCenterInsert(), removedP2Vec(), [ 'p1', 'p5', 'p6', 'p3', 'p4' ] )
		self.__checkOrder( twoInserts(), removedP2Vec(), [ 'p1', 'p5', 'p3', 'p6', 'p4' ] )

	def testRemovedP3Insertions( self ):
		# tests applying to "removed P3" vector
		self.__checkOrder( topInsert(), removedP3Vec(), [ 'p5', 'p1', 'p2', 'p4' ] )
		self.__checkOrder( doubleTopInsert(), removedP3Vec(), [ 'p5', 'p6', 'p1', 'p2', 'p4' ] )
		self.__checkOrder( bottomInsert(), removedP3Vec(), [ 'p1', 'p2', 'p4', 'p5' ] )
		self.__checkOrder( centerInsert(), removedP3Vec(), [ 'p1', 'p2', 'p5', 'p4' ] )
		self.__checkOrder( doubleCenterInsert(), removedP3Vec(), [ 'p1', 'p2', 'p5', 'p6', 'p4' ] )
		self.__checkOrder( twoInserts(), removedP3Vec(), [ 'p1', 'p2', 'p5', 'p6', 'p4' ] )

	def testReversedBaseInsertions(self):
		# tests applying to reversed base vector
		self.__checkOrder( topInsert(), reversedBaseVec(), [ 'p5', 'p4', 'p3', 'p2', 'p1' ] )
		self.__checkOrder( doubleTopInsert(), reversedBaseVec(), [ 'p5', 'p6', 'p4', 'p3', 'p2', 'p1' ] )
		self.__checkOrder( bottomInsert(), reversedBaseVec(), [ 'p4', 'p5', 'p3', 'p2', 'p1' ] )
		self.__checkOrder( centerInsert(), reversedBaseVec(), [ 'p4', 'p3', 'p2', 'p5', 'p1' ] )
		self.__checkOrder( doubleCenterInsert(), reversedBaseVec(), [ 'p4', 'p3', 'p2', 'p5', 'p6', 'p1' ] )
		self.__checkOrder( twoInserts(), reversedBaseVec(), [ 'p4', 'p3', 'p6', 'p2', 'p5', 'p1' ] )

	def testDifferentInsertions(self):
		# tests applying to different classes but same names vector
		self.__checkOrder( topInsert(), differentClassesVec(), [ 'p5', 'p2', 'p1', 'p3' ] )
		self.__checkOrder( doubleTopInsert(), differentClassesVec(), [ 'p5', 'p6', 'p2', 'p1', 'p3' ] )
		self.__checkOrder( bottomInsert(), differentClassesVec(), [ 'p5', 'p2', 'p1', 'p3' ] )
		self.__checkOrder( centerInsert(), differentClassesVec(), [ 'p5', 'p2', 'p1', 'p3' ] )
		self.__checkOrder( doubleCenterInsert(), differentClassesVec(), [ 'p5', 'p6', 'p2', 'p1', 'p3' ] )
		self.__checkOrder( twoInserts(), differentClassesVec(), [ 'p5', 'p6', 'p2', 'p1', 'p3' ] )

	def testNameClashingInsertions(self):
		# tests applying to unrelated but with name clashing vector
		self.__checkOrder( topInsert(), nameclashingVec(), [ 'p0', 'p5', 'p6', 'p7' ] )
		self.__checkOrder( doubleTopInsert(), nameclashingVec(), [ 'p0', 'p1', 'p5', 'p6', 'p7' ] )
		self.__checkOrder( bottomInsert(), nameclashingVec(), [ 'p0', 'p5', 'p6', 'p7' ] )
		self.__checkOrder( centerInsert(), nameclashingVec(), [ 'p0', 'p5', 'p6', 'p7' ] )
		self.__checkOrder( doubleCenterInsert(), nameclashingVec(), [ 'p0', 'p1', 'p5', 'p6', 'p7' ] )
		self.__checkOrder( twoInserts(), nameclashingVec(), [ 'p0', 'p1', 'p5', 'p6', 'p7' ] )

	def testUnrelatedInsertions(self):
		# tests applying to unrelated vector
		self.__checkOrder( topInsert(), unrelatedVec(), [ 'p5', 'p10', 'p11', 'p12' ] )
		self.__checkOrder( doubleTopInsert(), unrelatedVec(), [ 'p5', 'p6', 'p10', 'p11', 'p12' ] )
		self.__checkOrder( bottomInsert(), unrelatedVec(), [ 'p5', 'p10', 'p11', 'p12' ] )
		self.__checkOrder( centerInsert(), unrelatedVec(), [ 'p5', 'p10', 'p11', 'p12' ] )
		self.__checkOrder( doubleCenterInsert(), unrelatedVec(), [ 'p5', 'p6', 'p10', 'p11', 'p12' ] )
		self.__checkOrder( twoInserts(), unrelatedVec(), [ 'p5', 'p6', 'p10', 'p11', 'p12' ] )

	def testLocalizedReordering(self):
		# simple localized reorder in place
		self.__checkOrder( switchP2andP4(), baseVec(), [ 'p1', 'p4', 'p3', 'p2' ] )

		# test reordering with unknown items in between.
		extendedBaseVec = createVec( [ ('p0','floatParameter'), ('p1','floatParameter'), ('p5','floatParameter'), ('p2','maths/multiply'), ('p6','floatParameter'), ('p3', 'compoundObjectInOut'), ('p7','floatParameter'), ('p4', 'floatParameter'), ('p8','floatParameter') ] )
		self.__checkOrder( switchP2andP4(), extendedBaseVec, [ 'p0', 'p1', 'p5', 'p4', 'p3', 'p7', 'p2', 'p6', 'p8' ] )

		# now we prove that changes in the order of originally unmodified parameters are not affected when we apply the preset
		invExtendedBaseVec = createVec( [ ('p8','floatParameter'), ('p2','maths/multiply'), ('p6','floatParameter'), ('p3', 'compoundObjectInOut'), ('p7','floatParameter'), ('p4', 'floatParameter'), ('p0','floatParameter'), ('p5','floatParameter'), ('p1','floatParameter') ] )
		self.__checkOrder( switchP2andP4(), invExtendedBaseVec, [ 'p8', 'p4', 'p3', 'p7', 'p2', 'p6', 'p0', 'p5', 'p1' ] )


def createVec( classes ) :
	testObj = IECore.ClassVectorParameter( "vec", "", "IECORE_OP_PATHS" )
	for (pName,cName) in classes :
		testObj.setClass( pName, cName, 1 )
	return testObj

# creating some shared data for the following tests....
def baseVec():
	return createVec( [ ('p1','floatParameter'), ('p2','maths/multiply'), ('p3', 'compoundObjectInOut'), ('p4', 'floatParameter') ] )

def switchP2andP4():
	return createVec( [ ('p1','floatParameter'), ('p4', 'floatParameter'), ('p3', 'compoundObjectInOut'), ('p2','maths/multiply') ] )

def topInsert():
	return createVec( [ ('p5','maths/multiply'), ('p1','floatParameter'), ('p2','maths/multiply'), ('p3', 'compoundObjectInOut'), ('p4', 'floatParameter') ] )

def doubleTopInsert():
	return createVec( [ ('p5','maths/multiply'), ('p6','maths/multiply'), ('p1','floatParameter'), ('p2','maths/multiply'), ('p3', 'compoundObjectInOut'), ('p4', 'floatParameter') ] )

def bottomInsert():
	return createVec( [ ('p1','floatParameter'), ('p2','maths/multiply'), ('p3', 'compoundObjectInOut'), ('p4', 'floatParameter'), ('p5','maths/multiply') ] )

def centerInsert():
	return createVec( [ ('p1','floatParameter'), ('p2','maths/multiply'), ('p5','maths/multiply'), ('p3', 'compoundObjectInOut'), ('p4', 'floatParameter') ] )

def doubleCenterInsert():
	return createVec( [ ('p1','floatParameter'), ('p2','maths/multiply'), ('p5','maths/multiply'), ('p6','maths/multiply'), ('p3', 'compoundObjectInOut'), ('p4', 'floatParameter') ] )

def twoInserts():
	return createVec( [ ('p1','floatParameter'), ('p2','maths/multiply'), ('p5','maths/multiply'), ('p3', 'compoundObjectInOut'), ('p6','maths/multiply'), ('p4', 'floatParameter') ] )

def removedP2Vec():
	return createVec( [ ('p1','floatParameter'), ('p3', 'compoundObjectInOut'), ('p4', 'floatParameter') ] )

def removedP3Vec():
	return createVec( [ ('p1','floatParameter'), ('p2','maths/multiply'), ('p4', 'floatParameter') ] )

def reversedBaseVec():
	return createVec( [ ('p4', 'floatParameter'), ('p3', 'compoundObjectInOut'), ('p2','maths/multiply'), ('p1','floatParameter') ] )

def emptyVec():
	return createVec( [] )

def differentClassesVec():
	return createVec( [ ('p2','floatParameter'), ('p1','maths/multiply'), ('p3', 'floatParameter') ] )

def nameclashingVec():
	return createVec( [ ('p5','maths/multiply'), ('p6','floatParameter'), ('p7', 'floatParameter')  ] )

def unrelatedVec():
	return createVec( [ ('p10','maths/multiply'), ('p11','floatParameter'), ('p12', 'floatParameter')  ] )


if __name__ == "__main__":
	unittest.main()

