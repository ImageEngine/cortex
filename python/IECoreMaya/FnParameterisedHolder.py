##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

import maya.OpenMaya
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
	#
	# setParameterised( string className, int classVersion, string searchPathEnvVar )
	# Sets the held object by specifying a class that will be loaded using the IECore.ClassLoader.
	# searchPathEnvVar specifies an environment variable which holds a colon separated search path for the 
	# ClassLoader. This form allows the held class to be reinstantiated across scene save/load.
	def setParameterised( self, classNameOrParameterised, classVersion=None, envVarName=None ) :
	
		if isinstance( classNameOrParameterised, str ) :
			result = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised, classVersion, envVarName )
		else :
			result = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised )
	
		for c in self.__setParameterisedCallbacks :
			c( self )
	
		return result
	
	## Updates the node to reflect any addition or removal of parameters.
	def updateParameterised( self ) :
	
		return _IECoreMaya._parameterisedHolderUpdateParameterised( self )
	
	## Returns a tuple of the form (parameterised, className, classVersion, searchPathEnvVar).
	def getParameterised( self ) :
		
		return _IECoreMaya._parameterisedHolderGetParameterised( self )

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

	## Returns the IECore.Parameter object being represented by the given OpenMaya.MPlug
	# instance.
	def plugParameter( self, plug ) :
	
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
