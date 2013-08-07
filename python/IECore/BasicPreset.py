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

import os
import re

## Implements a Preset to permit values to be saved and restored 
## from a Parameterised object. BasicPresets can be created either
## as in-memory representations of the parameters, or saved to disk.
##
## The BasicPreset has a single parameter
##   "overwriteMatchingComponents" : Bool, which controls how 
##      ClassVector parameter are treated, when the preset is applied.
class BasicPreset( IECore.Preset ) :

	## The constructor is essentially in two forms:
	## 
	##    IECore.BasicPreset( parameterised, rootParameter=None, parameters=(), referenceData=False )
	##
	## This is the most common form, and should be used to create a new preset from the
	## given parameterised holding object.
	##
	##    IECore.BasicPreset( pathOrData )
	##  
	## This form is used to restore data into a preset for application, and should rarely
	## be used directly.
	##
	## \param pathOrData, this should be an absolute path to a CompoundObject on disk or a 
	## CompoundObject pointer itself. This object should contain the data structure for the preset.
	## \param parameterised, The Parameterised object holding the parameters to be saved.
	## \param rootParameter, IECore.Parameter, Where to start in the parmameter hierarchy. 
	## \param parameters, ( IECore.Parameter, ... ), A list of Parameters to include in the
	##              the preset. This allow certain values not to be included in the preset.
	## \param referenceData, bool, When enabled, this stops the preset mechanism from
	##              copying the value data from the parameters it encapsulates. This can save memory
	##              when the preset is to be written straight to disk. The default behaviour
	##				copies any parameter values so the preset is not dependent on the source
	##				parameters state at the time of application.
	def __init__( self, pathOrDataOrParameterised, rootParameter=None, parameters=(), referenceData=False ) :
		
		self._header = None
		self._data = None
		self._cob = None
		
		IECore.Preset.__init__( self )
				
		self.parameters().addParameters(
		
			[
				IECore.BoolParameter(
					name = "overwriteMatchingComponents",
					description = "When off, the preset will always append items to a " + \
					"ClassVectorParameter, otherwise, it will replace the existing " + \
					"classes with the same names, if they don't match the preset. " + \
					"This does not affect and parameter values, these are always set " + \
					"to match the preset.",
					defaultValue = False
				),
			]
		)
		
		if isinstance( pathOrDataOrParameterised, str ) or isinstance( pathOrDataOrParameterised, unicode ) :
			
			self._cob = pathOrDataOrParameterised
		
		elif isinstance( pathOrDataOrParameterised, IECore.CompoundObject ) :
		
			self._data = pathOrDataOrParameterised	
		
		elif hasattr( pathOrDataOrParameterised, "parameters" ):
			
			data = IECore.CompoundObject()
			
			if rootParameter is None:
				rootParameter = pathOrDataOrParameterised.parameters()
			
			BasicPreset._grabHierarchy( data, rootParameter, parameters )
			
			# We need to prune any class entries without parameters, so that
			# we don't meddle with classes the user asked us not to copy parameters for.
			BasicPreset._pruneHierarchy( data )
			
			if referenceData:
				self._data = data
			else:
				self._data = data.copy()
								
		else :		
			
			raise ValueError, "IECore.BasicPreset.__init__: Unsupported object passed: %s." % pathOrDataOrParameterised
		
	## \return a dictionary of metatdata about the preset. BasicPresets 
	## provide the following keys, when a preset has been saved to disk.
	## NOTE: Presets created by the 'Copy' method will not contain any
	## pertinent information in theses fields:
	##
	##   "title" : string, The user supplied name the preset.
	##   "description" : string, A multi-line string of arbitrary descriptive text.
	##   "categories" : ( string, .. ), A list of strings, one for each category
	##                  the preset is considered part of.
	def metadata( self ) :
		
		self._ensureHeader()
		
		h = self._header
		return {
			"title" : h["title"].value if "title" in h else self.__class__,
			"description" : h["description"].value if "description" in h else "",
			"categories" : list( h["categories"] ) if "categories" in h else (),
		}

	## \see IECore.Preset.applicableTo	
	def applicableTo( self, parameterised, rootParameter ) :
		
		self._ensureData()	
		return self._applicableTo( parameterised, rootParameter, self._data )

	## \see IECore.Preset.__call__
	# \param parameterList A list of Parameter pointers that the preset should apply to. 
	# \param parameterListExcludes A bool, which when True, will treat the parameterList as a
	# 'skip' list, rather than an 'application' list.
	# NOTE: When parameterListExcludes is False, all parent parameters of a desired leaf parameter 
	# must be in this list. Otherwise the preset will not consider the parent so will never
	# reach the child.
	def __call__( self, parameterised, rootParameter, parameterList=[], parameterListExcludes=False ) :
			
		self._ensureData()
		
		if not self.applicableTo( parameterised, rootParameter ) :
			raise RuntimeError, "IECore.BasicPreset: Sorry, this preset is not applicable to that parameter."
		
		if parameterList and not parameterListExcludes :
			# Not much point getting out of bed if the root isn't in there...
			if rootParameter not in parameterList:
				# Copy the list so we don't modify the one we were given.
				parameterList = parameterList[:]
				parameterList.append( rootParameter )
						
		self._applyHierarchy( parameterised, rootParameter, self._data, parameterList, parameterListExcludes )
											
	## This method will save the specified parameters to disk in such a was
	## as can be loaded by the IECore.ClassLoader
	## \param path, string, The file system location the preset should be saved to
	##              note: this should be a directory name, not the desired preset name.
	## \param name, string, The name of the preset, the preset will be saved under this
	##              name inside of 'path'. This name is not sanitised, and it is the 
	##              responsibility of the caller to ensure that it is a legal file system name.
	## \param title, string, The title of the preset, no character restrictions.
	## \param description, string, A description of the preset, no character restrictions.
	## \param categories, ( string, ... ) A list of categories the preset should be tagged with
	## \param version, int, the version of the preset, this will default to 1, used when saving
	## for the ClassLoader.
	## \param classLoadable, bool, if True (default) then the preset will be saved in a way that
	## can be loaded by the ClassLoader, otherwise, just a cob file is written containing the
	## presets data.
	def save( self, path, name, title="", description="", categories=(), version=1, classLoadable=True ) :
	
		if not self._data:
			raise RuntimeError, "IECore.BasicPreset.save: Unable to save, preset has no data."
	
		baseDir = path
		cobName = "%s.cob" % ( name, )
		pyFile = None
			
		if classLoadable :
			baseDir = "%s/%s" % ( path, name )
			cobName = "%s-%i.cob" % ( name, version )
			pyFile = "%s/%s-%i.py" % ( baseDir, name, version )
		
		cobFile = "%s/%s" % ( baseDir, cobName )	
			
		if not os.path.isdir( baseDir ) :
			os.makedirs( baseDir )
		
		if not os.path.isdir( baseDir ) :
			raise RuntimeError, "IECore.BasicPreset.save: Unable to create the directory '%s'" % baseDir
		
		w = IECore.Writer.create( self._data, cobFile )

		w["header"].getValue()["title"] = IECore.StringData( title if title else name )
		w["header"].getValue()["description"] = IECore.StringData( description )
		w["header"].getValue()["categories"] = IECore.StringVectorData( categories )
		w["header"].getValue()["dataVersion"] = IECore.IntData( 1 )

		w.write()
		
		if pyFile :	
			BasicPreset._writePy( pyFile, cobName, name )		
	
	def _ensureData( self ) :
		
		if self._data != None:
			return			

		if self._cob is not None:

			data = IECore.Reader.create( self._cob ).read()
			if not isinstance( data, IECore.CompoundObject ) :
				raise RuntimeError, "IECore.BasicPreset: Unable to retrieve data from '%s'." % self._cob 
			self._data = data
		
		if not self._data:
		
			raise RuntimeError, "IECore.BasicPreset: No data in preset." 

	def _ensureHeader( self ) :
		
		if self._cob != None:
			self._header = IECore.Reader.create( self._cob ).readHeader()
		else:
			self._header = {}
				
	@staticmethod
	def _writePy( fileName, cob, className  ) :
				
		f = open( fileName, "w" )
		f.write(
		
"""import IECore
import os.path

class %s( IECore.BasicPreset ):
	
	def __init__( self ):
		dir = os.path.dirname( __file__ )
		IECore.BasicPreset.__init__( self, dir+"/%s"	)

IECore.registerRunTimeTyped( %s )
		
""" % (	className, cob, className )

		)
		
	@staticmethod
	def _grabHierarchy( data, parameter, parameterList=() ) :
		
		if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :

			for p in parameter.keys() :
			
				data[p] = IECore.CompoundObject()

				BasicPreset._grabHierarchy(
					data[p],
					parameter[p],
					parameterList, 
				 )
				 
		else :
			
			if isinstance( parameter, IECore.ClassParameter ) :
				
				BasicPreset._grabClassParameter( parameter, data, parameterList )
			
			elif isinstance( parameter, IECore.ClassVectorParameter ) :
				
				BasicPreset._grabClassVectorParameter( parameter, data, parameterList )
			
			elif isinstance( parameter, IECore.CompoundParameter ) :
			
				# for classes derived from IECore.CompoundParameter:
				for p in parameter.keys() :

					data[p] = IECore.CompoundObject()

					BasicPreset._grabHierarchy(
						data[p],
						parameter[p],
						parameterList, 
					 )
				
			else :	
						
				# Some parameter types end up with different python instance
				# due to a boost bug, so 'if p in parameterList' fails.
				if parameterList:
					for p in parameterList:
						if parameter.isSame( p ) :		
							BasicPreset._grabParameter( parameter, data )
							break
				else :
					BasicPreset._grabParameter( parameter, data )
	
	@staticmethod	
	def _grabParameter( parameter, data ) :
		
		data["_value_"] = parameter.getValue()
	
	@staticmethod	
	def _grabClassParameter( parameter, data, parameterList ) :
		
		c = parameter.getClass( True )
		
		data["_className_"] = IECore.StringData( c[1] )
		data["_classVersion_"] = IECore.IntData( c[2] )
		data["_classSearchPaths_"] = IECore.StringData( c[3] )

		classNameFilter = "*"
		try :
			classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		data["_classNameFilter_"] = IECore.StringData( classNameFilter )

		data["_classValue_"] = IECore.CompoundObject()

		if c[0] :
			
			# Some classes may have no parameters, if they have been
			# specifically included in the parameter list, then we
			# want to save their instance specification anyway.
			if len( c[0].parameters() ) :

				BasicPreset._grabHierarchy(
					data["_classValue_"],
					c[0].parameters(),
					parameterList, 
				)
			
			elif parameterList :
		
				for p in parameterList:
					if parameter.isSame( p ) :			
						data["_noPrune_"] = IECore.BoolData( True )
				
			else :
			
				data["_noPrune_"] = IECore.BoolData( True )
			
	
	@staticmethod		
	def _grabClassVectorParameter( parameter, data, parameterList ) :
		
		classes = parameter.getClasses( True )
				
		data["_classSearchPaths_"] = IECore.StringData( parameter.searchPathEnvVar() )

		classNameFilter = "*"
		try :
			classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		data["_classNameFilter_" ] = IECore.StringData( classNameFilter )

		data["_classNames_"] = IECore.StringVectorData()
		data["_classVersions_"] = IECore.IntVectorData()
		data["_classOrder_"] = IECore.StringVectorData()

		data["_values_"] = IECore.CompoundObject()

		for c in classes:

			data["_classOrder_"].append( c[1] )
			data["_classNames_"].append( c[2] )
			data["_classVersions_"].append( c[3] )

			v = IECore.CompoundObject()

			BasicPreset._grabHierarchy(
				v,
				c[0].parameters(),
				parameterList, 
			)			

			data["_values_"][c[1]] = v 		
		
	def _applyHierarchy( self, parameterised, parameter, data, parameterList=[], invertList=False ) :
				
		if parameterList :
			if invertList : # its a 'skipList'
				if parameter in parameterList :
					return
			else :
				if parameter not in parameterList :
					return
		
		if "_className_" in data :
							
			self._applyClassParameter( parameterised, parameter, data, parameterList, invertList )
			
		elif "_classNames_" in data :
		
			self._applyClassVector( parameterised, parameter, data, parameterList, invertList )
		
		elif "_value_" in data :
				
			self._applyParameter( parameterised, parameter, data )
			
		else : # CompoundParameter
					
			for p in data.keys() :
			
				if p not in parameter :
					IECore.msg( 
						IECore.Msg.Level.Warning, 
						"IECore.BasicPreset", 
						"'%s' is missing from '%s' (%s)" % ( p, parameter.name, parameter ) 
					)
					continue
				
				self._applyHierarchy( parameterised, parameter[p], data[p], parameterList, invertList )
			
	def _applyParameter( self, parameterised, parameter, data ) :
		
		try:
			parameter.setValue( data["_value_"] )
		except Exception, e:
			IECore.msg( IECore.Msg.Level.Warning, "IECore.BasicPreset", str(e) )
		
	def _applyClassParameter( self, parameterised, parameter, data, parameterList=[], invertList=False ) :
		
		if not isinstance( parameter, IECore.ClassParameter ) :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.BasicPreset", 
				"Unable to restore to '%s' (%s) as it isnt a ClassParameter"
					% ( parameter.name, parameter )
			)
			return
		
		c = parameter.getClass( True )
		
		className = data["_className_"].value
		classVersion = data["_classVersion_"].value
		classPaths = data["_classSearchPaths_"].value
					
		if c[1] != className or c[2] != classVersion or c[3] != classPaths:
			parameter.setClass( className, classVersion, classPaths )
			
		c = parameter.getClass( False )
		if c and "_classValue_" in data :	
			self._applyHierarchy( parameterised, c.parameters(), data["_classValue_"], parameterList, invertList )		
		
	def _applyClassVector( self, parameterised, parameter, data, parameterList=[], invertList=False ) :
		
		if not isinstance( parameter, IECore.ClassVectorParameter ) :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.BasicPreset",
				"Unable to restore to '%s' (%s) as it isnt a ClassVectorParameter"
					% ( parameter.name, parameter )
			)
			return
		
		overwrite = self.parameters()["overwriteMatchingComponents"].getTypedValue()
				
		names = data["_classNames_"]
		versions = data["_classVersions_"]
		paramNames = data["_classOrder_"]
		
		for i in range( len( data["_classNames_"] ) ) :
	
			# We still have the class information, even if
			# there were no parameter values saved.
			if paramNames[i] not in data["_values_"] :
				continue
		
			if overwrite:
				
				c = None
				
				if paramNames[i] in parameter:			
					c = parameter.getClass( paramNames[i], True )
				
				if not c or c[1:] != ( paramNames[i], names[i], versions[i] ) :
					parameter.setClass( paramNames[i], names[i], versions[i] )
				
				c = parameter.getClass( paramNames[i], True )
			
			else:

				c = self._addClassToVector(
					parameter, 
					paramNames[i],
					names[i],
					versions[i],
				)
				
				
			self._applyHierarchy(
				parameterised,
				c[0].parameters(),
				data["_values_"][ paramNames[i] ],
				parameterList,
				invertList
			)
		
	def _addClassToVector( self, parameter, parameterName, className, classVersion ) :
			
		classes = parameter.getClasses( True )
		parameterNames = [ c[1] for c in classes ]
		if parameterName in parameterNames:
			parameterName = parameter.newParameterName()
		
		parameter.setClass( parameterName, className, classVersion )
		return parameter.getClass( parameterName, True )

	def _applicableTo( self, parameterised, parameter, data ) :
				
		if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :
			
			if "_classValue_" in data or "_values_" in data:
				return False
			
			for k in data.keys():
				if k not in parameter:
					return False
			
		elif isinstance( parameter, IECore.ClassParameter ) :
			
			if "_className_" not in data:
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False
								
		elif isinstance( parameter, IECore.ClassVectorParameter ) :
			
			if "_classNames_" not in data:
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False
				
			if data["_classSearchPaths_"].value != parameter.searchPathEnvVar() :
				return False
			
		else :
			
			if "_value_" not in data:
				return False
								
			if not parameter.valueValid( data["_value_"] )[0]:
				return False		
		
		return True
		
	@staticmethod
	def _pruneHierarchy( data ) :
	
		returnVal = True
	
		for k in data.keys() :
		
			if k == "_value_" or k == "_noPrune_": 
			
				returnVal = False
				
			elif isinstance( data[k], IECore.CompoundObject ):
			
				if BasicPreset._pruneHierarchy( data[k] ) :
					del data[k]
				else :
					returnVal = False			
		
		return returnVal		

IECore.registerRunTimeTyped( BasicPreset )
	
