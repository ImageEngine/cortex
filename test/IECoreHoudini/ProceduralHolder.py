##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

class TestProceduralHolder( IECoreHoudini.TestCase ):

	def testProceduralHolder(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder" )
		self.assert_( proc )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		self.assert_( fn )
		return fn

	def testLoadProcedural(self):
		fn = self.testProceduralHolder()
		cl = IECore.ClassLoader.defaultProceduralLoader().load( "sphereProcedural", 0 )()
		fn.setParameterised( cl )
		self.assertNotEqual( fn.getParameterised(), None )
		self.assertEqual( fn.getParameterised(), cl )
		return fn

	# tests creation within contexts (simulating from UIs)
	def testContextCreator( self ) :
		# test generic creation
		n = IECoreHoudini.FnProceduralHolder.create( "test", "parameterTypes" )
		self.assertEqual( n.path(), "/obj/test/test" )
		
		# test contextArgs outside UI mode fallback to generic behaviour
		contextArgs = { "toolname" : "ieProceduralHolder" }
		n2 = IECoreHoudini.FnProceduralHolder.create( "test", "parameterTypes", contextArgs=contextArgs )
		self.assertEqual( n2.path(), "/obj/test1/test" )
		
		# test parent arg
		geo = hou.node( "/obj" ).createNode( "geo", run_init_scripts=False )
		n3 = IECoreHoudini.FnProceduralHolder.create( "test", "parameterTypes", parent=geo, contextArgs=contextArgs )
		self.assertEqual( n3.path(), "/obj/geo1/test" )
		
		# test automatic conversion
		contextArgs["shiftclick"] = True
		n4 = IECoreHoudini.FnProceduralHolder.create( "test", "parameterTypes", parent=geo, contextArgs=contextArgs )
		self.assertEqual( n4.path(), "/obj/geo1/test1" )
		self.assertEqual( len(n4.outputConnectors()[0]), 1 )
		self.assertEqual( n4.outputConnectors()[0][0].outputNode().type().name(), "ieCortexConverter" )
		
		# test automatic conversion and output connections
		mountain = geo.createNode( "mountain" )
		contextArgs["outputnodename"] = mountain.path()
		n5 = IECoreHoudini.FnOpHolder.create( "test", "parameterTypes", parent=geo, contextArgs=contextArgs )
		self.assertEqual( n5.path(), "/obj/geo1/test2" )
		self.assertEqual( len(n5.outputConnectors()[0]), 1 )
		converter = n5.outputConnectors()[0][0].outputNode()
		self.assertEqual( converter.type().name(), "ieCortexConverter" )
		self.assertEqual( len(converter.outputConnectors()[0]), 1 )
		outputNode = converter.outputConnectors()[0][0].outputNode()
		self.assertEqual( outputNode.type().name(), "mountain" )
		self.assertEqual( outputNode, mountain )
	
	def testProceduralParameters(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder" )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		fn.setProcedural( "parameterTypes", 1 )

		# set a lot of parameters via houdini
		proc.parmTuple("parm_a").set( [123] )
		proc.parmTuple("parm_d").set( ["hello"] )
		proc.parmTuple("parm_g").set( (2,4) )
		proc.parmTuple("parm_h").set( (1,4,8) )
		proc.parmTuple("parm_i").set( (2,4) )
		proc.parmTuple("parm_i_3").set( (1, 2, 3, 4,
										5, 6, 7, 8,
										9, 10, 11, 12,
										13, 14, 15, 16 ) )
		proc.parmTuple("parm_i_4").set( (1, 2, 3, 4,
										5, 6, 7, 8,
										9, 10, 11, 12,
										13, 14, 15, 16 ) )
		proc.parmTuple("parm_compound_j").set( (1,4,8) )
		proc.parmTuple("parm_compound_k").set( (1, 2, 3, 4,
										5, 6, 7, 8,
										9, 10, 11, 12,
										13, 14, 15, 16 ) )
		proc.parmTuple("parm_l").set( (1,0,0) )
		proc.parmTuple("parm_m").set( (1,1,0,1) )
		proc.parmTuple("parm_o").set( ["myFile.tif"] )
		proc.parmTuple("parm_p").set( [os.getcwd()] )
		proc.parmTuple("parm_q").set( [True] )
		proc.parmTuple("parm_r").set( ["mySequence.####.tif"] )
		proc.parmTuple("parm_s").set( [-1, -2, 10, 20] )
		proc.parmTuple("parm_s_1").set( [-1, -2, 10, 20] )
		proc.parmTuple("parm_s_2").set( [-1, -2, -3, 10, 20, 30] )
		proc.parmTuple("parm_t").set( [-1, -2, -3, 10, 20, 30] )
		proc.parmTuple("parm_u").set( (64, 128) )
		proc.parmTuple("parm_v").set( (25,26,27) )

		# flush our parameters through to our parameterised procedural
		proc.cook(force=True)

		# generate our bounds
		parameterised = fn.getParameterised()
		self.failUnless( parameterised.isInstanceOf( IECore.TypeId.RunTimeTyped ) )
		box = parameterised.bound()
		self.assertEqual( box, IECore.Box3f( IECore.V3f(0,0,0), IECore.V3f(1,1,1) ) )
		return ( proc, parameterised )

	def testLotsQuickly(self):
		n = []
		for i in range(1000):
			n.append( IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 ) )
		for _n in n:
			_n.destroy()

	def testSaveAndLoad(self):
		save_file = "test/proceduralHolder_testData/proceduralSave_test.hip"

		# create a few procedurals
		n = []
		for i in range( 10 ):
			n.append( IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 ) )
		for i in range( 10 ):
			n.append( IECoreHoudini.FnProceduralHolder.create( "cortex_params", "parameterTypes", 1 ) )

		# set some values
		path1 = n[0].path()
		n[0].parm("parm_radius").set(10)
		n[0].parm("parm_theta").set(90)
		path2 = n[9].path()
		n[9].parm("parm_radius").set(5)
		n[9].parm("parm_theta").set(45)

		# save scene
		hou.hipFile.save(save_file)

		# new scene
		hou.hipFile.clear(suppress_save_prompt=True)

		# open scene
		hou.hipFile.load(save_file)

		# check parameters
		proc = hou.node(path1)
		self.failUnless( proc )
		self.assertEqual( proc.evalParm( "__className" ), "sphereProcedural" )
		self.assertEqual( proc.evalParm( "__classVersion" ), "1" )
		self.assertEqual( proc.evalParm("parm_radius"), 10 )
		self.assertEqual( proc.evalParm("parm_theta"), 90 )
		proc = hou.node(path2)
		self.failUnless( proc )
		self.assertEqual( proc.evalParm( "__className" ), "sphereProcedural" )
		self.assertEqual( proc.evalParm( "__classVersion" ), "1" )
		self.assertEqual( proc.evalParm("parm_radius"), 5 )
		self.assertEqual( proc.evalParm("parm_theta"), 45 )

	def testObjectWasDeleted(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		proc = geo.createNode( "ieProceduralHolder" )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		cl = IECore.ClassLoader.defaultProceduralLoader().load( "sphereProcedural", 1 )()
		proc.destroy()
		self.assertEqual( fn.hasParameterised(), False )
		fn.setParameterised(cl)

	def testProceduralReloadParameters(self):
		sphere = IECoreHoudini.FnProceduralHolder.create( "cortex_sphere", "sphereProcedural", 1 )

		# check the reload button doesn't clear expressions
		sphere.parm("parm_radius").setExpression("sin($FF)")
		hou.setFrame(0)
		rad = sphere.evalParm("parm_radius")
		self.assert_( rad > 0 )
		hou.setFrame(100)
		rad = sphere.evalParm("parm_radius")
		self.assert_( rad > 0.984 )
		self.assert_( rad < 0.985 )
		sphere.parm( "__classReloadButton" ).pressButton()
		rad = sphere.evalParm("parm_radius")
		self.assert_( rad > 0.984 )
		self.assert_( rad < 0.985 )
		self.assertEqual( sphere.parm("parm_radius").expression(), "sin($FF)" )
		hou.setFrame(0)
		rad = sphere.evalParm("parm_radius")
		self.assert_( rad > 0 )

		# now change the version to v2 and check things are still ok
		sphere.parm( "__classVersion" ).set( "2" )
		# if we're changing the menu programatically then we need to call pressButton()!!
		sphere.parm( "__classVersion" ).pressButton()
		self.assert_( not sphere.evalParm("parm_extra") )
		sphere.parm("parm_extra").set(True)
		self.failUnless( sphere.evalParm("parm_extra") )
		rad = sphere.evalParm("parm_radius")
		self.assert_( rad < 0.015 )
		hou.setFrame(100)
		rad = sphere.evalParm("parm_radius")
		self.assert_( rad > 0.984 )
		self.assert_( rad < 0.985 )

	def testHiddenParameters( self ):
		( proc, cl ) = self.testProceduralParameters()
		# check the hidden userData works
		self.assertEqual( proc.parmTuple("parm_a").parmTemplate().isHidden(), True )
		self.assertEqual( proc.parmTuple("parm_b").parmTemplate().isHidden(), False )
		self.assertEqual( proc.parmTuple("parm_c").parmTemplate().isHidden(), True )
		self.assertEqual( proc.parmTuple("parm_d").parmTemplate().isHidden(), False )
		# check setting the parameter still works
		proc.parmTuple("parm_a").set( [123] )
		proc.cook(force=True)
		self.assertEqual( cl['a'].getValue().value, 123 )
		
	def testParameterLabels( self ):
		( proc, cl ) = self.testProceduralParameters()
		# check the hidden userData works
		self.assertEqual( proc.parmTuple("parm_a").parmTemplate().label(), "Int" )
		self.assertEqual( proc.parmTuple("parm_b").parmTemplate().label(), "B" )
		self.assertEqual( proc.parmTuple("parm_c").parmTemplate().label(), "Double" )
		self.assertEqual( proc.parmTuple("parm_d").parmTemplate().label(), "D" )
		
	def testMatchString(self):
		(op,fn)=self.testProceduralParameters()
		fn = IECoreHoudini.FnProceduralHolder(op)
		self.assertEqual( op.parm( "__classMatchString" ).eval(), "*" )
		op.parm( "__className" ).set( "sphereProcedural" )
		op.parm( "__className" ).pressButton()
		cl = fn.getParameterised()
		self.assertEqual( cl.typeName(), "sphereProcedural" )
		op.parm( "__classMatchString" ).set( "nestedChild" )
		results = fn.classNames()
		self.assertEqual( len(fn.classNames()), 1 )
		op.parm( "__className" ).set( "sphereProcedural" ) # this still works, should it be invalid?
		op.parm( "__className" ).pressButton()
		cl = fn.getParameterised()
		self.assertEqual( cl.typeName(), "sphereProcedural" )
		op.parm( "__classMatchString" ).set( "*" )
		self.assert_( len(fn.classNames()) > 1 )
	
	def createProcedural( self, path="primitiveParameters/multiple", version=1 ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		proc = geo.createNode( "ieProceduralHolder" )
		fn = IECoreHoudini.FnProceduralHolder( proc )
		fn.setProcedural( path, version )
		
		return ( proc, fn )
	
	def testObjectParameterConversion( self ) :
		( proc, fn ) = self.createProcedural()
		torus = proc.createInputNode( 2, "torus" )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		converterSop = proc.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.Group )
		self.assertEqual( result.children()[0].typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.children()[0].numFaces(), 100 )
		self.assertEqual( result.children()[1].typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.children()[1].numPoints, 0 )
		
		torus.parm( "type" ).set( 1 )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.Group )
		self.assertEqual( result.children()[0].typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.children()[0].numFaces(), 100 )
		self.assertEqual( result.children()[1].typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.children()[1].numPoints, 0 )
	
	def testObjectParameterWithMultipleTypesConversion( self ) :
		( proc, fn ) = self.createProcedural()
		torus = proc.createInputNode( 3, "torus" )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		converterSop = proc.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.Group )
		self.assertEqual( result.children()[0].typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.children()[0].numFaces(), 100 )
		self.assertEqual( result.children()[1].typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.children()[1].numPoints, 0 )
		
		torus.parm( "type" ).set( 1 )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.numPoints, 100 )
	
	def testPointsParameterConversion( self ) :
		( proc, fn ) = self.createProcedural()
		torus = proc.createInputNode( 1, "torus" )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		converterSop = proc.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.numPoints, 100 )
		
		torus.parm( "type" ).set( 1 )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.numPoints, 100 )
	
	def testMeshParameterConversion( self ) :
		( proc, fn ) = self.createProcedural( "primitiveParameters/meshRender" )
		torus = proc.createInputNode( 0, "torus" )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		converterSop = proc.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.numFaces(), 100 )
		
		torus.parm( "type" ).set( 1 )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.numFaces(), 100 )
	
	# test an proceduralHolder with multiple inputs
	def testMultipleInputs( self ) :
		( proc, fn ) = self.createProcedural()
		torus = proc.createInputNode( 0, "torus" )
		box = proc.createInputNode( 2, "box" )
		torus2 = proc.createInputNode( 3, "torus" )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		converterSop = proc.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 208 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.Group )
		self.assertEqual( result.children()[0].typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.children()[0].numFaces(), 206 )
		self.assertEqual( result.children()[1].typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.children()[1].numPoints, 0 )
		
		torus2.parm( "type" ).set( 1 )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		self.assertEqual( len(converterSop.geometry().points()), 208 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.Group )
		self.assertEqual( result.children()[0].typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.children()[0].numFaces(), 106 )
		self.assertEqual( result.children()[0]["P"].data.size(), 108 )
		self.assertEqual( result.children()[1].typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.children()[1].numPoints, 100 )
	
	# test using op holders and procedural holders as inputs
	def testCortexInputs( self ) :
		( proc, fn ) = self.createProcedural()
		torus = proc.parent().createNode( "torus" )
		op = torus.createOutputNode( "ieOpHolder" )
		IECoreHoudini.FnOpHolder( op ).setOp( "objectDebug", 1 )
		op.parm( "parm_quiet" ).set( True )
		proc.setInput( 0, op )
		box = proc.createInputNode( 2, "box" )
		proc2 = proc.createInputNode( 3, "ieProceduralHolder" )
		fn2 = IECoreHoudini.FnProceduralHolder( proc2 )
		fn2.setProcedural( "primitiveParameters/meshRender", 1 )
		torus2 = proc2.createInputNode( 0, "torus" )
		proc.cook()
		self.assertEqual( proc.errors(), "" )
		self.assertEqual( proc2.errors(), "" )
		self.assertEqual( op.errors(), "" )
		self.assertEqual( len(proc.geometry().points()), 1 )
		self.assertEqual( len(proc2.geometry().points()), 1 )
		self.assertEqual( len(op.geometry().points()), 1 )
		converterSop = op.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.numFaces(), 100 )
		converterSop = proc2.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 100 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.numFaces(), 100 )
		converterSop = proc.createOutputNode( "ieCortexConverter" )
		self.assertEqual( len(converterSop.geometry().points()), 208 )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( converterSop ).convert()
		self.assertEqual( result.typeId(), IECore.TypeId.Group )
		self.assertEqual( result.children()[0].typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.children()[0].numFaces(), 206 )
		self.assertEqual( result.children()[1].typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result.children()[1].numPoints, 0 )
	
	def testAnimatedValues( self ) :
		
		sphere = IECoreHoudini.FnProceduralHolder.create( "test", "sphereProcedural", 1 )
		fn = IECoreHoudini.FnProceduralHolder( sphere )
		sphere.parm( "parm_radius" ).setExpression( "$FF" )
		hou.setFrame( 1 )
		self.assertEqual( sphere.evalParm( "parm_radius" ), 1 )
		self.assertEqual( fn.getProcedural().parameters()["radius"].getTypedValue(), 1 )
		hou.setFrame( 12.25 )
		self.assertEqual( sphere.evalParm( "parm_radius" ), 12.25  )
		# values haven't been flushed yet
		self.assertAlmostEqual( fn.getProcedural().parameters()["radius"].getTypedValue(), 1 )
		# so we flush them
		fn.setParameterisedValues()
		self.assertAlmostEqual( fn.getProcedural().parameters()["radius"].getTypedValue(), 12.25 )
	
	def testNameFilter( self ) :
		
		meshRender = IECoreHoudini.FnProceduralHolder.create( "meshRender", "primitiveParameters/meshRender", 1 )
		
		boxA = meshRender.parent().createNode( "box" )
		nameA = boxA.createOutputNode( "name" )
		nameA.parm( "name1" ).set( "boxA" )
		
		boxB = meshRender.parent().createNode( "box" )
		transformB = boxB.createOutputNode( "xform" )
		transformB.parm( "tx" ).set( 5 )
		nameB = transformB.createOutputNode( "name" )
		nameB.parm( "name1" ).set( "boxB" )
		
		boxC = meshRender.parent().createNode( "box" )
		transformC = boxC.createOutputNode( "xform" )
		transformC.parm( "tx" ).set( 10 )
		nameC = transformC.createOutputNode( "name" )
		nameC.parm( "name1" ).set( "boxC" )
		
		merge = meshRender.parent().createNode( "merge" )
		merge.setInput( 0, nameA )
		merge.setInput( 1, nameB )
		merge.setInput( 2, nameC )
		meshRender.setInput( 0, merge )
		
		# converts all 3 meshes as one (because the parameter type forces it)
		geo = meshRender.geometry()
		self.assertEqual( len(geo.prims()), 1 )
		self.assertEqual( geo.prims()[0].type(), hou.primType.Custom )
		self.assertEqual( meshRender.errors(), "" )
		self.assertEqual( meshRender.warnings(), "" )
		proc = IECoreHoudini.FromHoudiniGeometryConverter.create( meshRender ).convert()
		self.assertTrue( proc.isInstanceOf( IECore.TypeId.ParameterisedProcedural ) )
		self.assertEqual( proc.bound(), IECore.Box3f( IECore.V3f( -0.5, -0.5, -0.5 ), IECore.V3f( 10.5, 0.5, 0.5 ) ) )
		
		# setting to one name limits the bounds
		meshRender.parm( "parm_mesh_nameFilter" ).set( "boxB" )
		self.assertEqual( len(geo.prims()), 1 )
		self.assertEqual( geo.prims()[0].type(), hou.primType.Custom )
		self.assertEqual( meshRender.errors(), "" )
		self.assertEqual( meshRender.warnings(), "" )
		proc = IECoreHoudini.FromHoudiniGeometryConverter.create( meshRender ).convert()
		self.assertTrue( proc.isInstanceOf( IECore.TypeId.ParameterisedProcedural ) )
		self.assertEqual( proc.bound(), IECore.Box3f( IECore.V3f( 4.5, -0.5, -0.5 ), IECore.V3f( 5.5, 0.5, 0.5 ) ) )
		
		# setting to multiple names expands the bounds, but not all the way
		meshRender.parm( "parm_mesh_nameFilter" ).set( "* ^boxA" )
		self.assertEqual( len(geo.prims()), 1 )
		self.assertEqual( geo.prims()[0].type(), hou.primType.Custom )
		self.assertEqual( meshRender.errors(), "" )
		self.assertEqual( meshRender.warnings(), "" )
		proc = IECoreHoudini.FromHoudiniGeometryConverter.create( meshRender ).convert()
		self.assertTrue( proc.isInstanceOf( IECore.TypeId.ParameterisedProcedural ) )
		self.assertEqual( proc.bound(), IECore.Box3f( IECore.V3f( 4.5, -0.5, -0.5 ), IECore.V3f( 10.5, 0.5, 0.5 ) ) )
		
		# multiple CortexObjects cause warnings (because the parameter wants one mesh only)
		converter = merge.createOutputNode( "ieCortexConverter" )
		converter.parm( "resultType" ).set( 0 ) # Cortex
		meshRender.setInput( 0, converter )
		meshRender.parm( "__classReloadButton" ).pressButton() # clear the procedural parm values
		self.assertEqual( len(geo.prims()), 1 )
		self.assertEqual( geo.prims()[0].type(), hou.primType.Custom )
		self.assertEqual( meshRender.errors(), "" )
		self.assertNotEqual( meshRender.warnings(), "" )
		proc = IECoreHoudini.FromHoudiniGeometryConverter.create( meshRender ).convert()
		self.assertTrue( proc.isInstanceOf( IECore.TypeId.ParameterisedProcedural ) )
		self.assertEqual( proc.bound(), IECore.MeshPrimitive().bound() )
		
		# a single CortexObject will work fine
		meshRender.parm( "parm_mesh_nameFilter" ).set( "boxB" )
		self.assertEqual( len(geo.prims()), 1 )
		self.assertEqual( geo.prims()[0].type(), hou.primType.Custom )
		self.assertEqual( meshRender.errors(), "" )
		self.assertEqual( meshRender.warnings(), "" )
		proc = IECoreHoudini.FromHoudiniGeometryConverter.create( meshRender ).convert()
		self.assertTrue( proc.isInstanceOf( IECore.TypeId.ParameterisedProcedural ) )
		self.assertEqual( proc.bound(), IECore.Box3f( IECore.V3f( 4.5, -0.5, -0.5 ), IECore.V3f( 5.5, 0.5, 0.5 ) ) )
		
		# disabling the nameFilter brings the warnings back
		meshRender.setInput( 0, converter )
		meshRender.parm( "parm_mesh_useNameFilter" ).set( False )
		meshRender.parm( "__classReloadButton" ).pressButton() # clear the procedural parm values
		self.assertEqual( len(geo.prims()), 1 )
		self.assertEqual( geo.prims()[0].type(), hou.primType.Custom )
		self.assertEqual( meshRender.errors(), "" )
		self.assertNotEqual( meshRender.warnings(), "" )
		proc = IECoreHoudini.FromHoudiniGeometryConverter.create( meshRender ).convert()
		self.assertTrue( proc.isInstanceOf( IECore.TypeId.ParameterisedProcedural ) )
		self.assertEqual( proc.bound(), IECore.MeshPrimitive().bound() )
	
	def setUp( self ) :
		IECoreHoudini.TestCase.setUp( self )
		if not os.path.exists( "test/proceduralHolder_testData" ):
			os.mkdir( "test/proceduralHolder_testData" )

	def tearDown( self ) :
                if os.path.exists( "test/proceduralHolder_testData" ):
			shutil.rmtree( "test/proceduralHolder_testData" )

if __name__ == "__main__":
    unittest.main()
