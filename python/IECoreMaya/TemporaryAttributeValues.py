##########################################################################
#
#  Copyright (c) 2009-2012, Image Engine Design Inc. All rights reserved.
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

import maya.cmds
import maya.OpenMaya

import IECore

import StringUtil

## A context manager for controlling attribute values in with statements. It
# sets attributes to requested values on entering the block and resets them to
# their previous values on exiting the block.
class TemporaryAttributeValues :

	def __init__( self, attributeAndValues = {}, **kw ) :

		self.__attributesAndValues = attributeAndValues
		self.__attributesAndValues.update( kw )

	def __enter__( self ) :

		handlers = {
			"enum" : self.__simpleAttrHandler,
			"bool" : self.__simpleAttrHandler,
			"float" : self.__simpleAttrHandler,
			"long" : self.__simpleAttrHandler,
			"short" : self.__simpleAttrHandler,
			"float2" : IECore.curry( self.__numeric2AttrHandler, attributeType="float2" ),
			"long2" : IECore.curry( self.__numeric2AttrHandler, attributeType="long2" ),
			"short2" : IECore.curry( self.__numeric2AttrHandler, attributeType="short2" ),
			"float3" : IECore.curry( self.__numeric3AttrHandler, attributeType="float3" ),
			"long3" : IECore.curry( self.__numeric3AttrHandler, attributeType="long3" ),
			"short3" : IECore.curry( self.__numeric3AttrHandler, attributeType="short3" ),
			"string" : self.__stringAttrHandler,
		}

		self.__restoreCommands = []
		for attr, value in self.__attributesAndValues.items() :

			# check we can handle this type
			attrType = maya.cmds.getAttr( attr, type=True )
			handler = handlers.get( attrType, None )
			if not handler :
				raise TypeError( "Attribute \"%s\" has unsupported type \"%s\"." % ( attr, attrType ) )

			# store a command to restore the attribute value later
			origValue = maya.cmds.getAttr( attr )
			if isinstance( origValue, list ) and isinstance( origValue[0], tuple ) :
				origValue = origValue[0]

			self.__restoreCommands.append( IECore.curry( handler, attr, origValue ) )

			# and change the attribute value
			handler( attr, value )

	def __exit__( self, type, value, traceBack ) :

		for cmd in self.__restoreCommands :

			cmd()

	def __simpleAttrHandler( self, attr, value ) :

		maya.cmds.setAttr( attr, value )

	def __numeric2AttrHandler( self, attr, value, attributeType ) :

		maya.cmds.setAttr( attr, value[0], value[1], type=attributeType )

	def __numeric3AttrHandler( self, attr, value, attributeType ) :

		maya.cmds.setAttr( attr, value[0], value[1], value[2], type=attributeType )

	def __stringAttrHandler( self, attr, value ) :

		maya.cmds.setAttr( attr, value, type="string" )
