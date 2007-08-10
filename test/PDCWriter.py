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

import unittest
import sys
import IECore

class TestPDCWriter( unittest.TestCase ) :

	def testBasics( self ) :
	
		r = IECore.Reader.create( "test/data/pdcFiles/particleShape1.250.pdc" )
		p = r.read()
		
		w = IECore.Writer.create( p, "/tmp/particleShape1.250.pdc" )
		w.write()
		
		r = IECore.Reader.create( "/tmp/particleShape1.250.pdc" )
		p2 = r.read()
		
		self.assertEqual( p, p2 )
		
	def testFiltering( self ) :
	
		r = IECore.Reader.create( "test/data/pdcFiles/particleShape1.250.pdc" )
		p = r.read()
		
		w = IECore.Writer.create( p, "/tmp/particleShape1.250.pdc" )
		w.parameters().attributes.setValue( IECore.StringVectorData( ["position"] ) )
		w.write()
		
		for k in p.keys() :
			if k!="position" :
				del p[k]
		
		r = IECore.Reader.create( "/tmp/particleShape1.250.pdc" )
		p2 = r.read()
		
		self.assertEqual( p, p2 )
		
	def testBadObjectException( self ) :
	
		w = IECore.PDCParticleWriter( IECore.IntData(10), "/tmp/intData.pdc" )
		self.assertRaises( RuntimeError, w.write )
		
	def testWriteConstantData( self ) :
	
		p = IECore.PointsPrimitive( 1 )
		p["d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.DoubleData( 1 ) )
		p["v3d"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.V3dData( IECore.V3d( 1, 2, 3 ) ) )
		p["i"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Constant, IECore.IntData( 10 ) )
		
		w = IECore.Writer.create( p, "/tmp/particleShape1.250.pdc" )
		w.write()
		
		r = IECore.Reader.create( "/tmp/particleShape1.250.pdc" )
		p2 = r.read()
		
		self.assertEqual( p, p2 )
		
if __name__ == "__main__":
	unittest.main()   
	
