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

import hou

## A context object intended for use with python's "with" syntax. It ensures
# that all operations in the with block are performed in the given take,
# and that the previous take is restored if it still exists when the block exits.
class ActiveTake :

	def __init__( self, take ) :

		self.__take = take
		self.__prevTake = ActiveTake.name()

	def __enter__( self ) :

		if self.__take in ActiveTake.ls() :
			hou.hscript( "takeset %s" % self.__take )

	def __exit__( self, type, value, traceBack ) :

		if self.__prevTake in ActiveTake.ls() :
			hou.hscript( "takeset %s" % self.__prevTake )

	## \todo: remove this method when the hscript take commands are available in python
	@staticmethod
	def name() :

		return hou.hscript( "takeset" )[0].strip()

	## \todo: remove this method when the hscript take commands are available in python
	@staticmethod
	def ls() :

		return [ x.strip() for x in hou.hscript( "takels" )[0].strip().split( "\n" ) ]
