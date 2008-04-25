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

class MeshNormalsOpTest( unittest.TestCase ) :

	def testPlane( self ) :
	
		p = MeshPrimitive.createPlane( Box2f( V2f( -1 ), V2f( 1 ) ) )
		if "N" in p :
			del p["N"]
		self.assert_( not "N" in p )
		
		pp = MeshNormalsOp()( input=p )
		
		self.assert_( "N" in pp )
		self.assertEqual( pp["N"].interpolation, PrimitiveVariable.Interpolation.Vertex )
		
		normals = pp["N"].data
		self.assert_( normals.isInstanceOf( V3fVectorData.staticTypeId() ) )
		self.assertEqual( normals.size(), pp.variableSize( PrimitiveVariable.Interpolation.Vertex ) )
		
		for n in normals :
		
			self.assertEqual( n, V3f( 0, 0, 1 ) )

	def testOnlyNAdded( self ) :
	
		p = MeshPrimitive.createPlane( Box2f( V2f( -1 ), V2f( 1 ) ) )
		pp = MeshNormalsOp()( input=p )
		del pp["N"]
		
		self.assertEqual( pp, p )
		
	def testSphere( self ) :
	
		s = Reader.create( "test/IECore/data/cobFiles/pSphereShape1.cob" ).read()
		del s["N"]
		self.assert_( not "N" in s )
		
		ss = MeshNormalsOp()( input=s )
		
		self.assert_( "N" in ss )
		self.assertEqual( ss["N"].interpolation, PrimitiveVariable.Interpolation.Vertex )
		
		normals = ss["N"].data
		self.assert_( normals.isInstanceOf( V3fVectorData.staticTypeId() ) )
		self.assertEqual( normals.size(), ss.variableSize( PrimitiveVariable.Interpolation.Vertex ) )
		
		points = ss["P"].data
		for i in range( 0, normals.size() ) :
		
			self.assert_( math.fabs( normals[i].length() - 1 ) < 0.001 )
		
			p = points[i].normalize()
			self.assert_( normals[i].dot( p ) > 0.99 )
			self.assert_( normals[i].dot( p ) < 1.01 )
			
if __name__ == "__main__":
    unittest.main()   
