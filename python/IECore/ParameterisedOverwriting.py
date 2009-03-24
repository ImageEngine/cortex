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

import warnings

import IECore

"""
Implement new methods to the Parameterised class.
"""

def __parameterisedSetItemOp( self, attrName, attrValue ):
	"""
	Smart version of __setitem__ and __setattr__ operator. Uses Parameter.smartSetValue() function for
	dynamic type convertion.
	"""
	try:
		parameters = IECore.Parameterised.parameters( self )
	except:
		# it's probably a derived class and the constructor did not initialized Parameterised.
		# So the attribute must be a class attribute and not a Parameter attribute.
		self.__dict__[ attrName ] = attrValue
	else:
		if parameters.has_key( attrName ):
			parameters[ attrName ].smartSetValue( attrValue )
		else:
			# allow assignment of other attributes to the object.
			self.__dict__[ attrName ] = attrValue

## \todo Remove this in major version 5.
def __parameterisedSetAttrOp( self, attrName, attrValue ):
	"""
	Smart version of __setitem__ and __setattr__ operator. Uses Parameter.smartSetValue() function for
	dynamic type convertion.
	"""
	try:
		parameters = IECore.Parameterised.parameters( self )
	except:
		# it's probably a derived class and the constructor did not initialized Parameterised.
		# So the attribute must be a class attribute and not a Parameter attribute.
		self.__dict__[ attrName ] = attrValue
	else:
		if parameters.has_key( attrName ):
			parameters[ attrName ].smartSetValue( attrValue )
			warnings.warn( "Access to Parameters as attributes is deprecated - please use item style access instead.", DeprecationWarning, 2 )
		else:
			# allow assignment of other attributes to the object.
			self.__dict__[ attrName ] = attrValue	

IECore.Parameterised.__setitem__ = __parameterisedSetItemOp
IECore.Parameterised.__setattr__ = __parameterisedSetAttrOp

__all__ = []
