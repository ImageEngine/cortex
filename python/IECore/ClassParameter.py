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

import IECore

## The ClassParameter is a specialised CompoundParameter which allows its
# children to be specified by another Parameterised class which is loaded
# using the ClassLoader. This allows one class to easily nest another while
# exposing the other's parameters publicly.
class ClassParameter( IECore.CompoundParameter ) :

	def __init__( self, name, description, searchPathEnvVar, className="", classVersion=0, userData=None ) :

		IECore.CompoundParameter.__init__( self, name, description, userData=userData )

		self.__classInstance = None
		self.__className = ""
		self.__classVersion = 0
		self.__searchPathEnvVar = searchPathEnvVar

		self.setClass( className, classVersion, searchPathEnvVar )

	## Return the class being held. If withClassLoaderArgs is True then a tuple is returned
	# in the following form : ( class, className, classVersion, searchPathEnvVar ).
	def getClass( self, withClassLoaderArgs=False ) :

		if withClassLoaderArgs :
			return ( self.__classInstance, self.__className, self.__classVersion, self.__searchPathEnvVar )
		else :
			return self.__classInstance

	## Sets the class being held. The specified class is loaded using a ClassLoader and
	# the class' parameters are added to this parameter as children.
	def setClass( self, className, classVersion, searchPathEnvVar=None ) :

		searchPathToUse = searchPathEnvVar if searchPathEnvVar is not None else self.__searchPathEnvVar

		if ( className, classVersion, searchPathToUse ) == ( self.__className, self.__classVersion, self.__searchPathEnvVar ) :
			return

		self.__classInstance = None
		self.clearParameters()

		if className!="" :

			loader = IECore.ClassLoader.defaultLoader( searchPathToUse )

			self.__classInstance = loader.load( className, classVersion )()

			self.addParameters(
				self.__classInstance.parameters().values()
			)

		self.__className = className
		self.__classVersion = classVersion
		self.__searchPathEnvVar = searchPathToUse

	@staticmethod
	def _serialise( parameter, value ) :

		return [

			parameter.__className,
			str( parameter.__classVersion ),
			parameter.__searchPathEnvVar,

		]

	@staticmethod
	def _parse( args, parameter ) :

		parameter.setClass( args[0], int( args[1] ), args[2] )
		del args[0:3]

IECore.registerRunTimeTyped( ClassParameter, IECore.TypeId.ClassParameter )

IECore.ParameterParser.registerType( ClassParameter.staticTypeId(), ClassParameter._parse, ClassParameter._serialise )
