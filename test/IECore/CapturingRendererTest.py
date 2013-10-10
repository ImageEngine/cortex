##########################################################################
#
#  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

import threading
import unittest
import sys

import IECore

class CapturingRendererTest( unittest.TestCase ) :
	
	def testNoWorldYet( self ) :
	
		r = IECore.CapturingRenderer()
		self.assertRaises( RuntimeError, r.world )

	def testUserAttribute( self ) :

		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.setAttribute( "user:whatever", IECore.IntData( 20 ) )
			
		w = r.world()
		self.failUnless( isinstance( w, IECore.Group ) )
		
		wc = w.children()
		self.assertEqual( len( wc ), 0 )
		
		ws = w.state()
		self.assertEqual( len( ws ), 1 )
		self.failUnless( isinstance( ws[0], IECore.AttributeState ) )
		self.assertEqual( ws[0].attributes["user:whatever"], IECore.IntData( 20 ) )
	
	def testSimpleAttributesAndPrimitives( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.setAttribute( "a", IECore.IntData( 1 ) )
			r.sphere( 1, -1, 1, 360, {} )
			r.sphere( 1, -1, 1, 360, {} )
		
		w = r.world()
		
		self.assertEqual( w.getTransform(), None )
		self.assertEqual( len( w.children() ), 2 )
		self.assertEqual( len( w.state() ), 1 )
		
		self.failUnless( isinstance( w.children()[0], IECore.SpherePrimitive ) ) 
		self.failUnless( isinstance( w.children()[1], IECore.SpherePrimitive ) ) 
		self.assertEqual( w.state()[0].attributes["a"], IECore.IntData( 1 ) )
				
	def testInterleavedAttributesAndPrimitives( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.setAttribute( "a", IECore.IntData( 1 ) )
			r.sphere( 1, -1, 1, 360, {} )
			
			r.setAttribute( "a", IECore.IntData( 2 ) )
			r.sphere( 1, -1, 1, 360, {} )
			
		w = r.world()
		
		# we don't want state at the top level, as it would leak onto
		# both child objects
		self.assertEqual( len( w.state() ), 0 )			
		self.assertEqual( w.getTransform(), None )

		c = w.children()
		self.assertEqual( len( c ), 2 )
		
		self.failUnless( isinstance( c[0], IECore.Group ) )
		self.assertEqual( c[0].getTransform(), None )
		self.assertEqual( c[0].state()[0].attributes["a"], IECore.IntData( 1 ) )
		self.assertEqual( len( c[0].children() ), 1 )
		self.failUnless( isinstance( c[0].children()[0], IECore.SpherePrimitive ) )
		
		self.failUnless( isinstance( c[1], IECore.Group ) )
		self.assertEqual( c[1].getTransform(), None )
		self.assertEqual( c[1].state()[0].attributes["a"], IECore.IntData( 2 ) )
		self.assertEqual( len( c[1].children() ), 1 )
		self.failUnless( isinstance( c[1].children()[0], IECore.SpherePrimitive ) )

	def testNestedGroups( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
			with IECore.AttributeBlock( r ) :
				r.sphere( 1, -1, 1, 360, {} )
				
		w = r.world()
				
		self.assertEqual( len( w.state() ), 0 )
		self.assertEqual( w.getTransform(), None )
		
		c = w.children()
		self.assertEqual( len( c ), 1 )
		self.failUnless( isinstance( c[0], IECore.Group ) )
		self.assertEqual( c[0].getTransform(), None )
		self.assertEqual( len( c[0].state() ), 0 )

		c = c[0].children()
		self.assertEqual( len( c ), 1 )
		self.failUnless( isinstance( c[0], IECore.SpherePrimitive ) )
		
	def testNestedGroupsWithAttributes( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
			r.setAttribute( "user:something", IECore.IntData( 10 ) )
			with IECore.AttributeBlock( r ) :
				r.setAttribute( "user:something", IECore.IntData( 20 ) )
				r.sphere( 1, -1, 1, 360, {} )
				
		w = r.world()
				
		self.assertEqual( w.getTransform(), None )
		self.assertEqual( len( w.state() ), 1 )
		self.assertEqual( w.state()[0].attributes["user:something"], IECore.IntData( 10 ) )
		
		c = w.children()
		self.assertEqual( len( c ), 1 )
		self.failUnless( isinstance( c[0], IECore.Group ) )
		
		self.assertEqual( c[0].getTransform(), None )
		self.assertEqual( len( c[0].state() ), 1 )
		self.assertEqual( c[0].state()[0].attributes["user:something"], IECore.IntData( 20 ) )
		
		c = c[0].children()
		self.assertEqual( len( c ), 1 )
		self.failUnless( isinstance( c[0], IECore.SpherePrimitive ) )
		
	def testNestedGroupsWithInterleavedAttributes( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
			r.setAttribute( "user:something", IECore.IntData( 10 ) )
			with IECore.AttributeBlock( r ) :
				r.sphere( 1, -1, 1, 360, {} )
			r.setAttribute( "user:something", IECore.IntData( 20 ) )
			with IECore.AttributeBlock( r ) :
				r.sphere( 1, -1, 1, 360, {} )	
				
		w = r.world()
				
		self.assertEqual( len( w.state() ), 0 )
		self.assertEqual( w.getTransform(), None )
		
		c = w.children()
		self.assertEqual( len( c ), 2 )
		self.failUnless( isinstance( c[0], IECore.Group ) )
		self.failUnless( isinstance( c[1], IECore.Group ) )
		
		self.assertEqual( c[0].getTransform(), None )
		self.assertEqual( len( c[0].state() ), 1 )
		self.assertEqual( c[0].state()[0].attributes["user:something"], IECore.IntData( 10 ) )
		
		self.assertEqual( c[1].getTransform(), None )
		self.assertEqual( len( c[1].state() ), 1 )
		self.assertEqual( c[1].state()[0].attributes["user:something"], IECore.IntData( 20 ) )
		
		c0 = c[0].children()[0]
		self.assertEqual( c0.getTransform(), None )
		self.assertEqual( len( c0.state() ), 0 )
		self.assertEqual( len( c0.children() ), 1 )
		self.failUnless( isinstance( c0.children()[0], IECore.SpherePrimitive ) )
		
		c1 = c[1].children()[0]
		self.assertEqual( c1.getTransform(), None )
		self.assertEqual( len( c1.state() ), 0 )
		self.assertEqual( len( c1.children() ), 1 )
		self.failUnless( isinstance( c1.children()[0], IECore.SpherePrimitive ) )
		
	def testGetAttribute( self ) :
	
		r = IECore.CapturingRenderer()
	
		with IECore.WorldBlock( r ) :
		
			self.assertEqual( r.getAttribute( "iDontExist" ), None )
			
			r.setAttribute( "user:something", IECore.IntData( 1 ) )
			
			self.assertEqual( r.getAttribute( "user:something" ), IECore.IntData( 1 ) )
			
			with IECore.AttributeBlock( r ) :
			
				self.assertEqual( r.getAttribute( "user:something" ), IECore.IntData( 1 ) )
				r.setAttribute( "user:something", IECore.IntData( 2 ) )
				self.assertEqual( r.getAttribute( "user:something" ), IECore.IntData( 2 ) )
				
				r.setAttribute( "user:somethingElse", IECore.IntData( 20 ) )
				self.assertEqual( r.getAttribute( "user:somethingElse" ), IECore.IntData( 20 ) )
							
			self.assertEqual( r.getAttribute( "user:something" ), IECore.IntData( 1 ) )
			self.assertEqual( r.getAttribute( "user:somethingElse" ), None )

	def testTransformStack( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			self.assertEqual( r.getTransform(), IECore.M44f() )
			
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 1, 0 ) ) )
			self.assertEqual( r.getTransform(), IECore.M44f.createTranslated( IECore.V3f( 0, 1, 0 ) ) )
			
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 0, 0 ) ) )
			self.assertEqual( r.getTransform(), IECore.M44f.createTranslated( IECore.V3f( 1, 1, 0 ) ) )
			
			r.setTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
			self.assertEqual( r.getTransform(), IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
			
			with IECore.TransformBlock( r ) :
			
				r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 1, 0 ) ) )
				self.assertEqual( r.getTransform(), IECore.M44f.createTranslated( IECore.V3f( 1, 1, 1 ) ) )

			self.assertEqual( r.getTransform(), IECore.M44f.createTranslated( IECore.V3f( 0, 0, 1 ) ) )
			
	def testTransforms( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 0, 0 ) ) )
			
			r.sphere( 1, -1, 1, 360, {} )	
		
		w = r.world()
		
		self.assertEqual( w.getTransform().matrix, IECore.M44f.createTranslated( IECore.V3f( 1, 0, 0 ) ) )
		self.failUnless( isinstance( w.children()[0], IECore.SpherePrimitive ) )
		
	def testInterleavedTransformsAndPrimitives( self ) :
	
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 0, 0 ) ) )
			
			r.sphere( 1, -1, 1, 360, {} )	

			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 1, 0, 0 ) ) )

			r.sphere( 1, -1, 1, 360, {} )	
		
		w = r.world()
		
		self.assertEqual( w.getTransform(), None )
		
		c = w.children()
		self.assertEqual( len( c ), 2 )
		self.failUnless( isinstance( c[0], IECore.Group ) )
		self.failUnless( isinstance( c[1], IECore.Group ) )
		self.assertEqual( c[0].getTransform().matrix, IECore.M44f.createTranslated( IECore.V3f( 1, 0, 0 ) ) )
		self.assertEqual( c[1].getTransform().matrix, IECore.M44f.createTranslated( IECore.V3f( 2, 0, 0 ) ) )
		self.failUnless( isinstance( c[0].children()[0], IECore.SpherePrimitive ) )
		self.failUnless( isinstance( c[1].children()[0], IECore.SpherePrimitive ) )
	
	class SnowflakeProcedural( IECore.Renderer.Procedural ) :
	
		threadUse = {}
	
		def __init__( self, maxLevel = 3, level = 0 ) :
		
			IECore.Renderer.Procedural.__init__( self )
			
			self.__maxLevel = maxLevel
			self.__level = level
			
		def bound( self ) :
		
			return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )
			
		def render( self, renderer ) :
			
			threadID = threading.currentThread().getName()
			useCount = CapturingRendererTest.SnowflakeProcedural.threadUse.get( threadID, 0 )
			useCount += 1
			CapturingRendererTest.SnowflakeProcedural.threadUse[threadID] = useCount
			
			def emit( offset ) :
						
				with IECore.AttributeBlock( renderer ) :

					renderer.concatTransform( IECore.M44f.createTranslated( offset ) )
					renderer.concatTransform( IECore.M44f.createScaled( IECore.V3f( 0.5 ) ) )

					renderer.procedural( CapturingRendererTest.SnowflakeProcedural( self.__maxLevel, self.__level + 1 ) )
					
			if self.__level < self.__maxLevel :

				emit( IECore.V3f( 0, 1, 0 ) )
				emit( IECore.V3f( -1, 0, 0 ) )
				emit( IECore.V3f( 0, 0, 0 ) )
				emit( IECore.V3f( 1, 0, 0 ) )
				emit( IECore.V3f( 0, -1, 0 ) )

			else :

				renderer.sphere( 1, -1, 1, 360, {} )
			
		def hash( self ):
		
			h = IECore.MurmurHash()
			return h
	
	def testTopLevelProceduralThreading( self ) :
	
		# this is necessary so python will allow threads created by the renderer
		# to enter into python when those threads execute procedurals.
		IECore.initThreads()
		
		self.SnowflakeProcedural.threadUse.clear()
		
		r = IECore.CapturingRenderer()
	
		with IECore.WorldBlock( r ) :
		
			for i in range( 0, 1000 ) :
				r.procedural( self.SnowflakeProcedural( maxLevel = 0 ) )
		
		self.failUnless( len( self.SnowflakeProcedural.threadUse ) > 1 )
		self.assertEqual( sum( self.SnowflakeProcedural.threadUse.values() ), 1000 )
		
		w = r.world()
		self.assertEqual( len( w.state() ), 0 )
		self.assertEqual( len( w.children() ), 1000 )
		
		for c in w.children() :
		
			self.failUnless( isinstance( c, IECore.Group ) )
			self.assertEqual( len( c.children() ), 1 )
			self.failUnless( isinstance( c.children()[0], IECore.SpherePrimitive ) )
			
	def testRecursiveProceduralThreading( self ) :
	
		# this is necessary so python will allow threads created by the renderer
		# to enter into python when those threads execute procedurals.
		IECore.initThreads()
		
		self.SnowflakeProcedural.threadUse.clear()
		
		r = IECore.CapturingRenderer()
	
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.SnowflakeProcedural( maxLevel = 3 ) )
		
		self.failUnless( len( self.SnowflakeProcedural.threadUse ) > 1 )
		self.assertEqual( sum( self.SnowflakeProcedural.threadUse.values() ), 156 )
		
		w = r.world()
		self.assertEqual( len( w.state() ), 0 )
		self.assertEqual( len( w.children() ), 1 )
		
		def checkProceduralGroup( group, level=0 ) :
		
			self.failUnless( isinstance( group, IECore.Group ) )
			if level < 3 :
				self.assertEqual( len( group.children() ), 5 )
				for g in group.children() :
					self.failUnless( isinstance( g.getTransform(), IECore.MatrixTransform ) )
					self.assertEqual( len( g.children() ), 1 )
					checkProceduralGroup( g.children()[0], level + 1 )
			else :
				self.assertEqual( len( group.state() ), 0 )
				self.assertEqual( len( group.children() ), 1 )
				self.failUnless( isinstance( group.children()[0], IECore.SpherePrimitive ) )
				
		checkProceduralGroup( w.children()[0] )
		
		IECore.ObjectWriter( w, "/tmp/flake.cob" ).write()
	
	def testDisableProceduralThreading( self ) :
	
		# this is necessary so python will allow threads created by the renderer
		# to enter into python when those threads execute procedurals.
		IECore.initThreads()
		
		self.SnowflakeProcedural.threadUse.clear()
		
		rThreaded = IECore.CapturingRenderer()
	
		with IECore.WorldBlock( rThreaded ) :
		
			rThreaded.procedural( self.SnowflakeProcedural( maxLevel = 3 ) )
		
		self.failUnless( len( self.SnowflakeProcedural.threadUse ) > 1 )
		self.assertEqual( sum( self.SnowflakeProcedural.threadUse.values() ), 156 )

		self.SnowflakeProcedural.threadUse.clear()
		
		rNonthreaded = IECore.CapturingRenderer()
	
		with IECore.WorldBlock( rNonthreaded ) :
			rNonthreaded.setAttribute( "cp:procedural:reentrant", IECore.BoolData( False ) )		
			rNonthreaded.procedural( self.SnowflakeProcedural( maxLevel = 3 ) )
		
		self.failUnless( len( self.SnowflakeProcedural.threadUse ) == 1 )
		self.assertEqual( sum( self.SnowflakeProcedural.threadUse.values() ), 156 )
	
	def testProceduralExceptionHandling( self ) :
			
		# this is necessary so python will allow threads created by the renderer
		# to enter into python when those threads execute procedurals.
		IECore.initThreads()
		
		class NaughtyLittleChap( IECore.Renderer.Procedural ) :
		
			def __init__( self ) :
			
				IECore.Renderer.Procedural.__init__( self )
				
			def bound( self ) :
			
				return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )
				
			def render( self, renderer ) :
			
				raise RuntimeError
			
			def hash( self ):
			
				h = IECore.MurmurHash()
				return h
			
		r = IECore.CapturingRenderer()
	
		exceptionCaughtOK = False
		with IECore.WorldBlock( r ) :
		
			r.procedural( NaughtyLittleChap() )
			exceptionCaughtOK = True
			
		self.failUnless( exceptionCaughtOK )
		
		# remove the record of the exception so that it doesn't interfere
		# with subsequent tests - the traceback contains a reference to
		# the NaughtyLittleChap.
		sys.last_traceback = None
		IECore.RefCounted.collectGarbage()
		self.assertEqual( IECore.RefCounted.numWrappedInstances(), 0 )
		
	def testOptions( self ) :
	
		r = IECore.CapturingRenderer()
		
		r.setOption( "user:something", IECore.IntData( 1 ) )
		r.setOption( "something", IECore.IntData( 10 ) )
		
		self.assertEqual( r.getOption( "user:something" ), IECore.IntData( 1 ) )
		self.assertEqual( r.getOption( "something" ), IECore.IntData( 10 ) )
		self.assertEqual( r.getOption( "somethingUndefined" ), None )
		
		with IECore.WorldBlock( r ) :
		
			self.assertEqual( r.getOption( "user:something" ), IECore.IntData( 1 ) )
			self.assertEqual( r.getOption( "something" ), IECore.IntData( 10 ) )
			self.assertEqual( r.getOption( "somethingUndefined" ), None )
			
			m = IECore.CapturingMessageHandler()
			with m :
				r.setOption( "naughtyNaughty", IECore.IntData( 1 ) )
			
			self.assertEqual( len( m.messages ), 1 )
			self.assertEqual( m.messages[0].level, IECore.Msg.Level.Warning )
	
	class GetOptionProcedural( IECore.Renderer.Procedural ) :
	
		failure = False
	
		def __init__( self, maxLevel = 4, level = 0 ) :
		
			IECore.Renderer.Procedural.__init__( self )
			
			self.__maxLevel = maxLevel
			self.__level = level
			
		def bound( self ) :
		
			return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )
			
		def render( self, renderer ) :
					
			if self.__level < self.__maxLevel :

				with IECore.AttributeBlock( renderer ) :
					renderer.procedural( CapturingRendererTest.GetOptionProcedural( self.__maxLevel, self.__level + 1 ) )
			
			if renderer.getOption( "iDontExist" ) != None :
				CapturingRendererTest.GetOptionProcedural.failure = True
				
			if renderer.getOption( "user:ten" ) != IECore.IntData( 10 ) :
				CapturingRendererTest.GetOptionProcedural.failure = True
		
		def hash( self ):
		
			h = IECore.MurmurHash()
			return h
	
	def testGetOptionFromThreadedProcedurals( self ) :
	
		# this is necessary so python will allow threads created by the renderer
		# to enter into python when those threads execute procedurals.
		IECore.initThreads()
				
		r = IECore.CapturingRenderer()
	
		r.setOption( "user:ten", IECore.IntData( 10 ) )
	
		self.GetOptionProcedural.failure = False
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.GetOptionProcedural( maxLevel = 6 ) )
		
		self.assertEqual( self.GetOptionProcedural.failure, False )
	
	
	class NamedSnowflakeProcedural( IECore.Renderer.Procedural ) :
	
	
		def __init__( self, maxLevel = 3, level = 0, name="snowflake" ) :
		
			IECore.Renderer.Procedural.__init__( self )
			
			self.__maxLevel = maxLevel
			self.__level = level
			self.__name = name
			
		def bound( self ) :
		
			return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )
			
		def render( self, renderer ) :
						
			def emit( offset, childname, recurse ) :
						
				with IECore.AttributeBlock( renderer ) :

					renderer.concatTransform( IECore.M44f.createTranslated( offset ) )
					renderer.concatTransform( IECore.M44f.createScaled( IECore.V3f( 0.5 ) ) )
					
					newName = self.__name + "/" + childname
					
					if not recurse:
						newName = newName + "_leaf"
					
					renderer.setAttribute( "name", newName )
					
					if recurse:
						renderer.procedural( CapturingRendererTest.NamedSnowflakeProcedural( self.__maxLevel, self.__level + 1, newName ) )
					else:
						renderer.sphere( 1, -1, 1, 360, {} )
						
			
			recurse = self.__level < self.__maxLevel
			
			emit( IECore.V3f( 0, 1, 0 ), "one", recurse )
			emit( IECore.V3f( -1, 0, 0 ), "two", recurse )
			emit( IECore.V3f( 0, 0, 0 ), "three", recurse )
			emit( IECore.V3f( 1, 0, 0 ), "four", recurse )
			emit( IECore.V3f( 0, -1, 0 ), "five", recurse )

		def hash( self ):
		
			h = IECore.MurmurHash()
			return h
		
	def __getSphereNames( self, g, currentName = None ) :
		
		res = set()
		
		if isinstance( g, IECore.Group ):
			
			newName = currentName
			
			for s in g.state():
				if isinstance( s, IECore.AttributeState ):
					if "name" in s.attributes:
						newName = s.attributes["name"].value
			
			for c in g.children():
				if isinstance( c, IECore.Group ):
					res = res | self.__getSphereNames( c, newName )
				elif isinstance( c, IECore.SpherePrimitive ):
					res.add( newName )
		return res
	
	def __countSpheres( self, g ) :
		res = 0
		if isinstance( g, IECore.SpherePrimitive ):
			res = res + 1
		elif isinstance( g, IECore.Group ):
			for c in g.children():
				res = res + self.__countSpheres( c )
		
		return res
		
	def testObjectFilter( self ) :
		
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# the unfiltered procedural should output 125 spheres:
		self.assertEqual( self.__countSpheres( r.world() ), 125 )
		
		
		r.setOption( "cp:objectFilter", IECore.StringVectorData( [ "snowflake/*/*/two_leaf" ] ) )
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# this should output 25 spheres:
		self.assertEqual( self.__countSpheres( r.world() ), 25 )
		
		
		r.setOption( "cp:objectFilter", IECore.StringVectorData( [ "snowflake/one/*/*" ] ) )
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# this should also output 25 spheres:
		self.assertEqual( self.__countSpheres( r.world() ), 25 )
		
		
		r.setOption( "cp:objectFilter", IECore.StringVectorData( [ "snowflake/*/*/t*_leaf" ] ) )
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# this should output 50 spheres:
		self.assertEqual( self.__countSpheres( r.world() ), 50 )
		
		
		# "*" at the end of the filter means all children of the filter.:
		r.setOption( "cp:objectFilter", IECore.StringVectorData( [ "snowflake/one/*" ] ) )
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# this should also output 25 spheres:
		self.assertEqual( self.__countSpheres( r.world() ), 25 )
		
		
		# this shouldn't output any spheres, cause we're just rendering a group:
		r.setOption( "cp:objectFilter", IECore.StringVectorData( [ "snowflake/one" ] ) )
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# this should also output 25 spheres:
		self.assertEqual( self.__countSpheres( r.world() ), 0 )
		
		# test multiple filters:
		r.setOption( "cp:objectFilter", IECore.StringVectorData( [ "snowflake/one/two/t*_leaf", "snowflake/one/three/one_leaf" ] ) )
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# this should output 3 spheres:
		self.assertEqual( self.__countSpheres( r.world() ), 3 )
		
		# check they've got the right names:
		self.assertEqual( self.__getSphereNames( r.world() ), set( [ "snowflake/one/two/two_leaf", "snowflake/one/two/three_leaf", "snowflake/one/three/one_leaf" ] ) )
		


		# test names:
		r.setOption( "cp:objectFilter", IECore.StringVectorData( [ "snowflake/one/t*/t*_leaf" ] ) )
		
		with IECore.WorldBlock( r ) :
		
			r.procedural( self.NamedSnowflakeProcedural( maxLevel = 2 ) )
		
		# check they've got the right names:
		self.assertEqual(
			self.__getSphereNames( r.world() ),
			set(
				[
					"snowflake/one/two/two_leaf",
					"snowflake/one/two/three_leaf",
					"snowflake/one/three/two_leaf",
					"snowflake/one/three/three_leaf"
				]
			)
		)
	
	# convenience method for debugging the following tests if you've broken them:
	def printGroup( self, g, prefix="" ):

		print prefix, g

		if isinstance( g, IECore.Group ):

			print prefix
			for s in g.state():
				if isinstance( s, IECore.AttributeState ):
					print prefix, "attributes:", s.attributes
				elif isinstance( s, IECore.Shader ):
					print prefix, "shader:", s.name, s.type, s.parameters
			print prefix

			for c in g.children():
				self.printGroup( c, prefix + "\t" )
	
	
	def checkStructure( self, g, structure ):

		expectedAttributes = IECore.CompoundData( structure["attributes"] )
		attributeState = IECore.CompoundData()

		expectedShaders = set()
		for sh in structure["shaders"]:
			expectedShaders.add( str( sh ) )
		
		shaderState = set()

		for s in g.state():
			if isinstance( s, IECore.AttributeState ):
				attributeState = s.attributes
			elif isinstance( s, IECore.Shader ):
				shaderState.add( str( ( s.type, s.name, s.parameters ) ) )

		if expectedAttributes != attributeState :
			raise Exception( "Attribute state mismatch!!\n Expected = " + str( expectedAttributes ) + ",\n Actual = " + str( attributeState ) )
		
		if str( expectedShaders ) != str( shaderState ):
			raise Exception( "Shader state mismatch!!\n Expected = " + str( expectedShaders ) + ",\n Actual = " + str( shaderState ) )

		expectedChildren = structure["children"]

		if len( expectedChildren ) != len( g.children() ):
			raise Exception( "Child count mismatch!! Expected = " + str( len( expectedChildren ) ) + " Actual = " + str( len( g.children() ) ) )

		for childgroup, expectedStructure in zip( g.children(), expectedChildren ):

			if isinstance( childgroup, IECore.Group ):
				self.checkStructure( childgroup, expectedStructure )
			else:
				if childgroup.typeName() != expectedStructure:
					raise Exception( "Child type mismatch!!" )
	
	
	def testAttributesAndShaders( self ):
		
		r = IECore.CapturingRenderer()
		
		with IECore.WorldBlock( r ) :
		
			r.setAttribute( "user:test", IECore.BoolData( True ) )
			r.shader( "asdf", "surface", { "sss": IECore.IntData( 1 ) } )
			
			with IECore.AttributeBlock( r ):
				r.shader( "asdf2", "surface", { "aaa": IECore.IntData( 1 ) } )
				
				with IECore.AttributeBlock( r ):
				
					r.setAttribute( "name", IECore.StringData( "sphere1" ) )
					r.setAttribute( "yahyah", IECore.StringData( "blahblah" ) )
					r.shader( "sphere1Shader", "surface", { "aaa": IECore.IntData( 1 ) } )
					r.sphere( 1, -1, 1, 360, {} )
					
					r.setAttribute( "name", IECore.StringData( "sphere2" ) )
					r.shader( "sphere2Shader", "surface", { "bbb": IECore.IntData( 1 ) } )
					r.sphere( 1, -1, 1, 360, {} )
					
				with IECore.AttributeBlock( r ):
				
					r.setAttribute( "name", IECore.StringData( "sphere1" ) )
					r.setAttribute( "yahyah", IECore.StringData( "blahblah" ) )
					r.shader( "sphere1Shader", "surface", { "aaa": IECore.IntData( 1 ) } )
					r.sphere( 1, -1, 1, 360, {} )
		
		expectedStructure = {
			"attributes" : { "user:test": IECore.BoolData( True ) },
			"shaders" : [ ( "asdf", "surface", IECore.CompoundData( { "sss": IECore.IntData( 1 ) } ) ) ],
			"children" : [
				{
					"attributes" : {},
					"shaders" : [ ( "asdf2", "surface", IECore.CompoundData( { "aaa": IECore.IntData( 1 ) } ) ) ],
					"children" : [
						{
							"attributes" : {},
							"shaders" : {},
							"children" : [
								{
									"attributes" : { 'name':IECore.StringData( "sphere1" ), 'yahyah':IECore.StringData( "blahblah" ) },
									"shaders" : [ ( "sphere1Shader", "surface", IECore.CompoundData( { "aaa": IECore.IntData( 1 ) } ) ) ],
									"children" : [ "SpherePrimitive" ],
								},
								{
									"attributes" : { 'name':IECore.StringData( "sphere2" ), 'yahyah':IECore.StringData( "blahblah" ) },
									"shaders" : [
										( "sphere1Shader", "surface", IECore.CompoundData( { "aaa": IECore.IntData( 1 ) } ) ),
										( "sphere2Shader", "surface", IECore.CompoundData( { "bbb": IECore.IntData( 1 ) } ) )
									],
									"children" : [ "SpherePrimitive" ],
								},
							],

						},

						{
							"attributes" : {'name':IECore.StringData( "sphere1" ),'yahyah':IECore.StringData( "blahblah" ) },
							"shaders" : [ ( "sphere1Shader", "surface", IECore.CompoundData( { "aaa": IECore.IntData( 1 ) } ) ) ],
							"children" : [ "SpherePrimitive" ],
						},
					]
				}
			]
		}
		
		self.checkStructure( r.world(), expectedStructure )
	

	class ChainProcedural( IECore.Renderer.Procedural ) :

		def __init__( self, maxLevel = 3, level = 0, withAttributeBlock = True ) :

			IECore.Renderer.Procedural.__init__( self )

			self.__maxLevel = maxLevel
			self.__level = level
			self.__withAttributeBlock = withAttributeBlock

		def bound( self ) :

			return IECore.Box3f( IECore.V3f( -1 ), IECore.V3f( 1 ) )

		def render( self, renderer ) :
			
			if self.__withAttributeBlock:
				renderer.attributeBegin()

			renderer.shader( "shader%d" % self.__level, "surface", { "level": IECore.IntData( self.__level ) } )
			renderer.setAttribute( "name", IECore.StringData(str( self.__level) ) )
			renderer.setAttribute( "attr%d" % self.__level, IECore.StringData(str( self.__level) ) )

			if not renderer.getAttribute( "user:test" ) :
				raise Exception( "couldn't read user:test attribute on level", self.__level, "ChainProcedural" )

			if self.__level < self.__maxLevel :
				renderer.procedural( CapturingRendererTest.ChainProcedural( self.__maxLevel, self.__level + 1, self.__withAttributeBlock ) )
			else :
				renderer.sphere( 1, -1, 1, 360, {} )
			
			if self.__withAttributeBlock:
				renderer.attributeEnd()
		
		def hash( self ):
			
			h = IECore.MurmurHash()
			return h
	
	def testProceduralAttributesAndShaders( self ):
		
		# check that the reentrant structure is the same as the non reentrant one:
		for reentrancy in [ False, True ]:

			r = IECore.CapturingRenderer()
			
			with IECore.WorldBlock( r ) :

				r.setAttribute( "cp:procedural:reentrant", IECore.BoolData( reentrancy ) )
				
				# procedural throws an exception if the renderer can't do a getAttribute on this attribute,
				# so this is a test that it's picking it up correctly in multi level nested procedurals
				r.setAttribute( "user:test", IECore.BoolData( True ) )
				r.shader( "topshader", "surface", { "sss": IECore.IntData( 1 ) } )

				r.procedural( self.ChainProcedural( maxLevel = 1, withAttributeBlock = False ) )
			
			expectedStructure = {			
				"attributes" : {"cp:procedural:reentrant": IECore.BoolData( reentrancy ), "user:test": IECore.BoolData( True ) },
				"shaders" : [ ( "topshader", "surface", IECore.CompoundData( { "sss": IECore.IntData( 1 ) } ) ) ],
				"children" : [
					{
						"attributes" : {"name": IECore.StringData( "0" ), "attr0": IECore.StringData( "0" ) },
						"shaders" : [ ( "shader0", "surface", IECore.CompoundData( { "level": IECore.IntData( 0 ) } ) ) ],
						"children" : [
							{
								"attributes" : {"name": IECore.StringData( "1" ), "attr1": IECore.StringData( "1" ) },
								"shaders" : [ ( "shader1", "surface", IECore.CompoundData( { "level": IECore.IntData( 1 ) } ) ) ],
								"children" : [
									"SpherePrimitive"
								]
							},
						]
					},
				]

			}
			
			self.checkStructure( r.world(), expectedStructure )
	
	
	def testProceduralStateLeaking( self ):
		
		# this used to result in attributes leaking out of the ChainProcedural and affecting the sphere
		# following it - that shouldn't happen...
		
		for reentrancy in [ False, True ]:
			
			r = IECore.CapturingRenderer()
			with IECore.WorldBlock( r ) :

				r.setAttribute( "cp:procedural:reentrant", IECore.BoolData( reentrancy ) )

				# procedural throws an exception if it can't do a getAttribute  
				r.setAttribute( "user:test", IECore.BoolData( True ) )
				r.shader( "topshader", "surface", { "sss": IECore.IntData( 1 ) } )

				r.procedural( self.ChainProcedural( maxLevel = 1, withAttributeBlock = False ) )

				r.setAttribute( "asdf", IECore.BoolData( True ) )

				r.sphere( 1, -1, 1, 360, {} )

			expectedStructure = {
				"attributes" : {},
				"shaders" : [],
				"children" : [
					{
						"attributes" : {"cp:procedural:reentrant": IECore.BoolData( reentrancy ), "user:test": IECore.BoolData( True ) },
						"shaders" : [ ( "topshader", "surface", IECore.CompoundData( { "sss": IECore.IntData( 1 ) } ) ) ],
						"children" : [
							{
								"attributes" : {"name": IECore.StringData( "0" ), "attr0": IECore.StringData( "0" ) },
								"shaders" : [ ( "shader0", "surface", IECore.CompoundData( { "level": IECore.IntData( 0 ) } ) ) ],
								"children" : [
									{
										"attributes" : {"name": IECore.StringData( "1" ), "attr1": IECore.StringData( "1" ) },
										"shaders" : [ ( "shader1", "surface", IECore.CompoundData( { "level": IECore.IntData( 1 ) } ) ) ],
										"children" : [
											"SpherePrimitive"
										]
									},
								]
							},
						]
					},
					{
						"attributes" : {"cp:procedural:reentrant": IECore.BoolData( reentrancy ), "user:test": IECore.BoolData( True ), "asdf": IECore.BoolData( True ) },
						"shaders" : [ ( "topshader", "surface", IECore.CompoundData( { "sss": IECore.IntData( 1 ) } ) ) ],
						"children" : [
							"SpherePrimitive"
						]
					},
				]
			}

			self.checkStructure( r.world(), expectedStructure )	
	
	def testLights( self ) :
	
		r = IECore.CapturingRenderer()
		with IECore.WorldBlock( r ) :
		
			r.light( "myLight", "myLightHandle", { "intensity" : IECore.FloatData( 10 ) } )
			
		w = r.world()
		
		self.assertEqual( len( w.state() ), 1 )
		self.assertTrue( isinstance( w.state()[0], IECore.Light ) )
		self.assertEqual( w.state()[0].name, "myLight" )
		self.assertEqual( w.state()[0].handle, "myLightHandle" )
		self.assertEqual( w.state()[0].parameters, IECore.CompoundData( { "intensity" : IECore.FloatData( 10 ) } ) )

if __name__ == "__main__":
	unittest.main()

