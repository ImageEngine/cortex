##########################################################################
#
#  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

import random
import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.cmds

class SplineParameterHandlerTest( unittest.TestCase ) :

	class TestClassFloat( IECore.Parameterised ) :

		def __init__( self ) :

			IECore.Parameterised.__init__( self, "name", "description" )

			self.parameters().addParameter(
				IECore.SplineffParameter(
					name = "spline",
					description = "description",
					defaultValue = IECore.SplineffData(
						IECore.Splineff(
							IECore.CubicBasisf.catmullRom(),
							(
								( 0, 1 ),
								( 0, 1 ),
								( 1, 0 ),
								( 1, 0 ),
							),
						),
					),
				)
			)
			
	class TestClassColor( IECore.Parameterised ) :

		def __init__( self ) :

			IECore.Parameterised.__init__( self, "name", "description" )

			self.parameters().addParameter(
				IECore.SplinefColor3fParameter(
					name = "spline",
					description = "description",
					defaultValue = IECore.SplinefColor3fData(
						IECore.SplinefColor3f(
							IECore.CubicBasisf.catmullRom(),
							(
								( 0, IECore.Color3f( 1, 1, 1 ) ),
								( 0, IECore.Color3f( 1, 1, 1 ) ),
								( 1, IECore.Color3f( 0, 0, 0 ) ),
								( 1, IECore.Color3f( 0, 0, 0 ) ),
							),
						),
					),
				)
			)		

	def testRoundTripFloat( self ) :
				
		node = maya.cmds.createNode( "ieParameterisedHolderNode" )
	
		parameterised = SplineParameterHandlerTest.TestClassFloat()
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		fnPH.setParameterised( parameterised )

		random.seed( 199 )
		numTests = 10

		for i in range( 0, numTests ) :
							
			numPoints = int( random.random() * 12 ) + 2
			
			splinePoints = []
			
			for j in range( 0, numPoints ) :
			
				splinePoints.append( ( random.random(), random.random() ) )
				
			splinePoints.sort()
				
			splinePoints.insert( 0, splinePoints[0] )
			splinePoints.append( splinePoints[-1] )
			assert( len( splinePoints ) >= 4 )
		
			splineData = IECore.SplineffData( 
				IECore.Splineff( 
					IECore.CubicBasisf.catmullRom(), 			
					splinePoints
				)
			)

			parameterised.parameters()["spline"].setValue( splineData )

			# Put the value to the node's attributes
			fnPH.setNodeValue( parameterised.parameters()["spline"], False )

			# Retrieve the value from the node's attributes		
			fnPH.setParameterisedValue( parameterised.parameters()["spline"] )

			# The parameter value should not have changed
			
			data = parameterised.parameters()["spline"].getValue()
			self.assertEqual( len( data.value ), len( splineData.value ) )	
			
			for i in range( 0, len( data.value ) ) :
			
				self.assertAlmostEqual( data.value.keys()[i], splineData.value.keys()[i] )
				self.assertAlmostEqual( data.value.values()[i], splineData.value.values()[i] )		
				
	def testRoundTripColor( self ) :
				
		node = maya.cmds.createNode( "ieParameterisedHolderNode" )
	
		parameterised = SplineParameterHandlerTest.TestClassColor()
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		fnPH.setParameterised( parameterised )

		random.seed( 205 )
		numTests = 10

		for i in range( 0, numTests ) :
							
			numPoints = int( random.random() * 12 ) + 2
			
			splinePoints = []
			
			for j in range( 0, numPoints ) :
			
				splinePoints.append( ( random.random(), IECore.Color3f( random.random(), random.random(), random.random() ) ) )
				
			splinePoints.sort()
				
			splinePoints.insert( 0, splinePoints[0] )
			splinePoints.append( splinePoints[-1] )
			assert( len( splinePoints ) >= 4 )
		
			splineData = IECore.SplinefColor3fData( 
				IECore.SplinefColor3f( 
					IECore.CubicBasisf.catmullRom(), 			
					splinePoints
				)
			)

			parameterised.parameters()["spline"].setValue( splineData )

			# Put the value to the node's attributes
			fnPH.setNodeValue( parameterised.parameters()["spline"], False )

			# Retrieve the value from the node's attributes		
			fnPH.setParameterisedValue( parameterised.parameters()["spline"] )

			# The parameter value should not have changed
			
			data = parameterised.parameters()["spline"].getValue()
			self.assertEqual( len( data.value ), len( splineData.value ) )
			
			for i in range( 0, len( data.value ) ) :
			
				self.assertAlmostEqual( data.value.keys()[i], splineData.value.keys()[i] )
				
				c1 = data.value.values()[i]
				c2 = splineData.value.values()[i]
				
				v1 = IECore.V3f( c1[0], c1[1], c1[2] )
				v2 = IECore.V3f( c2[0], c2[1], c2[2] )				
				
				
				self.assert_( ( v1 - v2 ).length() < 1.e-4 )
		
	
if __name__ == "__main__":
	MayaUnitTest.TestProgram()
