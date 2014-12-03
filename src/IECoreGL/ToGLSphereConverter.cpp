//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SpherePrimitive.h"
#include "IECore/MessageHandler.h"

#include "IECoreGL/ToGLSphereConverter.h"
#include "IECoreGL/SpherePrimitive.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLSphereConverter );

ToGLConverter::ConverterDescription<ToGLSphereConverter> ToGLSphereConverter::g_description;

ToGLSphereConverter::ToGLSphereConverter( IECore::ConstSpherePrimitivePtr toConvert )
	:	ToGLConverter( "Converts IECore::SpherePrimitive objects to IECoreGL::SpherePrimitive objects.", IECore::SpherePrimitiveTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::SpherePrimitive>( toConvert ) );
}

ToGLSphereConverter::~ToGLSphereConverter()
{
}

IECore::RunTimeTypedPtr ToGLSphereConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	const IECore::SpherePrimitive *coreSphere = static_cast<const IECore::SpherePrimitive *>( src.get() );
	IECoreGL::SpherePrimitivePtr glSphere = new IECoreGL::SpherePrimitive( coreSphere->radius(), coreSphere->zMin(), coreSphere->zMax(), coreSphere->thetaMax() );

	for( IECore::PrimitiveVariableMap::const_iterator it = coreSphere->variables.begin(), eIt = coreSphere->variables.end(); it != eIt; ++it )
	{
		if( it->second.data )
		{
			glSphere->addPrimitiveVariable( it->first, it->second );
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLSphereConverter", boost::format( "No data given for primvar \"%s\"" ) % it->first );
		}
	}

	return glSphere;
}
