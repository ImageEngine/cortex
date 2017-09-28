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

class TestOpHolder( IECoreHoudini.TestCase ):

	def testOpHolder(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		op = geo.createNode( "ieOpHolder" )
		self.assert_( op )
		fn = IECoreHoudini.FnOpHolder( op )
		self.assert_( fn )
		return (op,fn)

	# tests a basic op, the function set and that it cooks as expected
	def testSimpleOp(self):
		(op, fn) = self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised( cl )
		self.assertNotEqual( fn.getParameterised(), None )
		self.assertEqual( fn.getParameterised(), cl )
		op.parm("parm_filename").set( self.__torusTestFile )
		op.cook() # cook using Houdini's cook mechanism, NOT operate()
		self.assertEqual( fn.getParameterised()["filename"].getValue(), IECore.StringData( self.__torusTestFile ) )
		result = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( result, IECore.Reader.create( self.__torusTestFile ).read() )

	# tests the alternative 'all in one' opHolder creator
	def testAlternateCreator(self):
		n = IECoreHoudini.FnOpHolder.create( "noise_deformer", "noiseDeformer", 1 )
		self.assert_( n )
		fn = IECoreHoudini.FnOpHolder( n )
		self.assert_( fn )
		op = fn.getParameterised()
		self.assert_( op )
		self.assertEqual( op.typeName(), "noiseDeformer" )

	# tests creation within contexts (simulating from UIs)
	def testContextCreator( self ) :
		# test generic creation
		n = IECoreHoudini.FnOpHolder.create( "vectorMaker", "vectors/V3fVectorCreator" )
		self.assertEqual( n.path(), "/obj/vectorMaker/vectorMaker" )

		# test contextArgs outside UI mode fallback to generic behaviour
		contextArgs = { "toolname" : "ieOpHolder" }
		n2 = IECoreHoudini.FnOpHolder.create( "vectorMaker", "vectors/V3fVectorCreator", contextArgs=contextArgs )
		self.assertEqual( n2.path(), "/obj/vectorMaker1/vectorMaker" )

		# test parent arg
		geo = hou.node( "/obj" ).createNode( "geo", run_init_scripts=False )
		n3 = IECoreHoudini.FnOpHolder.create( "vectorMaker", "vectors/V3fVectorCreator", parent=geo, contextArgs=contextArgs )
		self.assertEqual( n3.path(), "/obj/geo1/vectorMaker" )

		# test automatic conversion
		contextArgs["shiftclick"] = True
		n4 = IECoreHoudini.FnOpHolder.create( "noise", "noiseDeformer", parent=geo, contextArgs=contextArgs )
		self.assertEqual( n4.path(), "/obj/geo1/noise" )
		self.assertEqual( len(n4.outputConnectors()[0]), 1 )
		self.assertEqual( n4.outputConnectors()[0][0].outputNode().type().name(), "ieCortexConverter" )

		# test automatic conversion and output connections
		mountain = geo.createNode( "mountain" )
		contextArgs["outputnodename"] = mountain.path()
		n5 = IECoreHoudini.FnOpHolder.create( "noise", "noiseDeformer", parent=geo, contextArgs=contextArgs )
		self.assertEqual( n5.path(), "/obj/geo1/noise1" )
		self.assertEqual( len(n5.outputConnectors()[0]), 1 )
		converter = n5.outputConnectors()[0][0].outputNode()
		self.assertEqual( converter.type().name(), "ieCortexConverter" )
		self.assertEqual( len(converter.outputConnectors()[0]), 1 )
		outputNode = converter.outputConnectors()[0][0].outputNode()
		self.assertEqual( outputNode.type().name(), "mountain::2.0" if hou.applicationVersion()[0] >= 16 else "mountain" )
		self.assertEqual( outputNode, mountain )

	# test that a C++ op can be assigned using the function set
	def testCppOp(self):
		(op,fn) = self.testOpHolder()
		mesh_normals = IECore.MeshNormalsOp()
		self.assert_( mesh_normals )
		fn.setParameterised(mesh_normals)
		self.assertEqual( fn.getParameterised().typeName(), "MeshNormalsOp" )

	# test that we can wire opholders together
	def testWireTogether(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)

		v1_op = geo.createNode( "ieOpHolder", node_name="vector1" )
		fn = IECoreHoudini.FnOpHolder( v1_op )
		fn.setOp( "vectors/V3fVectorCreator", 1 )
		v1_op.parm("parm_size").set(3)
		v1_op.parmTuple("parm_value").set( (1,2,3) )

		v2_op = geo.createNode( "ieOpHolder", node_name="vector2" )
		fn = IECoreHoudini.FnOpHolder( v2_op )
		fn.setOp( "vectors/V3fVectorCreator", 1 )
		v2_op.parm("parm_size").set(3)
		v2_op.parmTuple("parm_value").set( (4,5,6) )

		add_op = geo.createNode( "ieOpHolder", node_name="add_vectors" )
		fn = IECoreHoudini.FnOpHolder( add_op )
		fn.setOp( "vectors/V3fVectorAdder", 1 )

		print_op = geo.createNode( "ieOpHolder", node_name="print_values" )
		fn = IECoreHoudini.FnOpHolder( print_op )
		fn.setOp( "objectDebug", 1 )
		print_op.parm("parm_quiet").set(True)

		# connect our ops together
		add_op.setInput( 0, v1_op )
		add_op.setInput( 1, v2_op )
		print_op.setInput( 0, add_op )

		# cook and check our output
		print_op.cook()
		fn = IECoreHoudini.FnOpHolder(print_op)
		result = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( result, IECore.V3fVectorData( [IECore.V3f(5,7,9),IECore.V3f(5,7,9),IECore.V3f(5,7,9)] ) )

	# test that a hip with opHolders wired together can be saved and reloaded & still evaluate
	def testSaveLoad(self):
		hou.hipFile.clear(suppress_save_prompt=True)
		save_file = "test/opHolder_testData/opSave_test.hip"
		self.testWireTogether()

		# save scene
		hou.hipFile.save(save_file)

		# new scene
		hou.hipFile.clear(suppress_save_prompt=True)

		# open scene
		hou.hipFile.load(save_file)

		# check some parameters are ok
		self.assertEqual( hou.node("/obj/geo1/vector1").parm("parm_size").eval(), 3 )
		self.assertEqual( hou.node("/obj/geo1/vector1").parmTuple("parm_value").eval(), (1,2,3) )
		self.assertEqual( hou.node("/obj/geo1/vector2").parm("parm_size").eval(), 3 )
		self.assertEqual( hou.node("/obj/geo1/vector2").parmTuple("parm_value").eval(), (4,5,6) )

		# check the result of our last opHolder
		n = hou.node("/obj/geo1/print_values")
		n.cook()
		fn = IECoreHoudini.FnOpHolder(n)
		result = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( result, IECore.V3fVectorData( [IECore.V3f(5,7,9),IECore.V3f(5,7,9),IECore.V3f(5,7,9)] ) )

	# tests changing op and inputs
	def testChangingOp( self ) :
		n = IECoreHoudini.FnOpHolder.create( "test_node", "vectors/V3fVectorCreator", 1 )
		fn = IECoreHoudini.FnOpHolder( n )
		op = fn.getParameterised()
		self.assertEqual( len(n.inputConnectors()), 0 )

		fn.setOp( "objectDebug", 1 )
		self.assertEqual( len(n.inputConnectors()), 1 )
		torus = n.createInputNode( 0, "torus" )
		self.assertEqual( torus, n.inputConnections()[0].inputNode() )
		self.assertEqual( 0, n.inputConnections()[0].inputIndex() )

		fn.setOp( "vectors/V3fVectorAdder", 1 )
		self.assertEqual( len(n.inputConnectors()), 2 )
		self.assertEqual( torus, n.inputConnections()[0].inputNode() )
		self.assertEqual( 0, n.inputConnections()[0].inputIndex() )
		box = n.createInputNode( 1, "box" )
		self.assertEqual( box, n.inputConnections()[1].inputNode() )
		self.assertEqual( 1, n.inputConnections()[1].inputIndex() )

		n.setInput( 0, None )
		self.assertEqual( len(n.inputConnectors()), 2 )
		self.assertEqual( len(n.inputConnections()), 1 )
		self.assertEqual( box, n.inputConnections()[0].inputNode() )
		self.assertEqual( 1, n.inputConnections()[0].inputIndex() )
		fn.setOp( "objectDebug", 1 )
		self.assertEqual( len(n.inputConnectors()), 1 )
		self.assertEqual( box, n.inputConnections()[0].inputNode() )
		self.assertEqual( 0, n.inputConnections()[0].inputIndex() )

		fn.setOp( "vectors/V3fVectorCreator", 1 )
		self.assertEqual( len(n.inputConnectors()), 0 )
		self.assert_( not n.inputConnectors() )

	# tests creation of a lot of opHolders
	def testLotsQuickly(self):
		n = []
		for i in range(1000):
			n.append( IECoreHoudini.FnOpHolder.create( "noise_deformer", "noiseDeformer", 1 ) )
		for _n in n:
			_n.destroy()

	# test using the noiseDeformer op
	def testModifyMesh(self):
		(op, fn) = self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised( cl )
		op.parm("parm_filename").set( self.__torusNormalsTestFile )
		deformer = op.createOutputNode( "ieOpHolder" )
		self.assert_( deformer )
		cl = IECore.ClassLoader.defaultOpLoader().load("noiseDeformer", 1)()
		self.assertEqual( cl.typeName(), "noiseDeformer" )
		fn = IECoreHoudini.FnOpHolder( deformer )
		fn.setParameterised( cl )
		deformer.parm("parm_magnitude").set( 2.5 )
		deformer.parmTuple("parm_frequency").set( (1,2,3) )
		deformer.cook()
		torus = IECore.Reader.create( self.__torusNormalsTestFile ).read()
		result = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( len(result["P"].data), len(torus["P"].data) )
		self.assertEqual( len(result["N"].data), len(torus["N"].data) )
		self.assertNotEqual( result["P"], torus["P"] )
		self.assertNotEqual( result["N"], torus["N"] )
		return ( op, deformer )

	# test the bbox on our Sop geometry is set correctly
	def testOutputBBox(self):
		(op,fn) = self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised( cl )
		op.parm("parm_filename").set( self.__torusNormalsTestFile )
		op.cook()
		geo = op.geometry()
		self.assert_( geo )
		bbox = geo.boundingBox()
		self.failUnless( bbox.isAlmostEqual(hou.BoundingBox(-1.5, -0.475528, -1.42658, 1.5, 0.475528, 1.42658)) )
		deformer = op.createOutputNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("noiseDeformer", 1)()
		fn = IECoreHoudini.FnOpHolder( deformer )
		fn.setParameterised( cl )
		self.assertEqual( len(deformer.inputConnectors()), 1 )
		deformer.parm("parm_magnitude").set(2)
		deformer.cook()
		geo2 = deformer.geometry()
		self.assert_( geo2 )
		bbox2 = geo2.boundingBox()
		self.assert_( not bbox2.isAlmostEqual(hou.BoundingBox(-1.5, -0.475528, -1.42658, 1.5, 0.475528, 1.42658)) )
		self.failUnless( bbox2.isAlmostEqual(hou.BoundingBox(-1.8938, -1.08025, -1.75561, 1.64279, 1.37116, 1.97013)) )
		return ( geo, deformer )

	# test an opHolder with 2 primitive inputs
	def testMultipleInputs(self):
		(geo, deformer) = self.testModifyMesh()
		swap = geo.createOutputNode( "ieOpHolder", node_name="swapP" )
		cl = IECore.ClassLoader.defaultOpLoader().load("swapAttribute", 1)()
		fn = IECoreHoudini.FnOpHolder( swap )
		fn.setParameterised( cl )
		swap.setInput( 1, deformer )
		swap.cook()
		src = IECoreHoudini.FnOpHolder( geo ).getParameterised().resultParameter().getValue()
		deformer = IECoreHoudini.FnOpHolder( deformer ).getParameterised().resultParameter().getValue()
		result = cl.resultParameter().getValue()
		self.failUnless( 'P' in result )
		self.assertNotEqual( result['P'].data, src['P'].data)
		self.assertEqual( result['P'].data, deformer['P'].data)
		self.assertEqual( result['N'].data, src['N'].data)
		self.assertNotEqual( result['N'].data, deformer['N'].data)

	# tests compound parameter support
	def testCompoundParameters(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("parameters/compoundParameters", 1)()
		fn.setParameterised( cl )

		# test we have the parameters & folders
		num_folders = [ type(p.parmTemplate()).__name__ for p in op.spareParms()].count("FolderSetParmTemplate")
		self.assertEqual( num_folders, 4 )
		p = op.parm( "parm_compound_1_jy" )
		self.assert_( p )
		self.assertEqual( p.containingFolders(), ('Parameters', 'My Compound 1') )
		p = op.parm( "parm_compound_2_kx" )
		self.assert_( p )
		self.assertEqual( p.containingFolders(), ('Parameters', 'My Compound 2') )
		p = op.parm( "parm_compound_3_compound_4_some_int" )
		self.assert_( p )
		self.assertEqual( p.containingFolders(), ('Parameters', 'My Compound 3', 'My Compound 4') )

		# test that houdini values get set on cortex parameters correctly
		p = op.parmTuple( "parm_compound_3_compound_4_some_int" )
		p.set( [345] )
		self.assertEqual( cl.parameters()["compound_3"]["compound_4"]["some_int"].getValue().value, 123 )
		op.cook()
		self.assertEqual( cl.parameters()["compound_3"]["compound_4"]["some_int"].getValue().value, 345 )

		p = op.parmTuple( "parm_compound_2_j" )
		p.set( [123.456, 456.789, 0.0] )
		self.assert_( ( cl.parameters()["compound_2"]["j"].getValue().value - IECore.V3d( 8,16,32 ) ).length() < 0.001 )
		op.cook()
		self.assert_( ( cl.parameters()["compound_2"]["j"].getValue().value - IECore.V3d( 123.456, 456.789, 0 ) ).length() < 0.001 )

		# test that caching parameters works
		op.parm( "__classReloadButton" ).pressButton()
		op.cook()
		self.assertEqual( cl.parameters()["compound_3"]["compound_4"]["some_int"].getValue().value, 345 )
		self.assert_( ( cl.parameters()["compound_2"]["j"].getValue().value - IECore.V3d( 123.456, 456.789, 0 ) ).length() < 0.001 )

	def testObjectParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("objectDebug", 1)()
		fn.setParameterised( cl )
		op.parm("parm_quiet").set( True )
		torus = op.createInputNode(0, "torus" )
		torus.parm( "rows" ).set( 10 )
		torus.parm( "cols" ).set( 10 )
		op.cook()
		result = cl.resultParameter().getValue()
		self.assertEqual( len( op.errors() ), 0 )
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		torus.parm("type").set(1)
		op.cook()
		result = cl.resultParameter().getValue()
		self.assertEqual( len( op.errors() ), 0 )
		self.assertEqual( result.typeId(), IECore.TypeId.PointsPrimitive )
		op2 = op.createInputNode(0, "ieOpHolder")
		fn2 = IECoreHoudini.FnOpHolder( op2 )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn2.setParameterised(cl)
		op2.parm("parm_filename").set( self.__torusTestFile )
		op.cook()
		result2 = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( len( op.errors() ), 0 )
		self.assertEqual( result2.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result2["P"].data, result["P"].data )

	def testObjectParameterWithMultipleTypesConversion( self ) :
		( op, fn ) = self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load( "multiTypeObject", 1 )()
		fn.setParameterised( cl )
		torus = op.createInputNode( 0, "torus" )
		torus.parm( "rows" ).set( 10 )
		torus.parm( "cols" ).set( 10 )
		op.cook()
		result = cl.resultParameter().getValue()
		self.assert_( not op.errors() )
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		torus.parm( "type" ).set( 1 )
		op.cook()
		result2 = cl.resultParameter().getValue()
		self.assert_( not op.errors() )
		self.assertEqual( result2.typeId(), IECore.TypeId.PointsPrimitive )
		op2 = op.createInputNode( 0, "ieOpHolder" )
		fn2 = IECoreHoudini.FnOpHolder( op2 )
		cl = IECore.ClassLoader.defaultOpLoader().load( "cobReader", 1 )()
		fn2.setParameterised( cl )
		op2.parm( "parm_filename" ).set( self.__torusTestFile )
		op.cook()
		result3 = fn.getParameterised().resultParameter().getValue()
		self.assert_( not op.errors() )
		self.assertEqual( result3.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result3["P"].data, result["P"].data )
		cl = IECore.ClassLoader.defaultOpLoader().load( "vectors/V3fVectorAdder", 1 )()
		fn2.setParameterised( cl )
		fn2.getParameterised().parameters()['vector1'].setValue( result["P"].data )
		fn2.getParameterised().parameters()['vector2'].setValue( result["P"].data )
		op.cook()
		result4 = fn.getParameterised().resultParameter().getValue()
		self.assert_( not op.errors() )
		self.assertEqual( result4.typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( result4["P"].data, result["P"].data + result["P"].data )

	def testPrimitiveParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("parameters/primitives/primParam", 1)()
		fn.setParameterised( cl )
		torus = op.createInputNode(0, "torus" )
		torus.parm( "rows" ).set( 10 )
		torus.parm( "cols" ).set( 10 )
		op.cook()
		result = cl.resultParameter().getValue()
		self.assertEqual( len( op.errors() ), 0 )
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		torus.parm("type").set(1)
		op.cook()
		result = cl.resultParameter().getValue()
		self.assertEqual( len( op.errors() ), 0 )
		self.assertEqual( result.typeId(), IECore.TypeId.PointsPrimitive )
		op2 = op.createInputNode(0, "ieOpHolder")
		fn = IECoreHoudini.FnOpHolder( op2 )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised(cl)
		op2.parm("parm_filename").set( self.__torusTestFile )
		op.cook()
		result2 = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( len( op.errors() ), 0 )
		self.assertEqual( result2.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result2["P"].data, result["P"].data )

	def testPointsParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("parameters/primitives/pointParam", 1)()
		fn.setParameterised( cl )
		cob = op.createInputNode(0, "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn2 = IECoreHoudini.FnOpHolder( cob )
		fn2.setParameterised( cl )
		cob.parm("parm_filename").set( self.__torusTestFile )
		self.assertRaises( hou.OperationFailed, op.cook )
		self.assertNotEqual( len( op.errors() ), 0 )
		cob = op.createInputNode(0, "torus" )
		op.cook() # should pass because torus will be converted to points
		self.assertEqual( fn.getParameterised()['input'].getValue().typeId(), IECore.TypeId.PointsPrimitive )
		self.assertEqual( fn.getParameterised().resultParameter().getValue().typeId(), IECore.TypeId.PointsPrimitive )

	def testPolygonsParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("parameters/primitives/polyParam", 1)()
		fn.setParameterised( cl )
		cob = op.createInputNode(0, "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn2 = IECoreHoudini.FnOpHolder( cob )
		fn2.setParameterised( cl )
		cob.parm("parm_filename").set( self.__torusTestFile )
		op.cook() # should pass because we have a mesh primitive
		torus = op.createInputNode(0, "torus" )
		op.cook() # should pass because torus will be converted to mesh
		self.assertEqual( fn.getParameterised()['input'].getValue().typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( fn.getParameterised().resultParameter().getValue().typeId(), IECore.TypeId.MeshPrimitive )
		op2 = torus.createOutputNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("parameters/primitives/pointParam", 1)()
		fn = IECoreHoudini.FnOpHolder( op2 )
		fn.setParameterised( cl )
		op2.cook()
		self.assertEqual( fn.getParameterised().resultParameter().getValue().typeId(), IECore.TypeId.PointsPrimitive )
		op.setInput( 0, op2 )
		self.assertRaises( hou.OperationFailed, op.cook )
		self.assertNotEqual( len( op.errors() ), 0 )

	def testGroupParameterConversion( self ) :
		( holder, fn ) = self.testOpHolder()
		fn.setOp( "parameters/groupParam", 1 )

		merge = holder.createInputNode( 0, "merge" )
		attrib1 = merge.createInputNode( 0, "attribcreate" )
		attrib1.parm( "name1" ).set( "name" )
		attrib1.parm( "class1" ).set( 1 ) # Prim
		attrib1.parm( "type1" ).set( 3 ) # String
		attrib1.parm( "string1" ).set( "torusGroup" )
		group1 = attrib1.createInputNode( 0, "group" )
		group1.parm( "crname" ).set( "torusGroup" )
		torus = group1.createInputNode( 0, "torus" )
		torus.parm( "rows" ).set( 10 )
		torus.parm( "cols" ).set( 10 )

		attrib2 = merge.createInputNode( 1, "attribcreate" )
		attrib2.parm( "name1" ).set( "name" )
		attrib2.parm( "class1" ).set( 1 ) # Prim
		attrib2.parm( "type1" ).set( 3 ) # String
		attrib2.parm( "string1" ).set( "boxGroup" )
		group2 = attrib2.createInputNode( 0, "group" )
		group2.parm( "crname" ).set( "boxGroup" )
		box = group2.createInputNode( 0, "box" )

		holder.parm( "parm_input_groupingMode" ).set( IECoreHoudini.FromHoudiniGroupConverter.GroupingMode.PrimitiveGroup )
		holder.cook()
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( fn.getOp()['input'].getValue().typeId(), IECore.TypeId.Group )
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.blindData(), IECore.CompoundData( { "name" : "torusGroup" } ) )
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 100 )

		group1.bypass( True )
		group2.bypass( True )
		attrib1.bypass( True )
		attrib2.bypass( True )
		holder.cook()
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( fn.getOp()['input'].getValue().typeId(), IECore.TypeId.Group )
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.blindData(), IECore.CompoundData() )
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 106 )

		## \todo: keep the names and convert in PrimitiveGroup mode. see todo in FromHoudiniGroupConverter.cpp

		attrib1.bypass( False )
		attrib2.bypass( False )
		holder.parm( "parm_input_groupingMode" ).set( IECoreHoudini.FromHoudiniGroupConverter.GroupingMode.NameAttribute )
		holder.parm( "parm_input_useNameFilter" ).set( False )
		holder.cook()
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( fn.getOp()['input'].getValue().typeId(), IECore.TypeId.Group )
		self.assertEqual( result.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( result.blindData(), IECore.CompoundData( { "name" : "boxGroup" } ) )
		self.assertEqual( result.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 6 )

	def testInputConnectionsSaveLoad( self ) :
		hou.hipFile.clear( suppress_save_prompt=True )

		( holder, fn ) = self.testOpHolder()
		fn.setOp( "parameters/groupParam", 2 )

		holderPath = holder.path()
		torusPath = holder.createInputNode( 0, "torus" ).path()
		boxPath = holder.createInputNode( 1, "box" ).path()

		self.assertEqual( len(holder.inputs()), 2 )
		self.assertEqual( holder.inputs()[0].path(), torusPath )
		self.assertEqual( holder.inputs()[1].path(), boxPath )

		hip = "test/opHolder_testData/opSave_test.hip"
		hou.hipFile.save( hip )
		hou.hipFile.clear( suppress_save_prompt=True )
		hou.hipFile.load( hip )

		holder = hou.node( holderPath )
		self.assertEqual( len(holder.inputs()), 2 )
		self.assertEqual( holder.inputs()[0].path(), torusPath )
		self.assertEqual( holder.inputs()[1].path(), boxPath )

	def testInvalidValidation(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised( cl )
		op.parm("parm_filename").set( self.__torusTestFile )
		op2 = op.createOutputNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("parameters/primitives/pointParam", 1)()
		fn = IECoreHoudini.FnOpHolder(op2)
		fn.setParameterised(cl)
		self.assertRaises( hou.OperationFailed, op2.cook )
		self.assertNotEqual( len( op2.errors() ), 0 )

	def testInvalidOp(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("noiseDeformer", 1)()
		fn.setParameterised( cl )
		self.assertRaises( hou.OperationFailed, op.cook )
		self.assertNotEqual( len( op.errors() ), 0 )

	def testMatchString(self):
		(op,fn)=self.testOpHolder()
		op.parm( "__classMatchString" ).set( "*" )
		op.parm( "__className" ).set( "cobReader" )
		op.parm( "__className" ).pressButton()
		cl = fn.getParameterised()
		self.assertEqual( cl.typeName(), "cobReader" )
		op.parm( "__classMatchString" ).set( "object*" )
		results = fn.classNames()
		self.assertEqual( len(fn.classNames()), 1 )
		op.parm( "__className" ).set( "cobReader" ) # this still works, should it be invalid?
		op.parm( "__className" ).pressButton()
		cl = fn.getParameterised()
		self.assertEqual( cl.typeName(), "cobReader" )
		op.parm( "__classMatchString" ).set("*")
		self.failUnless( len(fn.classNames()) > 1 )

	def testCategories( self ) :
		( op, fn ) = self.testOpHolder()
		op.parm( "__classMatchString" ).set( "*" )
		self.assertEqual( op.parm( "__classCategory" ).eval(), "" )

		op.parm( "__className" ).set( "cobReader" )
		self.assertEqual( op.parm( "__classCategory" ).eval(), "" )
		op.parm( "__className" ).pressButton()
		self.assertEqual( fn.getParameterised().typeName(), "cobReader" )
		self.assertEqual( fn.getParameterised().path, "cobReader" )

		op.parm( "__className" ).set( "vectors/V3fVectorCreator" )
		op.parm( "__className" ).pressButton()
		self.assertEqual( op.parm( "__classCategory" ).eval(), "vectors" )
		self.assertEqual( fn.getParameterised().typeName(), "V3fVectorCreator" )
		self.assertEqual( fn.getParameterised().path, "vectors/V3fVectorCreator" )

		op.parm( "__className" ).set( "" )
		op.parm( "__className" ).pressButton()
		self.assertEqual( op.parm( "__classCategory" ).eval(), "vectors" )
		op.parm( "__classCategory" ).set( "" )
		op.parm( "__classCategory" ).pressButton()
		self.assertRaises( hou.OperationFailed, op.cook )
		self.assertEqual( op.parm( "__className" ).eval(), "" )

		op.parm( "__className" ).set( "parameters/compoundParameters" )
		op.parm( "__className" ).pressButton()
		self.assertEqual( op.parm( "__classCategory" ).eval(), "parameters" )
		self.assertEqual( fn.getParameterised().typeName(), "compoundParameters" )
		self.assertEqual( fn.getParameterised().path, "parameters/compoundParameters" )
		op.parm( "__className" ).set( "parameters/primitives/pointParam" )
		op.parm( "__className" ).pressButton()
		self.assertEqual( op.parm( "__classCategory" ).eval(), "parameters/primitives" )
		self.assertEqual( fn.getParameterised().typeName(), "pointParam" )
		self.assertEqual( fn.getParameterised().path, "parameters/primitives/pointParam" )

		op.parm( "__classCategory" ).set( "" )
		op.parm( "__classCategory" ).pressButton()
		self.failUnless( len(fn.classNames()) > 4 )
		op.parm( "__classMatchString" ).set( "parameters/*" )
		self.assertEqual( len(fn.classNames()), 5 )

	def testSetOpValues( self ) :
		( holder, fn ) = self.testOpHolder()
		op = IECore.ClassLoader.defaultOpLoader().load( "noiseDeformer", 1 )()
		fn.setOp( op )
		self.assertEqual( tuple(op.parameters()['frequency'].defaultValue.value), holder.parmTuple( "parm_frequency" ).parmTemplate().defaultValue() )
		self.assertEqual( tuple(op.parameters()['frequency'].defaultValue.value), holder.parmTuple( "parm_frequency" ).eval() )
		self.assertEqual( tuple(op.parameters()['frequency'].getTypedValue()), holder.parmTuple( "parm_frequency" ).eval() )

		( holder2, fn2 ) = self.testOpHolder()
		op.parameters()['frequency'].setTypedValue( IECore.V3f( 0.2, 0.4, 0.6 ) )
		fn2.setOp( op )
		self.assertEqual( tuple(op.parameters()['frequency'].defaultValue.value), holder2.parmTuple( "parm_frequency" ).parmTemplate().defaultValue() )
		self.assertNotEqual( tuple(op.parameters()['frequency'].defaultValue.value), holder2.parmTuple( "parm_frequency" ).eval() )
		self.assertEqual( tuple(op.parameters()['frequency'].getTypedValue()), holder2.parmTuple( "parm_frequency" ).eval() )

	def testParameterDescriptions( self ) :
		( holder, fn ) = self.testOpHolder()
		fn.setOp( "parameters/compoundParameters" )
		parameters = fn.getOp().parameters()
		self.assertEqual( parameters['blah'].description, holder.parm( "parm_blah" ).parmTemplate().help() )
		self.assertEqual( parameters['compound_1']['j'].description, holder.parmTuple( "parm_compound_1_j" ).parmTemplate().help() )
		self.assertEqual( parameters['compound_1']['k'].description, holder.parmTuple( "parm_compound_1_k" ).parmTemplate().help() )
		self.assertEqual( parameters['compound_3']['compound_4']['some_int'].description, holder.parm( "parm_compound_3_compound_4_some_int" ).parmTemplate().help() )
		self.assertEqual( parameters['compound_5']['bool_1'].description, holder.parm( "parm_compound_5_bool_1" ).parmTemplate().help() )

	def testNumericPresetMenus( self ) :

		# at present, Int/FloatParameters only support presetsOnly presets, due to the limitations of hou.MenuParmTemplate
		( holder, fn ) = self.testOpHolder()
		holder.createInputNode( 0, "box" )
		fn.setOp( "parameters/groupParam", 2 )
		parm = holder.parm( "parm_switch" )
		self.failUnless( isinstance( parm, hou.Parm ) )
		template = parm.parmTemplate()
		self.failUnless( isinstance( template, hou.MenuParmTemplate ) )
		# the int values are stored as strings in this crazy Houdini world
		self.assertEqual( template.menuItems(), ( "20", "30" ) )
		self.assertEqual( template.menuLabels(), ( "A", "B" ) )
		self.assertEqual( template.defaultValue(), 0 )
		self.assertEqual( template.defaultValueAsString(), "20" )
		self.assertEqual( parm.eval(), 0 )
		self.assertEqual( parm.evalAsString(), "20" )

		# but on the op values are really the ints we require
		op = fn.getOp()
		self.assertEqual( op["switch"].getTypedValue(), 20 )
		parm.set( 1 )
		holder.cook()
		self.assertEqual( op["switch"].getTypedValue(), 30 )
		# Houdini 16 does not allow ordered menu parms to be set to non-menu items
		# if the parm is set to an index, and the index doesn't exist, then the parm is set to the closest item menu
		if hou.applicationVersion()[0] < 16:
			parm.set( 2 )
			self.assertRaises( hou.OperationFailed, holder.cook )
			parm.set( -1 )
			self.assertRaises( hou.OperationFailed, holder.cook )
			parm.set( 0 )
			holder.cook()
			self.failUnless( not holder.errors() )

		newHolder = holder.parent().createNode( "ieOpHolder" )
		newFn = IECoreHoudini.FnOpHolder( newHolder )
		op["switch"].setTypedValue( 30 )
		newFn.setOp( op )
		newParm = newHolder.parm( "parm_switch" )
		self.assertEqual( newParm.eval(), 1 )
		self.assertEqual( newParm.evalAsString(), "30" )

	def testMessageHandling( self ) :

		( holder, fn ) = self.testOpHolder()
		fn.setOp( "noiseDeformer" )

		self.assertRaises( hou.OperationFailed, holder.cook )
		self.failUnless( "Must have primvar 'N' in primitive!" in "".join( holder.errors() ) )

		torus = holder.createInputNode( 0, "torus" )
		self.assertRaises( hou.OperationFailed, holder.cook )
		self.failUnless( "Must have primvar 'N' in primitive!" in "".join( holder.errors() ) )

		holder2 = holder.createInputNode( 0, "ieOpHolder" )
		fn2 = IECoreHoudini.FnOpHolder( holder2 )
		fn2.setOp( "meshNormalsOp" )
		holder2.setInput( 0, torus )

		holder.cook()
		self.assertEqual( len( holder.errors() ), 0 )
		self.assertEqual( len( holder2.errors() ), 0 )

		fn2.setOp( "objectDebug", 2 )
		self.assertEqual( len( holder2.errors() ), 0 )
		self.assertEqual( len( holder2.warnings() ), 0 )

		holder2.parm( "parm_messageLevel" ).set( int(IECore.MessageHandler.Level.Warning) )
		holder2.cook()
		self.assertEqual( len( holder2.errors() ), 0 )
		self.assertNotEqual( len(holder2.warnings()), 0 )

		holder2.parm( "parm_messageLevel" ).set( int(IECore.MessageHandler.Level.Error) )
		self.assertRaises( hou.OperationFailed, holder2.cook )
		self.assertNotEqual( len( holder2.errors() ), 0 )
		self.assertEqual( len( holder2.warnings() ), 0 )

	def testAnimatedValues( self ) :

		noise = IECoreHoudini.FnOpHolder.create( "test", "noiseDeformer", 1 )
		fn = IECoreHoudini.FnOpHolder( noise )
		noise.parm( "parm_magnitude" ).setExpression( "$FF" )
		hou.setFrame( 1 )
		self.assertEqual( noise.evalParm( "parm_magnitude" ), 1 )
		self.assertEqual( fn.getOp().parameters()["magnitude"].getTypedValue(), 1 )
		hou.setFrame( 12.25 )
		self.assertEqual( noise.evalParm( "parm_magnitude" ), 12.25  )
		# values haven't been flushed yet
		self.assertAlmostEqual( fn.getOp().parameters()["magnitude"].getTypedValue(), 1 )
		# so we flush them
		fn.setParameterisedValues()
		self.assertAlmostEqual( fn.getOp().parameters()["magnitude"].getTypedValue(), 12.25 )

	def namedScene( self, opType ) :

		holder = IECoreHoudini.FnOpHolder.create( "holder", opType, 1 )

		geo = holder.parent()
		boxA = geo.createNode( "box" )
		nameA = boxA.createOutputNode( "name" )
		nameA.parm( "name1" ).set( "boxA" )

		boxB = geo.createNode( "box" )
		transformB = boxB.createOutputNode( "xform" )
		transformB.parm( "tx" ).set( 5 )
		nameB = transformB.createOutputNode( "name" )
		nameB.parm( "name1" ).set( "boxB" )

		boxC = geo.createNode( "box" )
		transformC = boxC.createOutputNode( "xform" )
		transformC.parm( "tx" ).set( 10 )
		nameC = transformC.createOutputNode( "name" )
		nameC.parm( "name1" ).set( "boxC" )

		merge = geo.createNode( "merge" )
		merge.setInput( 0, nameA )
		merge.setInput( 1, nameB )
		merge.setInput( 2, nameC )

		converter = merge.createOutputNode( "ieCortexConverter" )
		converter.parm( "resultType" ).set( 0 ) # Cortex
		holder.setInput( 0, converter )

		return holder

	def testMultipleOperations( self ) :

		holder = self.namedScene( "meshNormalsOp" )

		def verify( passThrough = [] ) :

			geo = holder.geometry()
			self.assertEqual( len( holder.errors() ), 0 )
			self.assertEqual( len( holder.warnings() ), 0 )
			self.assertEqual( len(geo.prims()), 3 )
			names = [ "boxA", "boxB", "boxC" ]
			for i in range( 0, len(geo.prims()) ) :
				prim = geo.prims()[i]
				self.assertEqual( prim.type(), hou.primType.Custom )
				self.assertEqual( prim.attribValue( "name" ), names[i] )

			result = IECoreHoudini.FromHoudiniGeometryConverter.create( holder ).convert()
			self.assertTrue( result.isInstanceOf( IECore.TypeId.Group ) )
			self.assertEqual( len(result.children()), 3 )
			for j in range( 0, len(result.children()) ) :
				child = result.children()[j]
				self.assertTrue( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
				self.assertTrue( child.arePrimitiveVariablesValid() )
				if child.blindData()["name"].value in passThrough :
					self.assertEqual( child.keys(), [ "P" ] )
				else :
					self.assertEqual( child.keys(), [ "N", "P" ] )

		# normals were added to each mesh individually
		verify()

		# non-matching shapes were passed through unmodified
		holder.parm( "parm_input_nameFilter" ).set( "* ^boxA" )
		verify( passThrough = [ "boxA" ] )

		# still operates multiple times for normal houdini geo
		holder.inputConnections()[0].inputNode().bypass( True )
		geo = holder.geometry()
		self.assertEqual( len( holder.errors() ), 0 )
		self.assertEqual( len( holder.warnings() ), 0 )
		self.assertEqual( len(geo.prims()), 8 )
		names = [ "boxA", "boxB", "boxC" ]
		for i in range( 0, 6 ) :
			prim = geo.prims()[i]
			self.assertEqual( prim.type(), hou.primType.Polygon )
			self.assertEqual( prim.attribValue( "name" ), "boxA" )
		prim = geo.prims()[6]
		self.assertEqual( prim.type(), hou.primType.Custom )
		self.assertEqual( prim.attribValue( "name" ), "boxB" )
		prim = geo.prims()[7]
		self.assertEqual( prim.type(), hou.primType.Custom )
		self.assertEqual( prim.attribValue( "name" ), "boxC" )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( holder ).convert()
		self.assertTrue( result.isInstanceOf( IECore.TypeId.Group ) )
		self.assertEqual( len(result.children()), 3 )
		for j in range( 0, len(result.children()) ) :
			child = result.children()[j]
			self.assertTrue( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
			self.assertTrue( child.arePrimitiveVariablesValid() )
			if child.blindData()["name"].value == "boxA" :
				self.assertEqual( child.keys(), [ "P" ] )
			else :
				self.assertEqual( child.keys(), [ "N", "P" ] )

		# no nameFilter with normal geo compresses to one mesh
		holder.parm( "parm_input_useNameFilter" ).set( False )
		geo = holder.geometry()
		self.assertEqual( len( holder.errors() ), 0 )
		self.assertEqual( len( holder.warnings() ), 0 )
		self.assertEqual( len(geo.prims()), 1 )
		prim = geo.prims()[0]
		self.assertEqual( prim.type(), hou.primType.Custom )
		self.assertEqual( geo.findPrimAttrib( "name" ), None )
		result = IECoreHoudini.FromHoudiniGeometryConverter.create( holder ).convert()
		self.assertTrue( result.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result.keys(), [ "N", "P" ] )

		# no nameFilter with CortexObjects may have unexpected results (because the input parameter wants a single mesh)
		holder.inputConnections()[0].inputNode().bypass( False )
		holder.cook()
		self.assertEqual( len( holder.errors() ), 0 )
		self.assertNotEqual( len(holder.warnings()), 0 )

	def testNameFilterOnSecondaryInputs( self ) :

		holder = self.namedScene( "meshMerge" )
		torus = holder.parent().createNode( "torus" )
		torus.parm( "rows" ).set( 10 )
		torus.parm( "cols" ).set( 10 )
		holder.setInput( 1, torus )

		def verify( numMergedFaces, passThrough = [] ) :

			geo = holder.geometry()
			self.assertEqual( len( holder.errors() ), 0 )
			self.assertEqual( len( holder.warnings() ), 0 )
			self.assertEqual( len(geo.prims()), 3 )
			names = [ "boxA", "boxB", "boxC" ]
			for i in range( 0, len(geo.prims()) ) :
				prim = geo.prims()[i]
				self.assertEqual( prim.type(), hou.primType.Custom )
				self.assertEqual( prim.attribValue( "name" ), names[i] )

			result = IECoreHoudini.FromHoudiniGeometryConverter.create( holder ).convert()
			self.assertTrue( result.isInstanceOf( IECore.TypeId.Group ) )
			self.assertEqual( len(result.children()), 3 )
			for j in range( 0, len(result.children()) ) :
				child = result.children()[j]
				self.assertTrue( child.isInstanceOf( IECore.TypeId.MeshPrimitive ) )
				self.assertTrue( child.arePrimitiveVariablesValid() )
				if child.blindData()["name"].value in passThrough :
					self.assertEqual( child.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 6 )
				else :
					self.assertEqual( child.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 6 + numMergedFaces )

		# torus is merged with each box
		verify( numMergedFaces = 100 )

		# torus is not merged with the passThrough boxes
		holder.parm( "parm_input_nameFilter" ).set( "* ^boxA" )
		verify( numMergedFaces = 100, passThrough = [ "boxA" ] )

		# multiple meshes in the second parameter may have unexpected results (because it wants a single mesh)
		holder.setInput( 1, holder.inputConnections()[0].inputNode() )
		holder.cook()
		self.assertEqual( len( holder.errors() ), 0 )
		self.assertNotEqual( len(holder.warnings()), 0 )

		# a single mesh will merge
		holder.parm( "parm_mesh_nameFilter" ).set( "boxB" )
		verify( numMergedFaces = 6, passThrough = [ "boxA" ] )

		# a bulk of normal houdini geo will also merge (it compresses to one mesh)
		converter = holder.inputConnections()[0].inputNode().createOutputNode( "ieCortexConverter" )
		holder.setInput( 1, converter )
		holder.parm( "parm_mesh_nameFilter" ).set( "*" )
		verify( numMergedFaces = 18, passThrough = [ "boxA" ] )
		holder.parm( "parm_mesh_nameFilter" ).set( "* ^boxA" )
		verify( numMergedFaces = 12, passThrough = [ "boxA" ] )
		holder.parm( "parm_mesh_useNameFilter" ).set( False )
		verify( numMergedFaces = 18, passThrough = [ "boxA" ] )

	def setUp( self ) :
		IECoreHoudini.TestCase.setUp( self )
		self.__torusTestFile = "test/IECoreHoudini/data/torus.cob" if hou.applicationVersion()[0] < 14 else "test/IECoreHoudini/data/torusH14.cob"
		self.__torusNormalsTestFile = "test/IECoreHoudini/data/torus_with_normals.cob"
		if not os.path.exists( "test/opHolder_testData" ):
			os.mkdir( "test/opHolder_testData" )

	def tearDown( self ) :
		if os.path.exists( "test/opHolder_testData" ):
			shutil.rmtree( "test/opHolder_testData" )

if __name__ == "__main__":
	unittest.main()
