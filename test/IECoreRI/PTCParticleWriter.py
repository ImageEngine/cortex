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
import os, os.path
import IECore
import IECoreRI

if hasattr( IECoreRI, "PTCParticleWriter" ):

	class PTCParticleWriterTester( unittest.TestCase ) :

		testfile = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "data/test.3Dbake" )
		tmpfile = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "data/tmp.3Dbake" )

		def __cleanUp( self ):
			try:
				os.remove( self.tmpfile )
			except:
				pass

		def testConstruction(self):

			r = IECore.Reader.create(self.testfile)
			pointCloud = r.read()

			r = IECore.Writer.create(pointCloud, self.tmpfile)
			self.assertEqual(type(r), IECoreRI.PTCParticleWriter)

			r = None

		def __testWriteFromFactory(self):
			
			self.__cleanUp()

			r = IECore.Reader.create(self.testfile)
			pointCloud = r.read()

			w = IECore.Writer.create( pointCloud, self.tmpfile )
			w.write()

			r = IECore.Reader.create(self.tmpfile)
			pointCloud2 = r.read()

			self.__cleanUp()

			self.assertEqual( pointCloud.numPoints, pointCloud2.numPoints )

			self.assertEqual( len( pointCloud[ "P" ].data ), len( pointCloud2[ "P" ].data ) )

			self.assertEqual( len( pointCloud.keys() ), 3 )
			self.assertEqual( set( pointCloud.keys() ), set( pointCloud2.keys() ) )

			data2Dict = dict()
			normals2 = pointCloud2[ "N" ].data
			normals = pointCloud["N"].data
			for (i, x) in enumerate( pointCloud2[ "P" ].data ):
				for ( j, y ) in enumerate( pointCloud[ "P" ].data ):
					
					if data2Dict.has_key( j ):
						continue

					if (y - x).length() < 0.00001 and (normals2[i] - normals[j]).length() < 0.00001:
						data2Dict[ j ] = i
						break
				else:
					raise Exception, "Could not find data point" + str(x)

			for var in pointCloud.keys():
				for p in xrange( 0, pointCloud.numPoints ):
					p1 = pointCloud[ var ].data[ p ]
					p2 = pointCloud2[ var ].data[ data2Dict[ p ] ]
					if p1 != p2:
						raise Exception, ("Variable %s does not match! (index %d): " % (var, p ) ) + str(p1) + " != " + str(p2)
			self.assertEqual( pointCloud.blindData(), pointCloud2.blindData() )

		def testWrite(self):

			self.__cleanUp()

			pointCloud = IECore.PointsPrimitive( 100 )
			data = []
			for t in xrange( 0, pointCloud.numPoints ):
				data.append( IECore.V3f(t) )
			pointCloud[ "P" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( data ) )
			pointCloud[ "another" ] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( data ) )
			pointCloud.blindData()["PTCParticleIO"] = IECore.CompoundData( { "xResolution": IECore.FloatData( 640 ), "yResolution": IECore.FloatData( 480 ), "aspectRatio": IECore.FloatData( 1.0 ) } )

			w = IECoreRI.PTCParticleWriter( pointCloud, self.tmpfile )
			w.write()

			r = IECore.Reader.create(self.tmpfile)
			pointCloud2 = r.read()

			self.__cleanUp()

			self.assertEqual( set( [ "P", "N", "width", "another" ] ), set( pointCloud2.keys() ) )
			for var in pointCloud.keys():
				if pointCloud[ var ].data != pointCloud2[ var ].data:
					raise Exception, "Variable %s does not match!" % var
			
			#for b in pointCloud.blindData()['PTCParticleIO'].keys():
			#	self.assertEqual( pointCloud.blindData()[b], pointCloud2.blindData()['PTCParticleIO'][b] )
	

if __name__ == "__main__":
    unittest.main()   
