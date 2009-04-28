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

import unittest

import IECore

class ParameterisedProceduralTest( unittest.TestCase ) :

	def testObject( self ) :
	
		self.assert_( IECore.Object.isType( IECore.TypeId.ParameterisedProcedural ) )
		self.assert_( IECore.Object.isAbstractType( IECore.TypeId.ParameterisedProcedural ) )
		self.assertRaises( RuntimeError, IECore.Object.create, IECore.TypeId.ParameterisedProcedural )

		self.assert_( IECore.Object.isType( IECore.TypeId.ReadProcedural ) )		
		self.failIf( IECore.Object.isAbstractType( IECore.TypeId.ReadProcedural ) )
		p = IECore.Object.create( IECore.TypeId.ReadProcedural )
		
	def testRunTimeTyped( self ) :	
		
		self.assertEqual( IECore.ReadProcedural.staticTypeName(), "ReadProcedural" )
		self.assertEqual( IECore.ReadProcedural.staticTypeId(), IECore.TypeId.ReadProcedural )		
		
		p = IECore.ReadProcedural()
		self.assertEqual( p.typeName(), "ReadProcedural" )
		self.assertEqual( p.typeId(), IECore.TypeId.ReadProcedural )				

	def testParameterAccess( self ) :
		
		p = IECore.ReadProcedural()

		self.assert_( p["files"]["name"].isInstanceOf( IECore.FileNameParameter.staticTypeId() ) )
		self.assert_( p.parameters().isInstanceOf( IECore.CompoundParameter.staticTypeId() ) )
		self.assert_( p.parameters()["files"].isSame( p["files"] ) )
	
	def testGrouping( self ) :
	
		p = IECore.ReadProcedural()
		g = IECore.Group()
		g.addChild( p )
		
	
if __name__ == "__main__":
        unittest.main()
