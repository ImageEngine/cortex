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

import IECore

## Registers a type id for an extension class. This makes TypeId.className
# available and also checks that no other type is trying to use the same id.
# It raises a RuntimeError if a conflicting type is already registered.
def registerTypeId( className, typeId ) :


	# check this type hasn't been registered already
	if hasattr( IECore.TypeId, className ):
		if getattr( IECore.TypeId, className ) != typeId:
			raise RuntimeError( "Type \"%s\" is already registered." % className )
			
		return	

	if typeId in IECore.TypeId.values :
		raise RuntimeError( "TypeId \"%d\" is already registered as \"%s\"." % (typeId, IECore.TypeId.values[typeId] ) )
		
	# register the new type id
	setattr( IECore.TypeId, className, IECore.TypeId( typeId ) )

## This function adds the necessary function definitions to a python
# class for it to properly implement the RunTimeTyped interface. It should
# be called once for all python classes inheriting from RunTimeTyped. It also
# calls registerTypeId() for you.
def makeRunTimeTyped( typ, typId, baseClass ) :

	registerTypeId( typ.__name__, typId )

	# add the typeId and typeName method overrides
	typ.typeId = lambda x : typId
	typ.typeName = lambda x: typ.__name__

	# add the staticTypeId and staticTypeName overrides
	typ.staticTypeId = staticmethod( lambda : typId )
	typ.staticTypeName = staticmethod( lambda : typ.__name__ )	
		
	# add the isInstanceOf method override
	def isInstanceOf( self, t, baseClass ) :
				
		if type( t ) is str :
			if self.staticTypeName() == t :
				return True
			else :
				return baseClass.isInstanceOf( self, t )
		elif type( t ) is IECore.TypeId :
			if self.staticTypeId() == t :
				return True
			else :
				return baseClass.isInstanceOf( self, t )
		
		raise TypeError( "Invalid type specifier ( %s )" % str( t ) )
		
	typ.isInstanceOf = lambda self, t : isInstanceOf( self, t, baseClass )
	
	# add the inheritsFrom method override
	def inheritsFrom( t, baseClass ) :
	
		if type( t ) is str :
			if baseClass.staticTypeName() == t :
				return True
			else :
				return baseClass.inheritsFrom( t )
		elif type(t) is IECore.TypeId :
			if baseClass.staticTypeId() == t :
				return True
			else :
				return baseClass.inheritsFrom( t )
		
		raise TypeError( "Invalid type specifier ( %s )" % str( t ) )
		
	typ.inheritsFrom = staticmethod( lambda t : inheritsFrom( t, baseClass ) )
	
__all__ = [ "registerTypeId", "makeRunTimeTyped" ]
