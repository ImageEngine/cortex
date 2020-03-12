##########################################################################
#
#  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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

import six
import string

import IECore

## Acts like a dictionary, but performs substitutions on any string or StringData values
# retrieved from it. Substitutions are specified as a dictionary where keys are tokens to
# be substituted and values are the corresponding substitutions. Substitution is performed
# using string.Template().
class SubstitutedDict :

	def __init__( self, dict, substitutions, dictClasses = set( [ dict, IECore.CompoundObject, IECore.CompoundParameter, IECore.LayeredDict ] ) ) :

		self.__dict = dict
		self.__substitutions = substitutions
		self.__dictClasses = dictClasses

	def __getitem__( self, key ) :

		value = self.__dict[key]

		if value.__class__ in self.__dictClasses :
			return SubstitutedDict( value, self.__substitutions, self.__dictClasses )

		if isinstance( value, six.string_types ) :
			return string.Template( value ).safe_substitute( self.__substitutions )
		elif isinstance( value, IECore.StringData ) :
			return IECore.StringData( string.Template( value.value ).safe_substitute( self.__substitutions ) )

		return value

	def __contains__( self, key ) :

		return self.__dict.__contains__( key )

	def __eq__( self, other ) :

		if not isinstance( other, SubstitutedDict ) :
			return False

		return (	self.__dict == other.__dict and
					self.__substitutions == other.__substitutions and
					self.__dictClasses == other.__dictClasses 	)

	def __ne__( self, other ) :

		return not self.__eq__( other )

	def keys( self ) :

		return self.__dict.keys()

	def values( self, substituted=True ) :

		if substituted :
			return [ self.get( k ) for k in self.__dict.keys() ]
		else :
			return self.__dict.values()

	def items( self, substituted=True ) :

		return zip( self.keys(), self.values() )

	def get( self, key, defaultValue=None, substituted=True ) :

		try :
			if substituted :
				return self.__getitem__( key )
			else :
				return self.__dict[key]
		except KeyError :
			return defaultValue

	def substitutions( self ) :

		return self.__substitutions
