##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
import IECore

class CompoundDataTest(unittest.TestCase):

	def testConstructors(self):
		"""Test constructors"""
		v1 = IECore.CompoundData()
		a = dict()
		a["1"] = IECore.IntData(1)
		v3 = IECore.CompoundData(a)
		self.assertEqual(v3.size(), 1)
		
	def testResize(self):
		"""Test resizing"""
		v = IECore.CompoundData()
		v["0"] = IECore.FloatData(2)
		self.assertEqual(v["0"], IECore.FloatData(2))
		v["1"] = IECore.FloatData(0)
		v["2"] = IECore.FloatData(3)
		v["3"] = IECore.FloatData(2)
		v["4"] = IECore.FloatData(5)
		self.assertEqual(v["4"], IECore.FloatData(5))
		self.assertEqual(len(v), 5)
		del(v["0"])
		self.assertEqual(len(v), 4)
		self.assert_(v.has_key("0") == False)
		v.clear()
		self.assertEqual(len(v), 0)
		
	def testAssignment(self):
		"""Test assignment"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
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
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v2 = v1.copy()
		v3 = v1.copy()
		v3["0"] = IECore.UIntData(5)
		self.assert_(v3["0"] == IECore.UIntData(5))
		self.assert_(v2["0"] == IECore.FloatData(1.2))
		v1["2"] = IECore.FloatData(5);
		self.assertEqual(len(v1), 3)
		self.assertEqual(len(v2), 2)
		
	def testSearch(self):
		"""Test search functions"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		self.assert_("0" in v1)
		self.assert_("3" not in v1)
		self.assert_(v1.has_key("1"))
		self.assert_(not v1.has_key("3"))
		self.assert_(v1.get("0") == IECore.FloatData(1.2))
		self.assert_(v1.get("0", IECore.IntData(10)) == IECore.FloatData(1.2))
		self.assert_(v1.get("xx", IECore.IntData(10)) == IECore.IntData(10))
		self.assertEqual(len(v1), 3)
		
	def testUpdate(self):
		"""Test update function"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v2 = IECore.CompoundData()
		v2["0"] = IECore.UIntData(5)
		v2["3"] = IECore.UIntData(6)
		v2.update(v1)
		self.assertEqual(len(v2), 4)
		self.assert_(v2["0"] == IECore.FloatData(1.2))
		self.assert_(v2["3"] == IECore.UIntData(6))
		v3 = dict()
		v3["1"] = IECore.CharData("a")
		v3["4"] = IECore.UCharData(9)
		v2.update(v3)
		self.assertEqual(len(v2), 5)
		self.assert_(v2["1"] == IECore.CharData("a"))
		self.assert_(v2["4"] == IECore.UCharData(9))
	
	def testSetDefault(self):
		"""Test setdefault function"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v2 = v1.copy()
		self.assertEqual(len(v1), 3)
		self.assert_(v1.setdefault("2", IECore.UIntData(10)) == IECore.FloatData(3))
		self.assertEqual(len(v1), 3)
		self.assert_(v1.setdefault("x", IECore.UIntData(10)) == IECore.UIntData(10))
		self.assertEqual(len(v1), 4)
	
	def testPop(self):
		"""Test pop functions"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v1["3"] = IECore.FloatData(4)
		self.assertEqual(len(v1), 4)
		prev = v1.popitem()
		self.assertEqual(len(v1), 3)
		self.assert_(prev == ('0', IECore.FloatData(1.2)))
		self.assert_(v1.pop("x", IECore.UIntData(10)) == IECore.UIntData(10))
		self.assert_(v1.pop("3", IECore.UIntData(10)) == IECore.FloatData(4))
		self.assertEqual(len(v1), 2)
		
	def testKeyValues(self):
		"""Test keys/values listing"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		self.assert_(v1.keys() == ['0', '1', '2'])
		vals = v1.values()
		self.assert_(vals == [IECore.FloatData(1.2), IECore.FloatData(2.3), IECore.FloatData(3)])
		items = v1.items()
		self.assert_(items == [('0', IECore.FloatData(1.2)), ('1', IECore.FloatData(2.3)), ('2', IECore.FloatData(3))])

	def testEquality(self):
		"""Test equality function"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v2 = IECore.CompoundData()
		v2["0"] = IECore.FloatData(1.2)
		v2["1"] = IECore.FloatData(2.3)
		v2["2"] = IECore.FloatData(3)
		v3 = v2.copy()
		del v3["2"]
		self.assert_(v1 == v2)
		self.assert_(not v1 != v2)
		self.assert_(not v1 == v3)
		self.assert_(not v2 == v3)
		v2["-1"] = IECore.FloatData(6)
		self.assert_(v1 != v2)
		self.assert_(not v1 == v2)
		del(v1["2"])
		self.assert_(v1 == v3)
	
	def testByValueItem(self):
		"""Test by value return type"""
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		self.assert_(v1["0"] == IECore.FloatData(1.2))
		a = v1["0"]
		a = IECore.UIntData(255)
		self.assert_(v1["0"] == IECore.FloatData(1.2))
		self.assert_(a == IECore.UIntData(255))
		
	def testLoadSave(self):
		"""Test load/save"""	
		
		iface = IECore.IndexedIOInterface.create( "test/CompoundData.fio", "/", IECore.IndexedIOOpenMode.Write )
		
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		v1["some:data"] = IECore.FloatData(3)		
		self.assert_(v1["0"] == IECore.FloatData(1.2))

		v1.save( iface, "test" )

		v2 = IECore.Object.load( iface, "test" )	
		self.assertEqual( v1, v2 )
		
	def testRepr(self):
		"""Test repr"""			
		
		v1 = IECore.CompoundData()
		
		r1 = repr(v1)
		
		self.assertEqual( eval(repr(v1)), v1 )
		
		v1 = IECore.CompoundData()
		v1["0"] = IECore.FloatData(1.2)
		v1["1"] = IECore.FloatData(2.3)
		v1["2"] = IECore.FloatData(3)
		
		self.assertEqual( eval(repr(v1)), v1 )
		
		v1 = IECore.CompoundData()
		v1["0"] = IECore.StringData( "test" )
		v1["1"] = IECore.CompoundData(
			{ "0" : IECore.StringData( "test" ),
			  "1" : IECore.M33fData() 
			}
		)
		v1["someMoreData"] = IECore.V3fVectorData()
		v1["A"] = IECore.Color4fVectorData()
		
		self.assertEqual( eval(repr(v1)), v1 )
		
	def tearDown(self):
        
		if os.path.isfile("./test/CompoundData.fio") :
			os.remove("./test/CompoundData.fio")

		
if __name__ == "__main__":
    unittest.main()   

