##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

## This class provides a useful base for implementing modal dialogues in an
# object oriented manner.
class ModalDialogue( IECoreMaya.UIElement ) :

	## Derived classes should call the base class __init__ before constructing
	# their ui. The result of self._topLevelUI() will be a form layout in which
	# the ui should be constructed. Clients shouldn't construct ModalDialogues
	# directly but instead use the run() method documented below.
	def __init__( self ) :

		IECoreMaya.UIElement.__init__( self, maya.cmds.setParent( query=True ) )
		ModalDialogue.__returnObject = None;

	## Should be called by derived classes when they wish to close their dialogue.
	# The result will be returned to the caller of the run() method. This differs
	# from the default maya behaviour for a layoutDialog, as we allow any python
	# type to be returned. If the dialog is dismissed directly with the maya.cmd
	# then that value will be returned instead.
	def _dismiss( self, result ) :

		ModalDialogue.__returnObject = result
		maya.cmds.layoutDialog( dismiss = 'ModalDialogueReturnObject' )

	## Call this method to open a dialogue - the return value is the object supplied
	# by the derived class, or the string supplied to layoutDialog( dismiss=%s ) if
	# called directly. If the string 'None', 'True', or 'False' is passed to a
	# direct call, then the equivalent python object is returned instead of a string.
	@classmethod
	def run( cls, *args, **kwargs ) :

		ModalDialogue.__toInstantiate = cls
		ModalDialogue.__calltimeArgs = args
		ModalDialogue.__calltimeKwargs = kwargs

		title = maya.mel.eval( 'interToUI( "%s" )' % cls.__name__ )
		mayaResult = maya.cmds.layoutDialog( ui = 'import IECoreMaya; IECoreMaya.ModalDialogue._ModalDialogue__instantiate()', title = title )

		if mayaResult == 'ModalDialogueReturnObject' :

			stopMeLeaking = ModalDialogue.__returnObject
			ModalDialogue.__returnObject = None
			return stopMeLeaking

		elif mayaResult == 'None' :
			return None

		elif mayaResult == 'True' :
			return True

		elif mayaResult == 'False' :
			return False

		return mayaResult

	@classmethod
	def __instantiate( cls ) :

		ModalDialogue.__currentDialogue = cls.__toInstantiate( *ModalDialogue.__calltimeArgs, **ModalDialogue.__calltimeKwargs )
		ModalDialogue.__calltimeArgs = None
		ModalDialogue.__calltimeKwargs = None

