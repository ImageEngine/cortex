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
import IECore

class TestJPEGReader(unittest.TestCase):

	testfile =    "test/data/jpg/bluegreen_noise.400x300.jpg"
	testoutfile = "test/data/jpg/bluegreen_noise.400x300.testoutput.exr"

	def testConstruction(self):

		v2id = IECore.V2iData(IECore.V2i(20, 20))

		r = IECore.Reader.create(self.testfile)
		self.assertEqual(type(r), IECore.JPEGImageReader)

	def testRead(self):

		r = IECore.Reader.create(self.testfile)
		self.assertEqual(type(r), IECore.JPEGImageReader)

		img = r.read()

		self.assertEqual(type(img), IECore.ImagePrimitive)

		# write test (JPEG -> EXR)
		w = IECore.Writer.create(img, self.testoutfile)
		self.assertEqual(type(w), IECore.EXRImageWriter)

		w.write()

	def testWindowRead(self):

		r = IECore.Reader.create(self.testfile)
		self.assertEqual(type(r), IECore.JPEGImageReader)
		r.parameters().dataWindow.setValue(IECore.Box2iData(IECore.Box2i(IECore.V2i(60, 60), IECore.V2i(100, 100))))

		img = r.read()

		self.assertEqual(type(img), IECore.ImagePrimitive)

		# write test (JPEG -> EXR)
		w = IECore.Writer.create(img, self.testoutfile)
		self.assertEqual(type(w), IECore.EXRImageWriter)

		w.write()

	def testWrite(self):

		testfile =    "test/data/jpg/bluegreen_noise.400x300.jpg"
		testoutfile = "test/data/jpg/bluegreen_noise.400x300.testoutput.jpg"

		r = IECore.Reader.create(testfile)
		self.assertEqual(type(r), IECore.JPEGImageReader)

		img = r.read()

		self.assertEqual(type(img), IECore.ImagePrimitive)

		# write test (JPEG -> JPEG)
		w = IECore.Writer.create(img, testoutfile)
		self.assertEqual(type(w), IECore.JPEGImageWriter)
		w.write()

	def testDataWindowWrite(self):

		testfile =    "test/data/exrFiles/redgreen_gradient_piz_256x256.exr"
		testoutfile = "test/data/jpg/redgreen_gradient_piz_256x256.testoutput_datawindow.jpg"

		# read the EXR (with data window subset)
		r = IECore.Reader.create(testfile)
		self.assertEqual(type(r), IECore.EXRImageReader)
		img = r.read()

		self.assertEqual(type(img), IECore.ImagePrimitive)

		# write test (EXR -> JPEG) with image-defined data window
		w = IECore.Writer.create(img, testoutfile)
		self.assertEqual(type(w), IECore.JPEGImageWriter)
		w.write()

if __name__ == "__main__":
	unittest.main()   
	
