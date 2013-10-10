##########################################################################
#
#  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

class IlluminateTest( IECoreRI.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/illuminate.rib"

	def testDeprecated( self ) :

		r = IECoreRI.Renderer( self.outputFileName )

		r.worldBegin()

		r.command( "ri:illuminate", { "handle" : IECore.StringData( "light1" ), "state" : IECore.BoolData( True ) } )
		r.command( "ri:illuminate", { "handle" : IECore.StringData( "light2" ), "state" : IECore.BoolData( False ) } )

		r.worldEnd()

		l = "".join( file( self.outputFileName ).readlines() )
		self.assert_( "Illuminate \"light1\" 1" in l )
		self.assert_( "Illuminate \"light2\" 0" in l )

	def test( self ) :
	
		r = IECoreRI.Renderer( self.outputFileName )
		
		r.worldBegin()
		
		r.light( "spotlight", "myLightHandle", { "intensity" : 2, "colour" : IECore.Color3f( 1, 0, 0 ) } )
		
		r.illuminate( "myLightHandle", 0 )
		
		r.worldEnd()

		l = "".join( file( self.outputFileName ).readlines() )

		self.assert_( "LightSource \"spotlight\" \"myLightHandle\"" in l )
		self.assert_( '"color colour" [ 1 0 0 ]' in l ) 
		self.assert_( '"int intensity" [ 2 ]' in l ) 
		self.assert_( "Illuminate \"myLightHandle\" 0" in l )

	def testAreaLight( self ) :
		r = IECoreRI.Renderer( self.outputFileName )
		
		r.worldBegin()
		
		r.light( "spotlight", "myLightHandle", { "intensity" : 2, "colour" : IECore.Color3f( 1, 0, 0 ), "ri:areaLight" : True } )
		r.light( "spotlight", "myLightHandle2", { "intensity" : 2, "colour" : IECore.Color3f( 1, 0, 0 ), "ri:areaLight" : False } )
		
		r.worldEnd()

		l = "".join( file( self.outputFileName ).readlines() )

		self.assert_( "AreaLightSource \"spotlight\" \"myLightHandle\"" in l )
		self.assert_( "LightSource \"spotlight\" \"myLightHandle2\"" in l )

if __name__ == "__main__":
    unittest.main()
