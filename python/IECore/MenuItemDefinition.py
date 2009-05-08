##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

## The MenuItemDefinition class defines the contents of a menu item for use
# with the MenuDefinition class. It does nothing towards actually implementing
# a user interface, but instead defines content for a user interface
# implementation to realise. This allows menus to be defined in a UI agnostic
# way and then used with different toolkits.
#
# The MenuItemDefinition has the following attributes :
#
# command : a callable object invoked when the user selects the menu item
#
# secondaryCommand : a callable object invoked when the user selects the menu item
# in some other way. this is toolkit dependent - for instance in maya this command
# would be used when the option box is selected.
#
# divider : True if the menu item is a divider, False otherwise.
#
# active : if False then the menu item is unselectable. may also be a callable
# object which returns a boolean value to allow dynamic activation
#
# description : a string with help for the menu item
#
# subMenu : a callable object which returns a MenuDefinition, to produce
# a dynamically generated submenu.
#
# \todo Validation of attribute values, so for instance divider and command
# can't both be set at the same time.
# \ingroup python
class MenuItemDefinition :

	__slots__ = [ "command", "secondaryCommand", "divider", "active", "description", "subMenu" ]

	def __init__( self, dictionary = None, **kwArgs ) :

		self.command = None
		self.secondaryCommand = None
		self.divider = False
		self.active = True
		self.description = ""
		self.subMenu = None

		if dictionary :
			for k, v in dictionary.items() :
				setattr( self, k, v )

		for k, v in kwArgs.items() :
			setattr( self, k, v )

	def __repr__( self ) :

		d = {}
		for s in self.__slots__ :
			d[s] = getattr( self, s )

		return "MenuItemDefinition( " + repr( d ) + " )"
