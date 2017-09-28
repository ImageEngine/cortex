##########################################################################
#
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

import IECoreMaya

## In Maya 2011 and 2012, the collapsible frameLayout became rather ugly,
# and stopped indenting the arrow with the label. This made complex uis
# consisting of lots of ClassVectorParameters and ClassParameters somewhat
# unreadable. So we introduce this class to get back some control. Aside
# from spelling collapsible properly and being prettier, this class also
# has the advantage of supporting annotations which are displayed on the label.
# As with the maya frameLayout, the preExpandCommand, expandCommand and
# collapseCommand are only called as a result of user action, and never as
# a result of a call to setCollapsed or getCollapsed. There are separate
# implementations for maya before qt and maya after qt.

class _CollapsibleMotif( IECoreMaya.UIElement ) :

	def __init__( self,
		label="",
		labelVisible=True,
		labelIndent=0,
		labelFont = "boldLabelFont",
		annotation="",
		collapsed = True,
		preExpandCommand = None,
		expandCommand = None,
		collapseCommand = None,
	) :


		kw = {}
		if preExpandCommand is not None :
			kw["preExpandCommand"] = preExpandCommand
		if expandCommand is not None :
			kw["expandCommand"] = expandCommand
		if collapseCommand is not None :
			kw["collapseCommand"] = collapseCommand

		# implementation for motif is pretty simple - just a frame layout

		IECoreMaya.UIElement.__init__( self,
			maya.cmds.frameLayout(

				label = label,
				labelVisible = labelVisible,
				labelIndent = labelIndent,
				labelAlign = "center",
				font = labelFont,
				borderVisible = False,
				collapsable = True,
				collapse = collapsed,
				marginWidth = 0,
				**kw

			)
		)

		# can't display it but at least we can store it
		self.__annotation = annotation

		self.__frameLayout = self._topLevelUI()

	## The maya frameLayout whose collapsibility is controlled by this
	# class. Add children by editing the contents of this layout.
	def frameLayout( self ) :

		return self._topLevelUI()

	def setLabel( self, label ) :

		maya.cmds.frameLayout( self.frameLayout(), edit=True, label = label )

	def getLabel( self ) :

		return maya.cmds.frameLayout( self.frameLayout(), query=True, label = True )

	def setAnnotation( self, annotation ) :

		self.__annotation = annotation

	def getAnnotation( self ) :

		return self.__annotation

	def getCollapsed( self ) :

		return maya.cmds.frameLayout( self.frameLayout(), query=True, collapse=True )

	def setCollapsed( self, collapsed ) :

		maya.cmds.frameLayout( self.frameLayout(), edit=True, collapse=collapsed )

class _CollapsibleQt(  IECoreMaya.UIElement ) :

	def __init__( self,
		label="",
		labelVisible=True,
		labelIndent=0,
		labelFont = "boldLabelFont",
		annotation="",
		collapsed = True,
		preExpandCommand = None,
		expandCommand = None,
		collapseCommand = None,
	) :

		IECoreMaya.UIElement.__init__( self, maya.cmds.formLayout() )

		attachForm = []
		attachControl = []

		# make the layout to put things in. this is actually a frameLayout, just
		# with the ugly header bit we don't like hidden.
		########################################################################

		self.__frameLayout = maya.cmds.frameLayout(

			labelVisible = False,
			borderVisible = False,
			collapsable = True,
			collapse = collapsed,
			marginWidth = 0,

		)

		# passing borderVisible=False to the constructor does bugger all so we have to do it with
		# an edit
		maya.cmds.frameLayout( self.__frameLayout, edit=True, borderVisible=False, marginWidth=0 )

		attachForm.append( ( self.__frameLayout, "left", 0 ) )
		attachForm.append( ( self.__frameLayout, "right", 0 ) )
		attachForm.append( ( self.__frameLayout, "bottom", 0 ) )

		# optional header, with the triangle for expanding and collapsing
		########################################################################

		self.__collapsibleIcon = None
		self.__labelControl = None
		if labelVisible :

			# have to make one button for the icon and one for the label
			# because otherwise the icon size changes when we toggle
			# the image, and the text moves.
			self.__collapsibleIcon = maya.cmds.iconTextButton(

				parent = self._topLevelUI(),
				height = 20,
				width = 15,
				image = "arrowRight.xpm",
				command = self.__toggle,
				annotation = annotation,

			)

			self.__labelControl = maya.cmds.iconTextButton(

				parent = self._topLevelUI(),
				height = 20,
				label = label,
				# the font flag appears to do nothing, but maybe it will
				# miraculously be supported in the future?
				font = labelFont,
				style = "textOnly",
				command = self.__toggle,
				annotation = annotation,

			)

			attachForm.append( ( self.__collapsibleIcon, "left", labelIndent ) )
			attachForm.append( ( self.__collapsibleIcon, "top", 0 ) )
			attachForm.append( ( self.__labelControl, "top", 0 ) )

			attachControl.append( ( self.__labelControl, "left", 0, self.__collapsibleIcon ) )
			attachControl.append( ( self.__frameLayout, "top", 0, self.__labelControl ) )

		else :

			attachForm.append( ( self.__frameLayout, "top", 0 ) )

		maya.cmds.formLayout(
			self._topLevelUI(),
			edit = True,
			attachForm = attachForm,
			attachControl = attachControl,
		)

		maya.cmds.setParent( self.__frameLayout )

		self.__annotation = annotation
		self.__labelText = label
		self.__preExpandCommand = preExpandCommand
		self.__expandCommand = expandCommand
		self.__collapseCommand = collapseCommand

	## The maya frameLayout whose collapsibility is controlled by this
	# class. Add children by editing the contents of this layout.
	def frameLayout( self ) :

		return self.__frameLayout

	def setLabel( self, label ) :

		self.__labelText = label
		if self.__labelControl is not None :
			maya.cmds.iconTextButton( self.__labelControl, edit=True, label=label )

	def getLabel( self ) :

		return self.__labelText

	def setAnnotation( self, annotation ) :

		self.__annotation = annotation
		if self.__labelControl is not None :
			maya.cmds.iconTextButton( self.__labelControl, edit=True, annotation=annotation )
			maya.cmds.iconTextButton( self.__collapsibleIcon, edit=True, annotation=annotation )

	def getAnnotation( self ) :

		return self.__annotation

	def getCollapsed( self ) :

		return maya.cmds.frameLayout( self.__frameLayout, query=True, collapse=True )

	def setCollapsed( self, collapsed ) :

		maya.cmds.frameLayout( self.__frameLayout, edit=True, collapse=collapsed )
		if self.__collapsibleIcon is not None :
			maya.cmds.iconTextButton(
				self.__collapsibleIcon,
				edit = True,
				image = "arrowRight.xpm" if collapsed else "arrowDown.xpm",
			)

	def __toggle( self ) :

		collapsed = not self.getCollapsed()
		if not collapsed and self.__preExpandCommand is not None :
				self.__preExpandCommand()

		self.setCollapsed( not self.getCollapsed() )

		if collapsed :
			if self.__collapseCommand is not None :
				self.__collapseCommand()
		else :
			if self.__expandCommand is not None :
				self.__expandCommand()

# choose the right implementation based on the current maya version
if maya.OpenMaya.MGlobal.apiVersion() >= 201100 :
	Collapsible = _CollapsibleQt
else :
	Collapsible = _CollapsibleMotif
