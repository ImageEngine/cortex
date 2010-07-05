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

import os, warnings
from IECore import warning, msg, Msg, error, Writer, CompoundParameter, ClassLoader, SearchPath

## This class manages loading and saving named preset values for Parameterised objects.
# It uses a given SearchPath to locate the presets. The presets must be classes which the
# __call__ method is implemented and the only parameter passed is the Parameterised object to be modified.
class PresetManager :

	## Creates a PresetManager which will load parameter presets using the given classLoader.
	# \param useClassPath If true, then it uses the 'path' member added to the Parameterised objects 
	# when loaded by ClassLoader. Otherwise it will only use the name of the Parameterised object.
	def __init__( self, searchPaths, useClassPath = True ) :
	
		warnings.warn( "The PresetManager class is depreciated, please use the Preset classes instead.", DeprecationWarning, 2 )
	
		self.__useClassPath = useClassPath
		self.__searchPaths = searchPaths
		self.__classLoader = ClassLoader( searchPaths )

	## Returns a copy of the SearchPath object given on the constructor.
	def searchPaths( self ):
		return SearchPath( ":".join( self.__searchPaths.paths ), ":" )

	## Refreshes the internal cache for the preset loader.
	def refresh( self ) :
		self.__classLoader.refresh()

	## Returns the preset names available for a given parameterised object.
	def presets( self, parameterised ):
		presets = self.__classLoader.classNames( self.__presetName( parameterised, "*" ) )
		return [ p.split("/")[-1] for p in presets ]

	## Converts from user preset name to class name
	def __presetName( self, parameterised, name ):
		if self.__useClassPath :
			return "%s/%s" % ( parameterised.path, name )
		else:
			return "%s/%s" % ( parameterised.typeName(), name )

	# This function uses the class name and version from the given parameterised object
	# to find the correspondent presetName and update the parameterised parameters that match.
	# It assumes the given parameterised object was loaded by ClassLoader and so, have the attributes 'path' and 'version'.
	def loadPreset( self, parameterised, presetName ) :
		preset = self.__classLoader.load( self.__presetName( parameterised, str(presetName) ) )()		
		preset( parameterised )

	# Saves the parameter values for the given Parameterised object only for the given Parameter list as a named preset.
	# The path should point to a valid location ( used by the internal SearchPath )
	def savePreset( self, parameterised, parameterList, path, presetName ) :

		# in order to match ClassLoader convention, the final path contains the presetVersion (constant) and has .py extension.
		presetPath = os.path.join( path, self.__presetName( parameterised, presetName ) )
		presetFile = os.path.join( presetPath, presetName + "-1.py" )

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


	__defaultManagers = {}

	## Returns a PresetManager configured to load from the paths defined by the
	# specified environment variable. The same object is returned each time,
	# allowing one manager to be shared by many callers.
	# It uses ClassLoader.defaultLoader() method.
	@classmethod
	def defaultManager( cls, envVar ) :

		mgr = cls.__defaultManagers.get( envVar, None )
		if mgr :
			return mgr

		sp = ""
		if envVar in os.environ :
			sp = os.environ[envVar]
		else :
			msg( Msg.Level.Warning, "PresetManager.defaultLoader", "Environment variable %s not set." % envVar )

		manager = cls( SearchPath( os.path.expandvars( sp ), ":" ) )
		cls.__defaultManagers[envVar] = manager

		return manager
