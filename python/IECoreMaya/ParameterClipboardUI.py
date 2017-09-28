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

from __future__ import with_statement

import IECore
import IECoreMaya

import maya.cmds

## The ParameterClipboardUI functions create menu items in relevant
## menus to allow the values of Parameterised object Parameters to be
## copied and pasted between nodes, or between different parameters on
## the same node. It currently does this using in-memory BasicPreset
## instances, held in a global variable.
## Pasting 'values' as a connection is also supported, though not using
## the basic preset mechanism.
__all__ = [ 'copy', 'copyClass', 'paste', 'pasteLinked', '_clearReferences' ]

def __copyPasteMenuModifier( menuDefinition, parameter, node, parent=None ) :

	with IECore.IgnoredExceptions( KeyError ) :
		if not parameter.userData()['UI']['copyPaste'].value :
			return

	global _ieCoreParameterClipboardUIBuffer

	fnPh = IECoreMaya.FnParameterisedHolder( node )
	plugPath = fnPh.parameterPlugPath( parameter )

	commandBase = 'import IECoreMaya; IECoreMaya.ParameterClipboardUI'

	copyString = '%s.copy( "%s" )' % ( commandBase, plugPath )
	copySomeString = '%s.copy( "%s", True )' % ( commandBase, plugPath )
	pasteString = '%s.paste( "%s" )' % ( commandBase, plugPath )
	pasteLinkedString = '%s.pasteLinked( "%s" )' % ( commandBase, plugPath )

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
	pasteLinkedActive = False

	if _ieCoreParameterClipboardUIBuffer and isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		pasteActive = _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter )
		if pasteActive and _ieCoreParameterClipboardUILastNode and maya.cmds.objExists( _ieCoreParameterClipboardUILastNode ) :
			pasteLinkedActive = True

	menuDefinition.append(
		"/Paste",
		{
			"command" : IECore.curry( maya.cmds.evalDeferred, pasteString ),
			"active" : pasteActive,
		}
	)

	menuDefinition.append(
		"/Paste Linked",
		{
			"command" : IECore.curry( maya.cmds.evalDeferred, pasteLinkedString ),
			"active" : pasteLinkedActive,
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
## These track the last node/parameters that were copied so we can potentially
## paste with connections. We still need the preset to be able to restore
## any missing classes contained within the preset.
_ieCoreParameterClipboardUILastParameterList = None
_ieCoreParameterClipboardUILastNode = None
_ieCoreParameterClipboardUILastRoot = None

def copy( plugPath, showUI=False ) :

	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot

	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )

	def copyCallback( node, rootParameter, parameters ):

		global _ieCoreParameterClipboardUIBuffer
		global _ieCoreParameterClipboardUILastParameterList
		global _ieCoreParameterClipboardUILastNode
		global _ieCoreParameterClipboardUILastRoot

		fnPh = IECoreMaya.FnParameterisedHolder( node )
		preset = IECore.BasicPreset( fnPh.getParameterised()[0], rootParameter, parameters=parameters )

		_ieCoreParameterClipboardUIBuffer = preset
		_ieCoreParameterClipboardUILastParameterList = parameters
		_ieCoreParameterClipboardUILastNode = node
		_ieCoreParameterClipboardUILastRoot = rootParameter

	# We need to make sure that the values in the parameterised are in sync
	fnPh.setParameterisedValues()

	if showUI :

		IECoreMaya.PresetsUI( parts[0], parameter ).selectParameters( copyCallback )

	else :

		paramList = []
		__getParameters( parameter, paramList )

		_ieCoreParameterClipboardUIBuffer = IECore.BasicPreset( fnPh.getParameterised()[0], parameter )
		_ieCoreParameterClipboardUILastParameterList = paramList
		_ieCoreParameterClipboardUILastNode = fnPh.fullPathName()
		_ieCoreParameterClipboardUILastRoot = parameter

def copyClass( plugPath, parentPlugPath ) :

	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot

	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )
	parent = fnPh.plugParameter( parentPlugPath )

	# We need to make sure that the values in the parameterised are in sync
	fnPh.setParameterisedValues()

	# This bit is slightly irritating, but we have to get all
	# The child parameters of the target class, so we only save that one
	paramList = []
	__getParameters( parameter, paramList )
	_ieCoreParameterClipboardUIBuffer = IECore.BasicPreset( fnPh.getParameterised()[0], parent, paramList )

	# For now, we only support pasting values linked. Otherwise, we'd have to go find
	# any classes we instantiated to know their plug prefix...
	_ieCoreParameterClipboardUILastParameterList = None
	_ieCoreParameterClipboardUILastNode = None
	_ieCoreParameterClipboardUILastRoot = None

