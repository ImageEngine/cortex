##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
from maya.OpenMaya import *
import re

from DAGPathParameter import DAGPathParameter

"""
Parameter class for specifying a list of Maya DAG paths.
"""
class DAGPathVectorParameter( IECore.StringVectorParameter ):

	class CheckType:
		
		DontCare = 0
		MustExist = 1
		MustNotExist = 2

	"""
	name - name of the parameter.
	description - description of the parameter.
	allowEmptyList - boolean that will accept the empty list even if the nodes should exist.
	check - can be CheckType.DontCare, CheckType.MustExist or CheckType.MustNotExist.
	typeRegex - regular expression used on parameter validation that validates based on the maya node type. Disable filtering using None.
	typeRegexDescription - human readable description for the regular expression used to filter node types. It's used when the validation fails.
	"""
	def __init__( self, name, description, defaultValue = IECore.StringVectorData(), allowEmptyList = True, 
		check = CheckType.DontCare, typeRegex = None, typeRegexDescription = "", 
		presets = {}, presetsOnly = False, userData = IECore.CompoundObject() ) :
		
		IECore.StringVectorParameter.__init__( self, name, description, defaultValue, presets, presetsOnly, userData )

		self.__allowEmptyList = allowEmptyList
		self.__mustExist = bool( check == DAGPathVectorParameter.CheckType.MustExist )
		self.__mustNotExist = bool( check == DAGPathVectorParameter.CheckType.MustNotExist )
		if typeRegex is None:
			self.typeRegex = None
		else:
			self.typeRegex = re.compile( typeRegex )
			if typeRegexDescription == "":
				self.typeRegexDesc = "Invalid type."
			else:
				self.typeRegexDesc = typeRegexDescription

	"""
	Defines two attributes: mustExist and mustNotExist and allowEmptyString exactly like PathParameter class.
	"""
	def __getattr__( self, attrName ):
		if attrName == "mustExist":
			return self.__mustExist
		elif attrName == "mustNotExist":
			return self.__mustNotExist
		elif attrName == "allowEmptyList":
			return self.__allowEmptyList
		else:
			return self.__dict__[ attrName ]

	"""
	Returns a regular expression that matches only valid Maya DAG paths.
	"""
	@staticmethod
	def pathValidator():
		return DAGPathParameter.pathValidator()

	"""
	Returns (True, "") only if the value is a correct DAG path string and also checks that the DAG node exists or doesn't exist 
	based on the CheckType passed to the constructor.
	Otherwise returns (False, errorMessage).
	"""
	def valueValid( self, value ) :
		
		v = IECore.StringVectorParameter.valueValid( self, value )
		if not v[0] :
			return v

		if len( value ) == 0 and not self.allowEmptyList:
			return False, "Empty list!"

		for item in value:

			if not self.pathValidator().match( item ) :
				return False, "%s is not a valid Maya DAG path." % item

			list = MSelectionList ()
			try:
				list.add( item )
			except:
				exist = False
			else:
				exist = True

				try:
					dp = MDagPath()
					list.getDagPath(0, dp)
					depNode = MFnDagNode( dp )
				except:
					IECore.debugException("failed to instantiate MDagPath from", item )
					return False, "'%s' is not a DAG node" % item

				if not self.typeRegex is None:
					nodeType = str(depNode.typeName())
					if self.typeRegex.match( nodeType ) is None:
						return False, ("Type '%s' not accepted: " % nodeType) + self.typeRegexDesc

			if self.mustExist :

				if not exist:
					return False, "DAG node %s does not exist" % item
			
			elif self.mustNotExist :

				if exist:
					return False, "DAG node %s already exists" % item

		return True, ""	

	"""
	Sets the internal VectorStringData value from the given MDagPath list
	"""	
	def setDAGPathVectorValue( self, dagNodeList ) :
		l = []
		for dagNode in dagNodeList:
			l.append( dagNode.fullPathName() )
		self.setValue( IECore.StringVectorData( l ) )
	
	"""
	Returns a list of MDagPath objects from the current selection.
	"""
	def getDAGPathVectorValue( self ) :
		dagNodePathList = self.getValue().value
		result = []
		for dagNodePath in dagNodePathList:
		
			try:
				list = MSelectionList()
				list.add( dagNodePath )
				dp = MDagPath()
				list.getDagPath(0, dp)
				result.append( dp )
			except:
				if self.mustExist :
					raise Exception, "Node '%s' does not exist!" % dagNodePath

		return result

IECore.makeRunTimeTyped( DAGPathVectorParameter, 350003, IECore.StringVectorParameter )
