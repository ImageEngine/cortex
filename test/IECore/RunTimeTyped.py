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
import IECore

class TestRunTimeTyped( unittest.TestCase ) :

	def test( self ) :
	
		i = IECore.IntData( 10 )
		self.assert_( i.isInstanceOf( "IntData" ) )
		self.assert_( i.isInstanceOf( "Data" ) )
		self.assert_( i.isInstanceOf( "Object" ) )
		self.assert_( i.isInstanceOf( "RunTimeTyped" ) )
		
		self.assert_( i.isInstanceOf( IECore.TypeId.IntData ) )
		self.assert_( i.isInstanceOf( IECore.TypeId.Data ) )
		self.assert_( i.isInstanceOf( IECore.TypeId.Object ) )
		self.assert_( i.isInstanceOf( IECore.TypeId.RunTimeTyped ) )
		
		self.assert_( IECore.IntData.inheritsFrom( "Data" ) )
		self.assert_( IECore.IntData.inheritsFrom( "Object" ) )
		self.assert_( IECore.IntData.inheritsFrom( "RunTimeTyped" ) )
		
		self.assert_( IECore.IntData.inheritsFrom( IECore.TypeId.Data ) )
		self.assert_( IECore.IntData.inheritsFrom( IECore.TypeId.Object ) )
		self.assert_( IECore.IntData.inheritsFrom( IECore.TypeId.RunTimeTyped ) )
		
		self.assert_( IECore.Data.inheritsFrom( IECore.TypeId.Object ) )
		self.assert_( IECore.Data.inheritsFrom( IECore.TypeId.RunTimeTyped ) )
		
		self.assert_( IECore.Object.inheritsFrom( IECore.TypeId.RunTimeTyped ) )

	def testStaticTypeBindings( self ) :
	
		typeNames={}
		typeIds = {}
	
		# Make that all classes derived from RunTimeTyped correctly bind staticTypeName and staticTypeId
		for typeName in dir(IECore):
			t = getattr(IECore, typeName)

			baseClasses = []
			
			# Iteratively expand base classes all the way to the top
			if hasattr(t, "__bases__"):
			
				baseClasses = [ t ]
				
				i=0
				while i < len(baseClasses):
					for x in baseClasses[i].__bases__:
						baseClasses.extend([x])
					
					i = i + 1	
						
			if IECore.RunTimeTyped in baseClasses:
				self.assert_( getattr(t, "staticTypeName") )
				self.assert_( getattr(t, "staticTypeId") )
				
				# Make sure that no 2 IECore classes provide the same typeId or typeName
				if t.staticTypeName() in typeNames:
					print(typeNames[t.staticTypeName()])
					raise Exception( "'%s' does not have a unique RunTimeTyped static type name (conflicts with '%s')." % ( t.__name__ , typeNames[t.staticTypeName()] ))
					
				if t.staticTypeId() in typeIds:
					raise Exception( "'%s' does not have a unique RunTimeTyped static type id (conflicts with '%s')." % (t.__name__ , typeIds[t.staticTypeId()] ))
				
				typeNames[t.staticTypeName()] = t.__name__
				typeIds[t.staticTypeId()] = t.__name__			
			
	
	def testMakeRunTimeTyped( self ) :
	
		# should raise because it's already been registered
		self.assertRaises( RuntimeError, IECore.makeRunTimeTyped, IECore.FileSequenceParameter, 100000, IECore.PathParameter )
		
		self.assertEqual( IECore.TypeId.FileSequenceParameter, IECore.FileSequenceParameter.staticTypeId() )
		self.assertEqual( IECore.TypeId.FileSequenceParameter, IECore.FileSequenceParameter( "", "" ).typeId() )
		
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( "PathParameter" ) )
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( IECore.TypeId.PathParameter ) )
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( "StringParameter" ) )
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( IECore.TypeId.StringParameter ) )
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( "Parameter" ) )
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( IECore.TypeId.Parameter ) )
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( "RunTimeTyped" ) )
		self.assert_( IECore.FileSequenceParameter.inheritsFrom( IECore.TypeId.RunTimeTyped ) )
		
		self.assert_( IECore.FileSequenceParameter( "", "" ).isInstanceOf( "FileSequenceParameter" ) )
		self.assert_( IECore.FileSequenceParameter( "", "" ).isInstanceOf( IECore.TypeId.FileSequenceParameter ) )
		self.assert_( IECore.FileSequenceParameter( "", "" ).isInstanceOf( "PathParameter" ) )
		self.assert_( IECore.FileSequenceParameter( "", "" ).isInstanceOf( IECore.TypeId.PathParameter ) )
		self.assertRaises( TypeError, IECore.FileSequenceParameter( "", "" ).isInstanceOf, 10 )

if __name__ == "__main__":
    unittest.main()   
