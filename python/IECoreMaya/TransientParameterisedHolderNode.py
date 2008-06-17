##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
import maya.cmds
import IECoreMaya
import _IECoreMaya
import StringUtil

class TransientParameterisedHolderNode( maya.OpenMaya.MFnDependencyNode ) :

	def __init__( self, layoutName, classNameOrParameterised, classVersion=None, envVarName=None ) :
		""" Creates a temporary TransientParameterisedHolderNode in order to present the UI for the specified
		    parameterised object in the given layout. The node is automatically deleted when the holding
		    layout is destroyed """
		
		nodeName = maya.cmds.createNode( "ieTransientParameterisedHolderNode", skipSelect = True )
		
		# Script jobs aren't available from maya.cmds. Maya Python bindings generate swig warnings
		# such as "swig/python detected a memory leak of type 'MCallbackId *', no destructor found"
		IECoreMaya.mel( 'scriptJob -uiDeleted "%s" "delete %s" -protected' % ( layoutName, nodeName ) )
		
		object = StringUtil.dependencyNodeFromString( nodeName )
				
		maya.OpenMaya.MFnDependencyNode.__init__( self, object )
	
		if isinstance( classNameOrParameterised, str ) :
			res = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised, classVersion, envVarName )
		else :
			assert( not classVersion )
			assert( not envVarName )
		
			res = _IECoreMaya._parameterisedHolderSetParameterised( self, classNameOrParameterised )
			
		parameterised = self.getParameterised()[0]
		
		if parameterised: 
			maya.cmds.setParent( layoutName )
			IECoreMaya.ParameterUI.create( object, parameterised.parameters() )
			maya.cmds.setParent( layoutName )			
			
		return res		
				
	def getParameterised( self ) :
		
		return _IECoreMaya._parameterisedHolderGetParameterised( self )
