##########################################################################
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

import os
import hou
import IECore
import IECoreHoudini
import unittest

class TestToHoudiniCoverterOp( IECoreHoudini.TestCase ):
	
	# make sure we can create the op
	def testCreateToHoudiniConverter(self)  :
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		op = geo.createNode( "ieOpHolder" )
		cl = IECore.ClassLoader.defaultOpLoader().load( "cobReader", 1)()
		cl['filename'] = "test/IECoreHoudini/data/torus.cob"
		fn = IECoreHoudini.FnOpHolder(op)
		fn.setParameterised(cl)
		IECoreHoudini.Utils.syncSopParametersWithOp(op)
		op.cook()
		self.assertEqual( cl.resultParameter().getValue().typeId(), IECore.TypeId.MeshPrimitive )
		return (op, fn)
	
	# check it works for points
	def testPointConversion(self):
		(op,fn) = self.testCreateToHoudiniConverter()
		torus = op.createOutputNode( "ieToHoudiniConverter" )
		scatter = torus.createOutputNode( "scatter" )
		attr = scatter.createOutputNode( "attribcreate", exact_type_name=True )
		attr.parm("name").set("testAttribute")
		attr.parm("value1").setExpression("$PT")
		to_cortex = attr.createOutputNode( "ieOpHolder" )
		cl = IECoreHoudini.Utils.op("objectDebug",1)
		fn = IECoreHoudini.FnOpHolder(to_cortex)
		fn.setParameterised(cl)
		to_cortex.parm("parm_quiet").set(True)
		to_houdini = to_cortex.createOutputNode("ieToHoudiniConverter")
		geo = to_houdini.geometry()
		attrNames = [ p.name() for p in geo.pointAttribs() ]
		attrNames.sort()
		self.assertEqual( attrNames, ["P", "Pw", "testAttribute"] )
		self.assertEqual( len(geo.points()), 5000 )
		self.assertEqual( len(geo.prims()), 0 )
	
	# check it works for polygons
	def testPolygonConversion(self):
		(op,fn) = self.testCreateToHoudiniConverter()
		torus = op.createOutputNode( "ieToHoudiniConverter" )
		geo = torus.geometry()
		self.assertEqual( len(geo.points()), 100 )
		self.assertEqual( len(geo.prims()), 100 )
		attrNames = [ p.name() for p in geo.pointAttribs() ]
		attrNames.sort()
		self.assertEqual( attrNames, ["P", "Pw"] )
		for p in geo.prims():
			self.assertEqual( p.numVertices(), 4 )
			self.assertEqual( p.type(), hou.primType.Polygon )
		n = hou.node("/obj/geo1")
		h_torus = n.createNode( "torus" )
		h_geo = h_torus.geometry()
		self.assertEqual( len(geo.pointAttribs()), len(h_geo.pointAttribs()) )
		self.assertEqual( len(geo.prims()), len(h_geo.prims()) )
	
	# test converting a procedural
	def testProceduralConversion( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		holder = geo.createNode( "ieProceduralHolder" )
		fn = IECoreHoudini.FnProceduralHolder( holder )
		fn.setProcedural( "pointRender", 1 )
		holder.parm( "parm_npoints" ).set( 123 )
		converter = holder.createOutputNode( "ieToHoudiniConverter" )
		geo = converter.geometry()
		self.assertEqual( len(geo.points()), 123 )
		self.assertEqual( len(geo.prims()), 0 )

		fn.setProcedural( "meshRender", 1 )
		holder.parm( "parm_path" ).set( "test/IECoreHoudini/data/torus_with_normals.cob" )
		geo = converter.geometry()
		self.assertEqual( len(geo.points()), 100 )
		self.assertEqual( len(geo.prims()), 100 )
		self.assertEqual( geo.pointAttribs()[-1].name(), "N" )

if __name__ == "__main__":
	unittest.main()
