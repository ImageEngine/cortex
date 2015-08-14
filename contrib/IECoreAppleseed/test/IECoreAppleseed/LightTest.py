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
import unittest

import appleseed

import IECore
import IECoreAppleseed

import AppleseedTest

class LightTest( AppleseedTest.TestCase ):

	def testDefaultFirstEnvLight( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "first_env_light" ) )
		r.light( "constant_environment_edf", "env_light", {} )
		r.attributeEnd()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "second_env_light" ) )
		r.light( "gradient_environment_edf", "env_light", {} )
		r.attributeEnd()

		scn = self._getScene( r )
		self.failUnless( len( scn.environment_edfs() ) == 1 )

		light = scn.environment_edfs()[0]
		self.failUnless( light.get_model() == "constant_environment_edf" )

	def testChooseEnvLightOption( self ) :

		r = IECoreAppleseed.Renderer()
		r.setOption( "as:environment_edf", IECore.StringData( "env_light" ) )
		r.worldBegin()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "ignore_me_light" ) )
		r.light( "constant_environment_edf", "env_light", {} )
		r.attributeEnd()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "env_light" ) )
		r.light( "gradient_environment_edf", "env_light", {} )
		r.attributeEnd()

		scn = self._getScene( r )
		self.failUnless( len( scn.environment_edfs() ) == 1 )

		light = scn.environment_edfs()[0]
		self.failUnless( light.get_model() == "gradient_environment_edf" )

	def testIlluminateEnvironmentLights( self ) :

		r = IECoreAppleseed.Renderer()
		r.setOption( "as:environment_edf", IECore.StringData( "env_light" ) )
		r.worldBegin()

		r.attributeBegin()
		r.setAttribute( "name", IECore.StringData( "env_light" ) )
		r.light( "constant_environment_edf", "env_light", {} )
		r.attributeEnd()

		scn = self._getScene( r )
		self.failUnless( len( scn.environment_edfs() ) == 1 )

		r.illuminate( "env_light", False )
		self.failUnless( len( scn.environment_edfs() ) == 0 )

		r.illuminate( "env_light", True )
		self.failUnless( len( scn.environment_edfs() ) == 1 )

	def testIlluminateSingularLights( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()
		r.light( "point_light", "light", {} )

		ass = self._getMainAssembly( r )
		self.failUnless( len( ass.lights() ) == 1 )

		r.illuminate( "light", False )
		self.failUnless( len( ass.lights() ) == 0 )

		r.illuminate( "light", True )
		self.failUnless( len( ass.lights() ) == 1 )

	def testLightPrefixes( self ) :

		r = IECoreAppleseed.Renderer()
		r.worldBegin()

		r.light( "ai:gradient_environment_edf", "arnoldEnvHandle", {} )
		scn = self._getScene( r )
		self.failUnless( len( scn.environment_edfs() ) == 0 )

		r.light( "as:gradient_environment_edf", "appleseedEnvHandle", {} )
		self.failUnless( len( scn.environment_edfs() ) == 1 )
		self.failUnless( scn.environment_edfs()[0].get_model() == "gradient_environment_edf" )

		r.light( "directional_light", "genericHandle", {} )
		r.light( "ri:point_light", "renderManHandle", {} )
		r.light( "as:point_light", "appleseedLight", {} )
		ass = self._getMainAssembly( r )

		self.failUnless( len( ass.lights() ) == 2 )
		self.failUnless( ass.lights()[0].get_model() == "directional_light" )
		self.failUnless( ass.lights()[1].get_model() == "point_light" )

if __name__ == "__main__":
	unittest.main()
