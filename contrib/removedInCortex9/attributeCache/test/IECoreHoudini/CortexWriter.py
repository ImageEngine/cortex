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
import glob
import shutil

class TestCortexWriter( IECoreHoudini.TestCase ):

	# test we can create a cortex_writer
	def testCreateWriter(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		torus = geo.createNode( "torus" )
		facet = geo.createNode( "facet" )
		facet.parm("postnml").set(True)
		mountain = geo.createNode( "mountain" )
		mountain.parm("offset1").setExpression( "$FF" )
		points = geo.createNode( "scatter" )
		writer = geo.createNode( "cortexWriter" )
		self.assert_( writer )
		facet.setInput( 0, torus )
		mountain.setInput( 0, facet )
		points.setInput( 0, mountain )
		writer.setInput( 0, points )
		return writer

	def render(self, writer, range):
		writer.parm("cache_geometry_to_disk_trange").set(1)
		writer.parm("cache_geometry_to_disk_f1").set(range[0])
		writer.parm("cache_geometry_to_disk_f2").set(range[1])
		writer.parm("cache_geometry_to_disk_execute").pressButton()

	# test the cortex_writer works writing a serialised object (COB)
	def testRenderWriterCOB(self):
		cache_file = "test/cortexWriter_testData/test.$F4.cob"
		writer = self.testCreateWriter()
		# set to cob
		writer.parm("export_format").set(1)
		# set path
		writer.parm("export_path").set(cache_file)
		# render
		self.render(writer, (1,100) )
		# check files
		files = glob.glob( "test/cortexWriter_testData/test.????.cob" )
		self.assertEqual( len(files), 100 )
		# load cob
		geo_1 = IECore.Reader.create( "test/cortexWriter_testData/test.0001.cob" ).read()
		geo_2 = IECore.Reader.create( "test/cortexWriter_testData/test.0051.cob" ).read()
		geo_3 = IECore.Reader.create( "test/cortexWriter_testData/test.0100.cob" ).read()
		# compare with a converter
		hou.setFrame(1)
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( writer.inputs()[0] )
		self.assertEqual( geo_1, converter.convert() )
		self.assertNotEqual( geo_2, converter.convert() )
		hou.setFrame(51)
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( writer.inputs()[0] )
		self.assertEqual( geo_2, converter.convert() )
		hou.setFrame(100)
		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( writer.inputs()[0] )
		self.assertEqual( geo_3, converter.convert() )

	# test the cortex_writer works writing an attribute cache (FIO)
	def testRenderWriterFIO(self):
		cache_file = "test/cortexWriter_testData/test.$F4.fio"
		writer = self.testCreateWriter()
		# set to fio
		writer.parm("export_format").set(0)
		# set path
		writer.parm("export_path").set(cache_file)
		# render
		self.render(writer, (1,100) )
		# load fio
		files = glob.glob( "test/cortexWriter_testData/test.????.fio" )
		self.assertEqual( len(files), 100 )
		# check objects
		ac = IECore.AttributeCache( "test/cortexWriter_testData/test.0001.fio", IECore.IndexedIO.OpenMode.Read )
		self.failUnless( "scatter1" in ac.objects() )
		p_1 = ac.read( "scatter1", "P" )
		n_1 = ac.read( "scatter1", "N" )
		self.assert_( p_1 )
		self.assertEqual( p_1.size(), 5000 )
		n_1 = ac.read( "scatter1", "N" )
		self.assert_( n_1 )
		self.assertEqual( n_1.size(), 5000 )
		ac = IECore.AttributeCache( "test/cortexWriter_testData/test.0100.fio", IECore.IndexedIO.OpenMode.Read )
		self.failUnless( "scatter1" in ac.objects() )
		p_2 = ac.read( "scatter1", "P" )
		n_2 = ac.read( "scatter1", "N" )
		self.assert_( p_2 )
		self.assertEqual( p_2.size(), 5000 )
		self.assertNotEqual( p_1, p_2 )
		n_2 = ac.read( "scatter1", "N" )
		self.assert_( n_2 )
		self.assertEqual( n_2.size(), 5000 )
		self.assertNotEqual( n_1, n_2 )
		# compare with a converter

	# test the cortex_writer works specifying an alternate object name
	# when writing an attribute cache (FIO)
	def testRenderWriterFIOAlternateName(self):
		cache_file = "test/cortexWriter_testData/test2.$F4.fio"
		writer = self.testCreateWriter()
		# set to fio
		writer.parm("export_format").set(0)
		# set path
		writer.parm("export_path").set(cache_file)
		# set alternate name
		writer.parm("name_object_from_sop").set(False)
		writer.parm("alternate_name").set("bob")
		# render
		self.render(writer, (1,100) )
		# load fio
		files = glob.glob( "test/cortexWriter_testData/test2.????.fio" )
		self.assertEqual( len(files), 100 )
		# check objects
		ac = IECore.AttributeCache( "test/cortexWriter_testData/test2.0001.fio", IECore.IndexedIO.OpenMode.Read )
		self.assert_( "scatter1" not in ac.objects() )
		self.failUnless( "bob" in ac.objects() )
		p_1 = ac.read( "bob", "P" )
		self.assert_( p_1 )
		self.assertEqual( p_1.size(), 5000 )
		n_1 = ac.read( "bob", "N" )
		self.assert_( n_1 )
		self.assertEqual( n_1.size(), 5000 )
		ac = IECore.AttributeCache( "test/cortexWriter_testData/test2.0100.fio", IECore.IndexedIO.OpenMode.Read )
		self.assert_( "scatter1" not in ac.objects() )
		self.failUnless( "bob" in ac.objects() )
		p_2 = ac.read( "bob", "P" )
		self.assert_( p_2 )
		self.assertEqual( p_2.size(), 5000 )
		self.assertNotEqual( p_1, p_2 )
		n_2 = ac.read( "bob", "N" )
		self.assert_( n_2 )
		self.assertEqual( n_2.size(), 5000 )
		self.assertNotEqual( n_1, n_2 )

	def setUp( self ) :
                IECoreHoudini.TestCase.setUp( self )
                if not os.path.exists( "test/cortexWriter_testData" ):
			os.mkdir( "test/cortexWriter_testData" )

	def tearDown( self ) :
                if os.path.exists( "test/cortexWriter_testData" ):
                        shutil.rmtree( "test/cortexWriter_testData" )

if __name__ == "__main__":
    unittest.main()
