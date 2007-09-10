##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
import random
from IECore import *

class TestObjectIO( unittest.TestCase ) :

	def testSimpleIO( self ) :
	
		o = IntData( 1 )
		self.assertEqual( o.value, 1 )
		o.save( "test/o.sql" )		
		oo = Object.load( "test/o.sql" )
		self.assertEqual( o.value, oo.value );
		
		o = StringData( "hello" )
		self.assertEqual( o.value, "hello" )
		o.save( "test/o.sql" )		
		oo = Object.load( "test/o.sql" )
		self.assertEqual( o.value, oo.value );
		self.assertEqual( o, oo );
		
	def testSimpleArrayIO( self ) :
	
		o = IntVectorData()
		for i in range( 0, 1000 ) :
			o.append( random.randint( -1000, 1000 ) )
		self.assertEqual( o.size(), 1000 )
			
		o.save( "test/o.sql" )
		oo = Object.load( "test/o.sql" )
		self.assertEqual( oo.size(), 1000 )
		
		for i in range( 0, 1000 ) :
			self.assertEqual( o[i], oo[i] )

		self.assertEqual( o, oo );
			
	def testStringArrayIO( self ) :
	
		words = [ "hello", "there", "young", "fellah" ]
		s = StringVectorData( words )
		self.assertEqual( s.size(), len( words ) )
		for i in range( 0, s.size() ) :
			self.assertEqual( s[i], words[i] )
		
		s.save( "test/o.sql" )
		ss = Object.load( "test/o.sql" )
		self.assertEqual( ss.size(), s.size() )
		
		for i in range( 0, s.size() ) :
			self.assertEqual( s[i], ss[i] )	

		self.assertEqual( s, ss );
			
	def testImathArrayIO( self ) :
	
		o = V3fVectorData()
		for i in range( 0, 1000 ) :
			o.append( V3f( i*3, i*3 + 1, i*3 + 2 ) )
		self.assertEqual( o.size(), 1000 )
		
		o.save( "test/o.sql" )
		oo = Object.load( "test/o.sql" )
		self.assertEqual( oo.size(), 1000 )
		
		for i in range( 0, 1000 ) :
			self.assertEqual( o[i], oo[i] )

		self.assertEqual( o, oo );
			
	def testOverwrite( self ) :
	
		o = IntData( 1 )
		self.assertEqual( o.value, 1 )
		o.save( "test/o.sql" )		
		oo = Object.load( "test/o.sql" )
		self.assertEqual( o.value, oo.value );

		# saving over an existing file should
		# obliterate it but currently doesn't
		o = StringData( "hello" )
		self.assertEqual( o.value, "hello" )
		o.save( "test/o.sql" )		
		oo = Object.load( "test/o.sql" )
		self.assertEqual( o.value, oo.value );

		self.assertEqual( o, oo );
		
	def testCompoundData( self ) :
	
		d = CompoundData()
		d["A"] = IntData( 10 )
		d["B"] = StringData( "hithere" )
		self.assertEqual( d["B"].value, "hithere" )
		
		d.save( "test/o.sql" )
		dd = Object.load( "test/o.sql" )
		self.assertEqual( d, dd )
		
	def testMultipleRef( self ) :
	
		d = CompoundData()
		i = IntData( 100 )
		d["ONE"] = i
		d["TWO"] = i
		
		self.assert_( d["ONE"].isSame( d["TWO"] ) )
		
		d.save( "test/o.sql" )
		
		dd = Object.load( "test/o.sql" )
		self.assertEqual( d, dd )
		self.assert_( dd["ONE"].isSame( dd["TWO"] ) )
		
	def tearDown( self ) :
	
		if os.path.isfile("test/o.sql"):
			os.remove("test/o.sql")
		

class TestEmptyContainerOptimisation( unittest.TestCase ) :

	"""Many of the base classes in IECore currently have no data to store
	in files, but were being allocated a container anyway. This test verifies
	that an io optimisation that doesn't create empty containers doesn't have
	any bad side effects. It's also useful to test the impact of the optimisation."""

	def test( self ) :
	
		c = CompoundData()
		
		for i in range( 0, 1000 ) :
		
			d = IntData( i )
			c[str(i)] = d
			c[str(i)+"SecondReference"] = d
		
		ObjectWriter( c, "test/emptyContainerOptimisation.cob" ).write()
	
		c1 = ObjectReader( "test/data/cobFiles/beforeEmptyContainerOptimisation.cob" ).read()
		c2 = ObjectReader( "test/emptyContainerOptimisation.cob" ).read()
		
		self.assertEqual( c1, c2 )
		
	def tearDown( self ) :
	
		if os.path.isfile( "test/emptyContainerOptimisation.cob" ) :
			os.remove( "test/emptyContainerOptimisation.cob" )	
	
if __name__ == "__main__":
    unittest.main()   
