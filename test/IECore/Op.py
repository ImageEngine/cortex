##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

class PythonOp( IECore.Op ) :

	def __init__( self ) :

		IECore.Op.__init__( self, "opDescription", IECore.StringParameter( name = "result", description = "", defaultValue = "" ) )
		self.parameters().addParameter( IECore.StringParameter( name = "name", description = "", defaultValue = "john" ) )

	def doOperation( self, operands ) :

		return IECore.StringData( operands['name'].value )

IECore.registerRunTimeTyped( PythonOp )

class TestPythonOp( unittest.TestCase ) :

	def testNewOp( self ) :

		o = PythonOp()
		self.assertEqual( o.operate(), IECore.StringData( "john" ) )
		o.parameters()["name"].setValue( IECore.StringData( "jim" ) )
		self.assertEqual( o.operate(), IECore.StringData( "jim" ) )
		self.assertEqual( o(), IECore.StringData( "jim" ) )

	def testSmartOp( self ):
		""" test smart operate function"""
		o = PythonOp()
		self.assertEqual( o( name = "jim" ), IECore.StringData( "jim" ) )
		self.assertEqual( o( name = IECore.StringData( "jimbo" ) ), IECore.StringData( "jimbo" ) )

		result = o( name = "roger" )
		self.assertEqual( result, IECore.StringData( "roger" ) )
		self.assertEqual( o.resultParameter().getValue(), result )

	def testDefaultConstructor( self ):

		abstractOps = {
			IECore.Op, IECore.ModifyOp,
			IECore.SequenceMergeOp, IECore.FileSequenceAnalyzerOp,
			IECore.Reader, IECore.Writer
		}

		def isOpWithMissingConstructor( c ):

			try :
				if not issubclass( c, IECore.Op ) :
					return False
			except TypeError :
				return False

			if c in abstractOps :
				return False

			try :
				c()
			except :
				return True

			return False

		IECore.RefCounted.collectGarbage()
		badClasses = [ getattr( IECore, x ) for x in dir( IECore ) if isOpWithMissingConstructor( getattr( IECore, x ) ) ]
		if len(badClasses) > 0:
			raise Exception, "The following Op classes don't have a default constructor: " + \
					", ".join( [ str( c ) for c in badClasses ] )

	def testSpecializedCompoundParameter( self ):

		# This Ops shows how to make cross-validation on Op parameters.
		# Op that will only operate if the 'first' parameter is greater then the 'second' parameter.
		class GreaterThenOp( IECore.Op ) :

			class MyCompound( IECore.CompoundParameter ):
				def __init__( self ):
					IECore.CompoundParameter.__init__( self,
							name = '',
							description = '',
							members = [
								IECore.IntParameter( 'first', '', 0 ),
								IECore.IntParameter( 'second', '', 0 ),
							]
					)
				def valueValid( self, value ) :

					res = IECore.CompoundParameter.valueValid( self, value )
					if not res[0]:
						return res

					if value['first'] > value['second']:
						return ( True, "" )

					return ( False, "First parameter is not greater then the second!" )

			def __init__( self ) :
				IECore.Op.__init__( self, "opDescription", GreaterThenOp.MyCompound(), IECore.StringParameter( "result", "", "" ) )

			def doOperation( self, operands ) :
				return IECore.StringData( "Yes!" )

		op = GreaterThenOp()
		op['first'] = 1
		op['second'] = 0
		op()
		op['second'] = 2
		self.assertRaises( Exception, op )

	def testRunTimeTyped( self ) :

		op = PythonOp()

		self.assertEqual( op.typeName(), "PythonOp" )

		self.failUnless( op.isInstanceOf( IECore.TypeId.Op ), True )
		self.failUnless( op.isInstanceOf( "Op" ), True )

		self.failUnless( op.isInstanceOf( "PythonOp" ), True )
		self.failUnless( op.isInstanceOf( PythonOp.staticTypeId() ), True )

	def testOperateWithArgs( self ) :

		op = PythonOp()
		# make sure we can call the op passing all the parameter values as a CompoundObject
		self.assertEqual( op( IECore.CompoundObject( { "name": IECore.StringData("jim") } ) ), IECore.StringData( "jim" ) )
		# make sure the last call did not affect the contents of the Op's parameters.
		self.assertEqual( op.parameters()['name'].getTypedValue(), "john" )

if __name__ == "__main__":
	unittest.main()

