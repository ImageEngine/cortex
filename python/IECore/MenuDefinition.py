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

import re
from MenuItemDefinition import MenuItemDefinition

## The MenuDefinition class defines the contents of a hierarchical menu
# containing MenuItemDefinition instances. It does nothing towards actually
# creating a user interface, but instead defines content for a user
# interface implementation to realise. This allows menus to be defined in a
# UI agnostic way and then used with different toolkits.
# \ingroup python
class MenuDefinition :

	def __init__( self, items = [] ) :
	
		self.__items = []
	
		for path, item in items :
		
			self.append( path, item )
	
	## Prepends a menu item to the menu. The item will
	# appear before all other items in its respective
	# submenu.		
	def prepend( self, path, item ) :

		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )

		self.__items.insert( 0, ( path, item ) )
		
	## Appends a menu item at the end. The item will
	# appear after all other items in its respective
	# submenu.		
	def append( self, path, item ) :

		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )

		self.__items.append( ( path, item ) )
	
	## Insert a menu item before the specified menu item.
	def insertBefore( self, path, item, beforePath ) :
	
		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )
		
		i = self.__pathIndex( beforePath )
		self.__items.insert( i, ( path, item ) )
		
	## Insert a menu item after the specified menu item.
	def insertAfter( self, path, item, afterPath ) :
		
		if isinstance( item, dict ) :
			item = MenuItemDefinition( item )

		self.remove( path, False )
		
		i = self.__pathIndex( afterPath )
		self.__items.insert( i+1, ( path, item ) )
	
	## Removes the named menu item. Raises a KeyError if
	# no such item exists and raiseIfMissing is True.	
	def remove( self, path, raiseIfMissing=True ) :
	
		index = None
		for i in range( 0, len( self.__items ) ) :
			if self.__items[i][0]==path :
				index = i
				break
				
		if not index is None :
			del self.__items[index]
		else :
			if raiseIfMissing :
				raise KeyError( path )

	## Removes all items whose paths match the given
	# regular expression.
	def removeMatching( self, regEx ) :
	
		if type( regEx ) is str :
			regEx = re.compile( regEx )
				
		toRemove = []
		for i in range( 0, len( self.__items ) ) :
			if regEx.search( self.__items[i][0] ) :
				toRemove.append( i )	
	
		toRemove.sort()
		toRemove.reverse()
		for i in toRemove :
			del self.__items[i]
		
	## Removes all menu items from the definition.
	def clear( self ) :
	
		del self.__items[:]
		
	## Returns a list of tuples of the form (path, MenuItemDefinition).
	# This can be used in realising the menu in a UI toolkit. This list
	# should be considered read-only - use the other methods to add and
	# remove items.	
	def items( self ) :
	
		return self.__items
	
	## Returns a new MenuDefinition containing only the menu items
	# that reside below the specified root path. The paths in this
	# new definition are all adjusted to be relative to the requested
	# root.	
	def reRooted( self, root ) :
	
		if not len( root ) :
			return MenuDefinition( [] )
		
		if root[-1]!="/" :
			root = root + "/"
			
		newItems = []
		for item in self.items() :
		
			if item[0].startswith( root ) :
				newItems.append( ( item[0][len(root)-1:], item[1] ) )
				
		return MenuDefinition( newItems )
		
	def __repr__( self ) :
	
		return "MenuDefinition( " + repr( self.items() ) + " )"

	def __pathIndex( self, path ) :
	
		for i in range( 0, len( self.__items ) ) :
		
			if self.__items[i][0]==path :
				return i
				
		raise KeyError( path )		
