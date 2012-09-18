
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

			return hash( ( self.__class__, self.__value ) )

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
