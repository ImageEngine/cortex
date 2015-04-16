##########################################################################
#
#  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

import os

import appleseed

import IECore
import IECoreAppleseed

import AppleseedTest

class MotionTest( AppleseedTest.TestCase ):

		def testCameraMotionBlur( self ) :

			r = IECoreAppleseed.Renderer()

			with IECore.MotionBlock( r, [ 0.25, 0.5, 0.75 ] ) :

				r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 0 ) ) )
				r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 1 ) ) )
				r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 2 ) ) )

			r.camera( "camera", {} )

			cam = self._getCamera( r )
			transforms = cam.transform_sequence().transforms()
			self.failUnless( len( transforms ) == 3 )
			self.failUnless( transforms[0][0] == 0.25 )
			self.failUnless( transforms[1][0] == 0.50 )
			self.failUnless( transforms[2][0] == 0.75 )

		def testTransformationMotionBlur( self ) :

			r = IECoreAppleseed.Renderer()

			r.worldBegin()

			r.attributeBegin()
			r.setAttribute( "name", "object" )

			self._createDefaultShader( r )

			with IECore.MotionBlock( r, [ 0.25, 0.5, 0.75 ] ) :

				r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 0 ) ) )
				r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 1 ) ) )
				r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 2 ) ) )

			m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			m.render( r )

			r.attributeEnd()

			scn = self._getMainAssembly( r )
			ass_instance = scn.assembly_instances()["object_assembly_instance"]

			transforms = ass_instance.transform_sequence().transforms()
			self.failUnless( len( transforms ) == 3 )
			self.failUnless( transforms[0][0] == 0.25 )
			self.failUnless( transforms[1][0] == 0.50 )
			self.failUnless( transforms[2][0] == 0.75 )

		def testDeformationMotionBlurPow2NumSamples( self ) :

			r = IECoreAppleseed.Renderer()
			r.camera( "camera", { "shutter" : IECore.V2fData( IECore.V2f( 0.25, 0.75 ) ) } )

			r.worldBegin()

			self._createDefaultShader( r )

			m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			m2 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )

			r.attributeBegin()
			r.setAttribute( "name", "object" )

			with IECore.MotionBlock( r, [ 0.25, 0.75 ] ) :

				m1.render( r )
				m2.render( r )

			r.attributeEnd()

			scn = self._getMainAssembly( r )
			ass = scn.assemblies()["object_assembly"]
			self.failUnless( len( ass.objects() ) == 1 )

			me = ass.objects()[0]
			self.failUnless( me.get_motion_segment_count() == 1 )

		def testDeformationMotionBlurNonPow2NumSamples( self ) :

			r = IECoreAppleseed.Renderer()
			r.camera( "camera", { "shutter" : IECore.V2fData( IECore.V2f( 0.25, 0.75 ) ) } )

			r.worldBegin()

			self._createDefaultShader( r )

			m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
			m2 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )
			m3 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -3 ), IECore.V2f( 3 ) ) )

			r.attributeBegin()
			r.setAttribute( "name", "object" )

			with IECore.MotionBlock( r, [ 0.25, 0.5, 0.75 ] ) :

				m1.render( r )
				m2.render( r )
				m3.render( r )

			r.attributeEnd()

			scn = self._getMainAssembly( r )
			ass = scn.assemblies()["object_assembly"]
			self.failUnless( len( ass.objects() ) == 1 )

			me = ass.objects()[0]
			self.failUnless( me.get_motion_segment_count() == 3 )

if __name__ == "__main__":
	unittest.main()