def paste( plugPath ) :

	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot

	if not _ieCoreParameterClipboardUIBuffer or not isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		return

	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )

	if not _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter ):
		raise RuntimeError, "The parameters on the clipboard are not applicable to '%s'" % plugPath

	fnPh.setParameterisedValues()

	with fnPh.parameterModificationContext() :
		_ieCoreParameterClipboardUIBuffer( fnPh.getParameterised()[0], parameter )

def pasteLinked( plugPath ) :

	global _ieCoreParameterClipboardUIBuffer
	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot

	if not _ieCoreParameterClipboardUIBuffer or not isinstance( _ieCoreParameterClipboardUIBuffer, IECore.Preset ) :
		return

	parts = plugPath.split( "." )
	fnPh = IECoreMaya.FnParameterisedHolder( parts[0] )
	parameter = fnPh.plugParameter( plugPath )

	if not _ieCoreParameterClipboardUIBuffer.applicableTo( fnPh.getParameterised()[0], parameter ):
		raise RuntimeError, "The parameters on the clipboard are not applicable to '%s'" % plugPath

	# Apply the preset to make sure that the children are there
	fnPh.setParameterisedValues()

	with fnPh.parameterModificationContext() :
		_ieCoreParameterClipboardUIBuffer( fnPh.getParameterised()[0], parameter )

	# Connect up
	if not maya.cmds.objExists( _ieCoreParameterClipboardUILastNode ) :
		raise RuntimeError, "The source node '%s' no longer exists." % _ieCoreParameterClipboardUILastNode

	if not _ieCoreParameterClipboardUILastRoot :
		raise RuntimeError, "Unable to link, the source root parameter is not known." % _ieCoreParameterClipboardUILastNode


	sourceNodeHolder = IECoreMaya.FnParameterisedHolder( _ieCoreParameterClipboardUILastNode )
	sourceRootPlugPath = sourceNodeHolder.parameterPlugPath( _ieCoreParameterClipboardUILastRoot )

	if sourceRootPlugPath == plugPath :
		raise RuntimeError, "The source and destination parameters are the same, unable to link."

	for p in _ieCoreParameterClipboardUILastParameterList :
		sourcePlugPath = sourceNodeHolder.parameterPlugPath( p )
		destPlugPath = sourcePlugPath.replace( sourceRootPlugPath, plugPath )
		if maya.cmds.objExists( sourcePlugPath ) and maya.cmds.objExists( destPlugPath ) :
			maya.cmds.connectAttr( sourcePlugPath, destPlugPath, force=True )

def __getParameters( parameter, paramList=[] ) :

	if parameter.staticTypeId() == IECore.TypeId.CompoundParameter :

		for p in parameter.values():
			__getParameters( p, paramList )

	elif isinstance( parameter, IECore.ClassParameter ) :

		c = parameter.getClass( False )
		if c:
			paramList.append( parameter )
			__getParameters( c.parameters(), paramList )

	elif isinstance( parameter, IECore.ClassVectorParameter ) :

		cl = parameter.getClasses( False )
		if cl :
			paramList.append( parameter )
			for c in cl :
				__getParameters( c.parameters(), paramList )

	else :
		paramList.append( parameter )

# We need to clear out the references we're holding on parameters when the scene changes
def _clearReferences( *args, **kwargs ) :

	global _ieCoreParameterClipboardUILastParameterList
	global _ieCoreParameterClipboardUILastNode
	global _ieCoreParameterClipboardUILastRoot

	_ieCoreParameterClipboardUILastParameterList = None
	_ieCoreParameterClipboardUILastNode = None
	_ieCoreParameterClipboardUILastRoot = None

_ieCoreParameterClipboardCallbacks = []
if hasattr( maya.cmds, "about" ) and not maya.cmds.about( batch=True ):
	_ieCoreParameterClipboardCallbacks.append( IECoreMaya.CallbackId( maya.OpenMaya.MSceneMessage.addCallback( maya.OpenMaya.MSceneMessage.kBeforeNew, _clearReferences ) ) )
	_ieCoreParameterClipboardCallbacks.append( IECoreMaya.CallbackId( maya.OpenMaya.MSceneMessage.addCallback( maya.OpenMaya.MSceneMessage.kBeforeOpen, _clearReferences ) ) )

