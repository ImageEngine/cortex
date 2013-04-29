//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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

#include "maya/MPxNode.h"
#include "maya/MSelectionList.h"
#include "maya/MFnDependencyNode.h"

#include "IECore/Parameterised.h"

#include "IECoreMaya/IECoreMaya.h"
#include "IECoreMaya/bindings/FnParameterisedHolderBinding.h"
#include "IECoreMaya/bindings/MayaPythonUtilBinding.h"
#include "IECoreMaya/bindings/FromMayaConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaPlugConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaObjectConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaCameraConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaCameraConverterBinding.h"
#include "IECoreMaya/bindings/MayaMeshBuilderBinding.h"
#include "IECoreMaya/bindings/FromMayaShapeConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaCurveConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaParticleConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaProceduralHolderConverterBinding.h"
#include "IECoreMaya/bindings/StandaloneBinding.h"
#include "IECoreMaya/bindings/FromMayaDagNodeConverterBinding.h"
#include "IECoreMaya/bindings/TypeIdBinding.h"
#include "IECoreMaya/bindings/MPlugFromPython.h"
#include "IECoreMaya/bindings/MObjectFromPython.h"
#include "IECoreMaya/bindings/MDagPathFromPython.h"
#include "IECoreMaya/bindings/ToMayaConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaPlugConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaObjectConverterBinding.h"
#include "IECoreMaya/bindings/MayaTypeIdBinding.h"
#include "IECoreMaya/bindings/FromMayaMeshConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaTransformConverterBinding.h"
#include "IECoreMaya/bindings/CallbackIdBinding.h"
#include "IECoreMaya/bindings/PlaybackFrameListBinding.h"
#include "IECoreMaya/bindings/MStringFromPython.h"
#include "IECoreMaya/bindings/ViewportPostProcessBinding.h"
#include "IECoreMaya/bindings/ImageViewportPostProcessBinding.h"
#include "IECoreMaya/bindings/ViewportPostProcessCallbackBinding.h"
#include "IECoreMaya/bindings/FromMayaGroupConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaUnitPlugConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaNumericPlugConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaArrayDataConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaCompoundNumericPlugConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaTransformationMatrixConverterBinding.h"
#include "IECoreMaya/bindings/MessageHandlerBinding.h"
#include "IECoreMaya/bindings/FnProceduralHolderBinding.h"
#include "IECoreMaya/bindings/FromMayaSkinClusterConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaSkinClusterConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaMeshConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaGroupConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaParticleConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaImageConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaImageConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaStringPlugConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaCameraConverterBinding.h"
#include "IECoreMaya/bindings/MayaSceneBinding.h"
#include "IECoreMaya/bindings/FnSceneShapeBinding.h"
#include "IECoreMaya/bindings/FromMayaLocatorConverterBinding.h"
#include "IECoreMaya/bindings/ToMayaLocatorConverterBinding.h"

using namespace IECore;
using namespace IECoreMaya;

using namespace boost::python;

/// \todo create proper assertions for unicode length for each version of Maya by OS.
/// Note: the line sizeof(Py_UNICODE) does not work on OSX
#ifdef LINUX
/// On Linux Maya is built with 4-byte Unicode characters, so we need to ensure that we're doing
/// the same so that all external symbols resolve correctly at runtime.
#if ( MAYA_API_VERSION >= 2008 )
BOOST_STATIC_ASSERT(sizeof(Py_UNICODE) == 4);
#endif
#endif

BOOST_PYTHON_MODULE(_IECoreMaya)
{
	bindMayaPythonUtil();
	bindFnParameterisedHolder();
	bindFromMayaConverter();
	bindFromMayaPlugConverter();
	bindFromMayaObjectConverter();
	bindFromMayaDagNodeConverter();
	bindFromMayaCameraConverter();
	bindMayaMeshBuilder();
	bindTypeId();
	bindFromMayaShapeConverter();
	bindFromMayaCurveConverter();
	bindFromMayaParticleConverter();
	bindFromMayaProceduralHolderConverter();
	bindStandalone();
	bindMPlugFromPython();
	bindMObjectFromPython();
	bindMDagPathFromPython();
	bindToMayaConverter();
	bindToMayaPlugConverter();
	bindToMayaObjectConverter();
	bindMayaTypeId();
	bindFromMayaMeshConverter();
	bindFromMayaTransformConverter();
	bindCallbackId();
	bindPlaybackFrameList();
	bindMStringFromPython();
	bindViewportPostProcess();
	bindImageViewportPostProcess();
	bindViewportPostProcessCallback();
	bindFromMayaGroupConverter();
	bindFromMayaUnitPlugConverter();
	bindFromMayaNumericPlugConverter();
	bindFromMayaArrayDataConverter();
	bindFromMayaCompoundNumericPlugConverter();
	bindFromMayaTransformationMatrixConverter();
	bindMessageHandler();
	bindFnProceduralHolder();
	bindFromMayaSkinClusterConverter();
	bindToMayaSkinClusterConverter();
	bindToMayaMeshConverter();
	bindToMayaGroupConverter();
	bindToMayaParticleConverter();
	bindToMayaImageConverter();
	bindFromMayaImageConverter();
	bindFromMayaStringPlugConverter();
	bindToMayaCameraConverter();
	bindMayaScene();
	bindFnSceneShape();
	bindFromMayaLocatorConverter();
	bindToMayaLocatorConverter();
}
