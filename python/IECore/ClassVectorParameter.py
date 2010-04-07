##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

## The ClassVectorParameter is similar to the ClassParameter but instead of holding
# a single class it holds many classes, each storing its parameters under a named
# child of the ClassVectorParameter.
class ClassVectorParameter( IECore.CompoundParameter ) :

	def __init__( self, name, description, searchPathEnvVar, classes=[], userData=None ) :
	
		IECore.CompoundParameter.__init__( self, name, description, userData=userData )
		
		self.__searchPathEnvVar = searchPathEnvVar
		self.__namesToInstances = {} # maps parameter names to [ classInstance, className, classVersion ] lists
		
		self.setClasses( classes )
	
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
	
		assert( isinstance( classes, list ) )
		for c in classes :
			assert( isinstance( c, tuple ) )
			assert( len( c ) == 3 )
			assert( isinstance( c[0], str ) )
			assert( isinstance( c[1], str ) )
			assert( isinstance( c[2], int ) )
			
		# first remove any existing parameters which we don't need
		
		neededNames = set( [ x[0] for x in classes ] )
		for parameter in self.values() :
			if parameter.name not in neededNames :
				self.removeParameter( parameter )
				del self.__namesToInstances[parameter.name]
		
		# and then create any new ones we need and reload and reorder existing
		# ones as necessary
		
		loader = IECore.ClassLoader.defaultLoader( self.__searchPathEnvVar )
		for i in range( 0, len( classes ) ) :
		
			requestedParameterName = classes[i][0]
			requestedClassName = classes[i][1]
			requestedClassVersion = classes[i][2]
		
			parameter = self.parameter( requestedParameterName )
			if not parameter :
				parameter = IECore.CompoundParameter( requestedParameterName, "" )
			
			instance = self.__namesToInstances.setdefault( requestedParameterName, [ None, "", 0 ] )
			
			if [ requestedClassName, requestedClassVersion ] != instance[1:] :
						
				instance[0] = loader.load( requestedClassName, requestedClassVersion )()
				instance[1] = requestedClassName
				instance[2] = requestedClassVersion
				
				parameter.clearParameters()
				parameter.addParameters( instance[0].parameters().values() )
			
			# make sure the parameter has the right order within the whole
			
			if len( self ) == i :	
				self.addParameter( parameter )
			else :
				keys = self.keys()
				currentIndex = -1
				try :
					currentIndex = keys.index( requestedParameterName )
				except :
					pass
				if currentIndex != i :
					if currentIndex!=-1 :
						self.removeParameter( parameter )
					self.insertParameter( parameter, self[keys[i]] )

	@staticmethod
	def _serialise( parameter ) :

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
