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

__nextDynamicRunTimeTypedId = None

## This function adds the necessary function definitions to a python
# class for it to properly implement the RunTimeTyped interface. It should
# be called once for all python classes inheriting from RunTimeTyped. It also
# calls registerTypeId() for you.
# typId is optional and if not defined, this function will associate a dynamic Id
#       in the range FirstDynamicTypeId and LastDynamicTypeId from TypeIds.h.
#       It's necessary to specify type Id for Object derived class or anything that
#       is serializable.
# If typeName is not specified then the name of the class itself is used - you may wish
# to provide an explicit typeName in order to prefix the name with a module name.
def registerRunTimeTyped( typ, typId = None, typeName = None ) :

	if typeName is None :
		typeName = typ.__name__
	
	runTypedBaseClass = filter( lambda c: issubclass( c, IECore.RunTimeTyped ), typ.__bases__ )[0]

	# constants below are the same as in TypeIds.h
	FirstDynamicTypeId = 300000
	LastDynamicTypeId = 399999

	# check if overwritting registration.
	if not hasattr( IECore.TypeId, typeName ) :

		if typId is None :

			global __nextDynamicRunTimeTypedId

			if __nextDynamicRunTimeTypedId is None :
				__nextDynamicRunTimeTypedId = FirstDynamicTypeId
			elif __nextDynamicRunTimeTypedId > LastDynamicTypeId:
				raise Exception, "Too many dynamic RunTimeTyped registered classes! You must change TypeIds.h and rebuild Cortex."

			typId = __nextDynamicRunTimeTypedId

			__nextDynamicRunTimeTypedId += 1

		__registerTypeId( IECore.TypeId( typId ), typeName, IECore.TypeId( runTypedBaseClass.staticTypeId() ) )

	else :
		# check if the new type Id is compatible with the previously registered one.
		prevTypId = getattr( IECore.TypeId, typeName )
		if prevTypId in xrange( FirstDynamicTypeId, LastDynamicTypeId+1 ) :
			if not typId is None :
				raise Exception, "Trying to set a type ID for %s previously registered as a dynamic type Id!" % typeName
		else :
			if typId is None :
				raise Exception, "Trying to re-register type %s as dynamic type Id!" % typeName
			elif typId != prevTypId :
				raise Exception, "Trying to re-register %s under different type Id: %s != %s" % ( typeName, str(typId), prevTypId )
		# necessary when the typeid is defined in IECore/TypeIds.h and bound in TypeIdBinding.cpp, but then
		# the class for that typeid is implemented in python (currently ClassParameter does this).
		if IECore.RunTimeTyped.typeNameFromTypeId( prevTypId )=="" :
			IECore.RunTimeTyped.registerType( prevTypId, typeName, IECore.TypeId( runTypedBaseClass.staticTypeId() ) )

	# Retrieve the correct value from the enum
	tId = getattr( IECore.TypeId, typeName )

	# add the typeId and typeName method overrides
	typ.typeId = lambda x : tId
	typ.typeName = lambda x: typeName

	# add the staticTypeId, staticTypeName, baseTypeId, and baseTypeName overrides
	typ.staticTypeId = staticmethod( lambda : tId )
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
		elif type(t) is IECore.TypeId :
			if type( baseClass ) is list :
				for base in baseClass :
					if base.staticTypeId() == t :
						return True
			else:
				if baseClass.staticTypeId() == t :
					return True
		else:
			raise TypeError( "Invalid type specifier ( %s )" % str( t ) )

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
		elif type( t ) is IECore.TypeId :
			if self.staticTypeId() == t :
				return True
		else :
			raise TypeError( "Invalid type specifier ( %s )" % str( t ) )

		return inheritsFrom( t, baseClass )

	typ.isInstanceOf = lambda self, t : isInstanceOf( self, t, runTypedBaseClass )
