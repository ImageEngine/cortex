##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

class TestPrimitive( unittest.TestCase ) :

	def testStateComponentsInstantiation( self ):

		IECoreGL.Primitive.DrawBound( False )
		IECoreGL.Primitive.DrawWireframe( True )
		IECoreGL.Primitive.WireframeWidth( 1.2 )
		IECoreGL.Primitive.DrawSolid( True )
		IECoreGL.Primitive.DrawOutline( True )
		IECoreGL.Primitive.OutlineWidth( 2.2 )
		IECoreGL.Primitive.DrawPoints( True )
		IECoreGL.Primitive.PointWidth( 2.2 )
		IECoreGL.Primitive.TransparencySort( True )
		IECoreGL.Primitive.Selectable( False )

	def testStateComponentsUsage( self ):

		g = IECoreGL.Group()
		g.getState().add( IECoreGL.Primitive.DrawBound( False ) )
		g.getState().add( IECoreGL.Primitive.DrawWireframe( True ) )
		g.getState().add( IECoreGL.Primitive.WireframeWidth( 1.2 ) )
		g.getState().add( IECoreGL.Primitive.DrawSolid( True ) )
		g.getState().add( IECoreGL.Primitive.DrawOutline( True ) )
		g.getState().add( IECoreGL.Primitive.OutlineWidth( 2.2 ) )
		g.getState().add( IECoreGL.Primitive.DrawPoints( True ) )
		g.getState().add( IECoreGL.Primitive.PointWidth( 2.2 ) )
		g.getState().add( IECoreGL.Primitive.TransparencySort( True ) )
		g.getState().add( IECoreGL.Primitive.Selectable( False ) )

	def testRunTimeTyped( self ) :

		self.assertTrue( IECore.RunTimeTyped.inheritsFrom( IECoreGL.Primitive.staticTypeId(), IECoreGL.Renderable.staticTypeId() ) )

if __name__ == "__main__":
    unittest.main()
