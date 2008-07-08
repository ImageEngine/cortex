##########################################################################
#
#  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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
import IECore
import IECoreRI
import os.path
import os

class ArrayPrimVarTest( unittest.TestCase ) :

	def test( self ) :
	
		m = IECore.MeshPrimitive.createPlane( IECore.Box2f( IECore.V2f( -1 ), IECore.V2f( 1 ) ) )
		m["testOne"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringVectorData( [ "one", "two" ] ) )
		m["testTwo"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringVectorData( [ "apple", "banana", "beetroot" ] ) )
		m["testThree"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.StringData( "hi" ) )
		m["testFour"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3fVectorData( [ IECore.V3f( 0 ), IECore.V3f( 1 ) ] ) )
		m["testFive"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.FloatVectorData( [ 10, 11, 12 ] ) )
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testArrayPrimVar.rib" )
		r.worldBegin()
		
		m.render( r )
		
		r.worldEnd()
		
		rib = "".join( open( "test/IECoreRI/output/testArrayPrimVar.rib" ).readlines() )

		self.assert_( '"constant string testOne[2]" [ "one" "two" ]' in rib )
		self.assert_( '"constant string testThree" [ "hi" ]' in rib )
		self.assert_( '"constant string testTwo[3]" [ "apple" "banana" "beetroot" ]' in rib )
		self.assert_( '"constant vector testFour[2]" [ 0 0 0 1 1 1 ]' in rib )
		self.assert_( '"constant float testFive[3]" [ 10 11 12 ]' in rib )
			
	def tearDown( self ) :
	
		if os.path.exists( "test/IECoreRI/output/testArrayPrimVar.rib" ) :
			os.remove( "test/IECoreRI/output/testArrayPrimVar.rib" )
				
if __name__ == "__main__":
    unittest.main()   
