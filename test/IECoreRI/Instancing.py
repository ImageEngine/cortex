##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
from IECore import *
import IECoreRI
import os

class InstancingTest( IECoreRI.TestCase ) :

	def test( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing.rib" )

		r.instanceBegin( "myObject", {} )
		r.geometry( "teapot", {}, {} )
		r.instanceEnd()

		r.worldBegin()

		r.instance( "myObject" )
		r.worldEnd()

		rib = "".join( open( "test/IECoreRI/output/instancing.rib" ).readlines() )

		self.assert_( "ObjectBegin" in rib )
		self.assert_( "ObjectEnd" in rib )
		self.assert_( "ObjectInstance" in rib )

	def test2( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing.rib" )

		r.instanceBegin( "myObject", {} )
		r.geometry( "teapot", {}, {} )
		r.instanceEnd()

		r.worldBegin()

		r.instance( "myObject" )
		r.worldEnd()

		rib = "".join( open( "test/IECoreRI/output/instancing.rib" ).readlines() )

		self.assert_( "ObjectBegin" in rib )
		self.assert_( "ObjectEnd" in rib )
		self.assert_( "ObjectInstance" in rib )

	def testInstancingAPI( self ) :

		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing3.rib" )

		r.instanceBegin( "myObject", {} )
		r.geometry( "teapot", {}, {} )
		r.instanceEnd()

		r.worldBegin()

		r.instance( "myObject" )

		r.worldEnd()

		rib = "".join( open( "test/IECoreRI/output/instancing3.rib" ).readlines() )

		self.assert_( "ObjectBegin" in rib )
		self.assert_( "ObjectEnd" in rib )
		self.assert_( "ObjectInstance" in rib )
		
	def testInstancingAcrossProcedurals( self ) :
			
		class InstanceInheritingProcedural( Renderer.Procedural ) :
		
			def __init__( self, root = True ) :
			
				Renderer.Procedural.__init__( self )
				
				self.__root = root
				
			def bound( self ) :
			
				return Box3f( V3f( -1 ), V3f( 1 ) )
			
			def render( self, renderer ) :
			
				if self.__root :
					renderer.instanceBegin( "myLovelySphere", {} )
					renderer.sphere( 1, -1, 1, 360, {} )
					renderer.instanceEnd()
					renderer.procedural( InstanceInheritingProcedural( False ) )
				else :
					renderer.instance( "myLovelySphere" )
		
			def hash( self ) :
			
				h = MurmurHash()
				return h
			
		# test writing a rib
		
		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing.rib" )
		with WorldBlock( r ) :		
			r.procedural( InstanceInheritingProcedural() )

		rib = "".join( open( "test/IECoreRI/output/instancing.rib" ).readlines() )
		self.failUnless( "ObjectInstance \"myLovelySphere\"" in rib )

		# test doing an actual render

		r = IECoreRI.Renderer( "" )
		r.display( "test", "ie", "rgba",
			{
				"driverType" : StringData( "ImageDisplayDriver" ),
				"handle" : StringData( "myLovelySphere" ),
				"quantize" : FloatVectorData( [ 0, 0, 0, 0 ] ),
			}
		)
		
		with WorldBlock( r ) :		
			r.concatTransform( M44f.createTranslated( V3f( 0, 0, -10 ) ) )
			r.procedural( InstanceInheritingProcedural() )

		image = ImageDisplayDriver.removeStoredImage( "myLovelySphere" )
		e = ImagePrimitiveEvaluator( image )
		r = e.createResult()
		e.pointAtUV( V2f( 0.5 ), r )
		self.failUnless( r.floatPrimVar( image["R"] ) > 0.95 )
		
	def testInstancingWithThreadedProcedurals( self ) :
			
		class InstanceMakingProcedural( Renderer.Procedural ) :
		
			def __init__( self, instanceName ) :
			
				Renderer.Procedural.__init__( self )
				
				self.__instanceName = instanceName
				
			def bound( self ) :
			
				return Box3f( V3f( -10 ), V3f( 10 ) )
				
			def render( self, renderer ) :
			
				renderer.instanceBegin( self.__instanceName, {} )
				renderer.sphere( 1, -1, 1, 360, {} )
				renderer.instanceEnd()
				
				renderer.instance( self.__instanceName )
			
			def hash( self ) :
				
				h = MurmurHash()
				return h
				
		initThreads()
		r = IECoreRI.Renderer( "" )
		
		with WorldBlock( r ) :
		
			r.concatTransform( M44f.createTranslated( V3f( 0, 0, -20 ) ) )
		
			for i in range( 0, 100 ) :
				r.procedural( InstanceMakingProcedural( "instance%d" % i ) )
	
	def testProceduralLevelInstancing( self ) :
		
		if IECoreRI.withRiProceduralV():

			class InstanceTestProcedural( Renderer.Procedural ) :

				renderCount = 0

				def __init__( self, instanceHash ) :

					Renderer.Procedural.__init__( self )

					self.__instanceHash = instanceHash

				def bound( self ) :

					return Box3f( V3f( -10 ), V3f( 10 ) )

				def render( self, renderer ) :
					InstanceTestProcedural.renderCount = InstanceTestProcedural.renderCount + 1
					pass

				def hash( self ) :
					return self.__instanceHash

			r = IECoreRI.Renderer("")

			# give it a camera using the ray trace hider, and turn shareinstances on:
			r.camera( "main", {
				"resolution" : V2iData( V2i( 1024, 200 ) ),
				"screenWindow" : Box2fData( Box2f( V2f( -1 ), V2f( 1 ) ) ),
				"cropWindow" : Box2fData( Box2f( V2f( 0.1, 0.1 ), V2f( 0.9, 0.9 ) ) ),
				"clippingPlanes" : V2fData( V2f( 1, 1000 ) ),
				"projection" : StringData( "perspective" ),
				"projection:fov" : FloatData( 45 ),
				"ri:hider" : StringData( "raytrace" ),
			} )
			r.setOption( "ri:trace:shareinstances", IntData( 1 ) )
			
			# chuck a couple of procedurals at it:
			h1 = MurmurHash()
			h2 = MurmurHash()
			
			h1.append( "instance1" )
			h2.append( "instance2" )
			
			with WorldBlock( r ) :
				r.procedural( InstanceTestProcedural(h1) )
				r.procedural( InstanceTestProcedural(h1) )
				r.procedural( InstanceTestProcedural(h2) )
				r.procedural( InstanceTestProcedural(h2) )

			# only two unique instances here, as there were 2 unique hashes....
			self.assertEqual( InstanceTestProcedural.renderCount, 2 )
			
			InstanceTestProcedural.renderCount = 0
			
			# the system shouldn't perform instancing when the hash method returns empty hashes:
			with WorldBlock( r ) :
				r.procedural( InstanceTestProcedural( MurmurHash() ) )
				r.procedural( InstanceTestProcedural( MurmurHash() ) )
				r.procedural( InstanceTestProcedural( MurmurHash() ) )
				r.procedural( InstanceTestProcedural( MurmurHash() ) )
			
			self.assertEqual( InstanceTestProcedural.renderCount, 4 )

		
	def testParameterisedProceduralInstancing( self ) :
	
		if IECoreRI.withRiProceduralV():

			class InstanceTestParamProcedural( ParameterisedProcedural ) :
				
				renderCount = 0

				def __init__( self ) :
					
					ParameterisedProcedural.__init__( self, "Instancing test" )
					
					self.parameters().addParameters(
			
						[
							BoolParameter(
								name = "p1",
								description = "whatever.",
								defaultValue = True,
							),

							StringParameter(
								name = "p2",
								description = "yup.",
								defaultValue = "blah"
							),
						]

					)
				
				def doBound( self, args ) :

					return Box3f( V3f( -10 ), V3f( 10 ) )

				def doRender( self, renderer, args ) :
					InstanceTestParamProcedural.renderCount = InstanceTestParamProcedural.renderCount + 1
					pass
			
			r = IECoreRI.Renderer("")
			
			# give it a camera using the ray trace hider, and turn shareinstances on:
			r.camera( "main", {
				"resolution" : V2iData( V2i( 1024, 200 ) ),
				"screenWindow" : Box2fData( Box2f( V2f( -1 ), V2f( 1 ) ) ),
				"cropWindow" : Box2fData( Box2f( V2f( 0.1, 0.1 ), V2f( 0.9, 0.9 ) ) ),
				"clippingPlanes" : V2fData( V2f( 1, 1000 ) ),
				"projection" : StringData( "perspective" ),
				"projection:fov" : FloatData( 45 ),
				"ri:hider" : StringData( "raytrace" ),
			} )
			r.setOption( "ri:trace:shareinstances", IntData( 1 ) )
			
			# chuck a couple of procedurals at it:
			
			proc1 = InstanceTestParamProcedural()
			proc2 = InstanceTestParamProcedural()
			
			proc1["p1"].setValue( False )
			proc1["p2"].setValue( StringData( "humpf" ) )
			
			with WorldBlock( r ) :
				
				proc1.render( r )
				proc2.render( r )
				proc2.render( r )
				proc2.render( r )
			
			# only two unique instances here....
			self.assertEqual( InstanceTestParamProcedural.renderCount, 2 )

	def testAutomaticInstancing( self ) :
	
		m = MeshPrimitive.createPlane( Box2f( V2f( -1 ), V2f( 1 ) ) )
		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing.rib" )
		
		with WorldBlock( r ) :
			m.render( r )
			m.render( r )
			
		rib = "".join( open( "test/IECoreRI/output/instancing.rib" ).readlines() )
		self.assertEqual( rib.count( "ObjectBegin" ), 0 )
		self.assertEqual( rib.count( "PointsGeneralPolygons" ), 2 )
		
		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing.rib" )
		with WorldBlock( r ) :
			r.setAttribute( "ri:automaticInstancing", True )
			m.render( r )
			m.render( r )
			
		rib = "".join( open( "test/IECoreRI/output/instancing.rib" ).readlines() )
		self.assertEqual( rib.count( "ObjectBegin" ), 1 )
		self.assertEqual( rib.count( "PointsGeneralPolygons" ), 1 )
		self.assertEqual( rib.count( "ObjectInstance" ), 2 )

	def testAutomaticInstancingWithMotionBlur( self ) :
	
		m = MeshPrimitive.createPlane( Box2f( V2f( -1 ), V2f( 1 ) ) )
		m2 = MeshPrimitive.createPlane( Box2f( V2f( -2 ), V2f( 2 ) ) )
		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing.rib" )
		
		with WorldBlock( r ) :

			r.setAttribute( "ri:automaticInstancing", True )

			with MotionBlock( r, [ 0, 1 ] ) :
				m.render( r )
				m2.render( r )
			with MotionBlock( r, [ 0, 1 ] ) :
				m.render( r )
				m2.render( r )
			
		rib = "".join( open( "test/IECoreRI/output/instancing.rib" ).readlines() )
		self.assertEqual( rib.count( "ObjectBegin" ), 1 )
		self.assertEqual( rib.count( "PointsGeneralPolygons" ), 2 )
		self.assertEqual( rib.count( "ObjectInstance" ), 2 )
	
	
	def testAutomaticInstancingWithTransformMotionBlur( self ) :
	
		m = MeshPrimitive.createPlane( Box2f( V2f( -1 ), V2f( 1 ) ) )
		r = IECoreRI.Renderer( "test/IECoreRI/output/instancing.rib" )
		
		with WorldBlock( r ) :

			r.setAttribute( "ri:automaticInstancing", True )
			
			with TransformBlock( r ):
			
				with MotionBlock( r, [ 0, 1 ] ) :
					r.setTransform( M44f.createTranslated( V3f( 0,0,0 ) ) )
					r.setTransform( M44f.createTranslated( V3f( 1,0,0 ) ) )
				m.render( r )
			
			with TransformBlock( r ):
			
				with MotionBlock( r, [ 0, 1 ] ) :
					r.setTransform( M44f.createTranslated( V3f( 0,0,0 ) ) )
					r.setTransform( M44f.createTranslated( V3f( 1,0,0 ) ) )
				m.render( r )
			
		rib = "".join( open( "test/IECoreRI/output/instancing.rib" ).readlines() )
		self.assertEqual( rib.count( "ObjectBegin" ), 1 )
		self.assertEqual( rib.count( "PointsGeneralPolygons" ), 1 )
		self.assertEqual( rib.count( "ObjectInstance" ), 2 )
	
	def testAutomaticInstancingWithThreadedProcedurals( self ) :
	
		class PlaneProcedural( Renderer.Procedural ) :
		
			def __init__( self ) :
			
				Renderer.Procedural.__init__( self )
			
			def bound( self ) :
			
				return Box3f( V3f( -10, -10, -0.01 ), V3f( 10, 10, 0.01 ) )
				
			def render( self, renderer ) :
			
				MeshPrimitive.createPlane( Box2f( V2f( -10 ), V2f( 10 ) ) ).render( renderer )
				
			def hash( self ) :
			
				h = MurmurHash()
				return h
			
		initThreads()
		r = IECoreRI.Renderer( "" )
		
		with WorldBlock( r ) :
		
			r.setAttribute( "ri:automaticInstancing", True )
			r.concatTransform( M44f.createTranslated( V3f( 0, 0, -20 ) ) )
		
			for i in range( 0, 1000 ) :
				r.procedural( PlaneProcedural() )


if __name__ == "__main__":
    unittest.main()
