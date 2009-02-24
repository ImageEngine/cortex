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

import IECore
import IECoreMaya

import maya.cmds
import maya.mel
import maya.OpenMayaUI

## The UIElement base class assists in implementing ui elements in an
# object oriented manner in python.
## \todo Preexisting ui elements should be rewritten to take
# advantage of this base class wherever possible.
class UIElement :

	__instances = {}
	
	## Derived classes must create a ui element which is the parent of the
	# rest of their ui, and call this init function passing it as the topLevelUI
	# parameter. The base class will ensure that the UIElement instance is kept
	# alive as long as the ui exists, and that it will be destroyed when the
	# ui is destroyed.
	def __init__( self, topLevelUI ) :
	
		instanceRecord = IECore.Struct()
		instanceRecord.instance = self
		instanceRecord.callbacks = []
		instanceRecord.uiDeletedCallbackId = IECoreMaya.CallbackId( maya.OpenMayaUI.MUiMessage.addUiDeletedCallback( topLevelUI, self.__uiDeleted, topLevelUI ) )
		UIElement.__instances[topLevelUI] = instanceRecord
	
		self.__topLevelUI = topLevelUI
				
	## Returns the name of the top level ui element for this instance.
	def _topLevelUI( self ) :
	
		return self.__topLevelUI
	
	## It's very natural to assign member functions to ui callbacks. There is a problem
	# with this however in that maya leaks references to any python callbacks passed
	# to it (tested in maya 2008). This causes a reference to be held to the UIElement
	# instance forever. This could be a real problem if the ui references significant amounts
	# of memory, which it may well do. This class wraps a callback function in such a way
	# that maya will not leak it.
	#
	# Example usage : maya.cmds.button( command=self._createCallback( self.__buttonPressed ) )
	#
	# If the mel parameter is True then it causes the creation of a mel command to call back and
	# invoke the python callback. This is useful for the cases where maya insists on treating callbacks as
	# mel commands even when they're added from python - for instance the outlinerEditor selectCommand
	# in maya 2008.
	def _createCallback( self, function, mel=False ) :
	
		callbacks = self.__instances[self._topLevelUI()].callbacks
		callbacks.append( function )
		
		pythonCmd = "import IECoreMaya; IECoreMaya.UIElement._UIElement__invokeCallback( '%s', %d )" % ( self._topLevelUI(), len( callbacks ) - 1 )
		if not mel :
			return pythonCmd
		else :
			return "python( \"%s\" )" % pythonCmd
	
	## This is called when the maya ui element corresponding to this
	# instance is deleted. It may be reimplemented by derived classes
	# to perform any necessary cleanup. One item of cleanup that might be
	# very important is to delete any IECoreMaya.CallbackId objects that
	# may be linking a maya message to a method of the instance - if this
	# is not done then a circular reference will prevent the instance from
	# dying, and the callbacks will continue to be despatched despite the
	# fact that the ui has long gone.
	def _topLevelUIDeleted( self ) :
		
		pass
	
	@staticmethod
	def __uiDeleted( topLevelUI ) :
				
		UIElement.__instances[topLevelUI].instance._topLevelUIDeleted()
		del UIElement.__instances[topLevelUI]

	@staticmethod
	def __invokeCallback( topLevelUI, callbackIndex, *args ) :
				
		callback = UIElement.__instances[topLevelUI].callbacks[callbackIndex]
		callback( *args )
