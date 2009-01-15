##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

## Creates a new class which provides enum-like functionality.
# The class has an attribute for each name passed, whose value is
# the instance of the Enum for that name. Enum instances hold an 
# integer value which can be retrieved using int( e ), and the
# name that value signifies can be retrieved using str( e ). Enum
# instances can be compared for equality and hashed to allow their use
# in dictionaries etc.
#
# For example :
#
# > E = IECore.Enum.create( "Apple", "Orange" )
# > a = E.Apple
# > print a, int( a )
#
# Apple, 0
#
# assert( E.Orange == E( 1 ) )
# assert( E.Orange != E.Apple )
#
# \todo Add a classmethod to return a tuple of all values.
def create( *names ) :

	class Enum :

		__slots__ = ( "__value" )
		__names = names

		def __init__( self, value ) :

			if value < 0 or value >= len( Enum.__names ) :
				raise ValueError( "Enum value out of range." )
				
			self.__value = value

		def __hash__( self ) :
		
			return hash( self.__value )

		def __cmp__( self, other ) :

			assert( type( self ) is type( other ) )
			return cmp( self.__value, other.__value )
			
		def __int__( self ) :
		
			return self.__value

		def __str__( self ) :

			return Enum.__names[self.__value]

	for i, name in enumerate( names ) :

		setattr( Enum, name, Enum( i ) )

	return Enum
