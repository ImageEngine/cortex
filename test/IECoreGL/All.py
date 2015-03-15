##########################################################################
#
#  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

from Shader import *
from State import *
from Renderer import *
from Group import *
from Texture import *
from ImmediateRenderer import *
from NameStateComponent import *
from HitRecord import *
from Selection import *
from Camera import *
from Image import *
from PointsPrimitive import *
from Orientation import *
from CurvesPrimitiveTest import *
from MeshPrimitiveTest import *
from AlphaTextureTest import *
from LuminanceTextureTest import *
from UserAttributesTest import *
from DeferredRenderer import *
from DiskPrimitiveTest import DiskPrimitiveTest
from ToGLTextureConverter import TestToGLTexureConverter
from PrimitiveTest import *
from CoordinateSystemTest import CoordinateSystemTest
from TextureLoaderTest import TextureLoaderTest
from FontTest import FontTest
from FontLoaderTest import FontLoaderTest
from ToGLConverterTest import ToGLConverterTest
from CachedConverterTest import CachedConverterTest
from InstancingTest import InstancingTest
from BufferTest import BufferTest
from ShadingTest import ShadingTest
from ShaderLoaderTest import ShaderLoaderTest
from ShaderStateComponentTest import ShaderStateComponentTest
from ToGLStateConverterTest import ToGLStateConverterTest

if IECore.withFreeType() :

	from TextTest import *

if __name__ == "__main__":
    unittest.main()
