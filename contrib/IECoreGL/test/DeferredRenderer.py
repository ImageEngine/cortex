##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

import unittest
import random
import IECore
import IECoreGL


IECoreGL.init( False )

class TestDeferredRenderer( unittest.TestCase ) :
	
	def __traverseGroup( self, g, result ) :
				
		n = g.getState().get( IECoreGL.NameStateComponent.staticTypeId() )
	
		if n != None:
	
			result.append( n.name() )
			
		else :
			
			result.append( "None" )
		
		for c in g.children() :
		
			self.__traverseGroup( c, result )
	
	def __buildGroup( self, parent, depth, name, maxDepth = 10 ) :
		""" Build a random hierarchy with uniquely named children """
	
		if depth >= maxDepth :
		
			return
			
		numChildren = int( 4 * random.random() )	
		
		for c in range( 0, numChildren ) :
		
			n = name + "." + str( c )
		
			child = IECore.Group()
			
			attributes = IECore.AttributeState()
			child.addState( attributes )

			attributes.attributes["name"] = IECore.StringData( n )
						
			parent.addChild( child )
			
			if random.random() > 0.1 :
				self.__buildGroup( child, depth + 1, n, maxDepth = maxDepth )
		
	def testSceneOrder( self ) :			
	
		# Make sure that scene order is consistent across multiple renders/renderers
	
		random.seed( 300 )
		root = IECore.Group()
		self.__buildGroup( root, 0, "root" )
		
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		
		r.worldBegin()
		root.render( r )
		r.worldEnd()
		
		result = []
		self.__traverseGroup( r.scene().root(), result )
		
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		
		r.worldBegin()
		root.render( r )
		r.worldEnd()
		
		result2 = []
		self.__traverseGroup( r.scene().root(), result2 )
				
		self.assertEqual( result, result2 )
		
		r.worldBegin()
		root.render( r )
		r.worldEnd()
		
		result2 = []
		self.__traverseGroup( r.scene().root(), result2 )
		
		self.assertEqual( result, result2 )
			
		
if __name__ == "__main__":
    unittest.main()   
