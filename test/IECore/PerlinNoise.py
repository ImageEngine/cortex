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
import IECore
import random

class TestPerlinNoise( unittest.TestCase ) :

	def testff( self ) :
	
		n = IECore.PerlinNoiseff()
		
		width = 200
		height = 200
				
		f = IECore.FloatVectorData( width * height )
		o = 0
		for i in range( 0, height ) :
			v = 0.5 + n.noise( i/50.0 )
			for j in range( 0, width ) :
				f[o] = v
				o += 1
		
		b = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( width-1, height-1 ) )		
		i = IECore.ImagePrimitive( b, b )
		i["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		i["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		i["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		
		IECore.Writer.create( i, "test/IECore/data/expectedResults/perlinff.exr" ).write()
		e = IECore.Reader.create( "test/IECore/data/expectedResults/perlinff.exr" ).read()
		
		op = IECore.ImageDiffOp()

		res = op(
			imageA = i,
			imageB = e,
			maxError = 0.0005
		)

		self.failIf( res.value ) # Tested against OpenEXR 1.6.1

			
	def testV2ff( self ) :
	
		n = IECore.PerlinNoiseV2ff()
	
		width = 200
		height = 200
				
		f = IECore.FloatVectorData( width * height )
		o = 0
		for i in range( 0, height ) :
			for j in range( 0, width ) :
				f[o] = 0.5 + n.noise( IECore.V2f( i/50.0, j/50.0 ) )
				o += 1
		
		b = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( width-1, height-1 ) )		
		i = IECore.ImagePrimitive( b, b ) 
		i["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		i["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		i["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
		
		e = IECore.Reader.create( "test/IECore/data/expectedResults/perlinV2ff.exr" ).read()
		
		op = IECore.ImageDiffOp()

		res = op(
			imageA = i,
			imageB = e,
			maxError = 0.0005
		)

		self.failIf( res.value ) # Tested against OpenEXR 1.6.1
		
	def testV3ff( self ) :
	
		n = IECore.PerlinNoiseV3ff()
		
		width = 200
		height = 200
				
		f = IECore.FloatVectorData( width * height )
		for frame in range( 0, 5 ) :
			o = 0
			for i in range( 0, height ) :
				for j in range( 0, width ) :
					f[o] = 0.5 + n.noise( IECore.V3f( i/50.0, j/50.0, frame/10.0 ) )
					o += 1
						
			b = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( width-1, height-1 ) )		
			i = IECore.ImagePrimitive( b, b )
			i["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
			i["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
			i["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, f )
			
			e = IECore.Reader.create( "test/IECore/data/expectedResults/perlin3d.%.4d.exr" % frame ).read()
		
			op = IECore.ImageDiffOp()
                
			res = op(
				imageA = i,
				imageB = e,
				maxError = 0.0005
			)
                
			self.failIf( res.value ) # Tested against OpenEXR 1.6.1

	def testV2fColor3f( self ) :
	
		n = IECore.PerlinNoiseV2fColor3f()
	
		width = 200
		height = 200
				
		r = IECore.FloatVectorData( width * height )
		g = IECore.FloatVectorData( width * height )
		b = IECore.FloatVectorData( width * height )
		o = 0
		for i in range( 0, height ) :
			for j in range( 0, width ) :
				c = n.noise( IECore.V2f( i/50.0, j/50.0 ) )
				r[o] = c.r + 0.5
				g[o] = c.g + 0.5
				b[o] = c.b + 0.5
				o += 1
		
		box = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( width-1, height-1 ) )		
		i = IECore.ImagePrimitive( box, box ) 
		i["R"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, r )
		i["G"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, g )
		i["B"] = IECore.PrimitiveVariable( IECore.PrimitiveVariable.Interpolation.Vertex, b )
		
		e = IECore.Reader.create( "test/IECore/data/expectedResults/perlinV2fColor3f.exr" ).read()
		
		op = IECore.ImageDiffOp()

		res = op(
			imageA = i,
			imageB = e,
			maxError = 0.0005
		)

		self.failIf( res.value ) # Tested against OpenEXR 1.6.1

#	def testSpeed( self ) :
#
#		numPoints = 100000
#		p = IECore.V3fVectorData( numPoints )
#		random.seed( 0 )
#		for i in xrange( 0, numPoints ) :
#			p[i] = IECore.V3f( random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ), random.uniform( -1000, 1000 ))
#		v = IECore.FloatVectorData( numPoints )
#
#		n = IECore.PerlinNoiseV3ff()
#		t = IECore.Timer( True )
#		for i in range( 0, 200 ) :
#			vv = n.noiseVector( p, v )
#
#		print t.stop()
#		self.assert_( vv.isSame( v ) )	
		
		# results
		######################################################
		# initial implementation (better weight interpolation)
		######################################################
		#
		#	10.82
		#	10.54
		#	10.76
		#	10.91
		#	10.4		10.686
		#
		######################################################
		# cheaper weight interpolation
		######################################################
		#
		#	9.78
		#	9.52
		#	9.79
		#	9.71
		#	9.73		9.706
		#
		######################################################
		# fast floor function
		######################################################
		#
		#	9.54
		#	9.52
		#	9.26
		#	9.32
		#	9.57		9.442
		#
		######################################################
		# static weight function
		######################################################
		#
		#	9.33
		#	9.15
		#	9.36
		#	9.39
		#	9.7			9.386
		#
		######################################################
		# inline weight function
		######################################################
		#
		#	9.49
		#	9.36
		#	9.24
		#	9.15
		#	9.31		9.31
		#
		######################################################
		# inline noise() and noiseWalk()
		######################################################
		#
		#	7.97
		#	8.08
		#	8.06
		#	8.07
		#	8.06		8.048
		#
		######################################################
		# inline VectorOps
		######################################################
		#
		#	8.01
		#	8.05
		#	7.92
		#	7.96
		#	7.96		7.98
		#
		######################################################
		# gradient in contiguous memory
		######################################################
		#
		#	7.84
		#	7.69
		#	7.85
		#	7.78
		#	7.84		7.8
		#
		######################################################
		# timings made while fixing the bug caused by
		# fastFloatFloor bugs when compiled with optimisiation.
		######################################################
		#
		# failing fast floor function
		#
		# 9.77
		# 9.76
		# 9.71
		# 9.69
		# 9.75
		#
		# passing floor function
		#
		# 8.79
		# 8.83
		# 8.84
		# 8.83
		# 8.84
		#
		# fixed fast floor function
		#
		# 7.2
		# 7.19
		# 7.15
		# 7.15
		# 7.18
		#
		######################################################
		
	def testRepeatability( self ) :
	
		n = IECore.PerlinNoiseV2ff( 10 )
		n2 = IECore.PerlinNoiseV2ff( 10 )
	
		width = 200
		height = 200
				
		for i in range( 0, height ) :
			for j in range( 0, width ) :
				self.assertAlmostEqual( n.noise( IECore.V2f( i/50.0, j/50.0 ) ), n2.noise( IECore.V2f( i/50.0, j/50.0 ) ), 10 )
					
							
if __name__ == "__main__":
	unittest.main()   
	
