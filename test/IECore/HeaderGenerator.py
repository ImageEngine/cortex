##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2021, Hypothetical Inc. All rights reserved.
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


"""Unit test for HeaderGenerator binding"""
import getpass
import platform
import unittest

import IECore

class TestHeaderGenerator(unittest.TestCase):

	def testHeader(self):
		"""Test HeaderGenerator"""

		header = IECore.HeaderGenerator.header()
		self.assertEqual( header['ieCoreVersion'].value, IECore.versionString() )
		( sysname, nodename, release, version, machine, processor ) = platform.uname()
		self.assertEqual( header["host"]["systemName"].value, sysname )
		self.assertEqual( header["host"]["nodeName"].value, nodename )
		self.assertEqual( header["host"]["systemRelease"].value, release )
		self.assertEqual( header["host"]["systemVersion"].value, version )
		self.assertEqual( header["host"]["machineName"].value, machine )
		self.assertEqual( header["userName"].value, getpass.getuser() )
		self.assertTrue( header.has_key( "timeStamp" ) )

if __name__ == "__main__":
	unittest.main()

