##########################################################################
#
#  Copyright (c) 2009-2015, Image Engine Design Inc. All rights reserved.
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
import os
import unittest

import maya.cmds
import imath

import IECore
import IECoreMaya

class SplineParameterHandlerTest( IECoreMaya.TestCase ) :

	class TestClassFloat( IECore.Parameterised ) :

		def __init__( self ) :

			IECore.Parameterised.__init__( self, "description" )

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

			IECore.Parameterised.__init__( self, "description" )

			self.parameters().addParameter(
				IECore.SplinefColor3fParameter(
					name = "spline",
					description = "description",
					defaultValue = IECore.SplinefColor3fData(
						IECore.SplinefColor3f(
							IECore.CubicBasisf.catmullRom(),
							(
								( 0, imath.Color3f( 1, 1, 1 ) ),
								( 0, imath.Color3f( 1, 1, 1 ) ),
								( 1, imath.Color3f( 0, 0, 0 ) ),
								( 1, imath.Color3f( 0, 0, 0 ) ),
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
		numTests = 100

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

				splinePoints.append( ( random.random(), imath.Color3f( random.random(), random.random(), random.random() ) ) )

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

				v1 = imath.V3f( c1[0], c1[1], c1[2] )
				v2 = imath.V3f( c2[0], c2[1], c2[2] )


				self.assertTrue( ( v1 - v2 ).length() < 1.e-4 )

	def testRoundTripAfterSerialisation( self ) :

		# make a scene with an OpHolder holding an op with a spline parameter
		fnOH = IECoreMaya.FnOpHolder.create( "test", "splineInput", 1 )
		opNode = fnOH.fullPathName()
		op = fnOH.getOp()

		# save it
		maya.cmds.file( rename = os.getcwd() + "/test/IECoreMaya/splineParameterHandlerTest.ma" )
		sceneFileName = maya.cmds.file( force = True, type = "mayaAscii", save = True )

		# load it
		maya.cmds.file( new=True, force=True )
		maya.cmds.file( sceneFileName, force=True, open=True )

		fnOH = IECoreMaya.FnOpHolder( opNode )
		op = fnOH.getOp()

		# stick a new value on
		splineData = IECore.SplineffData(
			IECore.Splineff(
				IECore.CubicBasisf.catmullRom(), (
					( 0, 0.644737 ),
					( 0, 0.644737 ),
					( 0.257426, 0.0789474 ),
					( 1, -0.3 ),
					( 1, -0.3 )
				)
			)
		)

		op["spline"].setValue( splineData )

		# convert the value to maya
		fnOH.setNodeValue( op["spline"] )

		# convert it back
		fnOH.setParameterisedValue( op["spline"] )

		# make sure it worked
		splineData2 = op["spline"].getValue()
		self.assertEqual( splineData, splineData2 )

		# do it all again just for kicks
		op["spline"].setValue( splineData )
		fnOH.setNodeValue( op["spline"] )
		fnOH.setParameterisedValue( op["spline"] )
		splineData2 = op["spline"].getValue()
		self.assertEqual( splineData, splineData2 )

	def testSparseEntries( self ) :

		# load a scene where we have a spline parameter with sparse entries.
		maya.cmds.file( os.getcwd() + "/test/IECoreMaya/scenes/splineWithSparseEntries.ma", force=True, open=True )

		fnOH = IECoreMaya.FnOpHolder( "test" )
		op = fnOH.getOp()

		# stick a new value on
		splineData = IECore.SplineffData(
			IECore.Splineff(
				IECore.CubicBasisf.catmullRom(), (
					( 0, 0.644737 ),
					( 0, 0.644737 ),
					( 0.257426, 0.0789474 ),
					( 1, -0.3 ),
					( 1, -0.3 )
				)
			)
		)

		op["spline"].setValue( splineData )

		# convert the value to maya
		fnOH.setNodeValue( op["spline"] )

		# convert it back
		fnOH.setParameterisedValue( op["spline"] )

		# make sure it worked
		splineData2 = op["spline"].getValue()
		self.assertEqual( splineData, splineData2 )

		# do it all again just for kicks
		op["spline"].setValue( splineData )
		fnOH.setNodeValue( op["spline"] )
		fnOH.setParameterisedValue( op["spline"] )
		splineData2 = op["spline"].getValue()
		self.assertEqual( splineData, splineData2 )

	@unittest.skipIf( maya.OpenMaya.MGlobal.apiVersion() >= 201500, "Reference edits for splines don't work in Maya 2016" )
	def testAddColorSplineToReferencedNode( self ) :

		# make a scene with an empty op holder
		######################################

		maya.cmds.createNode( "ieOpHolderNode" )

		maya.cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "opHolderReference.ma" ) )
		referenceScene = maya.cmds.file( force = True, type = "mayaAscii", save = True )

		# reference it in and add an op with a color spline
		###################################################

		maya.cmds.file( new = True, force = True )
		maya.cmds.file( referenceScene, reference = True, namespace = "ns1" )

		fnOH = IECoreMaya.FnOpHolder( "ns1:ieOpHolderNode1" )
		fnOH.setOp( "colorSplineInput", 1 )

		fnOH.setParameterisedValues()

		self.assertEqual(
			fnOH.getOp()["spline"].getValue().value,
			IECore.SplinefColor3f(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, imath.Color3f( 1 ) ),
					( 0, imath.Color3f( 1 ) ),
					( 1, imath.Color3f( 0 ) ),
					( 1, imath.Color3f( 0 ) ),
				),
			)
		)

		# save the scene, and reload it. check that we've worked
		# around another wonderful maya referencing bug
		########################################################

		maya.cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "opHolderReferencer.ma" ) )
		referencerScene = maya.cmds.file( force = True, type = "mayaAscii", save = True )

		maya.cmds.file( new = True, force = True )
		maya.cmds.file( referencerScene, force = True, open = True )

		fnOH = IECoreMaya.FnOpHolder( "ns1:ieOpHolderNode1" )

		fnOH.setParameterisedValues()

		self.assertEqual(
			fnOH.getOp()["spline"].getValue().value,
			IECore.SplinefColor3f(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, imath.Color3f( 1 ) ),
					( 0, imath.Color3f( 1 ) ),
					( 1, imath.Color3f( 0 ) ),
					( 1, imath.Color3f( 0 ) ),
				),
			)
		)

	@unittest.skipIf( maya.OpenMaya.MGlobal.apiVersion() >= 201500, "Reference edits for splines don't work in Maya 2016" )
	def testAddFloatSplineToReferencedNode( self ) :

		# make a scene with an empty op holder
		######################################

		maya.cmds.createNode( "ieOpHolderNode" )

		maya.cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "opHolderReference.ma" ) )
		referenceScene = maya.cmds.file( force = True, type = "mayaAscii", save = True )

		# reference it in and add an op with a color spline
		###################################################

		maya.cmds.file( new = True, force = True )
		maya.cmds.file( referenceScene, reference = True, namespace = "ns1" )

		fnOH = IECoreMaya.FnOpHolder( "ns1:ieOpHolderNode1" )
		fnOH.setOp( "splineInput", 1 )

		fnOH.setParameterisedValues()

		self.assertEqual(
			fnOH.getOp()["spline"].getValue().value,
			IECore.Splineff(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, 1 ),
					( 0, 1 ),
					( 1, 0 ),
					( 1, 0 ),
				),
			)
		)

		# save the scene, and reload it. check that we've worked
		# around another wonderful maya referencing bug
		########################################################

		maya.cmds.file( rename = os.path.join( os.getcwd(), "test", "IECoreMaya", "opHolderReferencer.ma" ) )
		referencerScene = maya.cmds.file( force = True, type = "mayaAscii", save = True )

		maya.cmds.file( new = True, force = True )
		maya.cmds.file( referencerScene, force = True, open = True )

		fnOH = IECoreMaya.FnOpHolder( "ns1:ieOpHolderNode1" )

		fnOH.setParameterisedValues()

		self.assertEqual(
			fnOH.getOp()["spline"].getValue().value,
			IECore.Splineff(
				IECore.CubicBasisf.catmullRom(),
				(
					( 0, 1 ),
					( 0, 1 ),
					( 1, 0 ),
					( 1, 0 ),
				),
			)
		)

	def tearDown( self ) :

		paths = [
			os.getcwd() + "/test/IECoreMaya/splineParameterHandlerTest.ma",
			os.path.join( os.getcwd(), "test", "IECoreMaya", "opHolderReference.ma" ),
			os.path.join( os.getcwd(), "test", "IECoreMaya", "opHolderReferencer.ma" ),
		]

		for path in paths :
			if os.path.exists( path ) :
				os.remove( path )

if __name__ == "__main__":
	IECoreMaya.TestProgram( plugins = [ "ieCore" ] )
