##########################################################################
#
#  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import unittest
import os

import nuke
import imath

import IECore
import IECoreScene
import IECoreNuke

class ParameterisedHolderTest( IECoreNuke.TestCase ) :

	def __checkParameterKnobs( self, parameter, node, knobName=None, parameterPath=None, ignore=set() ) :

		if knobName is None :
			knobName = "parm"

		if parameterPath in ignore :
			return

		if isinstance( parameter, IECore.CompoundParameter ) :
			for k in parameter.keys() :
				childKnobName = knobName + "_" + parameter[k].name
				if not parameterPath :
					childParameterPath = k
				else :
					childParameterPath = parameterPath + "." + k
				self.__checkParameterKnobs( parameter[k], node, childKnobName, childParameterPath, ignore )
		else :

			knob = node.knob( knobName )
			self.failUnless( knob is not None )

			if isinstance( knob, nuke.Enumeration_Knob ) :
				self.assertEqual( knob.value(), parameter.getCurrentPresetName() )
			else :

				knobValue = None
				try :
					knobValue = IECoreNuke.getKnobValue( knob )
					if isinstance( parameter, IECore.V2dParameter ) :
						# getKnobValue defaults to V2f
						knobValue = imath.V2d( knobValue[0], knobValue[1] )
					elif isinstance( parameter, IECore.V3dParameter ) :
						knobValue = imath.V3d( knobValue[0], knobValue[1], knobValue[2] )
				except :
					# not all knob types have accessors yet. some of the numeric
					# knobs don't have them because nuke has bugs and returns those
					# knobs as the wrong type. try to get the value another way.
					knobValue = knob.getValue()

				self.assertEqual( parameter.getValue().value, knobValue )

	def testModifyParametersAndTransferToKnobs( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "mult", "maths/multiply", 2 )

		self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 1 )
		self.assertEqual( fnOH.node().knob( "parm_b" ).getValue(), 2 )

		with fnOH.parameterModificationContext() as parameters :

			parameters["a"].setNumericValue( 10 )
			parameters["b"].setNumericValue( 20 )

		self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 10 )
		self.assertEqual( fnOH.node().knob( "parm_b" ).getValue(), 20 )

	def testModifyParametersAndUndo( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "mult", "maths/multiply", 2 )

		self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 1 )
		self.assertEqual( fnOH.node().knob( "parm_b" ).getValue(), 2 )

		with IECoreNuke.UndoEnabled() :

			with fnOH.parameterModificationContext() as parameters :

				parameters["a"].setNumericValue( 10 )
				parameters["b"].setNumericValue( 20 )

			self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 10 )
			self.assertEqual( fnOH.node().knob( "parm_b" ).getValue(), 20 )

		nuke.undo()

		self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 1 )
		self.assertEqual( fnOH.node().knob( "parm_b" ).getValue(), 2 )

	def testClassParameterSetClass( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "classParameterTest", 1 )

		with fnOH.parameterModificationContext() as parameterised :

			parameterised["cp"].setClass( "maths/multiply", 2 )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node() )

		self.assertEqual( parameterised.parameters().getValue(), fnOH.getParameterised()[0].parameters().getValue() )

	def testClassParameterSetClassAndValues( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "classParameterTest", 1 )

		with fnOH.parameterModificationContext() as parameterised :

			parameterised["cp"].setClass( "maths/multiply", 2 )
			parameterised["cp"]["a"].setNumericValue( 10 )
			parameterised["cp"]["a"].setNumericValue( 20 )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node() )

		self.assertEqual( parameterised.parameters().getValue(), fnOH.getParameterised()[0].parameters().getValue() )

	def testClassParameterSetClassAndValues( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "classParameterTest", 1 )

		with fnOH.parameterModificationContext() as parameterised :

			parameterised["cp"].setClass( "maths/multiply", 2 )
			parameterised["cp"]["a"].setNumericValue( 10 )
			parameterised["cp"]["a"].setNumericValue( 20 )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node() )

		nuke.nodeCopy( "test/IECoreNuke/parameterisedHolder.nk" )

		nuke.scriptClear()

		n = nuke.nodePaste( "test/IECoreNuke/parameterisedHolder.nk" )

		fnOH = IECoreNuke.FnOpHolder( n )

		parameterised2 = fnOH.getParameterised()[0]

		self.assertEqual( parameterised.parameters().getValue(), parameterised2.parameters().getValue() )

	def testNestedClassParameterSetClass( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "classParameterTest", 1 )

		with fnOH.parameterModificationContext() as parameterised :

			parameterised["cp"].setClass( "classParameterTest", 1 )
			parameterised["cp"]["cp"].setClass( "maths/multiply", 2 )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node() )

		self.assertEqual( parameterised.parameters().getValue(), fnOH.getParameterised()[0].parameters().getValue() )

	def testClassVectorParameter( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "classVectorParameterTest", 1 )

		with fnOH.parameterModificationContext() as parameterised :

			cv = parameterised["cv"]
			cv.setClasses( [
				( "p0", "maths/multiply", 1 ),
				( "p1", "floatParameter", 1 ),

			] )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node() )

		self.assertEqual( parameterised.parameters().getValue(), fnOH.getParameterised()[0].parameters().getValue() )

	def testNestedClassVectorParameter( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "classVectorParameterTest", 1 )

		with fnOH.parameterModificationContext() as parameterised :

			cv = parameterised["cv"]
			cv.setClasses( [
				( "p0", "classParameterTest", 1 ),
			] )

			cp = parameterised["cv"]["p0"]["cp"]
			cp.setClass( "maths/multiply", 2 )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node() )

		self.assertEqual( parameterised.parameters().getValue(), fnOH.getParameterised()[0].parameters().getValue() )

	def testMoreNestedClassVectorParameter( self ) :

		fnOH = IECoreNuke.FnOpHolder.create( "test", "classVectorParameterTest", 1 )

		with fnOH.parameterModificationContext() as parameterised :

			cv = parameterised["cv"]
			cv.setClasses( [
				( "p0", "classVectorParameterTest", 1 ),

			] )

			cv2 = cv["p0"]["cv"]

			cv2.setClasses( [
				( "p0", "classParameterTest", 1 ),
			] )

			cp = cv2["p0"]["cp"]
			cp.setClass( "maths/multiply", 2 )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node() )

		self.assertEqual( parameterised.parameters().getValue(), fnOH.getParameterised()[0].parameters().getValue() )

	def testParameterTypes( self ) :

		# the parameters for which we know we have no handler
		unsupported = { "c", "e", "f", "compound.k", "m", "s", "u", "v", "x", "y", "p1", "p4" }
		# the parameters for which we'll do our own testing because they are not straightforward to deal with in __checkParameterKnobs
		notEasy = set ( ( "p2", "p3" ) )

		mh = IECore.CapturingMessageHandler()
		with mh :
			fnOH = IECoreNuke.FnOpHolder.create( "test", "parameterTypes", 1 )

		# make sure there's an error reported for each unsupported parameter
		self.assertEqual( len( mh.messages ), len( unsupported ) )
		for n in unsupported :
			found = False
			t = "for parameter \"%s\"" % n.split( "." )[-1]
			for m in mh.messages :
				if t in m.message :
					found = True
					break
			self.assertEqual( found, True )

		self.__checkParameterKnobs( fnOH.getParameterised()[0].parameters(), fnOH.node(), ignore=unsupported | notEasy )

		self.assertEqual( fnOH.node().knob( "parm_p2Start" ).getValue(), [ 1, 1, 1 ] )
		self.assertEqual( fnOH.node().knob( "parm_p2End" ).getValue(), [ 1, 1, 1 ] )

		with fnOH.parameterModificationContext() as parameterised :

			parameterised.parameters()["d"].setTypedValue( "lalal" )
			parameterised.parameters()["p2"].setTypedValue( IECore.LineSegment3f( imath.V3f( 10, 11, 12 ), imath.V3f( 12, 10, 9 ) ) )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node(), ignore=unsupported | notEasy )
		self.__checkParameterKnobs( fnOH.getParameterised()[0].parameters(), fnOH.node(), ignore=unsupported | notEasy )

		self.assertEqual( fnOH.node().knob( "parm_p2Start" ).getValue(), [ 10, 11, 12 ] )
		self.assertEqual( fnOH.node().knob( "parm_p2End" ).getValue(), [ 2, -1, -3 ] )

	def testDefaultExpression( self ) :

		# create opholder and check the default expression we asked for works

		fnOH = IECoreNuke.FnOpHolder.create( "op", "add", 1 )
		self.assertEqual( fnOH.node().knob( "parm_a" ).toScript(), '{"frame * 2"}' )
		self.failUnless( fnOH.node().knob( "parm_a" ).isAnimated() )

		self.assertEqual( nuke.frame(), 1 )
		self.assertEqual( fnOH.node().knob( "parm_a" ).getValue(), 2 )

		# remove the expression, cut and paste the node, and make sure
		# it doesn't reappear

		fnOH.node().knob( "parm_a" ).clearAnimated()
		self.failIf( fnOH.node().knob( "parm_a" ).isAnimated() )

		nuke.nodeCopy( "test/IECoreNuke/parameterisedHolder.nk" )

		nuke.scriptClear()

		n = nuke.nodePaste( "test/IECoreNuke/parameterisedHolder.nk" )

		fnOH = IECoreNuke.FnOpHolder( n )
		self.assertEqual( fnOH.node().knob( "parm_a" ).toScript(), "2" )

	def tearDown( self ) :

		for f in [
				"test/IECoreNuke/parameterisedHolder.nk",
			] :

			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
	unittest.main()

