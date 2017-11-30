##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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
import sys
import unittest

class StandardRadialLensModelTest(unittest.TestCase):

	def testModelNames( self ):

		names = IECore.LensModel.lensModels()
		valid = len(names) > 0
		self.assertEqual( valid, True )
		self.assertEqual( names[0], "StandardRadialLensModel" )

	def testStandardRadialLensModel( self ):

		lens = IECore.LensModel.create( "StandardRadialLensModel" )
		lens["distortion"] = 0.2
		lens["anamorphicSqueeze"] = 1.
		lens["curvatureX"] = 0.2
		lens["curvatureY"] = 0.5
		lens["quarticDistortion"] = .1
		lens["lensCenterOffsetXCm"] = .25
		lens["lensCenterOffsetYCm"] = -.1
		lens.validate()

		# Test the full-format distortion.
		window = IECore.Box2i( IECore.V2i( 0, 0 ), IECore.V2i( 2047, 1555 ) )
		bbox = lens.bounds( IECore.LensModel.Distort, window, 2048, 1556 )
		self.assertEqual( bbox, IECore.Box2i( IECore.V2i( 268, 37 ), IECore.V2i( 2034, 1439 ) ) )

		bbox = lens.bounds( IECore.LensModel.Undistort, window, 2048, 1556 )
		self.assertEqual( bbox, IECore.Box2i( IECore.V2i( -1309, -659 ), IECore.V2i( 2243, 2740 ) ) )

	def testStandardRadialLensModelWindowed( self ):

		lens = IECore.LensModel.create( "StandardRadialLensModel" )
		lens["distortion"] = 0.2
		lens["anamorphicSqueeze"] = 1.
		lens["curvatureX"] = 0.2
		lens["curvatureY"] = 0.5
		lens["quarticDistortion"] = .1
		lens["lensCenterOffsetXCm"] = .25
		lens["lensCenterOffsetYCm"] = -.1
		lens.validate()

		# Test windowed distortion.
		window = IECore.Box2i( IECore.V2i( 140, 650 ), IECore.V2i( 1697, 1359 ) )
		bbox = lens.bounds( IECore.LensModel.Undistort, window, 2048, 1556 )
		self.assertEqual( bbox, IECore.Box2i( IECore.V2i( -635, 650 ), IECore.V2i( 1729, 2044 ) ) )

		bbox = lens.bounds( IECore.LensModel.Distort, window, 2048, 1556 )
		self.assertEqual( bbox, IECore.Box2i( IECore.V2i( 351, 640 ), IECore.V2i( 1696, 1298 ) ) )

	def testStandardRadialLensModelCreatorFromName( self ):

		lens = IECore.LensModel.create( "StandardRadialLensModel" )
		self.assertEqual( lens.typeName(), "StandardRadialLensModel" )

	def testStandardRadialLensModelCreatorFromObj( self ):

		o = IECore.CompoundObject()
		o["lensModel"] = IECore.StringData( "StandardRadialLensModel" )
		lens = IECore.LensModel.create(o)
		self.assertEqual( lens.staticTypeName(), "StandardRadialLensModel" )

	def testCreatorWithParameters( self ):

		o = IECore.CompoundObject()
		o["lensModel"] = IECore.StringData( "StandardRadialLensModel" )
		o["distortion"] = IECore.DoubleData( 0.2 )
		o["anamorphicSqueeze"] = IECore.DoubleData( 1. )
		o["curvatureX"] = IECore.DoubleData( 0.2 )
		o["curvatureY"] = IECore.DoubleData( 0.5 )
		o["quarticDistortion"] = IECore.DoubleData( .1 )

		lens = IECore.LensModel.create(o)
		self.assertEqual( lens.typeName(), "StandardRadialLensModel" )
		self.assertEqual( lens["distortion"].getNumericValue(), 0.2 )
		self.assertEqual( lens["anamorphicSqueeze"].getNumericValue(), 1. )
		self.assertEqual( lens["curvatureX"].getNumericValue(), 0.2 )
		self.assertEqual( lens["curvatureY"].getNumericValue(), 0.5 )

	def testReconstructionFromParameters( self ):

		l1 = IECore.LensModel.create( "StandardRadialLensModel" )
		l1["distortion"] = 0.2
		o = l1.parameters().getValue()
		self.assertEqual( o['lensModel'].value, "StandardRadialLensModel" )
		self.assertEqual( o['distortion'].value, 0.2 )

		l2 = IECore.LensModel.create(o);
		self.assertEqual( l2['lensModel'].getTypedValue(), "StandardRadialLensModel" )
		self.assertEqual( l2.typeName(), "StandardRadialLensModel" )
		self.assertEqual( l2["distortion"].getNumericValue(), 0.2 )

