##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

import os
import unittest
import time

import IECore
import IECoreRI
				
class RerenderingTest( unittest.TestCase ) :

	def testEditLight( self ) :
		
		# start an editable render with one light colour
		
		r = IECoreRI.Renderer( "" )
		
		r.setOption( "editable", True )
		
		r.display( "test", "ie", "rgba",
			{
				"driverType" : IECore.StringData( "ImageDisplayDriver" ),
				"handle" : IECore.StringData( "myLovelySphere" ),
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		with IECore.WorldBlock( r ) :
		
			r.light(
				"pointlight",
				"myLovelyLight",
				{
					"lightcolor" : IECore.Color3f( 1, 0.5, 0.25 ),
				}
			)
					
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
	
			r.shader( "surface", "matte", {} )
			r.sphere( 1, -1, 1, 360, {} )
				
		# give it a bit of time to finish
		
		time.sleep( 1 )
		
		# check we get the colour we expected
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()

		e.pointAtUV( IECore.V2f( 0.5 ), er )
		c = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		self.assertEqual( c / c[0], IECore.Color3f( 1, 0.5, 0.25 ) )
		
		# make an edit to the light color and check the colour has changed
		
		r.editBegin( "light", {} )
		
		r.light(
				"pointlight",
				"myLovelyLight",
				{
					"lightcolor" : IECore.Color3f( 0.25, 0.5, 1 ),
				}
			)
		
		r.editEnd()
		
		time.sleep( 1 )
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()
		
		c = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		self.assertEqual( c / c[2], IECore.Color3f( 0.25, 0.5, 1 ) )
	
	def testEditShader( self ) :
	
		# start an editable render with one light colour
		
		r = IECoreRI.Renderer( "" )
		
		r.setOption( "editable", True )
		
		r.display( "test", "ie", "rgba",
			{
				"driverType" : IECore.StringData( "ImageDisplayDriver" ),
				"handle" : IECore.StringData( "myLovelySphere" ),
				"quantize" : IECore.FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		with IECore.WorldBlock( r ) :
		
			r.light( "pointlight", "myLovelyLight", {} )
			
			with IECore.AttributeBlock( r ) :
			
				r.setAttribute( "name", "/sphere" )
			
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )
	
				r.shader( "surface", "matte", { "Kd" : 1 } )
				
				r.sphere( 1, -1, 1, 360, {} )
				
		# give it a bit of time to finish
		
		time.sleep( 1 )
		
		# record the colour produced by the first render
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5 ), er )
		initialColor = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		
		# make an edit to the shader and wait for it to take hold
		
		r.editBegin( "attribute", { "scopename" : "/sphere" } )
		r.shader( "surface", "matte", { "Kd" : 0.5 } )
		r.editEnd()
		time.sleep( 1 )
		
		# check the ratio of the two colours is as expected.
		
		i = IECore.ImageDisplayDriver.storedImage( "myLovelySphere" )
		e = IECore.ImagePrimitiveEvaluator( i )
		er = e.createResult()
		e.pointAtUV( IECore.V2f( 0.5 ), er )
		newColor = IECore.Color3f( er.floatPrimVar( i["R"] ), er.floatPrimVar( i["G"] ), er.floatPrimVar( i["B"] ) )
		
		self.assertEqual( newColor / initialColor, IECore.Color3f( .5 ) )
		
if __name__ == "__main__":
    unittest.main()
