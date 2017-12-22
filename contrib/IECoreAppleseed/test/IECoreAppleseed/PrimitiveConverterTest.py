##########################################################################
#
#  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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
import imath

import IECore
import IECoreScene
import IECoreAppleseed

import AppleseedTest

class PrimitiveConverterTest( AppleseedTest.TestCase ):

	def testSingleMaterial( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		self._createDefaultShader( r )

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = m1.copy()
		m3 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -2 ), imath.V2f( 2 ) ) )

		m1.render( r )
		m2.render( r )
		m3.render( r )

        # we should have 2 unique primitives, instanced 3 times.
		self.failUnless( self._countAssemblies( r ) == 2 )
		self.failUnless( self._countAssemblyInstances( r ) == 3 )

	def testMultipleMaterialsNoInstancing( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )

		self._createDefaultShader( r )
		m.render( r )

		self._createGlossyShader( r )
		m.render( r )

        # we should have 2 unique primitives, instanced 1 time each.
		self.failUnless( self._countAssemblies( r ) == 2 )
		self.failUnless( self._countAssemblyInstances( r ) == 2 )

	def testAttributesNoInstancing( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		self._createDefaultShader( r )

		m = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m.render( r )

		r.attributeBegin()
		r.setAttribute( "as:shading_samples", IECore.IntData( 4 ) )
		m.render( r )
		r.attributeEnd()

		r.attributeBegin()
		r.setAttribute( "as:alpha_map", IECore.StringData( "no_such_file.exr" ) )
		m.render( r )
		r.attributeEnd()

		r.attributeBegin()
		r.setAttribute( "as:photon_target", IECore.BoolData( True ) )
		m.render( r )
		r.attributeEnd()

		self.failUnless( self._countAssemblies( r ) == 4 )
		self.failUnless( self._countAssemblyInstances( r ) == 4 )

	def testNoAutoInstancing( self ) :

		r = IECoreAppleseed.Renderer()
		r.setOption( "as:automatic_instancing", False )
		r.worldBegin()

		self._createDefaultShader( r )

		m1 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -1 ), imath.V2f( 1 ) ) )
		m2 = m1.copy()
		m3 = IECoreScene.MeshPrimitive.createPlane( imath.Box2f( imath.V2f( -2 ), imath.V2f( 2 ) ) )

		m1.render( r )
		m2.render( r )
		m3.render( r )
		m2.render( r )

		self.failUnless( self._countAssemblies( r ) == 4 )
		self.failUnless( self._countAssemblyInstances( r ) == 4 )

if __name__ == "__main__":
	unittest.main()
