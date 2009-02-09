##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.cmds

class TemporaryAttributeValuesTest( unittest.TestCase ) :

	def test( self ) :
	
		s = maya.cmds.spaceLocator()[0]

		maya.cmds.addAttr( s, at="enum", sn="enumTest", enumName="A:B:C", defaultValue = 1 )
		self.assertEqual( maya.cmds.getAttr( s + ".enumTest" ), 1 )

		maya.cmds.addAttr( s, at="bool", sn="boolTest", defaultValue=1 )
		self.assertEqual( maya.cmds.getAttr( s + ".boolTest" ), 1 )
		
		maya.cmds.addAttr( s, at="float", sn="floatTest" )
		self.assertEqual( maya.cmds.getAttr( s + ".floatTest" ), 0 )
		
		maya.cmds.addAttr( s, at="long", sn="intTest" )
		self.assertEqual( maya.cmds.getAttr( s + ".intTest" ), 0 )
		
		maya.cmds.addAttr( s, at="float2", sn="float2Test" )
		maya.cmds.addAttr( s, at="float", sn="float2TestX", parent="float2Test" )
		maya.cmds.addAttr( s, at="float", sn="float2TestY", parent="float2Test" )
		self.assertEqual( maya.cmds.getAttr( s + ".float2TestX" ), 0 )
		self.assertEqual( maya.cmds.getAttr( s + ".float2TestY" ), 0 )
		
		maya.cmds.addAttr( s, at="long2", sn="int2Test" )
		maya.cmds.addAttr( s, at="long", sn="int2TestX", parent="int2Test", defaultValue=1 )
		maya.cmds.addAttr( s, at="long", sn="int2TestY", parent="int2Test", defaultValue=2 )
		self.assertEqual( maya.cmds.getAttr( s + ".int2TestX" ), 1 )
		self.assertEqual( maya.cmds.getAttr( s + ".int2TestY" ), 2 )

		maya.cmds.addAttr( s, at="float3", sn="float3Test" )
		maya.cmds.addAttr( s, at="float", sn="float3TestX", parent="float3Test", defaultValue=10 )
		maya.cmds.addAttr( s, at="float", sn="float3TestY", parent="float3Test", defaultValue=20 )
		maya.cmds.addAttr( s, at="float", sn="float3TestZ", parent="float3Test", defaultValue=30 )
		self.assertEqual( maya.cmds.getAttr( s + ".float3TestX" ), 10 )
		self.assertEqual( maya.cmds.getAttr( s + ".float3TestY" ), 20 )
		self.assertEqual( maya.cmds.getAttr( s + ".float3TestZ" ), 30 )


		maya.cmds.addAttr( s, at="short3", sn="short3Test" )
		maya.cmds.addAttr( s, at="short", sn="short3TestX", parent="short3Test", defaultValue=101 )
		maya.cmds.addAttr( s, at="short", sn="short3TestY", parent="short3Test", defaultValue=201 )
		maya.cmds.addAttr( s, at="short", sn="short3TestZ", parent="short3Test", defaultValue=301 )
		self.assertEqual( maya.cmds.getAttr( s + ".short3TestX" ), 101 )
		self.assertEqual( maya.cmds.getAttr( s + ".short3TestY" ), 201 )
		self.assertEqual( maya.cmds.getAttr( s + ".short3TestZ" ), 301 )

		maya.cmds.addAttr( s, dt="string", sn="stringTest" )
		maya.cmds.setAttr( s + ".stringTest", "hi", type="string" )
		self.assertEqual( maya.cmds.getAttr( s + ".stringTest" ), "hi" )
		
		context = IECoreMaya.TemporaryAttributeValues(
			{	
				s + ".enumTest" : 2,
				s + ".boolTest" : False,
				s + ".floatTest" : 10,
				s + ".intTest" : 20,
				s + ".float2Test" : ( 1, 2 ),
				s + ".int2Test" : IECore.V2i( 3, 4 ),
				s + ".float3Test" : ( 9, 6, 1 ),
				s + ".short3Test" : ( 500, 2, -1 ),
				s + ".stringTest" : "bye",
			}
		)
		
		with context : 
		
			self.assertEqual( maya.cmds.getAttr( s + ".enumTest" ), 2 )
			self.assertEqual( maya.cmds.getAttr( s + ".boolTest" ), 0 )
			self.assertEqual( maya.cmds.getAttr( s + ".floatTest" ), 10 )
			self.assertEqual( maya.cmds.getAttr( s + ".intTest" ), 20 )
			self.assertEqual( maya.cmds.getAttr( s + ".float2TestX" ), 1 )
			self.assertEqual( maya.cmds.getAttr( s + ".float2TestY" ), 2 )
			self.assertEqual( maya.cmds.getAttr( s + ".int2TestX" ), 3 )
			self.assertEqual( maya.cmds.getAttr( s + ".int2TestY" ), 4 )
			self.assertEqual( maya.cmds.getAttr( s + ".float3TestX" ), 9 )
			self.assertEqual( maya.cmds.getAttr( s + ".float3TestY" ), 6 )
			self.assertEqual( maya.cmds.getAttr( s + ".float3TestZ" ), 1 )
			self.assertEqual( maya.cmds.getAttr( s + ".short3TestX" ), 500 )
			self.assertEqual( maya.cmds.getAttr( s + ".short3TestY" ), 2 )
			self.assertEqual( maya.cmds.getAttr( s + ".short3TestZ" ), -1 )
			self.assertEqual( maya.cmds.getAttr( s + ".stringTest" ), "bye" )
		
		self.assertEqual( maya.cmds.getAttr( s + ".enumTest" ), 1 )
		self.assertEqual( maya.cmds.getAttr( s + ".boolTest" ), 1 )
		self.assertEqual( maya.cmds.getAttr( s + ".floatTest" ), 0 )
		self.assertEqual( maya.cmds.getAttr( s + ".intTest" ), 0 )
		self.assertEqual( maya.cmds.getAttr( s + ".float2TestX" ), 0 )
		self.assertEqual( maya.cmds.getAttr( s + ".float2TestY" ), 0 )
		self.assertEqual( maya.cmds.getAttr( s + ".int2TestX" ), 1 )
		self.assertEqual( maya.cmds.getAttr( s + ".int2TestY" ), 2 )
		self.assertEqual( maya.cmds.getAttr( s + ".float3TestX" ), 10 )
		self.assertEqual( maya.cmds.getAttr( s + ".float3TestY" ), 20 )
		self.assertEqual( maya.cmds.getAttr( s + ".float3TestZ" ), 30 )
		self.assertEqual( maya.cmds.getAttr( s + ".stringTest" ), "hi" )
	
		
if __name__ == "__main__":
	MayaUnitTest.TestProgram()
