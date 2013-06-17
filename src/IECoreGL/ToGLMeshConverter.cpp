//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MeshNormalsOp.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/MessageHandler.h"

#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/MeshPrimitive.h"
#include "IECoreGL/CachedConverter.h"

using namespace IECoreGL;


//////////////////////////////////////////////////////////////////////////
// CreateNormalsConverter
//////////////////////////////////////////////////////////////////////////

class ToGLMeshConverter::CreateNormalsConverter
{
	public :

		IECore::MurmurHash hash( const IECore::Object *object ) const
		{
			IECore::MurmurHash h;
			h.append( "CreateNormalsConverter");
			const IECore::MeshPrimitive *mesh = static_cast< const IECore::MeshPrimitive * >(object);
			mesh->verticesPerFace()->hash(h);
			mesh->vertexIds()->hash(h);
			IECore::PrimitiveVariableMap::const_iterator pIt = mesh->variables.find("P");
			pIt->second.data->hash(h);
			return h;
		}

		IECore::RunTimeTypedPtr operator()( const IECore::Object *object )
		{
			IECore::MeshNormalsOpPtr normalOp = new IECore::MeshNormalsOp();
			normalOp->inputParameter()->setValue( const_cast< IECore::Object * >(object) );
			normalOp->copyParameter()->setTypedValue( true );
			return normalOp->operate();
		}
};


//////////////////////////////////////////////////////////////////////////
// ToGLMeshConverter
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( ToGLMeshConverter );

ToGLConverter::ConverterDescription<ToGLMeshConverter> ToGLMeshConverter::g_description;

ToGLMeshConverter::ToGLMeshConverter( IECore::ConstMeshPrimitivePtr toConvert )
	:	ToGLConverter( "Converts IECore::MeshPrimitive objects to IECoreGL::MeshPrimitive objects.", IECore::MeshPrimitiveTypeId )
{
	srcParameter()->setValue( IECore::constPointerCast<IECore::MeshPrimitive>( toConvert ) );
}

ToGLMeshConverter::~ToGLMeshConverter()
{
}

IECore::RunTimeTypedPtr ToGLMeshConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECore::ConstMeshPrimitivePtr m = IECore::staticPointerCast< const IECore::MeshPrimitive>( src ); // safe because the parameter validated it for us

	// we copy first the mesh because we will run some ops on it.
	IECore::MeshPrimitivePtr mesh = m->copy();

	/// \todo consider generating Normals when 'P' is added as a primVar. So we can update only 'P' and have normals recomputed.
	if( mesh->interpolation() != "linear" )
	{
		// it's a subdivision mesh. in the absence of a nice subdivision algorithm to display things with,
		// we can at least make things look a bit nicer by calculating some smooth shading normals.
		// if interpolation is linear and no normals are provided then we assume the faceted look is intentional.
		if( mesh->variables.find( "N" )==mesh->variables.end() )
		{
			CachedConverterPtr cachedConverter = CachedConverter::defaultCachedConverter();
			CreateNormalsConverter createNormals;
			IECore::ConstRunTimeTypedPtr newMesh = cachedConverter->convert( mesh, createNormals );
			IECore::ConstDataPtr newNormals = static_cast< const IECore::MeshPrimitive * >(newMesh.get())->variables.find("N")->second.data;
			mesh->variables["N"] = IECore::PrimitiveVariable( IECore::PrimitiveVariable::Vertex, newNormals->copy() );
		}
	}

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
		throw IECore::Exception( "Could not find primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
	}

	const std::vector<int> &verticesPerFaceReadable = mesh->verticesPerFace()->readable();

	/// Check if the input mesh is already triangulated
	bool alreadyTriangulated = true;
	for ( std::vector<int>::const_iterator it = verticesPerFaceReadable.begin(); it != verticesPerFaceReadable.end(); it++ )
	{
		if (*it != 3)
		{
			alreadyTriangulated = false;
			break;
		}
	}

	MeshPrimitivePtr glMesh;

	if ( alreadyTriangulated )
	{
		glMesh = new MeshPrimitive( mesh->vertexIds() );
	}
	else
	{
		glMesh = new MeshPrimitive( mesh->verticesPerFace(), mesh->vertexIds() );
	}

	IECore::PrimitiveVariableMap::const_iterator sIt = mesh->variables.end();
	IECore::PrimitiveVariableMap::const_iterator tIt = mesh->variables.end();

	// add the primitives to the mesh (which know how to triangulate)
	for ( IECore::PrimitiveVariableMap::iterator pIt = mesh->variables.begin(); pIt != mesh->variables.end(); ++pIt )
	{
		/// only process valid prim vars
		if ( !mesh->isPrimitiveVariableValid( pIt->second ) )
		{
			continue;
		}

		if ( pIt->second.data )
		{
			if ( pIt->first == "s" )
			{
				sIt = pIt;
			}
			else if ( pIt->first == "t" )
			{
				tIt = pIt;
			}
			glMesh->addPrimitiveVariable( pIt->first, pIt->second );
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "MeshPrimitive", boost::format( "No data given for primvar \"%s\"" ) % pIt->first );
		}
	}

	/// \todo remove all this when we start supporting a V2f primVar for UVs.
	/// create variable 'st' from 's' and 't'
	if ( sIt != mesh->variables.end() && tIt != mesh->variables.end() )
	{
		if ( sIt->second.interpolation == tIt->second.interpolation && 
			 sIt->second.interpolation != IECore::PrimitiveVariable::Constant )
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
				glMesh->addPrimitiveVariable( "st", IECore::PrimitiveVariable( sIt->second.interpolation, stData ) );
			}
			else
			{
				IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "If specified, primitive variables \"s\" and \"t\" must be of type FloatVectorData and interpolation type FaceVarying." );
			}
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "If specified, primitive variables \"s\" and \"t\" must be of type FloatVectorData and non-Constant interpolation type." );
		}
	}
	else if ( sIt != mesh->variables.end() || tIt != mesh->variables.end() )
	{
		IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "Primitive variable \"s\" or \"t\" found, but not both." );
	}

	return glMesh;
}
