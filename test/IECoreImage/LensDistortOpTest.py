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

import sys
import unittest
import IECore
import IECoreImage

class LensDistortOpTest(unittest.TestCase):

	def testDistortOpWithStandardLensModel(self):

		# The lens model and parameters to use.
		o = IECore.CompoundObject()
		o["lensModel"] = IECore.StringData( "StandardRadialLensModel" )
		o["distortion"] = IECore.DoubleData( 0.2 )
		o["anamorphicSqueeze"] = IECore.DoubleData( 1. )
		o["curvatureX"] = IECore.DoubleData( 0.2 )
		o["curvatureY"] = IECore.DoubleData( 0.5 )
		o["quarticDistortion"] = IECore.DoubleData( .1 )

		# The input image to read.
		r = IECore.Reader.create("test/IECoreImage/data/exr/uvMapWithDataWindow.100x100.exr")
		img = r.read()

		# Create the Op and set it's parameters.
		op = IECoreImage.LensDistortOp()
		op["input"] = img
		op["mode"] = IECore.LensModel.Undistort
		op['lensModel'].setValue(o)

		# Run the Op.
		out = op()

		r = IECore.Reader.create("test/IECoreImage/data/exr/uvMapWithDataWindowDistorted.100x100.exr")
		img2 = r.read()

		self.assertEqual( img.displayWindow, img2.displayWindow )

