##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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
			if undoable :
				maya.cmds.ieParameterisedHolderClassModification( self.fullPathName(), classNameOrParameterised, classVersion, envVarName )
				# no need to despatch callbacks as that is done by the command, so that the callbacks happen on undo and redo too.
			else :
				_IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised, classVersion, envVarName )
				self._despatchSetParameterisedCallbacks( self.fullPathName() )
		else :
			result = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised )
			self._despatchSetParameterisedCallbacks( self.fullPathName() )

	## Returns a tuple of the form (parameterised, className, classVersion, searchPathEnvVar).
	def getParameterised( self ) :

		return _IECoreMaya._parameterisedHolderGetParameterised( self )

	## Returns a context manager for use with the with statement, to edit the contents
	# of ClassParameters and ClassVectorParameters in an undoable fashion.
	def classParameterModificationContext( self ) :
	
		return _ClassParameterModificationContext( self )

	## \deprecated
	## \todo Remove for major version 6
	def setClassParameterClass( self, parameter, className, classVersion, searchPathEnvVar ) :
		
		warnings.warn( "Use classParameterModificationContext() and manipulate the parameter yourself.", DeprecationWarning, 2 )
		
		with self.classParameterModificationContext() :
			parameter.setClass( className, classVersion, searchPathEnvVar )
		
	## \deprecated
	## \todo Remove for major version 6
	def setClassVectorParameterClasses( self, parameter, classes ) :
		
		warnings.warn( "Use classParameterModificationContext() and manipulate the parameter yourself.", DeprecationWarning, 2 )
		
		with self.classParameterModificationContext() :
			parameter.setClasses( classes )
		
	## Sets the values of the plugs representing the parameterised object,
	# using the current values of the parameters. If the undoable parameter is True
	# then this method is undoable using the standard maya undo mechanism.
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
	
	# Invoked by the ieParameterisedHolderClassModification MPxCommand. It must be invoked from there
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
	
	# Invoked by the ieParameterisedHolderClassModification MPxCommand. It must be invoked from there
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
				
			
class _ClassParameterModificationContext :

	def __init__( self, fnPH ) :
	
		self.__fnPH = fnPH

	def __enter__( self ) :
	
		self.__fnPH.setParameterisedValues()
		self.__originalValues = self.__fnPH.getParameterised()[0].parameters().getValue().copy()
		
	def __exit__( self, type, value, traceBack ) :
	
		_IECoreMaya._parameterisedHolderAssignUndoValue( self.__originalValues )
		maya.cmds.ieParameterisedHolderClassModification( self.__fnPH.fullPathName() )
