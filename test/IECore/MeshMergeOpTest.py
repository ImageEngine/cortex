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
from IECore import *
import math

class MeshMergeOpTest( unittest.TestCase ) :

	def testPlanes( self ) :
	
		p1 = MeshPrimitive.createPlane( Box2f( V2f( -1 ), V2f( 0 ) ) )
		p2 = MeshPrimitive.createPlane( Box2f( V2f( 0 ), V2f( 1 ) ) )
		
		merged = MeshMergeOp()( input=p1, mesh=p2 )
		
		for v in PrimitiveVariable.Interpolation.values :
			i = PrimitiveVariable.Interpolation( v )
			if i!=PrimitiveVariable.Interpolation.Invalid and i!=PrimitiveVariable.Interpolation.Constant :
				self.assertEqual( merged.variableSize( i ), p1.variableSize( i ) + p2.variableSize( i ) )
		
		self.assertEqual( len( merged["P"].data ), len( p1["P"].data ) + len( p2["P"].data ) )
					
if __name__ == "__main__":
    unittest.main()   
