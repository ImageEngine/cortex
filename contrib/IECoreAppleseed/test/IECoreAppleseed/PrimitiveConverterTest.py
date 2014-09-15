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
import unittest

import appleseed

import IECore
import IECoreAppleseed

class PrimitiveConverterTest( unittest.TestCase ):

    def testSingleMaterial( self ) :

        r = IECoreAppleseed.Renderer()
        r.worldBegin()

        self.__createDefaultShader( r )

        m1 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
        m2 = m1.copy()
        m3 = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -2 ), IECore.V2f( 2 ) ) )

        m1.render( r )
        m2.render( r )
        m3.render( r )

        self.failUnless( self.__countAssemblies( r ) == 2 )
        self.failUnless( self.__countAssemblyInstances( r ) == 3 )

    def testMultipleMaterialsNoInstancing( self ) :

        r = IECoreAppleseed.Renderer()
        r.worldBegin()

        self.__createDefaultShader( r )

        m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
        m.render( r )

        r.attributeBegin()
        r.setAttribute( "as:shading_samples", IECore.IntData( 4 ) )
        m.render( r )
        r.attributeEnd()

        r.attributeBegin()
        r.setAttribute( "as:alpha_map", IECore.StringData( "no_such_file.exr" ) )
        m.render( r )
        r.attributeEnd()

        self.failUnless( self.__countAssemblies( r ) == 3 )
        self.failUnless( self.__countAssemblyInstances( r ) == 3 )


    def __createDefaultShader( self, r ) :

        s = IECore.Shader( "data/shaders/matte.oso", "surface" )
        s.render( r )

    def __getMainAssembly( self, r ) :

        proj = r.appleseedProject()
        scn = proj.get_scene()
        return scn.assemblies().get_by_name( "assembly" )

    def __countAssemblies( self, r ) :

        ass = self.__getMainAssembly( r )
        return len( ass.assemblies() )

    def __countAssemblyInstances( self, r ) :

        ass = self.__getMainAssembly( r )
        return len( ass.assembly_instances() )

    def __writeAppleseedProject( self, r, filename ) :

        proj = r.appleseedProject()
        writer = appleseed.ProjectFileWriter()
        writer.write( proj, filename, appleseed.ProjectFileWriterOptions.OmitWritingGeometryFiles | appleseed.ProjectFileWriterOptions.OmitBringingAssets )

if __name__ == "__main__":
    unittest.main()
