##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

import inspect
import imath
import re

import IECore

## This class defines a means of parsing a list of string arguments with respect to a Parameter definition.
# It's main use is in providing values to be passed to Op instances in the do script. It now also provides
# the reverse operation - serialising parameter values into a form which can later be parsed.
# \ingroup python
class ParameterParser :

	__typesToParsers = {}
	__typesToSerialisers = {}

	## Parses the args to set the values of the parameters
	# held by a CompoundParameter object. args must be a list of strings.
	#
	# Parsing expects a series of entries of the form -parameterName value(s). It
	# is also possible to specify a series of child parameters which can be parsed without
	# their name being specified - this provides much less verbose command lines for commonly
	# used parameters, and the possibility of supporting the very common "command [options] filenames"
	# form of command. Flagless parameters are specified as userData in the parameters object passed -
	# this should be a StringVectorData named "flagless" in a CompoundData called "parser" -
	# these flagless parameters are parsed in the order they are specified in the data element.
	# If parsing fails at any time then a descriptive exception is raised.
	def parse(self, args, parameters):

		if not type( args ) is list :
			raise TypeError( "args must be a python list!" )

		flagless = None
		flaglessIndex = 0
		if "parser" in parameters.userData() :
			if "flagless" in parameters.userData()["parser"] :
				flagless = parameters.userData()["parser"]["flagless"]
				if not flagless.isInstanceOf( IECore.StringVectorData.staticTypeId() ) :
					raise TypeError( "\"flagless\" parameters must be specified by a StringVectorData instance." )

		parmsFound = {}
		result = parameters.defaultValue
		args = args[:] # a copy we can mess around with
		while len(args):

			# get the name of the parameter being specified
			if len( args[0] ) and args[0][0]=="-" :
				name = args[0][1:]
				del args[0]
			elif flagless and flaglessIndex < flagless.size() :
				name = flagless[flaglessIndex]
				flaglessIndex += 1
			else :
				raise SyntaxError( "Expected a flag argument (beginning with \"-\")" )

			# find the parameter being specified. this might be the child of a compound
			# denoted by "." separated names.
			nameSplit = name.split( "." )

			# recurses down the parameters tree
			p = parameters

			if p.__class__.__name__ == 'CompoundParameter':
				for i in range(0, len(nameSplit)):
					if not nameSplit[i] in p:
						raise SyntaxError( "\"%s\" is not a parameter name." % name )
					p = p[nameSplit[i]]

			if len( args ) :

				# see if the argument being specified is a preset name, in which case we
				# don't need a special parser.
				if args[0] in p.getPresets() :

					p.setValue( args[0] )
					del args[0]
					continue

				# or if it's asking us to read a value from a file, in which case we
				# also don't need a special parser
				if args[0].startswith( "read:" ) :

					fileName = args[0][5:]
					r = IECore.Reader.create( fileName )
					if not r :
						raise RuntimeError( "Unable to create reader for file \"%s\"." % fileName )

					p.setValidatedValue( r.read() )
					del args[0]
					continue

				# or if it's asking us to evaluate a python string to create a value,
				# in which case we also don't need a special parser
				if args[0].startswith( "python:" ) :

					toEval = args[0][7:]
					r = eval( toEval )
					p.setValidatedValue( r )
					del args[0]
					continue

			# we're gonna need a specialised parser
			parser = None
			typeId = p.typeId()
			while typeId != IECore.TypeId.Invalid :
				if typeId in self.__typesToParsers :
					parser = self.__typesToParsers[typeId]
					break
				typeId = IECore.RunTimeTyped.baseTypeId( typeId )

			if parser is None :
				raise SyntaxError( "No parser available for parameter \"%s\"." % name )

			try :
				parser( args, p )
			except Exception as e :
				raise SyntaxError( "Problem parsing parameter \"%s\" : %s " % ( name, e ) )

	## Returns a list of strings representing the values contained within parameters,
	# or alternately, the values contained within the values argument. This can later
	# be passed to parse() to retrieve the values. Parameter values will be validated,
	# and may throw an exception, unless the alternate values argument is used.
	def serialise( self, parameters, values=None ) :

		if values is None :
			values = parameters.getValidatedValue()

		return self.__serialiseWalk( parameters, values, "" )

	def __serialiseWalk( self, parameter, value, rootName ) :

		# Allow parameters to request not to be serialised.
		if "parser" in parameter.userData() and "serialise" in parameter.userData()["parser"]:
			if not parameter.userData()["parser"]["serialise"].value:
				return []

		# Try to find a serialiser
		serialiser = None
		typeId = parameter.typeId()
		while typeId != IECore.TypeId.Invalid :
			if typeId in self.__typesToSerialisers :
				serialiser = self.__typesToSerialisers[typeId]
				break
			if typeId != IECore.TypeId.CompoundParameter :
				# consider serialisers for base classes.
				typeId = IECore.RunTimeTyped.baseTypeId( typeId )
			else :
				# don't consider serialiser for base classes of CompoundParameter
				# so that it doesn't get an unecessary serialisation in the case
				# of a serialiser for Parameter being registered.
				break

		if serialiser is None and not isinstance( parameter, IECore.CompoundParameter ) :
			# bail if no serialiser available, unless it's a CompoundParameter in which case we'll content
			# ourselves with serialising the children if no serialiser is available.
			raise RuntimeError( "No serialiser available for parameter \"%s\"" % parameter.name )

		result = []
		if serialiser is not None :
			# we have a registered serialiser - use it
			try:
				s = serialiser( parameter, value )
				if not isinstance( s, list ) :
					raise RuntimeError( "Serialiser did not return a list." )
				for ss in s :
					if not isinstance( ss, str ) :
						raise RuntimeError( "Serialiser returned a list with an element which is not a string." )
			except Exception as e:
				raise RuntimeError("Problem serialising parameter \"%s\" : %s" % (parameter.name, e))

			# serialize
			result += [ '-' + rootName + parameter.name ] + s

		# recurse to children of CompoundParameters
		if parameter.isInstanceOf( IECore.CompoundParameter.staticTypeId() ) :
			path = rootName
			if parameter.name :
				path = path + parameter.name + '.'
			# recurse
			for childParm in parameter.values() :
				if value is not None and childParm.name in value.keys() :
					result += self.__serialiseWalk( childParm, value[childParm.name], path )
				else :
					result += self.__serialiseWalk( childParm, value, path )

		return result

	@classmethod
	## Registers a parser and serialiser for a new Parameter type.
	def registerType( cls, typeId, parser, serialiser ) :

		cls.__typesToParsers[typeId] = parser
		cls.__typesToSerialisers[typeId] = serialiser

	@classmethod
	## Registers a default parser and serialiser for a Parameter type
	# for which repr( parameter.getValue() ) yields a valid python statement.
	def registerTypeWithRepr( cls, typeId ) :

		cls.__typesToParsers[typeId] = None
		cls.__typesToSerialisers[typeId] = _serialiseUsingRepr

