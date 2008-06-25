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

#include <cassert>

#include "boost/format.hpp"

#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/MeshPrimitive.h"

#include "IECore/MeshPrimitive.h"
#include "IECore/TriangulateOp.h"
#include "IECore/MeshNormalsOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"

using namespace IECoreGL;

class ToGLMeshConverter::ToFaceVaryingConverter
{
	public:
	
		typedef IECore::DataPtr ReturnType;
		
		ToFaceVaryingConverter( IECore::ConstIntVectorDataPtr vertIds ) : m_vertIds( vertIds )
		{
			assert( m_vertIds );
		}
		
		template<typename T>
		IECore::DataPtr operator()( typename T::Ptr inData )
		{	
			assert( inData );
				
			const typename T::Ptr outData = new T();
			outData->writable().resize( m_vertIds->readable().size() );
			
			typename T::ValueType::iterator outIt = outData->writable().begin();
			
			for ( typename T::ValueType::size_type i = 0; i <  m_vertIds->readable().size(); i++ )
			{
				*outIt++ = inData->readable()[ m_vertIds->readable()[ i ] ];
			}
							
			return outData;
		}
		
		IECore::ConstIntVectorDataPtr m_vertIds;
};

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
	IECore::MeshPrimitivePtr mesh = boost::static_pointer_cast<IECore::MeshPrimitive>( src->copy() ); // safe because the parameter validated it for us
	
	IECore::TriangulateOpPtr op = new IECore::TriangulateOp();
	op->inputParameter()->setValue( mesh );
	
	mesh = IECore::runTimeCast< IECore::MeshPrimitive > ( op->operate() );
	assert( mesh );
			
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
	
	IECore::ConstV3fVectorDataPtr n = 0;
	IECore::PrimitiveVariableMap::const_iterator nIt = mesh->variables.find( "N" );
	if( nIt != mesh->variables.end() )
	{
		if( nIt->second.interpolation==IECore::PrimitiveVariable::Vertex || nIt->second.interpolation==IECore::PrimitiveVariable::Varying || nIt->second.interpolation==IECore::PrimitiveVariable::FaceVarying )
		{
			n = IECore::runTimeCast<const IECore::V3fVectorData>( nIt->second.data );
		}
		if( !n )
		{
			throw IECore::Exception( "Must specify primitive variable \"N\", of type V3fVectorData" );
		}
	}
	else
	{
		IECore::MeshNormalsOpPtr normOp = new IECore::MeshNormalsOp();
		normOp->inputParameter()->setValue( mesh );
	
		mesh = IECore::runTimeCast< IECore::MeshPrimitive > ( normOp->operate() );
		assert( mesh );
		
		nIt = mesh->variables.find( "N" );
		assert( nIt != mesh->variables.end() );		
		n = IECore::runTimeCast<const IECore::V3fVectorData>( nIt->second.data );
	}
	assert( n );
		
	MeshPrimitivePtr glMesh = new MeshPrimitive( mesh->vertexIds(), p );
	
	ToFaceVaryingConverter primVarConverter( mesh->vertexIds() );
	
	for ( IECore::PrimitiveVariableMap::iterator pIt = mesh->variables.begin(); pIt != mesh->variables.end(); ++pIt )
	{
		if ( pIt->second.data )
		{
			if ( pIt->second.interpolation==IECore::PrimitiveVariable::Vertex || pIt->second.interpolation==IECore::PrimitiveVariable::Varying )
			{
				IECore::DataPtr newData = IECore::despatchTypedData< ToFaceVaryingConverter, IECore::TypeTraits::IsVectorTypedData >( pIt->second.data, primVarConverter );
				pIt->second.interpolation = IECore::PrimitiveVariable::FaceVarying;
				glMesh->addVertexAttribute( pIt->first, newData );
			} 
			else if ( pIt->second.interpolation==IECore::PrimitiveVariable::FaceVarying )
			{
				glMesh->addVertexAttribute( pIt->first, pIt->second.data );
			}
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", boost::format( "No data given for primvar \"%s\"" ) % pIt->first );
		}
	}
	
	IECore::PrimitiveVariableMap::const_iterator sIt = mesh->variables.find( "s" );
	IECore::PrimitiveVariableMap::const_iterator tIt = mesh->variables.find( "t" );
	if ( sIt != mesh->variables.end() && tIt != mesh->variables.end() )
	{
		if ( sIt->second.interpolation == IECore::PrimitiveVariable::FaceVarying
			&&  tIt->second.interpolation == IECore::PrimitiveVariable::FaceVarying )
		{
			IECore::ConstFloatVectorDataPtr s = IECore::runTimeCast< const IECore::FloatVectorData >( sIt->second.data );	
			IECore::ConstFloatVectorDataPtr t = IECore::runTimeCast< const IECore::FloatVectorData >( tIt->second.data );	
			
			if ( s && t )
			{
				/// Should hold true if primvarsAreValid
				assert( s->readable().size() == t->readable().size() );
				
				IECore::V2fVectorDataPtr stData = new IECore::V2fVectorData();
				stData->writable().resize( s->readable().size() );
				
				for ( unsigned i = 0; i < s->readable().size(); i++ )
				{
					stData->writable()[i] = Imath::V2f( s->readable()[i], t->readable()[i] );
				}
				
				glMesh->addVertexAttribute( "st", stData );
			}
			else
			{
				IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "If specified, primitive variables \"s\" and \"t\" must be of type FloatVectorData and interpolation type FaceVarying." );
			}	
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "If specified, primitive variables \"s\" and \"t\" must be of type FloatVectorData and interpolation type FaceVarying." );
		}
	}
	else if ( sIt != mesh->variables.end() || tIt != mesh->variables.end() )
	{
		IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "Primitive variable \"s\" or \"t\" found, but not both." );
	}
	
	return glMesh;
}
