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

class TestObject( unittest.TestCase ) :

	def setUp( self ) :
	
		self.typeNames = [
			"FloatTypedData",
			"FloatVectorData",
			"V3fVectorData"
		]

	def testDataTypesDefinition( self ) :
		"""
		Function that checks if all the Data classes from IECore were defined on __dataTypesConversion dict.
		It raises an exception if there's any problem.
		"""
		import IECore
		def test(c):
			try:
				return issubclass(c, IECore.Data) and not (c is IECore.Data)
			except:
				return False
	
		dataClasses = filter(test, map(lambda x: getattr(IECore, x), dir(IECore)))
		notDefinedClasses = set(dataClasses).difference(IECore.getDataDerivedTypes())
		if len(notDefinedClasses) > 0:
			raise Exception, "The following classes were not defined on the conversion dictionaire: " + \
					", ".join( map(str, notDefinedClasses) ) + ".\nPlease, add them on DataTraits.py"
		
	def testObjectCreateAndCast( self ):
		"""
		Tests if all Object derived classes can be created using the factory function and 
		if they can be casted to Object pointers.
		PS: Data derived objects should be casted down to Data. Because Data is casted to Object.
		"""
		
		import IECore
		
		def objectDerived(c):
			try:
				return issubclass(c, IECore.Object) and not IECore.Object.isAbstractType(c.__name__)
			except:
				return False
	
		group = CompoundObject()
		objectClasses = filter(objectDerived, map(lambda x: getattr(IECore, x), dir(IECore)))
		notCreated = []
		notCasted = []
		for c in objectClasses:
			tId = Object.typeIdFromTypeName( c.__name__ )
			try:
				obj = Object.create(tId)
			except:
				notCreated.append(c)
			else:
				try:
					group["object"] = obj
				except:
					notCasted.append(c)

		errors = ""
		if len( notCreated ) :
			errors += "The following classes could not be created from Object.create() function: " + str( notCreated ) + "\n"
		if len( notCasted ):
			errors += "The following classes could not be casted to ObjectPtr:" + str( notCasted )

		if len( errors ):
			raise Exception( errors )

	def testDynamicDowncasting( self ):
		"""
		Tests if python can downcast a ObjectPtr to a proper derived Object instance.
		The downcast usually works, but when the object is used on a function that requires a derived class, then
		it didn't match. This problem was solved with the INTRUSIVE_PTR_PATCH used on the bindings.

		This does not test every class on IECore, but should...
		"""
		o = CompoundObject()
		o["first"] = IntData( 1 )
		t = CompoundData( { "first": o["first"] } )

	def testTypeIdToNameMapping( self ) :
	
		for tId in TypeId.values.values() :
		
			if tId==TypeId.Invalid :
				continue
			
			if Object.isType( tId ) :
				self.assertEqual( tId, Object.typeIdFromTypeName( Object.typeNameFromTypeId( tId ) ) )

	def testCreate( self ) :
		
		for tId in TypeId.values.values() :
		
			if tId==TypeId.Invalid :
				continue
				
			if Object.isType( tId ) and not Object.isAbstractType( tId ) :
						
				o = Object.create( tId )
				self.assertEqual( o.typeId(), tId )
				self.assertEqual( o.typeName(), Object.typeNameFromTypeId( tId ) )
				oo = Object.create( Object.typeNameFromTypeId( tId ) )
				self.assertEqual( oo.typeId(), tId )
				
	def testCopy( self ) :
	
		for tId in TypeId.values.values() :
		
			if tId==TypeId.Invalid :
				continue
			if Object.isType( tId ) and not Object.isAbstractType( tId ) :
				
				o = Object.create( tId )
				oo = o.copy()
				self.assertEqual( o, oo )
			
if __name__ == "__main__":
    unittest.main()   
