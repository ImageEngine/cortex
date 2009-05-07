##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.cmds

class FromMayaConverterTest( unittest.TestCase ) :

	def testFactory( self ) :
		
		sphereTransform = maya.cmds.polySphere()[0]
		sphereShape = maya.cmds.listRelatives( sphereTransform, shapes=True )[0]
		
		# get a converter for a plug
		converter = IECoreMaya.FromMayaConverter.create( str( sphereTransform ) + ".translateX" )
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaPlugConverter.staticTypeId() ) )
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaUnitPlugConverterd.staticTypeId() ) )
		
		converter = IECoreMaya.FromMayaConverter.create( str( sphereTransform ) + ".translateX", IECore.TypeId.FloatData )
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaPlugConverter.staticTypeId() ) )

		# get a converter for a dag node
		converter = IECoreMaya.FromMayaConverter.create( str( sphereTransform ) )
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaDagNodeConverter.staticTypeId() ) )
		
		converter = IECoreMaya.FromMayaConverter.create( str( sphereTransform ), IECore.TypeId.Group )
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaDagNodeConverter.staticTypeId() ) )
		
		# get a converter for a shape node
		converter = IECoreMaya.FromMayaConverter.create( str( sphereShape ) )
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaShapeConverter.staticTypeId() ) )
		
		converter = IECoreMaya.FromMayaConverter.create( str( sphereShape ), IECore.TypeId.MeshPrimitive )
		self.assert_( converter.isInstanceOf( IECoreMaya.FromMayaShapeConverter.staticTypeId() ) )

	def testTransformationMatrixConverter( self ):

		sphereTransform = maya.cmds.polySphere()[0]
		radToAng = 180./3.14159265
		maya.cmds.setAttr( str( sphereTransform ) + ".rotate", -1000*radToAng, 30*radToAng, 100*radToAng, type="double3" )

		## \todo This section sometimes fails due to use of the unpredictable default conversion (see FromMayaObjectConverter.h
		# for a description). We probably need to remove the default conversion so that people aren't exposed to it, or fix it
		# so we can define which the default conversion is.
		converter = IECoreMaya.FromMayaConverter.create( str( sphereTransform ) )
		res = converter.convert()
		self.assert_( not res.isInstanceOf( IECore.TransformationMatrixfData.staticTypeId() ) )
		self.assert_( not res.isInstanceOf( IECore.TransformationMatrixdData.staticTypeId() ) )

		# test TransformationMatrixData converter
		converter = IECoreMaya.FromMayaConverter.create( str( sphereTransform ), IECore.TypeId.TransformationMatrixfData )
		self.assert_( converter )
		transform = converter.convert()
		self.assert_( transform.isInstanceOf( IECore.TransformationMatrixfData.staticTypeId() ) )
		self.assertAlmostEqual( (transform.value.rotate - IECore.Eulerf( -1000, 30, 100 )).length(), 0, 2 )

if __name__ == "__main__":
	MayaUnitTest.TestProgram()
