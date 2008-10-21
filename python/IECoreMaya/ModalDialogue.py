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
	
	## Should be called by derived classes when they wish to close their dialogue.
	# The result will be returned to the caller of the run() method.
	def _dismiss( self, result ) :
	
		maya.cmds.layoutDialog( dismiss = result )
	
	## Call this method to open a dialogue - the return value is the string returned
	# by the dialogue in its _dismiss method.
	@classmethod
	def run( cls ) :
	
		ModalDialogue.__toInstantiate = cls
		title = maya.mel.eval( 'interToUI( "%s" )' % cls.__name__ )
		result = maya.cmds.layoutDialog( ui = 'python "import IECoreMaya; IECoreMaya.ModalDialogue._ModalDialogue__instantiate()"', title=title )
		return result
		
	@classmethod
	def __instantiate( cls ) :
	
		ModalDialogue.__currentDialogue = cls.__toInstantiate()
