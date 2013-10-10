##########################################################################
#
#  Copyright (c) 2008-9, Image Engine Design Inc. All rights reserved.
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
import re

__all__ = [ "dependencyNodeFromString", "dagPathFromString", "plugFromString" ]

## Utility function to return a dependency node as an MObject when
# given it's name as a string.
def dependencyNodeFromString( s ) :

	sl = maya.OpenMaya.MSelectionList()
	sl.add( s )

	if sl.length() > 1 :
		IECore.msg( IECore.Msg.Level.Warning, "IECoreMaya.dependencyNodeFromString", "Name \"%s\" is not unique." % s )

	result = maya.OpenMaya.MObject()
	sl.getDependNode( 0, result )
	return result

## Utility function to return the parent string when
# given a UI or DAG node's full path name as a string.
def parentFromString( s ) :

	tokens = s.split('|')
	tokens.pop()
	parent = '|'.join( tokens )

	return parent

## Utility function to return an MDagPath when
# given it's name as a string.
def dagPathFromString( s ) :

	sl = maya.OpenMaya.MSelectionList()
	sl.add( s )

	if sl.length() > 1 :
		IECore.msg( IECore.Msg.Level.Warning, "IECoreMaya.dagPathFromString", "Name \"%s\" is not unique." % s )

	result = maya.OpenMaya.MDagPath()
	sl.getDagPath( 0, result )
	return result

## Utility function to return an MPlug when
# given it's name as a string.
def plugFromString( s ) :

	sl = maya.OpenMaya.MSelectionList()
	sl.add( s )

	if sl.length() > 1 :
		IECore.msg( IECore.Msg.Level.Warning, "IECoreMaya.plugFromString", "Name \"%s\" is not unique." % s )

	result = maya.OpenMaya.MPlug()
	sl.getPlug( 0, result )
	return result

## Returns a full path to an MPlug.
def pathFromPlug( p ) :

	try :
		f = maya.OpenMaya.MFnDagNode( p.node() )
		nodePath = f.fullPathName()
	except :
		f = maya.OpenMaya.MFnDependencyNode( p.node() )
		nodePath = f.name()

	return nodePath + "." + p.partialName()

## Extracts the node name from a full path to an attribute.
def nodeFromAttributePath( a ) :

	return re.match( "^[^.]*", a ).group( 0 )
