##########################################################################
#
#  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

import six

import IECore

## The ClassVectorParameter is similar to the ClassParameter but instead of holding
# a single class it holds many classes, each storing its parameters under a named
# child of the ClassVectorParameter.
class ClassVectorParameter( IECore.CompoundParameter ) :

	def __init__( self, name, description, searchPathEnvVar, classes=[], userData=None ) :

		IECore.CompoundParameter.__init__( self, name, description, userData=userData )

		self.__searchPathEnvVar = searchPathEnvVar
		self.__namesToInstances = {} # maps parameter names to [ classInstance, className, classVersion ] lists

		self.setClasses( classes )

	## Returns the name of the environment variable which defines paths to search for child classes on.
	def searchPathEnvVar( self ) :

		return self.__searchPathEnvVar

	## Returns a list of the classes held as children. These are returned in the same order
	# as the child parameters holding them. If withClassLoaderArgs is True then a list of
	# tuples is returned, with each tuple being of the form ( classInstance, parameterName, className, classVersion ).
	def getClasses( self, withClassLoaderArgs=False ) :

		result = []
		for k in self.keys() :
			if withClassLoaderArgs :
				instance = self.__namesToInstances[k]
				result.append( ( instance[0], k, instance[1], instance[2] ) )
			else :
				result.append( self.__namesToInstances[k][0] )

		return result

	## Sets the classes held as children. Classes must be a list of tuples of the form
	# ( parameterName, className, classVersion ). If any tuple in the list matches an existing
	# child class, then that class will be preserved rather than replaced with a new instance
	# of the same thing.
	def setClasses( self, classes ) :

		# validate arguments and figure out what child parameter names we need

		assert( isinstance( classes, list ) )

		neededNames = set()
		for c in classes :
			assert( isinstance( c, tuple ) )
			assert( len( c ) == 3 )
			assert( isinstance( c[0], str ) )
			assert( isinstance( c[1], str ) )
			assert( isinstance( c[2], int ) )
			if c[0] in neededNames :
				raise ValueError( "Duplicate parameter name \"%s\"" % c[0] )
			neededNames.add( c[0] )

		# first remove any existing parameters which we don't need

		for parameterName in self.keys() :
			if parameterName not in neededNames :
				self.removeClass( parameterName )

		# and then create any new ones we need and reload and reorder existing
		# ones as necessary

		for i in range( 0, len( classes ) ) :

			# modify or add a parameter for this class

			self.setClass( classes[i][0], classes[i][1], classes[i][2] )
			parameter = self[classes[i][0]]

			# make sure the parameter has the right order within the whole

			self.removeParameter( parameter )
			if len( self ) == i :
				self.addParameter( parameter )
			else :
				keys = self.keys()
				self.insertParameter( parameter, self[keys[i]] )

	## Returns the class instance that the named parameter represents,
	# or if withClassLoaderArgs is True, then returns a tuple of the
	# form ( classInstance, className, classVersion ).
	def getClass( self, parameterOrParameterName, withClassLoaderArgs=False ) :

		if isinstance( parameterOrParameterName, six.string_types ) :
			parameterName = parameterOrParameterName
		else :
			parameterName = parameterOrParameterName.name

		if withClassLoaderArgs :
			return tuple( self.__namesToInstances[parameterName] )
		else :
			return self.__namesToInstances[parameterName][0]

	## Sets the class held by the named parameter, if no such
	# parameter exists then one will be appended. To insert
	# a parameter somewhere other than the end, use getClasses()
	# and setClasses().
	def setClass( self, parameterOrParameterName, className, classVersion ) :

		if isinstance( parameterOrParameterName, six.string_types ) :
			parameterName = parameterOrParameterName
			parameter = self.parameter( parameterOrParameterName )
			if not parameter :
				parameter = IECore.CompoundParameter( parameterName, "" )
				self.addParameter( parameter )
		else :
			parameter = parameterOrParameterName
			parameterName = parameter.name

		instance = self.__namesToInstances.setdefault( parameterName, [ None, "", 0 ] )

		if [ className, classVersion ] != instance[1:] :

			loader = IECore.ClassLoader.defaultLoader( self.__searchPathEnvVar )
			instance[0] = loader.load( className, classVersion )()
			instance[1] = className
			instance[2] = classVersion

			parameter.clearParameters()
			parameter.addParameters( instance[0].parameters().values() )

			# copy user data over:
			parameter.userData().copyFrom( instance[0].parameters().userData() )

	## Removes the class held by the named parameter.
	def removeClass( self, parameterName ) :

		self.removeParameter( parameterName )
		del self.__namesToInstances[parameterName]

	## Returns a good name for a new parameter. It's not compulsory to use this
	# function (any unique name is fine) but it can be useful to keep a consistent
	# naming convention, and it removes the need to come up with unique names some other
	# way.
	def newParameterName( self, prefix="p" ) :

		existingNames = set( self.keys() )
		for i in range( 0, len( existingNames ) + 1 ) :
			parameterName = "%s%d" % ( prefix, i )
			if parameterName not in existingNames :
				return parameterName

	@staticmethod
	def _serialise( parameter, value ) :

		result = [ parameter.__searchPathEnvVar ]

		classes = parameter.getClasses( True )

		result.append( str( len( classes ) ) )

		result += [ x[1] for x in classes ]
		result += [ x[2] for x in classes ]
		result += [ str( x[3] ) for x in classes ]

		return result

	@staticmethod
	def _parse( args, parameter ) :

		parameter.__searchPathEnvVar = args[0]
		del args[0]

		numClasses = int( args[0] )
		del args[0]

		parameterNames = args[:numClasses]
		del args[:numClasses]

		classNames = args[:numClasses]
		del args[:numClasses]

		classVersions = [ int( x ) for x in args[:numClasses] ]
		del args[:numClasses]

		parameter.setClasses( zip( parameterNames, classNames, classVersions ) )

IECore.registerRunTimeTyped( ClassVectorParameter, IECore.TypeId.ClassVectorParameter )

IECore.ParameterParser.registerType( ClassVectorParameter.staticTypeId(), ClassVectorParameter._parse, ClassVectorParameter._serialise )
