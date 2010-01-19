##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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
import maya.OpenMaya
import StringUtil

## This class extends Maya's MFnDagNode to add assorted helper functions.
class FnDagNode( maya.OpenMaya.MFnDagNode ) :

	## \param obj - MObject, This can also be a string or an MObjectHandle.
	def __init__( self, obj ) :
	
		if isinstance( obj, str ) or isinstance( obj, unicode ) :
		
			obj = StringUtil.dependencyNodeFromString( obj )
		
		elif isinstance( obj, maya.OpenMaya.MObjectHandle ) :
		
			assert( obj.isValid() )
			obj = obj.object()		

		maya.OpenMaya.MFnDagNode.__init__( self, obj )

	## Determines whether the DAG node is actually hidden in Maya.
	# This includes the effect of any parents visibility.
	# \return Bool
	def isHidden( self ) :

		return bool( self.hiddenPathNames( True ) )

	## Retrieves the names of any part of the nodes parent hierarchy that is hidden.
	# \param includeSelf - Bool, When True, the object itself will be listed if
	# it is hidden. When False, only parents will be listed. Defaults to True.
	# \return A list of hidden objects by name.
	def hiddenPathNames( self, includeSelf=True, o=None, hidden=None ) :

		if not o :
			o = self.name()
			
		if not isinstance( hidden, list ) :
			hidden = []

		if includeSelf :
			attr = "%s.visibility" % o
			if maya.cmds.objExists( o ) and not maya.cmds.getAttr( attr ) :
				hidden.append( o )

		parents = maya.cmds.listRelatives( o, parent=True )
		if parents :
			for p in parents :
				self.hiddenPathNames( True, p, hidden )

		return hidden



