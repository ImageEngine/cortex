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

import unittest
import os
import random
import thread
import gc

import IECore
import IECoreRI

class TeapotProcedural( IECore.Renderer.Procedural ) :

	def __init__( self ) :
	
		IECore.Renderer.Procedural.__init__( self )
		
	def bound( self ) :
						
		return IECore.Box3f( IECore.V3f( -5 ), IECore.V3f( 5 ) )
		
	def render( self, renderer ) :
	
		renderer.geometry( "teapot", {}, {} )
		
	def hash( self ):
		h = IECore.MurmurHash()
		return h
	
class ParameterisedTeapotProcedural( IECore.ParameterisedProcedural ) :

	def __init__( self ) :
	
		IECore.ParameterisedProcedural.__init__( self, "" )
		
	def doBound( self, args ) :
						
		return IECore.Box3f( IECore.V3f( -5 ), IECore.V3f( 5 ) )
		
	def doRender( self, renderer, args ) :
	
		renderer.geometry( "teapot", {}, {} )		
				
class ProceduralThreadingTest( IECoreRI.TestCase ) :

	outputFileName = os.path.dirname( __file__ ) + "/output/testProceduralThreading.tif"
	
	## This function serves as an example of performing a direct render to 3delight from
	# python, either using threaded python procedurals or not. You must do one
	# of two things :
	#
	# Either :
	#
	# call renderer.setAttribute( "ri:procedural:reentrant", 0 ) to ensure that the calls
	# to the python procedural will not be threaded.
	#
	# Or :
	#
	# ensure that threading has been enabled in python by calling IECore.initThreads prior to
	# rendering.
	#
	def performRender( self, withMultiThreading, withParameterisedProcedural ) :
	
		if withMultiThreading :
			# this is necessary so python will allow threads created by the renderer
			# to enter into python when those threads execute procedurals.
			IECore.initThreads()

		r = IECoreRI.Renderer( "" )
		
		r.camera( "main", {
				"projection" : IECore.StringData( "orthographic" ),
				"resolution" : IECore.V2iData( IECore.V2i( 256 ) ),
				"clippingPlanes" : IECore.V2fData( IECore.V2f( 1, 1000 ) ),
				"screenWindow" : IECore.Box2fData( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) ),
				"shutter" : IECore.V2fData( IECore.V2f( 0, 1 ) ),
			}
		)
		r.display( self.outputFileName, "tiff", "rgba", {} )
		r.setOption( "ri:pixelSamples", IECore.V2i( 16 ) )
		
		with IECore.WorldBlock( r ) :
			
			r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( 0, 0, -10 ) ) )
			r.concatTransform( IECore.M44f.createScaled( IECore.V3f( 0.1 ) ) )
				
			if not withMultiThreading :
				r.setAttribute( "ri:procedural:reentrant", 0 )
			
			random.seed( 0 )
			for i in range( 0, 20 ) :
			
				with IECore.TransformBlock( r ) :
				
					r.concatTransform( IECore.M44f.createTranslated( IECore.V3f( random.uniform( -10, 10 ), random.uniform( -10, 10 ), 0 ) ) )
					if withParameterisedProcedural :
						ParameterisedTeapotProcedural().render( r )
					else :
						r.procedural( TeapotProcedural() )

		imageCreated = IECore.Reader.create( "test/IECoreRI/output/testProceduralThreading.tif" ).read()
		expectedImage = IECore.Reader.create( "test/IECoreRI/data/testProceduralThreading.tif" ).read()

		self.assertEqual( IECore.ImageDiffOp()( imageA=imageCreated, imageB=expectedImage, maxError=0.01 ), IECore.BoolData( False ) )
	
	def test( self ) :
		
		# we do the same thing over and over to give it a decent chance of failing
		# as threading errors are somewhat random.
		
		while gc.collect() :
			pass
		IECore.RefCounted.collectGarbage()
		numInstances = IECore.RefCounted.numWrappedInstances()
		
		for i in range( 0, 20 ) :
			self.performRender( False, False )
			
		for i in range( 0, 20 ) :
			self.performRender( False, True )	

		for i in range( 0, 20 ) :
			self.performRender( True, False )
			
		for i in range( 0, 20 ) :
			self.performRender( True, True )		
		
		# clean up and check that we haven't leaked any procedurals
		while gc.collect() :
			pass
		IECore.RefCounted.collectGarbage()
		self.assertEqual( numInstances, IECore.RefCounted.numWrappedInstances() )

if __name__ == "__main__":
    unittest.main()
