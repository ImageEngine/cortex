//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
#include "IECoreMaya/bindings/ParameterisedHolderBinding.h"
#include "IECoreMaya/bindings/MayaPythonUtilBinding.h"
#include "IECoreMaya/bindings/MObjectBinding.h"
#include "IECoreMaya/bindings/NodeBinding.h"
#include "IECoreMaya/bindings/DagNodeBinding.h"
#include "IECoreMaya/bindings/FromMayaConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaPlugConverterBinding.h"
#include "IECoreMaya/bindings/PlugBinding.h"
#include "IECoreMaya/bindings/FromMayaObjectConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaCameraConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaCameraConverterBinding.h"
#include "IECoreMaya/bindings/MayaMeshBuilderBinding.h"
#include "IECoreMaya/bindings/FromMayaShapeConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaCurveConverterBinding.h"
#include "IECoreMaya/bindings/FromMayaParticleConverterBinding.h"
#include "IECoreMaya/bindings/StandaloneBinding.h"
#include "IECoreMaya/bindings/FromMayaDagNodeConverterBinding.h"
#include "IECoreMaya/bindings/TypeIdBinding.h"

using namespace IECore;
using namespace IECoreMaya;

using namespace boost::python;

/// Maya is built with 4-byte Unicode characters, so we need to ensure that we're doing 
/// the same so that all external symbols resolve correctly at runtime.
#if ( MAYA_API_VERSION >= 2008 )
/// \todo Assert for earlier versions too, once installations of Python 2.5 for gcc4.0.2
/// are built correctly
BOOST_STATIC_ASSERT(sizeof(Py_UNICODE) == 4);
#endif

BOOST_PYTHON_MODULE(_IECoreMaya)
{
	bindMayaPythonUtil();		
	bindMObject();
	bindNode();
	bindDagNode();
	bindParameterisedHolder();
	bindFromMayaConverter();
	bindFromMayaPlugConverter();
	bindPlug();
	bindFromMayaObjectConverter();
	bindFromMayaDagNodeConverter();	
	bindFromMayaCameraConverter();
	bindMayaMeshBuilder();
	bindTypeId();
	bindFromMayaShapeConverter();
	bindFromMayaCurveConverter();
	bindFromMayaParticleConverter();
	bindStandalone();	
}
