##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

from IECore import *

class TestCamera( unittest.TestCase ) :

	def test( self ) :
	
		c = Camera()
		self.assertEqual( c.getName(), "default" )
		self.assertEqual( c.getTransform(), None )
		self.assertEqual( c.parameters(), CompoundData() )

		cc = c.copy()
		self.assertEqual( cc.getName(), "default" )
		self.assertEqual( cc.getTransform(), None )
		self.assertEqual( cc.parameters(), CompoundData() )
		self.assertEqual( cc, c )
		
		Writer.create( cc, "test/data/camera.cob" ).write()
		ccc = Reader.create( "test/data/camera.cob" ).read()
		
		self.assertEqual( c, ccc )
		
		c.setName( "n" )
		self.assertEqual( c.getName(), "n" )
		
		c.setTransform( MatrixTransform( M44f.createScaled( V3f( 2 ) ) ) )
		self.assertEqual( c.getTransform(), MatrixTransform( M44f.createScaled( V3f( 2 ) ) ) )
		
		c.parameters()["fov"] = FloatData( 45 )
		self.assertEqual( c.parameters()["fov"], FloatData( 45 ) )
		
		# test copying and saving with some parameters and a transform
		cc = c.copy()
		self.assertEqual( cc, c )
		
		Writer.create( cc, "test/data/camera.cob" ).write()
		ccc = Reader.create( "test/data/camera.cob" ).read()
		self.assertEqual( ccc, c )
		
	def tearDown( self ) :
	
		if os.path.isfile( "test/data/camera.cob" ) :
			os.remove( "test/data/camera.cob" )	

if __name__ == "__main__":
        unittest.main()
