##########################################################################
#
#  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

import maya.OpenMaya
import maya.cmds
from FnParameterisedHolder import FnParameterisedHolder

## A function set for operating on the IECoreMaya::ProceduralHolder type.
class FnProceduralHolder( FnParameterisedHolder ) :

	## Initialise the function set for the given procedural object, which may
	# either be an MObject or a node name in string or unicode form.
	def __init__( self, object ) :

		FnParameterisedHolder.__init__( self, object )

	## Creates a new node with the specified name and holding the specified procedural
	# class type. Returns a function set instance operating on this new node.
	@staticmethod
	def create( nodeName, className, classVersion=-1 ) :

		holder = maya.mel.eval( "ieProceduralHolderCreate( \"%s\", \"%s\", %d )" % ( nodeName, className, classVersion ) )
		return FnProceduralHolder( holder )

	## Convenience method to call setParameterised with the environment variable
	# for the searchpaths set to "IECORE_PROCEDURAL_PATHS".
	def setProcedural( self, className, classVersion ) :

		self.setParameterised( className, classVersion, "IECORE_PROCEDURAL_PATHS" )

	## Convenience method to return the ParameterisedProcedural class held inside this
	# node.
	def getProcedural( self ) :

		return self.getParameterised()[0]

	## Returns a set of the names of any currently selected components. These names
	# are specified by the procedural by setting the "name" attribute in the
	# renderer.
	def selectedComponentNames( self ) :

		result = set()

		s = maya.OpenMaya.MSelectionList()
		maya.OpenMaya.MGlobal.getActiveSelectionList( s )

		fullPathName = self.fullPathName()
		for i in range( 0, s.length() ) :

			try :

				p = maya.OpenMaya.MDagPath()
				c = maya.OpenMaya.MObject()
				s.getDagPath( i, p, c )

				if p.node()==self.object() :

					fnC = maya.OpenMaya.MFnSingleIndexedComponent( c )
					a = maya.OpenMaya.MIntArray()
					fnC.getElements( a )

					for j in range( 0, a.length() ) :

						result.add( maya.cmds.getAttr( fullPathName + ".proceduralComponents[" + str( a[j] ) + "]" ) )

			except :
				pass

		return result

	## Selects the components specified by the passed names. If replace is True
	# then the current selection is deselected first.
	def selectComponentNames( self, componentNames ) :

		if not isinstance( componentNames, set ) :
			componentNames = set( componentNames )

		fullPathName = self.fullPathName()
		validIndices = maya.cmds.getAttr( fullPathName + ".proceduralComponents", multiIndices=True )
		toSelect = []
		for i in validIndices :
			componentName = maya.cmds.getAttr( fullPathName + ".proceduralComponents[" + str( i ) + "]" )
			if componentName in componentNames :
				toSelect.append( fullPathName + ".f[" + str( i ) + "]" )

		maya.cmds.select( clear=True )
		maya.cmds.selectMode( component=True )
		maya.cmds.hilite( fullPathName )
		for s in toSelect :
			maya.cmds.select( s, add=True )



