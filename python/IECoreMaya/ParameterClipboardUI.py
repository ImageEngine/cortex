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
import IECoreMaya

import maya.cmds

## The ParameterClipboardUI functions create menu items in relevent
## meus to allow the values of Parameterised object Parameters to be
## copied and pasted between nodes, or between different paramters on
## the same node. It currently does this using in-memory BasicPreset
## instances, held in a global variable. 

__all__ = [ 'copy', 'copyClass', 'paste', '_ieCoreParameterClipboardUIBuffer' ]

def __copyPasteMenuModifier( menuDefinition, parameter, node, parent=None ) :
		
	global _ieCoreParameterClipboardUIBuffer
	
	fnPh = IECoreMaya.FnParameterisedHolder( node )
	plugPath = fnPh.parameterPlugPath( parameter )
	
	commandBase = 'import IECoreMaya; IECoreMaya.ParameterClipboardUI'
	
	copyString = '%s.copy( "%s" )' % ( commandBase, plugPath ) 
	copySomeString = '%s.copy( "%s", True )' % ( commandBase, plugPath )
	pasteString = '%s.paste( "%s" )' % ( commandBase, plugPath ) 
	
	if len( menuDefinition.items() ):
		menuDefinition.append( "/CopyPasteDivider", { "divider" : True } )

	copyActive = True
	
	if isinstance( parameter, IECore.ClassParameter ) :
		if parameter.getClass( False ) == None :
			copyActive = False
			
	elif isinstance( parameter, IECore.ClassVectorParameter ) :
		classes = parameter.getClasses( False )
		if not len(classes) :
			copyActive = False

	# If we are actually a class in a vector, also give them an option
	# to copy the class
	if parent is not None and (
		 isinstance( parent, IECore.ClassVectorParameter )
		 or isinstance( parent, IECore.ClassParameter )
	):
		menuDefinition.append( 
			"/Copy Values",
			{
				"command" : IECore.curry( maya.cmds.evalDeferred, copyString ),		
				"secondaryCommand" : IECore.curry( maya.cmds.evalDeferred, copySomeString ),
				"active" : copyActive,
			}
		)
		
		parentPlugPath = fnPh.parameterPlugPath( parent )
		copyComponentString = '%s.copyClass( "%s", "%s" )' % ( commandBase, plugPath, parentPlugPath ) 

		menuDefinition.append( 
		"/Copy Component",
		{
			"command" : IECore.curry( maya.cmds.evalDeferred, copyComponentString ),		
			"active" : copyActive,
		}
	)
	
	else:
	
		menuDefinition.append( 
			"/Copy",
			{
				"command" : IECore.curry( maya.cmds.evalDeferred, copyString ),		
				"secondaryCommand" : IECore.curry( maya.cmds.evalDeferred, copySomeString ),
				"active" : copyActive,
			}
		)
	
	pasteActive = False

	if _ieCoreParameterClipboardUIBuffer and isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		pasteActive = _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter )
		
	menuDefinition.append( 
		"/Paste",
		{
			"command" : IECore.curry( maya.cmds.evalDeferred, pasteString ),		
			"active" : pasteActive,
		}	
	)

def __copyPasteVectorMenuModifier( menuDefinition, parent, parameter, node ) :
	__copyPasteMenuModifier( menuDefinition, parameter, node, parent=parent )

IECoreMaya.ParameterUI.registerPopupMenuCallback( __copyPasteMenuModifier )
IECoreMaya.ClassParameterUI.registerClassMenuCallback( __copyPasteMenuModifier )
IECoreMaya.ClassVectorParameterUI.registerClassMenuCallback( __copyPasteVectorMenuModifier )
IECoreMaya.ClassVectorParameterUI.registerToolsMenuCallback( __copyPasteMenuModifier )

## The copy buffer
_ieCoreParameterClipboardUIBuffer = None

def copy( plugPath, showUI=False ) :

	global _ieCoreParameterClipboardUIBuffer

	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )

	def copyCallback( preset ):
		global _ieCoreParameterClipboardUIBuffer
		_ieCoreParameterClipboardUIBuffer = preset

	if showUI :

		IECoreMaya.PresetsUI( parts[0], parameter ).copy( copyCallback )
	
	else :
	
		_ieCoreParameterClipboardUIBuffer = IECore.BasicPreset( fnPh.getParameterised()[0], parameter )

def copyClass( plugPath, parentPlugPath ) :

	global _ieCoreParameterClipboardUIBuffer

	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )
	parent = fnPh.plugParameter( parentPlugPath )

	# This bit is slightly irritating, but we have to get all 
	# The child parameters of the target class, so we only save that one
	paramList = []
	__getClassParameters( parameter, paramList )

	_ieCoreParameterClipboardUIBuffer = IECore.BasicPreset( fnPh.getParameterised()[0], parent, paramList )


def paste( plugPath ) :

	global _ieCoreParameterClipboardUIBuffer
	
	if not _ieCoreParameterClipboardUIBuffer or not isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		return
		
	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )
	
	if not _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter ):
		raise RuntimeError, "The parameters on the clipboard are not applicable to '%s'" % plugPath
	
	with fnPh.classParameterModificationContext() : 
		_ieCoreParameterClipboardUIBuffer( fnPh.getParameterised()[0], parameter )
	
	fnPh.setNodeValues()


def __getClassParameters( parameter, paramList=[] ) :

	if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :
		for p in parameter.values():
			__getClassParameters( p, paramList )
	elif isinstance( parameter, IECore.ClassParameter ) :
		
		c = parameter.getClass( False )
		if c:
			__getClassParameters( c.parameters(), paramList )
		
	elif isinstance( parameter, IECore.ClassVectorParameter ) :
		
		cl = parameter.getClasses( False )
		if cl :
			for c in cl :
				__getClassParameters( c.parameters(), paramList )
	
	else :
		paramList.append( parameter )
	
	


