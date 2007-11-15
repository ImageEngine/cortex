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
from _IECoreMaya import *
import re

"""
Parameter class for specifying Maya DAG paths.
"""
class DAGPathParameter( IECore.StringParameter ):

	class CheckType:
		
		DontCare = 0
		MustExist = 1
		MustNotExist = 2

	"""
	name - name of the parameter.
	description - description of the parameter.
	allowEmptyString - boolean that will accept the empty value even if the node should exist.
	check - can be CheckType.DontCare, CheckType.MustExist or CheckType.MustNotExist.
	typeRegex - regular expression used on parameter validation that validates based on the maya node type. Disable filtering using None.
	typeRegexDescription - human readable description for the regular expression used to filter node types. It's used when the validation fails.
	"""
	def __init__( self, name, description, defaultValue = "", allowEmptyString = True, 
		check = CheckType.DontCare, typeRegex = None, typeRegexDescription = "", presets = {}, presetsOnly = False, userData = IECore.CompoundObject() ) :
		
		IECore.StringParameter.__init__( self, name, description, defaultValue, presets, presetsOnly, userData )

		self.__allowEmptyString = allowEmptyString
		self.__mustExist = bool( check == DAGPathParameter.CheckType.MustExist )
		self.__mustNotExist = bool( check == DAGPathParameter.CheckType.MustNotExist )
		if typeRegex is None:
			self.__typeRegex = None
		else:
			self.__typeRegex = re.compile( typeRegex )
			if typeRegexDescription == "":
				self.__typeRegexDesc = "Invalid type."
			else:
				self.__typeRegexDesc = typeRegexDescription

	"""
	Defines two attributes: mustExist and mustNotExist and allowEmptyString exactly like PathParameter class.
	"""
	def __getattr__( self, attrName ):
		if attrName == "mustExist":
			return self.__mustExist
		elif attrName == "mustNotExist":
			return self.__mustNotExist
		elif attrName == "allowEmptyString":
			return self.__allowEmptyString
		else:
			return self.__dict__[ attrName ]

	"""
	Returns a regular expression that matches only valid Maya DAG paths.
	"""
	@staticmethod
	def pathValidator():
		return re.compile( "^(\|?[^\t\n\r\f\v\|]+)+\|?$" )

	"""
	Returns (True, "") only if the value is a correct DAG path string and also checks that the DAG node exists or doesn't exist 
	based on the CheckType passed to the constructor.
	Otherwise returns (False, errorMessage).
	"""
	def valueValid( self, value ) :
	
		v = IECore.StringParameter.valueValid( self, value )
		if not v[0] :
			return v

		if self.allowEmptyString and value.value=="" :
			return True, ""

		if not self.pathValidator().match( value.value ) :
			return False, "Not a valid Maya DAG path."

		try:
			node = DagNode( value.value )
			node.fullPathName()
		except:
			IECore.debugException("failed to instantiate DagNode from", value.value )
			exist = False
		else:
			exist = True

			if not self.__typeRegex is None:
				nodeType = node.typeName()
				if self.__typeRegex.match( nodeType ) is None:
					return False, ("Type '%s' not accepted: " % nodeType) + self.__typeRegexDesc

		if self.mustExist :

			if not exist:
				return False, "DAG node %s does not exist" % value.value
			
		elif self.mustNotExist :

			if exist:
				return False, "DAG node %s already exists" % value.value

		return True, ""	

	"""
	Sets the internal StringData value to node.name
	"""	
	def setDAGPathValue( self, dagNode ) :
		self.setValue( IECore.StringData( dagNode.fullPathName() ) )
	
	"""
	Gets the internal StringData value and creates a IECoreMaya.DagNode
	from it. Note that this can return None	if check is DontCare and 
	no matching node exists in Maya.
	"""
	def getDAGPathValue( self ) :
		dagNodePath = self.getValidatedValue().value
		dagNode = DagNode( dagNodePath )
		return dagNode

IECore.makeRunTimeTyped( DAGPathParameter, 350001, IECore.StringParameter )
