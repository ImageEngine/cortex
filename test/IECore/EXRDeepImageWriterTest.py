##########################################################################
#
#  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

from IECore import *
import sys
import unittest
import os

class EXRDeepImageWriterTest(unittest.TestCase):
	
	__output = "test/IECoreRI/data/exr/written.exr"

	def testCreator( self ) :
		
		writer = DeepImageWriter.create( EXRDeepImageWriterTest.__output )
		self.assertEqual( writer.typeId(), TypeId.EXRDeepImageWriter )
	
	def testCanWrite( self ) :
		
		self.failUnless( not EXRDeepImageWriter.canWrite( "" ) )
		self.failUnless( EXRDeepImageWriter.canWrite( EXRDeepImageWriterTest.__output ) )
	
	def testDefaultWriter( self ) :
		return
		pass
		writer = EXRDeepImageWriter()
		self.assertEqual( writer.parameters()['fileName'].getTypedValue(), "" )
		self.assertEqual( writer.parameters()['channelNames'].getValue(), StringVectorData( [ "R", "G", "B", "A" ] ) )
		self.assertEqual( writer.parameters()['halfPrecisionChannels'].getValue(), StringVectorData( [ "R", "G", "B", "A" ] ) )
		self.assertEqual( writer.parameters()['resolution'].getTypedValue(), V2i( 2048, 1556 ) )
		self.assertEqual( writer.parameters()['compression'].getTypedValue(), EXRDeepImageWriter.Compression.ZIPS )
		self.assertEqual( writer.parameters()['worldToCameraMatrix'].getTypedValue(), M44f() )
		self.assertEqual( writer.parameters()['worldToNDCMatrix'].getTypedValue(), M44f() )
	
	def testParameters( self ) :
	
		writer = EXRDeepImageWriter( EXRDeepImageWriterTest.__output )
		self.assertEqual( writer.parameters()['fileName'].getTypedValue(), EXRDeepImageWriterTest.__output )
		self.assertEqual( writer.parameters()['channelNames'].getValue(), StringVectorData( [ "R", "G", "B", "A" ] ) )
		self.assertEqual( writer.parameters()['halfPrecisionChannels'].getValue(), StringVectorData( [ "R", "G", "B", "A" ] ) )
		self.assertEqual( writer.parameters()['resolution'].getTypedValue(), V2i( 2048, 1556 ) )
		self.assertEqual( writer.parameters()['compression'].getTypedValue(), EXRDeepImageWriter.Compression.ZIPS )
		self.assertEqual( writer.parameters()['worldToCameraMatrix'].getTypedValue(), M44f() )
		self.assertEqual( writer.parameters()['worldToNDCMatrix'].getTypedValue(), M44f() )
	
	def testEmptyPixel( self ) :
		
		writer = EXRDeepImageWriter( EXRDeepImageWriterTest.__output )
		writer.parameters()["channelNames"].setValue( StringVectorData( [ "A" ] ) )
		p = DeepPixel( "A" )
		writer.writePixel( 0, 0, p )
		writer.writePixel( 0, 50, None )
		writer.writePixel( 0, 100, p )
		p.addSample( 1, [ 0.5 ] )
		writer.writePixel( 0, 1, p )
		del writer
		
		reader = EXRDeepImageReader( EXRDeepImageWriterTest.__output )
		self.failUnless( reader.readPixel( 0, 0 ) is None )
		self.failUnless( reader.readPixel( 0, 50 ) is None )
		self.failUnless( reader.readPixel( 0, 100 ) is None )
		rp = reader.readPixel( 0, 1 )
		self.failUnless( isinstance( rp, DeepPixel ) )
		self.assertEqual( rp[0], ( 0.5, ) )
	
	def testWriteSimplePixel( self ) :
		
		writer = EXRDeepImageWriter( EXRDeepImageWriterTest.__output )
		writer.parameters()['channelNames'].setValue( StringVectorData( [ "R", "G", "A" ] ) )
		writer.parameters()['resolution'].setTypedValue( V2i( 2, 2 ) )
		writer.parameters()['halfPrecisionChannels'].setValue( StringVectorData( [ "R", "A" ] ) )
		writer.parameters()['resolution'].setTypedValue( V2i( 2, 2 ) )
		
		p = DeepPixel( [ "R", "G", "A" ] )
		p.addSample( 1, [ 0.25, 0.25, .5 ] )
		
		p2 = DeepPixel( [ "R", "G", "A" ] )
		p2.addSample( 2, [ 0.125, 0.0625, 0.25 ] )
		
		p3 = DeepPixel( [ "R", "G", "A" ] )
		p3.addSample( 1, [ 0.25, 0.125, .125 ] )
		p3.addSample( 2, [ 0.0625, 0.25, 0.0625 ] )
		writer.writePixel( 0, 0, p )
		writer.writePixel( 0, 1, p2 )
		writer.writePixel( 1, 1, p3 )
		del writer
		
		reader = EXRDeepImageReader( EXRDeepImageWriterTest.__output )
		self.assertEqual( reader.dataWindow().size() + V2i( 1 ), V2i( 2, 2 ) )
		self.assertEqual( set( reader.channelNames() ), set( [ "R", "G", "A" ] ) )
		
		rp = reader.readPixel( 0, 0 )
		self.assertEqual( rp.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp.numSamples(), 1 )
		self.assertEqual( rp.getDepth( 0 ), 1 )
		self.assertEqual( dict( zip( rp.channelNames(), rp[0] ) ), { "R" : 0.25, "G" : 0.25, "A" : .5 } )
		
		rp2 = reader.readPixel( 0, 1 )
		self.assertEqual( rp2.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp2.numSamples(), 1 )
		self.assertEqual( rp2.getDepth( 0 ), 2 )
		self.assertEqual( dict( zip( rp2.channelNames(), rp2[0] ) ), { "R" : 0.125, "G" : 0.0625, "A" : 0.25 } )
		
		rp3 = reader.readPixel( 1, 1 )
		self.assertEqual( rp3.channelNames(), tuple(reader.channelNames()) )
		self.assertEqual( rp3.numSamples(), 2 )
		self.assertEqual( rp3.getDepth( 0 ), 1 )
		self.assertEqual( rp3.getDepth( 1 ), 2 )
		self.assertEqual( dict( zip( rp3.channelNames(), rp3[0] ) ), { "R" : 0.25, "G" : 0.125, "A" : .125 } )
		self.assertEqual( dict( zip( rp3.channelNames(), rp3[1] ) ), { "R" : 0.0625,  "G" : 0.25, "A" : 0.0625 } )
		self.failUnless( reader.readPixel( 1, 0 ) is None )
	
	def tearDown( self ) :
		
		if os.path.isfile( EXRDeepImageWriterTest.__output ) :
			os.remove( EXRDeepImageWriterTest.__output )

if __name__ == "__main__":
	unittest.main()

