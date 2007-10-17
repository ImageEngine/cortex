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

import StringIO
from IECore import Msg, msg, ClassLoader, formatParameterHelp, WrappedTextFormatter

## This class defines methods for using a ClassLoader object for a 
# predefined set of class contexts, which are prefixes for the full class name. 
# It's intended for loading classes derived from Op, ParameterisedProcedural and similar
# extension classes at specific environments.
class GlobbedClassLoader :

	## Creates a GlobbedClassLoader attached to a ClassLoader and restricted to a list of class contexts.
	def __init__( self, contexts = [ "" ], classLoader = ClassLoader.defaultOpLoader() ) :
	
		self.__contexts = list( contexts )
		self.__classLoader = classLoader
		
	## Returns an alphabetically sorted list
	# of all the classes found
	# on the ClassLoader contexts. The optional matchString
	# is concatenated with the contexts to narrow down
	# the set of names returned.
	def classNames( self, matchString = "*" ) :

		allClasses = []
		for context in self.__contexts:
			classes = self.__classLoader.classNames( "%s%s" % ( context, matchString) )
			allClasses.extend( set( classes ).difference( allClasses ) )

		allClasses.sort()
		return allClasses

	def __longClassName( self, name ):
		classes = self.classNames( name )
		if not len( classes ) :
			IECore.error( "Class \"%s\" does not exist.\n" % opName )
			return False
		elif len( classes )>1 :
			IECore.error( "Class name \"%s\" is ambiguous - could be any of the following : \n\t%s" % ( name, "\n\t".join( classes ) ) )
			return False
		return classes[0]

	## Returns the available versions of the specified
	# class as a list of ints, with the latest version
	# last. If the class doesn't exist returns an empty
	# list.
	def versions( self, name ) :
	
		className = self.__longClassName( name )
		return self.__classLoader.versions( className )
	
	## Loads the specified version of the named class.
	# Version defaults to getDefaultVersion( name ) if
	# not specified. Note that this returns the actual class
	# object itself rather than an instance of that class.
	# It uses ClassLoader.load method.
	def load( self, name, version = None ) :

		className = self.__longClassName( name )
		return self.__classLoader.load( className, version )

	## Loads the specified version of the named class and returns a multiline formated string with user help information.
	def help( self, name, version = None ) :
		
		textIO = StringIO.StringIO()
		myClass = self.load( name, version )()
		formatter = WrappedTextFormatter( textIO )
		formatter.paragraph( "Name    : " + myClass.name + "\n" )
		formatter.paragraph( myClass.description + "\n" )
		formatter.heading( "Parameters" )
		formatter.indent()
		for p in myClass.parameters().values() :
			formatParameterHelp( p, formatter )
		formatter.unindent()
		return textIO.getvalue()
