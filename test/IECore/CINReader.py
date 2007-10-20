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
import sys
from IECore import *

from math import pow

class TestCINReader(unittest.TestCase):

        testfile =    "test/IECore/data/cinFiles/bluegreen_noise.cin"
	testoutfile = "test/IECore/data/cinFiles/bluegreen_noise.testoutput.cin"

        def testConstruction(self):
                
		r = IECore.Reader.create(self.testfile)
		self.assertEqual(type(r), IECore.CINImageReader)


        def testRead(self):

                r = IECore.Reader.create(self.testfile)
		self.assertEqual(type(r), IECore.CINImageReader)

		img = r.read()
		
		self.assertEqual(type(img), IECore.ImagePrimitive)

		# write test (CIN -> EXR)
                w = IECore.Writer.create(img, self.testoutfile)
		self.assertEqual(type(w), IECore.CINImageWriter)

		w.write()


        def testWindowedRead(self):

		testfile = ["test/IECore/data/cinFiles/bluegreen_noise", "cin"]

		test_file_path = ".".join(testfile)
		test_outfile_path = ".".join([testfile[0], 'windowtestoutput', testfile[1]])

                # create a reader, read a sub-image
                r = IECore.Reader.create(test_file_path)
		self.assertEqual(type(r), IECore.CINImageReader)
		r.parameters().dataWindow.setValue(Box2iData(Box2i(V2i(100, 100), V2i(199, 199))))

		# read, verify
		img = r.read()
		self.assertEqual(type(img), IECore.ImagePrimitive)

		# write back the sub-image
                w = IECore.Writer.create(img, test_outfile_path)
		self.assertEqual(type(w), IECore.CINImageWriter)
		w.write()

        def testChannelRead(self):

		testfile = ["test/IECore/data/cinFiles/bluegreen_noise", "cin"]

		test_file_path = ".".join(testfile)
		test_outfile_path = ".".join([testfile[0], 'channeltestoutput', testfile[1]])

                # create a reader, constrain to a sub-image, R (red) channel
                r = IECore.Reader.create(test_file_path)
		self.assertEqual(type(r), IECore.CINImageReader)
		
		r.parameters().dataWindow.setValue(Box2iData(Box2i(V2i(100, 100), V2i(199, 199))))
		r.parameters().channels.setValue(StringVectorData(["R", "G"]))

		# read, verify
		img = r.read()
		self.assertEqual(type(img), IECore.ImagePrimitive)

		# write back the sub-image
                w = IECore.Writer.create(img, test_outfile_path)
		self.assertEqual(type(w), IECore.CINImageWriter)
		w.write()
		

                			
if __name__ == "__main__":
	unittest.main()   
	
