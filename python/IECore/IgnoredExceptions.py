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

## A context object intended for use with python's "with" syntax. It is used
# to replace this idiom :
#
# try :
#	value = someThing["with"]["keys"]["that"]["may"]["not"]["exist"]
# except KeyError :
#	pass
#
# with something slightly more concise and expressive of what is happening :
#
# with IgnoredExceptions( KeyError ) :
#	value = someThing["with"]["keys"]["that"]["may"]["not"]["exist"]
#
class IgnoredExceptions :

	## Accepts a variable number of exception types - these will be silently
	# ignored if they are thrown from within the body of the block.
	def __init__( self, *args ) :

		self.__toIgnore = args

	def __enter__( self ) :

		pass

	def __exit__( self, type, value, traceBack ) :

		if isinstance( value, self.__toIgnore ) :
			# Remove circular references which would cause everything in the
			# calling stack frame to exist far longer than necessary.
			value.__traceback__ = None
			return True

		if type is not None and issubclass( type, self.__toIgnore ) :
			return True
