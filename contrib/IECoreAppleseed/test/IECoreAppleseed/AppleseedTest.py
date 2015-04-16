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

import unittest

import IECore

import appleseed

class TestCase( unittest.TestCase ):

	def _createDefaultShader( self, r ) :

		s = IECore.Shader( "data/shaders/matte.oso", "surface" )
		s.render( r )

	def _createGlossyShader( self, r ) :

		s = IECore.Shader( "data/shaders/glossy.oso", "surface" )
		s.render( r )

	def _getScene( self, r ) :

		proj = r.appleseedProject()
		return proj.get_scene()

	def _getCamera( self, r ) :

		scn = self._getScene( r )
		return scn.get_camera()

	def _getMainAssembly( self, r ) :

		return self._getScene( r ).assemblies().get_by_name( "assembly" )

	def _countAssemblies( self, r ) :

		ass = self._getMainAssembly( r )
		return len( ass.assemblies() )

	def _countAssemblyInstances( self, r ) :

		ass = self._getMainAssembly( r )
		return len( ass.assembly_instances() )
