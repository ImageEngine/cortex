##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

import os
import hou
import IECore
import IECoreHoudini
import unittest

class TestAttributeRemap( IECoreHoudini.TestCase ):

	def testCreateObjects(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		tex = torus.createOutputNode( "texture" )
		point = tex.createOutputNode( "point" )
		point.parm("doclr").set(1)
		point.parm("diffr").setExpression( "rand($PT)" )
		point.parm("diffg").setExpression( "0.75" )
		point.parm("diffb").setExpression( "0.5" )
		attribute = point.createOutputNode( "attribute" )
		op = attribute.createOutputNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load("objectDebug", 1)()
		fn = IECoreHoudini.FnOpHolder( op )
		fn.setParameterised( cl )
		op.parm("parm_quiet").set(True)
		op.cook()
		res = cl.resultParameter().getValue()
		assert( res.keys()==["Cd","P","uv"] )
		assert( geo )
		assert( torus )
		assert( point )
		assert( attribute )
		assert( op )
		assert( fn )
		assert( fn )
		return (op, attribute)
	
	def testNormalBehaviour(self):
		(op, attr) = self.testCreateObjects()
		op.cook()
		fn = IECoreHoudini.FnOpHolder( op )
		assert( fn )
		cl = fn.getParameterised()
		object = cl.resultParameter().getValue()
		assert( object )
		assert( 'P' in object )
		assert( object['P'].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		assert( object['P'].data.typeId() == IECore.TypeId.V3fVectorData )
		assert( 'Cd' in object )
		assert( object['Cd'].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		assert( object['Cd'].data.typeId() == IECore.TypeId.V3fVectorData )
		
	def testBasicRemapping(self):
		(op, attr) = self.testCreateObjects()
		attr.parm("ridefault").set(True)
		op.cook()
		fn = IECoreHoudini.FnOpHolder( op )
		assert( fn )
		cl = fn.getParameterised()
		object = cl.resultParameter().getValue()
		self.assertEqual( object.keys(), ['Cs', 'P', 'rixlate', 's', 't'] )
		assert( object )
		assert( 'P' in object )
		assert( object['P'].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		assert( object['P'].data.typeId() == IECore.TypeId.V3fVectorData )
		assert( 'Cs' in object )
		assert( object['Cs'].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex ) # the default ri conversion sets this as vertex
		assert( object['Cs'].data.typeId() == IECore.TypeId.Color3fVectorData )
		self.failUnless( 'rixlate' in object )
		self.assertEqual( object['rixlate'].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( object['rixlate'].data.typeId(), IECore.TypeId.StringData )
		assert( 's' in object )
		assert( object['s'].interpolation == IECore.PrimitiveVariable.Interpolation.Varying )
		assert( object['s'].data.typeId() == IECore.TypeId.FloatVectorData )
		assert( 't' in object )
		assert( object['t'].interpolation == IECore.PrimitiveVariable.Interpolation.Varying )
		assert( object['t'].data.typeId() == IECore.TypeId.FloatVectorData )
		
	def testMappingManual(self):
		(op, attr) = self.testCreateObjects()
		attr.parm("hname0").set("Cd")
		attr.parm("riname0").set("col")
		attr.parm("ritype0").set("v_color")
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		assert(geo.typeId()==IECore.TypeId.MeshPrimitive)
		self.assertEqual( geo.keys(), ["P", "col", "rixlate", "uv"] )
		assert(geo['col'].interpolation == IECore.PrimitiveVariable.Interpolation.Varying )
		assert(geo['col'].data.typeId() == IECore.TypeId.Color3fVectorData )

	def testMappingOffsets(self):
		(op, attr) = self.testCreateObjects()
		attr.parm("hname0").set("Cd")
		attr.parm("riname0").set("col_r")
		attr.parm("ritype0").set("v_float")
		attr.parm("rioff0").set(0)
		attr.parm("hname1").set("Cd")
		attr.parm("riname1").set("col_g")
		attr.parm("ritype1").set("v_float")
		attr.parm("rioff1").set(1)
		attr.parm("hname2").set("Cd")
		attr.parm("riname2").set("col_b")
		attr.parm("ritype2").set("vtx_float") # try a vertex interpolation
		attr.parm("rioff2").set(2)
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		assert(geo.typeId()==IECore.TypeId.MeshPrimitive)
		assert("P" in geo.keys())
		assert("col_r" in geo.keys())
		assert("col_g" in geo.keys())
		assert("col_b" in geo.keys())
		assert("uv" in geo.keys())
		assert(geo['col_r'].interpolation == IECore.PrimitiveVariable.Interpolation.Varying )
		assert(geo['col_r'].data.typeId() == IECore.TypeId.FloatVectorData )
		assert(geo['col_r'].data[0]!=geo['col_r'].data[1])
		assert(geo['col_g'].interpolation == IECore.PrimitiveVariable.Interpolation.Varying )
		assert(geo['col_g'].data.typeId() == IECore.TypeId.FloatVectorData )
		assert(geo['col_g'].data[0]==geo['col_g'].data[1]==0.75)
		assert(geo['col_b'].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		assert(geo['col_b'].data.typeId() == IECore.TypeId.FloatVectorData )
		assert(geo['col_b'].data[0]==geo['col_b'].data[1]==0.5)

	def testDuplicateNaming(self):
		(op, attr) = self.testCreateObjects()
		top = attr.inputs()[0]
		attr1 = top.createOutputNode( "attribcreate" )
		attr1.parm("name").set("test")
		attr2 = attr1.createOutputNode( "attribcreate" )
		attr2.parm("class").set(1) # primitive
		attr2.parm("name").set("test")
		attr.setInput(0,attr2)
		attr.parm("hname0").set( "test" )
		attr.parm("riname0").set( "point_test" ) # a vertex point float
		attr.parm("ritype0").set("vtx_float")
		attr.parm("hname1").set( "test" )
		attr.parm("riname1").set( "prim_test" ) # a uniform primitive float
		attr.parm("ritype1").set("u_float")
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( geo.keys(), ['Cd', 'P', 'point_test', 'prim_test', 'rixlate', 'uv', 'varmap'] )
		assert( geo['point_test'].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		assert( len(geo['point_test'].data)==100 )
		assert( geo['prim_test'].interpolation == IECore.PrimitiveVariable.Interpolation.Uniform )
		assert( len(geo['prim_test'].data)==100 )
		
	def testPrimAttributes(self):
		(op, attr) = self.testCreateObjects()
		top = attr.inputs()[0]
		attr1 = top.createOutputNode( "attribcreate" )
		attr1.parm("class").set(1) # primitive
		attr1.parm("name").set("test")
		attr.setInput(0,attr1)
		attr.parm("hname0").set( "test" )
		attr.parm("riname0").set( "test" ) # a uniform prim float
		attr.parm("ritype0").set("u_float")
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( geo.keys(), ['Cd', 'P', 'rixlate', 'test', 'uv', 'varmap'] )
		assert( geo['test'].interpolation == IECore.PrimitiveVariable.Interpolation.Uniform )
		assert( len(geo['test'].data)==100 )
		attr.parm("hname0").set( "test" )
		attr.parm("riname0").set( "test" ) # a constant prim float
		attr.parm("ritype0").set("c_float")
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( geo.keys(), ['Cd', 'P', 'rixlate', 'test', 'uv', 'varmap'] )
		assert( geo['test'].interpolation == IECore.PrimitiveVariable.Interpolation.Constant )
		assert( len(geo['test'].data)==100 )
		
	def setUp( self ) :
		IECoreHoudini.TestCase.setUp( self )
		os.environ["IECORE_OP_PATHS"] = "test/ops"
	
	def tearDown( self ) :
		pass
	
if __name__ == "__main__":
	unittest.main()
