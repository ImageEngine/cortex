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

import unittest
import time
import threading
import random
import os

import IECore

class ThreadingTest( unittest.TestCase ) :

	def callSomeThings( self, things, args=(), kwArgs=(), threaded=False, iterations=1 ) :
	
		for i in range( 0, iterations ) :
				
			threads = []
			for j in range( 0, len( things ) ) :
				
				a = args[j] if args else ()
								
				kwa = kwArgs[j] if kwArgs else {}
				if threaded :
				
					t = threading.Thread( target=things[j], args=a, kwargs=kwa )
					t.start()
					threads.append( t )
					
				else :
				
					things[j]( *a, **kwa )
					
			for t in threads :

				t.join()

	def testThreadedOpGains( self ) :
	
		## Checks that we actually get a speedup by running a bunch of slow
		# C++ ops in parallel.
	
		ops = []
		kwArgs = []
		for i in range( 0, 4 ) :
		
			ops.append( IECore.ParticleMeshOp() )
			kwArgs.append( {
				"filename" : "test/IECore/data/pdcFiles/particleMesh.pdc",
				"useRadiusAttribute" : False,
				"radius" : 0.25,
				"threshold" : 1,
				"bound" : IECore.Box3f( IECore.V3f( -5, -7, -2 ), IECore.V3f( 3, 3, 3 ) ),
				"resolution" : IECore.V3i( 80, 80, 80 ),
			} )

		tStart = time.time()
		self.callSomeThings( ops, kwArgs=kwArgs, threaded=False )
		nonThreadedTime = time.time() - tStart
		
		tStart = time.time()
		self.callSomeThings( ops, kwArgs=kwArgs, threaded=True )
		threadedTime = time.time() - tStart
		
		self.failUnless( threadedTime < nonThreadedTime ) # may fail on single core machines or machines under varying load

	def testThreadedReaders( self ) :
	
		## Checks that we can read a bunch of files in parallel, even when one
		# of the Readers is implemented in python. We're using the CachedReader
		# here as it forces a call to Reader::create when the GIL isn't held yet.
	
		args = [
			( "test/IECore/data/exrFiles/ramp.exr", ),
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/exrFiles/checkerAnimated.0006.exr", ),
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/tiff/toTrace.tif", ),
			( "test/IECore/data/tiff/toTraceThinned.tif", ),
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/exrFiles/checkerAnimated.0006.exr", ),
			( "test/IECore/data/exrFiles/checkerAnimated.0006.exr", ),
			( "test/IECore/data/tiff/toTraceThinned.tif", ),
		]
	
		sp = IECore.SearchPath( "./", ":" )
		calls = [ lambda f : IECore.CachedReader( sp, IECore.ObjectPool(1024 * 1024 * 10) ).read( f ) ] * len( args )
		
		self.callSomeThings( calls, args, threaded=True )

	def testMixedCPPAndPython( self ) :
	
		## Checks that we can mix a bunch of C++ and python ops concurrently
		# without crashing

		ops = []
		kwArgs = []
		for i in range( 0, 4 ) :
		
			ops.append( IECore.ParticleMeshOp() )
			kwArgs.append( {
				"filename" : "test/IECore/data/pdcFiles/particleMesh.pdc",
				"useRadiusAttribute" : False,
				"radius" : 0.25,
				"threshold" : 1,
				"bound" : IECore.Box3f( IECore.V3f( -5, -7, -2 ), IECore.V3f( 3, 3, 3 ) ),
				"resolution" : IECore.V3i( 80, 80, 80 ),
			} )
			
			ops.append( IECore.ClassLsOp() )
			kwArgs.append( { "type" : "op" } )

		self.callSomeThings( ops, kwArgs=kwArgs, threaded=True, iterations=5 )

	def testReadingGains( self ) :
	
		## Checks that we can use a bunch of readers in different threads and
		# that we get a speedup of some sort doing that.
	
		args = [
			( "test/IECore/data/exrFiles/ramp.exr", ),
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/pdcFiles/particleMesh.pdc", ),
			( "test/IECore/data/cobFiles/ball.cob", ),
			( "test/IECore/data/jpg/21mm.jpg", ),
			( "test/IECore/data/jpg/exif.jpg", ),
			( "test/IECore/data/dpx/ramp.dpx", ),
		]

		calls = [ lambda f : IECore.Reader.create( f ).read() ] * len( args )
	
		tStart = time.time()
		self.callSomeThings( calls, args, threaded=False )
		nonThreadedTime = time.time() - tStart

		tStart = time.time()
		self.callSomeThings( calls, args, threaded=True )
		threadedTime = time.time() - tStart
				
		self.failUnless( threadedTime < nonThreadedTime ) # this could plausibly fail due to varying load on the machine / io but generally shouldn't

	def testWritingGains( self ) :
	
		image = IECore.Reader.create( "test/IECore/data/jpg/21mm.jpg" ).read()
		
		def write( o, f ) :
		
			 IECore.Writer.create( o, f ).write()
		
		calls = []
		for i in range( 0, 4 ) :
			fileName = "test/IECore/test%d.jpg" % i
			calls.append( IECore.curry( write, image, fileName ) )

		tStart = time.time()
		self.callSomeThings( calls, threaded=False )
		nonThreadedTime = time.time() - tStart

		tStart = time.time()
		self.callSomeThings( calls, threaded=True )
		threadedTime = time.time() - tStart
		
		self.failUnless( threadedTime < nonThreadedTime ) # this could plausibly fail due to varying load on the machine / io but generally shouldn't

	def testCachedReaderConcurrency( self ) :
	
		args = [
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/idxFiles/test.idx", ),
			( "test/IECore/data/cobFiles/intDataTen.cob", ),
			( "test/IECore/data/cobFiles/intDataTen.cob", ),
			( "test/IECore/data/cobFiles/pSphereShape1.cob", ),
			( "test/IECore/data/cobFiles/pSphereShape1.cob", ),
			( "test/IECore/data/cobFiles/pSphereShape1.cob", ),
			( "test/IECore/data/cobFiles/pSphereShape1.cob", ),
		] 
	
		cachedReader = IECore.CachedReader( IECore.SearchPath( "./", ":" ) )
		
		calls = [ lambda f : cachedReader.read( f ) ] * len( args )

		for i in range( 0, 5 ) :
			cachedReader.clear()
			self.callSomeThings( calls, args=args, threaded=True )

	def testCachedReaderGains( self ) :
	
		args = [
			( "test/IECore/data/jpg/21mm.jpg", ),
			( "test/IECore/data/jpg/exif.jpg", ),
			( "test/IECore/data/jpg/greyscaleCheckerBoard.jpg", ),
			( "test/IECore/data/dpx/ramp.dpx", ),
		] * 4
	
		cachedReader = IECore.CachedReader( IECore.SearchPath( "./", ":" ) )
		
		calls = [ lambda f : cachedReader.read( f ) ] * len( args )

		tStart = time.time()
		cachedReader.clear()
		self.callSomeThings( calls, args=args, threaded=False )
		nonThreadedTime = time.time() - tStart

		tStart = time.time()
		cachedReader.clear()
		self.callSomeThings( calls, args=args, threaded=True )
		threadedTime = time.time() - tStart
				
		self.failUnless( threadedTime < nonThreadedTime ) # this could plausibly fail due to varying load on the machine / io but generally shouldn't

	def testPythonColorConverterWithThread( self ) :

		def NewSRGBToLinear( inputColorSpace, outputColorSpace ) :
			converter = IECore.SRGBToLinearOp()
			return converter
	
		IECore.ColorSpaceTransformOp.registerConversion( 
			"newSRGB", "linear", NewSRGBToLinear
		)

		runThread = True

		def test():
			while runThread :
				pass

		newThread = threading.Thread(target=test)
		newThread.start()

		reader = IECore.Reader.create( "test/IECore/data/cinFiles/uvMap.512x256.cin" )
		reader['colorSpace'] = 'newSRGB'
		reader.read()

		runThread = False
		newThread.join()

	def testInterpolatedCacheGains( self ) :
	
		numObjects = 100
		numAttrs = 2
	
		def createCache( fileName ) :
		
			data = IECore.V3fVectorData( [ IECore.V3f( 1 ) ] * 50000 )
			cache = IECore.AttributeCache( fileName, IECore.IndexedIO.OpenMode.Write )
			for i in range( 0, numObjects ) :
				for j in range( 0, numAttrs ) :
					cache.write( "object%d" % i, "attr%d" % j, data )

		
		createCache( "test/IECore/interpolatedCache.0250.fio" )
		createCache( "test/IECore/interpolatedCache.0500.fio" )
		
		cache = IECore.InterpolatedCache(
			"test/IECore/interpolatedCache.####.fio",
			IECore.InterpolatedCache.Interpolation.Linear,
		)
		
		calls = []
		for i in range( 0, 200 ) :
			calls.append(
				IECore.curry(
					cache.read,
					1.5,
					"object%d" % random.uniform( 0, numObjects ),
					"attr%d" % random.uniform( 0, numAttrs )
				)
			)
					
		tStart = time.time()
		self.callSomeThings( calls, threaded=False )
		nonThreadedTime = time.time() - tStart

		tStart = time.time()
		self.callSomeThings( calls, threaded=True )
		threadedTime = time.time() - tStart
						
		self.failUnless( threadedTime < nonThreadedTime ) # this could plausibly fail due to varying load on the machine / io but generally shouldn't

	def tearDown( self ) :
		
		for f in [
			"test/IECore/test0.jpg",
			"test/IECore/test1.jpg",
			"test/IECore/test2.jpg",
			"test/IECore/test3.jpg",
			"test/IECore/interpolatedCache.0250.fio",
			"test/IECore/interpolatedCache.0500.fio",
		] :
			if os.path.exists( f ) :
				os.remove( f )
				
if __name__ == "__main__":
	unittest.main()

