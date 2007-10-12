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

"""Unit test for CompoundData binding"""

import os
import math
import unittest
from IECore import *

class CompoundDataTest(unittest.TestCase):

	def testConstructors(self):
		"""Test constructors"""
		v1 = CompoundData()
		a = dict()
		a["1"] = IntData(1)
		v3 = CompoundData(a)
		self.assertEqual(v3.size(), 1)
		
	def testResize(self):
		"""Test resizing"""
		v = CompoundData()
		v["0"] = FloatData(2)
		self.assertEqual(v["0"], FloatData(2))
		v["1"] = FloatData(0)
		v["2"] = FloatData(3)
		v["3"] = FloatData(2)
		v["4"] = FloatData(5)
		self.assertEqual(v["4"], FloatData(5))
		self.assertEqual(len(v), 5)
		del(v["0"])
		self.assertEqual(len(v), 4)
		self.assert_(v.has_key("0") == False)
		v.clear()
		self.assertEqual(len(v), 0)
		
	def testAssignment(self):
		"""Test assignment"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v2 = v1.copy()
		v3 = v1
		v4 = v1.copy()
		self.assertEqual(len(v1), 2)
		self.assertEqual(len(v1), len(v2))
		self.assertEqual(v1["0"], v2["0"])
		self.assertEqual(v1["1"], v2["1"])
		self.assertEqual(v1["0"], v4["0"])
		self.assertEqual(v1["1"], v4["1"])
		
	def testCopyOnWrite(self):
		"""Test copy-on-write behavior"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v2 = v1.copy()
		v3 = v1.copy()
		v3["0"] = UIntData(5)
		self.assert_(v3["0"] == UIntData(5))
		self.assert_(v2["0"] == FloatData(1.2))
		v1["2"] = FloatData(5);
		self.assertEqual(len(v1), 3)
		self.assertEqual(len(v2), 2)
		
	def testSearch(self):
		"""Test search functions"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		self.assert_("0" in v1)
		self.assert_("3" not in v1)
		self.assert_(v1.has_key("1"))
		self.assert_(not v1.has_key("3"))
		self.assert_(v1.get("0") == FloatData(1.2))
		self.assert_(v1.get("0", IntData(10)) == FloatData(1.2))
		self.assert_(v1.get("xx", IntData(10)) == IntData(10))
		self.assertEqual(len(v1), 3)
		
	def testUpdate(self):
		"""Test update function"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		v2 = CompoundData()
		v2["0"] = UIntData(5)
		v2["3"] = UIntData(6)
		v2.update(v1)
		self.assertEqual(len(v2), 4)
		self.assert_(v2["0"] == FloatData(1.2))
		self.assert_(v2["3"] == UIntData(6))
		v3 = dict()
		v3["1"] = CharData("a")
		v3["4"] = UCharData(9)
		v2.update(v3)
		self.assertEqual(len(v2), 5)
		self.assert_(v2["1"] == CharData("a"))
		self.assert_(v2["4"] == UCharData(9))
	
	def testSetDefault(self):
		"""Test setdefault function"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		v2 = v1.copy()
		self.assertEqual(len(v1), 3)
		self.assert_(v1.setdefault("2", UIntData(10)) == FloatData(3))
		self.assertEqual(len(v1), 3)
		self.assert_(v1.setdefault("x", UIntData(10)) == UIntData(10))
		self.assertEqual(len(v1), 4)
	
	def testPop(self):
		"""Test pop functions"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		v1["3"] = FloatData(4)
		self.assertEqual(len(v1), 4)
		prev = v1.popitem()
		self.assertEqual(len(v1), 3)
		self.assert_(prev == ('0', FloatData(1.2)))
		self.assert_(v1.pop("x", UIntData(10)) == UIntData(10))
		self.assert_(v1.pop("3", UIntData(10)) == FloatData(4))
		self.assertEqual(len(v1), 2)
		
	def testKeyValues(self):
		"""Test keys/values listing"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		self.assert_(v1.keys() == ['0', '1', '2'])
		vals = v1.values()
		self.assert_(vals == [FloatData(1.2), FloatData(2.3), FloatData(3)])
		items = v1.items()
		self.assert_(items == [('0', FloatData(1.2)), ('1', FloatData(2.3)), ('2', FloatData(3))])

	def testEquality(self):
		"""Test equality function"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		v2 = CompoundData()
		v2["0"] = FloatData(1.2)
		v2["1"] = FloatData(2.3)
		v2["2"] = FloatData(3)
		v3 = v2.copy()
		del v3["2"]
		self.assert_(v1 == v2)
		self.assert_(not v1 != v2)
		self.assert_(not v1 == v3)
		self.assert_(not v2 == v3)
		v2["-1"] = FloatData(6)
		self.assert_(v1 != v2)
		self.assert_(not v1 == v2)
		del(v1["2"])
		self.assert_(v1 == v3)
	
	def testByValueItem(self):
		"""Test by value return type"""
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		self.assert_(v1["0"] == FloatData(1.2))
		a = v1["0"]
		a = UIntData(255)
		self.assert_(v1["0"] == FloatData(1.2))
		self.assert_(a == UIntData(255))
		
	def testLoadSave(self):
		"""Test load/save"""	
		
		v1 = CompoundData()
		v1["0"] = FloatData(1.2)
		v1["1"] = FloatData(2.3)
		v1["2"] = FloatData(3)
		v1["some:data"] = FloatData(3)		
		self.assert_(v1["0"] == FloatData(1.2))

		v1.save("test/CompoundData.fio")

		v2 = Object.load("test/CompoundData.fio")	
		self.assertEqual( v1, v2 )	

	def tearDown(self):
        
		if os.path.isfile("./test/CompoundData.fio") :
			os.remove("./test/CompoundData.fio")

		
if __name__ == "__main__":
    unittest.main()   

