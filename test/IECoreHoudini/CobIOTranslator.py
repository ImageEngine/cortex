##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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
import imath
import IECore
import IECoreScene
import IECoreHoudini
import unittest
import os
import shutil

class TestCobIOTranslator( IECoreHoudini.TestCase ) :

	__testDir = "test/cobIO"
	__testFile = "%s/testCobIO.cob" % __testDir
	__testPDCFile = "%s/testPdcIO.pdc" % __testDir
	__testPTCFile = "%s/testPtcIO.ptc" % __testDir

	def torus( self ) :
		obj = hou.node( "/obj" )
		geo = obj.createNode( "geo", run_init_scripts=False )
		torus = geo.createNode( "torus" )
		facet = torus.createOutputNode( "facet" )
		facet.parm( "postnml" ).set( True )
		mountain = facet.createOutputNode( "mountain" )
		if hou.applicationVersion()[0] >= 16:
			mountain.parm("offsetx").setExpression("$FF")
		else:
			mountain.parm("offset1").setExpression( "$FF" )

		return mountain

	def points( self, inNode=None ) :
		if not inNode :
			inNode = self.torus()

		return inNode.createOutputNode( "scatter" )

	def curves( self, inNode=None ) :
		points = self.points( inNode )
		add = points.createOutputNode( "add" )
		add.parm( "stdswitcher1" ).set( 1 )
		add.parm( "switcher1" ).set( 1 )
		convert = add.createOutputNode( "convert" )
		convert.parm( "totype" ).set( 4 )

		return convert

	def reader( self ) :
		geo = hou.node( "/obj/geo1" )
		if not geo :
			obj = hou.node( "/obj" )
			geo = obj.createNode( "geo", run_init_scripts=False )

		reader = geo.createNode( "file" )
		reader.parm( "file" ).set( TestCobIOTranslator.__testFile )
		reader.parm( "filemode" ).set( 1 )

		return reader

	def writer( self, inNode ) :
		writer = inNode.createOutputNode( "file" )
		writer.parm( "file" ).set( TestCobIOTranslator.__testFile )
		writer.parm( "filemode" ).set( 2 )

		return writer

	def testReadWritePoints( self ) :
		points = self.points()
		writer = self.writer( points )
		reader = self.reader()

		self.assert_( not reader.geometry() )
		self.assert_( reader.errors() )
		writer.cook()
		self.assert_( reader.geometry() )
		self.assert_( not reader.errors() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( reader )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		result = converter.convert()
		self.assert_( result.isInstanceOf( IECore.TypeId( IECoreScene.TypeId.PointsPrimitive ) ) )

	def testReadWriteMesh( self ) :
		mesh = self.torus()
		writer = self.writer( mesh )
		reader = self.reader()

		self.assert_( not reader.geometry() )
		self.assert_( reader.errors() )
		writer.cook()
		self.assert_( reader.geometry() )
		self.assert_( not reader.errors() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( reader )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPolygonsConverter ) ) )
		result = converter.convert()
		self.assert_( result.isInstanceOf( IECore.TypeId( IECoreScene.TypeId.MeshPrimitive ) ) )

	def testReadWriteCurves( self ) :
		curves = self.curves()
		writer = self.writer( curves )
		reader = self.reader()

		self.assert_( not reader.geometry() )
		self.assert_( reader.errors() )
		writer.cook()
		self.assert_( reader.geometry() )
		self.assert_( not reader.errors() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( reader )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniCurvesConverter ) ) )
		result = converter.convert()
		self.assert_( result.isInstanceOf( IECore.TypeId( IECoreScene.TypeId.CurvesPrimitive ) ) )

	def testCantReadBadCob( self ) :
		writer = self.writer( self.torus() )
		reader = self.reader()

		self.assert_( not reader.geometry() )
		self.assert_( reader.errors() )
		writer.cook()
		self.assert_( reader.geometry() )
		self.assert_( not reader.errors() )

		f = file( "testCobIO.cob", "w" )
		f.write( "this is not a real cob" )
		f.close()

		reader.parm( "file" ).set( "testCobIO.cob" )
		self.assert_( not reader.geometry() )
		self.assert_( reader.errors() )

		os.remove( "testCobIO.cob" )

	def testCobWithNonPrimitiveData( self ) :
		IECore.ObjectWriter( imath.V3f( 1 ), TestCobIOTranslator.__testFile ).write()
		reader = self.reader()
		geo = reader.geometry()
		prims = geo.prims()
		self.assertFalse( reader.errors() )
		self.assertEqual( len(prims), 1 )
		self.assertEqual( prims[0].type(), hou.primType.Custom )
		self.assertEqual( prims[0].vertices()[0].point().number(), 0 )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( reader )
		self.assertTrue( isinstance( converter, IECoreHoudini.FromHoudiniCortexObjectConverter ) )
		result = converter.convert()
		self.assertEqual( result, IECore.V3fData( imath.V3f( 1 ) ) )

	def testReadWritePDC( self ) :
		points = self.points()
		writer = self.writer( points )
		writer.parm( "file" ).set( TestCobIOTranslator.__testPDCFile )
		reader = self.reader()
		reader.parm( "file" ).set( TestCobIOTranslator.__testPDCFile )

		self.assert_( not reader.geometry() )
		self.assert_( reader.errors() )
		writer.cook()
		self.assert_( reader.geometry() )
		self.assert_( not reader.errors() )

		converter = IECoreHoudini.FromHoudiniGeometryConverter.create( reader )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreHoudini.TypeId.FromHoudiniPointsConverter ) ) )
		result = converter.convert()
		self.assert_( result.isInstanceOf( IECore.TypeId( IECoreScene.TypeId.PointsPrimitive ) ) )

	def setUp( self ) :
		IECoreHoudini.TestCase.setUp( self )
		if not os.path.exists( TestCobIOTranslator.__testDir ) :
			os.mkdir( TestCobIOTranslator.__testDir )

	def tearDown( self ) :
		if os.path.exists( TestCobIOTranslator.__testDir ) :
			shutil.rmtree( TestCobIOTranslator.__testDir )

if __name__ == "__main__":
    unittest.main()
