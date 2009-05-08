##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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
import os, os.path
import IECore
import IECoreRI

if hasattr( IECoreRI, "PTCParticleReader" ):

	class PTCParticleReaderTester( unittest.TestCase ) :

		testfile = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "data/test.3Dbake" )

		def testConstruction(self):

			r = IECore.Reader.create(self.testfile)
			self.assertEqual(type(r), IECoreRI.PTCParticleReader)

		def testReadFromFactory(self):

			r = IECore.Reader.create(self.testfile)
			pointCloud = r.read()
			self.assertEqual( type(pointCloud), IECore.PointsPrimitive )

		def testRead(self):
			r = IECoreRI.PTCParticleReader()
			r['fileName'] = self.testfile
			pointCloud = r.read()
			self.assertEqual( type(pointCloud), IECore.PointsPrimitive )
			self.assertEqual( pointCloud.numPoints, 2975 )
			self.assertEqual( len( pointCloud["P"].data ), 2975 )
			self.assertEqual( set( pointCloud.blindData()['PTCParticleIO'].keys() ), set( [ "boundingBox", "worldToEye", "worldToNdc", "variableTypes" ] ) )
			r['percentage'] = 50
			pointCloud2 = r.read()
			self.assertEqual( len( pointCloud2["P"].data ), 1502 )

		def testCanRead(self) :

			self.assertEqual( IECoreRI.PTCParticleReader.canRead( "test/IECoreRI/data/test.3Dbake" ), True )
			self.assertEqual( IECoreRI.PTCParticleReader.canRead( "test/IECoreRI/data/sphere.cob" ), False )

if __name__ == "__main__":
    unittest.main()
