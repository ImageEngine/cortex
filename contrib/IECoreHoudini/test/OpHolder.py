##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

class TestOpHolder( unittest.TestCase ):

	def testOpHolder(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		op = geo.createNode( "ieOpHolder" )
		assert( op )
		fn = IECoreHoudini.FnOpHolder( op )
		assert( fn )
		return (op,fn)
	
	# tests a basic op, the function set and that it cooks as expected
	def testSimpleOp(self):
		(op, fn) = self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised( cl )
		assert( fn.getParameterised()!=None )
		assert( fn.getParameterised()==cl )
		op.parm("parm_filename").set("test/test_data/torus.cob")
		op.cook() # cook using Houdini's cook mechanism, NOT operate()
		assert( fn.getParameterised()["filename"].getValue()==IECore.StringData("test/test_data/torus.cob") )
		result = fn.getParameterised().resultParameter().getValue()
		assert( result == IECore.Reader.create("test/test_data/torus.cob").read() )
		
	# tests the alternative 'all in one' opHolder creator
	def testAlternateCreator(self):
		n = IECoreHoudini.FnOpHolder.create( "noise_deformer", "noiseDeformer", 1 )
		assert(n)
		fn = IECoreHoudini.FnOpHolder( n )
		assert(fn)
		op = fn.getParameterised()
		assert(op)
		assert(op.typeName()=="noiseDeformer")
		
	# test that a C++ op can be assigned using the function set
	def testCppOp(self):
		(op,fn) = self.testOpHolder()
		mesh_normals = IECore.MeshNormalsOp()
		assert( mesh_normals )
		fn.setParameterised(mesh_normals)
		assert(fn.getParameterised().typeName()=="MeshNormalsOp")
		
	# test that we can wire opholders together
	def testWireTogether(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		
		v1_op = geo.createNode( "ieOpHolder", node_name="vector1" )
		fn = IECoreHoudini.FnOpHolder( v1_op )
		cl = IECore.ClassLoader.defaultOpLoader().load("V3fVectorCreator", 1)()
		fn.setParameterised( cl )
		v1_op.parm("parm_size").set(3)
		v1_op.parmTuple("parm_value").set( (1,2,3) )
		
		v2_op = geo.createNode( "ieOpHolder", node_name="vector2" )
		fn = IECoreHoudini.FnOpHolder( v2_op )
		cl = IECore.ClassLoader.defaultOpLoader().load("V3fVectorCreator", 1)()
		fn.setParameterised( cl )
		v2_op.parm("parm_size").set(3)
		v2_op.parmTuple("parm_value").set( (4,5,6) )
		
		add_op = geo.createNode( "ieOpHolder", node_name="add_vectors" )
		fn = IECoreHoudini.FnOpHolder( add_op )
		cl = IECore.ClassLoader.defaultOpLoader().load("V3fVectorAdder", 1)()
		fn.setParameterised( cl )
		
		print_op = geo.createNode( "ieOpHolder", node_name="print_values" )
		fn = IECoreHoudini.FnOpHolder( print_op )
		cl = IECore.ClassLoader.defaultOpLoader().load("objectDebug", 1)()
		fn.setParameterised( cl )
		print_op.parm("parm_quiet").set(True)
		
		# connect our ops together
		add_op.setInput( 0, v1_op )
		add_op.setInput( 1, v2_op )
		print_op.setInput( 0, add_op )
		
		# cook and check our output
		print_op.cook()
		fn = IECoreHoudini.FnOpHolder(print_op)
		result = fn.getParameterised().resultParameter().getValue()
		assert( result == IECore.V3fVectorData( [IECore.V3f(5,7,9),IECore.V3f(5,7,9),IECore.V3f(5,7,9)] ) )
		
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
		assert( hou.node("/obj/geo1/vector1").parm("parm_size").eval()==3 )
		assert( hou.node("/obj/geo1/vector1").parmTuple("parm_value").eval()==(1,2,3) )
		assert( hou.node("/obj/geo1/vector2").parm("parm_size").eval()==3 )
		assert( hou.node("/obj/geo1/vector2").parmTuple("parm_value").eval()==(4,5,6) )
		
		# check the result of our last opHolder
		n = hou.node("/obj/geo1/print_values")
		n.cook()
		fn = IECoreHoudini.FnOpHolder(n)
		result = fn.getParameterised().resultParameter().getValue()
		assert( result == IECore.V3fVectorData( [IECore.V3f(5,7,9),IECore.V3f(5,7,9),IECore.V3f(5,7,9)] ) )
		
	# TODO: add a test to check that changing an op updates the inputs properly
	def testChangingOp(self):
		n = IECoreHoudini.FnOpHolder.create( "test_node", "V3fVectorCreator", 1)
		fn = IECoreHoudini.FnOpHolder(n)
		op = fn.getParameterised()
		assert( len(n.inputConnectors())==0 )
		fn.setParameterised( IECore.ClassLoader.defaultOpLoader().load("objectDebug",1)() )
		assert( len(n.inputConnectors())==1 )
		fn.setParameterised( IECore.ClassLoader.defaultOpLoader().load("V3fVectorAdder",1)() )
		assert( len(n.inputConnectors())==2 )

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
		op.parm("parm_filename").set("test/test_data/torus_with_normals.cob")
		deformer = op.createOutputNode( "ieOpHolder" )
		assert( deformer )
		cl = IECore.ClassLoader.defaultOpLoader().load("noiseDeformer", 1)()
		assert(cl.typeName()=="noiseDeformer")
		fn = IECoreHoudini.FnOpHolder( deformer )
		fn.setParameterised( cl )
		deformer.parm("parm_magnitude").set( 2.5 )
		deformer.parmTuple("parm_frequency").set( (1,2,3) )
		deformer.cook()
		torus = IECore.Reader.create( "test/test_data/torus_with_normals.cob" ).read()
		result = fn.getParameterised().resultParameter().getValue()
		assert( len(result["P"].data) == len(torus["P"].data) )
		assert( len(result["N"].data) == len(torus["N"].data) )
		assert( result["P"]!=torus["P"] )
		assert( result["N"]!=torus["N"] )
		return ( op, deformer )
		
	# test the bbox on our Sop geometry is set correctly
	def testOutputBBox(self):
		(op,fn) = self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised( cl )
		op.parm("parm_filename").set("test/test_data/torus_with_normals.cob")
		op.cook()
		geo = op.geometry()
		assert( geo )
		bbox = geo.boundingBox()
		assert( bbox.isAlmostEqual(hou.BoundingBox(-1.5, -0.475528, -1.42658, 1.5, 0.475528, 1.42658)) )
		deformer = op.createOutputNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("noiseDeformer", 1)()
		fn = IECoreHoudini.FnOpHolder( deformer )
		fn.setParameterised( cl )
		assert( len(deformer.inputConnectors())==1 )
		deformer.parm("parm_magnitude").set(2)
		deformer.cook()
		geo2 = deformer.geometry()
		assert( geo2 )
		bbox2 = geo2.boundingBox()
		assert( not bbox2.isAlmostEqual(hou.BoundingBox(-1.5, -0.475528, -1.42658, 1.5, 0.475528, 1.42658)) )
		assert( bbox2.isAlmostEqual(hou.BoundingBox(-1.8938, -1.08025, -1.75561, 1.64279, 1.37116, 1.97013)) )
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
		assert( 'P' in result )
		assert( result['P'].data != src['P'].data)
		assert( result['P'].data == deformer['P'].data)
		assert( result['N'].data == src['N'].data)
		assert( result['N'].data != deformer['N'].data)
		
	# tests compound parameter support
	def testCompoundParameters(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("compoundParameters", 1)()
		fn.setParameterised( cl )
		
		# test we have the parameters & folders
		num_folders = [ type(p.parmTemplate()).__name__ for p in op.spareParms()].count("FolderSetParmTemplate")
		assert( num_folders == 4 )
		p = op.parm( "parm_compound_1_parm_jy" )
		assert( p )
		assert( p.containingFolders() == ('Parameters', 'My Compound 1') )
		p = op.parm( "parm_compound_2_parm_kx" )
		assert( p )
		assert( p.containingFolders() == ('Parameters', 'My Compound 2') )
		p = op.parm( "parm_compound_3_parm_compound_4_parm_some_int" )
		assert( p )
		assert( p.containingFolders() == ('Parameters', 'My Compound 3', 'My Compound 4') )
		
		# test that houdini values get set on cortex parameters correctly
		p = op.parmTuple( "parm_compound_3_parm_compound_4_parm_some_int" )
		p.set( [345] )
		assert( cl.parameters()["compound_3"]["compound_4"]["some_int"].getValue().value == 123 )
		op.cook()
		assert( cl.parameters()["compound_3"]["compound_4"]["some_int"].getValue().value == 345 )
		
		p = op.parmTuple( "parm_compound_2_parm_j" )
		p.set( [123.456, 456.789, 0.0] )
		assert( ( cl.parameters()["compound_2"]["j"].getValue().value - IECore.V3d( 8,16,32 ) ).length()<0.001 )
		op.cook()
		assert( ( cl.parameters()["compound_2"]["j"].getValue().value - IECore.V3d( 123.456, 456.789, 0 ) ).length()<0.001 )
		
		# test that caching parameters works
		op.parm("__opReloadBtn").pressButton()
		op.cook()
		assert( cl.parameters()["compound_3"]["compound_4"]["some_int"].getValue().value == 345 )
		assert( ( cl.parameters()["compound_2"]["j"].getValue().value - IECore.V3d( 123.456, 456.789, 0 ) ).length()<0.001 )
		
	def testObjectParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("objectDebug", 1)()
		fn.setParameterised( cl )
		op.parm("parm_quiet").set( True )
		torus = op.createInputNode(0, "torus" )
		op.cook()
		result = cl.resultParameter().getValue()
		assert( op.errors()=="" )
		assert( result.typeId()==IECore.TypeId.MeshPrimitive )
		torus.parm("type").set(1)
		op.cook()
		result = cl.resultParameter().getValue()
		assert( op.errors()=="" )
		assert( result.typeId()==IECore.TypeId.PointsPrimitive )
		op2 = op.createInputNode(0, "ieOpHolder")
		fn = IECoreHoudini.FnOpHolder( op2 )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised(cl)
		op2.parm("parm_filename").set("test/test_data/torus.cob")
		op.cook()
		result2 = fn.getParameterised().resultParameter().getValue()
		assert( op.errors()=="" )
		assert( result2.typeId()==IECore.TypeId.MeshPrimitive )
		assert( result2["P"].data == result["P"].data )
		
	def testPrimitiveParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("primParam", 1)()
		fn.setParameterised( cl )
		torus = op.createInputNode(0, "torus" )
		op.cook()
		result = cl.resultParameter().getValue()
		assert( op.errors()=="" )
		assert( result.typeId()==IECore.TypeId.MeshPrimitive )
		torus.parm("type").set(1)
		op.cook()
		result = cl.resultParameter().getValue()
		assert( op.errors()=="" )
		assert( result.typeId()==IECore.TypeId.PointsPrimitive )
		op2 = op.createInputNode(0, "ieOpHolder")
		fn = IECoreHoudini.FnOpHolder( op2 )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised(cl)
		op2.parm("parm_filename").set("test/test_data/torus.cob")
		op.cook()
		result2 = fn.getParameterised().resultParameter().getValue()
		assert( op.errors()=="" )
		assert( result2.typeId()==IECore.TypeId.MeshPrimitive )
		assert( result2["P"].data == result["P"].data )
		
	def testPointsParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("pointParam", 1)()
		fn.setParameterised( cl )
		cob = op.createInputNode(0, "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn2 = IECoreHoudini.FnOpHolder( cob )
		fn2.setParameterised( cl )
		cob.parm("parm_filename").set("test/test_data/torus.cob")
		cook_failed = False
		try:
			op.cook() # This should fail because the parameter validation should fail
		except hou.OperationFailed:
			cook_failed = True
		assert(cook_failed)
		assert(op.errors()!="")
		cob = op.createInputNode(0, "torus" )
		op.cook() # should pass because torus will be converted to points
		assert( fn.getParameterised()['input'].getValue().typeId() == IECore.TypeId.PointsPrimitive )
		assert( fn.getParameterised().resultParameter().getValue().typeId() == IECore.TypeId.PointsPrimitive )

	def testPolygonsParameterConversion(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("polyParam", 1)()
		fn.setParameterised( cl )
		cob = op.createInputNode(0, "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn2 = IECoreHoudini.FnOpHolder( cob )
		fn2.setParameterised( cl )
		cob.parm("parm_filename").set("test/test_data/torus.cob")
		op.cook() # should pass because we have a mesh primitive
		torus = op.createInputNode(0, "torus" )
		op.cook() # should pass because torus will be converted to mesh
		assert( fn.getParameterised()['input'].getValue().typeId() == IECore.TypeId.MeshPrimitive )
		assert( fn.getParameterised().resultParameter().getValue().typeId() == IECore.TypeId.MeshPrimitive )
		op2 = torus.createOutputNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("pointParam", 1)()
		fn = IECoreHoudini.FnOpHolder( op2 )
		fn.setParameterised( cl )
		op2.cook()
		assert( fn.getParameterised().resultParameter().getValue().typeId() == IECore.TypeId.PointsPrimitive )
		op.setInput( 0, op2 )
		cook_failed = False
		try:
			op.cook() # This should fail because the parameter validation should fail
		except hou.OperationFailed:
			cook_failed = True
		assert(cook_failed)
		assert(op.errors()!="")
					
	def testInvalidValidation(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("cobReader", 1)()
		fn.setParameterised( cl )
		op.parm("parm_filename").set("test/test_data/torus.cob")
		op2 = op.createOutputNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("pointParam", 1)()
		fn = IECoreHoudini.FnOpHolder(op2)
		fn.setParameterised(cl)
		cook_failed = False
		try:
			op2.cook() # This should fail because the parameter validation should fail
		except hou.OperationFailed:
			cook_failed = True
		assert(cook_failed)
		assert(op2.errors()!="")
		
	def testInvalidOp(self):
		(op,fn)=self.testOpHolder()
		cl = IECore.ClassLoader.defaultOpLoader().load("noiseDeformer", 1)()
		fn.setParameterised( cl )
		op.createInputNode( 0, "torus" )
		cook_failed = False
		try:
			op.cook() # this should error because torus has no normals
		except hou.OperationFailed:
			cook_failed = True
		assert(cook_failed)
		assert(op.errors()!="")
		
	def setUp( self ) :
		os.environ["IECORE_OP_PATHS"] = "test/ops"
		if not os.path.exists( "test/opHolder_testData" ):
			os.mkdir( "test/opHolder_testData" )
	
	def tearDown( self ) :
		if os.path.exists( "test/opHolder_testData" ):
			shutil.rmtree( "test/opHolder_testData" )
		
if __name__ == "__main__":
	unittest.main()
