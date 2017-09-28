##########################################################################
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

import unittest
import weakref
import gc

import hou

import IECore
import IECoreHoudini

class TestNodeHandle( IECoreHoudini.TestCase ) :

	def createBox(self):
		obj = hou.node("/obj")
		geo = obj.createNode("geo", run_init_scripts=False)
		box = geo.createNode( "box" )
		return box

	# testing a class that uses NodeHandle internally can be created
	def testCreation( self ) :

		box = self.createBox()
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( box )
		self.assert_( isinstance( converter, IECoreHoudini.FromHoudiniPolygonsConverter ) )

	# testing deletion of HOM node is irrelevant
	def testDeleteHOMNode( self ) :

		box = self.createBox()
		w = weakref.ref( box )
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( box )

		del box
		gc.collect()

		self.assertEqual( w(), None )
		self.assert_( converter.convert().isInstanceOf( IECore.TypeId.MeshPrimitive ) )

	# testing deletion of node causes converter to return None
	def testDeleteNode( self ) :

		box = self.createBox()
		w = weakref.ref( box )
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( box )
		result = converter.convert()

		box.destroy()
		gc.collect()

		self.assertRaises( Exception, w() )
		self.assertEqual( converter.convert(), result )

	# testing new scene causes converter to return None
	def testNewScene( self ) :

		box = self.createBox()
		w = weakref.ref( box )
		converter = IECoreHoudini.FromHoudiniPolygonsConverter( box )
		result = converter.convert()

		hou.hipFile.clear( False )

		self.assertRaises( Exception, w() )
		self.assertEqual( converter.convert(), result )

if __name__ == "__main__":
    unittest.main()

