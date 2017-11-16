//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "boost/python.hpp"

#include "IECoreImageBindings/ChannelOpBinding.h"
#include "IECoreImageBindings/ClampOpBinding.h"
#include "IECoreImageBindings/ClientDisplayDriverBinding.h"
#include "IECoreImageBindings/ColorAlgoBinding.h"
#include "IECoreImageBindings/DisplayDriverBinding.h"
#include "IECoreImageBindings/DisplayDriverServerBinding.h"
#include "IECoreImageBindings/EnvMapSamplerBinding.h"
#include "IECoreImageBindings/HdrMergeOpBinding.h"
#include "IECoreImageBindings/ImageCropOpBinding.h"
#include "IECoreImageBindings/ImageDiffOpBinding.h"
#include "IECoreImageBindings/ImageDisplayDriverBinding.h"
#include "IECoreImageBindings/ImagePrimitiveBinding.h"
#include "IECoreImageBindings/ImagePrimitiveParameterBinding.h"
#include "IECoreImageBindings/ImageThinnerBinding.h"
#include "IECoreImageBindings/ImageReaderBinding.h"
#include "IECoreImageBindings/ImageWriterBinding.h"
#include "IECoreImageBindings/FontBinding.h"
#include "IECoreImageBindings/LensDistortOpBinding.h"
#include "IECoreImageBindings/LuminanceOpBinding.h"
#include "IECoreImageBindings/MedianCutSamplerBinding.h"
#include "IECoreImageBindings/MPlayDisplayDriverBinding.h"
#include "IECoreImageBindings/SplineToImageBinding.h"
#include "IECoreImageBindings/SummedAreaOpBinding.h"
#include "IECoreImageBindings/WarpOpBinding.h"

using namespace boost::python;
using namespace IECoreImageBindings;

BOOST_PYTHON_MODULE( _IECoreImage )
{
	bindImageReader();
	bindImageWriter();
	bindChannelOp();
	bindWarpOp();
	bindClampOp();
	bindColorAlgo();
	bindEnvMapSampler();
	bindHdrMergeOp();
	bindImageCropOp();
	bindImageDiffOp();
	bindImageThinner();
	bindImagePrimitive();
	bindImagePrimitiveParameter();
	bindFont();
	bindLensDistortOp();
	bindLuminanceOp();
	bindMedianCutSampler();
	bindSummedAreaOp();
	bindSplineToImage();
	bindDisplayDriver();
	bindDisplayDriverServer();
	bindClientDisplayDriver();
	bindImageDisplayDriver();
	bindMPlayDisplayDriver();

}
