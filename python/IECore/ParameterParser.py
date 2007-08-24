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
import shlex

## This class defines a means of parsing a list of string arguments with respect to a Parameter definition.
# It's main use is in providing values to be passed to Op instances in the do script. It now also provides
# the reverse operation - serialising parameter values into a form which can later be parsed.
#
# \ingroup python
class ParameterParser :

	def __init__( self ) :
	
		self.__typesToParsers = {
			IECore.BoolParameter : self.__parseBool,
			IECore.IntParameter : ( lambda args, parameter : self.__parseNumeric( IECore.IntData, True, args, parameter ) ),
			IECore.FloatParameter : ( lambda args, parameter : self.__parseNumeric( IECore.FloatData, False, args, parameter ) ),
			IECore.DoubleParameter : ( lambda args, parameter : self.__parseNumeric( IECore.DoubleData, False, args, parameter ) ),
			IECore.StringParameter : self.__parseString,
			IECore.ValidatedStringParameter : self.__parseString,
			IECore.PathParameter : self.__parseString,
			IECore.FileNameParameter : self.__parseString,
			IECore.DirNameParameter : self.__parseString,
			IECore.FileSequenceParameter : self.__parseString,
			IECore.FrameListParameter : self.__parseString,
			IECore.StringVectorParameter : self.__parseStringArray,
			IECore.IntVectorParameter : ( lambda args, parameter : self.__parseNumericArray( IECore.IntVectorData, True, args, parameter ) ),
			IECore.FloatVectorParameter : ( lambda args, parameter : self.__parseNumericArray( IECore.FloatVectorData, False, args, parameter ) ),
			IECore.DoubleVectorParameter : ( lambda args, parameter : self.__parseNumericArray( IECore.DoubleVectorData, False, args, parameter ) ),
			IECore.V2iParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.V2iData, IECore.V2i, 2, True, args, parameter ) ),
			IECore.V3iParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.V3iData, IECore.V3i, 3, True, args, parameter ) ),
			IECore.V2fParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.V2fData, IECore.V2f, 2, False, args, parameter ) ),
			IECore.V3fParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.V3fData, IECore.V3f, 3, False, args, parameter ) ),
			IECore.V2dParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.V2dData, IECore.V2d, 2, False, args, parameter ) ),
			IECore.V3dParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.V3dData, IECore.V3d, 3, False, args, parameter ) ),
			IECore.Color3fParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.Color3fData, IECore.Color3f, 3, False, args, parameter ) ),
			IECore.Color4fParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.Color4fData, IECore.Color4f, 4, False, args, parameter ) ),
			IECore.M44fParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.M44fData, IECore.M44f, 16, False, args, parameter ) ),
			IECore.M44dParameter : ( lambda args, parameter : self.__parseNumericCompound( IECore.M44dData, IECore.M44d, 16, False, args, parameter ) ),
			IECore.Box2fParameter : ( lambda args, parameter : self.__parseBox( IECore.Box2fData, IECore.Box2f, IECore.V2f, False, args, parameter ) ),
			IECore.Box3fParameter : ( lambda args, parameter : self.__parseBox( IECore.Box3fData, IECore.Box3f, IECore.V3f, False, args, parameter ) ),
			IECore.Box2dParameter : ( lambda args, parameter : self.__parseBox( IECore.Box2dData, IECore.Box2d, IECore.V2d, False, args, parameter ) ),
			IECore.Box3dParameter : ( lambda args, parameter : self.__parseBox( IECore.Box3dData, IECore.Box3d, IECore.V3d, False, args, parameter ) ),
			IECore.Box2iParameter : ( lambda args, parameter : self.__parseBox( IECore.Box2iData, IECore.Box2i, IECore.V2i, True, args, parameter ) ),
			IECore.Box3iParameter : ( lambda args, parameter : self.__parseBox( IECore.Box3iData, IECore.Box3i, IECore.V3i, True, args, parameter ) ),
		}
		
		self.__typesToSerialisers = {
			IECore.BoolParameter : self.__serialiseUsingStr,
			IECore.IntParameter : self.__serialiseUsingStr,
			IECore.FloatParameter : self.__serialiseUsingStr,
			IECore.DoubleParameter : self.__serialiseUsingStr,
			IECore.StringParameter : self.__serialiseString,
			IECore.ValidatedStringParameter : self.__serialiseString,
			IECore.PathParameter : self.__serialiseString,
			IECore.FileNameParameter : self.__serialiseString,
			IECore.DirNameParameter : self.__serialiseString,
			IECore.FileSequenceParameter : self.__serialiseString,
			IECore.FrameListParameter : self.__serialiseString,
			IECore.StringVectorParameter : self.__serialiseStringArray,
			IECore.IntVectorParameter : self.__serialiseUsingStr,
			IECore.FloatVectorParameter : self.__serialiseUsingStr,
			IECore.DoubleVectorParameter : self.__serialiseUsingStr,
			IECore.V2iParameter : self.__serialiseUsingStr,
			IECore.V3iParameter : self.__serialiseUsingStr,
			IECore.V2fParameter : self.__serialiseUsingStr,	
			IECore.V3fParameter : self.__serialiseUsingStr,	
			IECore.V2dParameter : self.__serialiseUsingStr,	
			IECore.V3dParameter : self.__serialiseUsingStr,	
			IECore.Color3fParameter : self.__serialiseUsingStr,	
			IECore.Color4fParameter : self.__serialiseUsingStr,
			IECore.M44fParameter : self.__serialiseUsingStr,
			IECore.M44dParameter : self.__serialiseUsingStr,
			IECore.Box2fParameter : self.__serialiseUsingStr,
			IECore.Box3fParameter : self.__serialiseUsingStr,
			IECore.Box2dParameter : self.__serialiseUsingStr,
			IECore.Box3dParameter : self.__serialiseUsingStr,
			IECore.Box2iParameter : self.__serialiseUsingStr,
			IECore.Box3iParameter : self.__serialiseUsingStr,
		}

	## Parses the args to set the values of the parameters
	# held by a CompoundParameters object. If any invalid values are discovered will
	# raise a descriptive exception. args may either be a list of strings or a string
	# itself.
	def parse(self, args, parameters):
			
		if type( args ) is str :
			args = shlex.split( args )

		parmsFound = {}
		result = parameters.defaultValue
		args = args[:] # a copy we can mess around with
		while len(args):
		
			if not len( args[0] ) or args[0][0] != "-" :
				raise SyntaxError( "Expected a flag argument (beginning with \"-\")" )

			# find the parameter being specified. this might be the child of a compound
			# denoted by "." separated names.
			name = args[0][1:]
			nameSplit = name.split( "." )

			# recurses down the parameters tree
			p = parameters

			if p.__class__.__name__ == 'CompoundParameter':
				for i in range(0, len(nameSplit)):
					if not nameSplit[i] in p:
						raise SyntaxError( "\"%s\" is not a parameter name." % name )
					p = p[nameSplit[i]]
				
			del args[0]
			
			if not len( args ) :
				raise SyntaxError( "Expected at least one argument following flag \"-%s\"." % name )
				
			# see if the argument being specified is a preset name, in which case we
			# don't need a special parser.	
			if args[0] in p.presets() :
				
				p.setValue( args[0] )
				del args[0]
			
			# or if it's asking us to read a value from a file, in which case we
			# also don't need a special parser	
			elif args[0].startswith( "read:" ) :
			
				fileName = args[0][5:]
				r = IECore.Reader.create( fileName )
				if not r :
					raise RuntimeError( "Unable to create reader for file \"%s\"." % fileName )
					
				p.setValidatedValue( r.read() )
				del args[0]
			
			# otherwise we're gonna need a specialised parser	
			else :	
				parmType = type( p )			
				if not parmType in self.__typesToParsers :
					raise SyntaxError( "No parser available for parameter \"%s\"." % name )

				f = self.__typesToParsers[parmType]
				try :
					f( args, p )
				except Exception, e :
					raise SyntaxError( "Problem parsing parameter \"%s\" : %s " % ( name, e ) )




	## Returns a string representing the values contained within parameters.
	# This string can later be passed to parse() to retrieve the values. May
	# throw an exception if the value held by any of the parameters is not valid.
	def serialise( self, parameters ) :
	
		return self.__serialiseWalk( parameters, "" )

	
	def __serialiseWalk(self, parameter, rootName):
		"""
		recursively assemble a serialized command-line argument style parameter instance
		"""

		result = ''
		
		if parameter.isInstanceOf("CompoundParameter"):

			# concatenate the path
			path = rootName
			if parameter.name: path = path + parameter.name + '.'

			# recurse
			result = ' '.join(map(lambda cp: self.__serialiseWalk(parameter[cp], path), parameter.keys()))
			
		else:

			# leaf parameter
			parmType = type(parameter)
			if not parmType in self.__typesToSerialisers:
				raise RuntimeError("No serialiser available for parameter \"%s\"" % parameter.name)

			# find the appropriate serializer for the given parameter type
			f = self.__typesToSerialisers[parmType]
			try:
				s = f(parameter)
			except Exception, e:
				raise RuntimeError("Problem serialising parameter \"%s\" : %s" % (parameter.name, e))

			# serialize
			result = '-' + rootName + parameter.name + ' ' + s
				
		return result

				
	@staticmethod		
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
		
		if not len( args ) :
			raise SyntaxError( "Expected a boolean value." )
		
		if not args[0] in validValues :
			raise SyntaxError( "Expected one of %s" % ", ".join( validValues.keys() ) )
		
		parameter.setValidatedValue( IECore.BoolData( validValues[args[0]] ) )
		del args[0]
		
	@staticmethod		
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
		
	@staticmethod		
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

	@staticmethod
	def __parseBox( dataType, boxType, elementType, integer, args, parameter ) :
	
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
				
		parameter.setValidatedValue( dataType( boxType( elementType( *values[:n/2] ), elementType( *values[n/2:] ) ) ) )
	
	@staticmethod
	def __parseString( args, parameter ) :
	
		parameter.setValidatedValue( IECore.StringData( args[0] ) )
		del args[0]
		
	@staticmethod
	def __parseStringArray( args, parameter ) :
	
		d = IECore.StringVectorData()
		foundFlag = False
		while len( args ) and not foundFlag :
			a = args[0]
			if a[0] == "-" :
				foundFlag = True
			else :
				d.append( a )
				del args[0]
			
		parameter.setValidatedValue( d )

	@staticmethod
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
					
		if not d.size() :
			raise SyntaxError( "Expected at least one numeric value." )
			
		parameter.setValidatedValue( d )
		
	@staticmethod
	def __serialiseString( parameter ) :
	
		return "'" + parameter.getTypedValue() + "'"
	
	@staticmethod
	def __serialiseStringArray( parameter ) :
	
		result = []
		for i in parameter.getValue() :
			result.append( "'" + i + "'" )	
		return " ".join( result )
	
	@staticmethod
	def __serialiseUsingStr( parameter ) :
		return str( parameter.getValidatedValue() )

__all__ = [ "ParameterParser" ]
