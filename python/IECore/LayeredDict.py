##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

from IECore import CompoundObject, CompoundParameter

## This class takes a stack of dictionary like objects and provides
# read only access to them as if they were one, falling through to
# lower dictionaries if the requested item isn't in the ones above.
class LayeredDict :

	## Constructs a new layered dictionary :
	#
	# dicts : a list of dictionaries to layer. dicts[0] has
	# highest precedence and dicts[-1] lowest. after construction
	# this list can be accessed as LayeredDict.layers, and may be
	# modified in place to change the layering.
	#
	# dictClasses : a set of classes to consider as dictionary types.
	# this comes into play when layeredDict["key"] yields an object of
	# that type. in this case the object is not returned alone, but instead
	# a new LayeredDict of that object and any others with the same key
	# is returned - this allows the layering to continue in dictionaries held
	# within the topmost dictionary.
	def __init__( self, dicts, dictClasses = set( [ dict, CompoundObject, CompoundParameter ] ) ) :

		for d in dicts :
			assert( d.__class__ in dictClasses )

		self.layers = dicts

		assert( isinstance( dictClasses, set ) )
		self.__dictClasses = dictClasses

	def __getitem__( self, key ) :

		for i in range( 0, len( self.layers ) ) :

			if key in self.layers[i] :

				value = self.layers[i][key]
				if not value.__class__ in self.__dictClasses :

					return value

				else :

					# need to return a LayeredDict
					dicts = [ value ]
					for j in range( i + 1, len( self.layers ) ) :
						if key in self.layers[j] :
							dicts.append( self.layers[j][key] )

					return LayeredDict( dicts )

		raise KeyError( key )

	def __contains__( self, key ) :

		for i in range( 0, len( self.layers ) ) :

			if key in self.layers[i] :
				return True

		return False

	def keys( self ) :

		allKeys = set()
		for d in self.layers :

			allKeys.update( d.keys() )

		return list( allKeys )

	def get( self, key, defaultValue ) :

		try :
			return self.__getitem__( key )
		except KeyError :
			return defaultValue

