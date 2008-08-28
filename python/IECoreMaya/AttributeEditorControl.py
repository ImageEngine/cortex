##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
import maya.mel
import maya.OpenMayaUI

## A base class to help in creating custom attribute editor controls
# in a nice object oriented manner.
## \todo Remove things from the __instances array when their layouts have
# been destroyed.
class AttributeEditorControl :
	
	## Derived classes should first call the base class __init__, before
	# building their ui.
	def __init__( self, attribute ) :
	
		self.__nodeName = attribute.split( "." )[0]
		self.__attributeName = attribute

	## Derived classes should first call the base class replace, before
	# reattaching their ui to the new attribute.
	def replace( self, attribute ) :
	
		self.__nodeName = attribute.split( "." )[0]
		self.__attributeName = attribute

	## Returns the name of the node this ui is used for.
	def nodeName( self ) :
	
		return self.__nodeName
		
	## Returns the name of the attribute this ui is used for.
	def attributeName( self ) :
	
		return self.__attributeName	

	@staticmethod
	def _new( className, attribute ) :
	
		# we smuggle the class name as a fake attribute name so we
		# need to get it back out now.
		className = ".".join( className.split( "." )[1:] )
		
		# the class name might also be in a namespace that isn't imported
		# in this scope. so import it.
		if not "." in className :
			cls = eval( className )
		else :
			names = className.split( "." )
			namespace = __import__( ".".join( names[:-1] ) )
			cls = getattr( namespace, names[-1] )
					
		parent = maya.cmds.setParent( q=True )
		control = cls( attribute )
		maya.cmds.setParent( parent )
		
		AttributeEditorControl.__instances[parent] = control
		
		# Script jobs aren't available from maya.cmds. Maya Python bindings generate swig warnings
		# such as "swig/python detected a memory leak of type 'MCallbackId *', no destructor found"
		maya.mel.eval( 'scriptJob -protected -uiDeleted "%s" "python \\"IECoreMaya.AttributeEditorControl._uiDeleted( \'%s\' )\\""' % ( parent, parent ) )
	
	@staticmethod
	def _replace( attribute ) :
	
		parent = maya.cmds.setParent( q=True )
		control = AttributeEditorControl.__instances[parent]
		control.replace( attribute )
	
	@staticmethod
	def _uiDeleted( parent ) :
	
		del AttributeEditorControl.__instances[parent]
		
	# Maps from parent ui names to AttributeEditorControl instances
	__instances = {}
