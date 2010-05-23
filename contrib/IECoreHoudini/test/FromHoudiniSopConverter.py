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

class TestFromHoudiniSopConverter( unittest.TestCase ):

	def createBox(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		box = geo.createNode( "box" )
		return box

	def createPoints(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		box = geo.createNode( "box" )
		facet = geo.createNode( "facet" )
		facet.parm("postnml").set(True)
		points = geo.createNode( "scatter" )
		facet.setInput( 0, box )
		points.setInput( 0, facet )
		return points

	# creates a converter
	def testCreateConverter(self):
		box = self.createBox()
		converter = IECoreHoudini.FromHoudiniSopConverter( box )
		assert( converter )
		return converter

	# performs geometry conversion
	def testDoConversion(self):
		converter = self.testCreateConverter()
		result = converter.convert()
		assert( result != None )

	# convert a mesh
	def testConvertMesh(self):
		converter = self.testCreateConverter()
		result = converter.convert()
		# TODO: don't support meshes right now
		#assert( result.typeId() == IECore.MeshPrimitive.staticTypeId() )
		bbox = result.bound()
		target = IECore.Box3f( IECore.V3f(-0.5,-0.5,-0.5), IECore.V3f(0.5,0.5,0.5) )
		assert( bbox == target )

	# convert some points
	def testConvertPoints(self):
		points = self.createPoints()
		converter = IECoreHoudini.FromHoudiniSopConverter( points )
		result = converter.convert()
		assert( result.typeId() == IECore.PointsPrimitive.staticTypeId() )
		assert( points.parm('npts').eval() == result.numPoints )
		assert( "P" in result.keys() )
		assert( "N" in result.keys() )

	# simple attribute conversion
	def testSetupAttributes(self):
		points = self.createPoints()
		geo = points.parent()
		attr = geo.createNode( "attribcreate" )
		attr.setInput( 0, points )
		attr.parm("name").set( "test_attribute" )
		attr.parm("type").set(0) # float
		attr.parm("size").set(1) # 1 element
		attr.parm("value1").set(123.456)
		attr.parm("value2").set(654.321)
		converter = IECoreHoudini.FromHoudiniSopConverter( attr )
		result = converter.convert()
		assert( "test_attribute" in result.keys() )
		assert( result["test_attribute"].data.size() == points.parm('npts').eval() )
		return attr

	# testing point attributes and types
	def testPointAttributes(self):
		attr = self.testSetupAttributes()
		converter = IECoreHoudini.FromHoudiniSopConverter( attr )
		result = converter.convert()
		attr.parm("value1").set(123.456)
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.FloatVectorData )
		assert( result["test_attribute"].data[0] > 123.0 )
		assert( result["test_attribute"].data.size() == 5000 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		attr.parm("type").set(1) # integer
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.IntVectorData )
		assert( result["test_attribute"].data[0] == 123 )
		assert( result["test_attribute"].data.size() == 5000 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		attr.parm("type").set(0) # float
		attr.parm("size").set(2) # 2 elementS
		attr.parm("value2").set(456.789)
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V2fVectorData )
		assert( result["test_attribute"].data[0] == IECore.V2f( 123.456, 456.789 ) )
		assert( result["test_attribute"].data.size() == 5000 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		attr.parm("type").set(1) # int
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V2iVectorData )
		assert( result["test_attribute"].data[0] == IECore.V2i( 123, 456 ) )
		assert( result["test_attribute"].data.size() == 5000 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		attr.parm("type").set(0) # float
		attr.parm("size").set(3) # 3 elements
		attr.parm("value3").set(999.999)
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V3fVectorData )
		assert( result["test_attribute"].data[0] == IECore.V3f( 123.456, 456.789, 999.999 ) )
		assert( result["test_attribute"].data.size() == 5000 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )
		attr.parm("type").set(1) # int
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V3iVectorData )
		assert( result["test_attribute"].data[0] == IECore.V3i( 123, 456, 999 ) )
		assert( result["test_attribute"].data.size() == 5000 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Vertex )

	# testing detail attributes and types
	def testDetailAttributes(self):
		attr = self.testSetupAttributes()
		attr.parm("class").set(0) # detail attribute
		converter = IECoreHoudini.FromHoudiniSopConverter( attr )
		result = converter.convert()
		attr.parm("value1").set(123.456)
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.FloatVectorData )
		assert( result["test_attribute"].data[0] > 123.0 )
		assert( result["test_attribute"].data.size() == 1 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Constant )
		attr.parm("type").set(1) # integer
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.IntVectorData )
		assert( result["test_attribute"].data[0] == 123 )
		assert( result["test_attribute"].data.size() == 1 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Constant )
		attr.parm("type").set(0) # float
		attr.parm("size").set(2) # 2 elementS
		attr.parm("value2").set(456.789)
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V2fVectorData )
		assert( result["test_attribute"].data[0] == IECore.V2f( 123.456, 456.789 ) )
		assert( result["test_attribute"].data.size() == 1 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Constant )
		attr.parm("type").set(1) # int
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V2iVectorData )
		assert( result["test_attribute"].data[0] == IECore.V2i( 123, 456 ) )
		assert( result["test_attribute"].data.size() == 1 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Constant )
		attr.parm("type").set(0) # float
		attr.parm("size").set(3) # 3 elements
		attr.parm("value3").set(999.999)
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V3fVectorData )
		assert( result["test_attribute"].data[0] == IECore.V3f( 123.456, 456.789, 999.999 ) )
		assert( result["test_attribute"].data.size() == 1 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Constant )
		attr.parm("type").set(1) # int
		result = converter.convert()
		assert( result["test_attribute"].data.typeId() == IECore.TypeId.V3iVectorData )
		assert( result["test_attribute"].data[0] == IECore.V3i( 123, 456, 999 ) )
		assert( result["test_attribute"].data.size() == 1 )
		assert( result["test_attribute"].interpolation == IECore.PrimitiveVariable.Interpolation.Constant )

	# testing that float[4] doesn't work!
	def testFloat4attr(self): # we can't deal with float 4's right now
		attr = self.testSetupAttributes()
		attr.parm("name").set( "test_attribute" )
		attr.parm("size").set(4) # 4 elements per point-attribute
		converter = IECoreHoudini.FromHoudiniSopConverter( attr )
		result = converter.convert()
		assert( "test_attribute" not in result.keys() ) # invalid due to being float[4]

	# testing conversion of animating geometry
	def testAnimatingGeometry(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		facet = geo.createNode( "facet" )
		facet.parm("postnml").set(True)
		mountain = geo.createNode( "mountain" )
		mountain.parm("offset1").setExpression( "$FF" )
		points = geo.createNode( "scatter" )
		facet.setInput( 0, torus )
		mountain.setInput( 0, facet )
		points.setInput( 0, mountain )
		converter = IECoreHoudini.FromHoudiniSopConverter( points )
		hou.setFrame(1)
		points_1 = converter.convert()
		hou.setFrame(2)
		points_2 = converter.convert()
		assert( points_1["P"].data != points_2["P"].data )

	# testing we can handle an object being deleted
	def testObjectWasDeleted(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		converter = IECoreHoudini.FromHoudiniSopConverter( torus )
		g1 = converter.convert()
		torus.destroy()
		g2 = converter.convert()
		assert( g2==None )

	def setUp( self ) :                
                os.environ["IECORE_PROCEDURAL_PATHS"] = "test/procedurals"

	def tearDown( self ) :
                pass

if __name__ == "__main__":
    unittest.main()
