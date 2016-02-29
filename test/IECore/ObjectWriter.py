##########################################################################
#
#  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
import os, sys
import IECore
import socket

class TestObjectWriter( unittest.TestCase ) :

	def testBasics( self ) :

		# read an old file:
		r = IECore.Reader.create( "test/IECore/data/cobFiles/compoundData.cob" )
		p = r.read()
		self.assertEqual(
			p,
			IECore.CompoundData({'banana':IECore.IntData( 2 ),'melon':IECore.FloatData( 2.5 ),'lemon':IECore.IntData( -3 ),'apple':IECore.IntData( 12 )})
		)

		w = IECore.Writer.create( p, "test/compoundData.cob" )
		w["compressionType"].setTypedValue("none")
		w.write()
		
		# check FileIndexedIO contents:
		fio = IECore.FileIndexedIO( "test/compoundData.cob", IECore.IndexedIO.OpenMode.Read )
		fio.subdirectory("header")
		fio.subdirectory("object")

		r = IECore.Reader.create( "test/compoundData.cob" )
		p2 = r.read()

		self.assertEqual( p, p2 )

	def testHeader( self ) :

		o = IECore.IntData()

		w = IECore.Writer.create( o, "test/intData.cob" )
		w["compressionType"].setTypedValue("none")
		w["header"].getValue()["testHeaderData"] = IECore.StringData( "i am part of a header" )
		w["header"].getValue()["testHeaderData2"] = IECore.IntData( 100 )
		w.write()

		h = IECore.Reader.create( "test/intData.cob" ).readHeader()

		for k in w["header"].getValue().keys() :
			self.assertEqual( w["header"].getValue()[k], h[k] )

		self.assertEqual( h["host"]["nodeName"].value, socket.gethostname() )
		self.assertEqual( h["ieCoreVersion"].value, IECore.versionString() )
		self.assertEqual( h["typeName"].value, "IntData" )

	def testBoundInHeader( self ) :

		o = IECore.SpherePrimitive()
		w = IECore.Writer.create( o, "test/spherePrimitive.cob" )
		w["compressionType"].setTypedValue("none")
		w.write()

		h = IECore.Reader.create( "test/spherePrimitive.cob" ).readHeader()

		self.assertEqual( h["bound"].value, o.bound() )

	def testCompressed( self ) :
		
		if IECore.withBlosc():

			floats = IECore.FloatVectorData( range(0,10000) )
			w = IECore.Writer.create( floats, "test/compressed.cob" )
			w["compressionType"].setTypedValue("blosc")

			w.write()

			# check FileIndexedIO contents:
			fio = IECore.FileIndexedIO( "test/compressed.cob", IECore.IndexedIO.OpenMode.Read )
			self.assertEqual( set( fio.entryIds() ), set( [ IECore.InternedString( s ) for s in ["header", "objectCompressed"] ] ) )
			self.assertEqual( set( fio.subdirectory("objectCompressed").entryIds() ), set( [ IECore.InternedString( s ) for s in ["0", "numBlocks"] ] ) )
			self.assertEqual( set( fio.subdirectory("objectCompressed").subdirectory("0").entryIds() ), set( [ IECore.InternedString( s ) for s in ["data", "compressedSize", "decompressedSize"] ] ) )
			self.assertEqual( fio.subdirectory("objectCompressed").read("numBlocks").value, 1 )

			# read back and check object:
			self.assertEqual( IECore.Reader.create( "test/compressed.cob" ).read(), floats )

	def testGiantCompressed( self ) :
		
		if IECore.withBlosc():

			# create a 1.5gb mesh so multiple blocks get written into the file:
			stupidMesh = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f(-1), IECore.V2f(1) ), IECore.V2i( 5000,5000 ) )

			w = IECore.Writer.create( stupidMesh, "test/compressed.cob" )
			w["compressionType"].setTypedValue("blosc")

			w.write()

			# check FileIndexedIO contents:
			fio = IECore.FileIndexedIO( "test/compressed.cob", IECore.IndexedIO.OpenMode.Read )
			self.assertEqual( set( fio.entryIds() ), set( [ IECore.InternedString( s ) for s in ["header", "objectCompressed"] ] ) )
			self.assertEqual( set( fio.subdirectory("objectCompressed").entryIds() ), set( [ IECore.InternedString( s ) for s in ["0", "1", "numBlocks"] ] ) )
			self.assertEqual( set( fio.subdirectory("objectCompressed").subdirectory("0").entryIds() ), set( [ IECore.InternedString( s ) for s in ["data", "compressedSize", "decompressedSize"] ] ) )
			self.assertEqual( set( fio.subdirectory("objectCompressed").subdirectory("1").entryIds() ), set( [ IECore.InternedString( s ) for s in ["data", "compressedSize", "decompressedSize"] ] ) )
			self.assertEqual( fio.subdirectory("objectCompressed").read("numBlocks").value, 2 )

			# read back and check object:
			self.assertEqual( IECore.Reader.create( "test/compressed.cob" ).read(), stupidMesh )

	def testCompressedHeader( self ) :
				
		if IECore.withBlosc():

			o = IECore.IntData()

			w = IECore.Writer.create( o, "test/compressed.cob" )
			w["compressionType"].setTypedValue("blosc")
			w["header"].getValue()["testHeaderData"] = IECore.StringData( "i am part of a header" )
			w["header"].getValue()["testHeaderData2"] = IECore.IntData( 100 )
			w.write()

			h = IECore.Reader.create( "test/compressed.cob" ).readHeader()

			for k in w["header"].getValue().keys() :
				self.assertEqual( w["header"].getValue()[k], h[k] )

			self.assertEqual( h["host"]["nodeName"].value, socket.gethostname() )
			self.assertEqual( h["ieCoreVersion"].value, IECore.versionString() )
			self.assertEqual( h["typeName"].value, "IntData" )

	def tearDown( self ) :
		
		for f in ( "test/compoundData.cob", "test/intData.cob", "test/compressed.cob" ) :
			if os.path.isfile( f ) :
				os.remove( f )

if __name__ == "__main__":
	unittest.main()
