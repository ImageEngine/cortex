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

import IECore
import IECoreRI

class SubsurfaceTest( unittest.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/subsurface.rib"
	
	def test( self ) :
	
		r = IECoreRI.Renderer( self.outputFileName )
		
		r.worldBegin()
		
		a = IECore.AttributeState()
		a.attributes["ri:subsurface"] = IECore.CompoundData(
			{
				"visibility" : IECore.StringData( "test" ),
				"meanfreepath" : IECore.Color3fData( IECore.Color3f( 1 ) ),
				"reflectance" : IECore.Color3fData( IECore.Color3f( 0 ) ),
				"refractionindex" : IECore.FloatData( 1.3 ),
				"scale" : IECore.FloatData( 0.1 ),
				"shadingrate" : IECore.FloatData( 10 ),
			}
		)
			
		a.render( r )
		
		r.worldEnd()
		
		l = "".join( file( self.outputFileName ).readlines() )
		
		p1 = l.find( 'Attribute "visibility" "string subsurface" [ "test" ]' )
		p2 = l.find( 'Attribute "subsurface" "color meanfreepath" [ 1 1 1 ] "color reflectance" [ 0 0 0 ] "float refractionindex" [ 1.3 ] "float scale" [ 0.1 ] "float shadingrate" [ 10 ]' )
		self.assertNotEqual( p1, -1 )
		self.assertNotEqual( p2, -1 )
		self.assert_( p2 > p1 )
					
	def tearDown( self ) :

		if os.path.exists( self.outputFileName ) :
			os.remove( self.outputFileName )
				
if __name__ == "__main__":
    unittest.main()   
