##########################################################################
#
#  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

import unittest

import IECore
import IECoreScene

class ParameterisedProceduralTest( unittest.TestCase ) :

	def testObject( self ) :

		self.assert_( IECore.Object.isType( IECore.TypeId.ParameterisedProcedural ) )
		self.assert_( IECore.Object.isAbstractType( IECore.TypeId.ParameterisedProcedural ) )
		self.assertRaises( RuntimeError, IECore.Object.create, IECore.TypeId.ParameterisedProcedural )

		self.assert_( IECore.Object.isType( IECore.TypeId.ReadProcedural ) )
		self.failIf( IECore.Object.isAbstractType( IECore.TypeId.ReadProcedural ) )
		p = IECore.Object.create( IECore.TypeId.ReadProcedural )

	def testRunTimeTyped( self ) :

		self.assertEqual( IECoreScene.ReadProcedural.staticTypeName(), "ReadProcedural" )
		self.assertEqual( IECoreScene.ReadProcedural.staticTypeId(), IECore.TypeId.ReadProcedural )

		p = IECoreScene.ReadProcedural()
		self.assertEqual( p.typeName(), "ReadProcedural" )
		self.assertEqual( p.typeId(), IECore.TypeId.ReadProcedural )

	def testParameterAccess( self ) :

		p = IECoreScene.ReadProcedural()

		self.assert_( p["files"]["name"].isInstanceOf( IECore.FileNameParameter.staticTypeId() ) )
		self.assert_( p.parameters().isInstanceOf( IECore.CompoundParameter.staticTypeId() ) )
		self.assert_( p.parameters()["files"].isSame( p["files"] ) )

	def testGrouping( self ) :

		p = IECoreScene.ReadProcedural()
		g = IECoreScene.Group()
		g.addChild( p )

	def testNoTypeIdOverride( self ) :

		## Really all the typeid functions should be overridden in all python
		# derived classes, but that won't always be the case and we sure don't
		# want things to blow up if they're absent.

		class T( IECoreScene.ParameterisedProcedural ) :

			def __init__( self ) :

				IECoreScene.ParameterisedProcedural.__init__( self )

		t = T()
		self.assertEqual( t.typeId(), IECoreScene.ParameterisedProcedural.staticTypeId() )
		self.assertEqual( t.typeName(), IECoreScene.ParameterisedProcedural.staticTypeName() )
		self.failUnless( t.isInstanceOf( IECoreScene.ParameterisedProcedural.staticTypeId() ) )
		self.failUnless( t.isInstanceOf( IECoreScene.ParameterisedProcedural.staticTypeName() ) )

	def testMissingDoRender( self ) :

		## This test exercises a problem in boost::python::wrapper::get_override
		# whereby failing to find an override would incorrectly set python error
		# flags (PyErr_Occurred() would return True), and we'd get really bizarre
		# errors erroneously reporting themselves later in execution. although in
		# the case of ParameterisedProcedural.doRender() the override should always
		# be present, we don't want to confuse people with really off the wall errors,
		# and outside of cortex we have other code where it's perfectly reasonable
		# not to provide an override - this is where the problem was first found.

		class MyProcedural( IECoreScene.ParameterisedProcedural ) :

			def __init__( self ) :

				IECoreScene.ParameterisedProcedural.__init__( self, "" )

			## oops! we forgot the doRender() method.

		IECore.registerRunTimeTyped( MyProcedural )

		c = IECoreScene.CapturingRenderer()

		g = IECoreScene.Group()
		g.addChild( MyProcedural() )

		with IECoreScene.WorldBlock( c ) :
			g.render( c )

	def testReturningNonWrappedPtr( self ) :

		r = IECoreScene.ReadProcedural()
		o = IECore.CompoundObject( { "c" : r } )

		self.assertTrue( o["c"].isSame( r ) )
		self.assertTrue( o["c"] is r )

if __name__ == "__main__":
        unittest.main()
