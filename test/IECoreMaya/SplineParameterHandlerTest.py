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

import IECore
import IECoreMaya
import unittest
import MayaUnitTest
import maya.cmds

class SplineParameterHandlerTest( unittest.TestCase ) :

	class TestClass( IECore.Parameterised ) :

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

	def testRoundTrip( self ) :
				
		node = maya.cmds.createNode( "ieParameterisedHolderNode" )
	
		parameterised = SplineParameterHandlerTest.TestClass()
		fnPH = IECoreMaya.FnParameterisedHolder( node )
		fnPH.setParameterised( parameterised )

		splineData = IECore.SplineffData( 
			IECore.Splineff( 
			IECore.CubicBasisf( 
				IECore.M44f( -0.5, 1.5, -1.5, 0.5, 1, -2.5, 2, -0.5, -0.5, 0, 0.5, 0, 0, 1, 0, 0 ), 
				1 
			), 
			
			( ( 0, 0 ), ( 0, 0 ), ( 1, 0 ),( 1, 0 ) ) )
		)
		
		parameterised.parameters()["spline"].setValue( splineData )
		
		# Put the value to the node's attributes						
		fnPH.setNodeValue( parameterised.parameters()["spline"] )
		
		# Retrieve the value from the node's attributes		
		fnPH.setParameterisedValue( parameterised.parameters()["spline"] )		
		
		# The parameter value should not have changed
		self.assertEqual( parameterised.parameters()["spline"].getValue(), splineData ) # Known bug

	
if __name__ == "__main__":
	MayaUnitTest.TestProgram()
