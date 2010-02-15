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

import os
from IECore import warning, error, Writer, CompoundParameter

## This class manages loading and saving named preset values for Parameterised objects.
# It uses a given class loader to locate the presets. The presets must be classes which the
# __call__ method is implemented and the only parameter passed is the Parameterised object to be modified.
class PresetManager :

	## Creates a PresetManager which will load parameter presets using the given classLoader.
	# \param useClassPath If true, then it uses the 'path' member added to the Parameterised objects 
	# when loaded by ClassLoader. Otherwise it will only use the name of the Parameterised object.
	def __init__( self, classLoader, useClassPath = True ) :
		self.__useClassPath = useClassPath
		self.__classLoader = classLoader

	## Returns the classLoader object used internally.
	def classLoader( self ):
		return self.__classLoader

	## Returns the preset names available for a given parameterised object.
	def presets( self, parameterised ):
		return self.__classLoader.classNames( self.__presetName( parameterised, "*" ) )

	## Converts from user preset name to class name
	def __presetName( self, parameterised, name ):
		if self.__useClassPath :
			return "%s/%s" % ( parameterised.path, name )
		else:
			return "%s/%s" % ( parameterised.name, name )

	# This function uses the class name and version from the given parameterised object
	# to find the correspondent presetName and update the parameterised parameters that match.
	# It assumes the given parameterised object was loaded by ClassLoader and so, have the attributes 'path' and 'version'.
	def loadPreset( self, parameterised, presetName ) :

		preset = self.__classLoader.load( self.__presetName( parameterised, presetName ) )()
		preset( parameterised )

	# Saves the parameter values for the given Parameterised object only for the given Parameter list as a named preset.
	# The path should point to a valid location ( used by SearchPaths on the ClassLoader )
	def savePreset( self, parameterised, parameterList, path, presetName ) :

		# in order to match ClassLoader convention, the final path contains the presetVersion (constant) and has .py extension.
		presetVersion = "1"
		presetPath = os.path.join( path, self.__presetName( parameterised, presetName ), presetVersion )
		presetFile = os.path.join( presetPath, presetName + ".py" )

		tmpParameterList = list( parameterList )
		parameterPaths = {}

		def saveParam( param, curParamPath ):
			myPath = list(curParamPath)
			myPath.append( param.name )
			# ignore root CompoundParameter name
			del( myPath[0] )
			parameterPaths[ param ] = myPath
			Writer.create( param.getValue(), os.path.join( presetPath, presetName + "." + ".".join( myPath ) + ".cob" ) )()

		def findParamIndex( param, paramList ):
			index = 0
			for p in paramList:
				if p.isSame( param ):
					return index
				index += 1
			return -1

		def recursiveSave( compoundParam, curParamPath ):

			paramIndex = findParamIndex( compoundParam, tmpParameterList )
			if paramIndex != -1:
				saveParam( compoundParam, curParamPath )
				del( tmpParameterList[ paramIndex ] )
				return
			
			newParamPath = list( curParamPath )
			newParamPath.append( compoundParam.name )

			for param in compoundParam.values():

				if isinstance( param, CompoundParameter ):
					recursiveSave( param, newParamPath )
				else:
					paramIndex = findParamIndex( param, tmpParameterList )
					if param in tmpParameterList :
						saveParam( param, newParamPath )
						del( tmpParameterList[ paramIndex ] )

		# make sure the preset dir exists
		if not os.path.exists( presetPath ):
			os.makedirs( presetPath )

		# save COB files
		recursiveSave( parameterised.parameters(), [] )

		# save the preset script.
		presetFile = open( presetFile, "w" )
		presetFile.write( """
import IECore
class %s:
	def __call__( self, parameterised ):
""" % presetName
		)
		parameterPathNames = parameterPaths.keys()
		for p in parameterList :
			# find same parameter in dictionary using isSame method...
			paramPath = parameterPaths[ parameterPathNames[findParamIndex(p, parameterPathNames)] ]
			presetFile.write( "\t\tIECore.PresetManager.applyParameterValue( parameterised, %s, IECore.Reader.create( '%s' )() )\n" % 
				( paramPath, os.path.join( presetPath, presetName + "." + ".".join( paramPath ) + ".cob" ) ) )

		presetFile.close()

	# Utility function that is used by standard preset scripts written by this same very PresetManager.
	@staticmethod
	def applyParameterValue( parameterised, parameterPath, parameterValue ):

		param = parameterised.parameters()
		for paramName in parameterPath :
			if not paramName in param:
				warning( "Ignoring parameter %s from preset." % ( ".".join( parameterPath ) ) )
				continue
			param = param[ paramName ]

		try:
			param.setValue( parameterValue )
		except Exception, e:
			warning( "Could not set preset value for parameter %s: %s" % ( ".".join( parameterPath ), str(e) ) )
