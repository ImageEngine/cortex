##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreRI

class RIBWriterTest( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/ribWriter.rib"

	def test( self ) :

		self.assertEqual( IECoreRI.RIBWriter.baseTypeName(), "Writer" )

		cube = IECore.ObjectReader( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()

		writer = IECoreRI.RIBWriter( cube, self.outputFileName )
		writer.write()

		l = "".join( file( self.outputFileName ).readlines() )
		self.assert_( "PointsGeneralPolygons" in l )
		self.assert_( "WorldBegin" not in l )
		self.assert_( "WorldEnd" not in l )

	def testWithWorld( self ) :

		cube = IECore.ObjectReader( "test/IECore/data/cobFiles/pCubeShape1.cob" ).read()

		writer = IECoreRI.RIBWriter( cube, self.outputFileName )

		writer["worldBlock"].setTypedValue( True )
		writer.write()

		l = "".join( file( self.outputFileName ).readlines() )
		self.assert_( "PointsGeneralPolygons" in l )
		self.assert_( "WorldBegin" in l )
		self.assert_( "WorldEnd" in l )

	def tearDown( self ) :

		if os.path.exists( self.outputFileName ) :
			os.remove( self.outputFileName )

if __name__ == "__main__":
    unittest.main()
