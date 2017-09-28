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

import nuke

## A context object intended for use with python's "with" block. It ensures
# that all operations in the block are performed with nuke's undo in a
# particular state (enabled or disabled) and that the previous state is correctly
# restored on exit from the block.
class UndoState :

	## state should be True to enable undo, and False to disable it.
	def __init__( self, state ) :

		self.__state = state

	def __enter__( self ) :

		self.__prevState = not nuke.Undo.disabled()
		if self.__state :
			nuke.Undo.enable()
		else :
			nuke.Undo.disable()

	def __exit__( self, type, value, traceBack ) :

		if self.__prevState :
			nuke.Undo.enable()
		else :
			nuke.Undo.disable()

## A context object intended for use with python's "with" block. It ensures
# that all operations in the block are performed with undo disabled, and that
# undo is reenabled if necessary upon exit from the block.
class UndoDisabled( UndoState ) :

	def __init__( self ) :

		UndoState.__init__( self, False )

## A context object intended for use with python's "with" block. It ensures
# that all operations in the block are performed with undo enabled, and that
# undo is disabled if necessary upon exit from the block.
class UndoEnabled( UndoState ) :

	def __init__( self ) :

		UndoState.__init__( self, True )

## A context object intended for use with python's "with" block. It groups a
# series of actions into a single Nuke undo.
class UndoBlock :

	def __enter__( self ) :

		nuke.Undo.begin()

	def __exit__( self, type, value, traceBack ) :

		nuke.Undo.end()

