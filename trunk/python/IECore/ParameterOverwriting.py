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

import IECore

"""
Implement new methods to the Parameter and CompoundParameter classes.
"""

def __parameterSmartSetValue( self, value ):
	"""
	Smart setValue operator for Parameter objects. Uses introspection on the given value to define
	how the value will be assigned to the Parameter object.
	"""
	if isinstance( value, IECore.Object ):
		self.setValue( value )

	elif hasattr( self, "setTypedValue" ):
		self.setTypedValue( value )

	else:
		raise TypeError, "Invalid parameter type"

def __compoundParameterSmartSetValue( self, value ):
	"""
	Smart setValue operator for CompoundParameter objects. Uses introspection on the given value to define
	how the value will be assigned to the CompoundParameter object. 
	"""

	if isinstance( value, IECore.CompoundObject ):
		self.setValue( value )

	elif isinstance( value, dict ):
		for ( n, v ) in value.items():
			self[ n ].smartSetValue( v )

	else:
		raise TypeError, "Invalid parameter type"

def __compoundParameterSetItem( self, attrName, attrValue ):
	"""
	Smart __setattr__/__setitem__ operator. Uses copyFrom function for type convertion.
	"""
	self[ attrName ].smartSetValue( attrValue )

# expand class definitions
IECore.Parameter.smartSetValue = __parameterSmartSetValue
IECore.CompoundParameter.smartSetValue = __compoundParameterSmartSetValue
IECore.CompoundParameter.__setattr__ = __compoundParameterSetItem
IECore.CompoundParameter.__setitem__ = __compoundParameterSetItem

__all__ = []
