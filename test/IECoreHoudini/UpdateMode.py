##########################################################################
#
#  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import hou
import IECoreHoudini
import unittest

class TestUpdateMode( IECoreHoudini.TestCase ) :

	def testSetMode( self ) :

		self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

		with IECoreHoudini.UpdateMode( hou.updateMode.Manual ) :

			self.assertEqual( hou.updateModeSetting(), hou.updateMode.Manual )

			with IECoreHoudini.UpdateMode( hou.updateMode.AutoUpdate ) :

				self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

			self.assertEqual( hou.updateModeSetting(), hou.updateMode.Manual )

		self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

	def testSetCurrentMode( self ) :

		self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

		with IECoreHoudini.UpdateMode( hou.updateMode.AutoUpdate ) :

			self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

		self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

	def testRaising( self ) :

		self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

		try :
			with IECoreHoudini.UpdateMode( hou.updateMode.Manual ) :

				raise RuntimeError("This error is intentional")

		except RuntimeError:

			self.assertEqual( hou.updateModeSetting(), hou.updateMode.AutoUpdate )

if __name__ == "__main__":
    unittest.main()
