##########################################################################
#
#  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

from __future__ import with_statement

import warnings

import maya.OpenMaya
import maya.cmds

import IECore

import _IECoreMaya
import StringUtil

## A function set for operating on the various IECoreMaya::ParameterisedHolder
# types. This allows setting and getting of plug and parameter values, and
# setting and getting of the Parameterised object being held.
class FnParameterisedHolder( maya.OpenMaya.MFnDependencyNode ) :

	## Initialise the function set for the given object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		if isinstance( object, str ) or isinstance( object, unicode ) :
			object = StringUtil.dependencyNodeFromString( object )

		maya.OpenMaya.MFnDependencyNode.__init__( self, object )

	## Sets the IECore.Parameterised object held by the node. This function can be called
	# in two ways :
	#
	# setParameterised( Parameterised p )
	# Directly sets the held object to the Parameterised instance p. Note that this
	# form doesn't provide enough information for the node to be reinstantiated
	# after saving and reloading of the maya scene - see the form below for that.
	# Also note that this form is not undoable, and that the undoable parameter will therefore
	# be ignored.
	#
	# setParameterised( string className, int classVersion, string searchPathEnvVar, bool undoable )
	# Sets the held object by specifying a class that will be loaded using the IECore.ClassLoader.
	# searchPathEnvVar specifies an environment variable which holds a colon separated search path for the
	# ClassLoader. This form allows the held class to be reinstantiated across scene save/load, and is
	# also undoable if requested using the undoable parameter. If classVersion is omitted, None, or negative,
	# then the highest available version will be used.
	def setParameterised( self, classNameOrParameterised, classVersion=None, envVarName=None, undoable=True ) :

		if isinstance( classNameOrParameterised, str ) :
			if classVersion is None or classVersion < 0 :
				classVersions = IECore.ClassLoader.defaultLoader( envVarName ).versions( classNameOrParameterised )
				classVersion = classVersions[-1] if classVersions else 0
			if undoable  :
				if self.getParameterised()[0] :
					self.setParameterisedValues()
					_IECoreMaya._parameterisedHolderAssignModificationState(
						self.getParameterised()[0].parameters().getValue().copy(),
						self._classParameterStates(),
						None,
						None
					)
				else :
					_IECoreMaya._parameterisedHolderAssignModificationState( None, None, None, None	)
				maya.cmds.ieParameterisedHolderModification( self.fullPathName(), classNameOrParameterised, classVersion, envVarName )
				# no need to despatch callbacks as that is done by the command, so that the callbacks happen on undo and redo too.
			else :
				_IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised, classVersion, envVarName )
				self._despatchSetParameterisedCallbacks( self.fullPathName() )
		else :
			result = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised )
			self._despatchSetParameterisedCallbacks( self.fullPathName() )

	## Returns a tuple of the form (parameterised, className, classVersion, searchPathEnvVar).
	# The returned parameterised object is not guaranteed to be in sync with the plug values.
	# Use setParameterisedValues function if you need that.
	def getParameterised( self ) :

		return _IECoreMaya._parameterisedHolderGetParameterised( self )

	## Returns a context manager for use with the with statement. This can be used to
	# scope edits to Parameter values (including the classes held by ClassParameters and
	# ClassVectorParameters) in such a way that they are automatically transferred onto
	# the maya attributes and furthermore in an undoable fashion.
	def parameterModificationContext( self ) :

		return _ParameterModificationContext( self )

	## Sets the values of the plugs representing the parameterised object,
	# using the current values of the parameters. If the undoable parameter is True
	# then this method is undoable using the standard maya undo mechanism.
	# \note If this is applied to a node in a reference, then reference edits will
	# be produced for every parameter plug, even if the values are not changing.
	# You may prefer to set parameter values within a parameterModificationContext()
	# instead as this automatically transfers the values to maya, while also avoiding
	# the reference edit problem.
	def setNodeValues( self, undoable=True ) :

		if undoable :
			maya.cmds.ieParameterisedHolderSetValue( self.fullPathName() )
		else :
			_IECoreMaya._parameterisedHolderSetNodeValues( self )

	## Set the value for the plug representing parameter, using the current
	# value of the parameter. If the undoable parameter is True
	# then this method is undoable using the standard maya undo mechanism.
	def setNodeValue( self, parameter, undoable=True ) :

		if undoable :
			maya.cmds.ieParameterisedHolderSetValue( self.fullPathName(), plug=self.parameterPlug( parameter ).partialName() )
		else :
			_IECoreMaya._parameterisedHolderSetNodeValue( self, parameter )

	## Transfers the values from the plugs of the node onto the
	# parameters of the held Parameterised object.
	def setParameterisedValues( self ) :

		return _IECoreMaya._parameterisedHolderSetParameterisedValues( self )

	## Sets the value of parameter from the value held by the plug representing it.
	def setParameterisedValue( self, parameter ) :

		return _IECoreMaya._parameterisedHolderSetParameterisedValue( self, parameter )

	## Returns the OpenMaya.MPlug object responsible for representing the given parameter.
	def parameterPlug( self, parameter ) :

		plugName = _IECoreMaya._parameterisedHolderParameterPlug( self, parameter )
		if plugName == "" :
			return maya.OpenMaya.MPlug()

		return StringUtil.plugFromString( self.fullPathName() + "." + plugName )

	## Returns a string containing a full pathname for the plug representing the given parameter.
	def parameterPlugPath( self, parameter ) :

		plugName = _IECoreMaya._parameterisedHolderParameterPlug( self, parameter )
		if not plugName :
			return ""

		return self.fullPathName() + "." + plugName

	## Returns the IECore.Parameter object being represented by the given fullPathName
	# of the maya plug or its OpenMaya.MPlug instance.
	def plugParameter( self, plug ) :

		if isinstance( plug, str ) or isinstance( plug, unicode ) :
			plug = StringUtil.plugFromString( plug )

		return _IECoreMaya._parameterisedHolderPlugParameter( self, plug )

	## Returns the full path name to this node.
	def fullPathName( self ) :

		try :
			f = maya.OpenMaya.MFnDagNode( self.object() )
			return f.fullPathName()
		except :
			pass

		return self.name()

	## Add a callback which will be invoked whenever FnParameterisedHolder.setParameterised
	# is called. The expected function signature is callback( FnParameterisedHolder ).
	@classmethod
	def addSetParameterisedCallback( cls, callback ) :

		cls.__setParameterisedCallbacks.add( callback )

	## Remove a previously added callback.
	@classmethod
	def removeSetParameterisedCallback( cls, callback ) :

		cls.__setParameterisedCallbacks.remove( callback )

	__setParameterisedCallbacks = set()
	@classmethod
	def _despatchSetParameterisedCallbacks( cls, nodeName ) :

		fnPH = FnParameterisedHolder( nodeName )
		for c in cls.__setParameterisedCallbacks :
			c( fnPH )

	## Adds a callback which will be invoked whenever FnParameterisedHolder.setClassVectorParameterClasses
	# is called. The expected function signature is callback( FnParameterisedHolder, parameter )
	@classmethod
	def addSetClassVectorParameterClassesCallback( cls, callback ) :

		cls.__setClassVectorParameterClassesCallbacks.add( callback )

	## Removes a callback added previously with addSetClassVectorParameterClassesCallback()
	@classmethod
	def removeSetClassVectorParameterClassesCallback( cls, callback ) :

		cls.__setClassVectorParameterClassesCallbacks.remove( callback )

	__setClassVectorParameterClassesCallbacks = set()

	# Invoked by the ieParameterisedHolderModification MPxCommand. It must be invoked from there
	# rather than the methods above so that callbacks get correctly despatched during undo and redo.
	@classmethod
	def _despatchSetClassVectorParameterClassesCallbacks( cls, plugPath ) :

		# This function gets called deferred (on idle) from ParameterisedHolderSetClassParameterCmd.cpp.
		# Because of the deferred nature of the call, it's possible that the plug has been destroyed before
		# we're called - in this case we just don't despatch callbacks.
		## \todo It might be better to not defer the call to this function, and have any callbacks which
		# need deferred evaluation (the ui callbacks in ClassVectorParameterUI for instance) arrange for that
		# themselves.
		if not maya.cmds.objExists( plugPath ) :
			return

		fnPH = FnParameterisedHolder( StringUtil.nodeFromAttributePath( plugPath ) )
		parameter = fnPH.plugParameter( plugPath )
		for c in cls.__setClassVectorParameterClassesCallbacks :
			c( fnPH, parameter )

	## Adds a callback which will be invoked whenever FnParameterisedHolder.setClassParameterClass
	# is called. The expected function signature is callback( FnParameterisedHolder, parameter )
	@classmethod
	def addSetClassParameterClassCallback( cls, callback ) :

		cls.__setClassParameterClassCallbacks.add( callback )

	## Removes a callback added previously with addSetClassParameterClassCallback()
	@classmethod
	def removeSetClassParameterClassCallback( cls, callback ) :

		cls.__setClassParameterClassCallbacks.remove( callback )

	__setClassParameterClassCallbacks = set()

	# Invoked by the ieParameterisedHolderModification MPxCommand. It must be invoked from there
	# rather than the methods above so that callbacks get correctly despatched during undo and redo.
	@classmethod
	def _despatchSetClassParameterClassCallbacks( cls, plugPath ) :

		# See comment in _despatchSetClassVectorParameterClassesCallbacks
		if not maya.cmds.objExists( plugPath ) :
			return

		fnPH = FnParameterisedHolder( StringUtil.nodeFromAttributePath( plugPath ) )
		parameter = fnPH.plugParameter( plugPath )
		for c in cls.__setClassParameterClassCallbacks :
			c( fnPH, parameter )

	def _classParameterStates( self, parameter=None, parentParameterPath="", result=None ) :

		if result is None :
			result = IECore.CompoundData()

		if parameter is None :
			parameter = self.getParameterised()[0].parameters()

		parameterPath = parameter.name
		if parentParameterPath :
			parameterPath = parentParameterPath + "." + parameterPath

		if isinstance( parameter, IECore.ClassParameter ) :

			classInfo = parameter.getClass( True )
			result[parameterPath] = IECore.CompoundData( {
				"className" : IECore.StringData( classInfo[1] ),
				"classVersion" : IECore.IntData( classInfo[2] ),
				"searchPathEnvVar" : IECore.StringData( classInfo[3] ),
			} )

		elif isinstance( parameter, IECore.ClassVectorParameter ) :

			classInfo = parameter.getClasses( True )
			if classInfo :
				classInfo = zip( *classInfo )
			else :
				classInfo = [ [], [], [], [] ]

			result[parameterPath] = IECore.CompoundData({
				"parameterNames" : IECore.StringVectorData( classInfo[1] ),
				"classNames" : IECore.StringVectorData( classInfo[2] ),
				"classVersions" : IECore.IntVectorData( classInfo[3] ),
			} )

		if isinstance( parameter, IECore.CompoundParameter ) :

			for c in parameter.values() :

				self._classParameterStates( c, parameterPath, result )

		return result

	## Returns the maya node type that this function set operates on
	@classmethod
	def _mayaNodeType( cls ):

		return "ieParameterisedHolderNode"

	## Lists the ieParameterisedHolderNodes in the current scene. The keyword arguments operate as follows :
	#
	# selection :
	# Only list holders in the current selection. Defaults to False
	#
	# fnSets :
	# Returns a list of FnParameterisedHolder instances if True, otherwise returns node names. Defaults to True
	#
	# classType :
	# Python class: if specified, only lists holders holding this class
	#
	@classmethod
	def ls( cls, selection=False, fnSets=True, classType=None ) :

		nodeNames = maya.cmds.ls( sl=selection, leaf=True, type=cls._mayaNodeType() )
		matches = []
		for n in nodeNames :
			fnH = cls( n )
			if classType is None or isinstance( fnH.getParameterised()[0], classType ) :
				matches.append( fnH )

		if fnSets :
			return matches
		else :
			return [ x.fullPathName() for x in matches ]

class _ParameterModificationContext :

	def __init__( self, fnPH ) :

		self.__fnPH = fnPH

	def __enter__( self ) :

		self.__fnPH.setParameterisedValues()
		self.__originalValues = self.__fnPH.getParameterised()[0].parameters().getValue().copy()
		self.__originalClasses = self.__fnPH._classParameterStates()

		return self.__fnPH.getParameterised()[0]

	def __exit__( self, type, value, traceBack ) :

		_IECoreMaya._parameterisedHolderAssignModificationState(
			self.__originalValues,
			self.__originalClasses,
			self.__fnPH.getParameterised()[0].parameters().getValue().copy(),
			self.__fnPH._classParameterStates(),
		)

		maya.cmds.ieParameterisedHolderModification( self.__fnPH.fullPathName() )

