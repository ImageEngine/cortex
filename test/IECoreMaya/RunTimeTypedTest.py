##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
import IECoreMaya

class RunTimeTypedTest( unittest.TestCase ) :

	def test( self ) :

		typeNames = {}
		typeIds = {}

		for c in [ getattr( IECoreMaya, x ) for x in dir( IECoreMaya ) ] :

			try :
				if not issubclass( c, IECore.RunTimeTyped ) :
					continue
			except TypeError :
				continue # c wasn't a class

			self.assertFalse( c.staticTypeId() in typeIds )
			self.assertFalse( c.staticTypeName() in typeNames )

			typeIds[c.staticTypeId()] = c
			typeNames[c.staticTypeName()] = c

			self.assertEqual( IECore.RunTimeTyped.typeNameFromTypeId( c.staticTypeId() ), c.staticTypeName() )
			self.assertEqual( IECore.RunTimeTyped.typeIdFromTypeName( c.staticTypeName() ), c.staticTypeId() )

			self.assertTrue( c.staticTypeId() in IECoreMaya.TypeId.values or c.staticTypeId() in IECore.TypeId.values )

			if c.staticTypeId() in IECoreMaya.TypeId.values :
				self.assertEqual( c.staticTypeId(), getattr( IECoreMaya.TypeId, c.staticTypeName() ) )
			else :
				self.assertEqual( c.staticTypeId(), getattr( IECore.TypeId, c.staticTypeName() ) )

if __name__ == "__main__":
	IECoreMaya.TestProgram()
