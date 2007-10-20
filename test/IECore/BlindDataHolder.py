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

"""Unit test for BlindDataHolder binding"""
import os
import unittest

from IECore import *

class TestBlindDataHolder(unittest.TestCase):
	def testConstructors(self):
		"""Test BlindDataHolder constructors"""
		b = BlindDataHolder()
		
		c = CompoundData()
		c["floatData"] = FloatData(3.0)
		
		b = BlindDataHolder(c)
		
		self.assertEqual( b.typeName(), "BlindDataHolder" )
		self.failIf( Object.isAbstractType( "BlindDataHolder") )
		
	def testBlindData(self):
		"""Test BlindDataHolder blindData"""
		
		b = BlindDataHolder()
		
		b.blindData()["floatData"] = FloatData(1.0)
		b.blindData()["intData"] = IntData(-5)
		
		self.assertEqual( b.blindData()["floatData"].value, 1.0 )
		self.assertEqual( b.blindData()["intData"].value, -5 )		
		
		self.assertEqual( len(b.blindData()), 2 )
		
	def testLoadSave(self):
	
		"""Test BlindDataHolder load/save"""	
		
		b1 = BlindDataHolder()
		
		b1.blindData()["floatData"] = FloatData(1.0)
		b1.blindData()["intData"] = IntData(-5)

		b1.save("test/BlindDataHolder.fio")

		b2 = Object.load("test/BlindDataHolder.fio")	
		self.assertEqual( b1, b2 )
		
	def tearDown(self):
	
		if os.path.isfile("./test/BlindDataHolder.fio") :
                        os.remove("./test/BlindDataHolder.fio")

		

		
	
if __name__ == "__main__":
        unittest.main()
