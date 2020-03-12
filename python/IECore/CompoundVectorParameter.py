##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

## This class is a CompoundParameter that only accepts vector parameters with the same length.
# \ingroup python

class CompoundVectorParameter ( IECore.CompoundParameter ):

	def __testParameterType( self, parameter ):
		data = parameter.getValue()
		if not IECore.isSequenceDataType( data ):
			raise TypeError( "The parameter %s cannot be added because it does not hold vector data object." % parameter.name )

	# overwrites base class definition just to limit the parameter types accepted.
	def addParameter( self, parameter ):
		self.__testParameterType( parameter )
		IECore.CompoundParameter.addParameter( self, parameter )

	# overwrites base class definition just to limit the parameter types accepted.
	def addParameters( self, parameters ):
		for parameter in parameters:
			self.__testParameterType( parameter )
		IECore.CompoundParameter.addParameters( self, parameters )

	# overwrites base class definition just to limit the parameter types accepted.
	def insertParameter( self, parameter, other ):
		self.__testParameterType( parameter )
		IECore.CompoundParameter.insertParameter( self, parameter, other )

	## Returns true only if all the vector parameters are of the same length and they also validate ok.
	def valueValid( self, value ) :

		res = IECore.CompoundParameter.valueValid( self, value )
		if not res[0]:
			return res

		size = None
		keys = value.keys()
		values = value.values()
		for i in range( 0, len( keys ) ) :

			thisSize = len( values[i] )
			if size is None:
				size = thisSize

			if size != thisSize :
				return ( False, ( "Parameter \"%s\" has wrong size ( expected %d but found %d )" % ( keys[i], size, thisSize ) ) )

		return ( True, "" )

IECore.registerRunTimeTyped( CompoundVectorParameter )