###################################################################################################################
# parsers and serialisers for the built in parameter types
###################################################################################################################

def __parseBool( args, parameter ) :

	validValues = {
		"True" : True,
		"False" : False,
		"On" : True,
		"Off" : False,
		"1" : True,
		"0" : False,
		"true" : True,
		"false" : False,
		"on" : True,
		"off" : False,
	}

	if not len( args ) or args[0] not in validValues :
		if parameter.defaultValue.value :
			if not len( args ) :
				raise SyntaxError( "Expected a boolean value." )
			else :
				raise SyntaxError( "Expected one of %s" % ", ".join( validValues.keys() ) )
		else :
			# if the default value of a parameter is False,
			# and no value has been provided after the "-parameterName"
			# flag, then we turn it on.
			parameter.setValidatedValue( IECore.BoolData( True ) )
			return

	parameter.setValidatedValue( IECore.BoolData( validValues[args[0]] ) )
	del args[0]

def __parseNumeric( dataType, integer, args, parameter ) :

	if not len( args ) :
		raise SyntaxError( "Expected a numeric value." )

	if integer :

		try :
			value = int( args[0] )
		except :
			raise SyntaxError( "Expected an integer value." )

	else :

		try :
			value = float( args[0] )
		except :
			raise SyntaxError( "Expected a float value." )

	parameter.setValidatedValue( dataType( value ) )
	del args[0]

