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
from IECore import *

class PythonOp( Op ) :

	def __init__( self ) :
	
		Op.__init__( self, "opName", "opDescription", StringParameter( name = "result", description = "", defaultValue = "" ) )
		self.parameters().addParameter( StringParameter( name = "name", description = "", defaultValue = "john" ) )

	def doOperation( self, operands ) :
	
		return StringData( operands.name.value )

class TestPythonOp( unittest.TestCase ) :

	def testNewOp( self ) :
	
		o = PythonOp()
		self.assertEqual( o.operate(), StringData( "john" ) )
		o.parameters()["name"].setValue( StringData( "jim" ) )
		self.assertEqual( o.operate(), StringData( "jim" ) )
		self.assertEqual( o(), StringData( "jim" ) )

	def testSmartOp( self ):
		""" test smart operate function"""
		o = PythonOp()
		self.assertEqual( o( name = "jim" ), StringData( "jim" ) )
		self.assertEqual( o( name = StringData( "jimbo" ) ), StringData( "jimbo" ) )

	def testDefaultConstructor( self ):
		import IECore
		exceptionList = [	IECore.Reader, IECore.Writer,
							IECore.ImageReader, IECore.ImageWriter, 
							IECore.ParticleReader, IECore.ParticleWriter 
						]
		def test(c):
			try:
				if issubclass(c, IECore.Op) and not c is IECore.Op and not c in exceptionList:
					# instantiate using default constructor
					f = c()
			except:
				return False
	
		RefCounted.collectGarbage()
		badClasses = filter(test, map(lambda x: getattr(IECore, x), dir(IECore)))
		if len(badClasses) > 0:
			raise Exception, "The following Op classes don't have a default constructor: " + \
					string.join(map(str, badClasses), ", ")
		

if __name__ == "__main__":
	unittest.main()
	
