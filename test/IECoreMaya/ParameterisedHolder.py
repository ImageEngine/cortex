##########################################################################
#
#  Copyright (c) 2008-2014, Image Engine Design Inc. All rights reserved.
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

import os.path

import maya.cmds as cmds
import maya.OpenMaya as OpenMaya

import IECore
import IECoreMaya

import sys

class TestParameterisedHolder( IECoreMaya.TestCase ) :

	def __checkAllParameterPlugs( self, fnOH, parameter=None ) :

		if parameter is not None :
			plug = fnOH.parameterPlug( parameter )
			self.failIf( plug.isNull() )
		else :
			parameter = fnOH.getParameterised()[0].parameters()

		if parameter.isInstanceOf( IECore.CompoundParameter.staticTypeId() ) :
			for p in parameter.values() :
				self.__checkAllParameterPlugs( fnOH, p )

	def testNode( self ):
		""" Test ParameterisedHolderNode """
		n = cmds.createNode( "ieParameterisedHolderNode" )
		h = IECoreMaya.FnParameterisedHolder( str(n) )
		self.assert_( h )

		p = IECore.SequenceLsOp()

		h.setParameterised( p )

		p.parameters()["dir"] = "testValue"
		h.setNodeValue( p.parameters()["dir"] )
		pl = h.parameterPlug( p.parameters()["dir"] )
		v = IECoreMaya.FromMayaPlugConverter.create( pl, IECore.TypeId.StringData ).convert()
		self.assertEqual( v.value, "testValue" )

		cmds.setAttr( pl.name(), "testValue2", typ="string" )
		h.setParameterisedValue( p.parameters()["dir"] )
		self.assertEqual( p.parameters()["dir"].getValue().value, "testValue2" )


	def testParameterisedHolderSetReference( self ):
		""" Test multiple references to ieParameterisedHolderSet nodes """

		nodeType = "ieParameterisedHolderSet"

		nodeName = cmds.createNode( nodeType )

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "reference.ma" ) )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		cmds.file( new = True, force = True )
		cmds.file( scene, reference = True, namespace = "ns1" )
		cmds.file( scene, reference = True, namespace = "ns2" )

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "referenceMaster.ma" ) )
		masterScene = cmds.file( force = True, type = "mayaAscii", save = True )

		cmds.file( masterScene, force = True, open = True )

		nodeName1 = "ns1:" + nodeName
		nodeName2 = "ns2:" + nodeName

		l = OpenMaya.MSelectionList()
		l.add( nodeName1 )
		l.add( nodeName2 )

		node1 = OpenMaya.MObject()
		l.getDependNode( 0, node1 )
		node2 = OpenMaya.MObject()
		l.getDependNode( 1, node2 )

		fn1 = OpenMaya.MFnDependencyNode( node1 )
		fn2 = OpenMaya.MFnDependencyNode( node2 )

		self.assert_( fn1.userNode() )
		self.assert_( fn2.userNode() ) # This failure is due to a Maya bug. When referencing the same scene twice, as an optimisation Maya will duplicate existing nodes instead of creating new ones. There is a bug in MPxObjectSet::copy() which gets exercised here. Setting the environment variable MAYA_FORCE_REF_READ to 1 will disable this optimisation, however.

	def testChangeDefault( self ) :
		""" Test that changing parameter defaults is correctly reflected in Maya attributes """

		def makeOp( defaultValue ) :

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
							IECore.Color3fParameter(
								name = "c",
								description = "",
								defaultValue = defaultValue
							),
						]
					)

			return TestOp()


		n = cmds.createNode( "ieParameterisedHolderNode" )
		h = IECoreMaya.FnParameterisedHolder( str(n) )
		self.assert_( h )

		p = makeOp( IECore.Color3f( 0, 0, 0 ) )
		h.setParameterised( p )
		dv = cmds.attributeQuery ( "parm_c", node = n, listDefault = True )
		self.assertEqual( dv, [ 0, 0, 0 ] )

		p = makeOp( IECore.Color3f( 1, 1, 1 ) )
		h.setParameterised( p )
		dv = cmds.attributeQuery ( "parm_c", node = n, listDefault = True )
		self.assertEqual( dv, [ 1, 1, 1 ] )

	def testDirectSettingOfOp( self ) :

		class TestOp( IECore.Op ) :

			def __init__( self ) :

				IECore.Op.__init__( self,
					"",
					IECore.FloatParameter(
						"result",
						"",
						0.0
					),
				)

				self.parameters().addParameter(

					IECore.FloatParameter(
						"a",
						"",
						0.0
					)

				)

			def doOperation( self, operands ) :

				return IECore.FloatData( operands["a"].value )

		node = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnParameterisedHolder( str( node ) )

		op = TestOp()
		fnOH.setParameterised( op )

		self.failUnless( cmds.objExists( node + ".result" ) )

		aAttr = fnOH.parameterPlugPath( op["a"] )

		cmds.setAttr( aAttr, 10 )
		self.assertEqual( cmds.getAttr( node + ".result" ), 10 )

		cmds.setAttr( aAttr, 20 )
		self.assertEqual( cmds.getAttr( node + ".result" ), 20 )


	def testLazySettingFromCompoundPlugs( self ) :

		class TestProcedural( IECore.ParameterisedProcedural ) :

			def __init__( self ) :

				IECore.ParameterisedProcedural.__init__( self, "" )

				self.parameters().addParameter(

					IECore.V3fParameter(
						"halfSize",
						"",
						IECore.V3f( 0 )
					)

				)

			def doBound( self, args ) :

				return IECore.Box3f( -args["halfSize"].value, args["halfSize"].value )

			def doRenderState( self, args ) :

				pass

			def doRender( self, args ) :

				pass

		node = cmds.createNode( "ieProceduralHolder" )
		fnPH = IECoreMaya.FnParameterisedHolder( str( node ) )

		p = TestProcedural()
		fnPH.setParameterised( p )

		self.assertEqual( cmds.getAttr( node + ".boundingBoxMin" ), [( 0, 0, 0 )] )
		cmds.setAttr( fnPH.parameterPlugPath( p["halfSize"] ), 1, 2, 3 )

		self.assertEqual( cmds.getAttr( node + ".boundingBoxMin" ), [( -1, -2, -3 )] )

	def testLazySettingFromArrayPlugs( self ) :

		class TestProcedural( IECore.ParameterisedProcedural ) :

			def __init__( self ) :

				IECore.ParameterisedProcedural.__init__( self, "" )

				self.parameters().addParameter(

					IECore.SplineffParameter(
						"spline",
						"",
						defaultValue = IECore.SplineffData(
							IECore.Splineff(
								IECore.CubicBasisf.catmullRom(),
								(
									( 0, 1 ),
									( 0, 1 ),
									( 1, 0 ),
									( 1, 0 ),
								),
							),
						),
					),

				)

			def doBound( self, args ) :

				v = args["spline"].value.points()[0][1]

				return IECore.Box3f( IECore.V3f( -v ), IECore.V3f( v ) )

			def doRenderState( self, args ) :

				pass

			def doRender( self, args ) :

				pass

		node = cmds.createNode( "ieProceduralHolder" )
		fnPH = IECoreMaya.FnParameterisedHolder( str( node ) )

		p = TestProcedural()
		fnPH.setParameterised( p )

		self.assertEqual( cmds.getAttr( node + ".boundingBoxMin" ), [( -1, -1, -1 )] )

		plugPath = fnPH.parameterPlugPath( p["spline"] )
		plugName = plugPath.partition( "." )[2]
		pointValuePlugPath = plugPath + "[0]." + plugName + "_FloatValue"

		cmds.setAttr( pointValuePlugPath, 2 )

		self.assertEqual( cmds.getAttr( node + ".boundingBoxMin" ), [( -2, -2, -2 )] )

	def testObjectParameterIOProblem( self ) :

		fnOP = IECoreMaya.FnOpHolder.create( "opHolder", "compoundObjectInOut", 1 )
		op = fnOP.getOp()

		c = IECore.CompoundObject( {
			"s" : IECore.StringData( "this" ),
			"t" : IECore.TimeCodeData( IECore.TimeCode( 12, 5, 3, 15, dropFrame = True, bgf1 = True, binaryGroup6 = 12 ) ),
		} )
		op.parameters()["input"].setValue( c )
		fnOP.setNodeValues()
		node = fnOP.name()

		cmds.file( rename = os.getcwd() + "/test/IECoreMaya/objectParameterIO.ma" )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		fnOP = IECoreMaya.FnOpHolder( node )
		fnOP.setParameterisedValues()
		op = fnOP.getOp()

		self.assertEqual( op.parameters()["input"].getValue(), c )

	def testObjectMFnDataParameterIOProblem( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "matrixParameter", 1 )
		op = fnOH.getOp()

		locator = cmds.spaceLocator()[0]

		parameterPlugPath = fnOH.parameterPlugPath( op.parameters()['matrix'] )
		attrPlugPath = '%s.worldMatrix' % ( locator )
		cmds.connectAttr( attrPlugPath, parameterPlugPath )

		cmds.file( rename = os.getcwd() + "/test/IECoreMaya/objectMFnDataParameterIO.ma" )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		connections = cmds.listConnections( parameterPlugPath, plugs=True, connections=True ) or []

		self.failUnless( attrPlugPath in connections )
		self.failUnless( parameterPlugPath in connections )

	def testNonStorableObjectParameter( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "unstorable", 1 )
		op = fnOH.getOp()
		node = fnOH.fullPathName()

		testObj = IECore.CompoundObject( { "someData" : IECore.BoolData( False ) } )

		with fnOH.parameterModificationContext() :
			op["input"].setValue( testObj )

		self.assertEqual( op["input"].getValue(), testObj )

		cmds.file( rename = os.getcwd() + "/test/IECoreMaya/nonStorableObjectParameter.ma" )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		fnPH = IECoreMaya.FnParameterisedHolder( node )
		fnPH.setParameterisedValues()

		op = fnPH.getParameterised()[0]
		self.assertEqual( op["input"].getValue(), op["input"].defaultValue )

	def testMeshParameterIOProblem( self ) :

		fnOP = IECoreMaya.FnOpHolder.create( "merge", "meshMerge", 1 )
		op = fnOP.getOp()

		mesh = IECore.MeshPrimitive.createBox( IECore.Box3f( IECore.V3f( -2, -2, -2 ), IECore.V3f( 2, 3, 4 ) ) )
		op.parameters()["input"].setValue( mesh )
		fnOP.setNodeValues()

		cmds.file( rename = os.getcwd() + "/test/IECoreMaya/meshParameterIO.ma" )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		fnOP = IECoreMaya.FnOpHolder( "merge" )
		fnOP.setParameterisedValues()
		op = fnOP.getOp()

		mesh2 = op.parameters()["input"].getValue()
		self.failUnless( mesh2.arePrimitiveVariablesValid() )
		del mesh2["N"]
		self.assertEqual( mesh2, mesh )

	def testOpHolder( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "maths/multiply", 2 )
		op = fnOH.getOp()

		self.assertEqual( cmds.attributeQuery( "result", node="opHolder", storable=True ), False )
		self.assertEqual( cmds.attributeQuery( "result", node="opHolder", writable=True ), False )

		aPlug = fnOH.parameterPlugPath( op["a"] )
		bPlug = fnOH.parameterPlugPath( op["b"] )

		cmds.setAttr( aPlug, 20 )
		cmds.setAttr( bPlug, 100 )

		self.failUnless( cmds.getAttr( "opHolder.result" ), 2000 )

	def testParameterTypes( self ) :

		node = cmds.createNode( "ieOpHolderNode" )
		fnPH = IECoreMaya.FnParameterisedHolder( node )

		op = IECore.ClassLoader.defaultOpLoader().load( "parameterTypes", 1 )()
		op.parameters().removeParameter( "m" ) # no color4f support in maya

		fnPH.setParameterised( op )

		for parameter in op.parameters().values() :

			self.failUnless( cmds.objExists( fnPH.parameterPlugPath( parameter ) ) )

	def testCompoundObjectConnections( self ) :

		fnOHA = IECoreMaya.FnOpHolder.create( "opA", "compoundObjectInOut", 1 )

		fnOHB = IECoreMaya.FnOpHolder.create( "opB", "compoundObjectInOut", 1 )
		opB = fnOHB.getOp()

		inputPlug = fnOHB.parameterPlugPath( opB["input"] )
		cmds.connectAttr( "opA.result", inputPlug )

		self.assertEqual( cmds.listConnections( inputPlug, source=True, destination=False, plugs=True ), [ "opA.result" ] )
		self.assertEqual( cmds.listConnections( inputPlug, source=False, destination=True, plugs=True ), None )

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "compoundObjectConnections.ma" ) )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		self.assertEqual( cmds.listConnections( inputPlug, source=True, destination=False, plugs=True ), [ "opA.result" ] )
		self.assertEqual( cmds.listConnections( inputPlug, source=False, destination=True, plugs=True ), None )

	def testDefaultConnections( self ) :

		# make an opholder for an op with default connections
		# and make sure they are made.

		fnOH = IECoreMaya.FnOpHolder.create( "opA", "mayaUserData", 1 )
		op = fnOH.getOp()

		tPlug = fnOH.parameterPlugPath( op["t"] )

		self.assertEqual( cmds.listConnections( tPlug, source=True, destination=False, plugs=True, skipConversionNodes=True ), [ "time1.outTime" ] )
		self.assertEqual( cmds.listConnections( tPlug, source=False, destination=True, plugs=True ), None )

		ePlug = fnOH.parameterPlugPath( op["e"] )

		eInputPlugs = cmds.listConnections( ePlug, source=True, destination=False, plugs=True )
		eInputNodes = cmds.listConnections( ePlug, source=True, destination=False )
		self.assertEqual( len( eInputNodes ), 1 )
		self.assertEqual( cmds.nodeType( eInputNodes[0] ), "expression" )

		# save the file

		cmds.file( rename = os.getcwd() + "/test/IECoreMaya/defaultConnections.ma" )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		# load it again and check the connections are still there

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		self.assertEqual( cmds.listConnections( tPlug, source=True, destination=False, plugs=True, skipConversionNodes=True ), [ "time1.outTime" ] )
		self.assertEqual( cmds.listConnections( tPlug, source=False, destination=True, plugs=True ), None )

		eInputNodes = cmds.listConnections( ePlug, source=True, destination=False )
		self.assertEqual( len( eInputNodes ), 1 )
		self.assertEqual( cmds.nodeType( eInputNodes[0] ), "expression" )

		# remove the connections and save

		cmds.disconnectAttr( "time1.outTime", tPlug )
		cmds.disconnectAttr( eInputPlugs[0], ePlug )

		self.assertEqual( cmds.listConnections( tPlug, source=True, destination=False, plugs=True, skipConversionNodes=True ), None )
		self.assertEqual( cmds.listConnections( ePlug, source=True, destination=False, plugs=True, skipConversionNodes=True ), None )

		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		# load again and check they remain disconnected

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		self.assertEqual( cmds.listConnections( tPlug, source=True, destination=False, plugs=True, skipConversionNodes=True ), None )
		self.assertEqual( cmds.listConnections( ePlug, source=True, destination=False, plugs=True, skipConversionNodes=True ), None )

	def testConnectedNodeNameValueProvider( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opA", "mayaUserData", 1 )
		op = fnOH.getOp()

		fnOH.setParameterisedValues()
		self.assertEqual( op["s"].getTypedValue(), "" )

		sPlug = fnOH.parameterPlugPath( op["s"] )
		cmds.connectAttr( "time1.outTime", sPlug )

		fnOH.setParameterisedValues()
		self.assertEqual( op["s"].getTypedValue(), "time1" )

	def testReferencedConnectedNodeNameValueProvider( self ) :

		# Create a scene with a ClassVector parameter containing a StringParameter
		# that uses the "connectedNodeName" value provider, and hook it up.
		##########################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classVectorParameterTest", 2 )
		op = fnOH.getOp()
		c = op["cv"]

		with fnOH.parameterModificationContext() :
			c.setClasses(
				[
					( "mud", "mayaUserData", 1 ),
				]
			)

		plugPath = fnOH.parameterPlugPath( c["mud"]["s"] )

		camera = cmds.createNode( "camera" )
		cmds.connectAttr( "%s.message" % camera, plugPath )

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "connectedNodeReference.ma" ) )
		referenceScene = cmds.file( force = True, type = "mayaAscii", save = True )

		# Reference this scene into a new one, and add another class.
		#############################################################

		cmds.file( new = True, force = True )
		cmds.file( referenceScene, reference = True, namespace = "ns1" )

		fnOH = IECoreMaya.FnOpHolder( "ns1:node" )
		op = fnOH.getOp()
		c = op["cv"]

		with fnOH.parameterModificationContext() :
			c.setClasses(
				[
					( "mud", "mayaUserData", 1 ),
					( "maths", "maths/multiply", 1 )
				]
			)

		plugPath =  fnOH.parameterPlugPath( c["mud"]["s"] )

		self.failUnless( cmds.isConnected( "ns1:%s.message" % camera, plugPath ) )

		# Save, and re-open scene, and make sure that the message connection survived
		#############################################################################

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "connectedNodeReference2.ma" ) )
		thisScene = cmds.file( force = True, type = "mayaAscii", save = True )
		cmds.file( thisScene, open = True, force = True )

		self.failUnless( cmds.isConnected( "ns1:%s.message" % camera, plugPath ) )

	def testClassParameter( self ) :

		class TestOp( IECore.Op ) :

			def __init__( self ) :

				IECore.Op.__init__( self,
					"",
					IECore.FloatParameter(
						"result",
						"",
						0.0
					),
				)

				self.parameters().addParameter(

					IECore.ClassParameter(
						"cp",
						"",
						"IECORE_OP_PATHS"
					)

				)

			def doOperation( self, operands ) :

				return IECore.FloatData( 1 )

		node = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnParameterisedHolder( str( node ) )

		op = TestOp()
		fnOH.setParameterised( op )

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( op["cp"]["a"] )
		bPlugPath = fnOH.parameterPlugPath( op["cp"]["b"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 1 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 2 )

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "stringParsing", 1, "IECORE_OP_PATHS" )

		self.failIf( cmds.objExists( aPlugPath ) )
		self.failIf( cmds.objExists( bPlugPath ) )

		emptyStringPlugPath = fnOH.parameterPlugPath( op["cp"]["emptyString"] )
		self.assertEqual( cmds.getAttr( emptyStringPlugPath ), "notEmpty" )

	def testClassParameterSaveAndLoad( self ) :

		# make an opholder with a ClassParameter, and set the held class
		####################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classParameterTest", 1 )

		op = fnOH.getOp()
		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		# check that maya has appropriate attributes for the held class,
		# and that the held class hasn't changed in the process.
		####################################################################

		heldClass2, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		self.failUnless( heldClass is heldClass2 )

		# change some parameter values and push them into maya.
		####################################################################

		op["cp"]["a"].setNumericValue( 10 )
		op["cp"]["b"].setNumericValue( 20 )
		fnOH.setNodeValues()

		self.assertEqual( cmds.getAttr( fnOH.parameterPlugPath( op["cp"]["a"] ) ), 10 )
		self.assertEqual( cmds.getAttr( fnOH.parameterPlugPath( op["cp"]["b"] ) ), 20 )

		# save the scene
		####################################################################

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "classParameter.ma" ) )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )
		cmds.file( new = True, force = True )

		# reload it and check we have the expected class and attributes
		####################################################################

		cmds.file( scene, open = True )

		fnOH = IECoreMaya.FnOpHolder( "node" )

		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		self.assertEqual( cmds.getAttr( fnOH.parameterPlugPath( op["cp"]["a"] ) ), 10 )
		self.assertEqual( cmds.getAttr( fnOH.parameterPlugPath( op["cp"]["b"] ) ), 20 )

	def testClassParameterUndo( self ) :

		# make an opholder with a ClassParameter, and check that there is
		# no class loaded
		####################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classParameterTest", 1 )
		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass, None )
		self.assertEqual( className, "" )
		self.assertEqual( classVersion, 0 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		# check that undo is enabled
		####################################################################

		self.assert_( cmds.undoInfo( query=True, state=True ) )

		# set the class and verify it worked
		####################################################################

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( heldClass["a"] )
		bPlugPath = fnOH.parameterPlugPath( heldClass["b"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 1 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 2 )

		# undo and check the class is unset
		#####################################################################

		cmds.undo()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )

		self.assertEqual( heldClass, None )
		self.assertEqual( className, "" )
		self.assertEqual( classVersion, 0 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		self.failIf( cmds.objExists( aPlugPath ) )
		self.failIf( cmds.objExists( bPlugPath ) )

	def testClassParameterUndoWithPreviousValues( self ) :

		# make an opholder with a ClassParameter, and check that there is
		# no class loaded
		####################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classParameterTest", 1 )
		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass, None )
		self.assertEqual( className, "" )
		self.assertEqual( classVersion, 0 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		# set the class and check it worked
		####################################################################

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( heldClass["a"] )
		bPlugPath = fnOH.parameterPlugPath( heldClass["b"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 1 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 2 )

		# change some attribute values
		####################################################################

		cmds.setAttr( aPlugPath, 10 )
		cmds.setAttr( bPlugPath, 20 )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

		# check that undo is enabled
		####################################################################

		self.assert_( cmds.undoInfo( query=True, state=True ) )

		# change the class to something else and check it worked
		####################################################################

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "stringParsing", 1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "stringParsing" )
		self.assertEqual( className, "stringParsing" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		plugPaths = []
		for p in heldClass.parameters().values() :

			plugPath = fnOH.parameterPlugPath( p )
			self.failUnless( cmds.objExists( plugPath ) )
			plugPaths.append( plugPath )

		self.failIf( cmds.objExists( aPlugPath ) )
		self.failIf( cmds.objExists( bPlugPath ) )

		# undo and check the previous class reappears, along with the
		# previous attribute values
		#####################################################################

		cmds.undo()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( heldClass["a"] )
		bPlugPath = fnOH.parameterPlugPath( heldClass["b"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

		for p in plugPaths :

			self.failIf( cmds.objExists( plugPath ) )

	def testClassParameterRemovalUndoWithChildren( self ) :

		# make an opholder with a ClassParameter, and check that there is
		# no class loaded
		####################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classParameterTest", 1 )
		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass, None )
		self.assertEqual( className, "" )
		self.assertEqual( classVersion, 0 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		# set the class and check it worked
		####################################################################

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "classParameterTest", 1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "classParameterTest" )
		self.assertEqual( className, "classParameterTest" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		# put a class inside the class and check it worked
		####################################################################

		with fnOH.parameterModificationContext() :
			op["cp"]["cp"].setClass( "maths/multiply", 1, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( op["cp"]["cp"]["a"] )
		bPlugPath = fnOH.parameterPlugPath( op["cp"]["cp"]["b"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 1 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 2 )

		# change some attribute values
		####################################################################

		cmds.setAttr( aPlugPath, 10 )
		cmds.setAttr( bPlugPath, 20 )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

		# check that undo is enabled
		####################################################################

		self.assert_( cmds.undoInfo( query=True, state=True ) )

		# remove the top level class
		####################################################################

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "", -1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass, None )
		self.assertEqual( className, "" )
		self.assertEqual( classVersion, -1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		self.failIf( cmds.objExists( aPlugPath ) )
		self.failIf( cmds.objExists( bPlugPath ) )

		# undo and check the previous class reappears, along with the child
		# class and previous attribute values
		#####################################################################

		cmds.undo()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "classParameterTest" )
		self.assertEqual( className, "classParameterTest" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		childClass, childClassName, childClassVersion, childSearchPath = heldClass["cp"].getClass( True )
		self.assertEqual( childClass.typeName(), "multiply" )
		self.assertEqual( childClassName, "maths/multiply" )
		self.assertEqual( childClassVersion, 1 )
		self.assertEqual( childSearchPath, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( childClass["a"] )
		bPlugPath = fnOH.parameterPlugPath( childClass["b"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

	def testClassParameterReferenceEdits( self ) :

		# make a file with a class parameter with no held class
		#######################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classParameterTest", 1 )
		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass, None )
		self.assertEqual( className, "" )
		self.assertEqual( classVersion, 0 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "classParameterReference.ma" ) )
		referenceScene = cmds.file( force = True, type = "mayaAscii", save = True )

		# make a new scene referencing that file
		#######################################################################

		cmds.file( new = True, force = True )
		cmds.file( referenceScene, reference = True, namespace = "ns1" )

		# set the held class and change some attribute values
		#######################################################################

		fnOH = IECoreMaya.FnOpHolder( "ns1:node" )
		op = fnOH.getOp()

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( heldClass["a"] )
		bPlugPath = fnOH.parameterPlugPath( heldClass["b"] )

		cmds.setAttr( aPlugPath, 10 )
		cmds.setAttr( bPlugPath, 20 )

		# save the scene
		#######################################################################

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "classParameterReferencer.ma" ) )
		referencerScene = cmds.file( force = True, type = "mayaAscii", save = True )

		# reload it and check all is well
		#######################################################################

		cmds.file( new = True, force = True )
		cmds.file( referencerScene, force = True, open = True )

		fnOH = IECoreMaya.FnOpHolder( "ns1:node" )
		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "multiply" )
		self.assertEqual( className, "maths/multiply" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( heldClass["a"] )
		bPlugPath = fnOH.parameterPlugPath( heldClass["b"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

	def testClassParameterReferenceEditsWithFloatParameters( self ) :

		# make a file with a class parameter with no held class
		#######################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classParameterTest", 1 )
		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass, None )
		self.assertEqual( className, "" )
		self.assertEqual( classVersion, 0 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "classParameterReference.ma" ) )
		referenceScene = cmds.file( force = True, type = "mayaAscii", save = True )

		# make a new scene referencing that file
		#######################################################################

		cmds.file( new = True, force = True )
		cmds.file( referenceScene, reference = True, namespace = "ns1" )

		# set the held class and change some attribute values
		#######################################################################

		fnOH = IECoreMaya.FnOpHolder( "ns1:node" )
		op = fnOH.getOp()

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "floatParameter", 1, "IECORE_OP_PATHS" )

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "floatParameter" )
		self.assertEqual( className, "floatParameter" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		fPlugPath = fnOH.parameterPlugPath( heldClass["f"] )

		cmds.setAttr( fPlugPath, -1 )
		self.assertEqual( cmds.getAttr( fPlugPath ), -1 )

		# save the scene
		#######################################################################

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "classParameterReferencer.ma" ) )
		referencerScene = cmds.file( force = True, type = "mayaAscii", save = True )

		# reload it and check all is well
		#######################################################################

		cmds.file( new = True, force = True )
		cmds.file( referencerScene, force = True, open = True )

		fnOH = IECoreMaya.FnOpHolder( "ns1:node" )
		op = fnOH.getOp()

		heldClass, className, classVersion, searchPath = op["cp"].getClass( True )
		self.assertEqual( heldClass.typeName(), "floatParameter" )
		self.assertEqual( className, "floatParameter" )
		self.assertEqual( classVersion, 1 )
		self.assertEqual( searchPath, "IECORE_OP_PATHS" )

		fPlugPath = fnOH.parameterPlugPath( heldClass["f"] )

		self.assertEqual( cmds.getAttr( fPlugPath ), -1 )

	def testClassParameterCompactPlugs( self ) :

		class TestOp( IECore.Op ) :

			def __init__( self ) :

				IECore.Op.__init__( self,
					"",
					IECore.FloatParameter(
						"result",
						"",
						0.0
					),
				)

				self.parameters().addParameter(

					IECore.ClassParameter(
						"cp",
						"",
						"IECORE_OP_PATHS"
					)

				)

			def doOperation( self, operands ) :

				return IECore.FloatData( 1 )

		node = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnParameterisedHolder( str( node ) )

		op = TestOp()
		fnOH.setParameterised( op )

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1, "IECORE_OP_PATHS" )

		aPlugPath = fnOH.parameterPlugPath( op["cp"]["a"] )
		bPlugPath = fnOH.parameterPlugPath( op["cp"]["b"] )
		cpPlugPath = fnOH.parameterPlugPath( op["cp"] )

		self.assertEqual( cmds.getAttr( cpPlugPath ), [ "maths/multiply", "1", "IECORE_OP_PATHS" ] )
		self.failUnless( not cmds.objExists( cpPlugPath + "__className" ) )
		self.failUnless( not cmds.objExists( cpPlugPath + "__classVersion" ) )
		self.failUnless( not cmds.objExists( cpPlugPath + "__searchPathEnvVar" ) )
		self.assertEqual( cmds.getAttr( aPlugPath ), 1 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 2 )

	def testOpHolderImport( self ) :

		# make a file with an op holder in it
		#######################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "maths/multiply", 2 )
		op = fnOH.getOp()

		aPlugPath = fnOH.parameterPlugPath( op["a"] )
		bPlugPath = fnOH.parameterPlugPath( op["b"] )

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "op.ma" ) )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		# import it into a new scene
		#######################################################################

		cmds.file( new = True, force = True )
		cmds.file( scene, i = True )

		cmds.setAttr( aPlugPath, 10 )
		cmds.setAttr( bPlugPath, 12 )

		self.assertEqual( cmds.getAttr( "node.result" ), 120 )

	def testClassVectorParameter( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classVectorParameterTest", 1 )
		op = fnOH.getOp()

		c = op["cv"]
		self.assertEqual( c.typeName(), "ClassVectorParameter" )

		self.assertEqual( len( c.getClasses() ), 0 )

		with fnOH.parameterModificationContext() :

			c.setClasses(

				[
					( "mult", "maths/multiply", 1 ),
					( "coIO", "compoundObjectInOut", 1 ),
				]

			)

		cl = c.getClasses()
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0].typeName(), "multiply" )
		self.assertEqual( cl[1].typeName(), "compoundObjectInOut" )

		self.assertEqual( len( c ), 2 )
		self.assertEqual( c.keys(), [ "mult", "coIO" ] )
		self.assertEqual( c["mult"].keys(), [ "a", "b" ] )
		self.assertEqual( c["coIO"].keys(), [ "input" ] )

		cl = c.getClasses( True )
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[1][0].typeName(), "compoundObjectInOut" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[1][1], "coIO" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[1][2], "compoundObjectInOut" )
		self.assertEqual( cl[0][3], 1 )
		self.assertEqual( cl[1][3], 1 )

		self.__checkAllParameterPlugs( fnOH, c )

	def testClassVectorParameterSaveAndLoad( self ) :

		# make an opholder with a ClassVectorParameter, and modify some plug
		# values
		#####################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classVectorParameterTest", 1 )
		op = fnOH.getOp()

		c = op["cv"]
		self.assertEqual( c.typeName(), "ClassVectorParameter" )

		self.assertEqual( len( c.getClasses() ), 0 )

		with fnOH.parameterModificationContext() :

			c.setClasses(

				[
					( "mult", "maths/multiply", 1 ),
					( "coIO", "compoundObjectInOut", 1 ),
				]

			)

		cl = c.getClasses()
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0].typeName(), "multiply" )
		self.assertEqual( cl[1].typeName(), "compoundObjectInOut" )

		self.__checkAllParameterPlugs( fnOH, c )

		aPlugPath = fnOH.parameterPlugPath( c["mult"]["a"] )
		bPlugPath = fnOH.parameterPlugPath( c["mult"]["b"] )

		cmds.setAttr( aPlugPath, 10 )
		cmds.setAttr( bPlugPath, 20 )

		# save the scene
		####################################################################

		cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "classVectorParameter.ma" ) )
		scene = cmds.file( force = True, type = "mayaAscii", save = True )

		# reload it and check we still have what we expect
		####################################################################

		cmds.file( new = True, force = True )
		cmds.file( scene, open = True )

		fnOH = IECoreMaya.FnOpHolder( "node" )

		op = fnOH.getOp()

		cl = op["cv"].getClasses()
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0].typeName(), "multiply" )
		self.assertEqual( cl[1].typeName(), "compoundObjectInOut" )

		self.__checkAllParameterPlugs( fnOH, op["cv"] )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

	def testClassVectorParameterUndo( self ) :

		# make an opholder and set a ClassVectorParameter
		##########################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classVectorParameterTest", 1 )
		op = fnOH.getOp()

		c = op["cv"]
		self.assertEqual( c.typeName(), "ClassVectorParameter" )

		self.assertEqual( len( c.getClasses() ), 0 )

		self.assert_( cmds.undoInfo( query=True, state=True ) )

		with fnOH.parameterModificationContext() :

			c.setClasses(

				[
					( "mult", "maths/multiply", 1 ),
					( "str", "stringParsing", 1 ),
				]

			)

		cl = c.getClasses()
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0].typeName(), "multiply" )
		self.assertEqual( cl[1].typeName(), "stringParsing" )

		self.assertEqual( len( c ), 2 )
		self.assertEqual( c.keys(), [ "mult", "str" ] )
		self.assertEqual( c["mult"].keys(), [ "a", "b" ] )
		self.assertEqual( c["str"].keys(), [ "emptyString", "normalString", "stringWithSpace", "stringWithManySpaces" ] )

		cl = c.getClasses( True )
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[1][0].typeName(), "stringParsing" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[1][1], "str" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[1][2], "stringParsing" )
		self.assertEqual( cl[0][3], 1 )
		self.assertEqual( cl[1][3], 1 )

		self.__checkAllParameterPlugs( fnOH, c )

		# undo and check we're back to square one
		##########################################################################

		cmds.undo()

		self.assertEqual( c.getClasses(), [] )

	def testClassVectorParameterUndoWithPreviousValues( self ) :

		# make an opholder with a ClassVectorParameter, and modify some plug
		# values
		#####################################################################

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classVectorParameterTest", 1 )
		op = fnOH.getOp()

		c = op["cv"]
		self.assertEqual( c.typeName(), "ClassVectorParameter" )

		self.assertEqual( len( c.getClasses() ), 0 )

		with fnOH.parameterModificationContext() :

			c.setClasses(

				[
					( "mult", "maths/multiply", 1 ),
				]

			)

		cl = c.getClasses( True )
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 1 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[0][3], 1 )

		self.__checkAllParameterPlugs( fnOH, c )

		aPlugPath = fnOH.parameterPlugPath( c["mult"]["a"] )
		bPlugPath = fnOH.parameterPlugPath( c["mult"]["b"] )

		cmds.setAttr( aPlugPath, 10 )
		cmds.setAttr( bPlugPath, 20 )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

		# change set of held classes to something totally different
		# and check that it worked
		#####################################################################

		with fnOH.parameterModificationContext() :

			c.setClasses(

				[
					( "str", "stringParsing", 1 ),
					( "spl", "splineInput", 1 ),
				]

			)

		cl = c.getClasses( True )
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 2 )
		self.assertEqual( cl[0][0].typeName(), "stringParsing" )
		self.assertEqual( cl[1][0].typeName(), "splineInput" )
		self.assertEqual( cl[0][1], "str" )
		self.assertEqual( cl[1][1], "spl" )
		self.assertEqual( cl[0][2], "stringParsing" )
		self.assertEqual( cl[1][2], "splineInput" )
		self.assertEqual( cl[0][3], 1 )
		self.assertEqual( cl[1][3], 1 )

		self.__checkAllParameterPlugs( fnOH, c )

		# undo and check we're back where we want to be
		#####################################################################

		cmds.undo()

		cl = c.getClasses( True )
		self.failUnless( isinstance( cl, list ) )
		self.assertEqual( len( cl ), 1 )
		self.assertEqual( cl[0][0].typeName(), "multiply" )
		self.assertEqual( cl[0][1], "mult" )
		self.assertEqual( cl[0][2], "maths/multiply" )
		self.assertEqual( cl[0][3], 1 )

		self.__checkAllParameterPlugs( fnOH, c )

		self.assertEqual( cmds.getAttr( aPlugPath ), 10 )
		self.assertEqual( cmds.getAttr( bPlugPath ), 20 )

	def testSetParameterisedUndo( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "stringParsing", 1 )
		op = fnOH.getOp()

		self.assertEqual( op.typeName(), "stringParsing" )

		self.__checkAllParameterPlugs( fnOH )

		self.assert_( cmds.undoInfo( query=True, state=True ) )

		fnOH.setOp( "maths/multiply", 1 )

		op = fnOH.getOp()
		self.assertEqual( op.typeName(), "multiply" )
		self.__checkAllParameterPlugs( fnOH )

		cmds.undo()

		op = fnOH.getOp()
		self.assertEqual( op.typeName(), "stringParsing" )
		self.__checkAllParameterPlugs( fnOH )

		cmds.redo()

		op = fnOH.getOp()
		self.assertEqual( op.typeName(), "multiply" )
		self.__checkAllParameterPlugs( fnOH )

	def testCreateOpHolderUndo( self ) :

		self.assert_( cmds.undoInfo( query=True, state=True ) )

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "stringParsing", 1 )

		self.failUnless( cmds.objExists( "opHolder" ) )

		cmds.undo()

		self.failIf( cmds.objExists( "opHolder" ) )

	def testCreateParameterisedHolderSetUndo( self ) :

		self.assert_( cmds.undoInfo( query=True, state=True ) )

		fnOH = IECoreMaya.FnParameterisedHolderSet.create( "mySet", "stringParsing", 1, "IECORE_OP_PATHS" )

		self.failUnless( cmds.objExists( "mySet" ) )

		cmds.undo()

		self.failIf( cmds.objExists( "mySet" ) )

	def testSetParameterisedCallbacks( self ) :

		self.__numCallbacks = 0
		def c( fnPH ) :

			self.assertEqual( fnPH.fullPathName(), "opHolder" )
			self.__numCallbacks += 1

		IECoreMaya.FnParameterisedHolder.addSetParameterisedCallback( c )

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "stringParsing", 1 )
		self.assertEqual( self.__numCallbacks, 1 )

		fnOH.setOp( "maths/multiply", 1 )
		self.assertEqual( self.__numCallbacks, 2 )

		cmds.undo()
		self.assertEqual( self.__numCallbacks, 3 )

		cmds.redo()
		self.assertEqual( self.__numCallbacks, 4 )

		IECoreMaya.FnParameterisedHolder.removeSetParameterisedCallback( c )

	def testSetParameterisedAndUndoOnEmptyHolder( self ) :

		n = cmds.createNode( "ieProceduralHolder" )

		fnPh = IECoreMaya.FnParameterisedHolder( n )
		self.assertEqual( fnPh.getParameterised()[0], None )

		fnPh.setParameterised( "read", "-1", "IECORE_PROCEDURAL_PATHS" )
		self.assertEqual( fnPh.getParameterised()[1], "read" )

		cmds.undo()
		self.assertEqual( fnPh.getParameterised()[0], None )

	def testEditClassParameters( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "classParameterTest", 1 )

		op = fnOH.getOp()

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "classParameterTest", 1 )
			op["cp"]["cp"].setClass( "maths/multiply", 1 )

		self.__checkAllParameterPlugs( fnOH )

		aPlugPath = fnOH.parameterPlugPath( op["cp"]["cp"]["a"] )
		self.failUnless( cmds.objExists( aPlugPath ) )

		cmds.undo()

		self.__checkAllParameterPlugs( fnOH )
		self.assertEqual( op["cp"].getClass(), None )
		self.failIf( cmds.objExists( aPlugPath ) )

		cmds.redo()

		self.__checkAllParameterPlugs( fnOH )

		aPlugPath = fnOH.parameterPlugPath( op["cp"]["cp"]["a"] )
		self.failUnless( cmds.objExists( aPlugPath ) )

	def testChangeClassAndRevertToClassWithClassParameters( self ) :

		## create a holder and put an op using ClassParameter in there

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "classParameterTest", 1 )

		op = fnOH.getOp()

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "classParameterTest", 1 )
			op["cp"]["cp"].setClass( "maths/multiply", 1 )

		self.__checkAllParameterPlugs( fnOH )

		aPlugPath = fnOH.parameterPlugPath( op["cp"]["cp"]["a"] )
		self.failUnless( cmds.objExists( aPlugPath ) )

		## change the values being held

		cmds.setAttr( aPlugPath, 123 )

		## change the op to be something simple
		fnOH.setOp( "maths/multiply", 1 )
		self.failIf( cmds.objExists( aPlugPath ) )

		## undo, and check we get all the original held classes and values back

		cmds.undo()

		op = fnOH.getOp()

		self.assertEqual( op["cp"]["cp"].getClass( True )[1:3], ( "maths/multiply", 1 ) )

		self.assertEqual( op["cp"]["cp"]["a"].getNumericValue(), 123 )

		aPlugPath = fnOH.parameterPlugPath( op["cp"]["cp"]["a"] )
		self.assertEqual( cmds.getAttr( aPlugPath ), 123 )

	def testUpgradeClassWithClassVectorParameter( self ) :

		# create a holder with a ClassVectorParameter with some classes

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "classVectorParameterTest", 1 )

		op = fnOH.getOp()

		with fnOH.parameterModificationContext() :

			op["cv"].setClasses( [
				( "m", "maths/multiply", 1 ),
				( "n", "maths/multiply", 1 ),
			] )

		c = op["cv"].getClasses( True )
		self.assertEqual( len( c ), 2 )
		self.assertEqual( c[0][1:], ( "m", "maths/multiply", 1 ) )
		self.assertEqual( c[1][1:], ( "n", "maths/multiply", 1 ) )

		aPlugPath = fnOH.parameterPlugPath( op["cv"]["m"]["a"] )

		# upgrade the parameterised class

		fnOH.setOp( "classVectorParameterTest", 2 )

		self.assertEqual( fnOH.getParameterised()[1:-1], ( "classVectorParameterTest", 2 ) )

		# and check the classes are still intact

		op = fnOH.getOp()

		c = op["cv"].getClasses( True )
		self.assertEqual( len( c ), 2 )
		self.assertEqual( c[0][1:], ( "m", "maths/multiply", 1 ) )
		self.assertEqual( c[1][1:], ( "n", "maths/multiply", 1 ) )

		# undo the upgrade

		cmds.undo()

		self.assertEqual( fnOH.getParameterised()[1:-1], ( "classVectorParameterTest", 1 ) )

		# and check the classes are still intact again

		op = fnOH.getOp()

		c = op["cv"].getClasses( True )
		self.assertEqual( len( c ), 2 )
		self.assertEqual( c[0][1:], ( "m", "maths/multiply", 1 ) )
		self.assertEqual( c[1][1:], ( "n", "maths/multiply", 1 ) )

	def testClassParameterCallbacks( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "classParameterTest", 1 )
		op = fnOH.getOp()

		self.__numCallbacks = 0
		def c( fnPH, parameter ) :

			self.assertEqual( fnPH.fullPathName(), "opHolder" )
			self.assertEqual( parameter.name, "cp" )
			self.__numCallbacks += 1

		IECoreMaya.FnParameterisedHolder.addSetClassParameterClassCallback( c )

		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1 )
		self.assertEqual( self.__numCallbacks, 1 )

		cmds.undo()
		self.assertEqual( self.__numCallbacks, 2 )

		cmds.redo()
		self.assertEqual( self.__numCallbacks, 3 )

		# setting the class to the same thing it already is should have
		# no effect.
		with fnOH.parameterModificationContext() :
			op["cp"].setClass( "maths/multiply", 1 )
		self.assertEqual( self.__numCallbacks, 3 )

		IECoreMaya.FnParameterisedHolder.removeSetClassParameterClassCallback( c )

	def testClassVectorParameterCallbacks( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "opHolder", "classVectorParameterTest", 1 )
		op = fnOH.getOp()

		self.__numCallbacks = 0
		def c( fnPH, parameter ) :

			self.assertEqual( fnPH.fullPathName(), "opHolder" )
			self.assertEqual( parameter.name, "cv" )
			self.__numCallbacks += 1

		IECoreMaya.FnParameterisedHolder.addSetClassVectorParameterClassesCallback( c )

		with fnOH.parameterModificationContext() :
			op["cv"].setClasses( [
				( "m", "maths/multiply", 1 ),
				( "n", "maths/multiply", 1 ),
			] )
		self.assertEqual( self.__numCallbacks, 1 )

		cmds.undo()
		self.assertEqual( self.__numCallbacks, 2 )

		cmds.redo()
		self.assertEqual( self.__numCallbacks, 3 )

		# setting the class to the same thing it already is should have
		# no effect.
		with fnOH.parameterModificationContext() :
			op["cv"].setClasses( [
				( "m", "maths/multiply", 1 ),
				( "n", "maths/multiply", 1 ),
			] )
		self.assertEqual( self.__numCallbacks, 3 )

		IECoreMaya.FnParameterisedHolder.removeSetClassVectorParameterClassesCallback( c )

	def testClassVectorParameterCompactPlugs( self ) :

		fnOH = IECoreMaya.FnOpHolder.create( "node", "classVectorParameterTest", 1 )
		op = fnOH.getOp()

		c = op["cv"]
		self.assertEqual( c.typeName(), "ClassVectorParameter" )
		self.assertEqual( len( c.getClasses() ), 0 )

		with fnOH.parameterModificationContext() :
			c.setClasses( [
				( "mult", "maths/multiply", 1 ),
				( "coIO", "compoundObjectInOut", 1 ),
			] )

		cPlugPath = fnOH.parameterPlugPath( c )
		self.assertEqual( cmds.getAttr( cPlugPath ), [ "mult", "maths/multiply", "1", "coIO", "compoundObjectInOut", "1" ] )
		self.failUnless( not cmds.objExists( cPlugPath + "__parameterNames" ) )
		self.failUnless( not cmds.objExists( cPlugPath + "__classNames" ) )
		self.failUnless( not cmds.objExists( cPlugPath + "__classVersions" ) )

	def testNumericParameterMinMax( self ) :

		# test no range

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0
			)
		)

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( opNode )
		fnOH.setParameterised( op )

		iPlugPath = fnOH.parameterPlugPath( op["i"] )

		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minExists=True, node=opNode ), False )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maxExists=True, node=opNode ), False )

		# test min only

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0,
				minValue = -10
			)
		)

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( opNode )
		fnOH.setParameterised( op )

		iPlugPath = fnOH.parameterPlugPath( op["i"] )

		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minExists=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minimum=True, node=opNode )[0], -10 )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maxExists=True, node=opNode ), False )

		# test min and max

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0,
				minValue = -10,
				maxValue = 10
			)
		)

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( opNode )
		fnOH.setParameterised( op )

		iPlugPath = fnOH.parameterPlugPath( op["i"] )

		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minExists=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minimum=True, node=opNode )[0], -10 )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maxExists=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maximum=True, node=opNode )[0], 10 )

	def testNumericParameterRangeAdded( self ) :

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0
			)
		)

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder(  opNode )
		fnOH.setParameterised( op )

		iPlugPath = fnOH.parameterPlugPath( op["i"] )

		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minExists=True, node=opNode ), False )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maxExists=True, node=opNode ), False )

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0,
				minValue = -2,
				maxValue = 2,
			)
		)

		fnOH.setParameterised( op )

		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minExists=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minimum=True, node=opNode )[0], -2 )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maxExists=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maximum=True, node=opNode )[0], 2 )

	def testNumericParameterRangeRemoved( self ) :

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0,
				minValue = -2,
				maxValue = 2,
			)
		)

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder(  opNode )
		fnOH.setParameterised( op )

		iPlugPath = fnOH.parameterPlugPath( op["i"] )

		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minExists=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minimum=True, node=opNode )[0], -2 )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maxExists=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maximum=True, node=opNode )[0], 2 )

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0
			)
		)

		fnOH.setParameterised( op )

		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], minExists=True, node=opNode ), False )
		self.assertEqual( cmds.attributeQuery( iPlugPath.rpartition( "." )[-1], maxExists=True, node=opNode ), False )

	def testParameterTypeChanges( self ) :

		"""Test maya attribute type with changing parameters types."""

		n = cmds.createNode( 'ieParameterisedHolderNode' )

		a = IECore.Parameterised( "a" )
		a.parameters().addParameter( IECore.IntParameter( "theParameter", "", 1  ) )

		b = IECore.Parameterised( "b" )
		b.parameters().addParameter( IECore.StringParameter( "theParameter", "", ""  ) )

		c = IECore.Parameterised( "c" )
		c.parameters().addParameter( IECore.FloatParameter( "theParameter", "", 1.0  ) )

		fnPH = IECoreMaya.FnParameterisedHolder( n )

		fnPH.setParameterised( a )
		fnPH.setNodeValues()

		# Check the Maya attribute holds an int.
		plugPath = fnPH.parameterPlugPath( a["theParameter"] )
		cmds.setAttr( plugPath, 2.75 )
		self.assertEqual( cmds.getAttr(plugPath), 3 )

		fnPH.setParameterised( b )
		fnPH.setNodeValues()

		# Should be a string now
		plugPath = fnPH.parameterPlugPath( b["theParameter"] )
		cmds.setAttr( plugPath, "test", type="string" )
		self.assertEqual( cmds.getAttr(plugPath), "test" )

		fnPH.setParameterised( c )
		fnPH.setNodeValues()

		# Should be a float now
		plugPath = fnPH.parameterPlugPath( c["theParameter"] )
		cmds.setAttr( plugPath, 3.75 )
		self.assertEqual( cmds.getAttr(plugPath), 3.75 )

		fnPH.setParameterised( a )
		fnPH.setNodeValues()

		# Should be an int again
		plugPath = fnPH.parameterPlugPath( a["theParameter"] )
		cmds.setAttr( plugPath, 4.75 )
		self.assertEqual( cmds.getAttr(plugPath), 5 )

	def testRemoveLockedAttributes( self ) :

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.IntParameter(
				"i",
				"d",
				0
			)
		)

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( opNode )
		fnOH.setParameterised( op )

		iPlugPath = fnOH.parameterPlugPath( op["i"] )
		cmds.setAttr( iPlugPath, lock=True )

		del op.parameters()["i"]
		fnOH.setParameterised( op )

		self.failIf( cmds.objExists( iPlugPath ) )

	def testRemoveLockedChildAttributes( self ) :

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameter(
			IECore.V3fParameter(
				"v",
				"d",
				IECore.V3f( 0 ),
			)
		)

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( opNode )
		fnOH.setParameterised( op )

		vPlugPath = fnOH.parameterPlugPath( op["v"] )
		cmds.setAttr( vPlugPath + "X", lock=True )

		del op.parameters()["v"]
		fnOH.setParameterised( op )

		self.failIf( cmds.objExists( vPlugPath ) )

	def testStorable( self ) :

		op = IECore.Op( "", IECore.IntParameter( "result", "", 0 ) )
		op.parameters().addParameters( [

			IECore.BoolParameter(
				name = "a",
				description = "",
				defaultValue = True,
			),
			IECore.IntParameter(
				name = "b",
				description = "",
				defaultValue = 1,
				userData = IECore.CompoundObject( { "maya" : { "storable" : IECore.BoolData( False ) } } )
			),
			IECore.StringParameter(
				name = "c",
				description = "",
				defaultValue = "",
				userData = IECore.CompoundObject( { "maya" : { "storable" : IECore.BoolData( True ) } } )
			),

		] )

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( opNode )
		fnOH.setParameterised( op )

		self.assertEqual( cmds.attributeQuery( fnOH.parameterPlugPath( op["a"] ).split( "." )[-1], storable=True, node=opNode ), True )
		self.assertEqual( cmds.attributeQuery( fnOH.parameterPlugPath( op["b"] ).split( "." )[-1], storable=True, node=opNode ), False )
		self.assertEqual( cmds.attributeQuery( fnOH.parameterPlugPath( op["c"] ).split( "." )[-1], storable=True, node=opNode ), True )

		with fnOH.parameterModificationContext() :
			op["a"].userData()["maya"] = IECore.CompoundObject( { "storable" : IECore.BoolData( False ) } )
			op["b"].userData()["maya"]["storable"]  = IECore.BoolData( True )

		self.assertEqual( cmds.attributeQuery( fnOH.parameterPlugPath( op["a"] ).split( "." )[-1], storable=True, node=opNode ), False )
		self.assertEqual( cmds.attributeQuery( fnOH.parameterPlugPath( op["b"] ).split( "." )[-1], storable=True, node=opNode ), True )

	def testBadArgsDoNotSegFault( self ) :

		opNode = cmds.createNode( "ieOpHolderNode" )
		fnOH = IECoreMaya.FnOpHolder( opNode )
		self.assertRaises( RuntimeError, IECore.curry( fnOH.setOp, "fake", -1 ) )

	def testShouldSave( self ) :

		class TestProcedural( IECore.ParameterisedProcedural ) :

			def __init__( self ) :

				IECore.ParameterisedProcedural.__init__( self, "" )

				self.parameters().addParameter(

					IECore.V3fParameter(
						"halfSize",
						"",
						IECore.V3f( 0 )
					)

				)

			def doBound( self, args ) :

				return IECore.Box3f( -args["halfSize"].value, args["halfSize"].value )

			def doRenderState( self, args ) :

				pass

			def doRender( self, args ) :

				pass

		node = cmds.createNode( "ieProceduralHolder" )
		fnPH = IECoreMaya.FnParameterisedHolder( str( node ) )
		p = TestProcedural()
		fnPH.setParameterised( p )

		cmds.setAttr( node + ".nodeState", 4 )


		# Save the scene out so we can reference it
		filename = os.path.join( os.getcwd(), "test", "IECoreMaya", "shouldSaveAttributes.ma")
		cmds.file( rename = filename )
		referenceScene = cmds.file( force = True, type = "mayaAscii", save = True )

		mayaFile = open( filename, 'r' )
		setAttrs = mayaFile.read().partition("createNode ieProceduralHolder")[2].partition("createNode")[0].split("\n")[1:]
		splitAttrs = [i.split('"') for i in setAttrs if "setAttr" in i]
		savedAttrNames = [ i[1] for i in splitAttrs if len(i) >= 2]
		mayaFile.close()

		self.assertTrue( ".nds" in savedAttrNames ) # Check that the nodeState attr we changed has been written
		self.assertTrue( not ".ihi" in savedAttrNames ) # Check that the isHistoricallyInteresting parm that is left default is not exported

		# Parm parameters are always saved, even when left default ( for backwards compatibility reasons )
		self.assertTrue( ".parm_halfSize" in savedAttrNames, msg = " ".join( savedAttrNames ) ) # This test can be removed if we decide our parameters don't require a special case


	def tearDown( self ) :

		for f in [
			"test/IECoreMaya/op.ma" ,
			"test/IECoreMaya/defaultConnections.ma" ,
			"test/IECoreMaya/compoundObjectConnections.ma" ,
			"test/IECoreMaya/reference.ma" ,
			"test/IECoreMaya/referenceMaster.ma",
			"test/IECoreMaya/classParameterReference.ma" ,
			"test/IECoreMaya/classParameterReferencer.ma" ,
			"test/IECoreMaya/objectParameterIO.ma",
			"test/IECoreMaya/objectMFnDataParameterIO.ma",
			"test/IECoreMaya/imageProcedural.ma",
			"test/IECoreMaya/classParameter.ma",
			"test/IECoreMaya/classVectorParameter.ma",
			"test/IECoreMaya/nonStorableObjectParameter.ma",
			"test/IECoreMaya/connectedNodeReference.ma",
			"test/IECoreMaya/connectedNodeReference2.ma",
			"test/IECoreMaya/meshParameterIO.ma",
			"test/IECoreMaya/shouldSaveAttributes.ma",
		] :

			if os.path.exists( f ) :

				os.remove( f )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
