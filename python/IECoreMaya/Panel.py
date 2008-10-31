##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

import pickle

## The Panel class provides a handy base class for the implementation
# of maya scripted panels. It allows the panel to be represented as
# an object with member data, reduces the number of methods that must
# be implemented, and allows normal python objects to be used as the
# state persistence mechanism.
class Panel( IECoreMaya.UIElement ) :

	## Derived classes must implement this to create their ui, calling
	# Panel.__init__ with the top level of the ui. A new instance is
	# made each time maya wishes to populate a ui with a panel (effectively
	# this method implements the create, init and add callbacks of the
	# maya scripted panel).
	def __init__( self, topLevelUI ) :
	
		IECoreMaya.UIElement.__init__( self, topLevelUI )
	
	## Must be implemented by subclasses to return a python object
	# representing their state. This is used for both persistence
	# across scene save and load, but also for when a panel is torn
	# off or reparented.
	def _saveState( self ) :
	
		raise NotImplementedError
	
	## Must be implemented to restore a state previously saved with
	# _saveState.
	def _restoreState( self, state ) :
	
		raise NotImplementedError
	
	## Call this to instantiate a panel. Note that one panel instantiation
	# as far as maya is concerned may result in multiple IECoreMaya.Panel instantiations -
	# typically one for each time maya wishes to reparent or copy the panel.
	@classmethod
	def create( cls, label=None ) :
		
		typeName = cls.__name__
		panel = maya.cmds.scriptedPanel( unParent=True, type=typeName, label=typeName )
		if label :
			maya.cmds.scriptedPanel( panel, edit=True, label=label )
		return panel
	
	## Must be called to register all subclasses. This should
	# typically be done during plugin initialisation.
	@classmethod
	def registerPanel( cls, subclass ) :
	
		typeName = subclass.__name__
		if not maya.cmds.scriptedPanelType( typeName, exists=True ) :
			
			# we have to implement the mel callbacks in iePanel.mel
			# as maya doesn't implement python scripted panels. the mel
			# methods we use call straight back to the implementations
			# below.	
			t = maya.cmds.scriptedPanelType(
				typeName,
				createCallback="iePanelCreate",
				initCallback="iePanelInit",
				addCallback="iePanelAdd",
				removeCallback="iePanelRemove",
				saveStateCallback="iePanelSave",
				deleteCallback="iePanelDelete",
			)
#			print 't=' + t
#			print 'typeName=' + typeName
			assert( t==typeName )
			cls.__panelTypes[t] = subclass
			
		else :
		
			assert( cls.__panelTypes[typeName]==subclass )
			
	# maps from the maya panel name to a record of the Panel instance and state for
	# that panel
	__panels = {}
	# maps from the maya panel type to the Panel subclass for that type
	__panelTypes = {}
	
	## The methods below implement the callbacks maya requires by using the methods
	## defined above.
	##############################################################################
			
	@classmethod
	def __create( cls, panelName ) :
			
		t = maya.cmds.scriptedPanel( panelName, query=True, type=True )
		panelType = cls.__panelTypes[t]
	
		panelRecord = IECore.Struct()
		panelRecord.type = panelType
		panelRecord.instance = None
		panelRecord.state = None
		Panel.__panels[panelName] = panelRecord
				
	@classmethod
	def __init( cls, panelName ) :
	
		# we don't need to do anything here
		pass
		
	@classmethod
	def __add( cls, panelName ) :
			
		panelRecord = cls.__panels[panelName]
		assert( not panelRecord.instance )
		
		panelRecord.instance = panelRecord.type()
		if panelRecord.state :
			panelRecord.instance._restoreState( panelRecord.state )
		
	@classmethod
	def __remove( cls, panelName ) :
		
		panelRecord = cls.__panels[panelName]
		assert( panelRecord.instance )
		
		panelRecord.state = panelRecord.instance._saveState()
		panelRecord.instance = None
		
	@classmethod
	def __save( cls, panelName ) :
	
		panelRecord = cls.__panels[panelName]
		if panelRecord.instance :
			state = panelRecord.instance._saveState()
		else :
			state = panelRecord.state
			
		if not state :
			return ""
		
		pickledState = pickle.dumps( state )
		encodedState = pickledState.encode( "hex" )
		return "iePanelRestore( \"%s\", \"%s\" )" % ( panelName, encodedState )
				
	@classmethod
	def __delete( cls, panelName ) :
	
		del cls.__panels[panelName]
	
	# This one isn't actually a required part of the maya protocol, but
	# we call it in the saved state strings to restore layouts when files
	# are opened.
	@classmethod
	def __restore( cls, panelName, encodedState ) :
			
		panelRecord = cls.__panels[panelName]
		decodedState = encodedState.decode( "hex" )
		unpickledState = pickle.loads( decodedState )
		
		panelRecord.state = unpickledState
		if panelRecord.instance :
			panelRecord.instance._restoreState( panelRecord.state )
		
	__panelStates = {}
