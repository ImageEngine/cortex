##########################################################################
#
#  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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
import unittest

import IECore
import IECoreAlembic

class ABCToMDCTest( unittest.TestCase ) :

	def test( self ) :

		IECoreAlembic.ABCToMDC()(
			inputFile = os.path.dirname( __file__ ) + "/data/cube.abc",
			outputFile = "/tmp/test.mdc"
		)

		m = IECore.ModelCache( "/tmp/test.mdc", IECore.IndexedIO.OpenMode.Read )
		self.assertEqual( m.childNames(), [ "group1" ] )
		self.assertEqual( m.readBound(), IECore.Box3d( IECore.V3d( -2 ), IECore.V3d( 2 ) ) )
		self.assertEqual( m.readObject(), None )
		self.assertEqual( m.readTransform(), IECore.M44d() )

		g = m.readableChild( "group1" )
		self.assertEqual( g.readTransform(), IECore.M44d.createScaled( IECore.V3d( 2 ) ) * IECore.M44d.createTranslated( IECore.V3d( 2, 0, 0 ) ) )
		self.assertEqual( g.readObject(), None )
		self.assertEqual( g.childNames(), [ "pCube1" ] )

		t = g.readableChild( "pCube1" )
		self.assertEqual( t.readTransform(), IECore.M44d.createTranslated( IECore.V3d( -1, 0, 0 ) ) )
		self.assertEqual( t.readObject(), None )
		self.assertEqual( t.childNames(), [ "pCubeShape1" ] )

		s = t.readableChild( "pCubeShape1" )
		self.assertEqual( s.readTransform(), IECore.M44d() )
		self.assertEqual( s.readBound(), IECore.Box3d( IECore.V3d( -1 ), IECore.V3d( 1 ) ) )
		self.assertEqual( s.childNames(), [] )

if __name__ == "__main__":
    unittest.main()
