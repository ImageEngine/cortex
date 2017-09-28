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

import IECore
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
						knobValue = IECore.V2d( knobValue[0], knobValue[1] )
					elif isinstance( parameter, IECore.V3dParameter ) :
						knobValue = IECore.V3d( knobValue[0], knobValue[1], knobValue[2] )
				except :
					# not all knob types have accessors yet. some of the numeric
					# knobs don't have them because nuke has bugs and returns those
					# knobs as the wrong type. try to get the value another way.
					knobValue = knob.getValue()

				self.assertEqual( parameter.getValue().value, knobValue )

	def testCreate( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		self.assertEqual( fnPH.node().name(), "procedural" )

		p = fnPH.getParameterised()

		self.failUnless( isinstance( p[0], IECore.ReadProcedural ) )
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )

		self.__checkParameterKnobs( p[0].parameters(), fnPH.node() )

	def testCreateWithoutVersion( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural2", "read" )

		self.assertEqual( fnPH.node().name(), "procedural2" )
		p = fnPH.getParameterised()

		self.failUnless( isinstance( p[0], IECore.ReadProcedural ) )
		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )

		self.__checkParameterKnobs( p[0].parameters(), fnPH.node() )

	def testReloadShouldntLoseValues( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 1 )

		fnPH.node().knob( "parm_files_frame" ).setValue( 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )

		fnPH.node().knob( "classReload" ).execute() # trigger reload

		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )

	def testReloadShouldntLoseAnimatedValues( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		fnPH.node().knob( "parm_files_frame" ).setExpression( "frame" )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).hasExpression() )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).isAnimated() )

		fnPH.node().knob( "classReload" ).execute() # trigger reload

		self.failUnless( fnPH.node().knob( "parm_files_frame" ).hasExpression() )
		self.failUnless( fnPH.node().knob( "parm_files_frame" ).isAnimated() )

	def testReloadShouldntLoseDefaultValues( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).defaultValue(), 1 )

		fnPH.node().knob( "parm_files_frame" ).setValue( 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )

		fnPH.node().knob( "classReload" ).execute() # trigger reload

		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).getValue(), 10 )
		self.assertEqual( fnPH.node().knob( "parm_files_frame" ).defaultValue(), 1 )

	def testCopyPaste( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		nuke.nodeCopy( "test/IECoreNuke/parameterisedHolder.nk" )

		nuke.scriptClear()

		n = nuke.nodePaste( "test/IECoreNuke/parameterisedHolder.nk" )

		fnPH = IECoreNuke.FnProceduralHolder( n )

		p = fnPH.getParameterised()

		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )

	def testRemoveClass( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		p = fnPH.getParameterised()

		self.assertEqual( p[1], "read" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 1 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )

		fnPH.setProcedural( "", 0 )

		p = fnPH.getParameterised()

		self.assertEqual( p[1], "" )
		self.failUnless( isinstance( p[2], int ) )
		self.assertEqual( p[2], 0 )
		self.assertEqual( p[3], "IECORE_PROCEDURAL_PATHS" )

		for k in fnPH.node().knobs() :

			self.failIf( k.startswith( "parm_" ) )

	def testGetParameterisedHasCorrectValues( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		fnPH.node().knob( "parm_files_frame" ).setValue( 100 )
		fnPH.node().knob( "parm_files_name" ).setValue( "test" )
		fnPH.node().knob( "parm_motion_blur" ).setValue( False )
		fnPH.node().knob( "parm_bounds_specified" ).setValue( [ 0, 1, 2, 3, 4, 5 ] )

		self.__checkParameterKnobs( fnPH.getParameterised()[0].parameters(), fnPH.node() )

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
		unsupported = set( ( "c", "e", "f", "compound.k", "m", "s", "u", "v", "x", "y", "p1", "p2", "p4", "p5", "p6", "p9" ) )
		# the parameters for which we have a handler but expect inputs instead of knobs
		inputsNotKnobs = set( ( "p3", ) )
		# the parameters for which we'll do our own testing because they are not straightforward to deal with in __checkParameterKnobs
		notEasy = set ( ( "p7", "p8" ) )

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

		self.__checkParameterKnobs( fnOH.getParameterised()[0].parameters(), fnOH.node(), ignore=unsupported | inputsNotKnobs | notEasy )

		self.assertEqual( fnOH.node().knob( "parm_p7Start" ).getValue(), [ 1, 1, 1 ] )
		self.assertEqual( fnOH.node().knob( "parm_p7End" ).getValue(), [ 1, 1, 1 ] )

		with fnOH.parameterModificationContext() as parameterised :

			parameterised.parameters()["d"].setTypedValue( "lalal" )
			parameterised.parameters()["p7"].setTypedValue( IECore.LineSegment3f( IECore.V3f( 10, 11, 12 ), IECore.V3f( 12, 10, 9 ) ) )

		self.__checkParameterKnobs( parameterised.parameters(), fnOH.node(), ignore=unsupported | inputsNotKnobs | notEasy )
		self.__checkParameterKnobs( fnOH.getParameterised()[0].parameters(), fnOH.node(), ignore=unsupported | inputsNotKnobs | notEasy )

		self.assertEqual( fnOH.node().knob( "parm_p7Start" ).getValue(), [ 10, 11, 12 ] )
		self.assertEqual( fnOH.node().knob( "parm_p7End" ).getValue(), [ 2, -1, -3 ] )

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

	def testReloadShouldntLoseFilenameParameterValues( self ) :

		fnPH = IECoreNuke.FnProceduralHolder.create( "procedural", "read", 1 )

		self.assertEqual( fnPH.node().knob( "parm_files_name" ).getText(), "" )

		fnPH.node().knob( "parm_files_name" ).setValue( "abcdef" )
		self.assertEqual( fnPH.node().knob( "parm_files_name" ).getText(), "abcdef" )

		fnPH.node().knob( "classReload" ).execute() # trigger reload

		self.assertEqual( fnPH.node().knob( "parm_files_name" ).getText(), "abcdef" )

	def tearDown( self ) :

		for f in [
				"test/IECoreNuke/parameterisedHolder.nk",
			] :

			if os.path.exists( f ) :
				os.remove( f )

if __name__ == "__main__":
	unittest.main()

