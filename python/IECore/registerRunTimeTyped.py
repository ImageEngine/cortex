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

import IECore

## Registers a type id for an extension class. This makes TypeId.className
# available and also checks that no other type is trying to use the same id.
# It raises a RuntimeError if a conflicting type is already registered.
def __registerTypeId( typeId, typeName, baseTypeId ) :

	assert( type( typeId ) is IECore.TypeId )
	assert( type( typeName ) is str )
	assert( type( baseTypeId ) is IECore.TypeId )

	# check this type hasn't been registered already
	if hasattr( IECore.TypeId, typeName ):
		if getattr( IECore.TypeId, typeName ) != typeId:
			raise RuntimeError( "Type \"%s\" is already registered." % typeName )

		return

	if typeId in IECore.TypeId.values :
		raise RuntimeError( "TypeId \"%d\" is already registered as \"%s\"." % (typeId, IECore.TypeId.values[typeId] ) )

	# update the TypeId enum
	setattr( IECore.TypeId, typeName, typeId )
	IECore.TypeId.values[ int( typeId ) ] = typeId

	# register the new type id
	IECore.RunTimeTyped.registerType( typeId, typeName, baseTypeId )

__nextTypeId = 300000 # Same as `TypeId::FirstPythonTypeId` defined in TypeIds.h

## This function adds the necessary function definitions to a python
# class for it to properly implement the RunTimeTyped interface. It should
# be called once for all python classes inheriting from RunTimeTyped.
# If `typeName` is not specified then the name of the class itself is used -
# you may wish to provide an explicit typeName in order to prefix the name
# with a module name.
## \todo It feels like this could probably be done automatically using a
# custom metaclass for RunTimeTyped?
def registerRunTimeTyped( typ, typeName = None ) :

	if typeName is None :
		typeName = typ.__name__

	runTypedBaseClass = next( c for c in typ.__bases__ if issubclass( c, IECore.RunTimeTyped ) )

	# As defined in TypeIds.h
	LastPythonTypeId = 399999

	if not hasattr( IECore.TypeId, typeName ) :

		# First registration.

		global __nextTypeId
		if __nextTypeId > LastPythonTypeId :
			raise Exception( "Too many dynamic RunTimeTyped registered classes! You must change TypeIds.h and rebuild Cortex." )

		typeId = IECore.TypeId( __nextTypeId )
		__nextTypeId += 1

		__registerTypeId( typeId, typeName, IECore.TypeId( runTypedBaseClass.staticTypeId() ) )

	else :

		# Re-registration - this can happen when reloading an Op for example.

		typeId = getattr( IECore.TypeId, typeName )
		assert( IECore.RunTimeTyped.typeNameFromTypeId( typeId ) != "" )

	# add the typeId and typeName method overrides
	typ.typeId = lambda x : typeId
	typ.typeName = lambda x: typeName

	# add the staticTypeId, staticTypeName, baseTypeId, and baseTypeName overrides
	typ.staticTypeId = staticmethod( lambda : typeId )
	typ.staticTypeName = staticmethod( lambda : typeName )
	typ.baseTypeId = staticmethod( lambda : runTypedBaseClass.staticTypeId() )
	typ.baseTypeName = staticmethod( lambda : runTypedBaseClass.staticTypeName() )

	# add the inheritsFrom method override
	def inheritsFrom( t, baseClass ) :

		if type( t ) is str :
			if type( baseClass ) is list :
				for base in baseClass :
					if base.staticTypeName() == t :
						return True
			else:
				if baseClass.staticTypeName() == t :
					return True
		else :
			if type( baseClass ) is list :
				for base in baseClass :
					if base.staticTypeId() == IECore.TypeId( t ) :
						return True
			else:
				if baseClass.staticTypeId() == IECore.TypeId( t ) :
					return True

		if type( baseClass ) is list :
			for base in baseClass:
				if base.inheritsFrom( t ):
					return True
		else:
			return baseClass.inheritsFrom( t )

		return False

	typ.inheritsFrom = staticmethod( lambda t : inheritsFrom( t, runTypedBaseClass ) )


	# add the isInstanceOf method override
	def isInstanceOf( self, t, baseClass ) :

		if type( t ) is str :
			if self.staticTypeName() == t :
				return True
		else :
			if IECore.TypeId( t ) == self.staticTypeId() :
				return True

		return inheritsFrom( t, baseClass )

	typ.isInstanceOf = lambda self, t : isInstanceOf( self, t, runTypedBaseClass )
