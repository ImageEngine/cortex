##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
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

import os.path

import maya.cmds

import IECore
import IECoreMaya

class ClassParameterUI( IECoreMaya.CompoundParameterUI ) :

	def __init__( self, node, parameter, **kw ) :

		self.__menuParent = None
		self.__currentClassInfo = parameter.getClass( True )[1:]

		# We have to do this after initialising self.__menuParent as
		# CompoundParameterUI.__init__ will call _createHeader in this
		# class, which populates self.__menuParent.
		IECoreMaya.CompoundParameterUI.__init__( self, node, parameter, **kw )

	def _createHeader( self, columnLayout, **kw ) :

		IECoreMaya.CompoundParameterUI._createHeader( self, columnLayout, **kw )

		maya.cmds.rowLayout( numberOfColumns = 2, parent = columnLayout )

		collapsable = True
		with IECore.IgnoredExceptions( KeyError ) :
			collapsable = self.parameter.userData()["UI"]["collapsable"].value

		lable = "Class" if collapsable else self.label()
		font = "smallPlainLabelFont" if collapsable else "tinyBoldLabelFont"

		maya.cmds.text(
			label = lable,
			font = font,
			align = "right",
			annotation = self.description()
		)

		self.__menuParent = maya.cmds.iconTextStaticLabel(
			image = "arrowDown.xpm",
			font = "smallBoldLabelFont",
			label = self.__menuParentLabel(),
			style = "iconAndTextHorizontal",
			height = 23,
		)

		# popup menu can be activated with either right or left buttons
		IECoreMaya.Menu( self.__menuDefinition, self.__menuParent )
		IECoreMaya.Menu( self.__menuDefinition, self.__menuParent, button = 1 )

	def replace( self, node, parameter ) :

		newClassInfo = parameter.getClass( True )[1:]
		if newClassInfo != self.__currentClassInfo :
			self._deleteChildParameterUIs()

		IECoreMaya.CompoundParameterUI.replace( self, node, parameter )

		if self.__menuParent :
			maya.cmds.iconTextStaticLabel( self.__menuParent, edit=True, label=self.__menuParentLabel() )

		self.__currentClassInfo = newClassInfo

	## We add support for the 'lazy' keyword argument. If True, then the layout
	# will not expand if the parameter doesn't contain a class.
	def setCollapsed( self, collapsed, propagateToChildren=0, **kw ) :

		lazy = kw["lazy"] if "lazy" in kw else False

		if not collapsed :
			if lazy and not self.parameter.getClass( False ) :
				collapsed = True

		IECoreMaya.CompoundParameterUI.setCollapsed( self, collapsed, propagateToChildren, **kw )

	def __classNameFilter( self ) :

		with IECore.IgnoredExceptions( KeyError ) :
			return self.parameter.userData()["UI"]["classNameFilter"].value

		return "*"

	def __menuParentLabel( self ) :

		classInfo = self.parameter.getClass( True )
		if classInfo[1] :
			labelPathStart = max( 0, self.__classNameFilter().find( "*" ) )
			return "%s v%d" % ( classInfo[1][labelPathStart:], classInfo[2] )
		else :
			return "Choose..."

	def __menuDefinition( self ) :

		result = IECore.MenuDefinition()

		menuPathStart = max( 0, self.__classNameFilter().find( "*" ) )

		classInfo = self.parameter.getClass( True )

		loader = IECore.ClassLoader.defaultLoader( classInfo[3] )
		for className in loader.classNames( self.__classNameFilter() ) :
			classVersions = loader.versions( className )
			for classVersion in classVersions :

				menuPath = "/" + className[menuPathStart:]
				if len( classVersions ) > 1 :
					menuPath += "/v" + str( classVersion )

				result.append(

					menuPath,

					IECore.MenuItemDefinition(
						command = IECore.curry( self.__setClass, className, classVersion, classInfo[3] ),
						active = className != classInfo[1] or classVersion != classInfo[2]
					)

				)

		result.append(
			"/RemoveDivider",
			IECore.MenuItemDefinition(
				divider = True
			)
		)

		result.append(

			"/Remove",

			IECore.MenuItemDefinition(
				command = IECore.curry( self.__setClass, "", 0, classInfo[3] ),
				active = classInfo[0] is not None
			)

		)

		for cb in self.__classMenuCallbacks :
			cb( result, self.parameter, self.node() )

		return result

	def __setClass( self, className, classVersion, searchPathEnvVar ) :

		fnPH = IECoreMaya.FnParameterisedHolder( self.node() )

		# To stop maya crashing, we need to delete the UI for the existing class
		# before we change it in C++, otherwise, an AE update gets triggered after
		# we have removed the parameters, but before we have removed the UI.
		self._deleteChildParameterUIs()

		with fnPH.parameterModificationContext() :
			self.parameter.setClass( className, classVersion, searchPathEnvVar )

	@staticmethod
	def _classSetCallback( fnPH, parameter ) :

		for instance in IECoreMaya.UIElement.instances( ClassParameterUI ) :
			if instance.parameter.isSame( parameter ) :
				instance.replace( instance.node(), instance.parameter )

	__classMenuCallbacks = []
	## Registers a callback which is able to modify the popup menu used to choose
	# the class held by this parameter. Callbacks should have the following signature :
	#
	# callback( menuDefinition, parameter, holderNode ).
	@classmethod
	def registerClassMenuCallback( cls, callback ) :

		cls.__classMenuCallbacks.append( callback )

IECoreMaya.FnParameterisedHolder.addSetClassParameterClassCallback( ClassParameterUI._classSetCallback )

IECoreMaya.ParameterUI.registerUI( IECore.ClassParameter.staticTypeId(), ClassParameterUI )
