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
		torus = op.createOutputNode( "ieCortexConverter" )
		scatter = torus.createOutputNode( "scatter" )
		attr = scatter.createOutputNode( "attribcreate", exact_type_name=True )
		attr.parm("name").set("testAttribute")
		attr.parm("value1").setExpression("$PT")
		to_cortex = attr.createOutputNode( "ieOpHolder" )
		cl = IECoreHoudini.Utils.op("objectDebug",1)
		fn = IECoreHoudini.FnOpHolder(to_cortex)
		fn.setParameterised(cl)
		to_cortex.parm("parm_quiet").set(True)
		to_houdini = to_cortex.createOutputNode("ieCortexConverter")
		geo = to_houdini.geometry()
		attrNames = [ p.name() for p in geo.pointAttribs() ]
		attrNames.sort()
		self.assertEqual( attrNames, ["P", "Pw", "testAttribute"] )
		self.assertEqual( len(geo.points()), 5000 )
		self.assertEqual( len(geo.prims()), 0 )
	
	# check it works for polygons
	def testPolygonConversion(self):
		(op,fn) = self.testCreateToHoudiniConverter()
		torus = op.createOutputNode( "ieCortexConverter" )
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
		converter = holder.createOutputNode( "ieCortexConverter" )
		geo = converter.geometry()
		self.assertEqual( len(geo.points()), 123 )
		self.assertEqual( len(geo.prims()), 0 )

		fn.setProcedural( "meshRender", 1 )
		holder.parm( "parm_path" ).set( "test/IECoreHoudini/data/torus_with_normals.cob" )
		geo = converter.geometry()
		self.assertEqual( len(geo.points()), 100 )
		self.assertEqual( len(geo.prims()), 100 )
		self.assertEqual( sorted([ x.name() for x in geo.pointAttribs() ]), [ "N", "P", "Pw" ] )
		self.assertTrue( geo.findPointAttrib( "N" ).isTransformedAsNormal() )
	
	def testAttributeFilter( self ) :
		
		torus = hou.node("/obj").createNode("geo", run_init_scripts=False).createNode( "torus" )
		color = torus.createOutputNode( "color" )
		color.parm( "class" ).set( 3 )
		color.parm( "colortype" ).set( 2 )
		rest = color.createOutputNode( "rest" )
		scale = rest.createOutputNode( "attribcreate" )
		scale.parm( "name1" ).set( "pscale" )
		scale.parm( "value1v1" ).setExpression( "$PT" )
		uvunwrap = scale.createOutputNode( "uvunwrap" )
		opHolder = uvunwrap.createOutputNode( "ieOpHolder" )
		fn = IECoreHoudini.FnOpHolder( opHolder )
		fn.setOp( "parameters/primitives/polyParam" )
		out = opHolder.createOutputNode( "ieCortexConverter" )
		
		# verify input
		inGeo = uvunwrap.geometry()
		self.assertEqual( sorted([ x.name() for x in inGeo.pointAttribs() ]), ['P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in inGeo.primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in inGeo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in inGeo.globalAttribs() ]), ['varmap'] )
		
		# verifty output
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		# verify intermediate op result
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( result.keys(), [ "Cs", "P", "Pref", "s", "t", "varmap", "width" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		
		# make sure P is forced
		out.parm( "attributeFilter" ).set( "* ^P" )
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		# have to filter the source attrs s, t and not uv
		out.parm( "attributeFilter" ).set( "* ^uv  ^pscale ^rest" )
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		out.parm( "attributeFilter" ).set( "* ^s ^t  ^width ^Pref" )
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		# make sure we can filter on both ends
		opHolder.parm( "parm_input_attributeFilter" ).set( "* ^s ^t  ^width ^Pref" )
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( result.keys(), [ "Cs", "P", "Pref", "s", "t", "varmap", "width" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		opHolder.parm( "parm_input_attributeFilter" ).set( "* ^uv  ^pscale ^rest" )
		opHolder.cook( True )
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( result.keys(), [ "Cs", "P", "varmap" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		# since the vars never made it to the op, the never make it out
		out.parm( "attributeFilter" ).set( "*" )
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
	
	def testStandardAttributeConversion( self ) :
		
		torus = hou.node("/obj").createNode("geo", run_init_scripts=False).createNode( "torus" )
		color = torus.createOutputNode( "color" )
		color.parm( "class" ).set( 3 )
		color.parm( "colortype" ).set( 2 )
		rest = color.createOutputNode( "rest" )
		scale = rest.createOutputNode( "attribcreate" )
		scale.parm( "name1" ).set( "pscale" )
		scale.parm( "value1v1" ).setExpression( "$PT" )
		uvunwrap = scale.createOutputNode( "uvunwrap" )
		opHolder = uvunwrap.createOutputNode( "ieOpHolder" )
		fn = IECoreHoudini.FnOpHolder( opHolder )
		fn.setOp( "parameters/primitives/polyParam" )
		out = opHolder.createOutputNode( "ieCortexConverter" )
		
		# verify input
		inGeo = uvunwrap.geometry()
		self.assertEqual( sorted([ x.name() for x in inGeo.pointAttribs() ]), ['P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in inGeo.primAttribs() ]), [] )
		self.assertEqual( sorted([ x.name() for x in inGeo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in inGeo.globalAttribs() ]), ['varmap'] )
		
		# verifty output
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		# verify intermediate op result
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( result.keys(), [ "Cs", "P", "Pref", "s", "t", "varmap", "width" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["Pref"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		
		sData = result["s"].data
		tData = result["t"].data
		inUvs = inGeo.findVertexAttrib( "uv" )
		outUvs = outGeo.findVertexAttrib( "uv" )
		
		i = 0
		for prim in inGeo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( inUvs )
				self.assertAlmostEqual( sData[i], uvValues[0] )
				self.assertAlmostEqual( tData[i], 1 - uvValues[1] )
				i += 1
		
		i = 0
		for prim in outGeo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( outUvs )
				self.assertAlmostEqual( sData[i], uvValues[0] )
				self.assertAlmostEqual( tData[i], 1 - uvValues[1] )
				i += 1		
		
		# turn off half the conversion
		opHolder.parm( "parm_input_convertStandardAttributes" ).set( False )
		
		# verifty output
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pw', 'pscale', 'rest'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cd', 'uv'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		# verify intermediate op result
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( result.keys(), [ "Cd", "P", "pscale", "rest", "uv", "varmap" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["rest"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		
		uvData = result["uv"].data
		inUvs = inGeo.findVertexAttrib( "uv" )
		outUvs = outGeo.findVertexAttrib( "uv" )
		
		i = 0
		for prim in inGeo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( inUvs )
				self.assertAlmostEqual( uvData[i][0], uvValues[0] )
				self.assertAlmostEqual( uvData[i][1], uvValues[1] )
				i += 1
		
		i = 0
		for prim in outGeo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( outUvs )
				self.assertAlmostEqual( uvData[i][0], uvValues[0] )
				self.assertAlmostEqual( uvData[i][1], uvValues[1] )
				i += 1
		
		# turn off the other half of the conversion
		opHolder.parm( "parm_input_convertStandardAttributes" ).set( True )
		out.parm( "convertStandardAttributes" ).set( False )
		
		# verifty output
		outGeo = out.geometry()
		self.assertEqual( sorted([ x.name() for x in outGeo.pointAttribs() ]), ['P', 'Pref', 'Pw', 'width'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.primAttribs() ]), ["ieMeshInterpolation"] )
		self.assertEqual( sorted([ x.name() for x in outGeo.vertexAttribs() ]), ['Cs', 's', 't'] )
		self.assertEqual( sorted([ x.name() for x in outGeo.globalAttribs() ]), ['varmap'] )
		
		# verify intermediate op result
		result = fn.getOp().resultParameter().getValue()
		self.assertEqual( result.keys(), [ "Cs", "P", "Pref", "s", "t", "varmap", "width" ] )
		self.assertTrue( result.arePrimitiveVariablesValid() )
		self.assertEqual( result["P"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		self.assertEqual( result["Pref"].data.getInterpretation(), IECore.GeometricData.Interpretation.Point )
		
		sData = result["s"].data
		tData = result["t"].data
		inUvs = inGeo.findVertexAttrib( "uv" )
		outS = outGeo.findVertexAttrib( "s" )
		outT = outGeo.findVertexAttrib( "t" )
		
		i = 0
		for prim in inGeo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				uvValues = vert.attribValue( inUvs )
				self.assertAlmostEqual( sData[i], uvValues[0] )
				self.assertAlmostEqual( tData[i], 1 - uvValues[1] )
				i += 1
		
		i = 0
		for prim in outGeo.prims() :
			verts = list(prim.vertices())
			verts.reverse()
			for vert in verts :
				self.assertAlmostEqual( sData[i], vert.attribValue( outS ) )
				self.assertAlmostEqual( tData[i], vert.attribValue( outT ) )
				i += 1		

if __name__ == "__main__":
	unittest.main()
