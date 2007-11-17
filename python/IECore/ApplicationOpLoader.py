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

import re
import os
import subprocess
import tempfile

from IECore import *

## This class loads Ops, and if instructed to by a piece of "application" userData contained within them,
#  wraps them such that they can be executed from within a particular application. 
class ApplicationOpLoader :

	__handlers = {}
	
	# Creates an ApplicationOpLoader attached to a particular class loader
	def __init__( self, classLoader = ClassLoader.defaultOpLoader() ) :
	
		self.__classLoader = classLoader
		
	## Returns an alphabetically sorted list
	# of all the classes found
	# on the ClassLoader contexts. The optional matchString
	# narrows down the set of names returned.
	def classNames( self, matchString = "*" ) :
		
		return self.__classLoader.classNames( matchString )

	## Returns the available versions of the specified
	# class as a list of ints, with the latest version
	# last. If the class doesn't exist returns an empty
	# list.
	def versions( self, name ) :

		return self.__classLoader.versions( name )

	## Loads the specified version of the named class, wrapped if applicable. The wrapper, if present, inherits
	# from and overrides methods in the class given to us by the attached ClassLoader (specified in the constructor).
	# Version defaults to getDefaultVersion( name ) if not specified. Note that this returns the actual class
	# object itself rather than an instance of that class.
	def load( self, name, version = None ) :

		WrappedClass = self.__classLoader.load( name, version )
		
		wrappedClassInstance = WrappedClass()
		
		application = None
		if hasattr( wrappedClassInstance.userData(), "application" ):					
			application = wrappedClassInstance.userData()["application"].value
				
		if application:		
					
			classWrapperName = application + name	
			
			ClassWrapper = type( classWrapperName, (WrappedClass,), {} )
						
			if not application in ApplicationOpLoader.__handlers:
				raise RuntimeError("No ApplicationOpLoader handler found for '%s'" % (application) )
				
			handler = ApplicationOpLoader.__handlers[ application ]
			
			for key, value in handler.items():
				
				setattr( ClassWrapper, key, curry( value, WrappedClass = WrappedClass ) )

			
			return ClassWrapper			
		else:
			return WrappedClass
		
		
	## Registers a new application that this ApplicationOpLoader can handle
	# The application's name should be specified, along with a dictionary of class name overrides. Most
	# applications will want to override at least __init__ and doOperation.
	@staticmethod
	def registerApplication( appName, overrides ):

		ApplicationOpLoader.__handlers[ appName ] = overrides
			


# \todo Move this elsewhere
def registerMaya():

	def init( self, WrappedClass = None ) :
				
		WrappedClass.__init__( self )

		self.parameters().addParameter(
			CompoundParameter(
				name = "maya",
				description = "Parameters specific for execution within Maya",

				members = 
				[
					FileNameParameter(
						name = "sceneFileName",
						description = "Name of scene file to open",

						defaultValue = "/home/mark/maya/projects/default/scenes/particleCacheOp.ma"
					)
				]

			)
		)

	def doOperation( self, operands, WrappedClass = None ):
		
		operandsFile = tempfile.mkstemp( suffix = ".cob" )
		resultsFile = tempfile.mkstemp( suffix = ".cob" )
		
		result = None
		try :
		
			cleanOperands = operands.copy()
			del cleanOperands["maya"]
			writer = Writer.create( cleanOperands, operandsFile[1] )
			writer.write()

			mel =        'loadPlugin "ieCore-%s.so";' % ( os.environ[ "IECORE_MAJOR_VERSION" ] )
			mel = mel + r'eval("iePython -command \"import sys\"");'	
			mel = mel + r'eval("iePython -command \"import IECore\"");'
			mel = mel + r'eval("iePython -command \"import IECoreMaya\"");'	
			mel = mel + r'eval("iePython -command \"r = IECore.Reader.create(\'%s\')\"");' % ( operandsFile[1] )
			mel = mel + r'eval("iePython -command \"operands = r.read()\"");'			
			mel = mel + r'eval("iePython -command \"opLoader = IECore.ClassLoader.defaultOpLoader()\"");'
			mel = mel + r'eval("iePython -command \"op = opLoader.load(\'%s\', %d)()\"");' % ( self.path, self.version )
			mel = mel + r'eval("iePython -command \"op.parameters().setValue( operands)\"");'
			mel = mel + r'eval("iePython -command \"result = op()\"");'
			mel = mel + r'eval("iePython -command \"w = IECore.Writer.create( result, \'%s\')\"");' % ( resultsFile[1] )
			mel = mel + r'eval("iePython -command \"w.write()\"");'						

			mel = mel +  'eval("ieSystemExit 0");'

			exitStatus = subprocess.call( 
				[
					'Render', 
					'-preRender', 
					mel, 
					self.parameters().maya.sceneFileName.getValue().value
				]
			)

			if not exitStatus == 0:	
				raise RuntimeError( "Error %d executing Render" % (exitStatus) )
			
			r = Reader.create( resultsFile[1] )
			result = r.read()
				
		finally:
		
			if os.path.exists( operandsFile[1] ) :
			
				os.remove( operandsFile[1] )
				
			if os.path.exists( resultsFile[1] ) :
			
				os.remove( resultsFile[1] )	
						

		return result
				
	ApplicationOpLoader.registerApplication ( 
		"maya",		
		{
			"__init__" : init,
			"doOperation" : doOperation
		}
	)


registerMaya()
