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
import os.path

from IECore import *

from IECoreGL import *
init( False )

class TestSelection( unittest.TestCase ) :

	def testSelect( self ) :
	
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		
		r.worldBegin()
	
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, 5 ) ) )

		r.setAttribute( "name", StringData( "one" ) )
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 0, 0 ) ) } )
		r.geometry( "sphere", {}, {} )

		r.concatTransform( M44f.createTranslated( V3f( -1, 0, 0 ) ) )
		r.setAttribute( "name", StringData( "two" ) )
		r.geometry( "sphere", {}, {} )

		r.concatTransform( M44f.createTranslated( V3f( 2, 0, 0 ) ) )
		r.setAttribute( "name", StringData( "three" ) )
		r.geometry( "sphere", {}, {} )
				
		r.worldEnd()
		
		s = r.scene()
		s.setCamera( PerspectiveCamera() )
		
		ss = s.select( Box2f( V2f( 0 ), V2f( 1 ) ) )
		names = [ x.name.value() for x in ss ]
		self.assertEqual( len( names ), 3 )
		self.assert_( "one" in names )
		self.assert_( "two" in names )
		self.assert_( "three" in names )
		
	def testRegionSelect( self ) :
	
		r = Renderer()
		r.setOption( "gl:mode", StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", StringData( os.path.dirname( __file__ ) + "/shaders" ) )
		
		r.worldBegin()
		
		r.concatTransform( M44f.createTranslated( V3f( 0, 0, 5 ) ) )

		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 0, 0 ) ) } )
		r.concatTransform( M44f.createTranslated( V3f( -2, -2, 0 ) ) )
		r.setAttribute( "name", StringData( "red" ) )
		r.geometry( "sphere", {}, {} )

		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 0, 1, 0 ) ) } )
		r.concatTransform( M44f.createTranslated( V3f( 0, 4, 0 ) ) )
		r.setAttribute( "name", StringData( "green" ) )
		r.geometry( "sphere", {}, {} )

		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 0, 0, 1 ) ) } )
		r.concatTransform( M44f.createTranslated( V3f( 4, 0, 0 ) ) )
		r.setAttribute( "name", StringData( "blue" ) )
		r.geometry( "sphere", {}, {} )
				
		r.shader( "surface", "color", { "colorValue" : Color3fData( Color3f( 1, 1, 1 ) ) } )
		r.concatTransform( M44f.createTranslated( V3f( 0, -4, 0 ) ) )
		r.setAttribute( "name", StringData( "white" ) )
		r.geometry( "sphere", {}, {} )

		r.worldEnd()
		
		s = r.scene()
		s.setCamera( PerspectiveCamera() )
		
		ss = s.select( Box2f( V2f( 0, 0.5 ), V2f( 0.5, 1 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "red" )

		ss = s.select( Box2f( V2f( 0 ), V2f( 0.5 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "green" )

		ss = s.select( Box2f( V2f( 0.5, 0 ), V2f( 1, 0.5 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "blue" )

		ss = s.select( Box2f( V2f( 0.5 ), V2f( 1 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "white" )
			
if __name__ == "__main__":
    unittest.main()   
