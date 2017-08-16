##########################################################################
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

import unittest
import os.path
import shutil

import IECore
import IECoreImage

import IECoreGL
IECoreGL.init( False )

class CoordinateSystemTest( unittest.TestCase ) :

	__outputFileName = os.path.dirname( __file__ ) + "/output/testCoordinateSystem.tif"

	def testNoVisualisation( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.__outputFileName, "tif", "rgba", {} )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			r.coordinateSystem( "myCoordSys" )

		i = IECore.Reader.create( self.__outputFileName ).read()

		a = i["A"]
		for i in range( a.size() ) :
			self.assertEqual( a[i], 0 )
			
	def testVisualisation( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "immediate" ) )

		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			}
		)
		r.display( self.__outputFileName, "tif", "rgba", {} )

		r.setOption( "gl:drawCoordinateSystems", IECore.BoolData( True ) )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
			r.coordinateSystem( "myCoordSys" )

		i = IECore.Reader.create( self.__outputFileName ).read()

		a = i["A"]
		self.failUnless( a[127*256 + 127] > 0 )
		self.failUnless( a[127*256 + 255] > 0 )
		self.failUnless( a[127] > 0 )
		self.assertEqual( a[200*127 + 127], 0 )
		self.assertEqual( a[120*127 + 127], 0 )
			
	def setUp( self ) :
		
		if not os.path.isdir( "test/IECoreGL/output" ) :
			os.makedirs( "test/IECoreGL/output" )
	
	def tearDown( self ) :
		
		if os.path.isdir( "test/IECoreGL/output" ) :
			shutil.rmtree( "test/IECoreGL/output" )

if __name__ == "__main__":
    unittest.main()
