##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

import os

import maya.cmds
import maya.OpenMaya

import IECore
import IECoreMaya

class TestOp( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "Tests stuff",
			IECore.IntParameter(
				name = "result",
				description = "",
				defaultValue = 0
			)
		)

		self.parameters().addParameters(
			[
				IECore.IntParameter(
					name = "i",
					description = "i",
					defaultValue = 1
				),
			]
		)

	def doOperation( self, args ) :

		return IECore.IntData( 10 )

class FnParameterisedHolderTest( IECoreMaya.TestCase ) :

	def test( self ) :

		node = maya.cmds.createNode( "ieOpHolderNode" )
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		self.assertEqual( fnPH.getParameterised(), ( None, "", 0, "" ) )

		op = TestOp()
		fnPH.setParameterised( op )
		parameterisedTuple = fnPH.getParameterised()
		self.assert_( parameterisedTuple[0].isSame( op ) )
		self.assertEqual( parameterisedTuple[1:], ( "", 0, "" ) )
		self.assertEqual( parameterisedTuple[0](), IECore.IntData( 10 ) )

		iPlug = fnPH.parameterPlug( op["i"] )
		self.assert_( isinstance( iPlug, maya.OpenMaya.MPlug ) )
		self.assert_( iPlug.asInt(), 1 )

		self.assert_( fnPH.plugParameter( iPlug ).isSame( op["i"] ) )
		self.assert_( fnPH.plugParameter( iPlug.name() ).isSame( op["i"] ) )

		iPlug.setInt( 2 )
		fnPH.setParameterisedValue( op["i"] )
		self.assert_( op["i"].getNumericValue(), 2 )

		op["i"].setNumericValue( 3 )
		fnPH.setNodeValue( op["i"] )
		self.assert_( iPlug.asInt(), 3 )

		iPlug.setInt( 10 )
		fnPH.setParameterisedValues()
		self.assert_( op["i"].getNumericValue(), 10 )

		op["i"].setNumericValue( 11 )
		fnPH.setNodeValues()
		self.assert_( iPlug.asInt(), 11 )

	def testFullPathName( self ) :

		node = maya.cmds.createNode( "ieOpHolderNode" )
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		self.assertEqual( node, fnPH.fullPathName() )

		procedural = maya.cmds.createNode( "ieProceduralHolder" )
		fnPH = IECoreMaya.FnParameterisedHolder( procedural )
		self.assertEqual( maya.cmds.ls( procedural, long=True )[0], fnPH.fullPathName() )

	def testPlugParameterWithNonUniqueNames( self ) :

		node = maya.cmds.createNode( "ieProceduralHolder" )
		node2 = maya.cmds.createNode( "ieProceduralHolder" )

		node = maya.cmds.ls( maya.cmds.rename( node, "iAmNotUnique" ), long=True )[0]
		node2 = maya.cmds.ls( maya.cmds.rename( node2, "iAmNotUnique" ), long=True )[0]

		fnPH = IECoreMaya.FnProceduralHolder( node )
		proc = IECore.ReadProcedural()
		fnPH.setParameterised( proc )
		self.assert_( fnPH.getParameterised()[0].isSame( proc ) )

		fnPH2 = IECoreMaya.FnProceduralHolder( node2 )
		proc2 = IECore.ReadProcedural()
		fnPH2.setParameterised( proc2 )
		self.assert_( fnPH2.getParameterised()[0].isSame( proc2 ) )

		# check that each function set references a different node.
		self.assert_( fnPH.object()!=fnPH2.object() )
		self.assert_( fnPH.fullPathName()!=fnPH2.fullPathName() )

		plug = fnPH.parameterPlug( proc["motion"]["blur"] )
		plug2 = fnPH2.parameterPlug( proc2["motion"]["blur"] )

		self.assertEqual( plug.node(), fnPH.object() )
		self.assertEqual( plug2.node(), fnPH2.object() )

		self.assertEqual( fnPH.parameterPlugPath( proc["motion"]["blur"] ), "|transform1|iAmNotUnique.parm_motion_blur" )
		self.assertEqual( fnPH2.parameterPlugPath( proc2["motion"]["blur"] ), "|transform2|iAmNotUnique.parm_motion_blur" )

		self.assert_( maya.cmds.isConnected( "time1.outTime", fnPH.parameterPlugPath( proc["files"]["frame"] ), iuc=True ) )
		self.assert_( maya.cmds.isConnected( "time1.outTime", fnPH2.parameterPlugPath( proc2["files"]["frame"] ), iuc=True ) )

	def testSetNodeValuesUndo( self ) :

		# make an opholder
		##########################################################################

		node = maya.cmds.createNode( "ieOpHolderNode" )
		fnPH = IECoreMaya.FnParameterisedHolder( node )

		op = IECore.ClassLoader.defaultOpLoader().load( "parameterTypes", 1 )()
		op.parameters().removeParameter( "m" ) # no color4f support in maya

		fnPH.setParameterised( op )

		# check we have the starting values we expect
		###########################################################################

		self.assertEqual( op["a"].getNumericValue(), 1 )
		aPlug = fnPH.parameterPlug( op["a"] )
		self.assertEqual( aPlug.asInt(), 1 )

		self.assertEqual( op["b"].getNumericValue(), 2 )
		bPlug = fnPH.parameterPlug( op["b"] )
		self.assertEqual( bPlug.asFloat(), 2 )

		self.assertEqual( op["c"].getNumericValue(), 3 )
		cPlug = fnPH.parameterPlug( op["c"] )
		self.assertEqual( cPlug.asDouble(), 3 )

		self.assertEqual( op["d"].getTypedValue(), "ssss" )
		dPlug = fnPH.parameterPlug( op["d"] )
		self.assertEqual( dPlug.asString(), "ssss" )

		self.assertEqual( op["e"].getValue(), IECore.IntVectorData( [ 4, -1, 2 ] ) )
		ePlug = fnPH.parameterPlug( op["e"] )
		fnE = maya.OpenMaya.MFnIntArrayData( ePlug.asMObject() )
		self.assertEqual( fnE[0], 4 )
		self.assertEqual( fnE[1], -1 )
		self.assertEqual( fnE[2], 2 )
		self.assertEqual( fnE.length(), 3 )

		self.assertEqual( op["f"].getValue(), IECore.StringVectorData( [ "one", "two", "three" ] ) )
		fPlug = fnPH.parameterPlug( op["f"] )
		fnF = maya.OpenMaya.MFnStringArrayData( fPlug.asMObject() )
		fList = []
		fnF.copyTo( fList )
		self.assertEqual( fList, [ "one", "two", "three" ] )

		self.assertEqual( op["g"].getTypedValue(), IECore.V2f( 1, 2 ) )
		gPlug = fnPH.parameterPlug( op["g"] )
		self.assertEqual( gPlug.child( 0 ).asFloat(), 1 )
		self.assertEqual( gPlug.child( 1 ).asFloat(), 2 )

		self.assertEqual( op["h"].getTypedValue(), IECore.V3f( 1, 1, 1 ) )
		hPlug = fnPH.parameterPlug( op["h"] )
		self.assertEqual( hPlug.child( 0 ).asFloat(), 1 )
		self.assertEqual( hPlug.child( 1 ).asFloat(), 1 )
		self.assertEqual( hPlug.child( 2 ).asFloat(), 1 )

		self.assertEqual( op["q"].getTypedValue(), False )
		qPlug = fnPH.parameterPlug( op["q"] )
		self.assertEqual( qPlug.asBool(), False )

		self.assertEqual( op["t"].getTypedValue(), IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) ) )
		tPlug = fnPH.parameterPlug( op["t"] )
		self.assertEqual( tPlug.child( 0 ).child( 0 ).asFloat(), -1 )
		self.assertEqual( tPlug.child( 0 ).child( 1 ).asFloat(), -1 )
		self.assertEqual( tPlug.child( 0 ).child( 2 ).asFloat(), -1 )
		self.assertEqual( tPlug.child( 1 ).child( 0 ).asFloat(), 1 )
		self.assertEqual( tPlug.child( 1 ).child( 1 ).asFloat(), 1 )
		self.assertEqual( tPlug.child( 1 ).child( 2 ).asFloat(), 1 )

		# change all the node values, making sure undo is enabled
		#############################################################################

		self.assert_( maya.cmds.undoInfo( query=True, state=True ) )

		# change the parameters
		op["a"].setNumericValue( 10 )
		op["b"].setNumericValue( 100 )
		op["c"].setNumericValue( 12 )
		op["d"].setTypedValue( "a" )
		op["e"].setValue( IECore.IntVectorData( [ 1, 2, 3, 4 ] ) )
		op["f"].setValue( IECore.StringVectorData( [ "hi" ] ) )
		op["g"].setTypedValue( IECore.V2f( 10, 100 ) )
		op["h"].setTypedValue( IECore.V3f( -1, -2, -3 ) )
		op["q"].setTypedValue( True )
		op["t"].setTypedValue( IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 0 ) ) )

		# check they are changed
		self.assertEqual( op["a"].getNumericValue(), 10 )
		self.assertEqual( op["b"].getNumericValue(), 100 )
		self.assertEqual( op["c"].getNumericValue(), 12 )
		self.assertEqual( op["d"].getTypedValue(), "a" )
		self.assertEqual( op["e"].getValue(), IECore.IntVectorData( [ 1, 2, 3, 4 ] ) )
		self.assertEqual( op["f"].getValue(), IECore.StringVectorData( [ "hi" ] ) )
		self.assertEqual( op["g"].getTypedValue(), IECore.V2f( 10, 100 ) )
		self.assertEqual( op["h"].getTypedValue(), IECore.V3f( -1, -2, -3 ) )
		self.assertEqual( op["q"].getTypedValue(), True )
		self.assertEqual( op["t"].getTypedValue(), IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 0 ) ) )

		# push the changes onto the node
		fnPH.setNodeValues()

		# check the node values are changed
		#############################################################################

		self.assertEqual( aPlug.asInt(), 10 )
		self.assertEqual( bPlug.asFloat(), 100 )
		self.assertEqual( cPlug.asDouble(), 12 )
		self.assertEqual( dPlug.asString(), "a" )

		fnE = maya.OpenMaya.MFnIntArrayData( ePlug.asMObject() )
		self.assertEqual( fnE[0], 1 )
		self.assertEqual( fnE[1], 2 )
		self.assertEqual( fnE[2], 3 )
		self.assertEqual( fnE[3], 4 )
		self.assertEqual( fnE.length(), 4 )

		fnF = maya.OpenMaya.MFnStringArrayData( fPlug.asMObject() )
		fList = []
		fnF.copyTo( fList )
		self.assertEqual( fList, [ "hi" ] )

		self.assertEqual( gPlug.child( 0 ).asFloat(), 10 )
		self.assertEqual( gPlug.child( 1 ).asFloat(), 100 )

		self.assertEqual( hPlug.child( 0 ).asFloat(), -1 )
		self.assertEqual( hPlug.child( 1 ).asFloat(), -2 )
		self.assertEqual( hPlug.child( 2 ).asFloat(), -3 )

		self.assertEqual( qPlug.asBool(), True )

		self.assertEqual( tPlug.child( 0 ).child( 0 ).asFloat(), -10 )
		self.assertEqual( tPlug.child( 0 ).child( 1 ).asFloat(), -10 )
		self.assertEqual( tPlug.child( 0 ).child( 2 ).asFloat(), -10 )
		self.assertEqual( tPlug.child( 1 ).child( 0 ).asFloat(), 0 )
		self.assertEqual( tPlug.child( 1 ).child( 1 ).asFloat(), 0 )
		self.assertEqual( tPlug.child( 1 ).child( 2 ).asFloat(), 0 )

		# check that the parameter values are unchanged in the process of
		# pushing them to maya
		#############################################################################

		self.assertEqual( op["a"].getNumericValue(), 10 )
		self.assertEqual( op["b"].getNumericValue(), 100 )
		self.assertEqual( op["c"].getNumericValue(), 12 )
		self.assertEqual( op["d"].getTypedValue(), "a" )
		self.assertEqual( op["e"].getValue(), IECore.IntVectorData( [ 1, 2, 3, 4 ] ) )
		self.assertEqual( op["f"].getValue(), IECore.StringVectorData( [ "hi" ] ) )
		self.assertEqual( op["g"].getTypedValue(), IECore.V2f( 10, 100 ) )
		self.assertEqual( op["h"].getTypedValue(), IECore.V3f( -1, -2, -3 ) )
		self.assertEqual( op["q"].getTypedValue(), True )
		self.assertEqual( op["t"].getTypedValue(), IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 0 ) ) )


		# undo, and check the node values are back to before
		#############################################################################

		maya.cmds.undo()

		self.assertEqual( aPlug.asInt(), 1 )
		self.assertEqual( bPlug.asFloat(), 2 )
		self.assertEqual( cPlug.asDouble(), 3 )
		self.assertEqual( dPlug.asString(), "ssss" )

		fnE = maya.OpenMaya.MFnIntArrayData( ePlug.asMObject() )
		self.assertEqual( fnE[0], 4 )
		self.assertEqual( fnE[1], -1 )
		self.assertEqual( fnE[2], 2 )
		self.assertEqual( fnE.length(), 3 )

		fnF = maya.OpenMaya.MFnStringArrayData( fPlug.asMObject() )
		fList = []
		fnF.copyTo( fList )
		self.assertEqual( fList, [ "one", "two", "three" ] )

		self.assertEqual( gPlug.child( 0 ).asFloat(), 1 )
		self.assertEqual( gPlug.child( 1 ).asFloat(), 2 )

		self.assertEqual( hPlug.child( 0 ).asFloat(), 1 )
		self.assertEqual( hPlug.child( 1 ).asFloat(), 1 )
		self.assertEqual( hPlug.child( 2 ).asFloat(), 1 )

		self.assertEqual( qPlug.asBool(), False )

		self.assertEqual( tPlug.child( 0 ).child( 0 ).asFloat(), -1 )
		self.assertEqual( tPlug.child( 0 ).child( 1 ).asFloat(), -1 )
		self.assertEqual( tPlug.child( 0 ).child( 2 ).asFloat(), -1 )
		self.assertEqual( tPlug.child( 1 ).child( 0 ).asFloat(), 1 )
		self.assertEqual( tPlug.child( 1 ).child( 1 ).asFloat(), 1 )
		self.assertEqual( tPlug.child( 1 ).child( 2 ).asFloat(), 1 )

		# check that the parameter values are unchanged in the undo process
		#############################################################################

		self.assertEqual( op["a"].getNumericValue(), 10 )
		self.assertEqual( op["b"].getNumericValue(), 100 )
		self.assertEqual( op["c"].getNumericValue(), 12 )
		self.assertEqual( op["d"].getTypedValue(), "a" )
		self.assertEqual( op["e"].getValue(), IECore.IntVectorData( [ 1, 2, 3, 4 ] ) )
		self.assertEqual( op["f"].getValue(), IECore.StringVectorData( [ "hi" ] ) )
		self.assertEqual( op["g"].getTypedValue(), IECore.V2f( 10, 100 ) )
		self.assertEqual( op["h"].getTypedValue(), IECore.V3f( -1, -2, -3 ) )
		self.assertEqual( op["q"].getTypedValue(), True )
		self.assertEqual( op["t"].getTypedValue(), IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 0 ) ) )

		# redo, and check they are changed again
		#############################################################################

		maya.cmds.redo()

		self.assertEqual( aPlug.asInt(), 10 )
		self.assertEqual( bPlug.asFloat(), 100 )
		self.assertEqual( cPlug.asDouble(), 12 )
		self.assertEqual( dPlug.asString(), "a" )

		fnE = maya.OpenMaya.MFnIntArrayData( ePlug.asMObject() )
		self.assertEqual( fnE[0], 1 )
		self.assertEqual( fnE[1], 2 )
		self.assertEqual( fnE[2], 3 )
		self.assertEqual( fnE[3], 4 )
		self.assertEqual( fnE.length(), 4 )

		fnF = maya.OpenMaya.MFnStringArrayData( fPlug.asMObject() )
		fList = []
		fnF.copyTo( fList )
		self.assertEqual( fList, [ "hi" ] )

		self.assertEqual( gPlug.child( 0 ).asFloat(), 10 )
		self.assertEqual( gPlug.child( 1 ).asFloat(), 100 )

		self.assertEqual( hPlug.child( 0 ).asFloat(), -1 )
		self.assertEqual( hPlug.child( 1 ).asFloat(), -2 )
		self.assertEqual( hPlug.child( 2 ).asFloat(), -3 )

		self.assertEqual( qPlug.asBool(), True )

		self.assertEqual( tPlug.child( 0 ).child( 0 ).asFloat(), -10 )
		self.assertEqual( tPlug.child( 0 ).child( 1 ).asFloat(), -10 )
		self.assertEqual( tPlug.child( 0 ).child( 2 ).asFloat(), -10 )
		self.assertEqual( tPlug.child( 1 ).child( 0 ).asFloat(), 0 )
		self.assertEqual( tPlug.child( 1 ).child( 1 ).asFloat(), 0 )
		self.assertEqual( tPlug.child( 1 ).child( 2 ).asFloat(), 0 )

		# check that the parameter values are unchanged in the redo process
		#############################################################################

		self.assertEqual( op["a"].getNumericValue(), 10 )
		self.assertEqual( op["b"].getNumericValue(), 100 )
		self.assertEqual( op["c"].getNumericValue(), 12 )
		self.assertEqual( op["d"].getTypedValue(), "a" )
		self.assertEqual( op["e"].getValue(), IECore.IntVectorData( [ 1, 2, 3, 4 ] ) )
		self.assertEqual( op["f"].getValue(), IECore.StringVectorData( [ "hi" ] ) )
		self.assertEqual( op["g"].getTypedValue(), IECore.V2f( 10, 100 ) )
		self.assertEqual( op["h"].getTypedValue(), IECore.V3f( -1, -2, -3 ) )
		self.assertEqual( op["q"].getTypedValue(), True )
		self.assertEqual( op["t"].getTypedValue(), IECore.Box3f( IECore.V3f( -10 ), IECore.V3f( 0 ) ) )

	def testSetNodeValueUndo( self ) :

		p = IECore.Parameterised( "" )
		p.parameters().addParameters(
			[
				IECore.IntParameter(
					"i",
					"",
					1
				),
				IECore.FloatParameter(
					"f",
					"",
					2
				)
			]
		)

		node = maya.cmds.createNode( "ieParameterisedHolderLocator" )
		fnOH = IECoreMaya.FnParameterisedHolder( node )
		fnOH.setParameterised( p )

		# check the start values are as expected

		self.assertEqual( p["i"].getNumericValue(), 1 )
		self.assertEqual( p["f"].getNumericValue(), 2 )

		self.assertEqual( fnOH.parameterPlug( p["i"] ).asInt(), 1 )
		self.assertEqual( fnOH.parameterPlug( p["f"] ).asInt(), 2 )

		# change both parameters

		self.assert_( maya.cmds.undoInfo( query=True, state=True ) )

		p["i"].setNumericValue( 10 )
		p["f"].setNumericValue( 11 )

		self.assertEqual( p["i"].getNumericValue(), 10 )
		self.assertEqual( p["f"].getNumericValue(), 11 )

		self.assertEqual( fnOH.parameterPlug( p["i"] ).asInt(), 1 )
		self.assertEqual( fnOH.parameterPlug( p["f"] ).asInt(), 2 )

		# but push only one into maya

		fnOH.setNodeValue( p["i"] )

		# and check we see what we expect

		self.assertEqual( p["i"].getNumericValue(), 10 )
		self.assertEqual( p["f"].getNumericValue(), 11 )

		self.assertEqual( fnOH.parameterPlug( p["i"] ).asInt(), 10 )
		self.assertEqual( fnOH.parameterPlug( p["f"] ).asInt(), 2 )

		# undo and check

		maya.cmds.undo()

		self.assertEqual( p["i"].getNumericValue(), 10 )
		self.assertEqual( p["f"].getNumericValue(), 11 )

		self.assertEqual( fnOH.parameterPlug( p["i"] ).asInt(), 1 )
		self.assertEqual( fnOH.parameterPlug( p["f"] ).asInt(), 2 )

		# redo and check

		maya.cmds.redo()

		self.assertEqual( p["i"].getNumericValue(), 10 )
		self.assertEqual( p["f"].getNumericValue(), 11 )

		self.assertEqual( fnOH.parameterPlug( p["i"] ).asInt(), 10 )
		self.assertEqual( fnOH.parameterPlug( p["f"] ).asInt(), 2 )

	def testSetNodeValuesExcessReferenceEdits( self ) :
		"""Test for excessive reference edits being generated by setNodeValues()"""
		
		IECoreMaya.FnOpHolder.create( "testOp", "maths/multiply", 2 )

		# Save the scene out so we can reference it
		maya.cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "referenceEditCounts.ma" ) )
		referenceScene = maya.cmds.file( force = True, type = "mayaAscii", save = True )

		# New scene, and read it in.
		maya.cmds.file( new = True, force = True )
		maya.cmds.file( referenceScene, reference = True, namespace = "ns1" )
		
		# Check there are no reference edits
		self.assertEqual( len(maya.cmds.referenceQuery( referenceScene, editStrings=True )), 0 )

		#set values, with no changes
		fnPH = IECoreMaya.FnParameterisedHolder( 'ns1:testOp' )
		fnPH.setParameterisedValues()
		fnPH.setNodeValues()
		
		# Check there are still no reference edits
		self.assertEqual( len(maya.cmds.referenceQuery( referenceScene, editStrings=True )), 0 ) # Expected failure, that there will be a 2 edits, as setting a plug value to its existing value still counts as an 'edit'.
		
		#change a value
		fnPH.getParameterised()[0].parameters()["a"].setNumericValue( 100 )
		fnPH.setNodeValues()
		
		# Check there is 1 reference edit
		self.assertEqual( len(maya.cmds.referenceQuery( referenceScene, editStrings=True )), 1 ) 
				
		
	def testBoxDefaultValue( self ) :

		node = maya.cmds.createNode( "ieOpHolderNode" )
		fnPH = IECoreMaya.FnParameterisedHolder( node )

		op = IECore.ClassLoader.defaultOpLoader().load( "parameterTypes", 1 )()
		op.parameters().removeParameter( "m" ) # no color4f support in maya

		fnPH.setParameterised( op )
		
		node, plug = fnPH.parameterPlugPath( op["s"] ).split( "." )
		self.assertEqual( maya.cmds.attributeQuery( plug + "Min", listDefault=True, node=node ), [ -1.0, -1.0 ] )
		self.assertEqual( maya.cmds.attributeQuery( plug + "Max", listDefault=True, node=node ), [ 1.0, 1.0 ] )

		node, plug = fnPH.parameterPlugPath( op["t"] ).split( "." )
		self.assertEqual( maya.cmds.attributeQuery( plug + "Min", listDefault=True, node=node ), [ -1.0, -1.0, -1.0 ] )
		self.assertEqual( maya.cmds.attributeQuery( plug + "Max", listDefault=True, node=node ), [ 1.0, 1.0, 1.0 ] )

	def testArrayPlugCreation( self ) :

		op = IECore.Op( 'test op', IECore.IntParameter( 'result', '', 0 ) )
		op.parameters().addParameters( [
			IECore.V3fVectorParameter( 'v3fVector', '', IECore.V3fVectorData() ),
			IECore.V3dVectorParameter( 'v3dVector', '', IECore.V3dVectorData() ),
			IECore.StringVectorParameter( 'stringVector', '', IECore.StringVectorData() ),
			IECore.DoubleVectorParameter( 'doubleVector', '', IECore.DoubleVectorData() ),
			IECore.FloatVectorParameter( 'floatVector', '', IECore.FloatVectorData() ),
			IECore.IntVectorParameter( 'intVector', '', IECore.IntVectorData() ),
			IECore.BoolVectorParameter( 'boolVector', '', IECore.BoolVectorData() ),
			IECore.M44fVectorParameter( 'm44fVector', '', IECore.M44fVectorData() ),
			IECore.M44dVectorParameter( 'm44dVector', '', IECore.M44dVectorData() ),
		] )
		
		node = maya.cmds.createNode( 'ieOpHolderNode' )
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		
		self.assert_( not maya.cmds.objExists( node+'.parm_v3fVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_v3dVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_stringVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_doubleVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_floatVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_intVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_boolVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_m44fVector' ) )
		self.assert_( not maya.cmds.objExists( node+'.parm_m44dVector' ) )
		
		fnPH.setParameterised( op )
		
		self.assert_( maya.cmds.objExists( node+'.parm_v3fVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_v3dVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_stringVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_doubleVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_floatVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_intVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_boolVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_m44fVector' ) )
		self.assert_( maya.cmds.objExists( node+'.parm_m44dVector' ) )
				
		self.assertEqual( maya.cmds.getAttr( node+'.parm_v3fVector', type=True ), 'vectorArray' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_v3dVector', type=True ), 'vectorArray' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_stringVector', type=True ), 'stringArray' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_doubleVector', type=True ), 'doubleArray' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_floatVector', type=True ), 'doubleArray' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_intVector', type=True ), 'Int32Array' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_boolVector', type=True ), 'Int32Array' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_m44fVector', type=True ), 'doubleArray' )
		self.assertEqual( maya.cmds.getAttr( node+'.parm_m44dVector', type=True ), 'doubleArray' )
	
	def testMatrixVectorPlugs( self ) :

		m44fVector = IECore.M44fVectorData( [ IECore.M44f( 1 ), IECore.M44f( 2 ), IECore.M44f( 3 ) ] )
		m44dVector = IECore.M44dVectorData( [ IECore.M44d( 1 ), IECore.M44d( 2 ), IECore.M44d( 3 ) ] )
		reverseM44fVector = IECore.M44fVectorData( [ IECore.M44f( 3 ), IECore.M44f( 2 ), IECore.M44f( 1 ) ] )
		reverseM44dVector = IECore.M44dVectorData( [ IECore.M44d( 3 ), IECore.M44d( 2 ), IECore.M44d( 1 ) ] )
		
		mayaArray = []
		for i in range( 0, 3 ) :
			for j in range( 0, 16 ) :
				mayaArray.append( i+1 )
		
		reverseMayaArray = list(mayaArray)
		reverseMayaArray.reverse()
		
		op = IECore.Op( 'test op', IECore.IntParameter( 'result', '', 0 ) )
		op.parameters().addParameters( [
			IECore.M44fVectorParameter( 'm44fVector', '', IECore.M44fVectorData() ),
			IECore.M44dVectorParameter( 'm44dVector', '', IECore.M44dVectorData() ),
		] )
		
		node = maya.cmds.createNode( 'ieOpHolderNode' )
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		fnPH.setParameterised( op )
		
		# set from cortex to maya
		self.assertNotEqual( mayaArray, maya.cmds.getAttr( node+'.parm_m44fVector' ) )
		self.assertNotEqual( mayaArray, maya.cmds.getAttr( node+'.parm_m44dVector' ) )
		fnPH.getParameterised()[0].parameters()['m44fVector'].setValue( m44fVector )
		fnPH.getParameterised()[0].parameters()['m44dVector'].setValue( m44dVector )
		fnPH.setNodeValues()
		self.assertEqual( mayaArray, maya.cmds.getAttr( node+'.parm_m44fVector' ) )
		self.assertEqual( mayaArray, maya.cmds.getAttr( node+'.parm_m44dVector' ) )
		
		# set from maya to cortex
		self.assertNotEqual( reverseM44fVector, fnPH.getParameterised()[0].parameters()['m44fVector'].getValue() )
		self.assertNotEqual( reverseM44dVector, fnPH.getParameterised()[0].parameters()['m44dVector'].getValue() )
		maya.cmds.setAttr( node+'.parm_m44fVector', reverseMayaArray, type="doubleArray" )
		maya.cmds.setAttr( node+'.parm_m44dVector', reverseMayaArray, type="doubleArray" )
		fnPH.setParameterisedValues()		
		self.assertEqual( reverseM44fVector, fnPH.getParameterised()[0].parameters()['m44fVector'].getValue() )
		self.assertEqual( reverseM44dVector, fnPH.getParameterised()[0].parameters()['m44dVector'].getValue() )
		
		# set to incorrect length from maya
		maya.cmds.setAttr( node+'.parm_m44fVector', [0,1,2], type="doubleArray" )
		maya.cmds.setAttr( node+'.parm_m44dVector', [0,1,2], type="doubleArray" )
		fnPH.setParameterisedValues()
		self.assertEqual( None, fnPH.getParameterised()[0].parameters()['m44fVector'].getValue() )
		self.assertEqual( None, fnPH.getParameterised()[0].parameters()['m44dVector'].getValue() )
	
	def tearDown( self ) :

		for f in [
			"test/IECoreMaya/referenceEditCounts.ma",
		] :

			if os.path.exists( f ) :
				os.remove( f )
				
if __name__ == "__main__":
	IECoreMaya.TestProgram()
