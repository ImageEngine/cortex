##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

import _IECore as IECore
from IECore import registerObject

## This class implements a CompoundParameter that do not validate optional parameters if they are undefined.
# This CompoundParameter derived class allows one to set a group of obligatory parameters that should always
# be validated. The non listed parameters are treated as optional. Optional parameters are not validated if their
# value is NullObject.
# \ingroup python
class OptionalCompoundParameter( IECore.CompoundParameter ):

	## Constructor
	# Uses the same parameters as CompoundParameter
	def __init__( self, *args, **argv ):
		IECore.CompoundParameter.__init__( self, *args, **argv )
		self.__obligatoryParameterNames = None

	## Defines a list of parameter names that must be validated.
	# The non listed parameters are treated as
	# optional. That means they can be set to NullObject. The obligatoryParameterNames can also be None.
	# In that case the validation used is from CompoundParameter
	def setObligatoryParameterNames( self, obligatoryParameterNames = None ):
		self.__obligatoryParameterNames = set( obligatoryParameterNames )

	## Returns the list of obligatory parameters or None if this OptionalCompoundParameter is working as a regular CompoundParameter.
	def getObligatoryParameterNames( self ):
		return self.__obligatoryParameterNames

	## Undefines the given parameter.
	def setParameterUndefined( self, paramName ):
		self[ paramName ] = IECore.NullObject()

	## Returns True if the given attribute is undefined and False otherwise.
	def getParameterUndefined( self, paramName ):
		return isinstance( self[ paramName ].getValue(), IECore.NullObject )

	## Overwrites default validation method from CompoundParameter.
	# This function does not validate undefined parameters ( values equal to NullObject ) not listed in the
	# obligatory parameter list.
	def valueValid( self, value ) :

		if self.__obligatoryParameterNames is None:

			return IECore.CompoundParameter.valueValid( self, value )

		else:

			if not isinstance( value, IECore.CompoundObject ):
				return ( False, "Not a CompoundObject!" )

			missing = self.__obligatoryParameterNames.difference( value.keys() )
			if len( missing ) > 0:
				return ( False, "Keys missing in CompoundObject: " + ", ".join( missing ) + "." )

			for name in value.keys():

				param = self[ name ]
				paramValue = value[ name ]

				if name in self.__obligatoryParameterNames or not isinstance( paramValue, IECore.NullObject ):
					( valid, msg ) = param.valueValid( paramValue )
					if not valid:
						return ( False, ("Error in parameter %s: " % name) + msg )

		return (True, "")

	## Smart getattr
	# Tries to use original CompoundParameter.__getattr__. If it fails, then try the local object dictionary.
	def __getattr__( self, attrName ):
		try:
			return IECore.CompoundParameter.__getattr__( self, attrName )
		except:
			return self.__dict__[ attrName ]

	## Smart setattr
	# Tries to use original CompoundParameter.__getattr__. If it fails, then try the local object dictionary.
	def __setattr__( self, attrName, attrValue ):
		try:
			parameter = IECore.CompoundParameter.__getattr__( self, attrName )
		except:
			self.__dict__[ attrName ] = attrValue
		else:
			parameter.smartSetValue( attrValue )

registerObject( OptionalCompoundParameter, 100009, IECore.CompoundParameter )