def __parseNumericCompound( dataType, elementType, n, integer, args, parameter ) :

	if not len( args ) :
		raise SyntaxError( "Expected %d numeric values." % n )

	values = []
	for i in range( 0, n ) :

		if integer :

			try :
				values.append( int( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d integer values." % n )

		else :

			try :
				values.append( float( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d float values." % n )

	parameter.setValidatedValue( dataType( elementType( *values ) ) )

def __parseBoxOrLine( dataType, boxType, elementType, integer, args, parameter ) :

	n = elementType.dimensions() * 2

	if not len( args ) :
		raise SyntaxError( "Expected %d numeric values." % n )

	values = []
	for i in range( 0, n ) :

		if integer :

			try :
				values.append( int( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d integer values." % n )

		else :

			try :
				values.append( float( args[0] ) )
				del args[0]
			except :
				raise SyntaxError( "Expected %d float values." % n )

	parameter.setValidatedValue( dataType( boxType( elementType( *values[:n//2] ), elementType( *values[n//2:] ) ) ) )

def __parseString( args, parameter ) :
	parameter.setValidatedValue( IECore.StringData( args[0] ) )
	del args[0]

def __parseStringArray( args, parameter ) :

	d = IECore.StringVectorData()

	acceptFlags = False
	if "parser" in parameter.userData() and "acceptFlags" in parameter.userData()["parser"] :
		acceptFlags = parameter.userData()["parser"]["acceptFlags"].value

	if acceptFlags :
		d.extend( args )
		del args[:]
	else :
		while len( args ) :
			a = args[0]
			if len( a ) and a[0] == "-" :
				break

			# special case where a leading "-" is escaped
			# to not be confused with a new parameter name
			# ex (parameter.a is a StringVectorParameter)
			# `-parameter.a test second \-extra -parameter.b ...`
			# where the values are ["test", "second", "-extra"]
			if re.match( r"^\\+-.*", a ):
				a = a[1:]

			d.append( a )
			del args[0]

	parameter.setValidatedValue( d )

def __parseBoolArray( args, parameter ) :

	d = IECore.BoolVectorData()

	validValues = {
		"True" : True,
		"False" : False,
		"On" : True,
		"Off" : False,
		"1" : True,
		"0" : False,
		"true" : True,
		"false" : False,
		"on" : True,
		"off" : False,
	}

	done = False
	while len( args ) and not done :
		a = args[0]
		if len(a) and a[0] == "-" :
			done = True
		elif not args[0] in validValues :
			raise SyntaxError( "Expected one of %s" % ", ".join( validValues.keys() ) )
		else :
			try :
				d.append( validValues[args[0]] )
				del args[0]
			except :
				done = True

	parameter.setValidatedValue( d )

def __parseNumericArray( dataType, integer, args, parameter ) :

	d = dataType()

	done = False
	while not done and len( args ) :
		if integer :
			try :
				d.append( int( args[0] ) )
				del args[0]
			except :
				done = True
		else :
			try :
				d.append( float( args[0] ) )
				del args[0]
			except :
				done = True

	parameter.setValidatedValue( d )

def __parseTransformationMatrix( dataType, args, parameter ) :

	if not len(args) :
		raise SyntaxError( "Expected 29 values." )

	if dataType == IECore.TransformationMatrixfData :
		vecType = imath.V3f
		angleType = imath.Eulerf
		orientationType = imath.Quatf
	else :
		vecType = imath.V3d
		angleType = imath.Eulerd
		orientationType = imath.Quatd

	t = type(dataType().value)()

	t.translate = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]

	t.scale = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]

	t.shear = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]

	t.rotate = angleType( float(args[0]), float(args[1]), float(args[2] ) )
	t.rotate.setOrder( getattr( imath.Eulerf, args[3] ) )
	del args[0:4]

	t.rotationOrientation = orientationType( float(args[0]), float(args[1]), float(args[2] ), float(args[3]) )
	del args[0:4]

	t.rotatePivot = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]

	t.rotatePivotTranslation = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]

	t.scalePivot = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]

	t.scalePivotTranslation = vecType( float(args[0]), float(args[1]), float(args[2] ) )
	del args[0:3]

	parameter.setValidatedValue( dataType(t) )

def __parseObject( args, parameter ) :

	v = args[0]
	v = IECore.hexToDecCharVector( v )
	mio = IECore.MemoryIndexedIO( v, IECore.IndexedIO.OpenMode.Read )
	v = IECore.Object.load( mio, "v" )
	parameter.setValidatedValue( v )
	del args[0]

def __serialiseString( parameter, value ) :

	return [ value.value ]

def __serialiseStringArray( parameter, value ) :

	return list(value)

def __serialiseStringArrayWithEscape( parameter, value ) :

	# when serialising string arrays that can start with "-"
	# we take care to escape it with a leading "\",
	# so it won't be confused with a new argument name
	# we also escape anything that starts with any number of "\" followed by a "-"
	# so they don't confused with an intentional escape
	return [ "\\{}".format( x ) if re.match( r"^\\*-.*", x ) else x for x in value ]

def __serialiseUsingStr( parameter, value ) :

	return [ str(value) ]

def __serialiseUsingSplitStr( parameter, value ) :

	return str(value).split()

def _serialiseUsingRepr( parameter, value ) :

	return [ "python:" + repr(value) ]

def __serialiseTransformationMatrix( parameter, value ) :

	def serialiseVec( vec ) :

		return [ str( vec[x] ) for x in range( 0, vec.dimensions() ) ]

	def serialiseQuat( quat ) :

		return [ str( quat.r() ) ] + serialiseVec( quat.v() )

	t = value.value
	retList = []
	retList.extend( serialiseVec( t.translate ) )
	retList.extend( serialiseVec( t.scale ) )
	retList.extend( serialiseVec( t.shear ) )
	retList.extend( serialiseVec( t.rotate ) )
	retList.append( str( t.rotate.order() ) )
	retList.extend( serialiseQuat( t.rotationOrientation ) )
	retList.extend( serialiseVec( t.rotatePivot ) )
	retList.extend( serialiseVec( t.rotatePivotTranslation ) )
	retList.extend( serialiseVec( t.scalePivot ) )
	retList.extend( serialiseVec( t.scalePivotTranslation ) )

	return retList

def __serialiseObject( parameter, value ) :

	mio = IECore.MemoryIndexedIO( IECore.CharVectorData(), IECore.IndexedIO.OpenMode.Write )
	value.save( mio, "v" )
	buf = mio.buffer()
	return [ IECore.decToHexCharVector( buf ) ]

def __serialiseVec( parameter, value ) :

	vec = value.value
	return [ str( vec[x] ) for x in range( 0, vec.dimensions() ) ]

def __serialiseBox( parameter, value ) :

	box = value.value
	result = []
	for vec in box.min(), box.max() :
		for x in range( 0, vec.dimensions() ) :
			result.append( str( vec[x] ) )

	return result

def __serialiseLineSegment( parameter, value ) :

	line = value.value
	result = []
	for vec in line.p0, line.p1 :
		for x in range( 0, vec.dimensions() ) :
			result.append( str( vec[x] ) )

	return result

ParameterParser.registerType( IECore.BoolParameter.staticTypeId(), __parseBool, __serialiseUsingStr )
ParameterParser.registerType( IECore.IntParameter.staticTypeId(), ( lambda args, parameter : __parseNumeric( IECore.IntData, True, args, parameter ) ), __serialiseUsingStr )
ParameterParser.registerType( IECore.FloatParameter.staticTypeId(), ( lambda args, parameter : __parseNumeric( IECore.FloatData, False, args, parameter ) ), __serialiseUsingStr )
ParameterParser.registerType( IECore.DoubleParameter.staticTypeId(), ( lambda args, parameter : __parseNumeric( IECore.DoubleData, False, args, parameter ) ), __serialiseUsingStr )
ParameterParser.registerType( IECore.StringParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.ValidatedStringParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.PathParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.PathVectorParameter.staticTypeId(), __parseStringArray, __serialiseStringArray )
ParameterParser.registerType( IECore.FileNameParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.DirNameParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.FileSequenceParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.FrameListParameter.staticTypeId(), __parseString, __serialiseString )
ParameterParser.registerType( IECore.StringVectorParameter.staticTypeId(), __parseStringArray, __serialiseStringArrayWithEscape )
ParameterParser.registerType( IECore.BoolVectorParameter.staticTypeId(), ( lambda args, parameter : __parseBoolArray( args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.IntVectorParameter.staticTypeId(), ( lambda args, parameter : __parseNumericArray( IECore.IntVectorData, True, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.FloatVectorParameter.staticTypeId(), ( lambda args, parameter : __parseNumericArray( IECore.FloatVectorData, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.DoubleVectorParameter.staticTypeId(), ( lambda args, parameter : __parseNumericArray( IECore.DoubleVectorData, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.V2iParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V2iData, imath.V2i, 2, True, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.V3iParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V3iData, imath.V3i, 3, True, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.V2fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V2fData, imath.V2f, 2, False, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.V3fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V3fData, imath.V3f, 3, False, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.V2dParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V2dData, imath.V2d, 2, False, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.V3dParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.V3dData, imath.V3d, 3, False, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.Color3fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.Color3fData, imath.Color3f, 3, False, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.Color4fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.Color4fData, imath.Color4f, 4, False, args, parameter ) ), __serialiseVec )
ParameterParser.registerType( IECore.M44fParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.M44fData, imath.M44f, 16, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.M44dParameter.staticTypeId(), ( lambda args, parameter : __parseNumericCompound( IECore.M44dData, imath.M44d, 16, False, args, parameter ) ), __serialiseUsingSplitStr )
ParameterParser.registerType( IECore.Box2fParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box2fData, imath.Box2f, imath.V2f, False, args, parameter ) ), __serialiseBox )
ParameterParser.registerType( IECore.Box3fParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box3fData, imath.Box3f, imath.V3f, False, args, parameter ) ), __serialiseBox )
ParameterParser.registerType( IECore.Box2dParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box2dData, imath.Box2d, imath.V2d, False, args, parameter ) ), __serialiseBox )
ParameterParser.registerType( IECore.Box3dParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box3dData, imath.Box3d, imath.V3d, False, args, parameter ) ), __serialiseBox )
ParameterParser.registerType( IECore.Box2iParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box2iData, imath.Box2i, imath.V2i, True, args, parameter ) ), __serialiseBox )
ParameterParser.registerType( IECore.Box3iParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.Box3iData, imath.Box3i, imath.V3i, True, args, parameter ) ), __serialiseBox )
ParameterParser.registerType( IECore.LineSegment3fParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.LineSegment3fData, IECore.LineSegment3f, imath.V3f, False, args, parameter ) ), __serialiseLineSegment )
ParameterParser.registerType( IECore.LineSegment3dParameter.staticTypeId(), ( lambda args, parameter : __parseBoxOrLine( IECore.LineSegment3dData, IECore.LineSegment3d, imath.V3d, False, args, parameter ) ), __serialiseLineSegment )
ParameterParser.registerType( IECore.SplineffParameter.staticTypeId(), None, _serialiseUsingRepr )
ParameterParser.registerType( IECore.SplinefColor3fParameter.staticTypeId(), None, _serialiseUsingRepr )
ParameterParser.registerType( IECore.TransformationMatrixfParameter.staticTypeId(), ( lambda args, parameter : __parseTransformationMatrix( IECore.TransformationMatrixfData, args, parameter ) ), __serialiseTransformationMatrix )
ParameterParser.registerType( IECore.TransformationMatrixdParameter.staticTypeId(), ( lambda args, parameter : __parseTransformationMatrix( IECore.TransformationMatrixdData, args, parameter ) ), __serialiseTransformationMatrix )
ParameterParser.registerType( IECore.ObjectParameter.staticTypeId(), __parseObject, __serialiseObject )

__all__ = [ "ParameterParser" ]
