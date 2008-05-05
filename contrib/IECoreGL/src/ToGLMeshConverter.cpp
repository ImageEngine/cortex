//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/MeshPrimitive.h"

#include "IECore/MeshPrimitive.h"

using namespace IECoreGL;

ToGLMeshConverter::ToGLMeshConverter( IECore::ConstMeshPrimitivePtr toConvert )
	:	ToGLConverter( staticTypeName(), "Converts IECore::MeshPrimitive objects to IECoreGL::MeshPrimitive objects.", IECore::MeshPrimitiveTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::MeshPrimitive>( toConvert ) );
}

ToGLMeshConverter::~ToGLMeshConverter()
{
}
		
IECore::RunTimeTypedPtr ToGLMeshConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECore::ConstMeshPrimitivePtr mesh = boost::static_pointer_cast<const IECore::MeshPrimitive>( src ); // safe because the parameter validated it for us
	
	IECore::ConstV3fVectorDataPtr p = 0;
	IECore::PrimitiveVariableMap::const_iterator pIt = mesh->variables.find( "P" );
	if( pIt!=mesh->variables.end() )
	{
		if( pIt->second.interpolation==IECore::PrimitiveVariable::Vertex )
		{
			p = IECore::runTimeCast<const IECore::V3fVectorData>( pIt->second.data );
		}	
	}
	if( !p )
	{
		throw IECore::Exception( "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
	}

	return new MeshPrimitive( mesh->verticesPerFace(), mesh->vertexIds(), p );
}
