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

import unittest
import IECore

class TestWriter( unittest.TestCase ) :

	def testSupportedExtensions( self ) :

		e = IECore.Writer.supportedExtensions()
		for ee in e :
			self.assert_( type( ee ) is str )

		expectedExtensions = [ "exr", "pdc", "cin", "dpx", "cob" ]
		if IECore.withTIFF() :
			expectedExtensions += [ "tif", "tiff" ]
		if IECore.withJPEG() :
			expectedExtensions += [ "jpg", "jpeg" ]

		for ee in expectedExtensions :
			self.assert_( ee in e )

		self.assert_( not "obj" in expectedExtensions )

		e = IECore.Writer.supportedExtensions( IECore.TypeId.ImageWriter )
		for ee in e :
			self.assert_( type( ee ) is str )
		expectedImageWriterExtensions = [ "exr", "cin", "dpx", "yuv" ]
		if IECore.withTIFF() :
			expectedImageWriterExtensions += [ "tif", "tiff" ]
		if IECore.withJPEG() :
			expectedImageWriterExtensions += [ "jpg", "jpeg" ]

		self.assertEqual( set( expectedImageWriterExtensions ), set( e ) )

		self.assert_( not "pdc" in expectedImageWriterExtensions )
		self.assert_( not "cob" in expectedImageWriterExtensions )




if __name__ == "__main__":
	unittest.main()

