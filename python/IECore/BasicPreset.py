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
		
		self.__header = None
		self.__data = None
		self.__cob = None
		
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
			
			self.__cob = pathOrDataOrParameterised
		
		elif isinstance( pathOrDataOrParameterised, IECore.CompoundObject ) :
		
			self.__data = pathOrDataOrParameterised	
		
		elif hasattr( pathOrDataOrParameterised, "parameters" ):
			
			data = IECore.CompoundObject()
			
			if rootParameter is None:
				rootParameter = pathOrDataOrParameterised.parameters()
			
			BasicPreset.__grabHierarchy_v1( data, rootParameter, parameters )
			# Remove any branches without leaf values. There should be 
			# a better way of dealing with this.
			BasicPreset.__pruneHierarchy_v1( data )
			
			if referenceData:
				self.__data = data
			else:
				self.__data = data.copy()
								
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
		
		self.__ensureHeader()
		
		h = self.__header
		return {
			"title" : h["title"].value if "title" in h else self.__class__,
			"description" : h["description"].value if "description" in h else "",
			"categories" : list( h["categories"] ) if "categories" in h else (),
		}

	## \see IECore.Preset.applicableTo	
	def applicableTo( self, parameterised, rootParameter ) :
		
		self.__ensureData()	
		return self.__applicableTo_v1( parameterised, rootParameter, self.__data )

	## \see IECore.Preset.__call__
	def __call__( self, parameterised, rootParameter ) :
			
		self.__ensureData()
		
		if not self.applicableTo( parameterised, rootParameter ) :
			raise RuntimeError, "IECore.BasicPreset: Sorry, this preset is not applicable to that parameter."
		
		self.__applyHierarchy_v1( parameterised, rootParameter, self.__data )
											
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
	## \param versino, int, the version of the preset, this will default to 1.
	def save( self, path, name, title="", description="", categories=(), version=1 ) :
	
		if not self.__data:
			raise RuntimeError, "IECore.BasicPreset.save: Unable to save, preset has no data."
		
		baseDir = "%s/%s" % ( path, name )
		
		if not os.path.isdir( baseDir ) :
			os.makedirs( baseDir )
		
		if not os.path.isdir( baseDir ) :
			raise RuntimeError, "IECore.BasicPreset.save: Unable to create the directory '%s'" % baseDir
		
		pyFile = "%s/%s-%i.py" % ( baseDir, name, version )
		cobName = "%s-%i.cob" % ( name, version )
		cobFile = "%s/%s" % ( baseDir, cobName )		
		
		w = IECore.Writer.create( self.__data, cobFile )

		w["header"].getValue()["title"] = IECore.StringData( title if title else name )
		w["header"].getValue()["description"] = IECore.StringData( description )
		w["header"].getValue()["categories"] = IECore.StringVectorData( categories )
		w["header"].getValue()["dataVersion"] = IECore.IntData( 1 )

		w.write()
		
		BasicPreset.__writePy( pyFile, cobName, name )
		
	def __ensureData( self ) :
		
		if self.__data != None:
			return			

		if self.__cob is not None:

			data = IECore.Reader.create( self.__cob ).read()
			if not isinstance( data, IECore.CompoundObject ) :
				raise RuntimeError, "IECore.BasicPreset: Unable to retrieve data from '%s'." % self.__cob 
			self.__data = data
		
		if not self.__data:
		
			raise RuntimeError, "IECore.BasicPreset: No data in preset." 

	def __ensureHeader( self ) :
		
		if self.__cob != None:
			self.__header = IECore.Reader.create( self.__cob ).readHeader()
		else:
			self.__header = {}
				
	@staticmethod
	def __writePy( fileName, cob, className  ) :
				
		f = open( fileName, "w" )
		f.write(
		
"""import IECore.BasicPreset
import os.path

class %s( IECore.BasicPreset ):
	
	def __init__( self ):
		dir = os.path.dirname( __file__ )
		IECore.BasicPreset.__init__( self, dir+"/%s"	)
		
""" % (	className,	cob, )

		)
		
	@staticmethod
	def __grabHierarchy_v1( data, parameter, parameterList=() ) :
				
		if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :

			for p in parameter.keys() :
			
				data[p] = IECore.CompoundObject()

				BasicPreset.__grabHierarchy_v1(
					data[p],
					parameter[p],
					parameterList, 
				 )
				 
		else :
			
			if isinstance( parameter, IECore.ClassParameter ) :
				
				BasicPreset.__grabClassParameter_v1( parameter, data, parameterList )
			
			elif isinstance( parameter, IECore.ClassVectorParameter ) :
				
				BasicPreset.__grabClassVectorParameter_v1( parameter, data, parameterList )
			
			else :	
						
				# Some parameter types end up with different python instance
				# due to a boost bug, so 'if p in parameterList' fails.
				if parameterList:
					for p in parameterList:
						if parameter.isSame( p ) :		
							BasicPreset.__grabParameter_v1( parameter, data )
							break
				else :
					BasicPreset.__grabParameter_v1( parameter, data )
	
	@staticmethod	
	def __grabParameter_v1( parameter, data ) :
		
		data["_value_"] = parameter.getValue()
	
	@staticmethod	
	def __grabClassParameter_v1( parameter, data, parameterList ) :
		
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

			BasicPreset.__grabHierarchy_v1(
				data["_classValue_"],
				c[0].parameters(),
				parameterList, 
			)
	
	@staticmethod		
	def __grabClassVectorParameter_v1( parameter, data, parameterList ) :
		
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

			BasicPreset.__grabHierarchy_v1(
				v,
				c[0].parameters(),
				parameterList, 
			)			

			data["_values_"][c[1]] = v 		
		
	def __applyHierarchy_v1( self, parameterised, parameter, data ) :
		
		if "_className_" in data :
							
			self.__applyClassParameter_v1( parameterised, parameter, data )
			
		elif "_classNames_" in data :
		
			self.__applyClassVector_v1( parameterised, parameter, data )
		
		elif "_value_" in data :
				
			self.__applyParameter_v1( parameterised, parameter, data )
			
		else : # CompoundParameter
					
			for p in data.keys() :
			
				if p not in parameter :
					print "'%s' is missing from '%s' (%s)" % ( p, parameter.name, parameter )
				
				self.__applyHierarchy_v1( parameterised, parameter[p], data[p] )
			
	def __applyParameter_v1( self, parameterised, parameter, data ) :
		
		try:
			parameter.setValue( data["_value_"] )
		except Exception, e:
			print e
		
	def __applyClassParameter_v1( self, parameterised, parameter, data ) :
		
		if not isinstance( parameter, IECore.ClassParameter ) :
			print "Unable to restore to '%s' (%s) as it isnt a ClassParameter" \
						% ( parameter.name, parameter )
			return
		
		c = parameter.getClass( True )
		
		className = data["_className_"].value
		classVersion = data["_classVersion_"].value
		classPaths = data["_classSearchPaths_"].value
					
		if c[1] != className or c[2] != classVersion or c[3] != classPaths:
			parameter.setClass( className, classVersion, classPaths )
			
		c = parameter.getClass( False )
		if c:	
			self.__applyHierarchy_v1( parameterised, c.parameters(), data["_classValue_"] )		
		
	def __applyClassVector_v1( self, parameterised, parameter, data ) :
		
		if not isinstance( parameter, IECore.ClassVectorParameter ) :
			print "Unable to restore to '%s' (%s) as it isnt a ClassVectorParameter" \
					% ( parameter.name, parameter )
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

				c = self.__addClassToVector_v1(
					parameter, 
					paramNames[i],
					names[i],
					versions[i],
				)
				
				
			self.__applyHierarchy_v1(
				parameterised,
				c[0].parameters(),
				data["_values_"][ paramNames[i] ],
			)
		
	def __addClassToVector_v1( self, parameter, parameterName, className, classVersion ) :
				
		classes = parameter.getClasses( True )
		parameterNames = [ c[1] for c in classes ]
			
		## \todo Currently this is in ClassVectorParameterUI, should there
		## be some default name mechanism outside of the UI code for continuity?
		if parameterName in parameterNames:
			for i in range( 0, len( classes ) + 1 ) :
				parameterName = "p%d" % i
				if parameterName not in parameterNames :
					break
		
		parameter.setClass( parameterName, className, classVersion )
		return parameter.getClass( parameterName, True )

	def __applicableTo_v1( self, parameterised, parameter, data ) :
				
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
	def __pruneHierarchy_v1( data ) :
	
		returnVal = True
	
		for k in data.keys() :
		
			if k == "_value_" : 
			
				returnVal = False
				
			elif isinstance( data[k], IECore.CompoundObject ):
			
				if BasicPreset.__pruneHierarchy_v1( data[k] ) :
					del data[k]
				else :
					returnVal = False			
		
		return returnVal		

IECore.registerRunTimeTyped( BasicPreset )
	
