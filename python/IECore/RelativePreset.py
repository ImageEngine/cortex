##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

## Implements a Preset that represents changes between two Parameter objects.
##
class RelativePreset( IECore.Preset ) :

	## \param currParameter, IECore.Parameter, represents the parameter state after all changes have been made. 
	## \param oldParameter, IECore.Parameter, represents the parameter state before any changes. 
	## \param referenceData, bool, When enabled, this stops the preset mechanism from
	##              copying the value data from the parameters it encapsulates. This can save memory
	##              when the preset is to be written straight to disk. The default behaviour
	##				copies any parameter values so the preset is not dependent on the source
	##				parameters state at the time of application.
	def __init__( self, currParameter=None, oldParameter=None, referenceData=False ) :
		
		IECore.Preset.__init__( self )

		self.__data = IECore.CompoundObject()

		if not isinstance( currParameter, IECore.Parameter ) :
			raise TypeError, "Parameter currParameter must be a IECore.Parameter object!"

		if not oldParameter is None :

			if not isinstance( oldParameter, IECore.Parameter ) :
				raise TypeError, "Parameter oldParameter must be a IECore.Parameter object!"

			if currParameter.typeId() != oldParameter.typeId() :
				raise TypeError, "Mismatching types for currParameter and oldParameter!"

		RelativePreset.__grabParameterChanges( currParameter, oldParameter, self.__data )
			
		if not referenceData:
			self.__data = self.__data.copy()
	
	## \see IECore.Preset.applicableTo	
	def applicableTo( self, parameterised, rootParameter ) :
		
		return RelativePreset.__applicableTo( rootParameter, self.__data )

	## \see IECore.Preset.__call__
	def __call__( self, parameterised, rootParameter ) :

		if not self.applicableTo( parameterised, rootParameter ) :
			raise RuntimeError, "Sorry, this preset is not applicable to the given parameter."
		
		self.__applyParameterChanges( rootParameter, self.__data )

	@staticmethod
	def __grabParameterChanges( currParameter, oldParameter, data, paramPath = "" ) :

		if oldParameter :

			if currParameter.staticTypeId() != oldParameter.staticTypeId() :
				raise Exception, "Incompatible parameter %s!" % paramPath

		if isinstance( currParameter, IECore.ClassParameter ) :

			RelativePreset.__grabClassParameterChanges( currParameter, oldParameter, data, paramPath )
			
		elif isinstance( currParameter, IECore.ClassVectorParameter ) :
			
			RelativePreset.__grabClassVectorParameterChanges( currParameter, oldParameter, data, paramPath )
			
		elif isinstance( currParameter, IECore.CompoundParameter ) :
			
			RelativePreset.__grabCompoundParameterChanges( currParameter, oldParameter, data, paramPath )

		else :

			RelativePreset.__grabSimpleParameterChanges( currParameter, oldParameter, data, paramPath )

	@staticmethod
	def __grabCompoundParameterChanges( currParameter, oldParameter, data, paramPath ) :

		for p in currParameter.keys() :
			
			newData = IECore.CompoundObject()
			childOldParam = None
			if oldParameter :
				if p in oldParameter.keys() :
					childOldParam = oldParameter[p]

			RelativePreset.__grabParameterChanges(
				currParameter[p],
				childOldParam, 
				newData,
				paramPath + "." + p
			)

			if len(newData) :
				data[p] = newData

		if len(data):
			data["_type_"] = IECore.StringData( "CompoundParameter" )

	@staticmethod
	def __grabSimpleParameterChanges( currParameter, oldParameter, data, paramPath ) :

		if oldParameter :

			if currParameter.getValue() == oldParameter.getValue() :
				return

		data["_type_"] = IECore.StringData( currParameter.typeName() )
		data["_value_"] = currParameter.getValue()
	
	@staticmethod
	def __grabClassParameterChanges( currParameter, oldParameter, data, paramPath ) :
		
		c = currParameter.getClass( True )
		
		className = IECore.StringData( c[1] )
		classVersion = IECore.IntData( c[2] )
		classSearchPaths = IECore.StringData( c[3] )
		classNameFilter = "*"
		try :
			classNameFilter = currParameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		classNameFilter = IECore.StringData( classNameFilter )

		oldClassName = None
		oldClassVersion = None
		childOldParam = None
		if oldParameter :
			oldClass = oldParameter.getClass( True )
			oldClassName = oldClass[1]
			oldClassVersion = oldClass[2]
			childOldParam = oldClass[0].parameters()

		classValue = IECore.CompoundObject()

		if c[0] :
	
			RelativePreset.__grabParameterChanges(
				c[0].parameters(),
				childOldParam,
				classValue,
				paramPath
			)

		if len(classValue):
			data["_classValue_"] = classValue

		if len(data) or className != oldClassName or classVersion != oldClassVersion :
			data["_className_"] = className
			data["_classVersion_"] = classVersion
			data["_classNameFilter_"] = classNameFilter
			data["_type_"] = IECore.StringData( "ClassParameter" )
	
	@staticmethod		
	def __grabClassVectorParameterChanges( currParameter, oldParameter, data, paramPath ) :
		
		classes = currParameter.getClasses( True )
				
		classSearchPaths = IECore.StringData( currParameter.searchPathEnvVar() )

		classNameFilter = "*"
		try :
			classNameFilter = currParameter.userData()["UI"]["classNameFilter"].value
		except :
			pass
		classNameFilter = IECore.StringData( classNameFilter )

		classNames = IECore.StringVectorData()
		classVersions = IECore.IntVectorData()
		classOrder = IECore.StringVectorData()

		values = IECore.CompoundObject()

		for c in classes:

			pName = c[1]
			classOrder.append( pName )
			classNames.append( c[2] )
			classVersions.append( c[3] )

			v = IECore.CompoundObject()

			childOldParam = None
			if oldParameter and pName in oldParameter.keys() :
				oldClass = oldParameter.getClass( pName )
				childOldParam = oldClass.parameters()

			RelativePreset.__grabParameterChanges(
				c[0].parameters(),
				childOldParam,
				v,
				paramPath + "." + pName
			)

			if len(v) :
				values[c[1]] = v

		removedParams = []
		if oldParameter :
			removedParams = list( set( oldParameter.keys() ).difference( classOrder ) )
			if removedParams :
				data["_removedParamNames_"] = IECore.StringVectorData( removedParams )
				data["_removedClassNames_"] = IECore.StringVectorData()
				for pName in removedParams :
					oldClass = oldParameter.getClass( pName, True )
					data["_removedClassNames_"].append( oldClass[1] )

		modifiedParams = IECore.StringVectorData()
		modifiedClassNames = IECore.StringVectorData()
		modifiedClassVersions = IECore.IntVectorData()
		addedParam = IECore.BoolVectorData()

		for i in xrange(0,len(classOrder)):
			pName = classOrder[i]
			cName = classNames[i]
			cVersion = classVersions[i]
			oldClassName = None
			oldClassVersion = None

			if oldParameter :
				try:
					oldClass = oldParameter.getClass( pName, True )
					oldClassName = oldClass[1]
					oldClassVersion = oldClass[2]

				except Exception, e:
					# added parameter...
					pass

			if cName != oldClassName or cVersion != oldClassVersion :
				modifiedParams.append( pName )
				modifiedClassNames.append( cName )
				modifiedClassVersions.append( cVersion )
				added = (oldClassName is None)
				# if we are changing the class type, we have to mark as if we
				# were removing it too
				if cName != oldClassName and not oldClassName is None:
					if not "_removedParamNames_" in data :
						data["_removedParamNames_"] = IECore.StringVectorData()
						data["_removedClassNames_"] = IECore.StringVectorData()
					data["_removedParamNames_"].append(pName)
					data["_removedClassNames_"].append(oldClassName)
					added = True

				addedParam.append( added )
		
		if len(modifiedParams) :
			data["_modifiedParamsNames_"] = modifiedParams
			data["_modifiedClassNames_"] = modifiedClassNames
			data["_modifiedClassVersions_"] = modifiedClassVersions
			data["_addedParam_"] = addedParam

		parameterOrder = classOrder
		baseOrder = classOrder
		if oldParameter :
			baseOrder = IECore.StringVectorData( filter( lambda n: not n in removedParams, oldParameter.keys() ) )
			
		if baseOrder != parameterOrder :
			data["_parameterOrder_"] = parameterOrder

		if len(values):
			# keep the original classes to which the parameters were edited
			for pName in values.keys() :
				values[pName]["_class_"] = IECore.StringData( classNames[classOrder.index(pName)] )
			data["_values_"] = values

		if len(data):
			data["_classNameFilter_" ] = classNameFilter
			data["_type_"] = IECore.StringData( "ClassVectorParameter" )

	@staticmethod
	def __applyParameterChanges( parameter, data, paramPath = "" ) :

		if isinstance( parameter, IECore.ClassParameter ) :

			RelativePreset.__applyClassParameterChanges( parameter, data, paramPath )

		elif isinstance( parameter, IECore.ClassVectorParameter ) :

			RelativePreset.__applyClassVectorChanges( parameter, data, paramPath )

		elif isinstance( parameter, IECore.CompoundParameter ) :

			RelativePreset.__applyCompoundParameterChanges( parameter, data, paramPath )

		elif isinstance( parameter, IECore.Parameter ) :

			RelativePreset.__applySimpleParameterChanges( parameter, data, paramPath )

		else :
			IECore.msg( 
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unrecognized type (%s) for parameter %s. Not affected by preset." % ( parameter.typeName(), parameter.name )
			)

	@staticmethod		
	def __applyCompoundParameterChanges( parameter, data, paramPath ) :
		
		if data["_type_"].value != "CompoundParameter" :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found CompoundParameter."
					% ( paramPath, data["_type_"].value )
			)
			return

		for p in data.keys() :
			if p in [ "_type_", "_class_" ] :
				continue

			if paramPath :
				newParamPath = paramPath + "." + p
			else :
				newParamPath = p

			if p not in parameter :
				IECore.msg( 
					IECore.Msg.Level.Warning, 
					"IECore.RelativePreset", 
					"Could not find parameter '%s'. Preset value ignored." % newParamPath
				)
				continue
				
			RelativePreset.__applyParameterChanges( parameter[p], data[p], newParamPath )
			
	@staticmethod		
	def __applySimpleParameterChanges( parameter, data, paramPath ) :

		if data["_type_"].value != parameter.typeName() :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found %s."
					% ( paramPath, data["_type_"].value, parameter.typeName() )
			)
			return
		
		try:
			parameter.setValue( data["_value_"] )
		except Exception, e:
			IECore.msg( IECore.Msg.Level.Warning, "IECore.RelativePreset", str(e) )
		
	@staticmethod		
	def __applyClassParameterChanges( parameter, data, paramPath ) :
		
		if data["_type_"].value != "ClassParameter" :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found ClassParameter."
					% ( paramPath, data["_type_"].value )
			)
			return

		c = parameter.getClass( True )
		
		className = data["_className_"].value
		classVersion = data["_classVersion_"].value

		if data["_className_"].value != className :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected to find %s but the current class is %s."
					% ( paramPath, data["_className_"].value, className )
			)
			return

		if c[1] != className or c[2] != classVersion :
			parameter.setClass( className, classVersion )
			
		c = parameter.getClass( False )
		if c :
			RelativePreset.__applyParameterChanges( c.parameters(), data["_classValue_"], paramPath )
		
	@staticmethod
	def __applyClassVectorChanges( parameter, data, paramPath ) :
		
		if data["_type_"].value != "ClassVectorParameter" :
			IECore.msg(
				IECore.Msg.Level.Warning, 
				"IECore.RelativePreset", 
				"Unable to set preset on '%s'. Expected %s but found ClassVectorParameter."
					% ( paramPath, data["_type_"].value )
			)
			return

		# remove parameters if they match in parameter name and class name
		if "_removedParamNames_" in data :
			for (i,pName) in enumerate( data["_removedParamNames_"] ):
				if pName in parameter.keys() :
					c = parameter.getClass( pName, True )
					if c and c[1] == data["_removedClassNames_"][i] :
						parameter.removeClass( pName )

		paramRemaps = {}		

		if "_modifiedParamsNames_" in data :
			modifiedParams = data["_modifiedParamsNames_"]
			modifiedClassNames = data["_modifiedClassNames_"]
			modifiedClassVersions = data["_modifiedClassVersions_"]
			addedParam = data["_addedParam_"]

			for i in range( len( modifiedClassNames ) ) :
	
				if addedParam[i] :

					# must add a new parameter, no matter what.

					paramName = modifiedParams[i]
					if paramName in parameter:

						newParamName = parameter.newParameterName()

						if not re.match("^p[0-9]+$", paramName) :
							IECore.msg(
								IECore.Msg.Level.Warning, 
								"IECore.RelativePreset", 
								"Custom parameter %s.%s is being renamed to %s..."
									% ( paramPath, paramName, newParamName )
							)
							
						paramRemaps[ modifiedParams[i] ] = newParamName
						paramName = newParamName

					parameter.setClass( paramName, modifiedClassNames[i], modifiedClassVersions[i] )

				else :

					# must find an existing matching parameter, no matter what
					if modifiedParams[i] in parameter:
						c = parameter.getClass( modifiedParams[i], True )
						if modifiedClassNames[i] == c[1] :
							if modifiedClassVersions[i] != c[2] :
								parameter.setClass( modifiedParams[i], modifiedClassNames[i], modifiedClassVersions[i] )
						else :
							IECore.msg(
								IECore.Msg.Level.Warning, 
								"IECore.RelativePreset", 
								"Parameter '%s.%s' has a different class. Expected %s but found %s. Ignoring class change on this parameter."
									% ( paramPath, modifiedParams[i], modifiedClassNames[i], c[1] )
							)
					else :
						IECore.msg(
							IECore.Msg.Level.Warning, 
							"IECore.RelativePreset", 
							"Unable to find parameter '%s.%s' in %s. Ignoring class change on this parameter."
								% ( paramPath, modifiedParams[i], parameter.name )
						)

		if "_values_" in data :

			for paramName in data["_values_"].keys() :

				remapedParamName = paramRemaps.get( paramName, paramName )
				presetValue = data["_values_"][paramName]

				if remapedParamName in parameter.keys() :
					c = parameter.getClass( remapedParamName, True )
					if c[1] == presetValue["_class_"].value :

						RelativePreset.__applyParameterChanges(
							c[0].parameters(),
							presetValue,
							paramPath + "." + remapedParamName
						)
					else :
						IECore.msg(
							IECore.Msg.Level.Warning, 
							"IECore.RelativePreset", 
							"Ignoring preset values for parameter %s.%s. Expected class %s but found %s."
								% ( paramPath, remapedParamName, presetValue["_class_"].value, c[1] )
						)

				else :
					IECore.msg(
						IECore.Msg.Level.Warning, 
						"IECore.RelativePreset", 
						"Unable to find parameter '%s.%s' in %s. Ignoring this preset changes."
							% ( paramPath, remapedParamName, parameter.name )
					)

		if "_parameterOrder_" in data :
			# there was some kind of change in the order of parameters as well...
			# get tuples ( classInstance, parameterName, className, classVersion )
			classList = parameter.getClasses( True )
			originalOrder = list(data["_parameterOrder_"])
			currOrder = map( lambda c: c[1], classList )

			def classOrder( c1, c2 ):
				try:
					i1 = originalOrder.index( c1[1] )
					i2 = originalOrder.index( c2[1] )
					return cmp( i1, i2 )
				except:
					i1 = currOrder.index( c1[1] )
					i2 = currOrder.index( c2[1] )
					return cmp( i1, i2 )

			classList.sort( classOrder )
			parameter.setClasses( map( lambda c: c[1:], classList ) )
		
	@staticmethod
	def __applicableTo( parameter, data ) :
				
		if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :
			
			if data["_type_"].value != "CompoundParameter":
				return False
			
		elif isinstance( parameter, IECore.ClassParameter ) :
			
			if data["_type_"].value != "ClassParameter":
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False

		elif isinstance( parameter, IECore.ClassVectorParameter ) :
			
			if data["_type_"].value != "ClassVectorParameter":
				return False
			
			classNameFilter = "*"
			try :
				classNameFilter = parameter.userData()["UI"]["classNameFilter"].value
			except :
				pass
			
			if classNameFilter != data["_classNameFilter_"].value:
				return False
				
		else :
			
			if data["_type_"].value != parameter.typeName():
				return False

			if not parameter.valueValid( data["_value_"] )[0]:
				return False		
		
		return True


IECore.registerRunTimeTyped( RelativePreset )
	
