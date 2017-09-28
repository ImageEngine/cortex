##########################################################################
#
#  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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
# The comparison on elements in a ClassVectorParameters takes in consideration both the parameter name and
# the loaded class name in order to consider the "same" element. We do that do try to work around the fact
# that the parameter names ("p0", "p1", etc) are very simple and easy to reapper after a sequence of removal/addition
# operations in a ClassVectorParameter. The method is not 100% safe but should work for most cases.
# \todo Consider adding a protected member that is responsible for that comparison and enable derived classes to
# do other kinds of comparisons, for example, using additional parameters such as user labels.
#
class RelativePreset( IECore.Preset ) :

	## \param currParameter, IECore.Parameter, represents the parameter state after all changes have been made.
	## \param oldParameter, IECore.Parameter, represents the parameter state before any changes.
	## \param compareFilter, callable function that receives currParameter and oldParameter child and it should
	## return a boolean to indicate if the difference should be computed or not.
	def __init__( self, currParameter=None, oldParameter=None, compareFilter = None ) :

		IECore.Preset.__init__( self )

		self.__data = IECore.CompoundObject()
		if compareFilter is None :
			self.__compareFilter = lambda x,y: True
		else :
			self.__compareFilter = compareFilter

		# accepts no parameters at all.
		if currParameter is None and oldParameter is None :
			return

		if not isinstance( currParameter, IECore.Parameter ) :
			raise TypeError, "Parameter currParameter must be a IECore.Parameter object!"

		if not oldParameter is None :

			if not isinstance( oldParameter, IECore.Parameter ) :
				raise TypeError, "Parameter oldParameter must be a IECore.Parameter object!"

			if currParameter.typeId() != oldParameter.typeId() :
				raise TypeError, "Mismatching types for currParameter and oldParameter!"

		self.__grabParameterChanges( currParameter, oldParameter, self.__data )

	## \see IECore.Preset.applicableTo
	def applicableTo( self, parameterised, rootParameter ) :

		return RelativePreset.__applicableTo( rootParameter, self.__data )

	def getDiffData( self ):
		"""Returns a IECore.CompoundObject instance that contains the description of all the differences between the two parameters provided when creating this preset."""
		return self.__data.copy()

	def setDiffData( self, data ):
		"""Use this function to recreate a RelativePreset from data previously returned by getDiffData()."""
		if not isinstance( data, IECore.CompoundObject ):
			raise TypeError, "Invalid data type! Must be a IECore.CompoundObject"

		self.__data = data.copy()

	## \see IECore.Preset.__call__
	def __call__( self, parameterised, rootParameter ) :

		if not self.applicableTo( parameterised, rootParameter ) :
			raise RuntimeError, "Sorry, this preset is not applicable to the given parameter."

		if len( self.__data ) :
			self.__applyParameterChanges( rootParameter, self.__data )

	def __grabParameterChanges( self, currParameter, oldParameter, data, paramPath = "" ) :

		if not oldParameter is None:

			if currParameter.staticTypeId() != oldParameter.staticTypeId() :
				raise Exception, "Incompatible parameter %s!" % paramPath

		if not self.__compareFilter( currParameter, oldParameter ) :
			return

		if isinstance( currParameter, IECore.ClassParameter ) :

			self.__grabClassParameterChanges( currParameter, oldParameter, data, paramPath )

		elif isinstance( currParameter, IECore.ClassVectorParameter ) :

			self.__grabClassVectorParameterChanges( currParameter, oldParameter, data, paramPath )

		elif isinstance( currParameter, IECore.CompoundParameter ) :

			self.__grabCompoundParameterChanges( currParameter, oldParameter, data, paramPath )

		else :

			self.__grabSimpleParameterChanges( currParameter, oldParameter, data, paramPath )

	def __grabCompoundParameterChanges( self, currParameter, oldParameter, data, paramPath ) :

		for p in currParameter.keys() :

			newData = IECore.CompoundObject()
			childOldParam = None
			if not oldParameter is None :
				if p in oldParameter.keys() :
					childOldParam = oldParameter[p]

			self.__grabParameterChanges(
				currParameter[p],
				childOldParam,
				newData,
				paramPath + "." + p
			)

			if len(newData) :
				data[p] = newData

		if len(data):
			data["_type_"] = IECore.StringData( "CompoundParameter" )

	def __grabSimpleParameterChanges( self, currParameter, oldParameter, data, paramPath ) :

		if not oldParameter is None :

			if currParameter.getValue() == oldParameter.getValue() :
				return

		data["_type_"] = IECore.StringData( currParameter.typeName() )
		data["_value_"] = currParameter.getValue().copy()

	def __grabClassParameterChanges( self, currParameter, oldParameter, data, paramPath ) :

		c = currParameter.getClass( True )

		className = c[1]
		classVersion = c[2]
		classNameFilter = "*"
		try :
			classNameFilter = currParameter.userData()["UI"]["classNameFilter"].value
		except :
			pass

		oldClassName = None
		oldClassVersion = None
		childOldParam = None
		if not oldParameter is None :
			oldClass = oldParameter.getClass( True )
			oldClassName = oldClass[1]
			oldClassVersion = oldClass[2]
			if oldClass[0] :
				childOldParam = oldClass[0].parameters()

		classValue = IECore.CompoundObject()

		if c[0] :

			self.__grabParameterChanges(
				c[0].parameters(),
				childOldParam,
				classValue,
				paramPath
			)

		if len(classValue):
			data["_classValue_"] = classValue

		if len(data) or className != oldClassName or classVersion != oldClassVersion :
			data["_className_"] = IECore.StringData(className)
			data["_classVersion_"] = IECore.IntData(classVersion)
			data["_classNameFilter_"] = IECore.StringData(classNameFilter)
			data["_type_"] = IECore.StringData( "ClassParameter" )

	def __grabClassVectorParameterChanges( self, currParameter, oldParameter, data, paramPath ) :

		classes = currParameter.getClasses( True )

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
			if not oldParameter is None and pName in oldParameter.keys() :
				oldClass = oldParameter.getClass( pName )
				if oldClass :
					childOldParam = oldClass.parameters()

			self.__grabParameterChanges(
				c[0].parameters(),
				childOldParam,
				v,
				paramPath + "." + pName
			)

			if len(v) :
				values[c[1]] = v

		removedParams = []
		if not oldParameter is None :
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

			if not oldParameter is None :
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
					removedParams.append(pName)
					added = True

				addedParam.append( added )

		if len(modifiedParams) :
			data["_modifiedParamsNames_"] = modifiedParams
			data["_modifiedClassNames_"] = modifiedClassNames
			data["_modifiedClassVersions_"] = modifiedClassVersions
			data["_addedParam_"] = addedParam

		# get all non-new parameters
		parameterOrder = filter( lambda n: not n in modifiedParams or not addedParam[ modifiedParams.index(n) ], classOrder )
		baseOrder = parameterOrder
		if not oldParameter is None :
			# get all non-deleted original parameters
			baseOrder = filter( lambda n: not n in removedParams, oldParameter.keys() )

		if baseOrder != parameterOrder :

			if len(baseOrder) != len(parameterOrder):
				raise Exception, "Unnexpected error. Unmatching parameter lists!"

			# clamp to the smallest list containing the differences
			for start in xrange(0,len(baseOrder)):
				if baseOrder[start] != parameterOrder[start] :
					break
			for endPos in xrange(len(baseOrder),0,-1):
				if baseOrder[endPos-1] != parameterOrder[endPos-1] :
					break

			data["_modifiedOrder_"] = IECore.StringVectorData( parameterOrder[start:endPos] )

		if len(values):
			# keep the original classes to which the parameters were edited
			for pName in values.keys() :
				values[pName]["_class_"] = IECore.StringData( classNames[classOrder.index(pName)] )
			data["_values_"] = values

		if len(data):
			data["_classNameFilter_" ] = classNameFilter
			data["_type_"] = IECore.StringData( "ClassVectorParameter" )
			data["_paramNames_"] = classOrder
			data["_classNames_"] = classNames

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

		if c[1] != className or c[2] != classVersion :
			parameter.setClass( className, classVersion )

		c = parameter.getClass( False )
		if c and '_classValue_' in data :
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
			addedCount = 0

			# first modify items
			for i in range( len( modifiedClassNames ) ) :

				if addedParam[i] :

					addedCount += 1

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

		# get a list of classes before the addition of new items
		newOrder = False
		newClassList = map( lambda c: c[1:], parameter.getClasses( True ) )
		newParamList = map( lambda c: c[0], newClassList )
		# compare each class with whatever existed when we created the RelativePreset and see which ones are the same
		sameClasses = set()
		for c in newClassList :

			if '_modifiedParamsNames_' in data :
				# If the preset has added this parameter it should not match current parameters in the vector, no matter if the class matches. Is it always the case?
				if c[0] in data['_modifiedParamsNames_'] :
					if data['_addedParam_'][ data['_modifiedParamsNames_'].index(c[0]) ] :
						continue

			try :
				i = data['_paramNames_'].index(c[0])
			except :
				continue
			if c[1] == data['_classNames_'][i] :
				sameClasses.add( c[0] )

		if "_modifiedOrder_" in data :
			# there was some kind of change in the order of parameters as well...

			modifiedOrder = filter( lambda pName: pName in sameClasses, data["_modifiedOrder_"] )

			# find the range of parameters that lie between the reordered parameters in the current vector
			firstParam = None
			lastParam = None
			for (i,pName) in enumerate(newParamList) :
				if pName in modifiedOrder :
					if firstParam is None:
						firstParam = i
					lastParam = i

			if firstParam != lastParam :

				# adds one by one the unknown parameters that lied between the reordered parameters.
				for pName in newParamList[firstParam:lastParam+1] :
					if not pName in modifiedOrder :
						modifiedOrder.insert( modifiedOrder.index(baseParam)+1, pName )
					baseParam = pName

				def classOrder( c1, c2 ):
					# if both elements were on the original reordering operation we use their relationship
					if c1[0] in modifiedOrder and c2[0] in modifiedOrder:
						i1 = modifiedOrder.index( c1[0] )
						i2 = modifiedOrder.index( c2[0] )
						return cmp( i1, i2 )

					# otherwise we use the current order.
					i1 = newParamList.index( c1[0] )
					i2 = newParamList.index( c2[0] )
					return cmp( i1, i2 )

				newClassList.sort( classOrder )
				newParamList = map( lambda c: c[0], newClassList )
				newOrder = True

		if "_modifiedParamsNames_" in data :
			# now add items to the appropriate spot in the newClassList and newParamList
			if addedCount :

				newOrder = True
				prevActualParam = None
				lastActualParamInsertion = None
				currClasses = parameter.getClasses( True )

				for pName in data["_paramNames_"] :
					if pName in sameClasses :
						if pName in newParamList :
							prevActualParam = pName
							continue
					if pName in modifiedParams :
						i = modifiedParams.index(pName)
						if addedParam[ i ] :
							if prevActualParam is None :
								if lastActualParamInsertion is None :
									# Here we assume that the new parameter should
									# go to the top because its predecessors don't exist on the
									# new vector. Maybe it could also print a warning message..
									lastActualParamInsertion = 0
								else :
									lastActualParamInsertion += 1
							else :
								lastActualParamInsertion = newParamList.index( prevActualParam ) + 1
								prevActualParam = None

							if pName in parameter:

								newParamName = parameter.newParameterName()

								if not re.match("^p[0-9]+$", pName) :
									IECore.msg(
										IECore.Msg.Level.Warning,
										"IECore.RelativePreset",
										"Custom parameter %s.%s is being renamed to %s..."
											% ( paramPath, pName, newParamName )
									)
								paramRemaps[ pName ] = newParamName
								pName = newParamName
							# add the parameter to the vector, so that next calls to parameter.newParameterName() will work.
							parameter.setClass( pName, modifiedClassNames[i], modifiedClassVersions[i] )
							# update our official new arrays
							newParamList.insert(lastActualParamInsertion, pName)
							newClassList.insert(lastActualParamInsertion, (pName,modifiedClassNames[i], modifiedClassVersions[i]) )


		# update parameters with new order
		if newOrder :
			parameter.setClasses( newClassList )

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

	@staticmethod
	def __applicableTo( parameter, data ) :

		if len(data) == 0 :
			return True

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

