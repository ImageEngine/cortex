##########################################################################
#
#  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

import traceback

import maya.cmds

import IECore
import IECoreMaya

class CompoundParameterUI( IECoreMaya.ParameterUI ) :

	_collapsedUserDataKey = "aeCollapsed"

	## Supports the following keyword arguments :
	#
	# bool "withCompoundFrame"
	# May be specified as True to force the creation of a
	# frameLayout even when this parameter is the toplevel parameter
	# for the node.
	#
	# list "visibleOnly"
	# A list of strings specifying the full parameter paths for
	# parameters which should be displayed. Any parameters not in
	# this list will not be visible.
	#
	# bool "labelVisible"
	# If not None, specifies whether or not the parameter label is visible. This
	# is used by the ClassVectorParameterUI ChildUI class.
	def __init__( self, node, parameter, labelVisible=None, **kw  ) :

		self.__childUIs = {}
		self.__headerCreated = False
		self.__kw = kw.copy()

		self.__kw["hierarchyDepth"] = self.__kw.get( "hierarchyDepth", -1 ) + 1

		originalParent = maya.cmds.setParent( query=True )

		collapsible = self.__parameterIsCollapsible( node, parameter )
		collapsed = self._retrieveCollapsedState( collapsible, parameter )

		# we always use a Collapsible ui to hold our children, and just hide
		# the header if we don't actually want to collapse it ever.

		self.__collapsible = IECoreMaya.Collapsible(

			# need to specify a label on creation or maya gets the size wrong.
			# we'll update the label below, once we can call the base class label() method.
			label = "mustSpecifySomething",
			labelFont = self._labelFont( self.__kw["hierarchyDepth"] ),
			labelIndent = self._labelIndent( self.__kw["hierarchyDepth"] ),
			labelVisible = labelVisible if labelVisible is not None else collapsible,
			collapsed = collapsed,
			expandCommand = self.__expand,
			preExpandCommand = self.__preExpand,
			collapseCommand = self.__collapse,

		)

		IECoreMaya.ParameterUI.__init__( self,

			node,
			parameter,
			# stealing someone else's top level ui for use as your own is really breaking the rules.
			# but we need to do it to reduce the nesting associated with making a new top level to put
			# the Collapsible class in, because otherwise maya 2010 will crash with deeply nested
			# hierarchies. we could stop doing this when we no longer need maya 2010 support.
			self.__collapsible._topLevelUI(),
			**kw

		)

		self.__collapsible.setLabel( self.label() )
		self.__collapsible.setAnnotation( self.description() )

		self.__columnLayout = maya.cmds.columnLayout(
			parent = self.__collapsible.frameLayout(),
			width = 381
		)

		if not collapsed :
			self.__preExpand()

		maya.cmds.setParent( originalParent )

	def replace( self, node, parameter ) :

		IECoreMaya.ParameterUI.replace( self, node, parameter )

		if self.__parameterIsCollapsible() :
			collapsed = self._retrieveCollapsedState( self.getCollapsed() )
			self.setCollapsed( collapsed, **self.__kw )

		if len( self.__childUIs ) :

			for pName in self.__childUIs.keys() :

				ui = self.__childUIs[pName]
				p = self.parameter[pName]

				ui.replace( node, p )

		else :

			if not self.getCollapsed() :
				with IECoreMaya.UITemplate( "attributeEditorTemplate" ) :
					self.__createChildUIs()

	## Gets the collapsed state for the frame holding the child parameter uis.
	def getCollapsed( self ) :

		return self.__collapsible.getCollapsed()

	## Sets the collapsed state for the frame holding the child parameter uis.
	# \param propagateToChildren How many levels of hierarchy to propagate
	# the new state to. If a Bool is passed, rather than an int, then
	# 'all' or 'none' is assumed, for backwards compatibility.
	def setCollapsed( self, collapsed, propagateToChildren=0, **kw ) :

		if type(propagateToChildren) == bool :
			propagateToChildren = 999 if propagateToChildren else 0

		if not collapsed :
			# maya only calls preexpand when the ui is expanded by user action,
			# not by a script - how annoying.
			self.__preExpand()

		if self.__parameterIsCollapsible() :
			self.__collapsible.setCollapsed( collapsed )

		self._storeCollapsedState( collapsed )

		if propagateToChildren > 0 :
			propagateToChildren = propagateToChildren - 1
			self.__propagateCollapsed( collapsed, propagateToChildren, **kw )

	# This will retrieve the collapsedState from the parameters userData. It uses the
	# default key if 'collapsedUserDataKey' was not provided in the UI constructor's **kw.
	def _retrieveCollapsedState( self, default=True, parameter=None ) :

		if parameter is None :
			parameter = self.parameter

		key = self.__kw.get( "collapsedUserDataKey", CompoundParameterUI._collapsedUserDataKey )
		if "UI" in parameter.userData() and key in parameter.userData()["UI"] :
			return parameter.userData()["UI"][ key ].value
		else :
			return default

	# This will store \param state in the parameters userData, under the default key,
	# unless 'collapsedUserDataKey' was provided in the UI constructor's **kw.
	def _storeCollapsedState( self, state ) :

		if "UI" not in self.parameter.userData() :
			self.parameter.userData()["UI"] = IECore.CompoundObject()

		key = self.__kw.get( "collapsedUserDataKey", CompoundParameterUI._collapsedUserDataKey )
		self.parameter.userData()["UI"][key] = IECore.BoolData( state )

	# Returns True if the ui should be collapsible for this parameter, False
	# otherwise.
	def __parameterIsCollapsible( self, node=None, parameter=None ) :

		if node is None :
			node = self.node()
		if parameter is None :
			parameter = self.parameter

		fnPH = IECoreMaya.FnParameterisedHolder( node )

		collapsible = not parameter.isSame( fnPH.getParameterised()[0].parameters() )
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = parameter.userData()["UI"]["collapsible"].value
		with IECore.IgnoredExceptions( KeyError ) :
			collapsible = parameter.userData()["UI"]["collapsable"].value

		collapsible = self.__kw.get( "withCompoundFrame", False ) or collapsible

		return collapsible

	@staticmethod
	def _labelFont( hierarchyDepth ) :

		if hierarchyDepth == 2 :
			return "smallBoldLabelFont"
		elif hierarchyDepth >= 3 :
			return "tinyBoldLabelFont"
		else :
			return "boldLabelFont"

	@staticmethod
	def _labelIndent( hierarchyDepth ) :

		return 5 + ( 8 * max( 0, hierarchyDepth-1 ) )

	## May be implemented by derived classes to present some custom ui at the
	# top of the list of child parameters. Implementations should first call the
	# base class method and then perform their custom behaviour, placing ui elements
	# into the provided columnLayout.
	def _createHeader( self, columnLayout, **kw ) :

		draggable = False
		try:
			draggable = self.parameter.userData()['UI']['draggable'].value
		except :
			pass

		## \todo Figure out what this draggable stuff is all about and document it.
		# I think it's intended to allow parameters to be dragged and dropped onto
		# an IECoreMaya.ParameterPanel but I can't get that to work right now.
		if draggable :

			maya.cmds.rowLayout(
				numberOfColumns = 2,
				columnWidth2 = ( 361, 20 ),
				parent = columnLayout

			)

			maya.cmds.text( label = "" )

			dragIcon = maya.cmds.iconTextStaticLabel(
				image = "pick.xpm",
				height = 20
			)
			self.addDragCallback( dragIcon, **kw )

		self.__headerCreated = True

	## May be called by derived classes if for any reason the child parameter
	# uis are deemed invalid - for instance if the child parameters have changed.
	# The uis will then be rebuilt during the next call to replace().
	def _deleteChildParameterUIs( self ) :

		maya.cmds.control( self.__columnLayout, edit=True, manage=False )

		for ui in self.__childUIs.values() :
			maya.cmds.deleteUI( ui.layout() )

		self.__childUIs = {}

		maya.cmds.control( self.__columnLayout, edit=True, manage=True )

	def __expand( self ) :

		self._storeCollapsedState( False )

		modifiers = maya.cmds.getModifiers()

		if modifiers & 1 :
			# shift is held
			self.__propagateCollapsed( False, 999, lazy=True )
		elif modifiers & 8 :
			# alt is held
			depth = 1;
			with IECore.IgnoredExceptions( KeyError ) :
				depth = self.parameter.userData()["UI"]["autoExpandDepth"].value
			self.__propagateCollapsed( False, depth, lazy=True )

	def __collapse(self):

		self._storeCollapsedState( True )

		# \todo Store collapse state
		modifiers = maya.cmds.getModifiers()

		if modifiers & 1 :
			# shift is held
			self.__propagateCollapsed( True, 999 )
		elif modifiers & 8 :
			# alt is held
			depth = 1;
			with IECore.IgnoredExceptions( KeyError ) :
				depth = self.parameter.userData()["UI"]["autoExpandDepth"].value
			self.__propagateCollapsed( True, depth )

	def __preExpand( self ) :

		# this is the most common entry point into the ui
		# creation code, and unfortunately it's called from
		# a maya ui callback. maya appears to suppress all
		# exceptions which occur in such callbacks, so we
		# have to wrap with our own exception handling to
		# make sure any errors become visible.
		try :
			with IECoreMaya.UITemplate( "attributeEditorTemplate" ) :
				if not self.__headerCreated :
					self._createHeader( self.__columnLayout, **self.__kw )
				if not len( self.__childUIs ) :
					self.__createChildUIs()
		except :

			IECore.msg( IECore.Msg.Level.Error, "IECoreMaya.ParameterUI", traceback.format_exc() )

	def __propagateCollapsed( self, collapsed, propagateDepth=999, **kw ) :

		for ui in self.__childUIs.values() :
			if hasattr( ui, "setCollapsed" ) :
				ui.setCollapsed( collapsed, propagateDepth, **kw )

	def __createChildUIs( self ) :

		self.__kw['labelWithNodeName'] = False

		for pName in self.parameter.keys():

			p = self.parameter[pName]

			visible = True
			try:
				visible = p.userData()['UI']['visible'].value
			except:
				pass

			if 'visibleOnly' in self.__kw :

				fullChildName = self.__kw['longParameterName']
				if fullChildName :
					fullChildName += "."
				fullChildName += pName

				visible = fullChildName in self.__kw['visibleOnly']

				if not visible and p.isInstanceOf( IECore.TypeId.CompoundParameter ) :

					for i in self.__kw['visibleOnly'] :
						if i.startswith( fullChildName + "." ) :
							visible = True
							break

			if visible:

				maya.cmds.setParent( self.__columnLayout )

				ui = IECoreMaya.ParameterUI.create( self.node(), p, **self.__kw )

				if ui:
					self.__childUIs[pName] = ui

IECoreMaya.ParameterUI.registerUI( IECore.TypeId.CompoundParameter, CompoundParameterUI )
