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
		self.assertEqual( res.keys(), ["Cs","P","s","t"] )
		self.assert_( geo )
		self.assert_( torus )
		self.assert_( point )
		self.assert_( attribute )
		self.assert_( op )
		self.assert_( fn )
		self.assert_( fn )
		return (op, attribute)
	
	def testNormalBehaviour(self):
		(op, attr) = self.testCreateObjects()
		op.cook()
		fn = IECoreHoudini.FnOpHolder( op )
		self.assert_( fn )
		cl = fn.getParameterised()
		object = cl.resultParameter().getValue()
		self.assert_( object )
		self.failUnless( 'P' in object )
		self.assertEqual( object['P'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( object['P'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.failUnless( 'Cs' in object )
		self.assertEqual( object['Cs'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( object['Cs'].data.typeId(), IECore.TypeId.Color3fVectorData )
		
	def testBasicRemapping(self):
		(op, attr) = self.testCreateObjects()
		attr.parm("ridefault").set(True)
		op.cook()
		fn = IECoreHoudini.FnOpHolder( op )
		self.assert_( fn )
		cl = fn.getParameterised()
		object = cl.resultParameter().getValue()
		self.assertEqual( object.keys(), ['Cs', 'P', 'rixlate', 's', 't'] )
		self.assert_( object )
		self.failUnless( 'P' in object )
		self.failUnless( object.arePrimitiveVariablesValid() )
		self.assertEqual( object['P'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( object['P'].data.typeId(), IECore.TypeId.V3fVectorData )
		self.failUnless( 'Cs' in object )
		self.assertEqual( object['Cs'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex ) # the default ri conversion sets this as vertex
		self.assertEqual( object['Cs'].data.typeId(), IECore.TypeId.Color3fVectorData )
		self.failUnless( 'rixlate' in object )
		self.assertEqual( object['rixlate'].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( object['rixlate'].data.typeId(), IECore.TypeId.StringData )
		self.failUnless( 's' in object )
		self.assertEqual( object['s'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( object['s'].data.typeId(), IECore.TypeId.FloatVectorData )
		self.failUnless( 't' in object )
		self.assertEqual( object['t'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( object['t'].data.typeId(), IECore.TypeId.FloatVectorData )
		
	def testMappingManual(self):
		(op, attr) = self.testCreateObjects()
		attr.parm("hname0").set("Cd")
		attr.parm("riname0").set("col")
		attr.parm("ritype0").set("v_color")
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( geo.typeId(), IECore.TypeId.MeshPrimitive )
		self.assertEqual( geo.keys(), ["P", "col", "rixlate", "s", "t"] )
		self.assertEqual( geo['col'].interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( geo['col'].data.typeId(), IECore.TypeId.Color3fVectorData )

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
		self.assertEqual( geo.typeId(), IECore.TypeId.MeshPrimitive )
		self.failUnless( "P" in geo.keys() )
		self.failUnless( "col_r" in geo.keys() )
		self.failUnless( "col_g" in geo.keys() )
		self.failUnless( "col_b" in geo.keys() )
		self.failUnless( "s" in geo.keys() )
		self.failUnless( "t" in geo.keys() )
		self.assertEqual( geo['col_r'].interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( geo['col_r'].data.typeId(), IECore.TypeId.FloatVectorData )
		self.assertNotEqual( geo['col_r'].data[0], geo['col_r'].data[1] )
		self.assertEqual( geo['col_g'].interpolation, IECore.PrimitiveVariable.Interpolation.Varying )
		self.assertEqual( geo['col_g'].data.typeId(), IECore.TypeId.FloatVectorData )
		self.assertEqual( geo['col_g'].data[0], geo['col_g'].data[1], 0.75 )
		self.assertEqual( geo['col_b'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( geo['col_b'].data.typeId(), IECore.TypeId.FloatVectorData )
		self.assertEqual( geo['col_b'].data[0], geo['col_b'].data[1], 0.5 )

	def testDuplicateNaming(self):
		(op, attr) = self.testCreateObjects()
		top = attr.inputs()[0]
		attr1 = top.createOutputNode( "attribcreate", exact_type_name=True )
		attr1.parm("name").set("test")
		attr2 = attr1.createOutputNode( "attribcreate", exact_type_name=True )
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
		self.assertEqual( geo.keys(), ['Cs', 'P', 'point_test', 'prim_test', 'rixlate', 's', 't', 'varmap'] )
		self.assertEqual( geo['point_test'].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( len(geo['point_test'].data), 100 )
		self.assertEqual( geo['prim_test'].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( len(geo['prim_test'].data), 100 )
		
	def testPrimAttributes(self):
		(op, attr) = self.testCreateObjects()
		top = attr.inputs()[0]
		attr1 = top.createOutputNode( "attribcreate", exact_type_name=True )
		attr1.parm("class").set(1) # primitive
		attr1.parm("name").set("test")
		attr.setInput(0,attr1)
		attr.parm("hname0").set( "test" )
		attr.parm("riname0").set( "test" ) # a uniform prim float
		attr.parm("ritype0").set("u_float")
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( geo.keys(), ['Cs', 'P', 'rixlate', 's', 't', 'test', 'varmap'] )
		self.assertEqual( geo['test'].interpolation, IECore.PrimitiveVariable.Interpolation.Uniform )
		self.assertEqual( len(geo['test'].data), 100 )
		attr.parm("hname0").set( "test" )
		attr.parm("riname0").set( "test" ) # a constant prim float
		attr.parm("ritype0").set("c_float")
		op.cook()
		fn = IECoreHoudini.FnOpHolder(op)
		geo = fn.getParameterised().resultParameter().getValue()
		self.assertEqual( geo.keys(), ['Cs', 'P', 'rixlate', 's', 't', 'test', 'varmap'] )
		self.assertEqual( geo['test'].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( len(geo['test'].data), 100 )
		
if __name__ == "__main__":
	unittest.main()
