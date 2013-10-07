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

import IECoreGL
IECoreGL.init( False )

class TestSelection( unittest.TestCase ) :

	def testSelect( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.setAttribute( "name", IECore.StringData( "one" ) )
			r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) } )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "two" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 2, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "three" ) )
			r.geometry( "sphere", {}, {} )

		s = r.scene()
		s.setCamera( IECoreGL.PerspectiveCamera() )

		ss = s.select( IECoreGL.Selector.Mode.GLSelect, IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		names = [ x.name.value() for x in ss ]
		self.assertEqual( len( names ), 3 )
		self.assert_( "one" in names )
		self.assert_( "two" in names )
		self.assert_( "three" in names )

	def testRegionSelect( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) } )
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -2, -2, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "red" ) )
			r.geometry( "sphere", {}, {} )

			r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 0, 1, 0 ) ) } )
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 4, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "green" ) )
			r.geometry( "sphere", {}, {} )

			r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 0, 0, 1 ) ) } )
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 4, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "blue" ) )
			r.geometry( "sphere", {}, {} )

			r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 1, 1 ) ) } )
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, -4, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "white" ) )
			r.geometry( "sphere", {}, {} )

		s = r.scene()
		s.setCamera( IECoreGL.PerspectiveCamera() )

		ss = s.select( IECoreGL.Selector.Mode.GLSelect, IECore.Box2f( IECore.V2f( 0, 0.5 ), IECore.V2f( 0.5, 1 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "red" )

		ss = s.select( IECoreGL.Selector.Mode.GLSelect, IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 0.5 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "green" )

		ss = s.select( IECoreGL.Selector.Mode.GLSelect, IECore.Box2f( IECore.V2f( 0.5, 0 ), IECore.V2f( 1, 0.5 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "blue" )

		ss = s.select( IECoreGL.Selector.Mode.GLSelect, IECore.Box2f( IECore.V2f( 0.5 ), IECore.V2f( 1 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "white" )

	def testIDSelect( self ) :
	
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
			
		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "frontLeft" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			r.setAttribute( "name", IECore.StringData( "backLeft" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 2, 0, 1 ) ) )
			r.setAttribute( "name", IECore.StringData( "frontRight" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			r.setAttribute( "name", IECore.StringData( "backRight" ) )
			r.geometry( "sphere", {}, {} )
			
		s = r.scene()
		s.setCamera( IECoreGL.OrthographicCamera() )

		ss = s.select( IECoreGL.Selector.Mode.IDRender, IECore.Box2f( IECore.V2f( 0.25, 0.5 ), IECore.V2f( 0.26, 0.51 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "frontLeft" )

		ss = s.select( IECoreGL.Selector.Mode.IDRender, IECore.Box2f( IECore.V2f( 0.75, 0.5 ), IECore.V2f( 0.76, 0.51 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "frontRight" )

	def testIDSelectDepths( self ) :
	
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
			
		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.setAttribute( "name", IECore.StringData( "ball" ) )
			r.geometry( "sphere", {}, {} )
			
		scene = r.scene()
		scene.setCamera( IECoreGL.OrthographicCamera() )

		s1 = scene.select( IECoreGL.Selector.Mode.GLSelect, IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		self.assertEqual( len( s1 ), 1 )
		self.assertEqual( s1[0].name.value(), "ball" )

		s2 = scene.select( IECoreGL.Selector.Mode.IDRender, IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		self.assertEqual( len( s2 ), 1 )
		self.assertEqual( s2[0].name.value(), "ball" )
		
		self.assertAlmostEqual( s1[0].depthMin, s2[0].depthMin, 5 )
	
	def testOcclusionQuerySelect( self ) :
	
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
			
		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "frontLeft" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			r.setAttribute( "name", IECore.StringData( "backLeft" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 2, 0, 1 ) ) )
			r.setAttribute( "name", IECore.StringData( "frontRight" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			r.setAttribute( "name", IECore.StringData( "backRight" ) )
			r.geometry( "sphere", {}, {} )
			
		s = r.scene()
		s.setCamera( IECoreGL.OrthographicCamera() )

		ss = s.select( IECoreGL.Selector.Mode.OcclusionQuery, IECore.Box2f( IECore.V2f( 0, 0 ), IECore.V2f( 0.25, 1 ) ) )
		self.assertEqual( len( ss ), 2 )
		self.assertEqual( set( [ x.name.value() for x in ss ] ), set( ( "frontLeft", "backLeft" ) ) )

		ss = s.select( IECoreGL.Selector.Mode.OcclusionQuery, IECore.Box2f( IECore.V2f( 0.75, 0 ), IECore.V2f( 1, 1 ) ) )
		self.assertEqual( len( ss ), 2 )
		self.assertEqual( set( [ x.name.value() for x in ss ] ), set( ( "frontRight", "backRight" ) ) )
	
	def testIDSelectWithAdditionalDisplayStyles( self ) :
	
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
			
		with IECore.WorldBlock( r ) :

			r.setAttribute( "gl:primitive:wireframe", IECore.BoolData( True ) )
			r.setAttribute( "gl:primitive:bound", IECore.BoolData( True ) )
			r.setAttribute( "gl:primitive:outline", IECore.BoolData( True ) )
			r.setAttribute( "gl:primitive:points", IECore.BoolData( True ) )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "frontLeft" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			r.setAttribute( "name", IECore.StringData( "backLeft" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 2, 0, 1 ) ) )
			r.setAttribute( "name", IECore.StringData( "frontRight" ) )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -1 ) ) )
			r.setAttribute( "name", IECore.StringData( "backRight" ) )
			r.geometry( "sphere", {}, {} )
			
		s = r.scene()
		s.setCamera( IECoreGL.OrthographicCamera() )

		ss = s.select( IECoreGL.Selector.Mode.IDRender, IECore.Box2f( IECore.V2f( 0.25, 0.5 ), IECore.V2f( 0.26, 0.51 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "frontLeft" )

		ss = s.select( IECoreGL.Selector.Mode.IDRender, IECore.Box2f( IECore.V2f( 0.75, 0.5 ), IECore.V2f( 0.76, 0.51 ) ) )
		self.assertEqual( len( ss ), 1 )
		self.assertEqual( ss[0].name.value(), "frontRight" )
	
	def testPointsPrimitiveSelect( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.setAttribute( "name", IECore.StringData( "pointsNeedSelectingToo" ) )           
			r.points( 1, { "P" : IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, IECore.V3fVectorData( [ IECore.V3f( 0 ) ] ) ) } )

		s = r.scene()
		s.setCamera( IECoreGL.PerspectiveCamera() )

		for mode in ( IECoreGL.Selector.Mode.GLSelect, IECoreGL.Selector.Mode.OcclusionQuery, IECoreGL.Selector.Mode.IDRender ) :
			ss = s.select( mode, IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
			names = [ x.name.value() for x in ss ]
			self.assertEqual( len( names ), 1 )
			self.assertEqual( names[0], "pointsNeedSelectingToo" )
	
	def testContextManager( self ) :
	
		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.setAttribute( "name", IECore.StringData( "one" ) )
			r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) } )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "two" ) )
			r.geometry( "sphere", {}, {} )

		scene = r.scene()
		scene.setCamera( None )
		
		IECoreGL.PerspectiveCamera().render( IECoreGL.State.defaultState() )
		
		hits = []
		with IECoreGL.Selector( IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ), IECoreGL.Selector.Mode.IDRender, hits ) as selector :
			IECoreGL.State.bindBaseState()
			selector.baseState().bind()
			scene.root().render( selector.baseState() )
				
		names = [ x.name.value() for x in hits ]
		self.assertEqual( len( names ), 2 )
		self.assert_( "one" in names )
		self.assert_( "two" in names )

	def testSelectableFlag( self ) :

		r = IECoreGL.Renderer()
		r.setOption( "gl:mode", IECore.StringData( "deferred" ) )
		r.setOption( "gl:searchPath:shader", IECore.StringData( os.path.dirname( __file__ ) + "/shaders" ) )

		with IECore.WorldBlock( r ) :

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -5 ) ) )

			r.setAttribute( "name", IECore.StringData( "selectableObj" ) )
			r.shader( "surface", "color", { "colorValue" : IECore.Color3fData( IECore.Color3f( 1, 0, 0 ) ) } )
			r.geometry( "sphere", {}, {} )

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( -1, 0, 0 ) ) )
			r.setAttribute( "name", IECore.StringData( "unselectableObj" ) )
			r.setAttribute( "gl:primitive:selectable", IECore.BoolData( False ) )

			r.geometry( "sphere", {}, {} )

		s = r.scene()
		s.setCamera( IECoreGL.PerspectiveCamera() )

		ss = s.select( IECoreGL.Selector.Mode.GLSelect, IECore.Box2f( IECore.V2f( 0 ), IECore.V2f( 1 ) ) )
		names = [ x.name.value() for x in ss ]
		self.assertEqual( names, [ "selectableObj" ] )
				
if __name__ == "__main__":
    unittest.main()
