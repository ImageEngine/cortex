##########################################################################
#
#  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
import os.path

import IECore
import IECoreImage
import IECoreGL
IECoreGL.init( False )

class TestToGLTexureConverter( unittest.TestCase ) :


	def testFromImage( self ) :
		""" Test conversion from an ImagePrimitive """

		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/images/colorBarsWithAlphaF512x512.exr" ).read()

		t = IECoreGL.ToGLTextureConverter( i ).convert()
		self.assertFalse( not t.isInstanceOf( IECoreGL.Texture.staticTypeId() ) )

		ii = t.imagePrimitive()

		res = IECoreImage.ImageDiffOp()(
			imageA = i,
			imageB = ii,
			maxError = 0.01,
			skipMissingChannels = False
		)

		self.assertFalse( res.value )

	def testFromCompoundData( self ) :
		""" Test conversion from a CompoundData representation of an ImagePrimitive """

		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/images/colorBarsWithAlphaF512x512.exr" ).read()

		cd = IECore.CompoundData()
		cd["displayWindow"] = IECore.Box2iData( i.displayWindow )
		cd["dataWindow"] = IECore.Box2iData( i.dataWindow )

		cnd = IECore.CompoundData()
		for channel in i.channelNames() :
			cnd[ channel ] = i[ channel ]

		cd["channels"] = cnd

		t = IECoreGL.ToGLTextureConverter( cd ).convert()
		self.assertFalse( not t.isInstanceOf( IECoreGL.Texture.staticTypeId() ) )

		ii = t.imagePrimitive()

		res = IECoreImage.ImageDiffOp()(
			imageA = i,
			imageB = ii,
			maxError = 0.01,
			skipMissingChannels = False
		)

		self.assertFalse( res.value )

	def testMissingChannelCreation( self ) :
		""" Test the creation of missing channels """

		i = IECore.Reader.create( os.path.dirname( __file__ ) + "/images/colorBarsWithAlphaF512x512.exr" ).read()

		cd = IECore.CompoundData()
		cd["displayWindow"] = IECore.Box2iData( i.displayWindow )
		cd["dataWindow"] = IECore.Box2iData( i.dataWindow )

		cnd = IECore.CompoundData()
		cnd[ "R" ] = i[ "R" ]

		cd["channels"] = cnd

		# We are missing a channel and so an exception should be thrown if we try to convert it with the default arguments.
		self.assertRaises( RuntimeError, IECoreGL.ToGLTextureConverter( cd ).convert )

		t = IECoreGL.ToGLTextureConverter( cd, True ).convert()

		ii = t.imagePrimitive()
		self.assertTrue( "R" in ii.channelNames() )
		self.assertTrue( "G" in ii.channelNames() )
		self.assertTrue( "B" in ii.channelNames() )



if __name__ == "__main__":
    unittest.main()
